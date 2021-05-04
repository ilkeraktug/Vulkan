# Vulkan
#### I'm learning Vulkan. I follow [vulkan-tutorial.com](https://vulkan-tutorial.com/) website to learn how to create a Vulkan program. The codes are copied/inspired from the given link.

## <img src="https://premake.github.io/premake-logo.png" width=32 /> Premake
### I'm using premake5 to build my projects. 
#### What is [premake](https://github.com/premake/premake-core/wiki)?
#### How to build project ?
```
git clone https://github.com/ilkeraktug/Vulkan.git
cd Vulkan
git submodule update --init --recursive
```
#### ⚠️ You need to install Vulkan SDK and should be added to PATH. VulkanSDK can be downloaded from [here.](https://www.lunarg.com/vulkan-sdk/)
#### When everything is updated, run ```GenerateProjects.bat``` and open Vulkan.sln file.
#### ⚠️ For now only Windows is supported!
___

# ‼️ Important if you do not have VS2019 ‼️
#### ```GenerateProjects.bat``` file generates VS 2019 solution file. If you want to generate another file check [here](https://github.com/premake/premake-core/wiki/Using-Premake)

### Example for MinGW : 
```
<SolutionDir> vendor\premake\bin\premake5 gmake2

![](https://media.giphy.com/media/xF6XV9VWX5JacjDCqX/giphy.gif)