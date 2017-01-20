#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
  vec4 vertex = vec4(inPosition.xyz, 1.0);
  gl_Position = ubo.proj * ubo.view * vertex;
  outColor = inColor;
}