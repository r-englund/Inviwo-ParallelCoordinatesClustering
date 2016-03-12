in flat int identifier;

uniform int _nData;
uniform bool _hasColoringData;

void main() {
    float alpha = 250.0 / float(_nData);

    alpha = clamp(alpha, 0.05, 0.1);

    if (_hasColoringData)
        FragData0 = vec4(float(identifier) / 3.0, 0.0, 0.0, alpha);
    else
        FragData0 = vec4(1.0, 1.0, 1.0, alpha);

}
