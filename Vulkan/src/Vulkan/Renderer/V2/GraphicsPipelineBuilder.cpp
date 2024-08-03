#include "pch.h"
#include "GraphicsPipelineBuilder.h"

GraphicsPipelineBuilder::GraphicsPipelineBuilder()
{
    Reset();
}

void GraphicsPipelineBuilder::Init()
{
    CI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    CI.stageCount = 0;
    CI.pStages = nullptr;
    CI.pVertexInputState = &m_VertexInputStateData.CI;
    CI.pInputAssemblyState = &m_InputAssemblyStateData.CI;
    CI.pTessellationState = nullptr;
    CI.pViewportState = &m_ViewportState.CI;
    CI.pRasterizationState = &m_RasterizationState.CI;
    CI.pMultisampleState = &m_MultisampleState.CI;
    CI.pDepthStencilState = &m_DepthStencilState.CI;
    CI.pColorBlendState = &m_ColorBlendState.CI;
    CI.pDynamicState = &m_DynamicState.CI;
    CI.layout = VK_NULL_HANDLE;
    CI.renderPass = VK_NULL_HANDLE;
    CI.subpass = 0;
    CI.basePipelineHandle = VK_NULL_HANDLE;
    CI.basePipelineIndex = 0;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::Reset()
{
    m_ShaderStage = ShaderStageData();
    m_VertexInputStateData = VertexInputStateData();
    m_InputAssemblyStateData = InputAssemblyStateData();
    m_RasterizationState = RasterizationState();
    m_ViewportState = ViewportStateData();
    m_MultisampleState = MultisampleState();
    m_DepthStencilState = DepthStencilState();
    m_ColorBlendState = ColorBlendState();
    m_DynamicState = DynamicState();
    
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddShaderStage(const VkDevice& device, const std::string& shaderPath, VkShaderStageFlagBits stage)
{
    m_ShaderStage.Stages.emplace_back(VulkanShader::GetShaderModule(device, shaderPath, stage));

    CI.stageCount = m_ShaderStage.Stages.size(); 
    CI.pStages = m_ShaderStage.Stages.data();

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddVertexInputState(uint32_t binding, std::initializer_list<vkglTF::VertexComponent> initList)
{
    m_VertexInputStateData.BindingDescription.binding = binding;
    m_VertexInputStateData.BindingDescription.stride = sizeof(vkglTF::VertexComponent);
    m_VertexInputStateData.BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    uint32_t location = 0;
    for(auto& e : initList)
    {
        VkVertexInputAttributeDescription desc{};
        desc.location = location++;
        desc.binding = binding;
        desc.format = GetVertexFormatFromVertexComponent(e);
        desc.offset = GetVertexOffsetFromVertexComponent(e);

        m_VertexInputStateData.AttributeDescriptions.emplace_back(desc);
    }

    m_VertexInputStateData.CI.vertexAttributeDescriptionCount = m_VertexInputStateData.AttributeDescriptions.size();
    m_VertexInputStateData.CI.pVertexAttributeDescriptions = m_VertexInputStateData.AttributeDescriptions.data();
        
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddInputAssemblyState(VkPrimitiveTopology topology)
{
    m_InputAssemblyStateData.CI.topology = topology;
    m_InputAssemblyStateData.CI.primitiveRestartEnable = VK_FALSE;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddRasterizationState(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace)
{
    m_RasterizationState.CI.depthClampEnable = VK_FALSE;
    m_RasterizationState.CI.rasterizerDiscardEnable = VK_TRUE;
    m_RasterizationState.CI.polygonMode = polygonMode;
    m_RasterizationState.CI.cullMode = cullMode;
    m_RasterizationState.CI.frontFace = frontFace;
    m_RasterizationState.CI.lineWidth = 1.0f;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddMultisampleState(VkSampleCountFlagBits sampleCount)
{
    m_MultisampleState.CI.rasterizationSamples = sampleCount;
    m_MultisampleState.CI.sampleShadingEnable = VK_FALSE;
    m_MultisampleState.CI.pSampleMask = nullptr;
    m_MultisampleState.CI.alphaToCoverageEnable = VK_FALSE;
    m_MultisampleState.CI.alphaToOneEnable = VK_FALSE;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddViewportState(float width, float height)
{
    m_ViewportState.Viewport.x = 0.0f;
    m_ViewportState.Viewport.y = 0.0f;
    m_ViewportState.Viewport.width = width;
    m_ViewportState.Viewport.height = height;
    m_ViewportState.Viewport.minDepth = 0.0f;
    m_ViewportState.Viewport.maxDepth = 1.0f;

    VkOffset2D    offset = {0, 0};
    VkExtent2D    extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    m_ViewportState.Scissors = { offset, extent};

    m_ViewportState.CI.viewportCount = 1;
    m_ViewportState.CI.scissorCount = 1;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDynamicViewportState()
{
    m_ViewportState.CI.viewportCount = 1;
    m_ViewportState.CI.scissorCount = 1;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDepthStencilState(VkBool32 depthTest, VkBool32 depthWrite)
{
    m_DepthStencilState.CI.depthTestEnable = depthTest;
    m_DepthStencilState.CI.depthWriteEnable = depthWrite;
    m_DepthStencilState.CI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    m_DepthStencilState.CI.depthBoundsTestEnable = VK_FALSE;
    m_DepthStencilState.CI.stencilTestEnable = VK_FALSE;

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddColorBlendAttachment(VkBool32 enableBlend, VkColorComponentFlags writeMask)
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = enableBlend;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = writeMask;

    m_ColorBlendState.ColorBlendAttachmentStates.emplace_back(colorBlendAttachment);
        
    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDynamicPipelineState(VkDynamicState dynamicState)
{
    m_DynamicState.DynamicStates.emplace_back(dynamicState);

    m_DynamicState.CI.dynamicStateCount = m_DynamicState.DynamicStates.size();
    m_DynamicState.CI.pDynamicStates = m_DynamicState.DynamicStates.data();

    return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::AddDynamicPipelineState(const std::vector<VkDynamicState>& dynamicStates)
{
    m_DynamicState.DynamicStates = dynamicStates;
        
    m_DynamicState.CI.dynamicStateCount = static_cast<uint32_t>(m_DynamicState.DynamicStates.size());
    m_DynamicState.CI.pDynamicStates = m_DynamicState.DynamicStates.data();

    return *this;
}

VkPipeline GraphicsPipelineBuilder::Create(VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass)
{
    CI.renderPass = renderPass;
    
    VkPipeline pipeline;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &CI, nullptr, &pipeline));

    return pipeline;
}

VkFormat GraphicsPipelineBuilder::GetVertexFormatFromVertexComponent(vkglTF::VertexComponent component) const
{
    switch (component) {
        case vkglTF::VertexComponent::Position: return VK_FORMAT_R32G32B32_SFLOAT;
        case vkglTF::VertexComponent::Normal: return VK_FORMAT_R32G32B32_SFLOAT;
        case vkglTF::VertexComponent::UV: return VK_FORMAT_R32G32_SFLOAT;
        case vkglTF::VertexComponent::Color: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case vkglTF::VertexComponent::Tangent: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case vkglTF::VertexComponent::Joint0: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case vkglTF::VertexComponent::Weight0: return VK_FORMAT_R32G32B32A32_SFLOAT;
    }

    return VK_FORMAT_UNDEFINED;
}

uint32_t GraphicsPipelineBuilder::GetVertexOffsetFromVertexComponent(vkglTF::VertexComponent component) const
{
    switch (component) {
    case vkglTF::VertexComponent::Position: return offsetof(vkglTF::Vertex, pos);
    case vkglTF::VertexComponent::Normal: return offsetof(vkglTF::Vertex, normal);
    case vkglTF::VertexComponent::UV: return offsetof(vkglTF::Vertex, uv);
    case vkglTF::VertexComponent::Color: return offsetof(vkglTF::Vertex, color);
    case vkglTF::VertexComponent::Tangent: return offsetof(vkglTF::Vertex, tangent);
    case vkglTF::VertexComponent::Joint0: return offsetof(vkglTF::Vertex, joint0);
    case vkglTF::VertexComponent::Weight0: return offsetof(vkglTF::Vertex, weight0);
    }

    return 0;
}
