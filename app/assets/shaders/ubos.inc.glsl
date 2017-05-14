#define PER_INSTANCE_DATA_PRE_COMBINE layout (binding = 0) uniform PerInstance \
{ \
  mat4 invProjMatrix; \
  mat4 invViewProjMatrix; \
  vec4 camPosition; \
  vec4 camParams; \
} uboPerInstance

#define PER_INSTANCE_DATA_POST_COMBINE layout (binding = 0) uniform PerInstance \
{ \
  ivec4 haltonSamples; \
  vec4 camParams; \
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
