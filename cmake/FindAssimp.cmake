INCLUDE(FindPackageHandleStandardArgs)

IF(NOT Assimp_INCLUDE_DIR)
FIND_PATH(Assimp_INCLUDE_DIR 
    NAMES assimp.h)
ENDIF()
  
IF(TARGET_64)
FIND_LIBRARY(Assimp_LIBRARY_DEBUG 
    NAMES assimpd
    PATHS ${Assimp_INCLUDE_DIR}/../lib)
    
FIND_LIBRARY(Assimp_LIBRARY_RELEASE 
    NAMES assimp
    PATHS ${Assimp_INCLUDE_DIR}/../lib)
    
FIND_FILE(Assimp_BINARY_DEBUG
    NAMES assimpd.dll
    PATHS ${Assimp_INCLUDE_DIR}/../bin)
    
FIND_FILE(Assimp_BINARY_RELEASE
    NAMES assimp.dll
    PATHS ${Assimp_INCLUDE_DIR}/../bin)
ELSE()
FIND_LIBRARY(Assimp_LIBRARY_DEBUG 
    NAMES assimpd
    PATHS ${Assimp_INCLUDE_DIR}/../lib)
    
FIND_LIBRARY(Assimp_LIBRARY_RELEASE 
    NAMES assimp 
    PATHS ${Assimp_INCLUDE_DIR}/../lib)
    
FIND_FILE(Assimp_BINARY_DEBUG
    NAMES assimpd.dll
    PATHS ${Assimp_INCLUDE_DIR}/../bin)
    
FIND_FILE(Assimp_BINARY_RELEASE
    NAMES assimp.dll
    PATHS ${Assimp_INCLUDE_DIR}/../bin)
ENDIF()
    
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Assimp 
    DEFAULT_MSG 
    Assimp_INCLUDE_DIR 
    Assimp_LIBRARY_DEBUG
    Assimp_LIBRARY_RELEASE)
    
IF(ASSIMP_FOUND)
    set(Assimp_LIBRARY optimized ${Assimp_LIBRARY_RELEASE} debug ${Assimp_LIBRARY_DEBUG} CACHE STRING "")
    mark_as_advanced(Assimp_LIBRARY_RELEASE Assimp_LIBRARY_DEBUG Assimp_BINARY_RELEASE)
ENDIF()