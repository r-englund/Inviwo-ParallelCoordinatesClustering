layout(location = 0) in float in_position;

layout (std430, binding = 0) readonly buffer Values {
    float data[];
} values;

layout (std430, binding = 1) readonly buffer Identifiers {
    int nClusters;
    int data[];
} identifiers;

uniform int _nDimensions;
uniform bool _hasColoringData;

out flat int identifier;
out flat int nClusters;

out vec3 debug;

const float PI = 3.1415926539;
// const float PI = 180.0;

void main() {
    // gl_VertexID = dimension
    // gl_InstanceID = data item
    // Thank you, past me, for this comment

    // Values in polar coordinates
    float value = (values.data[gl_InstanceID * _nDimensions + gl_VertexID] + 1.0) / 2.0;
    float angle = ((2 * PI) / _nDimensions) * gl_VertexID;

    float xPosition = value * cos(angle);
    float yPosition = value * sin(angle);

    gl_Position = vec4(xPosition, yPosition, 0.0, 1.0);

    if (_hasColoringData) {
        identifier = identifiers.data[gl_InstanceID];
        nClusters = identifiers.nClusters;
    }

    switch (gl_VertexID) {
        case 0:
            debug = vec3(1.0, 0.0, 0.0);
            break;
        case 1:
            debug = vec3(0.0, 1.0, 0.0);
            break;
        case 2:
            debug = vec3(0.0, 0.0, 1.0);
            break;
        case 3:
            debug = vec3(1.0, 1.0, 0.0);
            break;
        case 4:
            debug = vec3(1.0, 0.0, 1.0);
            break;
    }

    // debug = vec3(gl_VertexID / 5.0, gl_VertexID / 5.0, gl_VertexID / 5.0);
}
