////////////////////////////////////////////////////////////////////////////////
// General
////////////////////////////////////////////////////////////////////////////////

RWTexture2D<float4> RW_PerlinTexture : register(u0);

cbuffer PerlinCB : register(b0)
{
    uint width;
    uint height;
    uint tileCount;
    uint gridSize;
    
    uint numOctaves;
    float totalTime;
};

float Billow(float perlin)
{
    return abs(perlin);
}

float RidgedNoise(float perlin)
{
    float ridged_noise = 1.0f - Billow(perlin);
    return ridged_noise * ridged_noise;
}

////////////////////////////////////////////////////////////////////////////////
// Method 1
////////////////////////////////////////////////////////////////////////////////

float2 RandomGradient(uint ix, uint iy)
{
    const uint w = 8 * 4; // 4 = sizeof(uint)
    const uint s = w / 2;
    uint a = ix, b = iy;
    a *= 3284157443;
    
    b ^= a << s | a >> w - s;
    b *= 1911520717;
    
    a ^= b << s | b >> w - s;
    a *= 2048419325;
    
    // Map to [0, 2pi].
    float random = a * (3.14159265f / ~(~0u >> 1));
    
    return float2(sin(random + totalTime), cos(random + totalTime));
}

float CubicInterp(float a0, float a1, float w)
{
    return (a1 - a0) * (3.0f - 2.0f * w) * (w * w) + a0;
}

float DotGridGradient(int ix, int iy, float x, float y)
{
    float2 gradient = RandomGradient(ix, iy);
    
    // Calculate distance vector.
    float dx = x - ix;
    float dy = y - iy;
    
    // Calculate dot product of these two vectors.
    return dx * gradient.x + dy * gradient.y;
}

float Perlin1(float x, float y)
{
    // Define grid corners.
    int x0 = (int) floor(x);
    int y0 = (int) floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    // Calculate interpolation weights.
    float sx = x - x0;
    float sy = y - y0;

    // Interpolate the top two grid corners.
    float n0 = DotGridGradient(x0, y0, x, y);
    float n1 = DotGridGradient(x1, y0, x, y);
    float ix0 = CubicInterp(n0, n1, sx);

    // Interpolate the bottom two grid corners.
    n0 = DotGridGradient(x0, y1, x, y);
    n1 = DotGridGradient(x1, y1, x, y);
    float ix1 = CubicInterp(n0, n1, sx);

    // Interpolate the two interpolated values, now in y.
    return CubicInterp(ix0, ix1, sy);
}

void Method1(uint x, uint y)
{
    float val = 0.0f;
    float freq = 1.0f;
    float amp = 1.0f;
    
    for (int i = 0; i < numOctaves; i++)
    {
        val += Perlin1(x * freq / gridSize, y * freq / gridSize) * amp;

        freq *= 2.0f;
        amp /= 2.0f;
    }

    // Add a bit of contrast and clamp the value.
    val *= 1.2f;
    val = clamp(val, -1.0f, 1.0f);

    //float mapped = (val * 0.5f) + 0.5f; // from [-1, 1] to [0, 1]
    RW_PerlinTexture[int2(x, y)] = float4(val, val, val, 1.0f);
    //RW_PerlinTexture[int2(x, y)] = float4(Billow(mapped), Billow(mapped), Billow(mapped), 1.0f);
    //RW_PerlinTexture[int2(x, y)] = float4(RidgedNoise(mapped), RidgedNoise(mapped), RidgedNoise(mapped), 1.0f);
}

////////////////////////////////////////////////////////////////////////////////
// Method 2
////////////////////////////////////////////////////////////////////////////////

float2 RandomGradient(float2 p)
{
    //p = p + 0.01f;
    float x = dot(p, float2(123.4f, 234.5f));
    float y = dot(p, float2(234.5f, 345.6f));
    
    float2 gradient = float2(x, y);
    gradient = sin(gradient);
    gradient = gradient * 43758.5453f;
    return sin(gradient + totalTime);
}

float DrawVector(float2 p, float2 a, float2 b, float th)
{
    float l = length(b - a);
    float2 d = (b - a) / l;
    float2 q = (p - (a + b) * 0.5);
    q = mul(q, float2x2(d.x, -d.y, d.y, d.x));
    q = abs(q) - float2(1, th) * 0.5f;
    return length(max(q, 0.0f)) + min(max(q.x, q.y), 0.0f);
}

float2 Quentic(float2 p)
{
    return p * p * p * (10.0f + p * (-15.0f + p * 6.0f));
}

