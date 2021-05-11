#pragma once

#include <vulkan/vulkan_core.h>

enum class ShaderFormat
{
	Int = 0,
	Int2,
	Int3,
	Int4,
	Float,
	Float2,
	Float3,
	Float4,
	Mat3x3,
	Mat4x4
};

static VkFormat GetVkFormat(ShaderFormat shadarFormat)
{
	switch (shadarFormat)
	{
	case ShaderFormat::Int:		return VK_FORMAT_R32_SINT;
	case ShaderFormat::Int2:	return VK_FORMAT_R32G32_SINT;
	case ShaderFormat::Int3:	return VK_FORMAT_R32G32B32_SINT;
	case ShaderFormat::Int4:	return VK_FORMAT_R32G32B32A32_SINT;
	case ShaderFormat::Float:	return VK_FORMAT_R32_SFLOAT;
	case ShaderFormat::Float2:	return VK_FORMAT_R32G32_SFLOAT;
	case ShaderFormat::Float3:	return VK_FORMAT_R32G32B32_SFLOAT;
	case ShaderFormat::Float4:	return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
}

static uint32_t GetFormatSize(ShaderFormat shadarFormat)
{
	switch (shadarFormat)
	{
	case ShaderFormat::Int:		return 4;
	case ShaderFormat::Int2:	return 4 * 2;
	case ShaderFormat::Int3:	return 4 * 3;
	case ShaderFormat::Int4:	return 4 * 4;
	case ShaderFormat::Float:	return 4;
	case ShaderFormat::Float2:	return 4 * 2;
	case ShaderFormat::Float3:	return 4 * 3;
	case ShaderFormat::Float4:	return 4 * 4;
	case ShaderFormat::Mat3x3:	return 4 * 3 * 3;
	case ShaderFormat::Mat4x4:	return 4 * 4 * 4;
	}
}

struct Element
{
	std::string Name;
	ShaderFormat Format;
	VkFormat VkFormat;
	uint32_t Offset;

	Element() = default;
	Element(const std::string& name, ShaderFormat format)
		: Name(name), Format(format), VkFormat(GetVkFormat(format)), Offset(0) {}
};

class VertexBufferLayout
{
public:
	VertexBufferLayout() {};
	VertexBufferLayout(const std::initializer_list<Element>& elements)
		:m_Elements(elements), m_Stride(0)
	{
		CalculateOffsets();
	}

	const uint32_t GetStride() const { return m_Stride; }
	const std::vector<Element> GetElements() const { return m_Elements; }

private:
	void CalculateOffsets()
	{
		uint32_t offset = 0;

		for (auto& element : m_Elements)
		{
			element.Offset = offset;

			offset += GetFormatSize(element.Format);

			m_Stride += GetFormatSize(element.Format);
		}
	}

private:

	std::vector<Element> m_Elements;
	uint32_t m_Stride;
};