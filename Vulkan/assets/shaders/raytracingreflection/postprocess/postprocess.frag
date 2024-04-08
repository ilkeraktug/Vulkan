Texture2D textureColor : register(t0);
SamplerState samplerColor : register(s0);

float4 main([[vk::location(0)]] float2 inUV : TEXCOORD0) : SV_TARGET
{
	float2 UV = inUV;
	float4 color = textureColor.Sample(samplerColor, UV);
	
	return color;
}