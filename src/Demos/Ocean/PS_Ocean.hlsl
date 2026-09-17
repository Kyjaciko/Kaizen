#include "./Ocean.hlsli"

static const float3 DEEP_COLOR    = float3(0.0f, 0.1f, 0.4f);
static const float3 SHALLOW_COLOR = float3(0.4f, 0.6f, 0.9f);
static const float3 SKY_ZENITH    = float3(0.25f, 0.45f, 0.8f);
static const float3 SKY_HORIZON   = float3(0.75f, 0.85f, 0.95f);
static const float3 SUN_DIR       = normalize(float3(0.3f, 0.5f, 0.6f));

struct PS_INPUT
{
    float4 inPos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float2 xz = input.worldPos.xz;
    
    // Height and normal PER PIXEL from the heightmap.
    float fineTexel = kWorldScale / kTextureResolution / kScale3;
    float eps = max(fineTexel, 0.5f * length(fwidth(xz)));

    float hC = SampleHeight(xz);
    float hL = SampleHeight(xz - float2(eps, 0.0f)) * heightScale;
    float hR = SampleHeight(xz + float2(eps, 0.0f)) * heightScale;
    float hD = SampleHeight(xz - float2(0.0f, eps)) * heightScale;
    float hU = SampleHeight(xz + float2(0.0f, eps)) * heightScale;
    
    float3 N = normalize(float3(hL - hR, 2.0f * eps, hD - hU));
    float3 V = normalize(cameraPos - input.worldPos);
    
    // Fresnel (Schlick) between water color and sky reflection.
    float NdotV = saturate(dot(N, V));
    float fresnel = .02f + .98f * pow(1.f - NdotV, 5.f);
    
    float3 R = reflect(-V, N);
    R.y = max(R.y, 0.f);        // No sky below the horizon.
    
    float3 sky = lerp(SKY_HORIZON, SKY_ZENITH, R.y);
    float3 water = lerp(DEEP_COLOR, SHALLOW_COLOR, saturate(hC * .5f + .5f));
    water *= .5f + .5f * saturate(dot(N, SUN_DIR));
    
    float spec = pow(saturate(dot(R, SUN_DIR)), 256.f) * 2.f;
    
    float3 color = lerp(water, sky, fresnel) + spec;
    return float4(color, 1.f);
}