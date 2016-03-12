#include <modules/pcclustering/processors/pcpfiltering.h>

#include <modules/opengl/texture/textureutils.h>

namespace inviwo {

const ProcessorInfo PCPFiltering::processorInfo_{
    "pers.bock.PCPFiltering",  // Class identifier
    "PCP Filtering",            // Display name
    "Data Filtering",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo PCPFiltering::getProcessorInfo() const {
    return processorInfo_;
}

PCPFiltering::PCPFiltering()
    : Processor()
    , _binInport("in.bins")
    , _pcpInport("in.pcp")
    , _pcpOutport("out.pcp")
    , _coloringOutport("out.color")
    , _coloredBinOutport("out.coloredBin")
    , _coloringDimension("_coloringDimension", "Coloring Dimension", -1, -1, 15)
    , _countingShader({{ShaderType::Compute, "pcpfiltering_counting.comp" }}, Shader::Build::No)
    , _clusterDetectionShader({{ShaderType::Compute, "clusterdetection.comp" }}, Shader::Build::No)
    , _filteringShader({ { ShaderType::Compute, "pcpfiltering_filtering.comp" } }, Shader::Build::No)
{
    addPort(_binInport);
    addPort(_pcpInport);

    addPort(_pcpOutport);
    addPort(_coloringOutport);
    addPort(_coloredBinOutport);

    addProperty(_coloringDimension);

    _countingShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );

    _clusterDetectionShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
        );

    _filteringShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );


    _countingShader.build();
    _clusterDetectionShader.build();
    _filteringShader.build();

    _countingShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
    _clusterDetectionShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
    _filteringShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });

    _pcpData = std::make_shared<ParallelCoordinatesPlotData>();
    glGenBuffers(1, &_pcpData->ssboData);

    glGenBuffers(1, &_nValuesCounter);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _nValuesCounter);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glGenBuffers(1, &_memoryAccess);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _memoryAccess);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    _coloringData = std::make_shared<ColoringData>();
    glGenBuffers(1, &_coloringData->ssboColor);

    _coloredBinData = std::make_shared<ColoredBinData>();
    glGenBuffers(1, &_coloredBinData->ssboIndices);

    //glGenBuffers(1, &_nClustersBuffer);
}

PCPFiltering::~PCPFiltering() {
    glDeleteBuffers(1, &_nValuesCounter);
    glDeleteBuffers(1, &_memoryAccess);
    //glDeleteBuffers(1, &_nClustersBuffer);
}

void PCPFiltering::process() {
    if (!_binInport.hasData())
        return;

    if (!_pcpInport.hasData())
        return;

    std::shared_ptr<const BinningData> binInData = _binInport.getData();
    std::shared_ptr<const ParallelCoordinatesPlotData> pcpInData = _pcpInport.getData();

    _pcpData->nDimensions = pcpInData->nDimensions;

    int dataValues = countElements(pcpInData.get(), binInData.get());

    if (_coloringDimension >= 0 && _coloringDimension < _pcpData->nDimensions) {
        _pcpData->nValues = dataValues * _pcpData->nDimensions;
        _coloringData->nValues = pcpInData->nValues;
        _coloredBinData->nBins = binInData->nBins;
        _coloredBinData->selectedDimension = _coloringDimension;

        clusterDetection(binInData.get());
        _coloringData->hasData = true;
        _coloredBinData->hasData = true;
    }
    else {
        LogWarn("Skipping cluster detection of dimension: " << _coloringDimension);
        _coloringData->hasData = false;

    }
    filterData(pcpInData.get(), binInData.get(), _pcpData.get(), dataValues);

    _pcpOutport.setData(_pcpData);
    _coloringOutport.setData(_coloringData);
    _coloredBinOutport.setData(_coloredBinData);
}

int PCPFiltering::countElements(const ParallelCoordinatesPlotData* inData,
    const BinningData* binData)
{
    // First count the number of data values that will be written
    _countingShader.activate();
    _countingShader.setUniform("_nDimensions", inData->nDimensions);
    _countingShader.setUniform("_nBins", binData->nBins);

    // Storage for number of data values (not one value per dimension)
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _nValuesCounter);
    GLuint* ptr = (GLuint*)glMapBufferRange(
        GL_ATOMIC_COUNTER_BUFFER,
        0,
        sizeof(GLuint),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
        );
    ptr[0] = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, _nValuesCounter);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inData->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, binData->ssboBins);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        inData->nValues / inData->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _nValuesCounter);
    int* p = (int*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
    int nDataValuesPositive = p[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    _countingShader.deactivate();

    LogInfo("Number of accepted values: " << nDataValuesPositive);
    return nDataValuesPositive;

}

void PCPFiltering::clusterDetection(const BinningData* binData) {
    // Generate a buffer that stores for a single dimension the IDs in each bin by
    // doing a connected component analysis

    //GLuint fff;
    //glGenBuffers(1, &fff);
    //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, fff);
    //glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
    //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _coloredBinData->ssboIndices);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        (binData->nBins + 1) * sizeof(int),  // +1 for number of clusters
        nullptr,
        GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _nClustersBuffer);
    //GLuint* ptr = (GLuint*)glMapBufferRange(
    //    GL_ATOMIC_COUNTER_BUFFER,
    //    0,
    //    sizeof(GLuint),
    //    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
    //    );
    //ptr[0] = 0;
    //glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    _clusterDetectionShader.activate();

    _clusterDetectionShader.setUniform("_nBins", binData->nBins);
    _clusterDetectionShader.setUniform("_nDimensions", binData->nDimensions);
    _clusterDetectionShader.setUniform("_selectedDimension", _coloringDimension);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, binData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _coloredBinData->ssboIndices);
    //glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, _nClustersBuffer);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        1, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _coloredBinData->ssboIndices);
    int* p = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    int nClusters = p[0];
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    _coloringData->nClusters = nClusters;
    _coloredBinData->nClusters = nClusters;

    _clusterDetectionShader.deactivate();
}

void PCPFiltering::filterData(const ParallelCoordinatesPlotData* inData,
                              const BinningData* binData,
                              ParallelCoordinatesPlotData* outData, int nElements
    )
{
    _filteringShader.activate();
    _filteringShader.setUniform("_nDimensions", inData->nDimensions);
    _filteringShader.setUniform("_nBins", binData->nBins);

    _filteringShader.setUniform("_hasColoringInformation", _coloringData->hasData);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboData);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        outData->nValues * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    if (_coloringData->hasData) {
        _filteringShader.setUniform("_selectedDimension", _coloringDimension);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _coloringData->ssboColor);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            (_coloringData->nValues + 1) * sizeof(int), //+1 for the number of clusters
            nullptr,
            GL_DYNAMIC_DRAW
        );
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _memoryAccess);
    GLuint* ptr = (GLuint*)glMapBufferRange(
        GL_ATOMIC_COUNTER_BUFFER,
        0,
        sizeof(GLuint),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
    );
    ptr[0] = 0;
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, _memoryAccess);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inData->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, binData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, outData->ssboData);

    if (_coloringData->hasData) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _coloredBinData->ssboIndices);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, _coloringData->ssboColor);
    }

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        inData->nValues / inData->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _coloringData->ssboColor);
    int* p = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    p[0] = _coloredBinData->nClusters;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //LogInfo("Number of stored values: " << asd);

    _filteringShader.deactivate();
}

}  // namespace

