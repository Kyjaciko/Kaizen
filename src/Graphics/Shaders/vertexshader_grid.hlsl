cbuffer perObjectBuffer : register(b0)
{
    row_major float4x4 wvpMatrix;
    row_major float4x4 invViewMatrix;
    row_major float4x4 invProjectionMatrix;
    float cameraNear;
    float cameraFar;
};

struct VS_INPUT
{
    float3 inPos : POSITION;
    float4 inColor : COLOR;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float4 outColor : COLOR;
    
    float3 nearPoint : TEXCOORD0;
    float3 farPoint : TEXCOORD1;
};

float3 UnprojectPoint(float x, float y, float z, float4x4 invView, float4x4 invProjection)
{
    float4 ndc = float4(x, y, z, 1.0f);          // Input is already in NDC space.
    float4 view = mul(ndc, invProjectionMatrix);
    float4 world = mul(view, invViewMatrix);
    return world.xyz / world.w;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.outPosition = float4(input.inPos, 1.0f); // Directly using the NDC.
    output.outColor = input.inColor;
    
    // Draw line to check intersection with the XZ-plane (so to check if it's visible).
    output.nearPoint = UnprojectPoint(input.inPos.x, input.inPos.y, 0.0f, invViewMatrix, invProjectionMatrix);
    output.farPoint = UnprojectPoint(input.inPos.x, input.inPos.y, 1.0f, invViewMatrix, invProjectionMatrix);
    
    return output;
}