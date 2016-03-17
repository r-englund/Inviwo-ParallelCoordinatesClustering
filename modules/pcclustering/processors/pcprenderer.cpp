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
    , _coloringData("color")
    , _outport("image")
    , _horizontalBorder("_horizontalBorder", "Horizontal Border")
    , _verticalBorder("_verticalBorder", "Vertical Border")
    , _dimensionOrderingString("_dimensionOrderingString", "Dimension Ordering")
    , _transFunc("transferFunction", "Transfer Function")
    , _shader("pcprenderer.vert", "pcprenderer.frag")
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);

    glGenBuffers(1, &_dimensionOrderingBuffer);

    addPort(_inport);
    addPort(_coloringData);
    _coloringData.setOptional(true);

    addPort(_outport);

    addProperty(_horizontalBorder);
    _horizontalBorder.onChange([this]() {invalidateBuffer(); });
    addProperty(_verticalBorder);

    addProperty(_dimensionOrderingString);
    _dimensionOrderingString.onChange([this]() {
        _dimensionOrdering.clear();
        _dimensionOrdering.reserve(_dimensionOrderingString.get().size());
        for (char c : _dimensionOrderingString.get()) {
            int ia = c - '0';
            _dimensionOrdering.push_back(ia);
        }
        invalidateBuffer();
    });

    addProperty(_transFunc);

    _transFunc.get().clearPoints();
    _transFunc.get().addPoint(vec2(0, 1), vec4(0, 0, 0, 1));
    _transFunc.get().addPoint(vec2(1, 1), vec4(1, 1, 1, 1));

    _inport.onChange([this]() { invalidateBuffer(); });

    _shader.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
}

PCPRenderer::~PCPRenderer() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_dimensionOrderingBuffer);
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

    return minValue * (1.f - dim) + maxValue * dim;
}

void PCPRenderer::invalidateBuffer() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();


    if (_dimensionOrdering.empty()) {
        _dimensionOrdering.resize(data->nDimensions);
        std::iota(_dimensionOrdering.begin(), _dimensionOrdering.end(), 0);
    }
    if (_dimensionOrdering.size() != data->nDimensions) {
        LogError("Wrong dimensions. Dimension ordering: " << _dimensionOrdering.size() <<
            "  nDimensions: " << data->nDimensions
        );
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    std::vector<float> vertexData(data->nDimensions);
    for (int i = 0; i < data->nDimensions; ++i)
        vertexData[i] = dimensionLocation(i, _horizontalBorder, data->nDimensions);

        //int dim = _dimensionOrdering[i];


    glBufferData(GL_ARRAY_BUFFER,
        vertexData.size() * sizeof(float),
        vertexData.data(),
        GL_DYNAMIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _dimensionOrderingBuffer);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _dimensionOrdering.size() * sizeof(int),
        _dimensionOrdering.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void PCPRenderer::process() {
    if (!_inport.hasData())
        return;
    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();

    utilgl::ClearColor colorState(glm::vec4(0.0));
    utilgl::activateAndClearTarget(_outport);

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);
    utilgl::BlendModeEquationState blendEquation(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);
    utilgl::GlBoolState lineSmooth(GL_LINE_SMOOTH, true);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);


    _shader.activate();

    _shader.setUniform("_nDimensions", data->nDimensions);
    _shader.setUniform("_nData", data->nValues / data->nDimensions);
    _shader.setUniform("_horizontalBorder", _horizontalBorder);
    _shader.setUniform("_verticalBorder", _verticalBorder);

    TextureUnit tfUnit;
    utilgl::bindTexture(_transFunc, tfUnit);
    _shader.setUniform("_transFunc", tfUnit.getUnitNumber());

    glBindVertexArray(_vao);

    bool hasColoringData = _coloringData.hasData() && _coloringData.getData()->hasData;
    _shader.setUniform("_hasColoringData", hasColoringData);
    if (hasColoringData) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _coloringData.getData()->ssboColor);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _dimensionOrderingBuffer);

    glDrawArraysInstanced(GL_LINE_STRIP, 0, data->nDimensions, data->nValues / data->nDimensions);

    glBindVertexArray(0);

    _shader.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace

