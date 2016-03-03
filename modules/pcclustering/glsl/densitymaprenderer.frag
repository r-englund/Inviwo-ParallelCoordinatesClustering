#include "utils/structs.glsl"

layout (std430, binding = 0) buffer BinningValues {
    int values[];
} binning;


uniform int _nBins;
uniform int _nDimensions;

uniform ivec2 _outportSize;

int getBin(vec2 coords, int nBins) {
    const float v = coords.y;
    const int b = int(floor(v * nBins));
    return b;
}

float getBinNormalized(int bin) {
    return float(bin) / float(_nBins);
}

int getDimension(vec2 coords, int nDimensions) {
    const float v = coords.x;
    const int b = int(floor(v * nDimensions));
    return b;
}

float getDimensionNormalized(int dim) {
    return float(dim) / float(_nDimensions);
}

int getValue(int bin, int dimension) {
    return binning.values[bin * _nDimensions + dimension];
}

void main() {
    const vec2 texCoords = gl_FragCoord.xy * (1.f / vec2(_outportSize));
    // const vec2 texCoords = gl_FragCoord.xy * outportParameters.reciprocalDimensions;
    
    const int bin = getBin(texCoords, _nBins);
    const int dim = getDimension(texCoords, _nDimensions);

    const int value = getValue(bin, dim);

    FragData0 = vec4(
        getBinNormalized(bin),
        getDimensionNormalized(dim),
        0.0,
        1.0
    );


    FragData0 = vec4(
        vec3(value / 3001.0),
        1.0
    );


    // FragData0 = vec4(1.0, 0.0, 0.0, 1.0);


    // FragData0 = vec4(texCoords * 2, 0.0, 1.0);
    // FragData0 = vec4(outportParameters.dimensions, 0.0, 1.0);

    // FragData0 = vec4(vec3(value / 100000000.0), 1.0);
    // FragData0 = vec4(vec3(float(bin)), 1.0);
    // FragData0 = vec4(1.0, 0.0, 0.0, 1.0);
    // FragData0 = vec4(1.0);
}
