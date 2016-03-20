#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <modules/fontrendering/textrenderer.h>

#include <inviwo/core/util/glm.h>

void renderTextOverlay(
    inviwo::TextRenderer& renderer,
    const inviwo::vec2& dimension,
    const std::vector<int>& dimensionOrdering,
    const std::vector<int>& dimensionsEnabled
    );

#endif // __SUPPORT_H__