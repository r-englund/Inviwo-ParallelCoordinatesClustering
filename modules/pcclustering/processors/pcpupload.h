#ifndef IVW_PCPUPLOAD_H
#define IVW_PCPUPLOAD_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API PCPUpload: public Processor {
public:

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    PCPUpload();
    virtual ~PCPUpload() = default;

    virtual void process() override;

private:
    PCPRawDataInport _inport;
    PCPDataOutport _outport;

    std::shared_ptr<ParallelCoordinatesPlotData> _data;
};

}  // namespace

#endif  // IVW_PCPUPLOAD_H
