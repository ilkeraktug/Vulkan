//ByteAddressBuffer vertices;
//ByteAddressBuffer indices;

struct Vertex
{
    float3 position;
    float2 uv;
};

struct Triangle
{
    Vertex vertices[3];
    float2 uv;
};

uint getIndex(uint index)
{
	uint64_t address = indicesBufferAddress[0];
	return vk::RawBufferLoad<uint>(address + index * 4);
}

float4 getVertex(uint index)
{
	uint64_t address = verticesBufferAddress[0];
	return vk::RawBufferLoad<float4>(address + index * 16);
}

Triangle unpackTriangle(uint index, uint stride, float attributeX, float attributeY)
{
    Triangle tri;

    const uint triIndex = index * 3;
    
    for (uint i = 0; i < 3; i++) {
    	const uint currIndex = getIndex(triIndex + i) * 2;
    	
    	float4 d0 = getVertex(currIndex + 0);
    	float4 d1 = getVertex(currIndex + 1);
    	tri.vertices[i].position = d0.xyz;
    	tri.vertices[i].uv = float2(d0.w, d1.x);
    }
    
    float3 baryCentricCoord = float3(1.0f - attributeX - attributeY, attributeX, attributeY);
    
    tri.uv = tri.vertices[0].uv * baryCentricCoord.x + tri.vertices[1].uv * baryCentricCoord.y + tri.vertices[2].uv * baryCentricCoord.z;

	return tri;
}