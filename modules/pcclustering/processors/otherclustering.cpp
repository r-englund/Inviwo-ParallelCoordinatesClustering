#include <modules/pcclustering/processors/otherclustering.h>

#include <modules/opengl/texture/textureutils.h>
#include <inviwo/core/util/clock.h>
#include <bitset>
#include <set>

#include <modules/pcclustering/ext/dbscan/dbscan.h>
#include <modules/pcclustering/ext/kmeans/src/kmeans.h>

namespace inviwo {

const ProcessorInfo PCPOtherClustering::processorInfo_{
    "pers.bock.PCPOtherClustering",  // Class identifier
    "Other Clustering Methods",            // Display name
    "Data Filtering",               // Category
    CodeState::Experimental,          // Code state
    Tags::CPU,                  // Tags
};

const ProcessorInfo PCPOtherClustering::getProcessorInfo() const {
    return processorInfo_;
}

PCPOtherClustering::PCPOtherClustering()
    : Processor()
    , _inport("in_data")
    , _dbscanPcpOutport("out_dbscan_pcp")
    , _dbscanColoringOutport("out_dbscan_coloring")
    , _kmeansPcpOutport("out_kmeans_pcp")
    , _kmeansColoringOutport("out_kmeans_coloring")
    , _dbscanMinClusters("dbscan_minclusters", "DBScan Minimum Clusters", 2, 0, 30)
    , _dbscanEpsilon("dbscan_epsilon", "DBScan Epsilon", 0.2f, 0.f, 1.f)
    , _kMeansk("kmeans_k", "K-Means K", 2, 0, 30)
    , _normalization("normalization", "Normalization", true)
    , _invalidate("_invalidate", "Invalidate")
{
    addPort(_inport);

    addPort(_dbscanPcpOutport);
    addPort(_dbscanColoringOutport);
    addPort(_kmeansPcpOutport);
    addPort(_kmeansColoringOutport);

    _dbscanMinClusters.onChange([this]() { _dbScanDirty = true; });
    addProperty(_dbscanMinClusters);
    _dbscanEpsilon.onChange([this]() { _dbScanDirty = true; });
    addProperty(_dbscanEpsilon);

    _kMeansk.onChange([this]() { _kmeansDirty = true; });
    addProperty(_kMeansk);

    _invalidate.onChange([this]() {
        _dbScanDirty = true;
        _kmeansDirty = true;
    });
    addProperty(_normalization);

    addProperty(_invalidate);
    _invalidate.onChange([this]() {
        _dbScanDirty = true;
        _kmeansDirty = true;
        invalidate(InvalidationLevel::InvalidOutput);
    });

    _dbScanData = std::make_shared<ParallelCoordinatesPlotData>();
    glGenBuffers(1, &(_dbScanData->ssboData));
    _dbScanColoringData = std::make_shared<ColoringData>();
    glGenBuffers(1, &_dbScanColoringData->ssboColor);


    _kmeansData = std::make_shared<ParallelCoordinatesPlotData>();
    glGenBuffers(1, &(_kmeansData->ssboData));
    _kmeansColoringData = std::make_shared<ColoringData>();
    glGenBuffers(1, &_kmeansColoringData->ssboColor);
}

PCPOtherClustering::~PCPOtherClustering() {}

#pragma optimize ( "", off)

void PCPOtherClustering::process() {
    std::shared_ptr<const ParallelCoordinatesPlotRawData> inData = _inport.getData();
    const int nValues = inData->data.size();
    const int nDimensions = inData->minMax.size();
    const int nItems = nValues / nDimensions;

    if (_dbScanDirty) {
        // Run DBScan
        std::vector<dbscan::Point> points;
        points.reserve(nItems);

        // Fill points
        for (int i = 0; i < nItems; ++i) {
            dbscan::Point p;
            p.clusterID = UNCLASSIFIED;
            for (int j = 0; j < nDimensions; ++j) {
                float v = inData->data[i * nDimensions + j];
                if (_normalization) {
                    v = (v - inData->minMax[j].first) / (inData->minMax[j].second - inData->minMax[j].first);
                }
                p.data.push_back(v);
            }
            points.push_back(p);
        }

        dbscan::DBSCAN dbscan(_dbscanMinClusters, _dbscanEpsilon, std::move(points));
        dbscan.run();

        // Count clusters
        std::set<int> clusters;
        for (const dbscan::Point& p : dbscan.m_points) {
            clusters.insert(p.clusterID);
        }
        const int nClusters = clusters.size();
        LogInfo("Number of clusters: " << clusters.size());

        // Move into outport
        _dbScanData->nValues = nValues;
        _dbScanData->nDimensions = nDimensions;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _dbScanData->ssboData);

        std::vector<float> data;
        for (const dbscan::Point& p : dbscan.m_points) {
            for (int i = 0; i < p.data.size(); ++i) {
                data.push_back(p.data[i]);
            }
        }
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            data.size() * sizeof(float),
            data.data(),
            GL_STATIC_DRAW
        );
        _dbscanPcpOutport.setData(_dbScanData);


