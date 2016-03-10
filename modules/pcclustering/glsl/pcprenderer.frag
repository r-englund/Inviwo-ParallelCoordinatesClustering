uniform int _nData;

void main() {
    float alpha = 250.0 / float(_nData);

    alpha = min(alpha, 0.25);
    FragData0 = vec4(1.0, 1.0, 1.0, alpha);
}
