#ifndef IVW_PCPFILTERING_H
#define IVW_PCPFILTERING_H

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

class IVW_MODULE_PCCLUSTERING_API PCPFiltering : public Processor {
public:
    const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    PCPFiltering();
    ~PCPFiltering();

    void process() override;

private:
    void filterData(const ParallelCoordinatesPlotData* inData, 
        const BinningData* data, ParallelCoordinatesPlotData* outData);

    BinningDataInport _binInport;
    PCPDataInport _pcpInport;
    
    PCPDataOutport _pcpOutport;
    //PCPDataOutport _pcpOutportNegative;

    OptionPropertyInt _filteringMethod;

    Shader _countingShader;
    Shader _filteringShader;

    std::shared_ptr<ParallelCoordinatesPlotData> _pcpData;

    GLuint _nValuesCounter;
    GLuint _memoryAccess;
};

}  // namespace

#endif  // IVW_PCPFILTERING_H
