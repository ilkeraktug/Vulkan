Texture2D colorTexture : register(t3);
SamplerState colorTextureSampler : register(s3);
Texture2D normalMapTexture : register(t4);
SamplerState normalMapSampler : register(s4);

struct VSOutput
{
    float4 Position : SV_Position;
    [[vk::location(0)]] float3 Normal : NORMAL0;
    [[vk::location(1)]] float2 UV : TEXCOORD0;
    [[vk::location(2)]] float3 Color : COLOR0;
    [[vk::location(3)]] float3 WorldPos : POSITION0;
    [[vk::location(4)]] float3 Tangent : TEXCOORD1;
};

struct FSOutput
{
	float4 Position : SV_TARGET0;
	float4 Normal : SV_TARGET1;
	float4 Albedo : SV_TARGET2;
};

FSOutput main(VSOutput input)
{
    FSOutput output = (FSOutput)0;
    
    output.Position = float4(input.WorldPos, 1.0f);
    output.Albedo = colorTexture.Sample(colorTextureSampler, input.UV);
    
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Tangent);
    float3 B = normalize(cross(N, T));
    
    float3x3 TBN = float3x3(T, B, N);
    
    float4 normalMapValue = normalMapTexture.Sample(normalMapSampler, input.UV);
    float4 convertedNormalMap = normalize(normalMapValue * 2.0f - 1.0f);
    output.Normal = float4(mul(convertedNormalMap.xyz, TBN), 1.0f);
    
    return output;
}