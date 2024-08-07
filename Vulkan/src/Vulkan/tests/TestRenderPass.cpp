#include "pch.h"
#include "TestRenderPass.h"

#include "Vulkan/Renderer/glTFModel.h"
#include "Vulkan/Renderer/PerspectiveCamera.h"
#include "Vulkan/Renderer/VulkanShader.h"
#include "Vulkan/Renderer/V2/GraphicsPipelineBuilder.h"
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

        runBatchFile();
        loadassets();
        deferredSetup();
        shadowSetup();
        prepareUniformBuffers();
        prepareDescriptorSetLayout();
        preparePipeline();
        writeDescriptors();
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

        timer += timerSpeed * deltaTime;
        if (timer > 1.0f)
        {
            timer -= 1.0f;
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
        models.model.loadFromFile(AssetsPath + "models/armor/armor.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);
        models.background.loadFromFile(AssetsPath + "models/deferred_box.gltf", m_Core, m_Core->queue.TransferQueue, glTFLoadingFlags);
        textures.model.colorMap.loadFromFile(AssetsPath+ "models/armor/colormap_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.model.normalMap.loadFromFile(AssetsPath + "models/armor/normalmap_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.background.colorMap.loadFromFile(AssetsPath + "textures/stonefloor02_color_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
        textures.background.normalMap.loadFromFile(AssetsPath + "textures/stonefloor02_normal_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_Core, m_Core->queue.TransferQueue);
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
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings
        {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT, nullptr },
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr  },
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            {4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
        };
        
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
        descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCI.bindingCount = descriptorSetLayoutBindings.size();
        descriptorSetLayoutCI.pBindings = descriptorSetLayoutBindings.data();
        
        VK_CHECK(vkCreateDescriptorSetLayout(m_Core->GetDevice(), &descriptorSetLayoutCI, nullptr, &m_DescriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &m_DescriptorSetLayout;

        VK_CHECK(vkCreatePipelineLayout(m_Core->GetDevice(), &pipelineLayoutCI, nullptr, &m_PipelineLayout));
    }

    void TestRenderPass::writeDescriptors()
    {
        std::vector<VkDescriptorPoolSize> descriptorPools =
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
        };
        
        VkDescriptorPoolCreateInfo descriptorPoolCI{};
        descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCI.maxSets = 12;
        descriptorPoolCI.poolSizeCount = 2;
        descriptorPoolCI.pPoolSizes = descriptorPools.data();

        VK_CHECK(vkCreateDescriptorPool(m_Core->GetDevice(), &descriptorPoolCI, nullptr, &m_DescriptorPool));
        
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_DescriptorSetLayout;
        
        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &m_DescriptorSet));

        VkDescriptorBufferInfo binding0Info;
        binding0Info.buffer = shadowUniformBuffer->GetHandle();
        binding0Info.offset = 0;
        binding0Info.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo binding0gBufferInfo;
        binding0gBufferInfo.buffer = gBufferUniformBuffer->GetHandle();
        binding0gBufferInfo.offset = 0;
        binding0gBufferInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo soldiersBinding0gBufferInfo;
        soldiersBinding0gBufferInfo.buffer = soldiersUniformBuffer->GetHandle();
        soldiersBinding0gBufferInfo.offset = 0;
        soldiersBinding0gBufferInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo binding4Info;
        binding4Info.buffer = sceneUniformBuffer->GetHandle();
        binding4Info.offset = 0;
        binding4Info.range = VK_WHOLE_SIZE;

        VkDescriptorImageInfo binding1Info;
        binding1Info.sampler = scene.FrameBuffer->GetSampler();
        binding1Info.imageView = scene.FrameBuffer->GetImageView(0);
        binding1Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo binding2Info;
        binding2Info.sampler = scene.FrameBuffer->GetSampler();
        binding2Info.imageView = scene.FrameBuffer->GetImageView(1);
        binding2Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo binding3Info;
        binding3Info.sampler = scene.FrameBuffer->GetSampler();
        binding3Info.imageView = scene.FrameBuffer->GetImageView(2);
        binding3Info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorImageInfo binding5Info;
        binding5Info.sampler = shadow.FrameBuffer->GetSampler();
        binding5Info.imageView = shadow.FrameBuffer->GetImageView(0);
        binding5Info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        

        std::vector<VkWriteDescriptorSet> descriptorSetWrites =
        {
            init::getImageWrite(m_DescriptorSet, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &binding1Info),
            init::getImageWrite(m_DescriptorSet, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &binding2Info),
            init::getImageWrite(m_DescriptorSet, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &binding3Info),
            init::getBufferWrite(m_DescriptorSet, 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &binding4Info),
            init::getImageWrite(m_DescriptorSet, 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &binding5Info),
        };
        vkUpdateDescriptorSets(m_Core->GetDevice(), descriptorSetWrites.size(), descriptorSetWrites.data(), 0, nullptr);


        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.model));
        std::vector<VkWriteDescriptorSet> descriptorSetWrites1 =
        {
            init::getBufferWrite(descriptorSets.model, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &soldiersBinding0gBufferInfo),
            init::getImageWrite(descriptorSets.model, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textures.model.colorMap.descriptor),
            init::getImageWrite(descriptorSets.model, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textures.model.normalMap.descriptor),
        };

        vkUpdateDescriptorSets(m_Core->GetDevice(), descriptorSetWrites1.size(), descriptorSetWrites1.data(), 0, nullptr);
        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.background));

        std::vector<VkWriteDescriptorSet> descriptorSetWrites2 =
        {
            init::getBufferWrite(descriptorSets.background, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &binding0gBufferInfo),
            init::getImageWrite(descriptorSets.background, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textures.background.colorMap.descriptor),
            init::getImageWrite(descriptorSets.background, 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &textures.background.normalMap.descriptor),
        };
        vkUpdateDescriptorSets(m_Core->GetDevice(), descriptorSetWrites2.size(), descriptorSetWrites2.data(), 0, nullptr);
        

        VK_CHECK(vkAllocateDescriptorSets(m_Core->GetDevice(), &allocInfo, &descriptorSets.shadow));
        std::vector<VkWriteDescriptorSet> descriptorSetWrites3 =
        {
            init::getBufferWrite(descriptorSets.shadow, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &binding0Info),
        };

        vkUpdateDescriptorSets(m_Core->GetDevice(), descriptorSetWrites3.size(), descriptorSetWrites3.data(), 0, nullptr);
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
        Create(m_Core->GetDevice(), m_PipelineLayout, shadow.FrameBuffer->GetRenderPass());
        
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
        Create(m_Core->GetDevice(), m_PipelineLayout, m_Core->resources.renderPass);

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
        Create(m_Core->GetDevice(), m_PipelineLayout, scene.FrameBuffer->GetRenderPass());
    }

    void TestRenderPass::prepareUniformBuffers()
    {
        SceneUniformBufferStructObject.lights[0].Color = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
        SceneUniformBufferStructObject.lights[1].Color = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
        SceneUniformBufferStructObject.lights[2].Color = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

        SceneUniformBufferStructObject.lights[0].Position = glm::vec4(-14.0f, -0.5f, 15.0f, 1.0f);
        SceneUniformBufferStructObject.lights[1].Position = glm::vec4(14.0f, -4.0f, 12.0f, 1.0f);
        SceneUniformBufferStructObject.lights[2].Position = glm::vec4(0.0f, 5.0f, -4.0f, 1.0f);

        SceneUniformBufferStructObject.lights[0].Target = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
        SceneUniformBufferStructObject.lights[1].Target = glm::vec4(2.0f, 0.0f, 0.0f, 0.0f);
        SceneUniformBufferStructObject.lights[2].Target = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

        soldiersUniformBufferObject.Model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        soldiersUniformBufferObject.Model = glm::rotate(soldiersUniformBufferObject.Model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        SceneUniformBufferStructObject.debugShadow = 0;
        
        gBufferUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(gBufferUniformBufferStruct));
        shadowUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(ShadowUniformBufferStruct));
        sceneUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(SceneUniformBufferStruct));
        soldiersUniformBuffer = std::make_unique<V2::VulkanUniformBuffer2>(m_Core, sizeof(gBufferUniformBufferStruct));
    }

    void TestRenderPass::updateUniformBuffers()
    {
        SceneUniformBufferStructObject.lights[0].Position.x = -14.0f + std::abs(sin(glm::radians(timer * 360.0f)) * 20.0f);
        SceneUniformBufferStructObject.lights[0].Position.z = 15.0f + cos(glm::radians(timer *360.0f)) * 1.0f;
        
        SceneUniformBufferStructObject.lights[1].Position.x = 14.0f - std::abs(sin(glm::radians(timer * 360.0f)) * 2.5f);
        SceneUniformBufferStructObject.lights[1].Position.z = 13.0f + cos(glm::radians(timer *360.0f)) * 4.0f;
        
        SceneUniformBufferStructObject.lights[2].Position.x = 0.0f + sin(glm::radians(timer *360.0f)) * 4.0f;
        SceneUniformBufferStructObject.lights[2].Position.z = 4.0f + cos(glm::radians(timer *360.0f)) * 2.0f;

        for (uint32_t i = 0; i < 3; i++)
        {
            // mvp from light's pov (for shadows)
            glm::mat4 shadowProj = glm::perspective(glm::radians(100.0f), 1.0f, 0.1f, 64.0f);
            glm::mat4 shadowView = glm::lookAt(glm::vec3(SceneUniformBufferStructObject.lights[i].Position), glm::vec3(SceneUniformBufferStructObject.lights[i].Target), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 shadowModel = glm::mat4(1.0f);
            
            ShadowUniformBufferStructObject.MVPs[i] = shadowProj * shadowView * shadowModel;
            ShadowUniformBufferStructObject.InstancePos[0] = glm::vec4(0.0f);
            ShadowUniformBufferStructObject.InstancePos[1] = glm::vec4(-7.0f, 0.0, -4.0f, 0.0f);
            ShadowUniformBufferStructObject.InstancePos[2] = glm::vec4(4.0f, 0.0, -6.0f, 0.0f);
            
            SceneUniformBufferStructObject.lights[i].MVP = shadowProj * shadowView * shadowModel;
        }

        // SceneUniformBufferStructObject.viewPosition = glm::vec4(m_Camera->getPosition(), 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);
        SceneUniformBufferStructObject.viewPosition = m_Camera->getViewPosition();

        gBufferUniformBufferStructObject.InstancePos[0] = glm::vec4(0.0f);
        gBufferUniformBufferStructObject.InstancePos[1] = glm::vec4(-7.0f, 0.0, -4.0f, 0.0f);
        gBufferUniformBufferStructObject.InstancePos[2] = glm::vec4(4.0f, 0.0, -6.0f, 0.0f);

        gBufferUniformBufferStructObject.Model = glm::mat4(1.0f);//glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        gBufferUniformBufferStructObject.View = m_Camera->getViewMatrix();
        gBufferUniformBufferStructObject.Projection = m_Camera->getProjectionMatrix();

        soldiersUniformBufferObject.InstancePos[0] = glm::vec4(0.0f);
        soldiersUniformBufferObject.InstancePos[1] = glm::vec4(-7.0f, 0.0, -4.0f, 0.0f);
        soldiersUniformBufferObject.InstancePos[2] = glm::vec4(4.0f, 0.0, -6.0f, 0.0f);
        
        soldiersUniformBufferObject.View = m_Camera->getViewMatrix();
        soldiersUniformBufferObject.Projection = m_Camera->getProjectionMatrix();
        
        gBufferUniformBuffer->copyToBuffer(&gBufferUniformBufferStructObject, sizeof(gBufferUniformBufferStructObject));
        soldiersUniformBuffer->copyToBuffer(&soldiersUniformBufferObject, sizeof(soldiersUniformBufferObject));
        shadowUniformBuffer->copyToBuffer(&ShadowUniformBufferStructObject, sizeof(ShadowUniformBufferStructObject));
        sceneUniformBuffer->copyToBuffer(&SceneUniformBufferStructObject, sizeof(SceneUniformBufferStructObject));
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
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &descriptorSets.shadow, 0, NULL);
            models.background.draw(currentCmdBuffer);

            // Objects
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &descriptorSets.shadow, 0, NULL);
            models.model.bindBuffers(currentCmdBuffer);
            vkCmdDrawIndexed(currentCmdBuffer, models.model.indices.count, 3, 0, 0, 0);
            

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
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &descriptorSets.background, 0, NULL);
            models.background.draw(currentCmdBuffer);

            // Objects
            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &descriptorSets.model, 0, NULL);
            models.model.bindBuffers(currentCmdBuffer);
            vkCmdDrawIndexed(currentCmdBuffer, models.model.indices.count, 3, 0, 0, 0);
            
            vkCmdEndRenderPass(currentCmdBuffer);

            renderPassBI.renderPass = m_Core->resources.renderPass;
            renderPassBI.framebuffer = m_Core->resources.frameBuffers[i];
            renderPassBI.renderArea.extent.width = m_Core->swapchain.extent.width;
            renderPassBI.renderArea.extent.height = m_Core->swapchain.extent.height;
            renderPassBI.clearValueCount = 2;
            renderPassBI.pClearValues = sceneClearValues.data();

            vkCmdBeginRenderPass(currentCmdBuffer, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);

            // Final composition as full screen quad
            // Note: Also used for debug display if debugDisplayTarget > 0
            vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.scene);
            vkCmdDraw(currentCmdBuffer, 3, 1, 0, 0);
            
            vkCmdEndRenderPass(currentCmdBuffer);
            VK_CHECK(vkEndCommandBuffer(currentCmdBuffer));
        }
        
    }
}
