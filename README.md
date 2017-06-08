# Intrinsic

[![Contribute!](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/begla/Intrinsic/issues) [![Join the Chat](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Intrinsic-Engine/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Intrinsic is a Vulkan based cross-platform game and rendering engine. The project is currently in an early stage of development.

You can find some simple build and setup instructions in `GETTING_STARTED.md`.

# Features
* Fully multihreaded renderer and engine core
* Data oriented engine design and data driven rendering process
  * Inspired by the great blog posts of the Bitsquid/Stingray team, see http://bitsquid.blogspot.de/
* Hot reloading of textures, meshes, shaders and various configuration files
* Capable WYSIWYG editor IntrinsicEd
* Asset importer supporting various texture formats and FBX files for meshes
* Easily configurable benchmarking system
* Scripting via LUA (_work in progress_ - scripting interface far from complete)
* PhysX integration supporting simple rigid bodies and character controllers
* Easy CPU and GPU profiling using Microprofile

# Rendering Features
* Physically based rendering pipeline
  * Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
  * and https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
* Procedural sky radiance model
  * Based on http://cgg.mff.cuni.cz/projects/SkylightModelling/HosekWilkie_SkylightModel_SIGGRAPH2012_Preprint_lowres.pdf
* (Clustered) Deferred opaque and transparent lighting (_only point lights supported at the moment_)
* (Clustered bindless) Deferred decals
* (Clustered) Local irradiance volumes supporting dynamic day and night changes
* Global irradiance (skylight) using the approximated irradiance from the skylight model
* Volumetric fog considering all lighting parameters (sunlight, irradiance, lights, ...)
  * Based on https://bartwronski.files.wordpress.com/2014/08/bwronski_volumetric_fog_siggraph2014.pdf
* Volume based post effect system supporting various effects (fake DoF, vignette, chromatic aberration, bleach bypass, film grain, ...)
* Automated roughness adjustment to fight specular aliasing
  * Based on http://blog.selfshadow.com/publications/s2013-shading-course/rad/s2013_pbs_rad_notes.pdf
* Antialiasing: SMAA
  * See http://www.iryoku.com/smaa/
* HDR rendering with compute based bloom, luminance based eye adaption and filmic tonemapping
* Simple procedural lensflares and lensghosts
* Various material shaders for water, animated foliage and grass, opaque materials, terrain, ...

# Screenshots

![Intrinsic](media/screenshot_0.jpg)
![IntrinsicEd](media/screenshot_1.jpg)
![IntrinsicEd](media/screenshot_3.jpg)
![IntrinsicEd](media/screenshot_2.jpg)

# Build Status

| Platform | Build Status |
|:--------:|:------------:|
| Windows  | [![Windows Build Status](https://ci.appveyor.com/api/projects/status/eevcf6gfm77309ud?svg=true)](https://ci.appveyor.com/project/begla/intrinsic) |
| Linux    |  [![Linux Build Status](https://travis-ci.org/begla/Intrinsic.svg?branch=master)](https://travis-ci.org/begla/Intrinsic) |

# License

```
// Copyright 2016 Benjamin Glatzel
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
```

# Credits

Intrinsic uses the following open-source libraries:

* Qt (IntrinsicEd only, see https://www.qt.io/download)
* SDL 2.0 (see https://www.libsdl.org/download-2.0.php)
* Vulkan SDK (see https://lunarg.com/vulkan-sdk/)
* tlsf (see https://github.com/mattconte/tlsf)
* microprofile (https://github.com/jonasmr/microprofile)
* enkiTS (see https://github.com/dougbinks/enkiTS)
* gli (see http://gli.g-truc.net/0.8.1/index.html)
* glm (see http://glm.g-truc.net/0.9.8/index.html)
* glslang (see https://github.com/KhronosGroup/glslang)
* SPIRV-Cross (see https://github.com/KhronosGroup/SPIRV-Cross)
* LuaJIT (see http://luajit.org/)
* rapidjson (see https://github.com/miloyip/rapidjson)
* rlutil (see https://github.com/tapio/rlutil)
* SOL 2 (see https://github.com/ThePhD/sol2)
* sparsepp (see https://github.com/greg7mdp/sparsepp)
* tinydir (see https://github.com/cxong/tinydir)

Intrinsic uses the following proprietary libraries:

* PhysX 3 (see https://developer.nvidia.com/physx-sdk)
* FBX SDK (IntrinsicEd only, see http://usa.autodesk.com/adsk/servlet/pc/item?siteID=123112&id=10775847)
* BugSplat (see http://www.bugsplat.com/)

... and the following tools in binary format:

* NVTT (see https://developer.nvidia.com/gpu-accelerated-texture-compression)
* Cloc (see http://cloc.sourceforge.net)
* 7za (see http://www.7-zip.org/download.html)

Assets and icons sourced from:

* https://cloud.blender.org/
* http://www.hdrlabs.com/sibl/archive.html
* http://www.flaticon.com
* http://www.gametextures.com
