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
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);
    
    addPort(_inport);
    addPort(_outport);

    addProperty(_horizontalBorder);
    _horizontalBorder.onChange([this]() {invalidateBuffer(); });
    addProperty(_verticalBorder);

    _inport.onChange([this]() { invalidateBuffer(); });

    _shader.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

PCPRenderer::~PCPRenderer() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
}

float dimensionLocation(int dimension, float border, int nDimensions) {
    //  -1  -1+border    1-border     1
    //  |        |           |        |
    //
    // dimension(0)   -> (-1+border)
    // dimension(max) -> (1-border)
    //
    const float minValue = -1 + border;
    const float maxValue = 1 - border;

    const float dim = float(dimension) / (float(nDimensions) - 1.f);

    return minValue * (1.0 - dim) + maxValue * dim;
}

void PCPRenderer::invalidateBuffer() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    std::vector<float> vertexData(data->nDimensions);
    for (int i = 0; i < data->nDimensions; ++i)
        vertexData[i] = dimensionLocation(i, _horizontalBorder, data->nDimensions);

    glBufferData(GL_ARRAY_BUFFER,
        vertexData.size() * sizeof(float),
        vertexData.data(),
        GL_STATIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void PCPRenderer::process() {
    if (!_inport.hasData())
        return;
    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();

    utilgl::ClearColor colorState(glm::vec4(0.0));
    utilgl::activateAndClearTarget(_outport);

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);
    utilgl::BlendModeEquationState blendEquation(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);

    _shader.activate();

    _shader.setUniform("_nDimensions", data->nDimensions);
    _shader.setUniform("_nData", data->nValues / data->nDimensions);
    _shader.setUniform("_horizontalBorder", _horizontalBorder);
    _shader.setUniform("_verticalBorder", _verticalBorder);

    glBindVertexArray(_vao);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboData);

    glDrawArraysInstanced(GL_LINE_STRIP, 0, data->nDimensions, data->nValues / data->nDimensions);

    glBindVertexArray(0);

    _shader.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace

