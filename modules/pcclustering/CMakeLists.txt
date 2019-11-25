#--------------------------------------------------------------------
# ABufferGL Module
ivw_module(PCClustering)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES
    datastructures/pcpdata.h
    ext/kmeans/src/kmeans.h
    misc/cpr.h
    misc/parallelcoordinates_axis_permutation.h
    misc/support.h
    processors/clusterexport.h
    processors/clusterrenderer.h
    processors/dataframetopcprawdata.h
    processors/densitymapfiltering.h
    processors/densitymapgenerator.h
    processors/densitymaprenderer.h
    processors/otherclustering.h
    processors/pcpfileconverter.h
    processors/pcpfiltering.h
    processors/pcpgui.h
    processors/pcpreader.h
    processors/pcprenderer.h
    processors/pcpupload.h
    processors/pcpuploadrenderer.h
    processors/radarplotrenderer.h
    processors/scatterplotrenderer.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    datastructures/pcpdata.cpp
    ext/dbscan/dbscan.cpp
    ext/kmeans/src/kmeans.cpp
    ext/kmeans/src/point.cpp
    misc/parallelcoordinates_axis_permutation.cpp
    misc/support.cpp
    processors/clusterexport.cpp
    processors/clusterrenderer.cpp
    processors/dataframetopcprawdata.cpp
    processors/densitymapfiltering.cpp
    processors/densitymapgenerator.cpp
    processors/densitymaprenderer.cpp
    processors/otherclustering.cpp
    processors/pcpfileconverter.cpp
    processors/pcpfiltering.cpp
    processors/pcpgui.cpp
    processors/pcpreader.cpp
    processors/pcprenderer.cpp
    processors/pcpupload.cpp
    processors/pcpuploadrenderer.cpp
    processors/radarplotrenderer.cpp
    processors/scatterplotrenderer.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/clusterdetection.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/clusterrenderer.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/densitymapcounter.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/densitymapfiltering_percentage.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/densitymapfiltering_topology.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/densitymapgenerator.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/densitymaprenderer.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcpfiltering_counting.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcpfiltering_filtering.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcprenderer.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcprenderer.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/radarplotlines.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/radarplotrenderer.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/radarplotrenderer.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/scatterplotrenderer.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/scatterplotrenderer.frag
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_compile_definitions(inviwo-module-pcclustering PRIVATE __CL_ENABLE_EXCEPTIONS)

set_property(TARGET inviwo-module-pcclustering PROPERTY CXX_STANDARD 20)

set_target_properties(inviwo-module-pcclustering PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)

#--------------------------------------------------------------------
# Handle external resources
if(IVW_USE_GENERATED_RESOURCES)
#--------------------------------------------------------------------
# Generate shader resource (if GENERATE_SHADER_RESOURCES is checked)
    ivw_generate_shader_resource(${CMAKE_CURRENT_SOURCE_DIR}/glsl ${SHADER_FILES})
else()
#--------------------------------------------------------------------
# Add shader directory to pack
    ivw_add_to_module_pack(${CMAKE_CURRENT_SOURCE_DIR}/glsl)
endif()
