#include "pch.h"
#include "RTXTexture.h"

#include "Vulkan/Core/tools.h"
#include "Vulkan/Math/Transform.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"
#include "Vulkan/Renderer/RTXBuilder.h"
#include "Vulkan/Renderer/VulkanFunctions.h"
#include "Vulkan/Renderer/VulkanShader.h"
#include "Vulkan/Renderer/V2/VulkanBufferDeviceAddress.h"
#include "Vulkan/Renderer/V2/VulkanBufferStridedDeviceAddress.h"

namespace test
{
    RTXTexture::RTXTexture(VulkanCore* core)
    {
        Init(core);
        m_Camera = std::make_unique<PerspectiveCamera>(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
        m_Camera->setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
        runBatchFile();

        loadTexture();
        
        prepareStructureProperties();

        createBottomLevelAccelerationStructure();
        createTopLevelAccelerationStructure();

        createUniformBuffer();
        
        createRTXPipeline();
        createShaderBindingTable();
        createDescriptorSets();
        
        createPostProcessPipeline();
        
        //buildCmdBuffer();
        
        initSuccess = true;
    }

    RTXTexture::~RTXTexture()
    {
    }

    void RTXTexture::OnUpdate(float deltaTime)
    {
        m_Camera->OnUpdate(deltaTime);

        UniformData data{};
        data.InvProjection = glm::inverse(m_Camera->getProjectionMatrix());
        data.InvView = glm::inverse(m_Camera->getViewMatrix());

        uniformBuffer->copyToBuffer(&data, sizeof(data));
    }

    void RTXTexture::OnRender()
    {
        if(!initSuccess)
        {
            return;
        }
        
        VK_CHECK(vkWaitForFences(*m_Core, 1, &m_Core->sync.inFlightFence, VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(*m_Core, 1, &m_Core->sync.inFlightFence));
        
        m_Core->BeginScene();

        m_Core->resources.submitInfo.commandBufferCount = 1;
        m_Core->resources.submitInfo.pCommandBuffers = &m_Core->resources.drawCmdBuffers[m_Core->resources.imageIndex];

        buildCmdBuffer();

        VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &m_Core->resources.submitInfo, m_Core->sync.inFlightFence));

        VkResult err = m_Core->Submit();
        
        //TODO : Fences and Semaphores !
        vkDeviceWaitIdle(m_Core->GetDevice());
    }

    void RTXTexture::OnImGuiRender()
    {
    }

    void RTXTexture::windowResized()
    {
        Test::windowResized();
    }

