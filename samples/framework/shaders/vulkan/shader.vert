#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;
layout(location = 4) in mat4 inTransform;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	
	mat4 world_matrix = inTransform;

 	vec4 vertex = vec4(inPosition.xyz, 1.);

 	mat3 cross_matrix = mat3(
   		cross(world_matrix[1].xyz, world_matrix[2].xyz),
   		cross(world_matrix[2].xyz, world_matrix[0].xyz),
   		cross(world_matrix[0].xyz, world_matrix[1].xyz));
 	
 	float invdet = 1.0 / dot(cross_matrix[2], world_matrix[2].xyz);
 	mat3 normal_matrix = cross_matrix * invdet;
 	
 	outNormal = normal_matrix * inNormal;
    outColor = inColor;
    outTexCoord = inTexCoord;

    gl_Position = ubo.proj * ubo.view * ubo.model * world_matrix * vec4(inPosition, 1.0);
}