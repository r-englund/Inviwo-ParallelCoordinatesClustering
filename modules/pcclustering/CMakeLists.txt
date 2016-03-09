#--------------------------------------------------------------------
# ABufferGL Module
ivw_module(PCClustering)

#--------------------------------------------------------------------
# Add header files
set(HEADER_FILES   
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/pcpdata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/densitymapfiltering.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/densitymapgenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/densitymaprenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pcpfiltering.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pcpreader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pcprenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pcpupload.h
)
ivw_group("Header Files" ${HEADER_FILES})

#--------------------------------------------------------------------
# Add source files
set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/datastructures/pcpdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/densitymapfiltering.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/densitymapgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/densitymaprenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pcpfiltering.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pcpreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pcprenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/processors/pcpupload.cpp
)
ivw_group("Source Files" ${SOURCE_FILES})

#--------------------------------------------------------------------
# Add shaders
set(SHADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcphelper.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/densitymapgenerator.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/densitymaprenderer.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcprenderer.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/glsl/pcprenderer.frag
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffergeometrygl.frag
    #ABUFFER
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abufferlinkedlist.hglsl
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abuffersort.hglsl
    #${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abuffer.hglsl
	#${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abufferresolve.hglsl
	#${CMAKE_CURRENT_SOURCE_DIR}/glsl/abuffer/abufferreset.hglsl	
)
ivw_group("Shader Files" ${SHADER_FILES})

#--------------------------------------------------------------------
# Add definition
ivw_add_definition(__CL_ENABLE_EXCEPTIONS)

#--------------------------------------------------------------------
# Create module
ivw_create_module(${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

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