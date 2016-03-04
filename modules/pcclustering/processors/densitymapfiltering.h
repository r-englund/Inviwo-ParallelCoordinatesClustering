#ifndef IVW_DENSITYMAPFILTERING_H
#define IVW_DENSITYMAPFILTERING_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

#include <inviwo/core/ports/imageport.h>

#include <modules/opengl/shader/shader.h>


namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API DensityMapFiltering : public Processor {
public:
    const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    DensityMapFiltering();
    ~DensityMapFiltering();

    void process() override;

private:
    BinningDataInport _inport;
    BinningDataOutport _outport;

    OptionPropertyInt _filteringMethod;

    // Percentage
    FloatProperty _percentage;

    //Shader _shader;
    //GLuint _vao;
};

}  // namespace

#endif  // IVW_DENSITYMAPGENERATOR_H
