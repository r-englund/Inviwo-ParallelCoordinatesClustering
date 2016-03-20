#include "utils/structs.glsl"

layout (std430, binding = 0) readonly buffer DimensionMapping {
    int data[];
} dimensionMapping;

in vec3 texCoord_;

uniform int _nDimensions;

uniform float _offset;

uniform uint _dimensionMask;

int getDimension(vec2 coords, int nDimensions) {
    const float v = coords.x;
    const int b = int(floor(v * nDimensions));

    const int c = dimensionMapping.data[b];

    return c;
}

void main() {
    vec2 texCoords = texCoord_.xy;
    texCoords.y = (texCoords.y - _offset) / (1.0 - _offset);

    const int dim = getDimension(texCoords, _nDimensions);

    if (texCoord_.y < _offset) {
        uint useDimension = bitfieldExtract(_dimensionMask, dim, 1);
        if (useDimension == 0)
            FragData0 = vec4(vec3(0.25, 0.1, 0.1), 1.0);
        else
            FragData0 = vec4(vec3(0.1, 0.25, 0.1), 1.0);
        return;
    }

    if (abs(_offset - texCoord_.y) <= 0.001) {
        FragData0 = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    discard;
}
