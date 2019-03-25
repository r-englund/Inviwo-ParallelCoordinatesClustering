in flat int identifier;
in flat int nClusters;

in vec3 debug;


uniform int _nData;
uniform bool _hasColoringData;
uniform sampler2D _transFunc;
uniform float _alphaFactor;

void main() {
    float alpha = clamp(_alphaFactor * 1000.0 / float(_nData), 0.0, 1.0);
        alpha = clamp(alpha, 0.0, 0.25);

    if (_hasColoringData) {
        vec4 color = vec4(texture(_transFunc, vec2(float(identifier) / float(nClusters), 0.5)).rgb, alpha);
        FragData0 = color;
    }
    else {
        FragData0 = vec4(1.0, 1.0, 1.0, alpha);
    }


    // FragData0 = vec4(debug, alpha);
}
