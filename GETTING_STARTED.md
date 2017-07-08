# Getting Started

## Downloading Media Files

I'm trying to keep the size of the repository small. All large media files are available as a package and can be downloaded using the follwing link: http://www.intrinsic-engine.com/downloads/media.7z

Please extract the archive using 7z to the `app/media` directory.

## Windows

<p align="center"><img src="media/windows.png" height="239" alt="Windows"/></p>

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
9. Copy the static/dynamic libraries as well as the header files of both SDKs to folders named `bin` (DLLs), `lib` (LIBs) and `include` respectively. Since the FBX SDK DLLs are named equally for all configurations, they have to be put into separate folders: `bin/release` and `bin/debug`.

    **NOTE:** Since PhysX 3.4 there is a new set of libraries gathered under the name `PxShared`. Make sure to copy the DLLs, header files and static libraries to the folders too!

10. Switch to the `scripts_win32` folder in the Intrinsic repository directory
11. Execute `ConfigAndBuildDependenciesVS15.bat` to build some of the remaining dependencies automatically
12. Execute `CopyDllsToAppDir_Debug.bat` and `CopyDllsToAppDir_Release.bat`
13. Execute `ConfigVS15` to create a Visual Studio 2017 solution and all project files
14. If everything went well, you'll find the solution in the newly created `build` folder

    **NOTE:** You can also use one of the build scripts: `Build_Release.bat`, ... to get started

15. After compiling, execute `Intrinsic.exe` or `IntrinsicEd.exe` in the `app` directory

16. You're all set - yey!

## Linux (tested with Kubuntu 17.04 - x64)

<p align="center"><img src="media/ubuntu.png" height="239" alt="Ubuntu"/></p>

### Restrictions

InstrinsicEd is currently only supported on Windows. Full Linux support is not far off and is certainly planned for the future.

### Prerequisites

```
sudo apt install git cmake clang ninja-build libsdl2-dev
```

### Setup Instructions

1. Download and install the latest Vulkan SDK

    Add environment variables to your .zshrc or .bashrc:
    ```
    # Vulkan SDK  
    export VK_SDK_PATH=.../VulkanSDK/x.x.x.x/x86_64
    ```

2. Get access to the latest PhysX SDK

   ```
   cd .../PhysX-3.4/Source/compiler/linux64
   make release profile debug
   ```

3. Create a new directory called `Intrinsic_Dependencies` on the _same level_ as the Intrinsic repository folder
4. Inside, create a new folder `dependencies` containing another folder `physx3.4`
9. Copy the static/dynamic libraries and header files of both SDKs to folders named `lib` (static and dynamic libraries) and `include` respectively

    **NOTE:** Since PhysX 3.4 there is a new set of libraries gathered under the name `PxShared`. Make sure to copy the header files and static/dynamic libraries to the folders too!

10. Go to the `scripts_linux` folder in the Intrinsic repository dir
11. Execute the following scripts: `ConfigAndBuildDependencies`, `Config_Release`, `Build`, `CopyLibsToAppDir` and last but not least `Run_Release`
12. You're all set - yey!
