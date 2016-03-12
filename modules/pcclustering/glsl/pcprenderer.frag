in flat int identifier;
in flat int nClusters;

uniform int _nData;
uniform bool _hasColoringData;
uniform sampler2D _transFunc;

void main() {
    float alpha = 250.0 / float(_nData);

    alpha = clamp(alpha, 0.05, 0.1);

    if (_hasColoringData) {
        vec4 color = vec4(texture(_transFunc, vec2(float(identifier) / float(nClusters), 0.5)).rgb, alpha);
        FragData0 = color;
    }
    else
        FragData0 = vec4(1.0, 1.0, 1.0, alpha);

}
