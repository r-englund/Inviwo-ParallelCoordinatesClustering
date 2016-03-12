#ifndef IVW_CLUSTERRENDERER_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

#include <inviwo/core/ports/imageport.h>

#include <modules/opengl/shader/shader.h>


namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API ClusterRenderer : public Processor {
public:
    const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    ClusterRenderer();
    ~ClusterRenderer();

    void process() override;

private:
    ColoredBinDataInport _inport;
    ImageOutport _outport;

    TransferFunctionProperty _transFunc;

    Shader _shader;
};

}  // namespace

#endif  // IVW_CLUSTERRENDERER_H


