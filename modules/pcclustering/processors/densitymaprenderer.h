#ifndef IVW_DENSITYMAPRENDERER_H
#define IVW_DENSITYMAPRENDERER_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <inviwo/core/ports/imageport.h>

#include <modules/opengl/shader/shader.h>

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <modules/fontrendering/textrenderer.h>

#include <bitset>

namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API DensityMapRenderer : public Processor {
public:
    const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    DensityMapRenderer();
    ~DensityMapRenderer();

    void process() override;

private:
    void generateBuffers();

    void renderDensityMap();
    
    BinningDataInport _inport;
    ColoredBinDataInport _colorInport;

    StringProperty _dimensionOrderingString;
    std::vector<int> _dimensionOrdering;

    StringProperty _dimensionMaskString;
    std::bitset<32> _dimensionMask;


    ImageOutport _outport;

    TransferFunctionProperty _transFunc;
    FloatProperty _textBorder;

    Shader _shader;

    TextRenderer _textRenderer;

    GLuint _dimensionOrderingBuffer;
};

}  // namespace

#endif  // IVW_PCPRENDERER_H
