cbuffer perFrameBuffer : register(b0)
{
    row_major float4x4 viewProjectionMatrix;
};

cbuffer perObjectBuffer : register(b1)
{
    row_major float4x4 worldMatrix;
};

cbuffer perSpriteBuffer : register(b2)
{
    float4 uvTransform;
};

struct VS_INPUT
{
    float3 inPos : POSITION;
    float2 inTexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float2 outTexCoord : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4x4 wvpMatrix = mul(worldMatrix, viewProjectionMatrix);
    output.outPosition = mul(float4(input.inPos, 1.0f), wvpMatrix);
    //output.outTexCoord = input.inTexCoord;
    output.outTexCoord.x = input.inTexCoord.x * uvTransform.z + uvTransform.x;
    output.outTexCoord.y = input.inTexCoord.y * uvTransform.w + uvTransform.y;
    return output;
}