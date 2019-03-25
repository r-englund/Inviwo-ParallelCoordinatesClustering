#ifndef IVW_RADARPLOTRENDERER_H
#define IVW_RADARPLOTRENDERER_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

#include <inviwo/core/ports/imageport.h>

#include <modules/opengl/shader/shader.h>
#include <inviwo/core/properties/transferfunctionproperty.h>


namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API RadarPlotRenderer : public Processor {
public:
    const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    RadarPlotRenderer();
    ~RadarPlotRenderer();

    void process() override;

private:
    void invalidateBuffer();

    PCPDataInport _inport;
    ColoringDataInport _coloringData;

    ImageOutport _outport;

    BoolProperty _lineSmoothing;
    FloatProperty _alphaFactor;
    ButtonProperty _invalidate;

    //OptionPropertyInt _xAxisSelection;
    //OptionPropertyInt _yAxisSelection;
    //FloatProperty _glyphSize;

    TransferFunctionProperty _transFunc;

    Shader _shader;
    Shader _linesShader;
    GLuint _vbo;
    GLuint _vao;

    GLuint _linesvao;
    GLuint _linesvbo;
};

}  // namespace

#endif  // IVW_RADARPLOTRENDERER_H