    void RTXTexture::runBatchFile()
    {
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingtexture & for /r %i in (*.vert) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tvs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-extension=SPV_KHR_physical_storage_buffer -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.vspv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingtexture & for /r %i in (*.frag) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tps_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-extension=SPV_KHR_physical_storage_buffer -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.fspv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingtexture & for %i in (*.rchit) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-extension=SPV_KHR_physical_storage_buffer -fspv-target-env=vulkan1.2 %i -Fo %~ni.rchit.spv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingtexture & for %i in (*.rmiss) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-extension=SPV_KHR_physical_storage_buffer -fspv-target-env=vulkan1.2 %i -Fo %~ni.rmiss.spv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingtexture & for %i in (*.rgen) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain   -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing  -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-extension=SPV_KHR_physical_storage_buffer -fspv-target-env=vulkan1.2 %i -Fo %~ni.rgen.spv)");
        std::system( "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\raytracingtexture & for %i in (*.rahit) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain   -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing  -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-extension=SPV_KHR_physical_storage_buffer -fspv-target-env=vulkan1.2 %i -Fo %~ni.rahit.spv)");
    }

    void RTXTexture::prepareStructureProperties()
    {
        m_RayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

        VkPhysicalDeviceProperties2 physicalDeviceProperties2{}; 
        physicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        physicalDeviceProperties2.pNext = &m_RayTracingPipelineProperties;
    
        vkGetPhysicalDeviceProperties2(m_Core->GetPhysicalDevice(), &physicalDeviceProperties2);
    }

    void RTXTexture::createBottomLevelAccelerationStructure()
    {
        struct Vertex
        {
            float position[3];
            float uv[2];
            float normal[3];
        };

        Vertex vertices[4]=
        {
            { -1.0f, 0.0f, 0.0f , 0.0f, 0.0f, 0.0f, 0.0f, -1.0f},
            { 1.0f, 0.0f, 0.0f , 1.0f, 0.0f, 0.0f, 0.0f, -1.0f},
            { 1.0f, 1.0f, 0.0f , 1.0f, 1.0f, 0.0f, 0.0f, -1.0f},
            { -1.0f, 1.0f, 0.0f , 0.0f, 1.0f, 0.0f, 0.0f, -1.0f}
        };
        
        uint32_t indices[6] =
        {
            0, 1, 2,
            2, 3, 0
        };
        
        m_VertexBuffer = std::make_unique<V2::VulkanBufferDeviceAddress>(m_Core, sizeof(vertices),
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertices);

        m_IndexBuffer = std::make_unique<V2::VulkanBufferDeviceAddress>(m_Core, sizeof(indices),
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indices);

        m_SingleSquareBLAS = m_Core->m_RTXBuilder->buildBLAS(
            m_VertexBuffer.get(), sizeof(Vertex), sizeof(vertices) / sizeof(vertices[0]),
            m_IndexBuffer.get(), sizeof(indices) / sizeof(indices[0]), nullptr);

        m_sceneBLAS = m_Core->m_RTXBuilder->buildBLAS(m_Scene);
    }

    void RTXTexture::createTopLevelAccelerationStructure()
    {
        VkAccelerationStructureInstanceKHR singleSquareInstance{};
        singleSquareInstance.transform = Transform::Identity.GetAsVkTransformMatrixKHR();
        singleSquareInstance.instanceCustomIndex = 0;
        singleSquareInstance.mask = 0xff;
        singleSquareInstance.instanceShaderBindingTableRecordOffset = 0;
        singleSquareInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        singleSquareInstance.accelerationStructureReference = m_SingleSquareBLAS->GetDeviceAddress();

        VkAccelerationStructureInstanceKHR sceneInstance{};
        sceneInstance.transform = Transform({0.0f, 0.0f, 5.0f,}).GetAsVkTransformMatrixKHR();
        sceneInstance.instanceCustomIndex = 1;
        sceneInstance.mask = 0xff;
        sceneInstance.instanceShaderBindingTableRecordOffset = 0;
        sceneInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        sceneInstance.accelerationStructureReference = m_sceneBLAS->GetDeviceAddress();
        
        m_tlAS = m_Core->m_RTXBuilder->buildTLAS({singleSquareInstance/*, sceneInstance*/});
    }

    void RTXTexture::loadTexture()
    {
        m_Texture.loadFromFile("C:/dev/Vulkan/Vulkan/assets/textures/gratefloor_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.GraphicsQueue);

        vkglTF::memoryPropertyFlags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::FlipY | vkglTF::FileLoadingFlags::PreTransformVertices |
            vkglTF::FileLoadingFlags::PreMultiplyVertexColors;

        m_Scene.loadFromFile("C:/dev/Vulkan/Vulkan/assets/models/vulkanscene_shadow.gltf", m_Core,
                           m_Core->queue.TransferQueue, glTFLoadingFlags);
        
        VkCommandBuffer cmdBuffer = m_Core->createComputeCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
        vkTool::setImageLayout(
            cmdBuffer,
            m_Core->resources.RTXOutputImage->GetImage(),
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL);
        m_Core->flushComputeCommandBuffer(cmdBuffer, m_Core->queue.ComputeQueue);
    }

    void RTXTexture::createUniformBuffer()
    {
        uniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(UniformData));
    }

    void RTXTexture::createRTXPipeline()
    {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        shaderStages.reserve(4);
        
        shaderGroups.reserve(shaderStages.size());
        std::string basePath = "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingtexture/";
        {
            std::string raygenShaderPath = basePath + "raygen.rgen.spv";
            VkPipelineShaderStageCreateInfo raygenShader = VulkanShader::GetShaderModule(m_Core->GetDevice(), raygenShaderPath, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
            shaderStages.emplace_back(std::move(raygenShader));

            VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
            shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroupCI.generalShader = 0;
            shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
            
            shaderGroups.emplace_back(std::move(shaderGroupCI));
        }

        {
            std::string closestHitShaderPath = basePath + "closesthit.rchit.spv";
            VkPipelineShaderStageCreateInfo closestHitShader = VulkanShader::GetShaderModule(m_Core->GetDevice(), closestHitShaderPath, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
            shaderStages.emplace_back(std::move(closestHitShader));
            std::string anyHitShaderShaderPath = basePath + "anyhit.rahit.spv";
            VkPipelineShaderStageCreateInfo anyHitShader = VulkanShader::GetShaderModule(m_Core->GetDevice(), anyHitShaderShaderPath, VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
            shaderStages.emplace_back(std::move(anyHitShader));

            VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
            shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            shaderGroupCI.generalShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.closestHitShader = 1;
            shaderGroupCI.anyHitShader = 2;
            shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
            
            shaderGroups.emplace_back(std::move(shaderGroupCI));
        }
        {
            std::string missShaderShaderPath = basePath + "miss.rmiss.spv";
            VkPipelineShaderStageCreateInfo missHitShader = VulkanShader::GetShaderModule(m_Core->GetDevice(), missShaderShaderPath, VK_SHADER_STAGE_MISS_BIT_KHR);
            shaderStages.emplace_back(std::move(missHitShader));

            VkRayTracingShaderGroupCreateInfoKHR shaderGroupCI{};
            shaderGroupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            shaderGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            shaderGroupCI.generalShader = 3;
            shaderGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
            shaderGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
            
            shaderGroups.emplace_back(std::move(shaderGroupCI));
        }

        VkDescriptorSetLayoutBinding binding0{};
        binding0.binding = 0;
        binding0.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        binding0.descriptorCount = 1;
        binding0.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

        VkDescriptorSetLayoutBinding binding1{};
        binding1.binding = 1;
        binding1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        binding1.descriptorCount = 1;
        binding1.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

        VkDescriptorSetLayoutBinding binding2{};
        binding2.binding = 2;
        binding2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding2.descriptorCount = 1;
        binding2.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        
        VkDescriptorSetLayoutBinding binding3{};
        binding3.binding = 3;
        binding3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding3.descriptorCount = 1;
        binding3.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

        VkDescriptorSetLayoutBinding binding4{};
        binding4.binding = 4;
        binding4.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding4.descriptorCount = 1;
        binding4.stageFlags = VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

        VkDescriptorSetLayoutBinding binding5{};
        binding5.binding = 5;
        binding5.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding5.descriptorCount = 1;
        binding5.stageFlags = VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings
        {
            binding0, binding1, binding2, binding3, binding4, binding5
        };
        
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
        descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCI.bindingCount = descriptorSetLayoutBindings.size();
        descriptorSetLayoutCI.pBindings = descriptorSetLayoutBindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &m_RTXDescriptorSetLayout));
        
        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &m_RTXDescriptorSetLayout;
        pipelineLayoutCI.pushConstantRangeCount = 0;

        VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_RTXPipelineLayout));
        
        VkRayTracingPipelineCreateInfoKHR pipelineCI{};
        pipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipelineCI.stageCount = shaderStages.size();
        pipelineCI.pStages = shaderStages.data();
        pipelineCI.groupCount = shaderGroups.size();
        pipelineCI.pGroups = shaderGroups.data();
        pipelineCI.maxPipelineRayRecursionDepth = 2;
        pipelineCI.layout = m_RTXPipelineLayout;
        
        VK_CHECK(vkPFN::vkCreateRayTracingPipelinesKHR(m_Core->GetDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &m_RTXPipeline));
    }

    void RTXTexture::createPostProcessPipeline()
    {
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = { init::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1) };

    VkDescriptorSetLayoutCreateInfo descriptorLayoutCI = init::descriptorSetLayoutCreateInfo();
    descriptorLayoutCI.bindingCount = descriptorSetLayoutBindings.size();
    descriptorLayoutCI.pBindings = descriptorSetLayoutBindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorLayoutCI, nullptr, &m_PostProcess.DescriptorSetLayout));

    VkPipelineLayoutCreateInfo pipelineLayoutCI = init::pipelineLayoutCreateInfo();
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts = &m_PostProcess.DescriptorSetLayout;

    VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PostProcess.PipelineLayout));

    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStages =
    {
        VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingtexture/postprocess/postprocess.vspv", VK_SHADER_STAGE_VERTEX_BIT),    
        VulkanShader::GetShaderModule(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/raytracingtexture/postprocess/postprocess.fspv", VK_SHADER_STAGE_FRAGMENT_BIT)    
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCI = init::pipelineVertexInputState();
    vertexInputStateCI.vertexBindingDescriptionCount = 0;
    vertexInputStateCI.pVertexBindingDescriptions = nullptr;
    vertexInputStateCI.vertexAttributeDescriptionCount = 0;
    vertexInputStateCI.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = init::pipelineInputAssemblyState();
    inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = m_Core->swapchain.extent.width;
    viewport.height = m_Core->swapchain.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = viewport.width;
    scissor.extent.height = viewport.height;
    
    VkPipelineViewportStateCreateInfo viewportStateCI = init::pipelineViewportState();
    viewportStateCI.viewportCount = 1;
    viewportStateCI.pViewports = &viewport;
    viewportStateCI.scissorCount = 1;
    viewportStateCI.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCI = init::pipelineRasterizationState();
    rasterizationStateCI.depthClampEnable = VK_FALSE;
    rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCI.depthBiasEnable = VK_FALSE;
    rasterizationStateCI.depthBiasConstantFactor = 0.0f;
    rasterizationStateCI.depthBiasClamp = 0.0f;
    rasterizationStateCI.depthBiasSlopeFactor = 0.0f;
    rasterizationStateCI.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleStateCI = init::multiSampleState();
    multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCI.sampleShadingEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = init::pipelineDepthStencilState();
    depthStencilStateCI.depthTestEnable = VK_FALSE;
    depthStencilStateCI.depthWriteEnable = VK_FALSE;
    depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCI.stencilTestEnable = VK_FALSE;
    depthStencilStateCI.minDepthBounds = 0.0f;
    depthStencilStateCI.maxDepthBounds = 1.0f;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    VkPipelineColorBlendStateCreateInfo colorBlendStateCI = init::pipelineColorBlendState();
    colorBlendStateCI.logicOpEnable = VK_FALSE;
    colorBlendStateCI.attachmentCount = 1;
    colorBlendStateCI.pAttachments = &colorBlendAttachment;

    VkPipelineDynamicStateCreateInfo dynamicStateCI = init::dynamicState();
    dynamicStateCI.dynamicStateCount = 0;
    dynamicStateCI.pDynamicStates = nullptr;
    
    VkGraphicsPipelineCreateInfo graphicsPipelineCI = init::graphicsPipelineCreateInfo();
    graphicsPipelineCI.stageCount = pipelineShaderStages.size();
    graphicsPipelineCI.pStages = pipelineShaderStages.data();
    graphicsPipelineCI.pVertexInputState = &vertexInputStateCI;
    graphicsPipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    graphicsPipelineCI.pTessellationState = nullptr;
    graphicsPipelineCI.pViewportState = &viewportStateCI;
    graphicsPipelineCI.pRasterizationState = &rasterizationStateCI;
    graphicsPipelineCI.pMultisampleState = &multisampleStateCI;
    graphicsPipelineCI.pDepthStencilState = &depthStencilStateCI;
    graphicsPipelineCI.pColorBlendState = &colorBlendStateCI;
    graphicsPipelineCI.pDynamicState = &dynamicStateCI;
    graphicsPipelineCI.layout = m_PostProcess.PipelineLayout;
    graphicsPipelineCI.renderPass = m_Core->resources.renderPass;
    graphicsPipelineCI.subpass = 0;

    VK_CHECK(vkCreateGraphicsPipelines(m_Core->GetDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineCI, nullptr, &m_PostProcess.Pipeline));

    std::vector<VkDescriptorPoolSize> poolSizes =
        {
            init::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        };
    
    VkDescriptorPoolCreateInfo descriptorPoolCI = init::descriptorPoolCreateInfo();
    descriptorPoolCI.maxSets = poolSizes.size();
    descriptorPoolCI.poolSizeCount = poolSizes.size();
    descriptorPoolCI.pPoolSizes = poolSizes.data();

    VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_PostProcess.DescriptorPool));

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = init::descriptorSetAllocateInfo();
    descriptorSetAllocInfo.descriptorPool = m_PostProcess.DescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &m_PostProcess.DescriptorSetLayout;
    
    VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAllocInfo, &m_PostProcess.DescriptorSet));

    VkDescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo.sampler = m_Core->resources.RTXOutputImage->GetSampler();
    descriptorImageInfo.imageView = m_Core->resources.RTXOutputImage->GetImageView();
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    
    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = m_PostProcess.DescriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(m_Core->GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
    }

    void RTXTexture::createShaderBindingTable()
    {
        uint32_t shaderGroupHandleSize = m_RayTracingPipelineProperties.shaderGroupHandleSize;
        uint32_t shaderGroupHandleAligment = m_RayTracingPipelineProperties.shaderGroupHandleAlignment;
        
        uint32_t shaderGroupAlignedSize = (shaderGroupHandleSize + shaderGroupHandleAligment - 1) & ~(shaderGroupHandleAligment - 1);

        uint32_t shaderGroupDataSize = shaderGroupAlignedSize * shaderGroups.size();
        
        std::vector<uint8_t> shaderData(shaderGroupDataSize);
        //shaderData.reserve(shaderGroupDataSize);
        vkPFN::vkGetRayTracingShaderGroupHandlesKHR(m_Core->GetDevice(), m_RTXPipeline, 0, shaderGroups.size(), shaderGroupDataSize, shaderData.data());

        raygenStridedBuffer = std::make_unique<V2::VulkanBufferStridedDeviceAddress>(m_Core,
             shaderGroupAlignedSize, shaderGroupAlignedSize,
             VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, shaderData.data() + shaderGroupAlignedSize * 0);
        
        // anyHitStridedBuffer = std::make_unique<V2::VulkanBufferStridedDeviceAddress>(m_Core,
        //     shaderGroupAlignedSize, shaderGroupAlignedSize,
        //     VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
        //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, shaderData.data() + shaderGroupAlignedSize * 2);
        
        closestHitStridedBuffer = std::make_unique<V2::VulkanBufferStridedDeviceAddress>(m_Core,
            shaderGroupAlignedSize, shaderGroupAlignedSize,
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, shaderData.data() + shaderGroupAlignedSize * 1);
        
        missStridedBuffer = std::make_unique<V2::VulkanBufferStridedDeviceAddress>(m_Core,
            shaderGroupAlignedSize, shaderGroupAlignedSize,
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, shaderData.data() + shaderGroupAlignedSize * 2);

    }

    void RTXTexture::createDescriptorSets()
    {
        std::vector<VkDescriptorPoolSize> descriptorPoolSizes
        {
            {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}
        };
        
        VkDescriptorPoolCreateInfo descriptorPoolCI{};
        descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCI.maxSets = descriptorPoolSizes.size();
        descriptorPoolCI.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCI.pPoolSizes = descriptorPoolSizes.data();

        VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_RTXDescriptorPool));
        
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = m_RTXDescriptorPool;
        descriptorSetAllocInfo.descriptorSetCount = 1;
        descriptorSetAllocInfo.pSetLayouts = &m_RTXDescriptorSetLayout;
        
        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &descriptorSetAllocInfo, &m_RTXDescriptorSet));

        VkWriteDescriptorSetAccelerationStructureKHR tlASDescriptorSetWrite{};
        tlASDescriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        tlASDescriptorSetWrite.accelerationStructureCount = 1;
        tlASDescriptorSetWrite.pAccelerationStructures = &m_tlAS->GetHandle();
        
        VkWriteDescriptorSet binding0{};
        binding0.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        binding0.pNext = &tlASDescriptorSetWrite;
        binding0.dstSet = m_RTXDescriptorSet;
        binding0.dstBinding = 0;
        binding0.descriptorCount = 1;
        binding0.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        
        VkWriteDescriptorSet binding1{};
        binding1.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        binding1.dstSet = m_RTXDescriptorSet;
        binding1.dstBinding = 1;
        binding1.descriptorCount = 1;
        binding1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        binding1.pImageInfo = &m_Core->resources.RTXOutputImage->GetDescriptorInfo();
        
        VkWriteDescriptorSet binding2{};
        binding2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        binding2.dstSet = m_RTXDescriptorSet;
        binding2.dstBinding = 2;
        binding2.descriptorCount = 1;
        binding2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding2.pBufferInfo = &uniformBuffer->GetDescriptorInfo();

        VkWriteDescriptorSet binding3{};
        binding3.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        binding3.dstSet = m_RTXDescriptorSet;
        binding3.dstBinding = 3;
        binding3.descriptorCount = 1;
        binding3.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding3.pImageInfo = &m_Texture.descriptor;

        VkDeviceSize vertexBufferDeviceAddress = m_VertexBuffer->GetDeviceAddress();
        VkDeviceSize indexBufferDeviceAddress = m_IndexBuffer->GetDeviceAddress();
        
        m_VertexBufferDeviceAddressBuffer = std::make_unique<V2::VulkanBuffer2>(m_Core, sizeof(VkDeviceSize), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBufferDeviceAddress);
        m_IndexBufferDeviceAddressBuffer = std::make_unique<V2::VulkanBuffer2>(m_Core, sizeof(VkDeviceSize), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexBufferDeviceAddress);
        
        // VkDescriptorBufferInfo vertexBufferDeviceAddressBufferInfo{};
        // vertexBufferDeviceAddressBufferInfo.buffer = m_VertexBufferDeviceAddressBuffer->GetHandle();
        // vertexBufferDeviceAddressBufferInfo.offset = 0;
        // vertexBufferDeviceAddressBufferInfo.range = VK_WHOLE_SIZE;
        //
        // VkDescriptorBufferInfo indexBufferDeviceAddressBufferInfo{};
        // indexBufferDeviceAddressBufferInfo.buffer = m_IndexBufferDeviceAddressBuffer->GetHandle();
        // indexBufferDeviceAddressBufferInfo.offset = 0;
        // indexBufferDeviceAddressBufferInfo.range = VK_WHOLE_SIZE;
        
        VkWriteDescriptorSet binding4{};
        binding4.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        binding4.dstSet = m_RTXDescriptorSet;
        binding4.dstBinding = 4;
        binding4.descriptorCount = 1;
        binding4.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding4.pBufferInfo = &m_VertexBufferDeviceAddressBuffer->GetDescriptorInfo();

        VkWriteDescriptorSet binding5{};
        binding5.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        binding5.dstSet = m_RTXDescriptorSet;
        binding5.dstBinding = 5;
        binding5.descriptorCount = 1;
        binding5.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding5.pBufferInfo = &m_IndexBufferDeviceAddressBuffer->GetDescriptorInfo();
        
        std::vector<VkWriteDescriptorSet> writeDescriptorSets =
        {
            binding0, binding1, binding2, binding3, binding4, binding5
        };

        vkUpdateDescriptorSets(m_Core->GetDevice(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
    }

    void RTXTexture::buildCmdBuffer()
    {

        VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        VkImageSubresourceLayers subresourceLayer = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        uint32_t width = m_Core->swapchain.extent.width;
        uint32_t height = m_Core->swapchain.extent.height;

        VkClearValue clearValue[2];
        clearValue[0].color = { 0.0f };
        clearValue[1].depthStencil = { 1.0f, 0 };
        
        VkRenderPassBeginInfo renderpassBeginInfo = init::renderPassBeginInfo();
        renderpassBeginInfo.renderPass = m_Core->resources.renderPass;
        renderpassBeginInfo.renderArea.offset.x = 0;
        renderpassBeginInfo.renderArea.offset.y = 0;
        renderpassBeginInfo.renderArea.extent.width = width;
        renderpassBeginInfo.renderArea.extent.height = height;
        renderpassBeginInfo.clearValueCount = 2;
        renderpassBeginInfo.pClearValues = clearValue;
        
        const auto& cmdBuffers = m_Core->resources.drawCmdBuffers;
        for(int i = 0; i < cmdBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo cmdBufferBeginInfo = init::cmdBufferBeginInfo();

            VkCommandBuffer currentCmdBuffer = cmdBuffers[i];

            VK_CHECK(vkBeginCommandBuffer(currentCmdBuffer, &cmdBufferBeginInfo));

            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_RTXPipeline);
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_RTXPipelineLayout, 0, 1, &m_RTXDescriptorSet, 0, nullptr);

            VkStridedDeviceAddressRegionKHR dummyStridedDeviceAddress{};
            
            vkPFN::vkCmdTraceRaysKHR(currentCmdBuffer,
                &raygenStridedBuffer->GetStridedDeviceAddress(), &missStridedBuffer->GetStridedDeviceAddress(), &closestHitStridedBuffer->GetStridedDeviceAddress(),
                &dummyStridedDeviceAddress,
                m_Core->swapchain.extent.width,
                m_Core->swapchain.extent.height,
                1);

            renderpassBeginInfo.framebuffer = m_Core->resources.frameBuffers[i];
            vkCmdBeginRenderPass(currentCmdBuffer, &renderpassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            
            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PostProcess.Pipeline);
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PostProcess.PipelineLayout, 0, 1, &m_PostProcess.DescriptorSet, 0, nullptr);
            vkCmdDraw(currentCmdBuffer, 3, 1, 0, 0);
            
            vkCmdEndRenderPass(currentCmdBuffer);
            VK_CHECK(vkEndCommandBuffer(currentCmdBuffer));
        }
    }
}
