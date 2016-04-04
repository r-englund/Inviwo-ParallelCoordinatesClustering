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
    , _invalidate("_invalidate", "Invalidate")
    , _percentageFiltering({{ShaderType::Compute, "densitymapfiltering_percentage.comp" }}, Shader::Build::No)
    , _topologyFiltering({{ShaderType::Compute, "densitymapfiltering_topology.comp", }}, Shader::Build::No)
{
    addPort(_binInport);
    addPort(_pcpInport);

    _binInport.onChange([this](){recreateBuffers(); });
    _pcpInport.onChange([this]() {recreateBuffers(); });

    addPort(_binOutport);

    _filteringMethod.addOption("percentage", "Percentage", FilteringMethodOptionPercentage);
    _filteringMethod.addOption("topology", "Topology", FilteringMethodOptionTopological);
    addProperty(_filteringMethod);

    _filteringMethod.onChange([this]() { recreateBuffers(); });

    addProperty(_percentage);
    _percentage.onChange([this](){recreateBuffers();  });

    addProperty(_nClusters);
    _nClusters.onChange([this]() {recreateBuffers();  });


    _percentageFiltering.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );
    _percentageFiltering.build();
    _percentageFiltering.onReload([this]() {
        recreateBuffers(); 
        invalidate(InvalidationLevel::InvalidOutput); 
    });

    _topologyFiltering.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );
    _topologyFiltering.build();
    _topologyFiltering.onReload([this]() {
        recreateBuffers(); 
        invalidate(InvalidationLevel::InvalidOutput);
    });

    _binningData = std::make_shared<BinningData>();
    _binningData->ssboBins = 0;
    _binningData->ssboMinMax = 0;
    //glGenBuffers(1, &_binningData->ssboBins);

    //addProperty(_invalidate);
    _invalidate.onChange([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapFiltering::~DensityMapFiltering() {}

void DensityMapFiltering::recreateBuffers() {
    if (!_binInport.hasData())
        return;
    std::shared_ptr<const BinningData> inData = _binInport.getData();

    _binningData->nBins = inData->nBins;
    _binningData->nDimensions = inData->nDimensions;

    glDeleteBuffers(1, &_binningData->ssboBins);
    glDeleteBuffers(1, &_binningData->ssboMinMax);

    glGenBuffers(1, &_binningData->ssboBins);
    glGenBuffers(1, &_binningData->ssboMinMax);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _binningData->ssboBins);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _binningData->nBins * _binningData->nDimensions * sizeof(int),
        nullptr,
        GL_DYNAMIC_DRAW
        );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    std::vector<int> minMaxData(_binningData->nDimensions * 2);
    for (int i = 0; i < _binningData->nDimensions; ++i) {
        minMaxData[2 * i] = 0;
        minMaxData[2 * i + 1] = 1;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _binningData->ssboMinMax);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _binningData->nDimensions * 2 * sizeof(int),
        minMaxData.data(),
        GL_STATIC_DRAW
        );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void DensityMapFiltering::process() {
    if (!_binInport.hasData())
        return;

    std::shared_ptr<const BinningData> inData = _binInport.getData();

    //glFinish();
    //{
        //IVW_CPU_PROFILING("DensityMapFiltering");
        if (_filteringMethod.get() == FilteringMethodOptionPercentage)
            filterBinsPercentage(inData.get(), _binningData.get());
        else
            filterBinsTopology(inData.get(), _binningData.get());
    //glFinish();
    //}

    _binOutport.setData(_binningData);
}

void DensityMapFiltering::filterBinsPercentage(
    const BinningData* inData, BinningData* outData)
{
    _percentageFiltering.activate();

    _percentageFiltering.setUniform("_nBins", outData->nBins);
    _percentageFiltering.setUniform("_nDimensions", outData->nDimensions);
    _percentageFiltering.setUniform("_percentage", _percentage);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inData->ssboMinMax);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, outData->ssboBins);
    
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        outData->nBins, 1, 1,
        outData->nDimensions, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    _percentageFiltering.deactivate();
}

void DensityMapFiltering::filterBinsTopology(
    const BinningData* inData, BinningData* outData)
{
    _topologyFiltering.activate();

    _topologyFiltering.setUniform("_nBins", outData->nBins);
    _topologyFiltering.setUniform("_nDimensions", outData->nDimensions);
    _topologyFiltering.setUniform("_nClusters", _nClusters);
    _topologyFiltering.setUniform("INT_MAX", std::numeric_limits<int>::max());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outData->ssboBins);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        outData->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    _topologyFiltering.deactivate();
}

}  // namespace

