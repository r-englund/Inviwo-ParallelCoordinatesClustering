#ifndef IVW_PCPGUI_H
#define IVW_PCPGUI_H

#include <inviwo/core/processors/processor.h>

#include <modules/pcclustering/pcclusteringmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <modules/pcclustering/datastructures/pcpdata.h>

namespace inviwo {

class IVW_MODULE_PCCLUSTERING_API PCPGui: public Processor {
public:

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
    PCPGui();
    virtual ~PCPGui() = default;

    virtual void process() override;

private:
    PCPDataInport _inport;

    IntProperty _nDimensions;

    IntProperty _nSubcluster;

    OptionPropertyInt _enabledDimensionsOptions;
    StringProperty _enabledDimensionsString;
    
    OptionPropertyInt _dimensionOrderingOptions;
    StringProperty _dimensionOrderingString;

    bool _isUpdating;
};

}  // namespace

#endif  // IVW_PCPGUI_H
