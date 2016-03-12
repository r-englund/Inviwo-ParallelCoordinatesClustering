#ifndef IVW_CLUSTEREXPORT_H
#define IVW_CLUSTEREXPORT_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API ClusterExport : public Processor {
public:

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    ClusterExport();

    virtual void process() override;

private:
    void exportClusters();

    PCPRawDataInport _rawDataInport;
    PCPDataInport _dataInport;
    ColoringDataInport _clusterInport;

    FileProperty _outputFile;
    ButtonProperty _export;
};

}  // namespace

#endif  // IVW_CLUSTEREXPORT_H
