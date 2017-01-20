#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

vec4 ambient(vec3 _world_normal) {
	vec3 normal = normalize(_world_normal);
    vec3 alpha = (normal + 1.) * .5;
    vec4 bt = mix(vec4(.3, .3, .7, .7), vec4(.4, .4, .8, .8), alpha.xzxz);
    vec4 ambient = vec4(mix(vec3(bt.x, .3, bt.y), vec3(bt.z, .8, bt.w), alpha.y), 1.);
	return ambient;
}

void main() {
    //outColor = texture(texSampler, inTexCoord);
    outColor = ambient(inNormal) * vec4(inColor,1.0);
}