#pragma once
#include "tiny_obj_loader.h"

#include "VertexBuffer.h"
#include "Texture.h"

class Model
{
public:
	Model(const std::string& objPath, const std::string texturePath);
	~Model() = default;

	inline const std::vector<Vertex>& GetVertex() const { return m_Vertices; }
	inline const std::vector<uint32_t>& GetIndex() const { return m_Indices; }
	inline const Texture* GetTexture() const { return m_Texture; }
	inline Texture* GetTexture() { return m_Texture; }

private:
	void loadObj(const std::string& objPath);
private:
	Texture* m_Texture;

	tinyobj::attrib_t m_Attributes;
	std::vector<tinyobj::shape_t> m_Shapes;
	std::vector<tinyobj::material_t> m_Materials;

	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
};