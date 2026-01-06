Texture2D shaderTexture : register(t0);
SamplerState samplers : register(s0);

cbuffer perObjectBuffer : register(b0)
{
    row_major float4x4 wvpMatrix;
};

struct PS_INPUT
{
    float4 outPosition : SV_POSITION;
    float2 outTexCoord : TEXCOORD;
    float4 outColor : COLOR;
};

float4 main(PS_INPUT input) : SV_Target
{
    bool useTexture = (input.outTexCoord.x != 0.0f || input.outTexCoord.y != 0.0f);

    if (useTexture)
    {
        return shaderTexture.Sample(samplers, input.outTexCoord);
    }
    else
    {
        return input.outColor;
    }
}
