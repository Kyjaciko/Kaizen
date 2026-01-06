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
};

float2 RotateUV(float2 uv, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // -----------------------------------------------------------
    // STAP 5: Projectie & Intersectie (Grid -> Wereld op S_base)
    // -----------------------------------------------------------

    // We hebben een grid punt (u,v) tussen 0 en 1.
    // De projectorMatrix (Range * InvViewProj) transformeert dit direct
    // naar World Space lijnen.
    
    // Punt op Near Plane (Z = 0 in DX)
    float4 PwA = mul(float4(input.uv, 0.0f, 1.0f), projectorMatrix);
    
    // Punt op Far Plane (Z = 1 in DX)
    float4 PwB = mul(float4(input.uv, 1.0f, 1.0f), projectorMatrix);

    // Converteer van Homogeen naar Cartesiaans (Divide by W)
    // Dit geeft de daadwerkelijke 3D lijnen in de wereld.
    float3 lineStart = PwA.xyz / PwA.w;
    float3 lineEnd = PwB.xyz / PwB.w;

    // Bereken intersectie met het water vlak (Y = 0)
    // Lijnvergelijking: P = Start + t * (End - Start)
    // We willen weten waar P.y == 0.
    // 0 = Start.y + t * (End.y - Start.y)
    // t = -Start.y / (End.y - Start.y)
    
    float3 lineDir = lineEnd - lineStart;
    
    // Beveiliging tegen delen door nul (als we parallel aan water kijken)
    float t = 0.0f;
    if (abs(lineDir.y) > 0.0001f)
    {
        t = -lineStart.y / lineDir.y;
    }
    
    // Als t < 0 of t > 1, raakt het grid het water niet binnen de range.
    // Saturate klemt het vast aan de rand (horizon of near plane).
    t = saturate(t);

    // Het punt op het wateroppervlak (vlak)
    float3 Pw = lineStart + lineDir * t;

    // -----------------------------------------------------------
    // STAP 6 & 7: Displacement (Hoogtekaart toepassen)
    // -----------------------------------------------------------

    // 1. Hoe groot is je grid (aantal vertices in de breedte)?
// Dit moet je weten van je C++ code (bijv. 512).
    float gridResolution = 256.0f + 1.0f;

// 2. Wat is de afstand tot de camera?
    float dist = distance(cameraPos, Pw.xyz);

// 3. Bereken de geschatte wereld-grootte van 1 grid-cel op deze afstand.
// Dit is gebaseerd op de projectie matrix eigenschappen (perspectief = dist * constant).
// 'projectorScale' is een fudge factor die afhangt van je FOV (vaak rond 1.0 - 2.0).
    float worldSpaceGridSpacing = (dist / gridResolution) * 2.0f;

// 4. Mip Level berekening
// We willen dat 1 pixel in de texture overeenkomt met 'worldSpaceGridSpacing'.
// Texture size (bijv. 1024) speelt ook mee.
    float textureResolutie = 256.0f; // Grootte van je Perlin noise texture
    float worldScale = 500.0f; // Hoe groot 1 herhaling van de texture is in meters (base scale) // BELANGERIJK: Te klein (50.0): Veel swimming, want de golven zijn kleiner dan de grid-cel grootte in de verte.
                               // Te groot (500.0): Golven zien eruit als heuvels, niet als water.

                               // Golden Spot : Meestal rond de 100.0f tot 150.0f.
    
    float textureTexelSizeWorld = worldScale / textureResolutie;

// De verhouding tussen wat we kunnen tonen (grid) en wat de texture biedt.
    float ratio = worldSpaceGridSpacing / textureTexelSizeWorld;

// Log2 geeft het juiste miplevel
    float lod = log2(max(1.0f, ratio)) + 2.f; // In de praktijk is deze bereking te perfect dus gebruiken we een offset om flikkering/alisaing te voorkomen. BELANGERIJK
    
    float2 baseUV = Pw.xz / worldScale;
    //float combinedHeight = SampleStochastic(PerlinTexture, samplerState, Pw.xz / 20.0f, lod);

    // --- DE MAGIE: 3 Lagen met Rotatie ---
    
    // Laag 1: Basis (0 graden rotatie)
    float2 uv1 = baseUV;
    // float4 noise1 = PerlinTexture.SampleLevel(samplerState, uv1, 0); // Oude manier
    //float4 noise1 = PerlinTexture.SampleLevel(samplerState, uv1, lod); // Met LOD fix

    // Laag 1: Basis
    float4 noise1 = PerlinTexture.SampleLevel(samplerState, uv1, lod);

    // Laag 2: Geroteerd
    // Factor 2.1 zorgt dat het grid niet oplijnt met laag 1
    float2 uv2 = RotateUV(baseUV, 2.094) * 2.1f;
    uv2 += float2(5.15, 1.33);
    float4 noise2 = PerlinTexture.SampleLevel(samplerState, uv2, lod);

    // Laag 3: Geroteerd
    float2 uv3 = RotateUV(baseUV, 4.188) * 4.3f;
    uv3 += float2(7.91, -4.24);
    float4 noise3 = PerlinTexture.SampleLevel(samplerState, uv3, lod);

    // Combineer
    float combinedHeight = (noise1.r * 1.0f + noise2.r * 0.35f + noise3.r * 0.15f) / 1.5f;

    // Pas hoogte toe
    Pw.y += combinedHeight * heightScale;
    
    // Samevatting:
    // Swimming (sampling) blijft prominant: bepaald door snelheid van de animatie van de perlin noise, de hoogte van de golven en het aantal driehoeken in 1 'Lod', wordt veroorzaakt doordat de driehoek van plaats verspringen doordat de camera beweegt
    // Aliasing (nyquist) is sterk verminderd door gebruik van mipmapping (bandbreedte beperking) en LOD berekening, beperk de frequenties, waardoor de hoge frequenties weg gefiltered worden, zodat het raster het kan weergeven.
    // Tiling blijft deels zichtbaar: door de herhaling van de perlin noise textuur, vooral bij lage worldScale.
    
    // -----------------------------------------------------------
    // Output voorbereiden
    // -----------------------------------------------------------

    // Transformeer het berekende wereldpunt naar Camera Clip Space
    output.outPos = mul(float4(Pw, 1.0f), vpMatrix);
    output.worldPos = Pw;
    output.texUV = baseUV;

    return output;
}