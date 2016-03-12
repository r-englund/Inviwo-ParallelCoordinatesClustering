#include <modules/pcclustering/processors/densitymaprenderer.h>

#include <modules/opengl/texture/textureutils.h>


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
    , _transFunc("transferFunction", "Transfer Function")
    , _shader("densitymaprenderer.frag")
{
    addPort(_inport);
    addPort(_colorInport);
    _colorInport.setOptional(true);
    
    addPort(_outport);

    addProperty(_transFunc);

    _transFunc.get().clearPoints();
    _transFunc.get().addPoint(vec2(0, 1), vec4(0, 0, 0, 1));
    _transFunc.get().addPoint(vec2(1, 1), vec4(1, 1, 1, 1));

    _shader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

DensityMapRenderer::~DensityMapRenderer() {}

void DensityMapRenderer::process() {
    if (!_inport.hasData())
        return;

    bool hasColoringData = _colorInport.hasData() && _colorInport.getData()->hasData;

    std::shared_ptr<const BinningData> data = _inport.getData();

    utilgl::activateAndClearTarget(_outport);

    _shader.activate();

    _shader.setUniform("_nBins", data->nBins);
    _shader.setUniform("_nDimensions", data->nDimensions);
    _shader.setUniform(
        "_outportSize",
        glm::ivec2(_outport.getDimensions().x, _outport.getDimensions().y)
    );
    _shader.setUniform("_hasColoringData", hasColoringData);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboBins);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, data->ssboMinMax);

    TextureUnit tfUnit;
    if (hasColoringData) {
        _shader.setUniform("_selectedDimension", _colorInport.getData()->selectedDimension);

        utilgl::bindTexture(_transFunc, tfUnit);
        _shader.setUniform("_transFunc", tfUnit.getUnitNumber());

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _colorInport.getData()->ssboIndices);
    }

    utilgl::singleDrawImagePlaneRect();

    _shader.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace
