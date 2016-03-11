layout(location = 0) in float in_position;

layout (std430, binding = 0) readonly buffer Values {
    float data[];
} values;

uniform int _nDimensions;
uniform float _verticalBorder;

void main() {
    // gl_VertexID = dimension
    // gl_InstanceID = data item
    const float value = values.data[gl_InstanceID * _nDimensions + gl_VertexID];
    const float xPosition = in_position;
    const float yPosition = value * (1.0 - _verticalBorder); 
    
    gl_Position = vec4(xPosition, yPosition, 0.0, 1.0);
}
