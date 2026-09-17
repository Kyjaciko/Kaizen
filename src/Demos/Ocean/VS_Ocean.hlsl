#include "./Ocean.hlsli"

struct VS_INPUT
{
    float3 position : POSITION; // Not used.
    float2 uv : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 outPos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
};

// Make for every pixel on your screen a point on the water plane.
// The distance between the points will vary based on the distance to the camera, creating a LOD system.
//
// Imagine having a spotlight that illuminates a flat surface diagonally from above.
// If you were to put a transparent sheet of paper with a dotted grid on it in front of the spotlight, you would see the grid projected onto the surface.
// If you were to mark all the points of the grid with a pen, remove the spotlight and put your head in the 
// same position as the light was located in, you would see a grid that would look as if it were right in front
// of you and not seen from an angle. Therefore keeping the same detail with minimal amount of triangles.
float3 ProjectOnSbase(float2 uv)
{
    float4 a = mul(float4(uv, 0.f, 1.f), projectorMatrix); // Point on near plane (z = 0).
    float4 b = mul(float4(uv, 1.f, 1.f), projectorMatrix); // Point on far plane (z = 1).
    float denom = b.y - a.y;
    
    // Calculate intersection with water plane (Sbase, Y = 0).
    // t = -Start.y / (End.y - Start.y).
    // t outside [0, 1] -> ray doesn't hit the water plane (Sbase).
    float t = (abs(denom) > 1e-8f) ? (-a.y / denom) : 1.0f;
    t = saturate(t);

    float4 p = lerp(a, b, t);
    float3 world = p.xyz / max(p.w, 1e-6f);
    world.y = 0.f; // With clamped t, point will not lie exactly on Sbase.
    return world;
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // Project the pixel on the water plane (Sbase).
    float3 Pw = ProjectOnSbase(input.uv);

    // Grid distance in world space (largests counts).
    float3 PwRight = ProjectOnSbase(input.uv + float2(1.f / kGridResolution, 0.f));
    float3 PwUp    = ProjectOnSbase(input.uv + float2(0.f, 1.f / kGridResolution));
    float spacing = max(length(PwRight - Pw), length(PwUp - Pw));
    
    float texelWorld = kWorldScale / kTextureResolution;
    float lod = max(0.f, log2(spacing / texelWorld) + 1.f); // +1 -> Nyquist.
    
    Pw.y += SampleHeightLevel(Pw.xz, lod) * heightScale;

    // Transform the calculated world point to camera clip space.
    output.outPos   = mul(float4(Pw, 1.f), vpMatrix);
    output.worldPos = Pw;
    return output;
}