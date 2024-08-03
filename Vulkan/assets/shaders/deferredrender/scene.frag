Texture2D PositionTexture : register(t1);
SamplerState PositionTextureSampler : register(s1);

Texture2D NormalTexture : register(t2);
SamplerState NormalTextureSampler : register(s2);

Texture2D AlbedoTexture : register(t3);
SamplerState AlbedoTextureSampler : register(s3);

Texture2DArray ShadowMapArray : register(t5);
SamplerState ShadowMapSampler : register(s5);

struct Light
{
    float4 Position;
    float4 Target;
    float4 Color;
    float4x4 MVP;
};

struct UBO
{
    float3 viewPosition;
    Light lights[3];
    int debugShadow;
};

cbuffer ubo : register(b4) 
{
    UBO ubo;
}

struct VSOutput
{
	float4 Pos : SV_POSITION;
    [[vk::location(0)]] float2 UV : TEXCOORD0;
};


float shadowScale(float3 fragPos, int layer, float2 offset)
{
    float4 fragOnClipSpace = mul(ubo.lights[layer].MVP, float4(fragPos, 1.0f));
    
    float4 fragOnNDC = fragOnClipSpace / fragOnClipSpace.w;
    float2 fragAsUV = fragOnNDC.xy * 0.5f + 0.5f;
    
    float depth = ShadowMapArray.Sample(ShadowMapSampler, float3(fragAsUV + offset, layer)).r;
    
    if(depth < fragOnNDC.z)
    {
        return 0.75f;
        return 0.25f;
    }
    
    return 1.0f;
}

float filterPCF(float3 fragPos, int layer)
{
    uint width;
    uint height;
    uint elements;
    uint numberoflevels;
    
    ShadowMapArray.GetDimensions(0, width, height, elements, numberoflevels);
    
    float dx = 1 / width;
    float dy = 1 / height;
    
    float shadowFactor = 0.0f;
    
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            shadowFactor += shadowScale(fragPos, layer, float2(dx * x, dy * y));
        }
    }
    
    return shadowFactor / 4;
}

float3 calculateColor(float3 color, float3 fragPos)
{
    if(ubo.debugShadow == 0)
    {
        for(int i = 0; i < 3; i++)
        {
            color *= filterPCF(fragPos, i);
        }
    }
    
    return color;
}

float4 main(VSOutput input) : SV_TARGET
{
    float3 fragPos = PositionTexture.Sample(PositionTextureSampler, input.UV).rgb;
    float3 normal = NormalTexture.Sample(NormalTextureSampler, input.UV).rgb;
    float4 albedo = AlbedoTexture.Sample(AlbedoTextureSampler, input.UV);
    
    float depth0 = ShadowMapArray.Sample(ShadowMapSampler, float3(input.UV, 0)).r;
    float depth1 = ShadowMapArray.Sample(ShadowMapSampler, float3(input.UV, 1)).r;
    float depth2 = ShadowMapArray.Sample(ShadowMapSampler, float3(input.UV, 2)).r;
    
    float3 N = normalize(normal);
    float3 FragPosToViewDirection = normalize(ubo.viewPosition - fragPos);
    
    float3 fragColor = float3(0.0f, 0.0f, 0.0f);
    
    for(int i = 0; i < 3; i++)
    {
        float3 LightToTargetDirection = normalize(ubo.lights[i].Target - ubo.lights[i].Position).xyz;
        float3 LightToFrag = fragPos - ubo.lights[i].Position.xyz;
        float LightToFragDistance = length(LightToFrag);
        float3 LightToFragDirection = normalize(LightToFrag);
        
        float LightInnerAngle = cos(radians(15.0f));
        float LightOuterAngle = cos(radians(90.0f));
        float Range = 100.0f;
        
        float c1 = dot(LightToFragDirection, LightToTargetDirection);
        
        float AngleEffect = smoothstep(LightOuterAngle, LightInnerAngle, c1);
        float DistanceEffect = smoothstep(Range, 0.0f, LightToFragDistance);
        
        float3 ref = reflect(-LightToFragDirection, N);
        
        float c2 = max(0.0f, dot(FragPosToViewDirection, ref));
        
        float spec = pow(c2, 16) * albedo.a * 0.25f;
        
        float colorMulp = max(0.0f, dot(LightToFragDirection, N));
        
        fragColor += ((spec + colorMulp) * AngleEffect * DistanceEffect) * ubo.lights[i].Color.rgb * albedo.rgb;
    }
    
    fragColor = calculateColor(fragColor, fragPos);
    

    return float4(fragColor, 1.0f);
    return float4(albedo * (depth0 + depth1 + depth2));
    return float4((depth0 + depth1 + depth2), 0.0f, 0.0f, 1.0f);
    //return float4(depth, 0.0f, 0.0f, 1.0f);
    return float4(normal, 1.0f);
}
