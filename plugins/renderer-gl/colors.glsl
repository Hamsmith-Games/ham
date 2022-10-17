#version 450 core

#ifndef HAM_COLORS_GLSL
#define HAM_COLORS_GLSL

vec3 rgb2hsv(vec3 c){
    const vec4 k = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    const vec4 p = mix(vec4(c.bg, k.wz), vec4(c.gb, k.xy), step(c.b, c.g));
    const vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    const float d = q.x - min(q.w, q.y);
    const float e = 1.0e-10;

    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv_to_rgb(in vec3 c){
	const vec4 k = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
	const vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
	return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

#endif // !HAM_COLORS_GLSL
