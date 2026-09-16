Texture2D<float4> PerlinTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer OceanCB : register(b0)
{
    row_major float4x4 vpMatrix;
    row_major float4x4 projectorMatrix;
    float3 cameraPos;
    
    float heightScale; // Height of waves.
    
    float totalTime;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 outPos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float2 texUV : TEXCOORD1;
};

float2 RotateUV(float2 uv, float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    return float2(uv.x * c - uv.y * s, uv.x * s + uv.y * c);
}

// Make for every pixel on your screen a point on the water plane.
// The distance between the points will vary based on the distance to the camera, creating a LOD system.
//
// Imagine having a spotlight that illuminates a flat surface diagonally from above.
// If you were to put a transparent sheet of paper with a dotted grid on it in front of the spotlight, you would see the grid projected onto the surface.
// If you were to mark all the points of the grid with a pen, remove the spotlight and put your head in the 
// same position as the light was located in, you would see a grid that would look as if it were right in front
// of you and not seen from an angle. Therefore keeping the same detail with minimal amount of triangles.
float3 ProjectToWater(float2 uv)
{
    float4 PwA = mul(float4(uv, 0.0f, 1.0f), projectorMatrix); // Point on near plane (z = 0).
    float4 PwB = mul(float4(uv, 1.0f, 1.0f), projectorMatrix); // Point on far plane (z = 1).

    float3 lineStart = PwA.xyz / PwA.w;
    float3 lineEnd = PwB.xyz / PwB.w;
    
    // Calculate intersection with water plane (Sbase, Y = 0).
    // t = -Start.y / (End.y - Start.y).
    float3 lineDir = lineEnd - lineStart;
    
    // Avoid division by zero (meaning the camera is looking parallel to the water plane).
    float t = 0.0f;
    if (abs(lineDir.y) > 1e-4f)
    {
        t = -lineStart.y / lineDir.y;
    }

    // Point on water plane (Sbase).
    return (lineStart + lineDir * t);
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // Settings, should be passed by the CB.
    static const float textureResolution = 256.f;
    static const float gridResolution = 256.f;
    static const float worldScale = 500.f;
    
    // Project the pixel on the water plane (Sbase).
    float3 Pw = ProjectToWater(input.uv);

    // Apply displacement.
    float2 baseUV;
    {
        float distToCamera = distance(cameraPos, Pw.xyz);
        
        float2 duv = float2(1.0f / gridResolution, 0.0f);
        float3 PwRight = ProjectToWater(input.uv + duv);
        float3 spacingVector = PwRight - Pw;
        float worldSpaceGridSpacing = length(spacingVector);
        
        float textureTexelSizeWorld = worldScale / textureResolution;
        float ratio = worldSpaceGridSpacing / textureTexelSizeWorld;

        float lod = max(0.0f, log2(ratio));
        
        // Use 3 layers of Perlin noise to reduce tiling
        float4 noise1, noise2, noise3;
        baseUV = Pw.xz / worldScale;
        
        // Layer 1: Base (0° rotation)
        {
            float2 uv1 = baseUV;
            noise1 = PerlinTexture.SampleLevel(samplerState, uv1, lod);
        }
        
        // Layer 2: Rotated (factor 2.1 to avoid alignment with layer 1)
        {
            float2 uv2 = RotateUV(baseUV, 2.094) * 2.1f;
            uv2 += float2(5.15, 1.33);
            noise2 = PerlinTexture.SampleLevel(samplerState, uv2, lod);
        }
        
        // Layer 3: Rotated (factor 4.3)
        {
            float2 uv3 = RotateUV(baseUV, 4.188) * 4.3f;
            uv3 += float2(7.91, -4.24);
            noise3 = PerlinTexture.SampleLevel(samplerState, uv3, lod);
        }
        
        float combinedHeight = (noise1.r * 1.0f + noise2.r * 0.35f + noise3.r * 0.15f) / 1.5f;
        Pw.y += combinedHeight * heightScale;
    }

    // Transform the calculated world point to Camera Clip Space.
    output.outPos = mul(float4(Pw, 1.0f), vpMatrix);
    output.worldPos = Pw;
    output.texUV = baseUV;

    return output;
}