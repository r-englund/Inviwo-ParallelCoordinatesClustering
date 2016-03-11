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
    , _filteringMethod("_filteringMethod", "Filtering Method")
    , _countingShader({{ShaderType::Compute, "pcpfiltering_counting.comp" }}, Shader::Build::No)
    , _filteringShader({ { ShaderType::Compute, "pcpfiltering_filtering.comp" } }, Shader::Build::No)
{
    addPort(_binInport);
    addPort(_pcpInport);

    addPort(_pcpOutport);

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
}

PCPFiltering::~PCPFiltering() {
    glDeleteBuffers(1, &_nValuesCounter);
    glDeleteBuffers(1, &_memoryAccess);
}

void PCPFiltering::process() {
    if (!_binInport.hasData())
        return;

    if (!_pcpInport.hasData())
        return;

    std::shared_ptr<const BinningData> binInData = _binInport.getData();
    std::shared_ptr<const ParallelCoordinatesPlotData> pcpInData = _pcpInport.getData();

    _pcpData->nDimensions = pcpInData->nDimensions;

    filterData(pcpInData.get(), binInData.get(), _pcpData.get());

    _pcpOutport.setData(_pcpData);
}

void PCPFiltering::filterData(const ParallelCoordinatesPlotData* inData,
                              const BinningData* binData,
                              ParallelCoordinatesPlotData* outData
    )
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
    outData->nValues = nDataValuesPositive * outData->nDimensions;

    //
    // Then actually write out the number of values
    //
    _filteringShader.activate();
    _filteringShader.setUniform("_nDimensions", inData->nDimensions);
    _filteringShader.setUniform("_nBins", binData->nBins);

    //glDeleteBuffers(1, &outData->ssboData);
    //glGenBuffers(1, &outData->ssboData);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboData);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        outData->nValues * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _memoryAccess);
    ptr = (GLuint*)glMapBufferRange(
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

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        inData->nValues / inData->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _memoryAccess);
    p = (int*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
    int asd = p[0];
    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    LogInfo("Number of stored values: " << asd);

    _filteringShader.deactivate();
}

}  // namespace

