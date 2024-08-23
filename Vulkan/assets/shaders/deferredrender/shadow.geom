StructuredBuffer<float4> InstancePositions : register(t0);
StructuredBuffer<float4x4> LightMVPs : register(t1);

struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] uint InstanceIndex : TEXCOORD0;
};

struct GSOutput
{
    float4 Position : SV_POSITION;
    int Layer : SV_RenderTargetArrayIndex;
};

[instance(3)]
[maxvertexcount(3)]
void main(triangle VSOutput input[3], uint InvocationID : SV_GSInstanceID, inout TriangleStream<GSOutput> output)
{
    float4 instancePos = InstancePositions.Load(input[0].InstanceIndex);
    
    float4x4 LightMVP = LightMVPs.Load(InvocationID);
    
    for(int i = 0; i < 3; i++)
    {
        float4 tempPos = instancePos + InstancePositions.Load(i);
        GSOutput outputStruct = (GSOutput)0;
        outputStruct.Position = mul(tempPos, LightMVP);
        outputStruct.Layer = InvocationID;
        output.Append(outputStruct);
    }
    
    output.RestartStrip();
}