        _dbScanColoringData->nValues = nValues;
        _dbScanColoringData->hasData = true;
        _dbScanColoringData->nClusters = nClusters;

        std::vector<int> clusterData;
        clusterData.reserve(nValues + 1);
        clusterData.push_back(nClusters);
        for (const dbscan::Point& p : dbscan.m_points) {
            clusterData.push_back(p.clusterID);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _dbScanColoringData->ssboColor);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            clusterData.size() * sizeof(int),
            clusterData.data(),
            GL_STATIC_DRAW
        );
        _dbscanColoringOutport.setData(_dbScanColoringData);

        _dbScanDirty = false;
    }

    if (_kmeansDirty) {
        // Run K-Means
        KMeans kmeans(_kMeansk);
        // Fill points
        std::vector<Point> points;
        points.reserve(nItems);
        for (int i = 0; i < nItems; ++i) {
            std::vector<double> values;
            for (int j = 0; j < nDimensions; ++j) {
                float v = inData->data[i * nDimensions + j];
                if (_normalization) {
                    v = (v - inData->minMax[j].first) / (inData->minMax[j].second - inData->minMax[j].first);
                }
                values.push_back(v);
            }

            Point p(values);
            points.push_back(p);
        }

        kmeans.init(points);
        kmeans.run();

        // Move into outport
        _kmeansData->nValues = nValues;
        _kmeansData->nDimensions = nDimensions;
        kmeans.getPoints();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _kmeansData->ssboData);
        std::vector<float> data;
        for (const Point& p : kmeans.getPoints()) {
            for (int i = 0; i < p.data_.size(); ++i) {
                data.push_back(p.data_[i]);
            }
        }
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            data.size() * sizeof(float),
            data.data(),
            GL_STATIC_DRAW
        );
        _kmeansPcpOutport.setData(_kmeansData);

        _kmeansColoringData->nClusters = _kMeansk;
        _kmeansColoringData->hasData = true;
        _kmeansColoringData->nValues = nValues;

        std::vector<int> clusterData;
        clusterData.reserve(nValues + 1);
        clusterData.push_back(_kMeansk);
        for (const Point& p : kmeans.getPoints()) {
            clusterData.push_back(p.cluster_);
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _kmeansColoringData->ssboColor);
        glBufferData(
            GL_SHADER_STORAGE_BUFFER,
            clusterData.size() * sizeof(int),
            clusterData.data(),
            GL_STATIC_DRAW
        );
        _kmeansColoringOutport.setData(_kmeansColoringData);

        _kmeansDirty = false;
    }
}

