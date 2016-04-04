layout(location = 0) in vec2 in_position;

// layout (std430, binding = 0) readonly buffer Values {
//     float data[];
// } values;

layout (std430, binding = 2) readonly buffer Identifiers {
    int nClusters;
    int data[];
} identifiers;

layout (std430, binding = 3) readonly buffer DimensionOrdering {
    int data[];
} dimensionOrdering;

uniform int _nDimensions;
uniform float _verticalBorder;
uniform bool _hasColoringData;

out flat int identifier;
out flat int nClusters;

void main() {
    // gl_VertexID = dimension
    // gl_InstanceID = data item

    // Rearrange order of the axes
    // const int dim = dimensionOrdering.data[gl_VertexID];

    // const float value = values.data[gl_InstanceID * _nDimensions + dim];
    // const float xPosition = in_position;
    // const float yPosition = (value + _verticalBorder) / (1.0 + _verticalBorder);
    // const float yPosition = 
    // const float yPosition = (value + _verticalBorder) * (1.0 - _verticalBorder); 
    
    gl_Position = vec4(in_position, 0.0, 1.0);

    if (_hasColoringData) {
        identifier = identifiers.data[gl_InstanceID];
        nClusters = identifiers.nClusters;
    }
}
