#include <modules/pcclustering/processors/pcpreader.h>

//#include "volumesource.h"
//#include <inviwo/core/resources/resourcemanager.h>
//#include <inviwo/core/resources/templateresource.h>
//#include <inviwo/core/common/inviwoapplication.h>
//#include <inviwo/core/util/filesystem.h>
//#include <inviwo/core/util/raiiutils.h>
//#include <inviwo/core/io/datareaderfactory.h>
//#include <inviwo/core/io/rawvolumereader.h>
//#include <inviwo/core/network/processornetwork.h>
//#include <inviwo/core/datastructures/volume/volumeram.h>
//#include <inviwo/core/common/inviwoapplication.h>
//#include <inviwo/core/io/datareaderexception.h>
//
//#include <math.h>

namespace inviwo {

const ProcessorInfo PCPReader::processorInfo_{
    "pers.bock.PCPReader",  // Class identifier
    "PCP Data Reader",            // Display name
    "Data Input",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo PCPReader::getProcessorInfo() const {
    return processorInfo_;
}

PCPReader::PCPReader()
    : Processor()
    , _outport("data")
    , _file("filename", "File")
    , _reload("reload", "Reload data")
{
    auto l = [this]() { load(); };
    _file.onChange(l);
    _reload.onChange(l);

    addPort(_outport);

    addProperty(_file);
    addProperty(_reload);
}

void PCPReader::load() {
    std::string fileName = _file;
    std::ifstream file(fileName);

    ParallelCoordinatesPlotRawData* data = new ParallelCoordinatesPlotRawData;

    int dimensions;
    {
        std::string line;
        std::getline(file, line);
        std::stringstream s(line);
        s >> dimensions;
    }


    for (int i = 0; i < dimensions; ++i)
        data->minMax.emplace_back(std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    int i = 0;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream s(line);
        for (int j = 0; j < dimensions; ++j) {
            float value;
            s >> value;
            data->data.push_back(value);

            data->minMax[j].first = std::min(data->minMax[j].first, value);
            data->minMax[j].second = std::max(data->minMax[j].second, value);
        }
        ++i;
    }

    LogInfo("Dataset information: " << fileName);
    for (int i = 0; i < dimensions; ++i) {
        LogInfo("Min/Max (dimension " << i << "): " << data->minMax[i].first << " / " << data->minMax[i].second);
    }

    _outport.setData(data);
}

void PCPReader::process() {}

}  // namespace

