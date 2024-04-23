
Texture2D myTexture : register(t3);
SamplerState mySampler : register(s3);

StructuredBuffer<uint64_t> verticesBufferAddress : register(t4);
StructuredBuffer<uint64_t> indicesBufferAddress : register(t5);

struct PayloadParams
{
    float3 hitColor;
};

struct hitAttributes
{
    float x;
    float y;
};

#include "../Common/Common.hlsl"

[shader("closesthit")]
void main(inout PayloadParams io_Payload, in hitAttributes attributes)
{
    uint PrimitiveID = PrimitiveIndex();
    
    Triangle tri = unpackTriangle(PrimitiveID, 32, attributes.x, attributes.y);
   
    float4 color = myTexture.SampleLevel(mySampler, tri.uv, 0);
    //io_Payload.hitColor = float3(tri.uv, 0.0f);
    io_Payload.hitColor = color.rgb;
}