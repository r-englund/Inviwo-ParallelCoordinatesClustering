#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <modules/fontrendering/textrenderer.h>

#include <inviwo/core/util/glm.h>

#include <bitset>

void renderTextOverlay(
    inviwo::TextRenderer& renderer,
    const inviwo::vec2& dimension,
    const std::vector<int>& dimensionOrdering,
    std::bitset<32> dimensionMask
    );

#endif // __SUPPORT_H__