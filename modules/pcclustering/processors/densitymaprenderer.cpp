#include <modules/pcclustering/processors/densitymaprenderer.h>

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

const ProcessorInfo DensityMapRenderer::processorInfo_{
    "pers.bock.DensityMapRenderer",  // Class identifier
    "Density Map Renderer",            // Display name
    "Renderer",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo DensityMapRenderer::getProcessorInfo() const {
    return processorInfo_;
}

DensityMapRenderer::DensityMapRenderer()
    : Processor()
    , _inport("data")
    , _outport("outport")
    , _renderScaling("_renderScaling", "Scaling Factor", 1.f, 1.f, 5000.f)
    , _shader("densitymaprenderer.frag")
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_countBuffer);

    addPort(_inport);
    addPort(_outport);

    addProperty(_renderScaling);

    _shader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapRenderer::~DensityMapRenderer() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_countBuffer);
}

void DensityMapRenderer::process() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const BinningData> data = _inport.getData();

    LGL_ERROR;
    utilgl::activateAndClearTarget(_outport);
    LGL_ERROR;

    _shader.activate();
    LGL_ERROR;

    _shader.setUniform("_nBins", data->nBins);
    _shader.setUniform("_nDimensions", data->nDimensions);
    _shader.setUniform("_renderScaling", _renderScaling);
    _shader.setUniform(
        "_outportSize",
        glm::ivec2(_outport.getDimensions().x, _outport.getDimensions().y)
    );
    
    //glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 7, _countBuffer);
    //glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);

    //GLuint zero = 0;
    //glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);


    glBindVertexArray(_vao);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboBins);

    utilgl::singleDrawImagePlaneRect();


    glBindVertexArray(0);

    _shader.deactivate();
    utilgl::deactivateCurrentTarget();
    LGL_ERROR;
}

}  // namespace
