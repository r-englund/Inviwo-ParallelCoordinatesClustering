#ifndef IVW_PCPUPLOADRENDERER_H
#define IVW_PCPUPLOADRENDERER_H

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

#include <bitset>

namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API PCPUploadRenderer : public Processor {
public:
    const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    PCPUploadRenderer();
    ~PCPUploadRenderer();

    void process() override;

private:
    void invalidateBuffer();
    void renderParallelCoordinates();
    void renderBackground();

    PCPRawDataInport _inport;
    ColoringDataInport _coloringData;

    ImageOutport _outport;

    FloatProperty _horizontalBorder;
    FloatProperty _verticalBorder;
    
    BoolProperty _lineSmoothing;
    BoolProperty _depthTesting;

    StringProperty _dimensionOrderingString;
    std::vector<int> _dimensionOrdering;

    StringProperty _dimensionMaskString;
    std::bitset<32> _dimensionMask;

    TransferFunctionProperty _transFunc;
    FloatProperty _alphaFactor;

    BoolProperty _enableTextRendering;

    ButtonProperty _invalidate;

    TextRenderer _textRenderer;
    
    Shader _shader;
    Shader _backgroundShader;
    GLuint _vao;
    GLuint _vbo;

    GLuint _dimensionOrderingBuffer;

    std::vector<GLsizei> _multiDrawCount;
    std::vector<GLsizei> _multiDrawIndices;
    std::vector<GLsizei> _drawElements;

};

}  // namespace

#endif  // IVW_PCPUPLOADRENDERER_H
