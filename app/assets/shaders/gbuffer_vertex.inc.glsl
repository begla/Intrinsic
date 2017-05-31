#define PER_INSTANCE_UBO layout (binding = 0) uniform PerInstance \
{ \
  mat4 worldMatrix; \
  mat4 normalMatrix; \
  mat4 worldViewProjMatrix; \
  mat4 worldViewMatrix; \
  vec4 data0; \
} uboPerInstance

#define INPUT() layout (location = 0) in vec3 inPosition; \
layout (location = 1) in vec2 inUV0; \
layout (location = 2) in vec3 inNormal; \
layout (location = 3) in vec3 inTangent; \
layout (location = 4) in vec3 inBinormal; \
layout (location = 5) in vec4 inColor
