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
    , _dimensions("dimensions", "Dimensions", 5, 1, 20)
    , _reload("reload", "Reload data")
{
    auto l = [this]() { load(); };
    _file.onChange(l);
    _reload.onChange(l);

    addPort(_outport);

    addProperty(_file);
    addProperty(_dimensions);
    addProperty(_reload);
}

void PCPReader::load() {
    std::string fileName = _file;

    std::ifstream file(fileName);

    ParallelCoordinatesPlotRawData* data = new ParallelCoordinatesPlotRawData;

    for (int i = 0; i < _dimensions; ++i)
        data->minMax.emplace_back(std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    std::string line;
    int i = 0;
    while (std::getline(file, line)) {
        std::stringstream s(line);

        //data->data.push_back(std::vector<float>(_dimensions));
        for (int j = 0; j < _dimensions; ++j) {
            float value;
            s >> value;
            data->data.push_back(value);
            //data->data[i * _dimensions + j] = value;
            //data->data[i][j] = value;

            data->minMax[j].first = std::min(data->minMax[j].first, value);
            data->minMax[j].second = std::max(data->minMax[j].second, value);
        }
        ++i;
    }

    LogInfo("Min/Max");
    for (int i = 0; i < _dimensions; ++i)
        LogInfo(i << ": " << data->minMax[i].first << " " << data->minMax[i].second);

    //for (int i = 0; i < data->data.size() / _dimensions; ++i) {
    //    for (int j = 0; j < _dimensions; ++j) {
    //        data->data[i * _dimensions + j] = (data->data[i * _dimensions + j] - data->minMax[j].first) / (data->minMax[j].second - data->minMax[j].first);;
    //        //data->data[i][j] = (data->data[i][j] - data->minMax[j].first) / (data->minMax[j].second - data->minMax[j].first);
    //    }
    //}

    _outport.setData(data);
}

void PCPReader::process() {
    //if (!isDeserializing_ && volumes_ && !volumes_->empty()) {
    //    size_t index =
    //        std::min(volumes_->size() - 1, static_cast<size_t>(volumeSequence_.index_.get() - 1));

    //    if (!(*volumes_)[index]) return;

    //    basis_.updateEntity(*(*volumes_)[index]);
    //    information_.updateVolume(*(*volumes_)[index]);

    //    outport_.setData((*volumes_)[index]);
    //}
}

//void VolumeSource::deserialize(Deserializer& d) {
//    {
//        isDeserializing_ = true;
//        Processor::deserialize(d);
//        addFileNameFilters();
//        isDeserializing_ = false;
//    }
//    load(true);
//}

}  // namespace

