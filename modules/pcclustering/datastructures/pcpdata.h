#ifndef IVW_PCPDATA_H
#define IVW_PCPDATA_H

#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/datastructures/datatraits.h>
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

template <>
struct inviwo::DataTraits<ParallelCoordinatesPlotRawData> {
    static std::string classIdentifier() { return "org.inviwo.ParallelCoordinatesPlotRawData"; }
    static std::string dataName() { return "ParallelCoordinatesPlotRawData"; }
    static uvec3 colorCode() { return uvec3(141, 211, 199); }
    static Document info(const ParallelCoordinatesPlotRawData &data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "PCP Raw Data", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("Number of Parameters: "), data.minMax.size());
        tb(H("Number of Rows: "), (data.data.size() / data.minMax.size()));
        tb(H("Number of Cells: "), data.data.size());
        return doc;
    }
};

struct ParallelCoordinatesPlotData {
    ParallelCoordinatesPlotData();
    ~ParallelCoordinatesPlotData();

    GLuint ssboData;
    int nDimensions;
    int nValues;
};

using PCPDataInport = inviwo::DataInport<ParallelCoordinatesPlotData>;
using PCPDataOutport = inviwo::DataOutport<ParallelCoordinatesPlotData>;


template <>
struct inviwo::DataTraits<ParallelCoordinatesPlotData> {
    static std::string classIdentifier() { return "org.inviwo.ParallelCoordinatesPlotData"; }
    static std::string dataName() { return "ParallelCoordinatesPlotData"; }
    static uvec3 colorCode() { return uvec3(190, 186, 218); }
    static Document info(const ParallelCoordinatesPlotData &data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "PCP Data", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("ssboData: "), data.ssboData);
        tb(H("nDimensions: "), data.nDimensions);
        tb(H("nValues: "), data.nValues);
        return doc;
    }
};


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


template <>
struct inviwo::DataTraits<BinningData> {
    static std::string classIdentifier() { return "org.inviwo.BinningData"; }
    static std::string dataName() { return "BinningData"; }
    static uvec3 colorCode() { return uvec3(253, 180, 98); }
    static Document info(const BinningData &data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "Binning Data", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("ssboBins: "), data.ssboBins);
        tb(H("ssboMinMax: "), data.ssboMinMax);
        tb(H("nBins: "), data.nBins);
        tb(H("nDimensions: "), data.nDimensions);
        return doc;
    }
};

struct ColoredBinData {
    ColoredBinData();
    ~ColoredBinData();
    GLuint ssboIndices;
    int nBins;
    bool hasData;
    int selectedDimension;
    int nClusters;
};

using ColoredBinDataInport = inviwo::DataInport<ColoredBinData>;
using ColoredBinDataOutport = inviwo::DataOutport<ColoredBinData>;

template <>
struct inviwo::DataTraits<ColoredBinData> {
    static std::string classIdentifier() { return "org.inviwo.ColoredBinData"; }
    static std::string dataName() { return "ColoredBinData"; }
    static uvec3 colorCode() { return uvec3(217, 217, 217); }
    static Document info(const ColoredBinData &data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "Colored Binning Data", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("ssboIndices: "), data.ssboIndices);
        tb(H("nBins: "), data.nBins);
        tb(H("hasData: "), data.hasData);
        tb(H("selectedDimension: "), data.selectedDimension);
        tb(H("nClusters: "), data.nClusters);
        return doc;
    }
};

struct ColoringData {
    ColoringData();
    ~ColoringData();

    GLuint ssboColor;
    int nValues;
    bool hasData;
    int nClusters;
};

using ColoringDataInport = inviwo::DataInport<ColoringData>;
using ColoringDataOutport = inviwo::DataOutport<ColoringData>;

template <>
struct inviwo::DataTraits<ColoringData> {
    static std::string classIdentifier() { return "org.inviwo.ColoringData"; }
    static std::string dataName() { return "ColoringData"; }
    static uvec3 colorCode() { return uvec3(251, 128, 114); }
    static Document info(const ColoringData &data) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "Coloring Data", {{"style", "color:white;"}});
        utildoc::TableBuilder tb(doc.handle(), P::end());
        tb(H("ssboColor: "), data.ssboColor);
        tb(H("nValues: "), data.nValues);
        tb(H("hasData: "), data.hasData);
        tb(H("nClusters: "), data.nClusters);
        return doc;
    }
};

#endif  // IVW_PCPDATA_H
