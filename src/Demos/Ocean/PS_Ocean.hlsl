// PS_Ocean.hlsl

// Constanten (Zorg dat deze matchen met je wens, Deep/Shallow)
static const float3 DEEP_COLOR = float3(0.0f, 0.1f, 0.4f);
static const float3 SHALLOW_COLOR = float3(0.4f, 0.6f, 0.9f); // Iets lichter gemaakt voor contrast

// Input struct moet EXACT matchen met VS_OUTPUT
struct PSInput
{
    float4 inPos : SV_POSITION;
    float3 worldPos : TEXCOORD0; // De berekende positie op het water
    float2 texUV : TEXCOORD1; // De coordinaten gebruikt voor de noise
};

float4 main(PSInput input) : SV_TARGET
{
    // Omdat we geen height meer direct doorgeven in de struct (maar wel via UV kunnen samplen
    // of via de worldPos.y kunnen afleiden), moeten we even kijken wat we willen.
    
    // Optie A: Gebruik de Y-hoogte van de vertex (golftop vs dal)
    // De Vertex Shader heeft Pw.y += height * scale gedaan.
    // Dus input.worldPos.y bevat de golfhoogte.
    
    // We normaliseren dit een beetje voor de kleur.
    // Stel max golfhoogte is 5.0f.
    float heightRatio = input.worldPos.y / 5.0f;
    
    // Zorg dat we niet buiten [0, 1] gaan (dit fixt het 'fel wit' worden)
    heightRatio = saturate(heightRatio);

    // Lerp tussen diep en ondiep op basis van hoogte
    float3 waterColor = lerp(DEEP_COLOR, SHALLOW_COLOR, heightRatio);

    return float4(waterColor, 1.0f);
    
    // DEBUG OPTIE (Als het nog steeds wit/zwart is, uncomment dit):
    // return float4(frac(input.worldPos.x), frac(input.worldPos.z), 0, 1); // Grid test
}