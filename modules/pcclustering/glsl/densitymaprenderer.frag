#include "utils/structs.glsl"

layout (std430, binding = 0) readonly buffer BinningValues {
    int values[];
} binning;

layout (std430, binding = 1) readonly buffer MinMax {
    int values[];
} minMax;

layout (std430, binding = 2) readonly buffer Identifiers {
    int nClusters;
    int data[];
} identifiers;

uniform int _nBins;
uniform int _nDimensions;

uniform bool _hasColoringData;
uniform int _selectedDimension;
uniform sampler2D _transFunc;

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
    return binning.values[dimension * _nBins + bin];
    // return binning.values[bin * _nDimensions + dimension];
}

ivec2 getMinMax(int dimension) {
    return ivec2(
        minMax.values[dimension * 2],
        minMax.values[dimension * 2 + 1]
    );
}

void main() {
    const vec2 texCoords = gl_FragCoord.xy * (1.f / vec2(_outportSize));
    
    const int bin = getBin(texCoords, _nBins);
    const int dim = getDimension(texCoords, _nDimensions);

    const int value = getValue(bin, dim);

    const ivec2 minMaxValue = getMinMax(dim);
    const float normalizedValue = (float(value) - float(minMaxValue.x)) / (float(minMaxValue.y) - float(minMaxValue.x));

    vec4 c = vec4(vec3(normalizedValue), 0.2);

    if (_hasColoringData && (dim == _selectedDimension)) {
        int identifier = identifiers.data[bin];
        vec4 color = texture(_transFunc, vec2(float(identifier) / float(identifiers.nClusters), 0.5));
        FragData0 = color * c;
    }
    else {
        FragData0 = c;
    }

    // FragData0 = vec4(1.0, 0.0, 0.0, 1.0);


    // FragData0 = vec4(texCoords * 2, 0.0, 1.0);
    // FragData0 = vec4(outportParameters.dimensions, 0.0, 1.0);

    // FragData0 = vec4(vec3(value / 100000000.0), 1.0);
    // FragData0 = vec4(vec3(float(bin)), 1.0);
    // FragData0 = vec4(1.0, 0.0, 0.0, 1.0);
    // FragData0 = vec4(1.0);
}
