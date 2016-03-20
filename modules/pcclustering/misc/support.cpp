#include <modules/pcclustering/misc/support.h>

void renderTextOverlay(
    inviwo::TextRenderer& renderer,
    const inviwo::vec2& dimension,
    const std::vector<int>& dimensionOrdering,
    std::bitset<32> dimensionMask)
{
    using namespace inviwo;

    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vec2 scale(2.f / dimension);

    const int FontSize = 20;
    renderer.setFontSize(FontSize);

    const float yPos = -0.975f;
    for (int i = 0; i < dimensionOrdering.size(); ++i) {
        float t = float(i) / float(dimensionOrdering.size());
        float xPos = (t - 0.5f) * 2.f;

        float offset = 2.f / float(dimensionOrdering.size());
        
        xPos += offset / 2.f;

        std::string text({ char(dimensionOrdering[i] + 'A') });

        vec4 color = vec4(1.0);
        if (!dimensionMask.test(i))
            color.rgb = vec3(0.35f);

        renderer.render(text.c_str(), xPos, yPos, scale, color);
    }

    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
}