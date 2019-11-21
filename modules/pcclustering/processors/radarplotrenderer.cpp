#include <modules/pcclustering/processors/radarplotrenderer.h>

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

const ProcessorInfo RadarPlotRenderer::processorInfo_{
    "pers.bock.RadarPlotRenderer",  // Class identifier
    "RadarPlot Renderer",            // Display name
    "Renderer",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo RadarPlotRenderer::getProcessorInfo() const {
    return processorInfo_;
}

RadarPlotRenderer::RadarPlotRenderer()
    : Processor()
    , _inport("data")
    , _coloringData("color")
    , _outport("image")
    , _lineSmoothing("_lineSmoothing", "Line Smoothing")
    , _alphaFactor("_alphaFactor", "Alpha Factor", 1.f, 0.f, 1.f)
    //, _xAxisSelection("_xAxisSelection", "X Axis")
    //, _yAxisSelection("_yAxisSelection", "Y Axis")
    //, _glyphSize("_glyphSize", "Glyph Size")
    , _transFunc("transferFunction", "Transfer Function")
    , _invalidate("invalidate", "Invalidate")
    , _shader("radarplotrenderer.vert", "radarplotrenderer.frag")
    , _linesShader("radarplotlines.frag")
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    float data[2] = { 0.f, 0.f };
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(float), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);
    
    addPort(_inport);
    addPort(_coloringData);
    _coloringData.setOptional(true);

    addPort(_outport);

    addProperty(_lineSmoothing);
    addProperty(_alphaFactor);
    addProperty(_invalidate);
    addProperty(_transFunc);

    auto& tf = _transFunc.get();
    tf.clear();
    tf.add(0, vec4(0, 0, 0, 1));
    tf.add(1, vec4(1, 1, 1, 1));
    _transFunc.setCurrentStateAsDefault();

    _inport.onChange([this]() { invalidateBuffer(); });

    _shader.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    _linesShader.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    _invalidate.onChange([this]() {invalidate(InvalidationLevel::InvalidOutput); });

    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    float vboData[2 * 20] = {
        // 20 hard-coded dimensions ought to be enough for everyone
        0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
        0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f
    };
    glBufferData(GL_ARRAY_BUFFER, 20 * 2 * sizeof(float), vboData, GL_STATIC_DRAW);


    glGenVertexArrays(1, &_linesvao);
    glGenBuffers(1, &_linesvbo);

    glBindVertexArray(_linesvao);
    glBindBuffer(GL_ARRAY_BUFFER, _linesvbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
}

RadarPlotRenderer::~RadarPlotRenderer() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);

    glDeleteVertexArrays(1, &_linesvao);
    glDeleteBuffers(1, &_linesvbo);
}

void RadarPlotRenderer::invalidateBuffer() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();


    glBindVertexArray(_linesvao);
    glBindBuffer(GL_ARRAY_BUFFER, _linesvbo);

    std::vector<float> values;
    for (int i = 0; i < data->nDimensions; ++i) {
        float angle = ((2 * glm::pi<float>()) / data->nDimensions) * i;

        float xPos = cos(angle) * 10;
        float yPos = sin(angle) * 10;

        // Define a single line from 0 to pos
        values.push_back(0.f);
        values.push_back(0.f);
        values.push_back(xPos);
        values.push_back(yPos);
    }
    glBufferData(
        GL_ARRAY_BUFFER,
        2 * 2 * data->nDimensions * sizeof(float),
        values.data(),
        GL_STATIC_DRAW
    );

    glBindVertexArray(0);
}

void RadarPlotRenderer::process() {
    if (!_inport.hasData())
        return;
    std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();

    utilgl::ClearColor colorState(glm::vec4(0.0));
    utilgl::activateAndClearTarget(_outport);

    //utilgl::GlBoolState alpha(GL_ALPHA, true);
    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, false);
    utilgl::BlendModeEquationState blendEquation(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);

    utilgl::GlBoolState lineSmooth(GL_LINE_SMOOTH, _lineSmoothing);
    if (_lineSmoothing)
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    _shader.activate();

    _shader.setUniform("_nDimensions", data->nDimensions);
    _shader.setUniform("_nData", data->nValues / data->nDimensions);

    TextureUnit tfUnit;
    utilgl::bindTexture(_transFunc, tfUnit);
    _shader.setUniform("_transFunc", tfUnit.getUnitNumber());
    _shader.setUniform("_alphaFactor", _alphaFactor);

    glBindVertexArray(_vao);

    bool hasColoringData = _coloringData.hasData() && _coloringData.getData()->hasData;
    _shader.setUniform("_hasColoringData", hasColoringData);
    if (hasColoringData) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _coloringData.getData()->ssboColor);
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboData);

    glDrawArraysInstanced(GL_LINE_LOOP, 0, data->nDimensions, data->nValues / data->nDimensions);
    _shader.deactivate();

    
    

    _linesShader.activate();

    glBindVertexArray(_linesvao);
    glLineWidth(2.0);
    glDrawArrays(GL_LINE_STRIP, 0, 4 * data->nDimensions);
    glLineWidth(1.0);
    _linesShader.deactivate();


    glBindVertexArray(0);


    utilgl::deactivateCurrentTarget();
}

}  // namespace

