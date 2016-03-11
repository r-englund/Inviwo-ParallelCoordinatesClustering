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
    data->data.resize(nValues);
    file.read(reinterpret_cast<char*>(data->data.data()), nValues * sizeof(float));

    for (int i = 0; i < nValues / dimensions; ++i) {
        for (int j = 0; j < dimensions; ++j) {
            data->data[i * dimensions + j] =
                (data->data[i * dimensions + j] - data->minMax[j].first) / (data->minMax[j].second - data->minMax[j].first);
            data->data[i * dimensions + j] -= 0.5f;
            data->data[i * dimensions + j] *= 2.f;

        }
        ++i;
    }


    _outport.setData(data);
}

void PCPReader::process() {}

}  // namespace

