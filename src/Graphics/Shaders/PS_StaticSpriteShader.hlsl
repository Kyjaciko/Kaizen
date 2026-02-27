Texture2D shaderTexture : register(t0);
SamplerState samplers : register(s0);

struct PS_INPUT
{
    float4 outPosition : SV_POSITION;
    float2 outTexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_Target
{
    return shaderTexture.Sample(samplers, input.outTexCoord);
}