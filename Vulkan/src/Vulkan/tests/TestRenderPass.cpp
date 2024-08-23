#include "pch.h"
#include "TestRenderPass.h"

#include "Vulkan/Core/Input.h"
#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"
#include "Vulkan/Renderer/V2/DescriptorSetBuilder.h"
#include "Vulkan/Renderer/V2/FullScreenRenderer.h"
#include "Vulkan/Renderer/V2/GraphicsPipelineBuilder.h"
#include "Vulkan/Renderer/V2/MaterialIDRenderer.h"
#include "Vulkan/Renderer/V2/VulkanFrameBuffer.h"
#include "Vulkan/Renderer/V2/VulkanUniformBuffer2.h"

namespace test
{
    TestRenderPass::TestRenderPass(VulkanCore* core)
    {
        Init(core);
        
        m_Camera = std::make_unique<PerspectiveCamera>(m_Core->swapchain.extent.width, m_Core->swapchain.extent.height, core);
        // m_Camera->setPosition(glm::vec3(-5.0f, 0.0f, -5.0f));
        m_Camera->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        
        m_MaterialIDRenderer = std::make_unique<MaterialIDRenderer>(m_Core, m_Camera.get());
        m_FullScreenRenderer = std::make_unique<FullScreenRenderer>(m_Core);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = m_MaterialIDRenderer->GetFrameBuffer()->GetSampler();
        imageInfo.imageView = m_MaterialIDRenderer->GetFrameBuffer()->GetImageView(0);
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        m_FullScreenRenderer->displayTexture(imageInfo);

        runBatchFile();
        loadassets();
        deferredSetup();
        shadowSetup();
        prepareUniformBuffers();
        prepareDescriptorSetLayout();
        preparePipeline();
        buildDeferredCommandBuffers();
        
        init = true;
    }

    TestRenderPass::~TestRenderPass()
    {
    }

    void TestRenderPass::OnUpdate(float deltaTime)
    {
        if(!init)
        {
            return;
        }
        
        m_Camera->OnUpdate(deltaTime);
        m_MaterialIDRenderer->OnUpdate(deltaTime);

        timer += timerSpeed * deltaTime;
        if (timer > 1.0f)
        {
            timer -= 1.0f;
        }

        if(Input::IsKeyPressed(GLFW_KEY_Y) && !bPressed)
        {
            bVisTexture = !bVisTexture;
            bPressed = !bPressed;
        }

        if(bPressed && Input::IsKeyReleased(GLFW_KEY_Y))
        {
            bPressed = !bPressed;
        }
        
        
        updateUniformBuffers();
    }

    void TestRenderPass::OnRender()
    {
        if (!init)
        {
            return;
        }

        m_Core->BeginScene();

        m_Core->resources.submitInfo.commandBufferCount = 1;
        m_Core->resources.submitInfo.pCommandBuffers = &m_Core->resources.drawCmdBuffers[m_Core->resources.imageIndex];

        buildDeferredCommandBuffers();

        VK_CHECK(vkQueueSubmit(m_Core->queue.GraphicsQueue, 1, &m_Core->resources.submitInfo, VK_NULL_HANDLE));

        VkResult err = m_Core->Submit();

        //TODO : Fences and Semaphores !
        vkDeviceWaitIdle(m_Core->GetDevice());
    }

    void TestRenderPass::OnImGuiRender()
    {
    }

    void TestRenderPass::runBatchFile()
    {
        std::system(
        "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for /r %i in (*.vert) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tvs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.vspv)");
        std::system(
            "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for /r %i in (*.frag) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tps_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.fspv)");
        std::system(
        "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for /r %i in (*.geom) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tgs_6_1 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo C:%~pi%~ni.geom.spv)");
        std::system(
            "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for %i in (*.rchit) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rchit.spv)");
        std::system(
            "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for %i in (*.rmiss) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain  -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rmiss.spv)");
        std::system(
            "@echo off & cd C:\\dev\\Vulkan\\Vulkan\\assets\\shaders\\deferredrender & for %i in (*.rgen) do (%VULKAN_SDK%/Bin/dxc.exe -spirv -Tlib_6_3 -Emain   -fspv-extension=SPV_KHR_ray_query -fspv-extension=SPV_KHR_ray_tracing  -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fspv-extension=SPV_EXT_descriptor_indexing -fspv-target-env=vulkan1.2 %i -Fo %~ni.rgen.spv)");
    }

