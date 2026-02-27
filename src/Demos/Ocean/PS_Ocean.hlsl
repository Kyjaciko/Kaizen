// PS_Ocean.hlsl
Texture2D<float4> PerlinTexture : register(t0);
Texture2D<float> FresnelLookup : register(t1);

SamplerState samplerState : register(s0);
SamplerState fresnelSampler : register(s1);

// Constanten (Zorg dat deze matchen met je wens, Deep/Shallow)
static const float3 DEEP_COLOR = float3(0.0f, 0.05f, 0.15f);
static const float3 SHALLOW_COLOR = float3(0.1f, 0.3f, 0.4f); // Iets lichter gemaakt voor contrast

cbuffer lightBuffer : register(b0)
{
    float3 ambientLightColor;
    float ambientLightStrength;
    
    float3 dynamicLightColor;
    float dynamicLightStrength;
    
    float3 dynamicLightPosition;
    float3 padding;
};

cbuffer OceanCB : register(b1)
{
    row_major float4x4 wvpMatrix; // Niet nodig voor projected grid, we bouwen eigen posities
    row_major float4x4 vpMatrix; // Camera View * Projection
    row_major float4x4 projectorMatrix; // Dit is jouw 'outFinalMatrix' (Range * InvProjector)
    float3 cameraPos;
    
    float heightScale; // Hoogte van de golven
    
    float totalTime;
};

// Input struct moet EXACT matchen met VS_OUTPUT
struct PSInput
{
    float4 inPos : SV_POSITION;
    float3 worldPos : TEXCOORD0; // De berekende positie op het water
    float2 texUV : TEXCOORD1; // De coordinaten gebruikt voor de noise
    float3 viewVec : NORMAL; // <--- Essentieel voor belichting
};

/*float4 main(PSInput input) : SV_TARGET
{
    //
    // Show height.
    //

    /*float heightRatio = input.worldPos.y / heightScale;
    float3 waterColor = lerp(DEEP_COLOR, SHALLOW_COLOR, heightRatio);
    return float4(waterColor, 1.0f);*/
    
    //
    // Show normals.
    //
    
    /*// Zorg dat de normal genormaliseerd is
    float3 N = normalize(input.normal);

    // Map van [-1,1] => [0,1] zodat we hem als kleur kunnen zien
    float3 normalColor = N * 0.5f + 0.5f;

    return float4(normalColor * 1.5f, 1.0f);
}*/

float2 RotateUV(float2 uv, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
}

// HULPFUNCTIE: Haal de hoogte op (Zelfde logica als in VS, maar nu voor pixels)
float GetWaveHeight(float2 pXZ, float lod)
{
    float worldScale = 50.0f;
    float2 baseUV = pXZ / worldScale;

    // Laag 1
    float4 noise1 = PerlinTexture.Sample(samplerState, baseUV, lod); // In PS gebruiken we Sample() ipv SampleLevel()

    // Laag 2
    float2 uv2 = RotateUV(baseUV, 2.094) * 2.5f + float2(5.15f, 1.33f);
    float4 noise2 = PerlinTexture.Sample(samplerState, uv2, lod);

    // Laag 3
    float2 uv3 = RotateUV(baseUV, 4.188) * 5.0f + float2(7.91f, -4.24f);
    float4 noise3 = PerlinTexture.Sample(samplerState, uv3, lod);

    return (noise1.r * 1.0f + noise2.r * 0.35f + noise3.r * 0.15f) / 1.5f;
}

float4 main(PSInput input) : SV_TARGET
{
    // -------------------------------------------------------------------------
    // STAP 1: Per-Pixel Normaal Berekening (Tegen de blokkerige reflectie)
    // -------------------------------------------------------------------------
    // We "voelen" de helling van de golf op deze exacte pixel.
    float2 pXZ = input.worldPos.xz;
    
    // Kleine stapjes om de helling te meten (Finite Difference)
    float gridResolution = 256.0f + 1.0f;
    float eps = 0.1f; /*1.0f / gridResolution;*/
    
    float hCenter = GetWaveHeight(pXZ, 0);
    float hRight = GetWaveHeight(pXZ + float2(eps, 0), 0);
    float hForward = GetWaveHeight(pXZ + float2(0, eps), 0);

    float3 vRight = float3(eps, (hRight - hCenter) * heightScale, 0);
    float3 vForward = float3(0, (hForward - hCenter) * heightScale, eps);

    // De haarscherpe normaal voor deze pixel!
    float3 N = normalize(cross(vForward, vRight));

    // show
    
    /*// Map van [-1,1] => [0,1] zodat we hem als kleur kunnen zien
    float3 normalColor = N * 0.5f + 0.5f;

    return float4(normalColor * 1.5f, 1.0f);*/
    
    //
    
    float3 R = reflect(input.viewVec, N); // requires per pixel normal
    float fresnel = FresnelLookup.Sample(fresnelSampler, float2(saturate(dot(N, R)), 0.0f));
    //float R0 = 0.02f;
    // Hoe schuiner je kijkt (dot(N,V) wordt klein), hoe hoger de fresnel (tot 1.0)
    //float fresnel = R0 + (1.0f - R0) * pow(saturate(1.0f - dot(N, R)), 5.0f);
    
    // -------------------------------------------------------------------------
    // 4. Specular Highlight (De Zon)
    // -------------------------------------------------------------------------
    // Blinn-Phong (N dot HalfVector) geeft vaak mooiere resultaten op golven
    float3 sunVec = normalize(dynamicLightPosition);
    float3 H = normalize(sunVec + input.viewVec);
    float specularStrength = pow(saturate(dot(N, H)), 50.0f); // 300 = glans
    
    float3 sunHighlight = dynamicLightColor * dynamicLightStrength * specularStrength;

    // -------------------------------------------------------------------------
    // 5. De Water Kleur (Ambient / Refractie)
    // -------------------------------------------------------------------------
    // Mix diep en ondiep water op basis van de golfhoogte (optioneel)
    float3 waterBase = lerp(DEEP_COLOR, SHALLOW_COLOR, saturate(hCenter + 0.5f));
    //float3 waterBase = (1.f, 1.f, 1.f);
    
    // Voeg ambient licht toe
    waterBase *= (ambientLightColor * ambientLightStrength);
    
    /*float3 sunVec = normalize(dynamicLightPosition);
    float specularFactor = pow(saturate(dot(R, sunVec)), 100); // 100 - 500
    float3 refl_sun = dynamicLightStrength * dynamicLightColor * specularFactor;*/
    
    float3 skyColor = float3(0.1f, 0.1f, 0.1f);
    float3 horizonColor = float3(0.1f, 0.1f, 0.1f);
    float3 envReflection = lerp(horizonColor, skyColor, saturate(R.y));
    
    float3 totalReflection = envReflection + sunHighlight;
    
    float3 finalColor = lerp(waterBase, totalReflection, fresnel);
    return float4(finalColor, 1.0f);

    // Map van [-1,1] => [0,1] zodat we hem als kleur kunnen zien
    /*float3 normalColor = N * 0.5f + 0.5f;

    return float4(normalColor * 1.5f, 1.0f);*/
}