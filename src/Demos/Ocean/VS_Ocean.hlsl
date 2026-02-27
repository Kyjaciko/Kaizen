/*Texture2D<float4> PerlinTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer OceanCB : register(b0)
{
    row_major float4x4 wvpMatrix;
    row_major float4x4 vpMatrix;
    row_major float4x4 projectorMatrix;
    float heightScale;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 outPos   : SV_POSITION;
    float height    : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Sample the PerlinTexture.
    float4 sampleColor = PerlinTexture.SampleLevel(samplerState, input.uv, 0);
    float height = sampleColor.r;
    
    // Determine world position and transfer to clip space.
    float3 worldPosition;
    worldPosition.x = input.position.x;
    worldPosition.y = height * heightScale;
    worldPosition.z = input.position.z;
    output.outPos = mul(float4(worldPosition, 1.0f), wvpMatrix);

    // Pass height to pixelshader.
    output.height = height;
    return output;
}*/

Texture2D<float4> PerlinTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer OceanCB : register(b0)
{
    row_major float4x4 wvpMatrix; // Niet nodig voor projected grid, we bouwen eigen posities
    row_major float4x4 vpMatrix; // Camera View * Projection
    row_major float4x4 projectorMatrix; // Dit is jouw 'outFinalMatrix' (Range * InvProjector)
    float3 cameraPos;
    
    float heightScale; // Hoogte van de golven
    
    float totalTime;
};

struct VS_INPUT
{
    float3 position : POSITION; // Wordt genegeerd, we gebruiken UVs als grid
    float2 uv : TEXCOORD0; // [0..1] Grid coordinaten
};

struct VS_OUTPUT
{
    float4 outPos : SV_POSITION;
    float3 worldPos : TEXCOORD0; // Handig voor belichting in PS
    float2 texUV : TEXCOORD1; // UVs voor normal mapping etc.
    float3 viewVec : NORMAL; // <--- Essentieel voor belichting
};

float2 RotateUV(float2 uv, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
}

// -------------------------------------------------------------------------
// Hulpfunctie: Ray-Plane Intersectie (Generiek)
// P = Origin + t * Dir
// Plane: dot(N, P) + d = 0
// -------------------------------------------------------------------------
float3 IntersectRayPlane(float3 rayOrigin, float3 rayDir, float3 planeNormal, float planeDist)
{
    // Bereken t
    float denom = dot(planeNormal, rayDir);
    
    // Vermijd delen door nul (parallel aan vlak)
    if (abs(denom) < 0.0001f)
        return rayOrigin + rayDir * 1000.0f; // Far away

    float t = -(dot(planeNormal, rayOrigin) + planeDist) / denom;
    t = max(0.0f, t); // Alleen vooruit kijken

    return rayOrigin + rayDir * t;
}

float3 IntersectionBasePlane(float2 gridUV, float3 planeNormal, float planeDist)
{
    // 1. Definieer het Watervlak (Base Plane)
    // Dit kan elk vlak zijn! (bv. schuin).
    // Vergelijking: dot(N, P) + D = 0.
    // Voor Y=0 vlak: N=(0,1,0), D=0.
    //float3 planeNormal = float3(0, 1, 0);
    //float planeDist = 0.0f;

    // 2. Projector: Van Grid UV -> World Ray
    float4 PwA = mul(float4(gridUV, 0.0f, 1.0f), projectorMatrix); // Near
    float4 PwB = mul(float4(gridUV, 1.0f, 1.0f), projectorMatrix); // Far
    
    float3 lineStart = PwA.xyz / PwA.w;
    float3 lineEnd = PwB.xyz / PwB.w;
    float3 lineDir = lineEnd - lineStart; // Niet genormaliseerd, is segment

    // 3. Intersectie met het Base Plane
    return IntersectRayPlane(lineStart, lineDir, planeNormal, planeDist);
}

