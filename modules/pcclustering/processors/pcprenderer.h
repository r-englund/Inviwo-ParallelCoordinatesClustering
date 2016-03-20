#ifndef IVW_PCPRENDERER_H
#define IVW_PCPRENDERER_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/stringproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

#include <inviwo/core/ports/imageport.h>

#include <modules/opengl/shader/shader.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <modules/fontrendering/fontrenderingmoduledefine.h>
#include <modules/fontrendering/textrenderer.h>

namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API PCPRenderer : public Processor {
public:
    const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    PCPRenderer();
    ~PCPRenderer();

    void process() override;

private:
    void invalidateBuffer();
    void renderParallelCoordinates();

    PCPDataInport _inport;
    ColoringDataInport _coloringData;

    ImageOutport _outport;

    FloatProperty _horizontalBorder;
    FloatProperty _verticalBorder;
    StringProperty _dimensionOrderingString;

    std::vector<int> _dimensionOrdering;

    TransferFunctionProperty _transFunc;

    TextRenderer _textRenderer;
    
    Shader _shader;
    GLuint _vao;
    GLuint _vbo;

    GLuint _dimensionOrderingBuffer;
};

}  // namespace

#endif  // IVW_PCPRENDERER_H
