#version 450

#define gl_VertexID gl_VertexIndex
#define gl_InstanceID gl_InstanceIndex

layout(binding = 0) uniform VertexGlobalData{
	mat4 view_proj;
} globals;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec3 uv;

const vec2 positions[4] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0)
);

const vec2 uvs[4] = vec2[](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

layout(location = 0) out vec3 pos_f;
layout(location = 1) out vec3 norm_f;
layout(location = 2) out vec2 uv_f;

void main(){
	pos_f = pos;
	norm_f = norm;
	uv_f = uvs[gl_VertexID];

	gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}
