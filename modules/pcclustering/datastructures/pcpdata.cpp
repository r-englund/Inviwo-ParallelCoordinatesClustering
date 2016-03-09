#include <modules/pcclustering/datastructures/pcpdata.h>

#include <modules/opengl/shader/shader.h>
#include <modules/opengl/inviwoopengl.h>

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
