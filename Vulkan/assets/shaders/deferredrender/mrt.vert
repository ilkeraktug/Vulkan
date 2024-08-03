
struct VSInput
{
[[vk::location(0)]] float4 Position : POSITION0;
[[vk::location(1)]] float2 UV : TEXCOORD0;
[[vk::location(2)]] float3 Color : COLOR0;
[[vk::location(3)]] float3 Normal : NORMAL0;
[[vk::location(4)]] float3 Tangent : TEXCOORD1;
};


struct UBO
{
    float4x4 model;    
    float4x4 view;    
    float4x4 projection;
    float4 InstancePos[3];    
};

cbuffer UBO : register(b0)
{
    UBO ubo;
}

struct VSOutput
{
	float4 Position : SV_POSITION;
[[vk::location(0)]] float3 Normal : NORMAL0;
[[vk::location(1)]] float2 UV : TEXCOORD0;
[[vk::location(2)]] float3 Color : COLOR0;
[[vk::location(3)]] float3 WorldPos : POSITION0;
[[vk::location(4)]] float3 Tangent : TEXCOORD1;
};


VSOutput main(VSInput input, uint InstanceID : SV_InstanceID)
{
    float4 tempPos = input.Position + ubo.InstancePos[InstanceID];
    
    VSOutput output = (VSOutput)0;
    
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;
    
    output.UV = input.UV;
    
    output.Color = input.Color;
    
    float4 worldPos = mul(ubo.model, tempPos);
    float4 viewPos = mul(ubo.view, worldPos);
    float4 screenPos = mul(ubo.projection, viewPos);
    
   output.Position = screenPos;
   
   output.WorldPos = worldPos.xyz;
    
   
   
   return output;
}