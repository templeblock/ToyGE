INCLUDE(FindPackageHandleStandardArgs)

IF(NOT FreeType_INCLUDE_DIR)
FIND_PATH(FreeType_INCLUDE_DIR 
    NAMES FreeType.h)
ENDIF()
  
IF(TARGET_64)
FIND_LIBRARY(FreeType_LIBRARY_DEBUG 
    NAMES FreeType_64d
    PATHS ${FreeType_INCLUDE_DIR}/../lib)
    
FIND_LIBRARY(FreeType_LIBRARY_RELEASE 
    NAMES FreeType_64 
    PATHS ${FreeType_INCLUDE_DIR}/../lib)
ELSE()
FIND_LIBRARY(FreeType_LIBRARY_DEBUG 
    NAMES FreeType_d
    PATHS ${FreeType_INCLUDE_DIR}/../lib)
    
FIND_LIBRARY(FreeType_LIBRARY_RELEASE 
    NAMES FreeType 
    PATHS ${FreeType_INCLUDE_DIR}/../lib)
ENDIF()
    
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeType 
    DEFAULT_MSG 
    FreeType_INCLUDE_DIR 
    FreeType_LIBRARY_DEBUG
    FreeType_LIBRARY_RELEASE)
    
IF(FREETYPE_FOUND)
    set(FreeType_LIBRARY optimized ${FreeType_LIBRARY_RELEASE} debug ${FreeType_LIBRARY_DEBUG} CACHE STRING "")
    mark_as_advanced(FreeType_LIBRARY_RELEASE FreeType_LIBRARY_DEBUG FreeType_BINARY_RELEASE)
ENDIF()