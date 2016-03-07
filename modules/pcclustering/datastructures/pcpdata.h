#ifndef IVW_PCPDATA_H
#define IVW_PCPDATA_H

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <vector>

#include <modules/opengl/openglutils.h>

//using ParallelCoordinatesPlotData = std::vector<float>;

struct ParallelCoordinatesPlotRawData {
    std::vector<float> data;
    std::vector<std::pair<float, float>> minMax;
};

using PCPRawDataInport = inviwo::DataInport<ParallelCoordinatesPlotRawData>;
using PCPRawDataOutport = inviwo::DataOutport<ParallelCoordinatesPlotRawData>;

struct ParallelCoordinatesPlotData {
    GLuint ssboData;
    GLuint ssboMinMax;
    int nDimensions;
    int nValues;
};

using PCPDataInport = inviwo::DataInport<ParallelCoordinatesPlotData>;
using PCPDataOutport = inviwo::DataOutport<ParallelCoordinatesPlotData>;

struct BinningData {
    GLuint ssboBins;
    GLuint ssboMinMax;
    int nBins;
    int nDimensions;
};

using BinningDataInport = inviwo::DataInport<BinningData>;
using BinningDataOutport = inviwo::DataOutport<BinningData>;


#endif  // IVW_PCPDATA_H
