struct Payload
{
    float3 hitValue;
};

[shader("miss")]
void main(inout Payload io_Payload)
{
    io_Payload.hitValue = float3(0.0f, 0.0f, 0.2f);
}