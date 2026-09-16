// Should add fadeStart and fadeEnd to this CB.
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

float ComputeDepth(float3 position)
{
    float4 clip_position = mul(float4(position.xyz, 1.0f), wvpMatrix);
    return saturate(clip_position.z / clip_position.w);
}

// Infinite Grid (no fading).
float4 Grid(float3 pixelPosition, float distanceLines, float3 color)
{
    float2 coord = pixelPosition.xz / distanceLines;
    float2 derivative = fwidth(coord);                          // Anti-aliasing.
    float2 grid = abs(frac(coord - 0.5f) - 0.5f) / derivative;  // Calculate the distance to the nearest grid line.
    float _line = min(grid.x, grid.y);                          // Get the minimum distance to the nearest line, so closest to a z-axis or x-axis grid line.
    float4 pixel_color = float4(color.rgb, 1.0f - min(_line, 1.0f));
    return pixel_color;
}

// Grid with fading, dependend on the distance to the camera.
float4 Grid(float3 pixelPosition, float distanceLines, float3 color, float3 cameraPos, float fadeStart, float fadeEnd)
{
    float4 pixel_color = Grid(pixelPosition, distanceLines, color);
    
    float distToCamera = length(pixelPosition.xz - cameraPos.xz);
    float radialFade = 1.f - smoothstep(fadeStart, fadeEnd, distToCamera);
    pixel_color.a *= radialFade;
    
    return pixel_color;
}

// Dynamic grid size with fading.
float4 GridLOD(float3 pixelPosition, float baseCellSize, float3 color, float3 cameraPos, float fadeStartFactor, float fadeEndFactor, float baseLOD = 2.f)
{
    float camHeight = max(abs(cameraPos.y), 1e-4f);
    
    // Continous LOD.
    float lodLevel = log(camHeight / baseCellSize) / log(baseLOD);
    lodLevel = max(lodLevel, 0.f);
    
    float cellSize0 = baseCellSize * pow(baseLOD, floor(lodLevel));
    float cellSize1 = cellSize0 * baseLOD;
    
    float cellSizeContinous = baseCellSize * pow(baseLOD, lodLevel);
    float fadeStart = cellSizeContinous * fadeStartFactor;
    float fadeEnd = cellSizeContinous * fadeEndFactor;
    
    float4 grid0 = Grid(pixelPosition, cellSize0, color, cameraPos, fadeStart, fadeEnd);
    float4 grid1 = Grid(pixelPosition, cellSize1, color, cameraPos, fadeStart, fadeEnd);
    
    grid0.a *= (1.f - frac(lodLevel));
    return grid0 + grid1 * (1.f - grid0.a);
}

PS_OUTPUT main(PS_INPUT input)
{
    // Calculate intersaction with y = 0 (XZ-plane), using the parametric equation of a line.
    float t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);
    
    // If it is outside the camera's view frustum, discard the pixel.
    if (t < 0.0f || t > 1.0f)
        discard;
    
    // Determine the position of the pixel and draw with the relevant color.
    float3 pixel_position = input.nearPoint + t * (input.farPoint - input.nearPoint);
    float4 color = float4(.2f, .2f, .2f, 1.f);
    float3 cameraPos = invViewMatrix._41_42_43;
    
    // Infinite grid (no fading).
    /*{
        color = Grid(pixel_position, 10.0f, input.outColor.rgb) + Grid(pixel_position, 1.0f, input.outColor.rgb);
    }*/
    
    // Grid with fading.
    /*{
        float fadeStart = 0.f;
        float fadeEnd = 100.f;
        color = Grid(pixel_position, 10.0f, input.outColor.rgb, cameraPos, fadeStart, fadeEnd) + Grid(pixel_position, 1.0f, input.outColor.rgb, cameraPos, fadeStart, fadeEnd);
    }*/
    
    // Grid with dynamic size and fading.
    {
        float fadeStart = .5f;
        float fadeEnd = 8.f;
        color = GridLOD(pixel_position, .2f, input.outColor.rgb, cameraPos, fadeStart, fadeEnd);
    }
    
    PS_OUTPUT output;
    //output.color = float4(ComputeDepth(pixel_position).xxx, 1.f);
    //output.color = float4(frac(pixel_position.x * .1f), frac(pixel_position.z * .1f), 0.f, 1.f);
    output.color = color;
    output.depth = ComputeDepth(pixel_position);
    return output;
}