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

#### When everything is updated, run ```GenerateProjects.bat``` and open Vulkan.sln file.

___

# ‼️ Important if you do not have VS2019 ‼️
#### ```GenerateProjects.bat``` file generates VS 2019 solution file. If you want to generate another file check [here](https://github.com/premake/premake-core/wiki/Using-Premake)

### Example for MinGW <img src="https://user-images.githubusercontent.com/63074357/112556700-cb1d9580-8ddb-11eb-890d-a66221820231.png" width="32" height="32"> : 
```
<SolutionDir> vendor\premake\bin\premake5 gmake2
