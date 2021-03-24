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
### :warning: If you get error below replace ```\\``` with ```/``` in .gitmodules file.
<img src="https://user-images.githubusercontent.com/63074357/112389659-28dfae00-8d06-11eb-9be2-f91d4cf6b777.png" width="5000" height="40">

#### When everything is updated, run ```GenerateProjects.bat``` and open Vulkan.sln file.

___

# ‼️ Important : 
### ```GenerateProjects.bat``` file generates VS 2019 solution file. If you want to generate another file check [here](https://github.com/premake/premake-core/wiki/Using-Premake)

## Example for xcode4 <img src="https://user-images.githubusercontent.com/63074357/112390951-28e0ad80-8d08-11eb-8a8c-f343cd300a26.png" width="32" height="32"> : 
```
<SolutionDir> vendor\premake5\bin\premake5 xcode4
