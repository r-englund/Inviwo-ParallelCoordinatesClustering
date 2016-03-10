layout(location = 0) in float in_position;

layout (std430, binding = 0) readonly buffer Values {
    float data[];
} values;

layout (std430, binding = 1) readonly buffer MinMaxValues {
    // min/max .. min/max .. min/max .....
    float values[];
} minMaxValues;

uniform int _nDimensions;
uniform float _horizontalBorder;
uniform float _verticalBorder;

// struct Values {
// 	uint16_t value[10];
// };

// layout (std430, binding=2) buffer shaderData {
// 	float minimumValues[10];
// 	float maximumValues[10];
//     Values values[];
// };

// uniform float _minValue;
// uniform float _maxValue;

float dimensionLocation(int dimension) {
    //  -1  -1+border    1-border     1
    //  |        |           |        |
    //
    // dimension(0)   -> (-1+border)
    // dimension(max) -> (1-border)
    //
    const float minValue = -1 + _horizontalBorder;
    const float maxValue =  1 - _horizontalBorder;

    const float dim = dimension / (_nDimensions - 1.0);

    return minValue * (1.0 - dim) + maxValue * dim;
}

vec2 minMaxValue(int dimension) {
    vec2 result;
    result.x = minMaxValues.values[dimension * 2];
    result.y = minMaxValues.values[dimension * 2 + 1];
    return result;
}

float valueLocation(float value, vec2 minMax) {
    const float minValue = -1 + _verticalBorder;
    const float maxValue =  1 - _verticalBorder;

    // Incorporate minMax
    const float v = (value - minMax.x) / (minMax.y - minMax.x);

    return minValue * (1.0 - v) + maxValue * v;
}

void main() {
    int dimension = gl_VertexID;

    float inp = values.data[gl_InstanceID * _nDimensions + gl_VertexID];
    
    // float location = dimensionLocation(dimension);
    float location = in_position;
    
    vec2 minMax = minMaxValue(dimension);
    float value = valueLocation(inp, minMax);
    

    vec2 pos = vec2(location, value);

    // pos = in_position;

    gl_Position = vec4(pos, 0.0, 1.0);

// gl_VertexID
    // gl_Position = vec4(in_position, 0.0, 0.0, 1.0);
    // gl_Position = vec4(0.5, in_position, 0.0, 1.0);

    // gl_Position = vec4(0.5, 0.5, 0.0, 1.0);
}