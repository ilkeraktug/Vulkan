struct UBO
{
    float4x4 view;
    float4x4 projection;
};

[[vk::binding(0, 0)]]
cbuffer ubo : register(b0)
{
    UBO ubo;
}

struct ModelUBO
{
    float4x4 model;
};

[[vk::binding(0, 1)]]
cbuffer ubo : register(b0)
{
    ModelUBO modelUbo;
}

struct VSOutput
{
    float4 outPos : SV_POSITION;
};

VSOutput main(float4 inputPos : POSITION0, uint InstanceId : SV_InstanceID)
{
    VSOutput output = (VSOutput)0;
    float4 tempPos = inputPos;
    
    float4 worldPos = mul(modelUbo.model, tempPos);
    float4 viewPos = mul(ubo.view, worldPos);
    float4 screenPos = mul(ubo.projection, viewPos);

    output.outPos = screenPos;
    
    return output;
}