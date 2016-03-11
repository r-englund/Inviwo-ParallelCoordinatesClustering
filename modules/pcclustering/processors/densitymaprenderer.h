#ifndef IVW_DENSITYMAPRENDERER_H
#define IVW_DENSITYMAPRENDERER_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

#include <inviwo/core/ports/imageport.h>

#include <modules/opengl/shader/shader.h>


namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API DensityMapRenderer : public Processor {
public:
    const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    DensityMapRenderer();
    ~DensityMapRenderer();

    void process() override;

private:
    BinningDataInport _inport;

    ImageOutport _outport;

    FloatProperty _renderScaling;

    Shader _shader;
};

}  // namespace

#endif  // IVW_PCPRENDERER_H
