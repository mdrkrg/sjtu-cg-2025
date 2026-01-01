#ifndef SRGB_GLSL
#define SRGB_GLSL

/// Convert sRGB to linear
vec3 sRGBToLinear(vec3 srgb) {
    bvec3 cutoff = lessThan(srgb, vec3(0.04045));
    vec3 lower = srgb / 12.92;
    vec3 higher = pow((srgb + 0.055) / 1.055, vec3(2.4));
    return mix(higher, lower, cutoff);
}

/// Convert linear to sRGB
vec3 linearToSRGB(vec3 linear) {
    bvec3 cutoff = lessThan(linear, vec3(0.0031308));
    vec3 lower = linear * 12.92;
    vec3 higher = 1.055 * pow(linear, vec3(1.0 / 2.4)) - 0.055;
    return mix(higher, lower, cutoff);
}

#endif // SRGB_GLSL
