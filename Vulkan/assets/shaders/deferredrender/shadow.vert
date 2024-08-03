
struct VSOutput
{
    float4 Position : SV_POSITION;
    [[vk::location(0)]] uint InstanceIndex : TEXCOORD0;
};

VSOutput main([[vk::location(0)]] float4 Position : POSITION0, uint InstanceID : SV_InstanceID)
{
    VSOutput output = (VSOutput)0;
    output.Position = Position;
    output.InstanceIndex = InstanceID;

    return output;
}
