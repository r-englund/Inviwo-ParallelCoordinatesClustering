#ifndef IVW_PCPDATA_H
#define IVW_PCPDATA_H

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <vector>

#include <modules/opengl/openglutils.h>

//using ParallelCoordinatesPlotData = std::vector<float>;

struct ParallelCoordinatesPlotRawData {
    ParallelCoordinatesPlotRawData();
    ~ParallelCoordinatesPlotRawData();

    std::vector<float> data;
    std::vector<std::pair<float, float>> minMax;
};

using PCPRawDataInport = inviwo::DataInport<ParallelCoordinatesPlotRawData>;
using PCPRawDataOutport = inviwo::DataOutport<ParallelCoordinatesPlotRawData>;

struct ParallelCoordinatesPlotData {
    ParallelCoordinatesPlotData();
    ~ParallelCoordinatesPlotData();

    GLuint ssboData;
    int nDimensions;
    int nValues;
};

using PCPDataInport = inviwo::DataInport<ParallelCoordinatesPlotData>;
using PCPDataOutport = inviwo::DataOutport<ParallelCoordinatesPlotData>;

struct BinningData {
    BinningData();
    ~BinningData();

    GLuint ssboBins;
    GLuint ssboMinMax;
    int nBins;
    int nDimensions;
};

using BinningDataInport = inviwo::DataInport<BinningData>;
using BinningDataOutport = inviwo::DataOutport<BinningData>;

struct ColoredBinData {
    ColoredBinData();
    ~ColoredBinData();
    GLuint ssboIndices;
    int nBins;
    bool hasData;
};

using ColoredBinDataInport = inviwo::DataInport<ColoredBinData>;
using ColoredBinDataOutport = inviwo::DataOutport<ColoredBinData>;


struct ColoringData {
    ColoringData();
    ~ColoringData();

    GLuint ssboColor;
    int nValues;
    bool hasData;
};

using ColoringDataInport = inviwo::DataInport<ColoringData>;
using ColoringDataOutport = inviwo::DataOutport<ColoringData>;


#endif  // IVW_PCPDATA_H
