struct PayloadParams
{
    float3 hitColor;
};

[shader("miss")]
void main(inout PayloadParams io_Payload)
{
    io_Payload.hitColor = float3(0.0f, 0.0f, 0.2f);
}