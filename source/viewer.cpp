#include "viewer.hpp"

#include <iostream>
#include <fstream>
#include <limits>

Viewer::~Viewer() {
  for (auto i = size_t{0}; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

  cleanupSwapChain();

  vkDestroyDevice(logicalDevice, nullptr);
  vkDestroySurfaceKHR(vkInstance, surface, nullptr);
  vkDestroyInstance(vkInstance, nullptr);

  glfwTerminate();
  glfwDestroyWindow(window);
}

void Viewer::run() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  auto glfwVideoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  width = 0.8 * glfwVideoMode->width;
  height = 0.8 * glfwVideoMode->height;

  window = glfwCreateWindow(width, height, "3D Viewer", nullptr, nullptr);

  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, exit_key_callback);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

  VkApplicationInfo applicationInfo{
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pApplicationName = "3D Viewer",
    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
    .pEngineName = "3D Viewer",
    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
    .apiVersion = VK_API_VERSION_1_1
  };

  auto glfwExtensionCount = uint32_t{0};
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  VkInstanceCreateInfo instanceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pApplicationInfo = &applicationInfo,
    .enabledExtensionCount = glfwExtensionCount,
    .ppEnabledExtensionNames = glfwExtensions
  };

  if (vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance) != VK_SUCCESS) {
    throw std::runtime_error("Could not create vulkan instance");
  }

  if (glfwCreateWindowSurface(vkInstance, window, nullptr, &surface) != VK_SUCCESS) {
    throw std::runtime_error("Could not create window surface");
  }

  auto pyhsicalDeviceCount = uint32_t{0};
  vkEnumeratePhysicalDevices(vkInstance, &pyhsicalDeviceCount, nullptr);
  std::vector<VkPhysicalDevice> physicalDevices(pyhsicalDeviceCount);
  vkEnumeratePhysicalDevices(vkInstance, &pyhsicalDeviceCount, physicalDevices.data());
  physicalDevice = physicalDevices[0];

  auto queuePriority = float{1.0f};
  VkDeviceQueueCreateInfo logicalDeviceQueueCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = 0,
    .queueCount = 1,
    .pQueuePriorities = &queuePriority
  };

  VkPhysicalDeviceFeatures logicalDeviceFeatures{};
  VkDeviceCreateInfo logicalDeviceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .queueCreateInfoCount = 1,
    .pQueueCreateInfos = &logicalDeviceQueueCreateInfo,
    .enabledExtensionCount = 1,
    .ppEnabledExtensionNames = logicalDeviceExtensions.data(),
    .pEnabledFeatures = &logicalDeviceFeatures
  };

  if (vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
    throw std::runtime_error("Could not create logical device");
  }

  vkGetDeviceQueue(logicalDevice, 0, 0, &graphicsQueue);
  vkGetDeviceQueue(logicalDevice, 0, 0, &presentationQueue);

  createSwapChain();  

  VkSemaphoreCreateInfo semaphoreCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
  };

  VkFenceCreateInfo fenceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT
  };

  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  for (auto i = size_t{0}; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
        || vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
        || vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
          throw std::runtime_error("failed to record command buffer!");
    }
  }

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    drawFrame();
  }

  vkDeviceWaitIdle(logicalDevice);
}

