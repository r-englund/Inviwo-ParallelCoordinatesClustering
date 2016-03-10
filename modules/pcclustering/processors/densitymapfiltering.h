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
    void filterBins(const BinningData* inData, BinningData* outData);
    void filterBinsPercentage(const BinningData* inData, BinningData* outData);
    void filterBinsTopology(const BinningData* inData, BinningData* outData);

    void recreateBuffers();

    BinningDataInport _binInport;
    PCPDataInport _pcpInport;
    
    BinningDataOutport _binOutport;

    OptionPropertyInt _filteringMethod;

    // Percentage
    FloatProperty _percentage;

    // Topology
    IntProperty _nClusters;

    Shader _percentageFiltering;
    Shader _topologyFiltering;

    std::shared_ptr<BinningData> _binningData;
};

}  // namespace

#endif  // IVW_DENSITYMAPGENERATOR_H