// -------------------------------------------------------------------------
// Berekent de uiteindelijke wereldpositie voor een gegeven Grid UV
// Doet Projectie -> Intersectie -> Displacement
// -------------------------------------------------------------------------
float3 GetDisplacedWorldPos(float2 gridUV, float lod, float worldScale, float3 planeNormal = float3(0, 1, 0), float planeDist = 0.0f)
{
    // 3. Intersectie met het Base Plane
    float3 basePos = IntersectionBasePlane(gridUV, planeNormal, planeDist);

    // 4. Noise Sampling (Displacement berekenen)
    float2 noiseUV = basePos.xz / worldScale; // Let op: voor verticale watervallen moet je hier xy of yz gebruiken

    // ... Noise Lagen ...
    float4 noise1 = PerlinTexture.SampleLevel(samplerState, noiseUV, lod);

    float2 uv2 = RotateUV(noiseUV, 2.094f) * 2.1f;
    uv2 += float2(5.15f, 1.33f);
    float4 noise2 = PerlinTexture.SampleLevel(samplerState, uv2, lod);

    float2 uv3 = RotateUV(noiseUV, 4.188f) * 4.3f;
    uv3 += float2(7.91f, -4.24f);
    float4 noise3 = PerlinTexture.SampleLevel(samplerState, uv3, lod);

    float h = (noise1.r * 1.0f + noise2.r * 0.35f + noise3.r * 0.15f) / 1.5f;

    // 5. Displace in de richting van de PLANE NORMAL (Niet perse Y-as!)
    return basePos + (planeNormal * h * heightScale);
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // -----------------------------------------------------------
    // -------------- Projection: Position the grid --------------
    // -----------------------------------------------------------
    
    float3 basePos = IntersectionBasePlane(input.uv, float3(0, 1, 0), 0.0f);

    // -----------------------------------------------------------
    // ---------------------- Anti-aliasing ----------------------
    // -----------------------------------------------------------
    
// 1. Hoe groot is je grid (aantal vertices in de breedte)?
    float gridResolution = 256.0f + 1.0f;
    float dist = distance(cameraPos, basePos);

    // Calculate the estimated world size of 1 grid cell at this distance.
// 'projectorScale' is een fudge factor die afhangt van je FOV (vaak rond 1.0 - 2.0).
    float worldSpaceGridSpacing = (dist / gridResolution) * 2.0f;

// 4. Mip Level berekening
// We willen dat 1 pixel in de texture overeenkomt met 'worldSpaceGridSpacing'.
// Texture size (bijv. 1024) speelt ook mee.
    float textureResolutie = 256.0f + 1.0f; // Grootte van je Perlin noise texture
    float worldScale = 50.0f; // Hoe groot 1 herhaling van de texture is in meters (base scale) // BELANGERIJK: Te klein (50.0): Veel swimming, want de golven zijn kleiner dan de grid-cel grootte in de verte.
                               // Te groot (500.0): Golven zien eruit als heuvels, niet als water.

                               // Golden Spot : Meestal rond de 100.0f tot 150.0f.
    
    float textureTexelSizeWorld = worldScale / textureResolutie;

// De verhouding tussen wat we kunnen tonen (grid) en wat de texture biedt.
    float ratio = worldSpaceGridSpacing / textureTexelSizeWorld;

// Log2 geeft het juiste miplevel
    float lod = log2(max(1.0f, ratio)) + 2.f; // In de praktijk is deze bereking te perfect dus gebruiken we een offset om flikkering/alisaing te voorkomen. BELANGERIJK
    
    // -----------------------------------------------------------
    // ----------------- Height map displacement -----------------
    // -----------------------------------------------------------
    
    //float hCenter = GetWaveHeight(Pw, lod, worldScale);

    // -----------------------------------------------------------
    // ----------------- Height map displacement -----------------
    // -----------------------------------------------------------

    float texelSize = 1.0f / gridResolution;
    // 1. Centrum
    float3 pCenter = GetDisplacedWorldPos(input.uv, lod, worldScale);
    
    // 2. Buur U (Rechts in grid space)
    float3 pU = GetDisplacedWorldPos(input.uv + float2(texelSize, 0), lod, worldScale);
    
    // 3. Buur V (Boven in grid space)
    float3 pV = GetDisplacedWorldPos(input.uv + float2(0, texelSize), lod, worldScale);

    // 4. Bereken vectoren
    float3 vTangent = pU - pCenter;
    float3 vBitangent = pV - pCenter;

    // 5. Kruisproduct voor normaal
    // Volgorde hangt af van je assenstelsel (Left/Right handed), probeer om te draaien als licht verkeerd is.
    float3 normal = normalize(cross(vBitangent, vTangent));
    
    // Samevatting:
    // Swimming (sampling) blijft prominant: bepaald door snelheid van de animatie van de perlin noise, de hoogte van de golven en het aantal driehoeken in 1 'Lod', wordt veroorzaakt doordat de driehoek van plaats verspringen doordat de camera beweegt
    // Aliasing (nyquist) is sterk verminderd door gebruik van mipmapping (bandbreedte beperking) en LOD berekening, beperk de frequenties, waardoor de hoge frequenties weg gefiltered worden, zodat het raster het kan weergeven.
    // Tiling blijft deels zichtbaar: door de herhaling van de perlin noise textuur, vooral bij lage worldScale.
    
    // -----------------------------------------------------------
    // Output voorbereiden
    // -----------------------------------------------------------

    //float3 V = normalize(cameraPos - pCenter);
    float3 V = normalize(pCenter - cameraPos);
    
    // Transformeer het berekende wereldpunt naar Camera Clip Space
    output.outPos = mul(float4(pCenter, 1.0f), vpMatrix);
    output.worldPos = pCenter;
    output.texUV = pCenter.xz / worldScale;
    output.viewVec = V;

    return output;
}