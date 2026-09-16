static const float3 DEEP_COLOR = float3(0.0f, 0.1f, 0.4f);
static const float3 SHALLOW_COLOR = float3(0.4f, 0.6f, 0.9f);

struct PSInput
{
    float4 inPos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float2 texUV : TEXCOORD1;
};

float4 main(PSInput input) : SV_TARGET
{
    // Max wave height is 5.f (kWaveHeight);
    // Should be fetched from the constant buffer!
    float heightNormalized = input.worldPos.y / 5.f;

    float3 waterColor = lerp(DEEP_COLOR, SHALLOW_COLOR, heightNormalized);
    return float4(waterColor, 1.0f);
}