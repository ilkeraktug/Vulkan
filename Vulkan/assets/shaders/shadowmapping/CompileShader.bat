for %%f in (*.vert) do (
	%VULKAN_SDK%/Bin32/glslc.exe %%f -o %%~nf.vspv
)

for %%f in (*.frag) do (
	%VULKAN_SDK%/Bin32/glslc.exe %%f -o %%~nf.fspv
)

PAUSE