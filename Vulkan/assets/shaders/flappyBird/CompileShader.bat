%VULKAN_SDK%/Bin32/glslc.exe pipeObject.vert -o pipeObjectVertex.spv
%VULKAN_SDK%/Bin32/glslc.exe pipeObject.frag -o pipeObjectFragment.spv

%VULKAN_SDK%/Bin32/glslc.exe background.vert -o backgroundVertex.spv
%VULKAN_SDK%/Bin32/glslc.exe background.frag -o backgroundFragment.spv

%VULKAN_SDK%/Bin32/glslc.exe birdObject.vert -o birdObjectVertex.spv
%VULKAN_SDK%/Bin32/glslc.exe birdObject.frag -o birdObjectFragment.spv
pause