float Perlin2(float2 uv)
{
    // Setup grid.
    //float2 uv = float2(x, y) / float2(width - 1, height - 1);
    //uv *= tileCount;
    
    float2 grid_id = floor(uv);
    float2 grid_uv = frac(uv);

    // Show grid.
    //float3 color = float3(grid_uv, 0.0f);
    //RW_PerlinTexture[int2(x, y)] = float4(color.rgb, 1.0f);
    
    // Define grid corners.
    float2 bl = grid_id;
    float2 br = grid_id + float2(1.0f, 0.0f);
    float2 tl = grid_id + float2(0.0f, 1.0f);
    float2 tr = grid_id + float2(1.0f, 1.0f);
    
    // Randomize gradients for each corner.
    float2 grad_bl = RandomGradient(bl);
    float2 grad_br = RandomGradient(br);
    float2 grad_tl = RandomGradient(tl);
    float2 grad_tr = RandomGradient(tr);
    
    // Find distance from current pixel to each grid corner.
    float2 distance_bl = grid_uv;
    float2 distance_br = grid_uv - float2(1.0f, 0.0f);
    float2 distance_tl = grid_uv - float2(0.0f, 1.0f);
    float2 distance_tr = grid_uv - float2(1.0f, 1.0f);

    // Calculate the dot products of gradients + distances.
    float dotBl = dot(grad_bl, distance_bl);
    float dotBr = dot(grad_br, distance_br);
    float dotTl = dot(grad_tl, distance_tl);
    float dotTr = dot(grad_tr, distance_tr);
    
    // Smooth out girdUvs.
    grid_uv = Quentic(grid_uv);
    
    // Linear interpolation between 4 dot products.
    float b = lerp(dotBl, dotBr, grid_uv.x);
    float t = lerp(dotTl, dotTr, grid_uv.x);
    float perlin = lerp(b, t, grid_uv.y);
    
    return perlin;
    
    // Show perlin noise.
    //RW_PerlinTexture[int2(x, y)] = float4(perlin, perlin, perlin, 1.0f);
    //RW_PerlinTexture[int2(x, y)] = float4(Billow(perlin), Billow(perlin), Billow(perlin), 1.0f);
    //RW_PerlinTexture[int2(x, y)] = float4(RidgedNoise(perlin), RidgedNoise(perlin), RidgedNoise(perlin), 1.0f);
    
    // Visualize gradients.
    //float2 grid_cell = grid_id + grid_uv;
    //float dist_g1 = DrawVector(grid_cell, bl, bl + grad_bl / 2.0f, 0.02f);
    //float dist_g2 = DrawVector(grid_cell, br, br + grad_br / 2.0f, 0.02f);
    //float dist_g3 = DrawVector(grid_cell, tl, tl + grad_tl / 2.0f, 0.02f);
    //float dist_g4 = DrawVector(grid_cell, tr, tr + grad_tr / 2.0f, 0.02f);
    
    // Show vectors.
    /*if (dist_g1 < 0.0 || dist_g2 < 0.0 || dist_g3 < 0.0 || dist_g4 < 0.0)
    {
        RW_PerlinTexture[int2(x, y)] = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }*/
}

// Is actually called Fractional Brownian Motion (FBM).
void Method2(uint x, uint y)
{
    // Setup grid.
    float2 uv = float2(x, y) / float2(width - 1, height - 1);
    uv *= tileCount;
    
    float fbm_noise = 0.0f;
    float amplitude = 1.0f;

    for (int i = 0; i < numOctaves; i++)
    {
        fbm_noise += Perlin2(uv) * amplitude;
        
        uv *= 2.0f;
        amplitude /= 2.0f;
    }
    
    // Show perlin noise.
    RW_PerlinTexture[int2(x, y)] = float4(fbm_noise, fbm_noise, fbm_noise, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////
// Method 3: Uses method 2 to create a point/grid effect.
////////////////////////////////////////////////////////////////////////////////

float Perlin2(uint x, uint y)
{
    // Setup grid.
    float2 uv = float2(x, y) / float2(width - 1, height - 1);
    uv *= tileCount;
    
    return Perlin2(uv);
}

void Method3(uint x, uint y)
{
    x *= 4.0f;
    y *= 4.0f;
    float perlin = Perlin2(x, y);
    
    // Show perlin noise.
    RW_PerlinTexture[int2(x, y)] = float4(perlin, perlin, perlin, 1.0f);
    //RW_PerlinTexture[int2(x, y)] = float4(Billow(perlin), Billow(perlin), Billow(perlin), 1.0f);
    //RW_PerlinTexture[int2(x, y)] = float4(RidgedNoise(perlin), RidgedNoise(perlin), RidgedNoise(perlin), 1.0f);
}

////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint x = DTid.x;
    uint y = DTid.y;
    
    if (x >= width || y >= height)
        return;
    
    // All values output, so color, values are between [-1, 1].
    // So if flip, abs(), the value for let's say red so => RW_PerlinTexture[int2(x, y)] = float4(abs(perlin), perlin, perlin, 1.0f);
    // We get a red color where the perlin noise is negative, so where i should be black it is red.
    Method1(x, y);
    //Method2(x, y);
    //Method3(x, y);
}
