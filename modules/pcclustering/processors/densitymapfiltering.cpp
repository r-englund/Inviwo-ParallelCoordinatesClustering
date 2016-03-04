#include <modules/pcclustering/processors/densitymapfiltering.h>

#include <modules/opengl/texture/textureutils.h>

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

namespace {
    const int FilteringMethodOptionPercentage = 0;
    const int FilteringMethodOptionTopological = 1;
}

namespace inviwo {

const ProcessorInfo DensityMapFiltering::processorInfo_{
    "pers.bock.DensityMapFiltering",  // Class identifier
    "Density Map Filtering",            // Display name
    "Data Filtering",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo DensityMapFiltering::getProcessorInfo() const {
    return processorInfo_;
}

DensityMapFiltering::DensityMapFiltering()
    : Processor()
    , _inport("in.data")
    , _outport("out.data")
    , _filteringMethod("_filteringMethod", "Filtering Method")
    , _percentage("_percentage", "Percentage")
    //, _nBins("_nBins", "# Bins", 32, 1, 2048)
    //, _shader({{ShaderType::Compute, "densitymapgenerator.comp" }}, Shader::Build::No)
{
    //glGenVertexArrays(1, &_vao);

    addPort(_inport);
    addPort(_outport);

    _filteringMethod.addOption("percentage", "Percentage", FilteringMethodOptionPercentage);
    _filteringMethod.addOption("topological", "Topological", FilteringMethodOptionTopological);
    addProperty(_filteringMethod);

    addProperty(_percentage);


    //addProperty(_nBins);

    //_shader.getShaderObject(ShaderType::Compute)->addShaderExtension(
    //    "GL_ARB_compute_variable_group_size",
    //    true
    //);

    //_shader.build();

    //_shader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapFiltering::~DensityMapFiltering() {
    //glDeleteVertexArrays(1, &_vao);
}

void DensityMapFiltering::process() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const BinningData> inData = _inport.getData();

    BinningData* outData = new BinningData;
    outData->nBins = inData->nBins;
    outData->nDimensions = inData->nDimensions;
    glGenBuffers(1, &outData->ssboBins);

    std::vector<int> values(outData->nBins * outData->nDimensions);
    if (_filteringMethod.get() == FilteringMethodOptionPercentage) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, inData->ssboBins);
        int* mappedData = reinterpret_cast<int*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));

        for (int i = 0; i < outData->nDimensions; ++i) {
            // First find min-max values
            auto minMax = std::minmax_element(
                mappedData + i * outData->nBins,
                mappedData + i * outData->nBins + outData->nBins

            );

            // Then do a binary selection of the values
            float percentage = _percentage;
            std::transform(
                mappedData + i * outData->nBins,
                mappedData + i * outData->nBins + outData->nBins,
                values.begin() + i * outData->nBins,
                [minMax, percentage](int v) {
                    float normalized = static_cast<float>(v - *minMax.first) / static_cast<float>(*minMax.second - *minMax.first);
                    return normalized >= percentage ? 1 : 0;
                }
            );
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    else {

    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboBins);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        values.size() * sizeof(int),
        values.data(),
        GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    _outport.setData(outData);
}


}  // namespace

