#define PER_INSTANCE_DATA_PRE_COMBINE layout (binding = 0) uniform PerInstance \
{ \
  mat4 invViewMatrix; \
  mat4 invProjMatrix; \
  mat4 invViewProjMatrix; \
  vec4 camPosition; \
  vec4 camParams; \
  vec4 postParams0; \
} uboPerInstance

#define PER_INSTANCE_DATA_POST_COMBINE layout (binding = 0) uniform PerInstance \
{ \
  vec4 haltonSamples; \
  vec4 camParams; \
  vec4 postParams0; \
} uboPerInstance

#define PER_INSTANCE_DATA_SSAO_TEMP_REPROJ layout (binding = 0) uniform PerInstance \
{ \
  mat4 invViewProjMatrix; \
  mat4 invProjMatrix; \
  mat4 projMatrix; \
  mat4 prevViewMatrix; \
} uboPerInstance

#define PER_INSTANCE_DATA_SSAO_HBAO layout (binding = 0) uniform PerInstance \
{ \
  mat4 invProjMatrix; \
} uboPerInstance

#define PER_INSTANCE_DATA_BLUR layout (binding = 0) uniform PerInstance \
{ \
  vec4 blurParams; \
  vec4 camParams; \
} uboPerInstance

#define PER_INSTANCE_DATA_SMAA_VERT layout (binding = 0) uniform PerInstance \
{ \
  vec4 backbufferSize; \
} uboPerInstance

#define PER_INSTANCE_DATA_SMAA_FRAG layout (binding = 1) uniform PerInstance \
{ \
  vec4 backbufferSize; \
} uboPerInstance

#define PER_FRAME_DATA(x) \
layout (binding = x) uniform PerFrame \
{ \
  mat4 viewMatrix; \
  mat4 invProjMatrix; \
  mat4 invViewMatrix; \
  \
  vec4 skyModelConfigs[7]; \
  vec4 skyModelRadiances; \
  \
  vec4 sunLightDirVS; \
  vec4 sunLightDirWS; \
  vec4 skyLightSH[7]; \
  vec4 sunLightColorAndIntensity; \
} uboPerFrame
