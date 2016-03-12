#include "utils/structs.glsl"

layout (std430, binding = 0) readonly buffer Indices {
    int data[];
} indices;

uniform int _nBins;
uniform ivec2 _outportSize;

int getBin(vec2 coords, int nBins) {
    const float v = coords.y;
    const int b = int(floor(v * nBins));
    return b;
}

void main() {
    const vec2 texCoords = gl_FragCoord.xy * (1.f / vec2(_outportSize));
    
    const int bin = getBin(texCoords, _nBins);
    const int value = indices.data[bin];

    FragData0 = vec4(float(value) / 3.0, 0.0, 0.0, 1.0);
    // const int value = getValue(bin, dim);
}
