RaytracingAccelerationStructure rtxAS : register(t0);


cbuffer u_CamProps : register(b2)
{
    float4x4 inverseView;
    float4x4 inverseProj;
    float4 lightPos;
};

StructuredBuffer<float4> vertices : register(t3); 
StructuredBuffer<uint> indices : register(t4); 

struct Payload
{
    float3 hitValue;
};

struct PayloadShadow
{
    bool shadowed;
};

struct Fix
{
    float x;
    float y;
};

struct Vertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float4 color;
    float4 _pad0; 
};

Vertex unpack(int index)
{
    int vertexStride = 96 / 16;
    
    float4 d0 = vertices[index * vertexStride + 0];
    float4 d1 = vertices[index * vertexStride + 1];
    float4 d2 = vertices[index * vertexStride + 2];
    
    Vertex v;
    v.position = d0.xyz;
    v.normal = float3(d0.w, d1.x, d1.y);
    //v.uv = float2(d1.z, d1.w);  
    v.color = float4(d2.xyz, 1.0f);  
    
    return v;
}

[shader("closesthit")]
void main(inout Payload io_Payload, in Fix attributes)
{
    uint PrimitiveID = PrimitiveIndex();
    float3 hitLocation = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    
    Vertex v0 = unpack(indices[3 * PrimitiveID + 0]);
    Vertex v1 = unpack(indices[3 * PrimitiveID + 1]);
    Vertex v2 = unpack(indices[3 * PrimitiveID + 2]);
    
    float3 barycentricCoord = float3(1 - attributes.x - attributes.y, attributes.x, attributes.y);
    float3 normal = normalize(v0.normal * barycentricCoord.x + v1.normal * barycentricCoord.y + v2.normal * barycentricCoord.z);

    float3 normalizedLightPos = normalize(float3(lightPos.xyz));
    float lightDot = max(dot(normalizedLightPos, normal), 0.2);
    //io_Payload.hitValue =   v0.position; //* lightDot;
    io_Payload.hitValue = v0.color.rgb * lightDot;

    RayDesc rayDesc;
    rayDesc.Origin = hitLocation;
    rayDesc.TMin = 0.1f;
    rayDesc.TMax = 100000.0f;
    rayDesc.Direction = normalize(lightPos - float4(hitLocation, 1.0f)).xyz;

    PayloadShadow o_PlayloadShadow;
    o_PlayloadShadow.shadowed = true;
    
    TraceRay(rtxAS, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xff, 0, 0, 1, rayDesc, o_PlayloadShadow);
    
    if(o_PlayloadShadow.shadowed)
    {
        io_Payload.hitValue *= 0.3f;
    }            
}