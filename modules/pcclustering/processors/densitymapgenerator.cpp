#include <modules/pcclustering/processors/densitymapgenerator.h>

#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/util/clock.h>

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

const ProcessorInfo DensityMapGenerator::processorInfo_{
    "pers.bock.DensityMapGenerator",  // Class identifier
    "Density Map Generator",            // Display name
    "Renderer",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo DensityMapGenerator::getProcessorInfo() const {
    return processorInfo_;
}

DensityMapGenerator::DensityMapGenerator()
    : Processor()
    , _inport("in_data")
    , _outport("out_data")
    , _nBins("_nBins", "# Bins", 32, 1, 2048)
    , _invalidate("_invalidate", "Invalidate")
    , _densityMapGeneratorShader({{ShaderType::Compute, "densitymapgenerator.comp" }}, Shader::Build::No)
    , _densityMapCounterShader({{ShaderType::Compute, "densitymapcounter.comp" }}, Shader::Build::No)
{
    addPort(_inport);
    addPort(_outport);

    _inport.onChange([this]() { invalidateBuffers(); });

    addProperty(_nBins);

    _densityMapGeneratorShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );
    _densityMapGeneratorShader.build();
    _densityMapGeneratorShader.onReload([this]() {
        invalidateBuffers(); 
        invalidate(InvalidationLevel::InvalidOutput);
    });

    _densityMapCounterShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
        );
    _densityMapCounterShader.build();
    _densityMapCounterShader.onReload([this]() {
        invalidateBuffers();
        invalidate(InvalidationLevel::InvalidOutput);
    });

    _binningData = std::make_shared<BinningData>();
    glGenBuffers(1, &_binningData->ssboBins);
    glGenBuffers(1, &_binningData->ssboMinMax);

    addProperty(_invalidate);
    _invalidate.onChange([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapGenerator::~DensityMapGenerator() {}

void DensityMapGenerator::invalidateBuffers() {
    glDeleteBuffers(1, &_binningData->ssboBins);
    glDeleteBuffers(1, &_binningData->ssboMinMax);

    glGenBuffers(1, &_binningData->ssboBins);
    glGenBuffers(1, &_binningData->ssboMinMax);
}

void DensityMapGenerator::process() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _binningData->ssboBins);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _nBins * data->nDimensions * sizeof(int),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _binningData->ssboMinMax);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        data->nDimensions * 2 * sizeof(int),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    _binningData->nBins = _nBins;
    _binningData->nDimensions = data->nDimensions;

    _densityMapGeneratorShader.activate();
    _densityMapGeneratorShader.setUniform("_nBins", _nBins);
    _densityMapGeneratorShader.setUniform("_nDimensions", data->nDimensions);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _binningData->ssboBins);

    glFinish();
    {
        IVW_CPU_PROFILING("DensityMapGenerator");

        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glDispatchComputeGroupSizeARB(
            data->nValues / _nBins, 1, 1,
            _nBins, 1, 1
            );
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        glFinish();
    }
    _densityMapGeneratorShader.deactivate();


    _densityMapCounterShader.activate();
    _densityMapCounterShader.setUniform("_nBins", _nBins);
    _densityMapCounterShader.setUniform("_nDimensions", data->nDimensions);
    _densityMapCounterShader.setUniform("INT_MAX", std::numeric_limits<int>::max());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _binningData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _binningData->ssboMinMax);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        data->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    _densityMapCounterShader.deactivate();
    glFinish();

    _outport.setData(_binningData);
    LGL_ERROR;
}


}  // namespace

