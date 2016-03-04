#include <modules/pcclustering/processors/pcprenderer.h>

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

const ProcessorInfo PCPRenderer::processorInfo_{
    "pers.bock.PCPRenderer",  // Class identifier
    "PCP Renderer",            // Display name
    "Renderer",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo PCPRenderer::getProcessorInfo() const {
    return processorInfo_;
}

PCPRenderer::PCPRenderer()
    : Processor()
    , _inport("data")
    , _outport("image")
    , _horizontalBorder("_horizontalBorder", "Horizontal Border")
    , _verticalBorder("_verticalBorder", "Vertical Border")
    , _shader("pcprenderer.vert", "pcprenderer.frag")
{
    glGenVertexArrays(1, &_vao);

    addPort(_inport);
    addPort(_outport);

    addProperty(_horizontalBorder);
    addProperty(_verticalBorder);

    _shader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

PCPRenderer::~PCPRenderer() {
    glDeleteVertexArrays(1, &_vao);
}

void PCPRenderer::process() {
    if (!_inport.hasData())
        return;
    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();

    utilgl::ClearColor colorState(glm::vec4(0.0));
    utilgl::activateAndClearTarget(_outport);

    utilgl::GlBoolState blendModeEnableState(GL_BLEND, true);
    utilgl::BlendModeEquationState blendModeState(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
    //utilgl::BlendModeState blendModeState(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _shader.activate();

    _shader.setUniform("_nDimensions", data->nDimensions);
    _shader.setUniform("_horizontalBorder", _horizontalBorder);
    _shader.setUniform("_verticalBorder", _verticalBorder);

    glBindVertexArray(_vao);


    glBindBuffer(GL_ARRAY_BUFFER, data->ssboData);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, data->ssboMinMax);

    for (int i = 0; i < data->nValues / data->nDimensions; ++i)
        glDrawArrays(GL_LINE_STRIP, i * data->nDimensions, data->nDimensions);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    _shader.deactivate();

    utilgl::deactivateCurrentTarget();
    LGL_ERROR;
}


}  // namespace

