#ifndef IVW_PCPFILECONVERTER_H
#define IVW_PCPFILECONVERTER_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API PCPFileConverter : public Processor {
public:

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    PCPFileConverter();
    virtual ~PCPFileConverter() = default;

    virtual void process() override;

private:
    void load();

    FileProperty _inputFile;
    FileProperty _outputFile;
    ButtonProperty _convert;

    bool isDeserializing_;
};

}  // namespace

#endif  // IVW_PCPFILECONVERTER_H
