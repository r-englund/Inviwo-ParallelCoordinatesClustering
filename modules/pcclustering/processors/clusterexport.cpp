#include <modules/pcclustering/processors/clusterexport.h>

#include <modules/opengl/openglutils.h>

//#include <inviwo/core/resources/resourcemanager.h>
//#include <inviwo/core/resources/templateresource.h>
//#include <inviwo/core/common/inviwoapplication.h>
//#include <inviwo/core/util/filesystem.h>
//#include <inviwo/core/io/datareaderfactory.h>
//#include <inviwo/core/io/rawvolumereader.h>
//#include <inviwo/core/network/processornetwork.h>
//#include <inviwo/core/datastructures/volume/volumeram.h>
//#include <inviwo/core/common/inviwoapplication.h>
//#include <inviwo/core/io/datareaderexception.h>
//
//#include <math.h>

namespace inviwo {

const ProcessorInfo ClusterExport::processorInfo_{
    "pers.bock.ClusterExport",  // Class identifier
    "Cluster Export",            // Display name
    "Data Output",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo ClusterExport::getProcessorInfo() const {
    return processorInfo_;
}

ClusterExport::ClusterExport()
    : Processor()
    , _rawDataInport("in_raw")
    , _dataInport("in_data")
    , _clusterInport("in_cluster")
    , _outputFile("_outputFile", "Output File")
    , _export("_export", "Export")
{
    addPort(_rawDataInport);
    addPort(_dataInport);
    addPort(_clusterInport);

    addProperty(_outputFile);
    addProperty(_export);

    _export.onChange([this]() { exportClusters(); });
}

void ClusterExport::exportClusters() {
    bool rawDataExists = _rawDataInport.hasData();
    bool dataExists = _dataInport.hasData();
    bool clustersExists = _clusterInport.hasData() && _clusterInport.getData()->hasData;

    if (!rawDataExists || !dataExists || !clustersExists) {
        LogError("Missing data");
        return;
    }

    std::shared_ptr<const ParallelCoordinatesPlotRawData> rawData = _rawDataInport.getData();
    std::shared_ptr<const ParallelCoordinatesPlotData> data = _dataInport.getData();
    std::shared_ptr<const ColoringData> clusterData = _clusterInport.getData();

    assert(data->nValues / data->nDimensions == clusterData->nValues);

    std::string outputFilename = _outputFile;
    std::ofstream mainFile(outputFilename);

    std::vector<std::ofstream> outputFiles(clusterData->nClusters);
    for (int i = 1; i <= clusterData->nClusters; ++i)
        outputFiles[i - 1].open(outputFilename + std::to_string(i));



    std::vector<int> clusterValues(data->nValues / data->nDimensions);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clusterData->ssboColor);
    int* clusterPtr = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    std::copy(
        clusterPtr + 1,
        clusterPtr + 1 + data->nValues / data->nDimensions,
        clusterValues.begin()
    );
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    std::vector<float> dataValues(data->nValues);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data->ssboData);
    float* dataPtr = (float*) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    std::copy(dataPtr, dataPtr + data->nValues, dataValues.begin());
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    std::vector<std::pair<float, float>> minMax = rawData->minMax;

    for (int i = 0; i < data->nValues / data->nDimensions; ++i) {
        int cluster = clusterValues[i];

        if (cluster == -1)
            cluster = cluster;
        std::ofstream& file = outputFiles[cluster - 1];

        for (int j = 0; j < data->nDimensions; ++j) {
            float value = dataValues[i * data->nDimensions + j];
            float normalized = (value + 1.f) / 2.f;
            float denormalized = minMax[j].second * normalized + minMax[j].first * (1.f - normalized);
            file << denormalized << " ";
        }
        file << std::endl;
    }
}

void ClusterExport::process() {}

}  // namespace
