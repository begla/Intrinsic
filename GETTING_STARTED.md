# Getting Started

## Windows

### Prerequisites

* Visual Studio 2017
* CMake >= 3.8.1
* Qt >= 5.9
* Vulkan SDK >= 1.0.51.0
* PhysX 3.4
* FBX SDK 2015.1

### Setup Instructions

1. Download and install the Qt (open-source) distribution
2. Create a new environment variable `INTR_QTDIR` pointing to `...\QtX.X\X.X\msvc2017_64`

3. Download an install the Vulkan SDK

4. Get access to PhysX SDK 3.4 from NVIDIA: https://developer.nvidia.com/physx-sdk
5. Follow the instructions from NVIDIA and compile the PhysX libraries for the configurations *Release*, *Profile* and *Debug*

   **NOTE:** Make sure that PhysX links the Runtime Library dynamically as Intrinsic does (set `/MD` or `/MDd` for all PhysX project files)

6. Download and install the Autodesk FBX SDK 2015.1

7. Create a new directory called `Intrinsic_Dependencies` on the _same level_ as the Intrinsic repository folder
8. Inside, create a new folder `dependencies` containing two folders `physx3.4` and `fbx`
9. Copy the static/dynamic libraries and header files of both SDKs to folders named `bin` (DLLs), `lib` (LIBs) and `include` respectively

   **NOTE:** Since PhysX 3.4 there is a new set of libraries gathered under the name `PxShared`. Make sure to copy the DLLs, header files and static libraries to the folders too! Especially `PxFoundation`!

10. Switch to the `scripts_win32` folder in the Intrinsic repository directory
11. Execute `ConfigAndBuildDependenciesVS15.bat` to build some of the remaining dependencies automatically
12. Execute `CopyDllsToAppDir_Debug.bat` and `CopyDllsToAppDir_Release.bat`
13. Execute `ConfigVS15` to create a Visual Studio 2017 solution and all project files
14. If everything went well, you'll find the solution in the newly created `build` folder

    **NOTE:** You can also use one of the many build scripts: `Build_Release.bat`, ... to get started

15. Execute `Intrinsic.exe` or `IntrinsicEd.exe` in the `app` directory

16. You're all set - yey!

## Linux (tested with Ubuntu 16.10)

### Prerequisites

```
sudo apt install git cmake ninja-build libsdl2-dev
```

### Setup Instructions

1. Download and install the Qt (open-source) distribution

   Add environment variables to your .zshrc or .bashrc:
   ```
   # Intrinsic  
   export INTR_QTDIR="[...]/Qt/x.x/gcc_64"
   ```

2. Download and install the Vulkan SDK

    Add environment variables to your .zshrc or .bashrc:
    ```
    # Vulkan SDK  
    VULKAN_SDK=~/[...]/VulkanSDK/1.0.30.0/x86_64  
    VK_LAYER_PATH=$VULKAN_SDK/etc/explicit_layer.d  

    export VK_LAYER_PATH  
    export PATH=$VULKAN_SDK/bin:$PATH
    ```

3. Get access to the latest PhysX SDK  

   After cloning of the git repository apply patches from https://github.com/NVIDIAGameWorks/PhysX-3.3/pull/57
   ```
   cd .../PhysX-3.3/PhysXSDK/Source/compiler/linux64  
   make release profile debug  
   ```
   Add environment variables to your .zshrc or .bashrc:
   ```
   export PHYSX_HOME="$HOME/[...]/PhysX-3.3/PhysXSDK/"
   ```

4. Download and install Autodesk FBX SDK  
5. Go to the `scripts_linux` folder in the Intrinsic repository dir
6. Execute the following scripts: `ConfigAndBuildDependencies`, `Config_Release`, `Build` and last but not least `Run_Release`
7. You're all set - yey!
