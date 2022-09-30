#version 450

layout(binding = 0) uniform FragmentGlobalData{
	float time;
} globals;

layout(location = 0) in vec3 vert_f;
layout(location = 1) in vec3 norm_f;
layout(location = 2) in vec2 uv_f;

layout(location = 0) out vec4 out_color;

void main(){
	out_color = vec4(uv_f, 1.0, 1.0);
}
