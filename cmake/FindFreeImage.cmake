INCLUDE(FindPackageHandleStandardArgs)

IF(NOT FreeImage_INCLUDE_DIR)
FIND_PATH(FreeImage_INCLUDE_DIR 
    NAMES FreeImage.h)
ENDIF()

IF(TARGET_64)
FIND_LIBRARY(FreeImage_LIBRARY_DEBUG 
    NAMES FreeImage
    PATHS ${FreeImage_INCLUDE_DIR}/../lib)
    
FIND_LIBRARY(FreeImage_LIBRARY_RELEASE 
    NAMES FreeImage
    PATHS ${FreeImage_INCLUDE_DIR}/../lib)
    
FIND_FILE(FreeImage_BINARY_DEBUG
    NAMES FreeImage.dll
    PATHS ${FreeImage_INCLUDE_DIR}/../bin)
    
FIND_FILE(FreeImage_BINARY_RELEASE
    NAMES FreeImage.dll
    PATHS ${FreeImage_INCLUDE_DIR}/../bin)
ELSE()
FIND_LIBRARY(FreeImage_LIBRARY_DEBUG 
    NAMES FreeImage
    PATHS ${FreeImage_INCLUDE_DIR}/../lib)
    
FIND_LIBRARY(FreeImage_LIBRARY_RELEASE 
    NAMES FreeImage 
    PATHS ${FreeImage_INCLUDE_DIR}/../lib)
   
FIND_FILE(FreeImage_BINARY_DEBUG
    NAMES FreeImage.dll
    PATHS ${FreeImage_INCLUDE_DIR}/../bin)
    
FIND_FILE(FreeImage_BINARY_RELEASE
    NAMES FreeImage.dll
    PATHS ${FreeImage_INCLUDE_DIR}/../bin)
ENDIF()
    
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FreeImage 
    DEFAULT_MSG 
    FreeImage_INCLUDE_DIR 
    FreeImage_LIBRARY_DEBUG
    FreeImage_LIBRARY_RELEASE)
    
IF(FREEIMAGE_FOUND)
    set(FreeImage_LIBRARY optimized ${FreeImage_LIBRARY_RELEASE} debug ${FreeImage_LIBRARY_DEBUG} CACHE STRING "")
    mark_as_advanced(FreeImage_LIBRARY_RELEASE FreeImage_LIBRARY_DEBUG FreeImage_BINARY_RELEASE)
ENDIF()