void Viewer::drawFrame() {
  vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

  auto imageIndex = uint32_t{0};
  auto acquireNextImageResult = vkAcquireNextImageKHR(logicalDevice, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

  if (acquireNextImageResult == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapChain();
      return;
  } else if (acquireNextImageResult != VK_SUCCESS && acquireNextImageResult != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("failed to acquire swap chain image!");
  }

  std::vector<VkSemaphore> waitSemaphores{imageAvailableSemaphores[currentFrame]};
  std::vector<VkPipelineStageFlags> waitStages{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  std::vector<VkSemaphore> signalSemaphores{renderFinishedSemaphores[currentFrame]};

  VkSubmitInfo submitInfo{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = waitSemaphores.data(),
    .pWaitDstStageMask = waitStages.data(),
    .commandBufferCount = 1,
    .pCommandBuffers = &commandBuffers[imageIndex],
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = signalSemaphores.data()
  };

  vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

  if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkSwapchainKHR swapChains[] = {swapChain};
  VkPresentInfoKHR presentInfo = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = signalSemaphores.data(),
    .swapchainCount = 1,
    .pSwapchains = swapChains,
    .pImageIndices = &imageIndex,
  };

  auto queuePresentResult = vkQueuePresentKHR(presentationQueue, &presentInfo);

  if (queuePresentResult == VK_ERROR_OUT_OF_DATE_KHR || queuePresentResult == VK_SUBOPTIMAL_KHR || resizeHappended) {
    resizeHappended = false;
    recreateSwapChain();
  } else if (queuePresentResult != VK_SUCCESS) {
      throw std::runtime_error("failed to present swap chain image!");
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Viewer::recreateSwapChain() {
  vkDeviceWaitIdle(logicalDevice);

  cleanupSwapChain();
  createSwapChain();
}

void Viewer::createSwapChain() {
  VkSurfaceFormatKHR surfaceFormat{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  VkPresentModeKHR presentationMode{VK_PRESENT_MODE_FIFO_KHR};

  auto newWidth = int{0};
  auto newHeight = int{0};
  glfwGetFramebufferSize(window, &newWidth, &newHeight);
  VkExtent2D swapExtent{uint32_t(newWidth), uint32_t(newHeight)};

  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

  VkSwapchainCreateInfoKHR swapChainCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .surface = surface,
    .minImageCount = surfaceCapabilities.minImageCount + 1,
    .imageFormat = surfaceFormat.format,
    .imageColorSpace = surfaceFormat.colorSpace,
    .imageExtent = swapExtent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .preTransform = surfaceCapabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = presentationMode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE
  };

  if (vkCreateSwapchainKHR(logicalDevice, &swapChainCreateInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("Could not create swap chain");
  }

  auto swapChainImageCount = uint32_t{0};
  vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, nullptr);
  swapChainImages.resize(swapChainImageCount);
  vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, swapChainImages.data());

  swapChainImageViews.resize(swapChainImages.size());
  for (auto i = size_t{0}; i < swapChainImages.size(); i++) {
    VkComponentMapping componentMapping{
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY
    };

    VkImageSubresourceRange imageSubResourceRange{
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1
    };

    VkImageViewCreateInfo imageViewCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = swapChainImages[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = surfaceFormat.format,
      .components = componentMapping,
      .subresourceRange = imageSubResourceRange      
    };

    if (vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("Could not create image view");
    }
  }

  VkAttachmentDescription attachmentDescription{
    .format = surfaceFormat.format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };

  VkAttachmentReference attachmentReference{
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  VkSubpassDescription subpassDescription{
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &attachmentReference
  };

  VkSubpassDependency subpassDependency{
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
  };

  VkRenderPassCreateInfo renderPassCreateInfo{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .attachmentCount = 1,
    .pAttachments = &attachmentDescription,
    .subpassCount = 1,
    .pSubpasses = &subpassDescription,
    .dependencyCount = 1,
    .pDependencies = &subpassDependency
  };

  if (vkCreateRenderPass(logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("Could not create render pass");   
  }

  VkShaderModule vertexShaderModule, fragmentShaderModule;
  createShaderModuleFromBinary("../source/shaders/vert.spv", vertexShaderModule);
  createShaderModuleFromBinary("../source/shaders/frag.spv", fragmentShaderModule);

  VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
    .module = vertexShaderModule,
    .pName = "main"
  };

  VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    .module = fragmentShaderModule,
    .pName = "main"
  };

  auto shaderStages = std::vector<VkPipelineShaderStageCreateInfo>{
    vertexShaderStageCreateInfo,
    fragmentShaderStageCreateInfo
  };

  VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = 0
  };

  VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
  };

  VkViewport viewport{
    .x = 0.0f,
    .y = 0.0f,
    .width = float(swapExtent.width),
    .height = float(swapExtent.height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };

  VkRect2D scissor{
    .offset = {0, 0},
    .extent = swapExtent
  };

  VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .viewportCount = 1,
    .pViewports = &viewport,
    .scissorCount = 1,
    .pScissors = &scissor
  };

  VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.0f
  };

  VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
    .minSampleShading = 1.0f
  };

  VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{
    .blendEnable = VK_FALSE,
    .colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT
      | VK_COLOR_COMPONENT_G_BIT
      | VK_COLOR_COMPONENT_B_BIT
      | VK_COLOR_COMPONENT_A_BIT    
  };

  VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .attachmentCount = 1,
    .pAttachments = &pipelineColorBlendAttachmentState
  };

  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
  };

  if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Could not create pipeline layout");
  }

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = 2,
    .pStages = shaderStages.data(),
    .pVertexInputState = &pipelineVertexInputStateCreateInfo,
    .pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
    .pViewportState = &pipelineViewportStateCreateInfo,
    .pRasterizationState = &pipelineRasterizationStateCreateInfo,
    .pMultisampleState = &pipelineMultisampleStateCreateInfo,
    .pColorBlendState = &pipelineColorBlendStateCreateInfo,
    .layout = pipelineLayout,
    .renderPass = renderPass,
    .subpass = 0
  };

  if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
    throw std::runtime_error("Could not create graphics pipeline");
  }

  vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
  vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);

  swapChainFramebuffers.resize(swapChainImageViews.size());
  for (auto i = size_t{0}; i < swapChainImageViews.size(); i++) {
    auto attachments = std::vector<VkImageView>{swapChainImageViews[i]};

    VkFramebufferCreateInfo frameBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = renderPass,
      .attachmentCount = 1,
      .pAttachments = attachments.data(),
      .width = swapExtent.width,
      .height = swapExtent.height,
      .layers = 1
    };

    if (vkCreateFramebuffer(logicalDevice, &frameBufferCreateInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Could not create frame buffer");
    }
  }

  VkCommandPoolCreateInfo commandPoolCreateInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .queueFamilyIndex = 0
  };

  if (vkCreateCommandPool(logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }

  commandBuffers.resize(swapChainFramebuffers.size());

  VkCommandBufferAllocateInfo commandBufferAllocateInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .commandPool = commandPool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = uint32_t(commandBuffers.size())
  };

  if (vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }

  for (auto i = size_t{0}; i < commandBuffers.size(); i++) {
    VkCommandBufferBeginInfo commandBufferBeginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    };

    if (vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRect2D renderArea{
      .offset = {0, 0},
      .extent = swapExtent
    };

    VkClearValue clearValue = {0.0f, 0.0f, 0.0f, 1.0f};

    VkRenderPassBeginInfo renderPassBeginInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = renderPass,
      .framebuffer = swapChainFramebuffers[i],
      .renderArea = renderArea,
      .clearValueCount = 1,
      .pClearValues = &clearValue
    };

    vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffers[i]);

    if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to record command buffer!");
    }
  }
}

void Viewer::cleanupSwapChain() {
  for (auto framebuffer : swapChainFramebuffers) {
      vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
  }

  vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
  vkDestroyRenderPass(logicalDevice, renderPass, nullptr);


  for (auto swapChainImageView : swapChainImageViews) {
    vkDestroyImageView(logicalDevice, swapChainImageView, nullptr);
  }

  vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
}

void Viewer::exit_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void Viewer::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
  auto app = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(window));
  app->resizeHappended = true;
}

void Viewer::createShaderModuleFromBinary(const std::string& filename, VkShaderModule& shaderModule) {
  std::ifstream shader{filename, std::ios::ate | std::ios::binary};

  if (!shader.is_open()) {
    throw std::runtime_error("failed to open shader file!");
  }

  auto fileSize = size_t{shader.tellg()};
  std::vector<char> buffer(fileSize);

  shader.seekg(0);
  shader.read(buffer.data(), fileSize);
  shader.close();

  VkShaderModuleCreateInfo shaderModuleCreateInfo{
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = buffer.size(),
    .pCode = reinterpret_cast<const uint32_t*>(buffer.data())
  };

  if (vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Could not create shader module");
  }
}
