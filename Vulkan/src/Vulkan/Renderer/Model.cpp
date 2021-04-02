#include "pch.h"
#include "Model.h"

Model::Model(const std::string& objPath, const std::string texturePath)
{
	loadObj(objPath);
	m_Texture = new Texture(texturePath);
}

void Model::loadObj(const std::string& objPath)
{
	std::string warn, err;

	if (!tinyobj::LoadObj(&m_Attributes, &m_Shapes, &m_Materials, &warn, &err, objPath.c_str()))
	{
		VK_WARN("{0}, {1}", warn.c_str(), err.c_str());
		VK_ASSERT(false, "");
	}

	for (const auto& face : m_Shapes)
	{
		for (const auto& index : face.mesh.indices)
		{
			Vertex vertex;
			vertex.Position = { m_Attributes.vertices[3 * index.vertex_index + 0], 
								m_Attributes.vertices[3 * index.vertex_index + 1], 
								m_Attributes.vertices[3 * index.vertex_index + 2]
			};

			vertex.Normal = {	m_Attributes.normals[3 * index.normal_index + 0],
								m_Attributes.normals[3 * index.normal_index + 1],
								m_Attributes.normals[3 * index.normal_index + 2]
			};

			vertex.TexCoords = {m_Attributes.texcoords[2 * index.texcoord_index + 0],
								1.0f - m_Attributes.texcoords[2 * index.texcoord_index + 1]
			};

			m_Vertices.push_back(vertex);
			m_Indices.push_back(m_Indices.size());
		}
	}

}
