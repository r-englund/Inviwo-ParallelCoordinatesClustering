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
    , _countingShader({{ShaderType::Compute, "densitymapfiltering_counting.comp" }}, Shader::Build::No)
    , _filteringShader({ { ShaderType::Compute, "densitymapfiltering_filtering.comp" } }, Shader::Build::No)
{
    addPort(_binInport);
    addPort(_pcpInport);

    addPort(_binOutport);
    addPort(_pcpOutport);

    _filteringMethod.addOption("percentage", "Percentage", FilteringMethodOptionPercentage);
    _filteringMethod.addOption("topological", "Topological", FilteringMethodOptionTopological);
    addProperty(_filteringMethod);

    addProperty(_percentage);


    _countingShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );

    _filteringShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );


    _countingShader.build();
    _filteringShader.build();

    _countingShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
    _filteringShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
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
    glGenBuffers(1, &outData->ssboBins);

    LGL_ERROR;
    filterBins(inData.get(), outData);
    LGL_ERROR;
    _binOutport.setData(outData);

    if (!_pcpInport.hasData())
        return;


    std::shared_ptr<const ParallelCoordinatesPlotData> pcpInData = _pcpInport.getData();

    ParallelCoordinatesPlotData* pcpOutData = new ParallelCoordinatesPlotData;
    pcpOutData->nDimensions = pcpInData->nDimensions;
    //pcpOutData->ssboMinMax = pcpInData->ssboMinMax;
    LGL_ERROR;
    glGenBuffers(1, &pcpOutData->ssboData);
    glGenBuffers(1, &pcpOutData->ssboMinMax);

    glBindBuffer(GL_COPY_WRITE_BUFFER, pcpOutData->ssboMinMax);
    glBufferData(
        GL_COPY_WRITE_BUFFER,
        pcpInData->nDimensions * 2 * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_COPY_READ_BUFFER, pcpInData->ssboMinMax);
    glCopyBufferSubData(
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        0,
        0,
        pcpInData->nDimensions * 2 * sizeof(float)
        );
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    LGL_ERROR;

    filterData(pcpInData.get(), outData, pcpOutData);
    LGL_ERROR;


    _pcpOutport.setData(pcpOutData);


    // Temp: Compare values
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, pcpInData->ssboData);
    //float* mappedDataIn = reinterpret_cast<float*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));

    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, pcpOutData->ssboData);
    //float* mappedDataOut = reinterpret_cast<float*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY));

    //for (int i = 0; i < pcpOutData->nDimensions * pcpOutData->nValues; ++i) {
    //    if (mappedDataIn[i] != mappedDataOut[i])
    //        LogInfo(i);
    //}
    LGL_ERROR;

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

void DensityMapFiltering::filterData(const ParallelCoordinatesPlotData* inData,
                               BinningData* binData, ParallelCoordinatesPlotData* outData)
{
    // First count the number of data values that will be written
    _countingShader.activate();
    _countingShader.setUniform("_nDimensions", inData->nDimensions);
    _countingShader.setUniform("_nBins", binData->nBins);

    // Storage for number of data values (not one value per dimension)
    GLuint nValuesCounter = 0;
    glGenBuffers(1, &nValuesCounter);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, nValuesCounter);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, nValuesCounter);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inData->ssboMinMax);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, inData->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, binData->ssboBins);

    LGL_ERROR;
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        inData->nValues / inData->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    LGL_ERROR;

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, nValuesCounter);
    int* ptr = (int*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_WRITE);
    int nDataValues = ptr[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    LGL_ERROR;

    glDeleteBuffers(1, &nValuesCounter);
    LGL_ERROR;

    LogInfo(nDataValues);

    outData->nValues = nDataValues * outData->nDimensions;
    _countingShader.deactivate();


    // Then actually write out the number of values
    _filteringShader.activate();
    _filteringShader.setUniform("_nDimensions", inData->nDimensions);
    _filteringShader.setUniform("_nBins", binData->nBins);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboData);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        outData->nValues * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    LGL_ERROR;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    LGL_ERROR;

    //GLuint memoryBarrier = 0;
    //glGenBuffers(1, &memoryBarrier);
    //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, memoryBarrier);
    //glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    //LGL_ERROR;

    //glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, memoryBarrier);
    //LGL_ERROR;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inData->ssboMinMax);
    LGL_ERROR;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, inData->ssboData);
    LGL_ERROR;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, binData->ssboBins);
    LGL_ERROR;
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, outData->ssboData);
    LGL_ERROR;
    
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        inData->nValues / inData->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    LGL_ERROR;

    _filteringShader.deactivate();
    LGL_ERROR;
}

}  // namespace

