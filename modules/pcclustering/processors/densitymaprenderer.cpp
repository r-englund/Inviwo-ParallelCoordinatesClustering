#include <modules/pcclustering/processors/densitymaprenderer.h>

#include <modules/pcclustering/misc/support.h>
#include <modules/opengl/texture/textureutils.h>

#include <bitset>

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
    , _colorInport("color")
    , _outport("outport")
    , _dimensionOrderingString("_dimensionOrderingString", "Dimension Ordering")
    , _dimensionMaskString("_dimensionMask", "Dimension Mask")
    , _enableTextRendering("_enableTextRendering", "Enable Text Rendering")
    , _transFunc("transferFunction", "Transfer Function")
    , _textBorder("_textBorder", "Text Border", 0.05f, 0.f, 1.f)
    , _shader("densitymaprenderer.frag")
{
    glGenBuffers(1, &_dimensionOrderingBuffer);

    addPort(_inport);
    addPort(_colorInport);
    _colorInport.setOptional(true);
    
    addPort(_outport);

    addProperty(_enableTextRendering);

    addProperty(_dimensionOrderingString);
    _dimensionOrderingString.onChange([this]() {
        _dimensionOrdering.clear();
        _dimensionOrdering.reserve(_dimensionOrderingString.get().size());
        for (char c : _dimensionOrderingString.get()) {
            int ia = c - '0';
            _dimensionOrdering.push_back(ia);
        }

        generateBuffers();
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
    addProperty(_textBorder);

    
    auto& tf = _transFunc.get();
    tf.clear();
    tf.add(0, vec4(0, 0, 0, 1));
    tf.add(1, vec4(1, 1, 1, 1));
    _transFunc.setCurrentStateAsDefault();

    _shader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapRenderer::~DensityMapRenderer() {
    glDeleteBuffers(1, &_dimensionOrderingBuffer);
}

void DensityMapRenderer::generateBuffers() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _dimensionOrderingBuffer);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _dimensionOrdering.size() * sizeof(int),
        _dimensionOrdering.data(),
        GL_STATIC_DRAW
        );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void DensityMapRenderer::process() {
    if (!_inport.hasData())
        return;

    utilgl::activateAndClearTarget(_outport);

    renderDensityMap();
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

void DensityMapRenderer::renderDensityMap() {
    bool hasColoringData = _colorInport.hasData() && _colorInport.getData()->hasData;

    std::shared_ptr<const BinningData> data = _inport.getData();

    _shader.activate();

    _shader.setUniform("_nBins", data->nBins);
    _shader.setUniform("_nDimensions", data->nDimensions);
    _shader.setUniform("_offset", _textBorder);
    _shader.setUniform("_hasColoringData", hasColoringData);
    uint32_t dm = _dimensionMask.to_ulong();
    _shader.setUniform("_dimensionMask", dm);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data->ssboMinMax);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _dimensionOrderingBuffer);

    TextureUnit tfUnit;
    if (hasColoringData) {
        _shader.setUniform("_selectedDimension", _colorInport.getData()->selectedDimension);

        utilgl::bindTexture(_transFunc, tfUnit);
        _shader.setUniform("_transFunc", tfUnit.getUnitNumber());

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _colorInport.getData()->ssboIndices);
    }

    utilgl::singleDrawImagePlaneRect();
    _shader.deactivate();
}

}  // namespace
