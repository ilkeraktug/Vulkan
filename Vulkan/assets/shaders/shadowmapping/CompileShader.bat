for %%f in (*.vert) do (
	%VULKAN_SDK%/Bin/glslc.exe %%f -o %%~nf.vspv
)

for %%f in (*.frag) do (
	%VULKAN_SDK%/Bin/glslc.exe %%f -o %%~nf.fspv
)

PAUSE