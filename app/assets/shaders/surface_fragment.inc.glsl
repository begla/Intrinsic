#define UV0_TRANSFORMED_ANIMATED vec2(inUV0.x * uboPerMaterial.uvOffsetScale.z + uboPerMaterial.uvOffsetScale.x, \
    (1.0 - inUV0.y) * uboPerMaterial.uvOffsetScale.w + uboPerMaterial.uvOffsetScale.y) \
    + uboPerMaterial.uvAnimation.xy * vec2(uboPerInstance.data0.w)
#define UV0_TRANSFORMED_ANIMATED_FACTOR(_fact) vec2(inUV0.x * uboPerMaterial.uvOffsetScale.z + uboPerMaterial.uvOffsetScale.x, \
    (1.0 - inUV0.y) * uboPerMaterial.uvOffsetScale.w + uboPerMaterial.uvOffsetScale.y) \
    + _fact * uboPerMaterial.uvAnimation.xy * vec2(uboPerInstance.data0.w)
#define UV0_TRANSFORMED vec2(inUV0.x * uboPerMaterial.uvOffsetScale.z + uboPerMaterial.uvOffsetScale.x, \
  (1.0 - inUV0.y) * uboPerMaterial.uvOffsetScale.w + uboPerMaterial.uvOffsetScale.y)
#define UV0 vec2(inUV0.x, (1.0 - inUV0.y))

#define PER_INSTANCE_UBO() layout (binding = 1) uniform PerInstance \
{ \
  vec4 data0; \
} uboPerInstance

#define PER_MATERIAL_UBO() layout (binding = 2) uniform PerMaterial \
{ \
  vec4 uvOffsetScale; \
  vec4 uvAnimation; \
  vec4 pbrBias; \
  \
  uvec4 data0; \
} uboPerMaterial

#define BINDINGS() layout (binding = 3) uniform sampler2D albedoTex; \
layout (binding = 4) uniform sampler2D normalTex; \
layout (binding = 5) uniform sampler2D roughnessTex

#define BINDINGS_LAYERED() layout (binding = 3) uniform sampler2D albedoTex0; \
layout (binding = 4) uniform sampler2D normalTex0; \
layout (binding = 5) uniform sampler2D roughnessTex0; \
layout (binding = 6) uniform sampler2D albedoTex1; \
layout (binding = 7) uniform sampler2D normalTex1; \
layout (binding = 8) uniform sampler2D roughnessTex1; \
layout (binding = 9) uniform sampler2D albedoTex2; \
layout (binding = 10) uniform sampler2D normalTex2; \
layout (binding = 11) uniform sampler2D roughnessTex2; \
layout (binding = 12) uniform sampler2D blendMaskTex

#define OUTPUT() \
layout (location = 0) out vec4 outAlbedo; \
layout (location = 1) out vec4 outNormal; \
layout (location = 2) out vec4 outParameter0
