#pragma once
#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/tests/TestGraphicsPipeline.h"

class GraphicsPipelineBuilder
{
public:
    
    static GraphicsPipelineBuilder& Get()
    {
        if(s_Instance == nullptr)
        {
            s_Instance = new GraphicsPipelineBuilder();
        }

        return *s_Instance;
    }

    GraphicsPipelineBuilder();
    
    GraphicsPipelineBuilder& Reset();
    
    GraphicsPipelineBuilder& AddShaderStage(VkPipelineShaderStageCreateInfo shaderStageCI);

    GraphicsPipelineBuilder& AddShaderStage(const VkDevice& device, const std::string& shaderPath, VkShaderStageFlagBits stage);

    GraphicsPipelineBuilder& AddVertexInputState(uint32_t binding, std::initializer_list<vkglTF::VertexComponent> initList);
    
    GraphicsPipelineBuilder& AddEmptyVertexInputState();

    GraphicsPipelineBuilder& AddInputAssemblyState(VkPrimitiveTopology topology);
    
    GraphicsPipelineBuilder& AddRasterizationState(VkPolygonMode polygonMode, VkCullModeFlags cullMode = VK_CULL_MODE_NONE, VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE);

    GraphicsPipelineBuilder& AddMultisampleState(VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

    GraphicsPipelineBuilder& AddViewportState(float width, float height);
    
    GraphicsPipelineBuilder& AddDynamicViewportState();

    GraphicsPipelineBuilder& AddDepthStencilState(VkBool32 depthTest, VkBool32 depthWrite);

    GraphicsPipelineBuilder& AddEmptyColorBlendAttachment();

    GraphicsPipelineBuilder& AddColorBlendAttachment(VkBool32 enableBlend, VkColorComponentFlags writeMask);
    
    GraphicsPipelineBuilder& AddDynamicPipelineState(VkDynamicState dynamicState);

    GraphicsPipelineBuilder& AddDynamicPipelineState(const std::vector<VkDynamicState>& dynamicStates);

    VkPipeline Create(VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass);
    
private:
    void Init();

    VkFormat GetVertexFormatFromVertexComponent(vkglTF::VertexComponent component) const;
    uint32_t GetVertexOffsetFromVertexComponent(vkglTF::VertexComponent component) const;
private:

    VkGraphicsPipelineCreateInfo CI{};

    struct ShaderStageData
    {
        std::vector<VkPipelineShaderStageCreateInfo> Stages;
        
    } m_ShaderStage;

    struct VertexInputStateData
    {
        VertexInputStateData()
        {
            CI.vertexBindingDescriptionCount = 1;
            CI.pVertexBindingDescriptions = &BindingDescription;
        }
        
        VkPipelineVertexInputStateCreateInfo CI{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

        
        VkVertexInputBindingDescription BindingDescription{};
        std::vector<VkVertexInputAttributeDescription> AttributeDescriptions{};
        
    } m_VertexInputStateData;

    struct InputAssemblyStateData
    {
        VkPipelineInputAssemblyStateCreateInfo CI{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    } m_InputAssemblyStateData;

    struct RasterizationState
    {
        VkPipelineRasterizationStateCreateInfo CI{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    } m_RasterizationState;
    
    struct ViewportStateData
    {
        ViewportStateData()
        {
            CI.pViewports = &Viewport;
            CI.pScissors = &Scissors;
        }
        
        VkPipelineViewportStateCreateInfo CI{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        VkViewport Viewport{};
        VkRect2D Scissors{};
    } m_ViewportState;

    struct MultisampleState
    {
        VkPipelineMultisampleStateCreateInfo CI{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    } m_MultisampleState;

    struct DepthStencilState
    {
        VkPipelineDepthStencilStateCreateInfo CI{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    } m_DepthStencilState;

    struct ColorBlendState
    {
        ColorBlendState()
        {
            CI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            CI.logicOpEnable = VK_FALSE;
        }
        
        VkPipelineColorBlendStateCreateInfo CI{};

        std::vector<VkPipelineColorBlendAttachmentState> ColorBlendAttachmentStates;
        
    } m_ColorBlendState;

    struct DynamicState
    {
        DynamicState()
        {
            CI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        }
        
        VkPipelineDynamicStateCreateInfo CI{};
        
        std::vector<VkDynamicState> DynamicStates;
        
    } m_DynamicState;
    
    static GraphicsPipelineBuilder* s_Instance;
};