//PCPFiltering::PCPFiltering()
//    : Processor()
//    , _binInport("in_bins")
//    , _pcpInport("in_pcp")
//    , _pcpOutport("out_pcp")
//    , _coloringOutport("out_color")
//    , _coloredBinOutport("out_coloredBin")
//    , _coloringDimension("_coloringDimension", "Coloring Dimension", -1, -1, 15)
//    , _parallelismSlider("_parallelismSlider", "Parallelism Slider", 32, 1, 1024)
//    , _dimensionMaskString("_dimensionMask", "Dimension Mask")
//    , _invalidate("_invalidate", "Invalidate")
//    , _countingShader({{ShaderType::Compute, "pcpfiltering_counting.comp" }}, Shader::Build::No)
//    , _clusterDetectionShader({{ShaderType::Compute, "clusterdetection.comp" }}, Shader::Build::No)
//    , _filteringShader({ { ShaderType::Compute, "pcpfiltering_filtering.comp" } }, Shader::Build::No)
//{
//    addPort(_binInport);
//    addPort(_pcpInport);
//
//    addPort(_pcpOutport);
//    addPort(_coloringOutport);
//    addPort(_coloredBinOutport);
//
//    addProperty(_coloringDimension);
//    addProperty(_dimensionMaskString);
//    addProperty(_parallelismSlider);
//    _dimensionMaskString.onChange([this]() {
//        std::string s = _dimensionMaskString.get();
//        if (s.empty())
//            _dimensionMask = 0;
//        else {
//            std::bitset<32> mask(s);
//
//            _dimensionMask = static_cast<uint32_t>(mask.to_ulong());
//        }
//    });
//
//    _countingShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
//        "GL_ARB_compute_variable_group_size",
//        true
//    );
//
//    _clusterDetectionShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
//        "GL_ARB_compute_variable_group_size",
//        true
//        );
//
//    _filteringShader.getShaderObject(ShaderType::Compute)->addShaderExtension(
//        "GL_ARB_compute_variable_group_size",
//        true
//    );
//
//
//    _countingShader.build();
//    _clusterDetectionShader.build();
//    _filteringShader.build();
//
//    _countingShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
//    _clusterDetectionShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
//    _filteringShader.onReload([this]() {invalidate(InvalidationLevel::InvalidOutput); });
//
//    _pcpData = std::make_shared<ParallelCoordinatesPlotData>();
//    glGenBuffers(1, &_pcpData->ssboData);
//
//    glGenBuffers(1, &_nValuesCounter);
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _nValuesCounter);
//    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
//
//    glGenBuffers(1, &_memoryAccess);
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _memoryAccess);
//    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
//
//    _coloringData = std::make_shared<ColoringData>();
//    glGenBuffers(1, &_coloringData->ssboColor);
//
//    _coloredBinData = std::make_shared<ColoredBinData>();
//    glGenBuffers(1, &_coloredBinData->ssboIndices);
//
//    addProperty(_invalidate);
//    _invalidate.onChange([this]() {invalidate(InvalidationLevel::InvalidOutput); });
//    //glGenBuffers(1, &_nClustersBuffer);
//}
//
//PCPFiltering::~PCPFiltering() {
//    glDeleteBuffers(1, &_nValuesCounter);
//    glDeleteBuffers(1, &_memoryAccess);
//    //glDeleteBuffers(1, &_nClustersBuffer);
//}
//
//void PCPFiltering::process() {
//    if (!_binInport.hasData())
//        return;
//
//    if (!_pcpInport.hasData())
//        return;
//
//    std::shared_ptr<const BinningData> binInData = _binInport.getData();
//    std::shared_ptr<const ParallelCoordinatesPlotData> pcpInData = _pcpInport.getData();
//
//    _pcpData->nDimensions = pcpInData->nDimensions;
//
//    int dataValues = 0;
//    glFinish();
//    {
//        IVW_CPU_PROFILING("CountElements");
//        dataValues = countElements(pcpInData.get(), binInData.get());
//        glFinish();
//    }
//
//    if (_coloringDimension >= 0 && _coloringDimension < _pcpData->nDimensions) {
//        _pcpData->nValues = dataValues * _pcpData->nDimensions;
//        
//        _coloringData->nValues = dataValues;
//        
//        _coloredBinData->nBins = binInData->nBins;
//        _coloredBinData->selectedDimension = _coloringDimension;
//
//        glFinish();
//        {
//            IVW_CPU_PROFILING("ClusterDetection");
//            clusterDetection(binInData.get());
//            glFinish();
//        }
//
//        std::bitset<32> mask(_dimensionMask);
//        _coloringData->hasData = mask.test(_coloringDimension);
//        _coloredBinData->hasData = true;
//    }
//    else {
//        LogWarn("Skipping cluster detection of dimension: " << _coloringDimension);
//        _coloringData->hasData = false;
//
//    }
//    glFinish();
//    {
//        IVW_CPU_PROFILING("FilterData");
//        filterData(pcpInData.get(), binInData.get(), _pcpData.get(), dataValues);
//        glFinish();
//    }
//    
//
//    _pcpOutport.setData(_pcpData);
//    _coloringOutport.setData(_coloringData);
//    _coloredBinOutport.setData(_coloredBinData);
//}
//
//int PCPFiltering::countElements(const ParallelCoordinatesPlotData* inData,
//    const BinningData* binData)
//{
//    // First count the number of data values that will be written
//    _countingShader.activate();
//    _countingShader.setUniform("_nDimensions", inData->nDimensions);
//    _countingShader.setUniform("_nBins", binData->nBins);
//    uint32_t dm = _dimensionMask;
//    _countingShader.setUniform("_dimensionMask", dm);
//    _countingShader.setUniform("_nValues", inData->nValues);
//
//    // Storage for number of data values (not one value per dimension)
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _nValuesCounter);
//    GLuint* ptr = (GLuint*)glMapBufferRange(
//        GL_ATOMIC_COUNTER_BUFFER,
//        0,
//        sizeof(GLuint),
//        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
//        );
//    ptr[0] = 0;
//    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
//
//    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, _nValuesCounter);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inData->ssboData);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, binData->ssboBins);
//
//    glMemoryBarrier(GL_ALL_BARRIER_BITS);
//    glDispatchComputeGroupSizeARB(
//        int(ceil((inData->nValues / inData->nDimensions) / float(_parallelismSlider))), 1, 1,
//        _parallelismSlider, 1, 1
//        );
//    glMemoryBarrier(GL_ALL_BARRIER_BITS);
//
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _nValuesCounter);
//    int* p = (int*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
//    int nDataValuesPositive = p[0];
//    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
//
//    _countingShader.deactivate();
//
//    LogInfo("Number of accepted values: " << nDataValuesPositive);
//    return nDataValuesPositive;
//
//}
//
//void PCPFiltering::clusterDetection(const BinningData* binData) {
//    // Generate a buffer that stores for a single dimension the IDs in each bin by
//    // doing a connected component analysis
//
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _coloredBinData->ssboIndices);
//    glBufferData(
//        GL_SHADER_STORAGE_BUFFER,
//        (binData->nBins + 1) * sizeof(int),  // +1 for number of clusters
//        nullptr,
//        GL_DYNAMIC_DRAW
//    );
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//
//    _clusterDetectionShader.activate();
//
//    _clusterDetectionShader.setUniform("_nBins", binData->nBins);
//    _clusterDetectionShader.setUniform("_nDimensions", binData->nDimensions);
//    _clusterDetectionShader.setUniform("_selectedDimension", _coloringDimension);
//    
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, binData->ssboBins);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _coloredBinData->ssboIndices);
//
//    glMemoryBarrier(GL_ALL_BARRIER_BITS);
//    glDispatchComputeGroupSizeARB(
//        1, 1, 1,
//        1, 1, 1
//        );
//    glMemoryBarrier(GL_ALL_BARRIER_BITS);
//
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _coloredBinData->ssboIndices);
//    int* p = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
//    int nClusters = p[0];
//    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//
//    LogInfo("Number of Clusters: " << nClusters);
//
//    _coloringData->nClusters = nClusters;
//    _coloredBinData->nClusters = nClusters;
//
//    _clusterDetectionShader.deactivate();
//}
//
//void PCPFiltering::filterData(const ParallelCoordinatesPlotData* inData,
//                              const BinningData* binData,
//                              ParallelCoordinatesPlotData* outData, int nElements
//    )
//{
//    _filteringShader.activate();
//    _filteringShader.setUniform("_nDimensions", inData->nDimensions);
//    _filteringShader.setUniform("_nBins", binData->nBins);
//    uint32_t dm = _dimensionMask;
//    _filteringShader.setUniform("_dimensionMask", dm);
//    _filteringShader.setUniform("_nValues", inData->nValues);
//
//    _filteringShader.setUniform("_hasColoringInformation", _coloringData->hasData);
//
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outData->ssboData);
//    glBufferData(
//        GL_SHADER_STORAGE_BUFFER,
//        outData->nValues * sizeof(float),
//        nullptr,
//        GL_DYNAMIC_DRAW
//    );
//
//    if (_coloringData->hasData) {
//        _filteringShader.setUniform("_selectedDimension", _coloringDimension);
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _coloringData->ssboColor);
//        glBufferData(
//            GL_SHADER_STORAGE_BUFFER,
//            (_coloringData->nValues + 1) * sizeof(int), //+1 for the number of clusters
//            nullptr,
//            GL_DYNAMIC_DRAW
//        );
//    }
//
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//    
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, _memoryAccess);
//    GLuint* ptr = (GLuint*)glMapBufferRange(
//        GL_ATOMIC_COUNTER_BUFFER,
//        0,
//        sizeof(GLuint),
//        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
//    );
//    ptr[0] = 0;
//    glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
//    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
//
//    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, _memoryAccess);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inData->ssboData);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, binData->ssboBins);
//    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, outData->ssboData);
//
//    if (_coloringData->hasData) {
//        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _coloredBinData->ssboIndices);
//        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, _coloringData->ssboColor);
//    }
//
//    glMemoryBarrier(GL_ALL_BARRIER_BITS);
//    glDispatchComputeGroupSizeARB(
//        int(ceil((inData->nValues / inData->nDimensions) / float(_parallelismSlider))), 1, 1,
//        _parallelismSlider, 1, 1
//        );
//    glMemoryBarrier(GL_ALL_BARRIER_BITS);
//
//    if (_coloringData->hasData) {
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _coloringData->ssboColor);
//        int* p = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
//        p[0] = _coloredBinData->nClusters;
//        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//    }
//
//    _filteringShader.deactivate();
//}

}  // namespace

