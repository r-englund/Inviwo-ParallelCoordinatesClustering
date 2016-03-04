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
    , _shader({{ShaderType::Compute, "densitymapgenerator.comp" }}, Shader::Build::No)
{
    glGenVertexArrays(1, &_vao);

    addPort(_inport);
    addPort(_outport);

    addProperty(_nBins);

    _shader.getShaderObject(ShaderType::Compute)->addShaderExtension(
        "GL_ARB_compute_variable_group_size",
        true
    );

    _shader.build();

    _shader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapGenerator::~DensityMapGenerator() {
    glDeleteVertexArrays(1, &_vao);
}

void DensityMapGenerator::process() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();

    BinningData* outData = new BinningData;
    glGenBuffers(1, &outData->ssboBins);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboBins);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _nBins * data->nDimensions * sizeof(int),
        nullptr,
        GL_DYNAMIC_DRAW
    );
    outData->nBins = _nBins;
    outData->nDimensions = data->nDimensions;

    _shader.activate();
    _shader.setUniform("_nBins", _nBins);
    _shader.setUniform("_nDimensions", data->nDimensions);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data->ssboMinMax);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, outData->ssboBins);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    //glDispatchCompute(data->nValues / 32, 1, 1);
    glDispatchComputeGroupSizeARB(
        data->nValues / 32, 1, 1,
        32, 1, 1
    );
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    _shader.deactivate();

    _outport.setData(outData);
    LGL_ERROR;
}


}  // namespace

