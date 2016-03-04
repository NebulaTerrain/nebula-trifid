#-------------------------------------------------------------------------------
# terrainrenderer
#-------------------------------------------------------------------------------
	
set(SOURCES_files_terrainrenderer
	terrainrenderer.h
	terrainrenderer.cc
)
source_group("terrainrenderer" FILES ${SOURCES_files_grid})

set(SOURCES_terrainrenderer
    ${SOURCES_files_terrainrenderer}
)

set(SOURCES_files
../stdneb.h
../stdneb.cc
)
	
SOURCE_GROUP("Precompiled" FILES ${SOURCES_files})
set(SOURCES_SRCS
	${SOURCES_terrainrenderer}
	${SOURCES_files}
)

add_library( terrainrenderer STATIC ${SOURCES_SRCS} )
TARGET_LINK_LIBRARIES(terrainrenderer render)
SET_TARGET_PROPERTIES(terrainrenderer PROPERTIES FOLDER "N3SDK/addons")
N_SET_COMPILER_DEFAULTS()
IF(N_USE_PRECOMPILED_HEADERS)
    N_SET_PCH(terrainrenderer stdneb.h stdneb.cc ../)
ENDIF()
