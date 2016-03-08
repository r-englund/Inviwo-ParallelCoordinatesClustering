#include <modules/pcclustering/processors/densitymapfiltering.h>

#include <modules/opengl/texture/textureutils.h>

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
    , _filteringMethod("_filteringMethod", "Filtering Method")
    , _percentage("_percentage", "Percentage")
    , _nClusters("_nClusters", "Number of Clusters", 1, 0, 64)
    , _percentageFiltering({{ShaderType::Compute, "densitymapfiltering_percentage.comp" }}, Shader::Build::No)
    , _topologyFiltering({{ShaderType::Compute, "densitymapfiltering_topology.comp", }}, Shader::Build::No)
{
    addPort(_binInport);
    addPort(_pcpInport);

    addPort(_binOutport);

    _filteringMethod.addOption("percentage", "Percentage", FilteringMethodOptionPercentage);
    _filteringMethod.addOption("topology", "Topology", FilteringMethodOptionTopological);
    addProperty(_filteringMethod);

    addProperty(_percentage);

    addProperty(_nClusters);


    _percentageFiltering.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );
    _percentageFiltering.build();
    _percentageFiltering.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });

    _topologyFiltering.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );
    _topologyFiltering.build();
    _topologyFiltering.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapFiltering::~DensityMapFiltering() {}

void DensityMapFiltering::process() {
    if (!_binInport.hasData())
        return;

    std::shared_ptr<const BinningData> inData = _binInport.getData();

    BinningData* outData = new BinningData;
    outData->nBins = inData->nBins;
    outData->nDimensions = inData->nDimensions;
    glGenBuffers(1, &outData->ssboBins);
    glGenBuffers(1, &outData->ssboMinMax);
    
    std::vector<int> minMaxData(outData->nDimensions * 2);
    for (int i = 0; i < outData->nDimensions; ++i) {
        minMaxData[2 * i] = 0;
        minMaxData[2 * i + 1] = 1;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboMinMax);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        outData->nDimensions * 2 * sizeof(int),
        minMaxData.data(),
        GL_STATIC_DRAW
    );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    LGL_ERROR;

    filterBins(inData.get(), outData);

    _binOutport.setData(outData);
}

void DensityMapFiltering::filterBins(const BinningData* inData, BinningData* outData) {
    if (_filteringMethod.get() == FilteringMethodOptionPercentage)
        filterBinsPercentage(inData, outData);
    else
        filterBinsTopology(inData, outData);
    LGL_ERROR;
}

void DensityMapFiltering::filterBinsPercentage(
    const BinningData* inData, BinningData* outData)
{
    _percentageFiltering.activate();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboBins);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        inData->nBins * inData->nDimensions * sizeof(int),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    _percentageFiltering.setUniform("_nBins", outData->nBins);
    _percentageFiltering.setUniform("_nDimensions", outData->nDimensions);
    _percentageFiltering.setUniform("_percentage", _percentage);

    _percentageFiltering.setUniform("INT_MAX", std::numeric_limits<int>::max());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outData->ssboBins);
    

    LGL_ERROR;
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        outData->nBins, 1, 1,
        outData->nDimensions, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    LGL_ERROR;

    _percentageFiltering.deactivate();
}

void DensityMapFiltering::filterBinsTopology(
    const BinningData* inData, BinningData* outData)
{
    _topologyFiltering.activate();

    LGL_ERROR;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboBins);
    LGL_ERROR;
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        inData->nBins * inData->nDimensions * sizeof(int),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    LGL_ERROR;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    LGL_ERROR;

    _topologyFiltering.setUniform("_nBins", outData->nBins);
    _topologyFiltering.setUniform("_nDimensions", outData->nDimensions);
    _topologyFiltering.setUniform("_nClusters", _nClusters);
    _topologyFiltering.setUniform("INT_MAX", std::numeric_limits<int>::max());
    LGL_ERROR;

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outData->ssboBins);

    LGL_ERROR;
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        outData->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    LGL_ERROR;

    _topologyFiltering.deactivate();
}

}  // namespace

