Texture2D textureColor : register(t0);
SamplerState samplerColor : register(s0);

struct FSOutput
{
    float4 color : SV_TARGET;
};

FSOutput main([[vk::location(0)]] float2 inUV : TEXCOORD0)
{
    FSOutput output = (FSOutput)0;
    
	float2 UV = inUV;
	output.color.rgb = textureColor.Sample(samplerColor, UV).rgb;
	output.color.a = 1.0f;
	//output.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
	return output;
}