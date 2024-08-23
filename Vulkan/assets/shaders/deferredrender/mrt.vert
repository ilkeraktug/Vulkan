
struct ModelData
{
    float4x4 ModelMatrix;
};

struct CameraData
{
    float4 Position;
    float4x4 View;
    float4x4 Projection;
};

cbuffer ModelData : register(b0)
{
    ModelData modelDataUBO;   
}

cbuffer CameraData : register(b1)
{
    CameraData cameraDataUBO;   
}

StructuredBuffer<float4> InstancePositions : register(t2);

struct VSInput
{
[[vk::location(0)]] float4 Position : POSITION0;
[[vk::location(1)]] float2 UV : TEXCOORD0;
[[vk::location(2)]] float3 Color : COLOR0;
[[vk::location(3)]] float3 Normal : NORMAL0;
[[vk::location(4)]] float3 Tangent : TEXCOORD1;
};

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
    float4 tempPos = input.Position + InstancePositions.Load(InstanceID);
    
    VSOutput output = (VSOutput)0;
    
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;
    
    output.UV = input.UV;
    
    output.Color = input.Color;
    
    float4 worldPos = mul(modelDataUBO.ModelMatrix, tempPos);
    float4 viewPos = mul(cameraDataUBO.View, worldPos);
    float4 screenPos = mul(cameraDataUBO.Projection, viewPos);
    
   output.Position = screenPos;
   
   output.WorldPos = worldPos.xyz;

   return output;
}