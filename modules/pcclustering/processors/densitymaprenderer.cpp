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
    , _dimensionOrderingString("_dimensionOrderingString", "Dimension Ordering")
    , _transFunc("transferFunction", "Transfer Function")
    , _shader("densitymaprenderer.frag")
{
    addPort(_inport);
    addPort(_colorInport);
    _colorInport.setOptional(true);
    
    addPort(_outport);

    addProperty(_dimensionOrderingString);
    _dimensionOrderingString.onChange([this]() {
        _dimensionOrdering.clear();
        _dimensionOrdering.reserve(_dimensionOrderingString.get().size());
        for (char c : _dimensionOrderingString.get()) {
            int ia = c - '0';
            _dimensionOrdering.push_back(ia);
        }
    });

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


    GLuint dimensionOrdering;
    glGenBuffers(1, &dimensionOrdering);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dimensionOrdering);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER,
        _dimensionOrdering.size() * sizeof(int),
        _dimensionOrdering.data(),
        GL_STATIC_DRAW
    );
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


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
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dimensionOrdering);

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

    glDeleteBuffers(1, &dimensionOrdering);
}

}  // namespace
