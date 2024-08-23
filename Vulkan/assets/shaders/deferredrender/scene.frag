Texture2D PositionTexture : register(t0);
SamplerState PositionTextureSampler : register(s0);

Texture2D NormalTexture : register(t1);
SamplerState NormalTextureSampler : register(s1);

Texture2D AlbedoTexture : register(t2);
SamplerState AlbedoTextureSampler : register(s2);

Texture2DArray ShadowMapArray : register(t3);
SamplerState ShadowMapSampler : register(s3);

StructuredBuffer<float4x4> LightMVPs : register(t4);
StructuredBuffer<float4> LightData : register(t5);

struct CameraData
{
    float4 Position;
    float4x4 View;
    float4x4 Projection;
};

cbuffer CameraData : register(b6)
{
    CameraData cameraDataUBO;   
}

struct Light
{
    float4 Position;
    float4 Target;
    float4 Color;
};

Light getLightData(int index)
{
    Light light = (Light)0;
    
    light.Position = LightData.Load(index * 3);
    light.Target = LightData.Load(index * 3 + 1);
    light.Color = LightData.Load(index * 3 + 2);
    
    return light;
}

float4x4 getLightMVP(int index)
{
    return LightMVPs.Load(index);
}

struct VSOutput
{
	float4 Pos : SV_POSITION;
    [[vk::location(0)]] float2 UV : TEXCOORD0;
};

float shadowScale(float3 fragPos, int layer, float2 offset)
{
    float4 fragOnClipSpace = mul(getLightMVP(layer), float4(fragPos, 1.0f));
    
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
    if(true)//ubo.debugShadow == 0)
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
    float3 FragPosToViewDirection = normalize(cameraDataUBO.Position.xyz - fragPos);
    
    float3 fragColor = float3(0.0f, 0.0f, 0.0f);
    
    for(int i = 0; i < 3; i++)
    {
        Light lightData = getLightData(i);
        
        float3 LightToTargetDirection = normalize(lightData.Target - lightData.Position).xyz;
        float3 LightToFrag = fragPos - lightData.Position.xyz;
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
        
        fragColor += ((spec + colorMulp) * AngleEffect * DistanceEffect) * lightData.Color.rgb * albedo.rgb;
    }
    
    fragColor = calculateColor(fragColor, fragPos);
    

    return float4(fragColor, 1.0f);
    return float4(albedo * (depth0 + depth1 + depth2));
    return float4((depth0 + depth1 + depth2), 0.0f, 0.0f, 1.0f);
    //return float4(depth, 0.0f, 0.0f, 1.0f);
    return float4(normal, 1.0f);
}
