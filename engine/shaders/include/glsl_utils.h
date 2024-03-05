#ifndef GLSL_UTILS_H
#define GLSL_UTILS_H

vec2 ndcxy_to_uv(vec2 ndcxy) { return ndcxy * vec2(0.5, 0.5) + vec2(0.5, 0.5); }
vec2 uv_to_ndcxy(vec2 uv) { return uv * vec2(2.0, 2.0) + vec2(-1.0, -1.0); }

vec4 GammaCorrection(vec4 color, float gamma) {
    return vec4(pow(color.rgb, vec3(1.0 / gamma)), color.a);
}

#endif