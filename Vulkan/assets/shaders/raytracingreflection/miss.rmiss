
struct RayPayload
{
    float3 color;
    float distance;
    float3 normal;
    float reflector;
};

[shader("miss")]
void main(inout RayPayload payload)
{
    const float3 gradientStart = float3(0.5, 0.6, 1.0);
    const float3 gradientEnd = float3(1.0, 1.0, 1.0);
    
    float3 rayDirectionWS = WorldRayDirection();
    float3 normalizedRayDirection = normalize(rayDirectionWS);
    
    float t = 0.5f * (normalizedRayDirection.y + 1.0f);
    
    float3 missColor = (1.0f - t) * gradientStart + t * gradientEnd;
    payload.color = missColor;
    payload.distance = -1.0f;
    payload.normal = float3(0, 0, 0);
    payload.reflector = 0.0f;
}