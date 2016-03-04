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
    , _binInport("in.bins")
    , _pcpInport("in.pcp")
    , _binOutport("out.bins")
    , _pcpOutport("out.pcp")
    , _filteringMethod("_filteringMethod", "Filtering Method")
    , _percentage("_percentage", "Percentage")
    , _shader({{ShaderType::Compute, "densitymapfiltering.comp" }}, Shader::Build::No)
{
    //glGenBuffers(1, &_accumulationBuffer);

    addPort(_binInport);
    addPort(_pcpInport);

    addPort(_binOutport);
    addPort(_pcpOutport);

    _filteringMethod.addOption("percentage", "Percentage", FilteringMethodOptionPercentage);
    _filteringMethod.addOption("topological", "Topological", FilteringMethodOptionTopological);
    addProperty(_filteringMethod);

    addProperty(_percentage);


    _shader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );

    _shader.build();

    _shader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapFiltering::~DensityMapFiltering() {
    //glDeleteBuffers(1, &_accumulationBuffer);
}


void DensityMapFiltering::process() {
    if (!_binInport.hasData())
        return;

    std::shared_ptr<const BinningData> inData = _binInport.getData();

    BinningData* outData = new BinningData;
    outData->nBins = inData->nBins;
    outData->nDimensions = inData->nDimensions;
    outData->ssboBins = inData->ssboBins;
    //glGenBuffers(1, &outData->ssboBins);

    filterBins(inData.get(), outData);
    _binOutport.setData(outData);



    if (!_pcpInport.hasData())
        return;


    std::shared_ptr<const ParallelCoordinatesPlotData> pcpInData = _pcpInport.getData();
    ParallelCoordinatesPlotData* pcpOutData = new ParallelCoordinatesPlotData;
    pcpOutData->nDimensions = pcpInData->nDimensions;
    pcpOutData->ssboMinMax = pcpInData->ssboMinMax;
    glGenBuffers(1, &pcpOutData->ssboData);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, pcpOutData->ssboData);
    //glBufferData(GL_SHADER_)

    //GLuint nValuesBuffer = 0;
    //glGenBuffers(1, &nValuesBuffer);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, nValuesBuffer);
    //glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), nullptr, GL_DYNAMIC_DRAW);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLuint nValuesCounter = 0;
    glGenBuffers(1, &nValuesCounter);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, nValuesCounter);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);


    _shader.activate();
    _shader.setUniform("_nDimensions", pcpInData->nDimensions);
    _shader.setUniform("_nBins", outData->nBins);
    
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, nValuesCounter);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pcpInData->ssboMinMax);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pcpInData->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, outData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pcpOutData->ssboData);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //glDispatchCompute(data->nValues / 32, 1, 1);
    glDispatchComputeGroupSizeARB(
        pcpInData->nValues / pcpInData->nDimensions, 1, 1,
        1, 1, 1
    );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    _shader.deactivate();

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, nValuesCounter);
    int* ptr = (int*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
    int nDataValues = ptr[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glDeleteBuffers(1, &nValuesCounter);

    pcpOutData->nValues = nDataValues * pcpOutData->nDimensions;


    _pcpOutport.setData(pcpOutData);


}

void DensityMapFiltering::filterBins(const BinningData* inData, BinningData* outData) {
    std::vector<int> values(outData->nBins * outData->nDimensions);
    if (_filteringMethod.get() == FilteringMethodOptionPercentage)
        filterBinsPercentage(inData, outData, values);
    else
        filterBinsTopology(inData, outData, values);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboBins);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        values.size() * sizeof(int),
        values.data(),
        GL_DYNAMIC_DRAW
        );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void DensityMapFiltering::filterBinsPercentage(
    const BinningData* inData, BinningData* outData, std::vector<int>& values)
{
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

void DensityMapFiltering::filterBinsTopology(
    const BinningData* inData, BinningData* outData, std::vector<int>& values)
{
}

}  // namespace

