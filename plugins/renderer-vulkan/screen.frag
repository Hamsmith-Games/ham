#version 450

layout(location = 0) in vec2 uv_f;

layout(location = 0) out vec4 out_color;

void main(){
	out_color = vec4(uv_f, 1.0, 1.0);
}
