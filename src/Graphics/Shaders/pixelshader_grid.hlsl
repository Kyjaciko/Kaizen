cbuffer perObjectBuffer : register(b0)
{
    row_major float4x4 wvpMatrix;
    row_major float4x4 invViewMatrix;
    row_major float4x4 invProjectionMatrix;
    float cameraNear;
    float cameraFar;
};

struct PS_INPUT
{
    float4 outPosition : SV_POSITION;
    float4 outColor : COLOR;
    
    float3 nearPoint : TEXCOORD0;
    float3 farPoint : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};

float4 Grid(float3 pixelPosition, float distanceLines, float3 color)
{
    float2 coord = pixelPosition.xz / distanceLines;            // Line-coordinate in the XZ-plane.
    float2 derivative = fwidth(coord);                          // Anti-aliasing to make it look better.
    float2 grid = abs(frac(coord - 0.5f) - 0.5f) / derivative;  // Calculate the distance to the nearest grid line.
    float _line = min(grid.x, grid.y);                          // Get the minimum distance to the nearest line, so closest to a z-axis or x-axis grid line.
    float4 pixel_color = float4(color.rgb, 1.0f - min(_line, 1.0f));
    return pixel_color;
}

/*float4 main(PS_INPUT input) : SV_TARGET
{
    // Calculate intersaction with y = 0 (XZ-plane), using the parametric equation of a line.
    float t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);
    
    // If it is outside the camera's view frustum, discard the pixel.
    if (t < 0.0f || t > 1.0f)
        discard;
    
    // Determine the position of the pixel and draw with the relevant color.
    float3 pixel_position = input.nearPoint + t * (input.farPoint - input.nearPoint);
    float4 color = Grid(pixel_position, 10.0f, input.outColor.rgb) + Grid(pixel_position, 1.0f, float3(input.outColor.rgb));
    return color;
}*/

float ComputeDepth(float3 position)
{
    float4 clip_position = mul(float4(position.xyz, 1.0f), wvpMatrix);
    return saturate(clip_position.z / clip_position.w);
}

PS_OUTPUT main(PS_INPUT input) : SV_TARGET
{
    // Calculate intersaction with y = 0 (XZ-plane), using the parametric equation of a line.
    float t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);
    
    // If it is outside the camera's view frustum, discard the pixel.
    if (t < 0.0f || t > 1.0f)
        discard;
    
    // Determine the position of the pixel and draw with the relevant color.
    float3 pixel_position = input.nearPoint + t * (input.farPoint - input.nearPoint);
    float4 color = Grid(pixel_position, 10.0f, input.outColor.rgb) + Grid(pixel_position, 1.0f, input.outColor.rgb);
    
    PS_OUTPUT output;
    output.color = float4(ComputeDepth(pixel_position), ComputeDepth(pixel_position), ComputeDepth(pixel_position), 1.0f);
    //output.color = color;
    output.depth = ComputeDepth(pixel_position);
    return output;
}