#ifndef IVW_PCPREADER_H
#define IVW_PCPREADER_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API PCPReader : public Processor {
public:

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    PCPReader();
    virtual ~PCPReader() = default;

    virtual void process() override;

private:
    void load();

    PCPRawDataOutport _outport;

    FileProperty _file;
    ButtonProperty _reload;

    bool isDeserializing_;
};

}  // namespace

#endif  // IVW_PCPREADER_H
