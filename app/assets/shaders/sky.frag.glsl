// Intrinsic
// Copyright (c) 2016 Benjamin Glatzel
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : enable

#include "lib_math.glsl"
#include "surface_fragment.inc.glsl"

// Ubos
PER_MATERIAL_UBO();
PER_INSTANCE_UBO();

// Bindings
layout (binding = 3) uniform samplerCube albedoTex;

// Input
layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec3 inNormal;

// Output
layout (location = 0) out vec4 outAlbedo;

void main()
{ 
  const vec4 albedo = textureLod(albedoTex, inNormal, 1.0);
  outAlbedo = vec4(albedo.rgb * uboPerInstance.data0.x, 1.0);
}
