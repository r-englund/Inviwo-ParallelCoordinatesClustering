#include <modules/pcclustering/processors/pcpuploadrenderer.h>

#include <modules/pcclustering/misc/support.h>

#include <modules/opengl/texture/textureutils.h>

#include <bitset>

#define PRIM_RESTART (GLsizei(-1))

namespace inviwo {

const ProcessorInfo PCPUploadRenderer::processorInfo_{
    "pers.bock.PCPUploadRenderer",  // Class identifier
    "PCP Upload Renderer",            // Display name
    "Renderer",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo PCPUploadRenderer::getProcessorInfo() const {
    return processorInfo_;
}

PCPUploadRenderer::PCPUploadRenderer()
    : Processor()
    , _inport("data")
    , _coloringData("color")
    , _outport("image")
    , _horizontalBorder("_horizontalBorder", "Horizontal Border")
    , _verticalBorder("_verticalBorder", "Vertical Border")
    , _lineSmoothing("_lineSmoothing", "Line Smoothing")
    , _depthTesting("_depthTesting", "Depth Testing")
    , _dimensionOrderingString("_dimensionOrderingString", "Dimension Ordering")
    , _dimensionMaskString("_dimensionMask", "Dimension Mask")
    , _enableTextRendering("_enableTextRendering", "Text Rendering")
    , _alphaFactor("_alphaFactor", "Alpha Factor", 1.f, 1.f, 100000.f)
    , _transFunc("transferFunction", "Transfer Function")
    , _invalidate("invalidate", "Invalidate")
    //, _textBorder("_textBorder", "Text Border", 0.05f, 0.f, 1.f)
    , _shader("pcprenderer_upload.vert", "pcprenderer.frag")
    , _backgroundShader("pcprenderer_background.frag")
    , _multiDrawCount(nullptr)
    , _multiDrawIndices(nullptr)
    , _drawElements(nullptr)
{
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);


    glGenBuffers(1, &_dimensionOrderingBuffer);

    addPort(_inport);
    addPort(_coloringData);
    _coloringData.setOptional(true);

    addPort(_outport);

    addProperty(_horizontalBorder);
    _horizontalBorder.onChange([this](){ invalidateBuffer(); });
    addProperty(_verticalBorder);

    addProperty(_lineSmoothing);
    addProperty(_depthTesting);

    addProperty(_alphaFactor);

    addProperty(_enableTextRendering);

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
    addProperty(_dimensionMaskString);
    _dimensionMaskString.onChange([this]() {
        std::string s = _dimensionMaskString.get();
        if (s.empty())
            _dimensionMask = 0;
        else {
            _dimensionMask = std::bitset<32>(s);
        }
    });

    addProperty(_transFunc);

    addProperty(_invalidate);

    //addProperty(_textBorder);

    _transFunc.get().clearPoints();
    _transFunc.get().addPoint(vec2(0, 1), vec4(0, 0, 0, 1));
    _transFunc.get().addPoint(vec2(1, 1), vec4(1, 1, 1, 1));

    _inport.onChange([this]() { invalidateBuffer(); });

    _shader.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });
    _backgroundShader.onReload([this]() { invalidate(InvalidationLevel::InvalidOutput); });

    _invalidate.onChange([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

PCPUploadRenderer::~PCPUploadRenderer() {
    glDeleteVertexArrays(1, &_vao);
    glDeleteBuffers(1, &_vbo);
    glDeleteBuffers(1, &_dimensionOrderingBuffer);
}

float dimensionLocation2(int dimension, float border, int nDimensions) {
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

void PCPUploadRenderer::invalidateBuffer() {
    if (!_inport.hasData())
        return;

    std::shared_ptr<const ParallelCoordinatesPlotRawData> data = _inport.getData();
    int nValues = data->data.size();
    int nDimensions = data->minMax.size();
    int nLines = nValues / nDimensions;

    std::vector<float> values;
    values.reserve(nValues * 2);
    int i = 0;
    for (float f : data->data) {
        values.push_back(dimensionLocation2(i, _horizontalBorder, nDimensions));
        values.push_back(f);
        i = (i + 1) % nDimensions;
    }


    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        values.size() * sizeof(float),
        values.data(),
        GL_STATIC_DRAW
    );
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    delete _multiDrawCount;
    delete _multiDrawIndices;

    _multiDrawCount = new GLsizei[nLines];
    _multiDrawIndices = new GLsizei[nLines];

    for (int i = 0; i < nLines; ++i) {
        _multiDrawCount[i] = data->minMax.size();
        _multiDrawIndices[i] = i * data->minMax.size();
    }

    
    delete _drawElements;
    _drawElements = new GLsizei[data->data.size() + nLines];
    int k = 0;
    for (int i = 0; i < nLines; ++i) {
        int baseIndex = i * (nDimensions + 1);
        for (int j = 0; j < nDimensions; ++j, ++k) {
            _drawElements[baseIndex + j] = k;
            std::cout << k << std::endl;
        }
        _drawElements[baseIndex + nDimensions] = PRIM_RESTART;
        std::cout << PRIM_RESTART << std::endl;
    }


    //std::shared_ptr<const ParallelCoordinatesPlotData> data = _inport.getData();


    //if (_dimensionOrdering.empty()) {
    //    _dimensionOrdering.resize(data->nDimensions);
    //    std::iota(_dimensionOrdering.begin(), _dimensionOrdering.end(), 0);
    //}
    //if (_dimensionOrdering.size() != data->nDimensions) {
    //    LogError("Wrong dimensions. Dimension ordering: " << _dimensionOrdering.size() <<
    //        "  nDimensions: " << data->nDimensions
    //    );
    //    return;
    //}

    //glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    //std::vector<float> vertexData(data->nDimensions);
    //for (int i = 0; i < data->nDimensions; ++i)
    //    vertexData[i] = dimensionLocation(i, _horizontalBorder, data->nDimensions);

    //    //int dim = _dimensionOrdering[i];


    //glBufferData(GL_ARRAY_BUFFER,
    //    vertexData.size() * sizeof(float),
    //    vertexData.data(),
    //    GL_DYNAMIC_DRAW
    //);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _dimensionOrderingBuffer);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _dimensionOrdering.size() * sizeof(int),
        _dimensionOrdering.data(),
        GL_STATIC_DRAW
    );

    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void PCPUploadRenderer::process() {
    if (!_inport.hasData())
        return;
    utilgl::ClearColor colorState(glm::vec4(0.0));
    utilgl::activateAndClearTarget(_outport);

    //glFinish();
    //{
        //IVW_CPU_PROFILING("PCPUploadRenderer");
        renderParallelCoordinates();
        //glFinish();
    //}
    renderBackground();
    if (_enableTextRendering) {
        renderTextOverlay(
            _textRenderer,
            _outport.getData()->getDimensions(),
            _dimensionOrdering,
            _dimensionMask
        );
    }


    utilgl::deactivateCurrentTarget();
}

void PCPUploadRenderer::renderParallelCoordinates() {
    std::shared_ptr<const ParallelCoordinatesPlotRawData> data = _inport.getData();
    int nDimensions = data->minMax.size();
    int nValues = data->data.size();

    utilgl::GlBoolState depthTest(GL_DEPTH_TEST, !_depthTesting);
    utilgl::BlendModeEquationState blendEquation(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD);

    utilgl::GlBoolState lineSmooth(GL_LINE_SMOOTH, _lineSmoothing);
    if (_lineSmoothing)
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    _shader.activate();

    _shader.setUniform("_nDimensions", nDimensions);
    _shader.setUniform("_nData", nValues / nDimensions);
    _shader.setUniform("_horizontalBorder", _horizontalBorder);
    _shader.setUniform("_verticalBorder", _verticalBorder);
    _shader.setUniform("_depthTesting", !_depthTesting);
    _shader.setUniform("_alphaFactor", _alphaFactor);

    TextureUnit tfUnit;
    utilgl::bindTexture(_transFunc, tfUnit);
    _shader.setUniform("_transFunc", tfUnit.getUnitNumber());

    glBindVertexArray(_vao);

    bool hasColoringData = _coloringData.hasData() && _coloringData.getData()->hasData;
    _shader.setUniform("_hasColoringData", hasColoringData);
    if (hasColoringData) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _coloringData.getData()->ssboColor);
    }

    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboData);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _dimensionOrderingBuffer);

    //for (int i = 0; i < nValues / nDimensions; ++i) {
    //    glDrawElements(GL_LINE_STRIP, nDimensions, )
    //}

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(PRIM_RESTART);

    glDrawElements(GL_LINE_STRIP, nValues / nDimensions, GL_UNSIGNED_INT, _drawElements);


    glDisable(GL_PRIMITIVE_RESTART);
    //glDrawArrays(GL_LINE_STRIP, 0, nValues);

    //glMultiDrawArrays(GL_LINE_STRIP, _multiDrawIndices, _multiDrawCount, nValues / nDimensions);

    //glDrawArraysInstanced(GL_LINE_STRIP, 0, data->nDimensions, data->nValues / data->nDimensions);

    glBindVertexArray(0);

    _shader.deactivate();
}

void PCPUploadRenderer::renderBackground() {
    std::shared_ptr<const ParallelCoordinatesPlotRawData> data = _inport.getData();

    _backgroundShader.activate();
    _backgroundShader.setUniform("_offset", _verticalBorder);
    _backgroundShader.setUniform("_nDimensions", int(data->minMax.size()));
    uint32_t dm = _dimensionMask.to_ulong();
    _backgroundShader.setUniform("_dimensionMask", dm);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _dimensionOrderingBuffer);
    utilgl::singleDrawImagePlaneRect();
    _backgroundShader.deactivate();
}

}  // namespace