    void TestRenderPass::loadassets()
    {
        std::string AssetsPath = "C:\\dev\\Vulkan\\Vulkan\\assets\\";
        const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;

        models.model = std::make_shared<vkglTF::Model>();
        models.background = std::make_shared<vkglTF::Model>();
        
        models.model->loadFromFile(AssetsPath + "models/armor/armor.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);
        models.background->loadFromFile(AssetsPath + "models/deferred_box.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);
        textures.model.colorMap.loadFromFile(AssetsPath+ "models/armor/colormap_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.model.normalMap.loadFromFile(AssetsPath + "models/armor/normalmap_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.background.colorMap.loadFromFile(AssetsPath + "textures/stonefloor02_color_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.background.normalMap.loadFromFile(AssetsPath + "textures/stonefloor02_normal_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);

        if(m_MaterialIDRenderer)
        {
            m_MaterialIDRenderer->AddModel(models.background);
            m_MaterialIDRenderer->AddModel(models.model);
        }
    }

    void TestRenderPass::deferredSetup()
    {
        scene.FrameBuffer = std::make_unique<VulkanFrameBuffer>(m_Core);
        scene.FrameBuffer->AddAttachment(scene.ColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, scene.ColorExtent.width, scene.ColorExtent.height, 1);
        scene.FrameBuffer->AddAttachment(scene.ColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, scene.ColorExtent.width, scene.ColorExtent.height, 1);
        scene.FrameBuffer->AddAttachment(scene.ColorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, scene.ColorExtent.width, scene.ColorExtent.height, 1);
        scene.FrameBuffer->AddAttachment(scene.DepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, scene.ColorExtent.width, scene.ColorExtent.height, 1);
        scene.FrameBuffer->Create();
    }

    void TestRenderPass::shadowSetup()
    {
        shadow.FrameBuffer = std::make_unique<VulkanFrameBuffer>(m_Core);
        shadow.FrameBuffer->AddAttachment(shadow.Format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, shadow.Extent.width, shadow.Extent.height, 3);
        shadow.FrameBuffer->Create();
    }

    void TestRenderPass::prepareDescriptorSetLayout()
    {
#if 0
        // DescriptorSetBuilder::Get().Begin(m_Core->GetDevice()).
        // AddDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT).
        // AddDescriptorLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        // AddDescriptorLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        // AddDescriptorLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        // AddDescriptorLayoutBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT).
        // AddDescriptorLayoutBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        // CreateDescriptorLayout(m_DescriptorSetLayout).
        // AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10).
        // AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10).
        // CreateDescriptorPool().
        // AllocateDescriptorSet(m_DescriptorSet).
        // AddImageInfo(1, scene.FrameBuffer->GetImageView(0), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, scene.FrameBuffer->GetSampler()).
        // AddImageInfo(2, scene.FrameBuffer->GetImageView(1), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, scene.FrameBuffer->GetSampler()).
        // AddImageInfo(3, scene.FrameBuffer->GetImageView(2), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, scene.FrameBuffer->GetSampler()).
        // AddBufferInfo(4, sceneUniformBuffer->GetHandle()).
        // AddImageInfo(5, shadow.FrameBuffer->GetImageView(0), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, shadow.FrameBuffer->GetSampler()).
        // UpdateDescriptorSet().
        // AllocateDescriptorSet(descriptorSets.model).
        // AddBufferInfo(0, soldiersUniformBuffer->GetHandle()).
        // AddImageInfo(1, &textures.model.colorMap.descriptor).
        // AddImageInfo(2, &textures.model.normalMap.descriptor).
        // UpdateDescriptorSet().
        // AllocateDescriptorSet(descriptorSets.background).
        // AddBufferInfo(0, gBufferUniformBuffer->GetHandle()).
        // AddImageInfo(1, &textures.background.colorMap.descriptor).
        // AddImageInfo(2, &textures.background.normalMap.descriptor).
        // UpdateDescriptorSet().
        // AllocateDescriptorSet(descriptorSets.shadow).
        // AddBufferInfo(0, shadowUniformBuffer->GetHandle()).
        // UpdateDescriptorSet();
#endif

        DescriptorSetBuilder::Get().Begin(m_Core->GetDevice()).
        AddDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).
        AddDescriptorLayoutBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).
        AddDescriptorLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).
        AddDescriptorLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorLayoutBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10).
        AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10).
        AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10).
        CreateDescriptorPool(descriptorPools.gBuffer).
        CreateDescriptorLayout(descriptorSetLayouts.gBuffer).
        AllocateDescriptorSet(descriptorSets.model).
        AddBufferInfo(0, modelUniformBuffer->GetHandle()).
        AddBufferInfo(1, cameraUniformBuffer->GetHandle()).
        AddBufferInfo(2, instancePositionBuffer->GetHandle(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).
        AddImageInfo(3, &textures.model.colorMap.descriptor).
        AddImageInfo(4, &textures.model.normalMap.descriptor).
        UpdateDescriptorSet().
        AllocateDescriptorSet(descriptorSets.background).
        AddBufferInfo(0, backgroundUniformBuffer->GetHandle()).
        AddBufferInfo(1, cameraUniformBuffer->GetHandle()).
        AddBufferInfo(2, instancePositionBuffer->GetHandle(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).
        AddImageInfo(3, &textures.background.colorMap.descriptor).
        AddImageInfo(4, &textures.background.normalMap.descriptor).
        UpdateDescriptorSet();

        DescriptorSetBuilder::Get().Begin(m_Core->GetDevice()).
        AddDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorLayoutBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorLayoutBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorLayoutBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorLayoutBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorLayoutBinding(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT).
        AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10).
        AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10).
        AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10).
        CreateDescriptorPool(descriptorPools.scene).
        CreateDescriptorLayout(descriptorSetLayouts.scene).
        AllocateDescriptorSet(descriptorSets.scene).
        AddImageInfo(0, scene.FrameBuffer->GetImageView(0), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, scene.FrameBuffer->GetSampler()).
        AddImageInfo(1, scene.FrameBuffer->GetImageView(1), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, scene.FrameBuffer->GetSampler()).
        AddImageInfo(2, scene.FrameBuffer->GetImageView(2), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, scene.FrameBuffer->GetSampler()).
        AddImageInfo(3, shadow.FrameBuffer->GetImageView(0), VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, shadow.FrameBuffer->GetSampler()).
        AddBufferInfo(4, LightMVPsBuffer->GetHandle(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).
        AddBufferInfo(5, LightDataBuffer->GetHandle(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).
        AddBufferInfo(6, cameraUniformBuffer->GetHandle()).
        UpdateDescriptorSet();
        
        DescriptorSetBuilder::Get().Begin(m_Core->GetDevice()).
        AddDescriptorLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT).
        AddDescriptorLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT).
        AddDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10).
        CreateDescriptorPool(descriptorPools.shadow).
        CreateDescriptorLayout(descriptorSetLayouts.shadow).
        AllocateDescriptorSet(descriptorSets.shadow).
        AddBufferInfo(0, instancePositionBuffer->GetHandle(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).
        AddBufferInfo(1, LightDataBuffer->GetHandle(), VK_DESCRIPTOR_TYPE_STORAGE_BUFFER).
        UpdateDescriptorSet();

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &descriptorSetLayouts.gBuffer;

        VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &pipelineLayouts.gBuffer));

        pipelineLayoutCI.pSetLayouts = &descriptorSetLayouts.scene;
        VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &pipelineLayouts.scene));

        pipelineLayoutCI.pSetLayouts = &descriptorSetLayouts.shadow;
        VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &pipelineLayouts.shadow));
    }

    void TestRenderPass::preparePipeline()
    {
        std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_DEPTH_BIAS};

        pipelines.shadow = GraphicsPipelineBuilder::Get().Reset().
        AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/shadow.vspv", VK_SHADER_STAGE_VERTEX_BIT).
        AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/shadow.geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT).
        AddVertexInputState(0, {vkglTF::VertexComponent::Position}).
        AddInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST).
        AddDynamicViewportState().
        AddRasterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE).
        AddMultisampleState().
        AddDepthStencilState(VK_FALSE, VK_TRUE).
        AddEmptyColorBlendAttachment().
        AddDynamicPipelineState(dynamicStateEnables).
        Create(m_Core->GetDevice(), pipelineLayouts.shadow, shadow.FrameBuffer->GetRenderPass());
        
        pipelines.scene = GraphicsPipelineBuilder::Get().Reset().
        AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/scene.vspv", VK_SHADER_STAGE_VERTEX_BIT).
        AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/scene.fspv", VK_SHADER_STAGE_FRAGMENT_BIT).
        AddEmptyVertexInputState().
        AddInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST).
        AddDynamicViewportState().
        AddRasterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE).
        AddMultisampleState().
        AddDepthStencilState(VK_TRUE, VK_TRUE).
        AddColorBlendAttachment(VK_FALSE, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT).
        AddDynamicPipelineState(dynamicStateEnables).
        Create(m_Core->GetDevice(), pipelineLayouts.scene, m_Core->resources.renderPass);

        pipelines.gBuffer = GraphicsPipelineBuilder::Get().Reset().
        AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/mrt.vspv", VK_SHADER_STAGE_VERTEX_BIT).
        AddShaderStage(m_Core->GetDevice(), "C:/dev/Vulkan/Vulkan/assets/shaders/deferredrender/mrt.fspv", VK_SHADER_STAGE_FRAGMENT_BIT).
        AddVertexInputState(0, {vkglTF::VertexComponent::Position, vkglTF::VertexComponent::UV, vkglTF::VertexComponent::Color, vkglTF::VertexComponent::Normal, vkglTF::VertexComponent::Tangent}).
        AddInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST).
        AddDynamicViewportState().
        AddRasterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE).
        AddMultisampleState().
        AddDepthStencilState(VK_TRUE, VK_TRUE).
        AddColorBlendAttachment(VK_FALSE, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT).
        AddColorBlendAttachment(VK_FALSE, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT).
        AddColorBlendAttachment(VK_FALSE, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT).
        AddDynamicPipelineState(dynamicStateEnables).
        Create(m_Core->GetDevice(), pipelineLayouts.gBuffer, scene.FrameBuffer->GetRenderPass());
    }

    void TestRenderPass::prepareUniformBuffers()
    {
        m_InstancePositions.emplace_back(0.0f);
        m_InstancePositions.emplace_back(-7.0f, 0.0, -4.0f, 0.0f);
        m_InstancePositions.emplace_back(4.0f, 0.0, -6.0f, 0.0f);

        AddLight(glm::vec4(-14.0f, -0.5f, 15.0f, 1.0f), glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::mat4(1.0f));
        AddLight(glm::vec4(14.0f, -4.0f, 12.0f, 1.0f), glm::vec4(2.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::mat4(1.0f));
        AddLight(glm::vec4(0.0f, 5.0f, -4.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::mat4(1.0f));
        
        modelUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(ModelData));
        backgroundUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(ModelData));
        
        cameraUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(CameraData));
        instancePositionBuffer = std::make_unique<V2::VulkanBuffer2>(m_Core, sizeof(glm::vec4) * m_InstancePositions.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        instancePositionBuffer->copyToBuffer(m_InstancePositions.data(), sizeof(glm::vec4) * m_InstancePositions.size());
        
        LightMVPsBuffer = std::make_unique<V2::VulkanBuffer2>(m_Core, sizeof(LightMVPs) * m_LightMVPs.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        LightMVPsBuffer->copyToBuffer(m_LightMVPs.data(), sizeof(LightMVPs) * m_LightMVPs.size());
        
        LightDataBuffer = std::make_unique<V2::VulkanBuffer2>(m_Core, sizeof(LightData) * m_LightData.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        LightDataBuffer->copyToBuffer(m_LightData.data(), sizeof(LightData) * m_LightData.size());
        
        // soldiersUniformBufferObject.Model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // soldiersUniformBufferObject.Model = glm::rotate(soldiersUniformBufferObject.Model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //
        // SceneUniformBufferStructObject.debugShadow = 0;
    }

    void TestRenderPass::updateUniformBuffers()
    {
        m_LightData[0].Position.x = -14.0f + std::abs(sin(glm::radians(timer * 360.0f)) * 20.0f);
        m_LightData[0].Position.z = 15.0f + cos(glm::radians(timer *360.0f)) * 1.0f;
        
        m_LightData[1].Position.x = 14.0f - std::abs(sin(glm::radians(timer * 360.0f)) * 2.5f);
        m_LightData[1].Position.z = 13.0f + cos(glm::radians(timer *360.0f)) * 4.0f;
        
        m_LightData[2].Position.x = 0.0f + sin(glm::radians(timer *360.0f)) * 4.0f;
        m_LightData[2].Position.z = 4.0f + cos(glm::radians(timer *360.0f)) * 2.0f;
        
        for (uint32_t i = 0; i < 3; i++)
        {
            // mvp from light's pov (for shadows)
            glm::mat4 shadowProj = glm::perspective(glm::radians(100.0f), 1.0f, 0.1f, 64.0f);
            glm::mat4 shadowView = glm::lookAt(glm::vec3(m_LightData[i].Position), glm::vec3(m_LightData[i].Target), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 shadowModel = glm::mat4(1.0f);
            
            m_LightMVPs[i].MVP = shadowProj * shadowView * shadowModel;
        }

        CameraData cameraData{};
        cameraData.Position = glm::vec4(m_Camera->getViewPosition(), 1.0f);
        cameraData.View = m_Camera->getViewMatrix();
        cameraData.Projection = m_Camera->getProjectionMatrix();

        ModelData modelModelData{};
        modelModelData.ModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelModelData.ModelMatrix = glm::rotate(modelModelData.ModelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        
        ModelData backgroundModelData{};
        backgroundModelData.ModelMatrix = glm::mat4(1.0f);

        modelUniformBuffer->copyToBuffer(&modelModelData, sizeof(ModelData));
        backgroundUniformBuffer->copyToBuffer(&backgroundModelData, sizeof(ModelData));
        
        cameraUniformBuffer->copyToBuffer(&cameraData, sizeof(cameraData));

        LightMVPsBuffer->copyToBuffer(m_LightMVPs.data(), sizeof(LightMVPs) * m_LightMVPs.size());
        LightDataBuffer->copyToBuffer(m_LightData.data(), sizeof(LightData) * m_LightData.size());
    }

    void TestRenderPass::buildDeferredCommandBuffers()
    {
        VkClearValue clearValue{};
        clearValue.color = {{0.0f, 0.0f, 0.0f, 0.0f} };
        clearValue.depthStencil = { 1.0f, 0 };

        VkClearValue clearValue2{};
        clearValue2.color.float32[0] = 0.0f;
        clearValue2.color.float32[1] = 0.0f;
        clearValue2.color.float32[2] = 0.0f;
        clearValue2.color.float32[3] = 0.0f;
        
        clearValue2.depthStencil.depth = 1.0f;
        clearValue2.depthStencil.stencil = 0.0f;

        VkClearValue clearValue3{};
        
        clearValue3.depthStencil.depth = 1.0f;
        clearValue3.depthStencil.stencil = 0.0f;

        std::vector<VkClearValue> sceneClearValues =
        {
            clearValue2, clearValue3
        };

        VkRect2D scissor;
        
        VkRenderPassBeginInfo renderPassBI = init::renderPassBeginInfo();
        renderPassBI.clearValueCount = 1;
        renderPassBI.pClearValues = &clearValue;
        
        std::array<VkClearValue, 4> clearValues = {};
        
        VkCommandBufferBeginInfo cmdBufferBI = init::commandBufferBeginInfo();

        for(int i = 0; i < 3; i++)
        {
            VkCommandBuffer& currentCmdBuffer = m_Core->resources.drawCmdBuffers[i];

            renderPassBI.framebuffer = shadow.FrameBuffer->GetHandle();
            renderPassBI.renderPass = shadow.FrameBuffer->GetRenderPass();
            renderPassBI.renderArea = { {0, 0 }, { shadow.Extent.width, shadow.Extent.height } };

            VK_CHECK(vkBeginCommandBuffer(currentCmdBuffer, &cmdBufferBI));
            vkCmdBeginRenderPass(currentCmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdSetDepthBias(
            currentCmdBuffer,
            1.25f,
            0.0f,
            1.75f);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = shadow.Extent.width;
            viewport.height = shadow.Extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            vkCmdSetViewport(currentCmdBuffer, 0, 1, &viewport);
            
            scissor.offset = {0, 0};
            scissor.extent = { shadow.Extent.width, shadow.Extent.height };
            
            vkCmdSetScissor(currentCmdBuffer, 0, 1, &scissor);

            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.shadow);
            // Background
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.shadow, 0, 1, &descriptorSets.shadow, 0, NULL);
            models.background->draw(currentCmdBuffer);

            // Objects
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.shadow, 0, 1, &descriptorSets.shadow, 0, NULL);
            models.model->bindBuffers(currentCmdBuffer);
            vkCmdDrawIndexed(currentCmdBuffer, models.model->indices.count, 3, 0, 0, 0);
            

            vkCmdEndRenderPass(currentCmdBuffer);
            
            // Clear values for all attachments written in the fragment shader
            clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[3].depthStencil = { 1.0f, 0 };


            renderPassBI.renderPass = scene.FrameBuffer->GetRenderPass();
            renderPassBI.framebuffer = scene.FrameBuffer->GetHandle();
            renderPassBI.renderArea.extent.width = scene.ColorExtent.width;
            renderPassBI.renderArea.extent.height = scene.ColorExtent.height;
            renderPassBI.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassBI.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(currentCmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
            
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = scene.ColorExtent.width;
            viewport.height = scene.ColorExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            vkCmdSetViewport(currentCmdBuffer, 0, 1, &viewport);
            
            scissor.offset = {0, 0};
            scissor.extent = { scene.ColorExtent.width, scene.ColorExtent.height };
            
            vkCmdSetScissor(currentCmdBuffer, 0, 1, &scissor);

            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.gBuffer);
            // Background
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.gBuffer, 0, 1, &descriptorSets.background, 0, NULL);
            models.background->draw(currentCmdBuffer);

            // Objects
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.gBuffer, 0, 1, &descriptorSets.model, 0, NULL);
            models.model->bindBuffers(currentCmdBuffer);
            vkCmdDrawIndexed(currentCmdBuffer, models.model->indices.count, 3, 0, 0, 0);
            
            vkCmdEndRenderPass(currentCmdBuffer);

            if(m_MaterialIDRenderer)
            {
                m_MaterialIDRenderer->OnRender(currentCmdBuffer);
            }
            
            renderPassBI.renderPass = m_Core->resources.renderPass;
            renderPassBI.framebuffer = m_Core->resources.frameBuffers[i];
            renderPassBI.renderArea.extent.width = m_Core->swapchain.extent.width;
            renderPassBI.renderArea.extent.height = m_Core->swapchain.extent.height;
            renderPassBI.clearValueCount = 2;
            renderPassBI.pClearValues = sceneClearValues.data();

            vkCmdBeginRenderPass(currentCmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts.scene, 0, 1, &descriptorSets.scene, 0, nullptr);

            // Final composition as full screen quad
            // Note: Also used for debug display if debugDisplayTarget > 0
            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.scene);
            vkCmdDraw(currentCmdBuffer, 3, 1, 0, 0);

            if(m_FullScreenRenderer && bVisTexture)
            {
                m_FullScreenRenderer->draw2(currentCmdBuffer);
            }
            
            vkCmdEndRenderPass(currentCmdBuffer);
            
            VK_CHECK(vkEndCommandBuffer(currentCmdBuffer));
        }
        
    }

    void TestRenderPass::AddLight(glm::vec4 position, glm::vec4 target, glm::vec4 color, glm::mat4 MVP)
    {
        m_LightData.emplace_back(position, target, color);
        m_LightMVPs.emplace_back(MVP);
    }
}
