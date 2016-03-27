#include <modules/pcclustering/processors/pcpfileconverter.h>

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

const ProcessorInfo PCPFileConverter::processorInfo_{
    "pers.bock.PCPFileConverter",  // Class identifier
    "PCP File Converter",            // Display name
    "Data Input",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo PCPFileConverter::getProcessorInfo() const {
    return processorInfo_;
}

PCPFileConverter::PCPFileConverter()
    : Processor()
    , _inputFile("in.filename", "Input File")
    , _outputFile("out.filename", "Output File")
    , _convert("convert", "Convert data")
{
    _convert.onChange([this]() { load(); });

    addProperty(_inputFile);
    addProperty(_outputFile);
    addProperty(_convert);
}

void PCPFileConverter::load() {
    std::string inFileName = _inputFile;
    std::string outFileName = _outputFile;

    std::ifstream inFile(inFileName);
    std::ofstream outFile(outFileName, std::ofstream::binary);

    if (!(inFile.good() && outFile.good())) {
        LogError("Error opening file");
        return;
    }

    LogInfo("Loading dimensions");
    int8_t dimensions;
    {
        std::string line;
        std::getline(inFile, line);
        std::stringstream s(line);
        int d;
        s >> d;
        dimensions = static_cast<int8_t>(d);
    }
    LogInfo("Dimensions: " << dimensions);

    int32_t nDataItems;
    {
        std::string line;
        std::getline(inFile, line);
        std::stringstream s(line);
        int d;
        s >> d;
        nDataItems = static_cast<int32_t>(d);
    }
    LogInfo("Number of Data Items: " << int(nDataItems));

    LogInfo("Creating Min/Max structure");
    std::vector<std::pair<float, float>> minMax;
    for (int i = 0; i < dimensions; ++i)
        minMax.emplace_back(std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    LogInfo("Creating data structure");
    std::vector<float> data;
    data.reserve(nDataItems * dimensions);
    int i = 0;
    std::string line;
    LogInfo("Start loading data");
    std::stringstream s;
    while (std::getline(inFile, line)) {
        s.clear();
        s.str(line);
        for (int j = 0; j < dimensions; ++j) {
            float value;
            s >> value;
            data.push_back(value);

            minMax[j].first = std::min(minMax[j].first, value);
            minMax[j].second = std::max(minMax[j].second, value);
        }
        ++i;
    }
    LogInfo("Finished loading data with " << data.size() << " items");

    LogInfo("Copying Min/Max data");
    std::vector<float> minMaxValues(minMax.size() * 2);
    for (int i = 0; i < dimensions; ++i) {
        minMaxValues[i * 2] = minMax[i].first;
        minMaxValues[i * 2 + 1] = minMax[i].second;
    }

    int64_t nValues = data.size();

    LogInfo("Writing dimensions");
    outFile.write(reinterpret_cast<const char*>(&dimensions), sizeof(int8_t));
    LogInfo("Writing Min/Max values");
    outFile.write(reinterpret_cast<const char*>(minMaxValues.data()), dimensions * 2 * sizeof(float));
    LogInfo("Writing number of values");
    outFile.write(reinterpret_cast<const char*>(&nValues), sizeof(int64_t));
    LogInfo("Writing values");
    outFile.write(reinterpret_cast<const char*>(data.data()), nValues * sizeof(float));
}

void PCPFileConverter::process() {}

}  // namespace

