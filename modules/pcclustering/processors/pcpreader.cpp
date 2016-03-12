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
    std::ifstream file(fileName, std::ifstream::binary);
    
    if (!filesystem::fileExists(fileName)) {
        LogError("Error opening file '" << fileName << "'");
        return;
    }


    ParallelCoordinatesPlotRawData* data = new ParallelCoordinatesPlotRawData;
    LogInfo("Dataset information");

    int8_t dimensions;
    file.read(reinterpret_cast<char*>(&dimensions), sizeof(int8_t));
    LogInfo("Dimensions: " << int(dimensions));

    std::vector<float> minMaxValues(dimensions * 2);
    file.read(reinterpret_cast<char*>(minMaxValues.data()), dimensions * 2 * sizeof(float));

    LogInfo("Min/Max Values:");
    for (int i = 0; i < dimensions; ++i) {
        data->minMax.push_back(std::make_pair(minMaxValues[i * 2], minMaxValues[i * 2 + 1]));
        LogInfo(i << ": " << data->minMax[i].first << " / " << data->minMax[i].second);
    }

    int64_t nValues;
    file.read(reinterpret_cast<char*>(&nValues), sizeof(int64_t));
    LogInfo("Number of total values: " << nValues);
    LogInfo("Number of data values: " << nValues / dimensions);
    data->data.resize(nValues);
    file.read(reinterpret_cast<char*>(data->data.data()), nValues * sizeof(float));

    for (int i = 0; i < nValues; ++i) {
        const int dim = i % dimensions;
        const float minValue = data->minMax[dim].first;
        const float maxValue = data->minMax[dim].second;

        float& v = data->data[i];
        v = (v - minValue) / (maxValue - minValue);
        v = (v - 0.5f) * 2.f;
    }

    _outport.setData(data);
}

void PCPReader::process() {}

}  // namespace

