#include <modules/pcclustering/datastructures/pcpdata.h>

#include <modules/opengl/shader/shader.h>
#include <modules/opengl/inviwoopengl.h>

ParallelCoordinatesPlotRawData::ParallelCoordinatesPlotRawData() {
    LogInfo("ParallelCoordinatesPlotRawData::ParallelCoordinatesPlotRawData");
}

ParallelCoordinatesPlotRawData::~ParallelCoordinatesPlotRawData() {
    LogInfo("ParallelCoordinatesPlotRawData::~ParallelCoordinatesPlotRawData");
}

ParallelCoordinatesPlotData::ParallelCoordinatesPlotData() {
    LogInfo("ParallelCoordinatesPlotData::ParallelCoordinatesPlotData");
}

ParallelCoordinatesPlotData::~ParallelCoordinatesPlotData() {
    LogInfo("ParallelCoordinatesPlotData::~ParallelCoordinatesPlotData");
}

BinningData::BinningData() {
    LogInfo("BinningData::BinningData");
}

BinningData::~BinningData() {
    LogInfo("BinningData::~BinningData");
}

ParallelCoordinatesPlotData* copyData(const ParallelCoordinatesPlotData* input) {
    ParallelCoordinatesPlotData* result = new ParallelCoordinatesPlotData;
    result->nDimensions = input->nDimensions;
    LGL_ERROR;
    glGenBuffers(1, &result->ssboData);
    glGenBuffers(1, &result->ssboMinMax);

    glBindBuffer(GL_COPY_WRITE_BUFFER, result->ssboMinMax);
    glBufferData(
        GL_COPY_WRITE_BUFFER,
        input->nDimensions * 2 * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW
        );

    glBindBuffer(GL_COPY_READ_BUFFER, input->ssboMinMax);
    glCopyBufferSubData(
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        0,
        0,
        input->nDimensions * 2 * sizeof(float)
        );
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    LGL_ERROR;

    return result;
}

void copyData(const ParallelCoordinatesPlotData* input, ParallelCoordinatesPlotData* output) {
    output->nDimensions = input->nDimensions;
    LGL_ERROR;
    glGenBuffers(1, &output->ssboData);
    glGenBuffers(1, &output->ssboMinMax);

    glBindBuffer(GL_COPY_WRITE_BUFFER, output->ssboMinMax);
    glBufferData(
        GL_COPY_WRITE_BUFFER,
        input->nDimensions * 2 * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW
        );

    glBindBuffer(GL_COPY_READ_BUFFER, input->ssboMinMax);
    glCopyBufferSubData(
        GL_COPY_READ_BUFFER,
        GL_COPY_WRITE_BUFFER,
        0,
        0,
        input->nDimensions * 2 * sizeof(float)
        );
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    LGL_ERROR;
}
