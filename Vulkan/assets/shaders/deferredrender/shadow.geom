
struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] uint InstanceIndex : TEXCOORD0;
};

struct UBO
{
    float4x4 MVP[3];
    float4 InstancePos[3];
};

cbuffer c_ubo : register(b0)
{
    UBO ubo;
}

struct GSOutput
{
    float4 Position : SV_POSITION;
    int Layer : SV_RenderTargetArrayIndex;
};

[instance(3)]
[maxvertexcount(3)]
void main(triangle VSOutput input[3], uint InvocationID : SV_GSInstanceID, inout TriangleStream<GSOutput> output)
{
    float4 instancePos = ubo.InstancePos[input[0].InstanceIndex];
    
    for(int i = 0; i < 3; i++)
    {
        float4 tempPos = instancePos + input[i].Position;
        GSOutput outputStruct = (GSOutput)0;
        outputStruct.Position = mul(tempPos, ubo.MVP[InvocationID]);
        outputStruct.Layer = InvocationID;
        output.Append(outputStruct);
    }
    
    output.RestartStrip();
}