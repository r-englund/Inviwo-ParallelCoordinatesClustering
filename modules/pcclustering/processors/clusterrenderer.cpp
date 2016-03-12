#include <modules/pcclustering/processors/clusterrenderer.h>

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

const ProcessorInfo ClusterRenderer::processorInfo_{
    "pers.bock.ClusterRenderer",  // Class identifier
    "Cluster Renderer",            // Display name
    "Renderer",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo ClusterRenderer::getProcessorInfo() const {
    return processorInfo_;
}

ClusterRenderer::ClusterRenderer()
    : Processor()
    , _inport("data")
    , _outport("outport")
    , _shader("clusterrenderer.frag")
    , _transFunc("transferFunction", "Transfer Function")
{
    addPort(_inport);
    addPort(_outport);

    addProperty(_transFunc);

    _transFunc.get().clearPoints();
    _transFunc.get().addPoint(vec2(0, 1), vec4(0, 0, 0, 1));
    _transFunc.get().addPoint(vec2(1, 1), vec4(1, 1, 1, 1));

    _shader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
}

ClusterRenderer::~ClusterRenderer() {}

void ClusterRenderer::process() {
    if (!_inport.hasData() && _inport.getData()->hasData)
        return;

    std::shared_ptr<const ColoredBinData> data = _inport.getData();

    utilgl::activateAndClearTarget(_outport);

    _shader.activate();

    _shader.setUniform("_nBins", data->nBins);
    _shader.setUniform(
        "_outportSize",
        glm::ivec2(_outport.getDimensions().x, _outport.getDimensions().y)
        );

    TextureUnit tfUnit;
    utilgl::bindTexture(_transFunc, tfUnit);
    _shader.setUniform("_transFunc", tfUnit.getUnitNumber());


    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, data->ssboIndices);

    utilgl::singleDrawImagePlaneRect();

    _shader.deactivate();
    utilgl::deactivateCurrentTarget();
}

}  // namespace
