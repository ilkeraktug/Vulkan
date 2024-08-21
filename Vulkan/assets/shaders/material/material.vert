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

VSOutput main(float3 inputPos : POSITION0, uint InstanceId : SV_InstanceID)
{
    VSOutput output = (VSOutput)0;
    float4 tempPos = float4(inputPos,1.0f);// /*+ uboInstancePos.instancePos[InstanceId]*/;
    
    float4x4 projView = mul(ubo.view, ubo.projection);
    float4x4 mvp = mul(modelUbo.model, projView);
    output.outPos = mul(tempPos, mvp);
    
    return output;
}