#-------------------------------------------------------------------------------
# terrainaddon
#-------------------------------------------------------------------------------

# glob sources and put them in folders
FILE(GLOB SRC "*.cc")
FILE(GLOB HEAD "*.h")
LIST(APPEND SRCS ${SRC} ${HEAD})
SOURCE_GROUP("terrainaddon" FILES ${SRCS})

SET(PCH_files	
    ../stdneb.cc
)
SOURCE_GROUP("Precompiled" FILES ${PCH_files})

LIST(APPEND ALLSRCS ${SRCS} ${PCH_files})

ADD_LIBRARY(terrainaddon STATIC ${ALLSRCS})
SET_TARGET_PROPERTIES(terrainaddon PROPERTIES FOLDER "N3SDK/addons")

# link to render, will pull in dependencies and includes
TARGET_LINK_LIBRARIES(terrainaddon PUBLIC render)

# default nebula compiler settings
N_SET_COMPILER_DEFAULTS()
IF(N_USE_PRECOMPILED_HEADERS)
	N_SET_PCH(terrainaddon stdneb.h stdneb.cc ../)
ENDIF()