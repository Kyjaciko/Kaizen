#ifndef OCEAN_HLSLI
#define OCEAN_HLSLI

Texture2D<float4> PerlinTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer OceanCB : register(b0)
{
    row_major float4x4 vpMatrix;
    row_major float4x4 projectorMatrix;
    
    float3 cameraPos;
    float heightScale; // Height of waves.
    
    float totalTime;
    float3 _pad;
};

static const float kTextureResolution = 256.f;
static const float kGridResolution = 256.f;
static const float kWorldScale = 500.f;

static const float kScale2 = 2.1f;
static const float kScale3 = 4.3f;

static const float kAngle2 = 2.094f;
static const float kAngle3 = 4.188f;

float2 RotateUV(float2 uv, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
}

void GetLayerUVs(float2 worldXZ, out float2 uv1, out float2 uv2, out float2 uv3)
{
    float2 baseUV = worldXZ / kWorldScale;
    
    // Rotate every layer.
    uv1 = baseUV;
    uv2 = RotateUV(baseUV, kAngle2) * kScale2 + float2(5.15f, 1.33f);
    uv3 = RotateUV(baseUV, kAngle3) * kScale3 + float2(7.91f, -4.24f);
    
    // Each layer scrolls in a different direction and speed (in texture units/sec),
    uv2 += float2(-0.006f, 0.013f) * totalTime;
    uv3 += float2(0.017f, -0.009f) * totalTime;
}

float CombineLayers(float n1, float n2, float n3)
{
    return (n1 * 1.0f + n2 * 0.35f + n3 * 0.15f) / 1.5f; // [-1, 1].
}

float SampleHeightLevel(float2 worldXZ, float lod)
{
    float2 uv1, uv2, uv3;
    GetLayerUVs(worldXZ, uv1, uv2, uv3);
    
    float n1 = PerlinTexture.SampleLevel(samplerState, uv1, lod).r;
    float n2 = PerlinTexture.SampleLevel(samplerState, uv2, lod + log2(kScale2)).r;
    float n3 = PerlinTexture.SampleLevel(samplerState, uv3, lod + log2(kScale3)).r;
    
    return CombineLayers(n1, n2, n3);
}

float SampleHeight(float2 worldXZ)
{
    float2 uv1, uv2, uv3;
    GetLayerUVs(worldXZ, uv1, uv2, uv3);

    float n1 = PerlinTexture.Sample(samplerState, uv1).r;
    float n2 = PerlinTexture.Sample(samplerState, uv2).r;
    float n3 = PerlinTexture.Sample(samplerState, uv3).r;

    return CombineLayers(n1, n2, n3);
}

#endif