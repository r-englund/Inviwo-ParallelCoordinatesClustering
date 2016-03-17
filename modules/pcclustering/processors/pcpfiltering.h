#ifndef IVW_PCPFILTERING_H
#define IVW_PCPFILTERING_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/stringproperty.h>

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
    int countElements(const ParallelCoordinatesPlotData* inData,
        const BinningData* data);

    void clusterDetection(const BinningData* data);

    void filterData(const ParallelCoordinatesPlotData* inData, 
        const BinningData* data, ParallelCoordinatesPlotData* outData, int nElements);

    BinningDataInport _binInport;
    PCPDataInport _pcpInport;
    
    PCPDataOutport _pcpOutport;
    ColoringDataOutport _coloringOutport;
    ColoredBinDataOutport _coloredBinOutport;

    IntProperty _coloringDimension;
    StringProperty _dimensionMaskString;
    uint32_t _dimensionMask;

    Shader _countingShader;
    Shader _clusterDetectionShader;
    Shader _filteringShader;

    std::shared_ptr<ParallelCoordinatesPlotData> _pcpData;
    std::shared_ptr<ColoringData> _coloringData;
    std::shared_ptr<ColoredBinData> _coloredBinData;

    GLuint _nValuesCounter;
    GLuint _memoryAccess;
    //GLuint _nClustersBuffer;
};

}  // namespace

#endif  // IVW_PCPFILTERING_H
