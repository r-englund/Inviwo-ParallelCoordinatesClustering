#include <modules/pcclustering/processors/densitymapgenerator.h>

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
    , _inport("in.data")
    , _outport("out.data")
    , _nBins("_nBins", "# Bins", 32, 1, 2048)
    , _densityMapGeneratorShader({{ShaderType::Compute, "densitymapgenerator.comp" }}, Shader::Build::No)
    , _densityMapCounterShader({{ShaderType::Compute, "densitymapcounter.comp" }}, Shader::Build::No)
{
    addPort(_inport);
    addPort(_outport);

    addProperty(_nBins);

    _densityMapGeneratorShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );
    _densityMapGeneratorShader.build();
    _densityMapGeneratorShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });

    _densityMapCounterShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
        );
    _densityMapCounterShader.build();
    _densityMapCounterShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapGenerator::~DensityMapGenerator() {}

void DensityMapGenerator::process() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();

    BinningData* outData = new BinningData;
    glGenBuffers(1, &outData->ssboBins);
    glGenBuffers(1, &outData->ssboMinMax);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboBins);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _nBins * data->nDimensions * sizeof(int),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboMinMax);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        data->nDimensions * 2 * sizeof(int),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    outData->nBins = _nBins;
    outData->nDimensions = data->nDimensions;

    _densityMapGeneratorShader.activate();
    _densityMapGeneratorShader.setUniform("_nBins", _nBins);
    _densityMapGeneratorShader.setUniform("_nDimensions", data->nDimensions);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data->ssboMinMax);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, outData->ssboBins);
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, outData->ssboMinMax);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        data->nValues / _nBins, 1, 1,
        _nBins, 1, 1
    );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    _densityMapGeneratorShader.deactivate();


    _densityMapCounterShader.activate();
    _densityMapCounterShader.setUniform("_nBins", _nBins);
    _densityMapCounterShader.setUniform("_nDimensions", data->nDimensions);
    _densityMapCounterShader.setUniform("INT_MAX", std::numeric_limits<int>::max());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outData->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outData->ssboMinMax);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchComputeGroupSizeARB(
        data->nDimensions, 1, 1,
        1, 1, 1
        );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    _densityMapCounterShader.deactivate();

    _outport.setData(outData);
    LGL_ERROR;
}


}  // namespace

