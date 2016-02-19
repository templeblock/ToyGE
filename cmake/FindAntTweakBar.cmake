INCLUDE(FindPackageHandleStandardArgs)

IF(NOT AntTweakBar_INCLUDE_DIR)
FIND_PATH(AntTweakBar_INCLUDE_DIR 
    NAMES AntTweakBar.h)
ENDIF()

IF(TARGET_64)    
FIND_LIBRARY(AntTweakBar_LIBRARY_DEBUG 
    NAMES AntTweakBar64
    PATHS ${AntTweakBar_INCLUDE_DIR}/../lib)
    
FIND_LIBRARY(AntTweakBar_LIBRARY_RELEASE 
    NAMES AntTweakBar64 
    PATHS ${AntTweakBar_INCLUDE_DIR}/../lib)
    
FIND_FILE(AntTweakBar_BINARY_DEBUG
    NAMES AntTweakBar64.dll
    PATHS ${AntTweakBar_INCLUDE_DIR}/../bin)
    
FIND_FILE(AntTweakBar_BINARY_RELEASE
    NAMES AntTweakBar64.dll
    PATHS ${AntTweakBar_INCLUDE_DIR}/../bin)
ELSE()
FIND_LIBRARY(AntTweakBar_LIBRARY_DEBUG 
    NAMES AntTweakBar
    PATHS ${AntTweakBar_INCLUDE_DIR}/../lib)
    
FIND_LIBRARY(AntTweakBar_LIBRARY_RELEASE 
    NAMES AntTweakBar 
    PATHS ${AntTweakBar_INCLUDE_DIR}/../lib)

FIND_FILE(AntTweakBar_BINARY_DEBUG
    NAMES AntTweakBar.dll
    PATHS ${AntTweakBar_INCLUDE_DIR}/../bin)

FIND_FILE(AntTweakBar_BINARY_RELEASE
    NAMES AntTweakBar.dll
    PATHS ${AntTweakBar_INCLUDE_DIR}/../bin)
ENDIF()
    
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AntTweakBar 
    DEFAULT_MSG 
    AntTweakBar_INCLUDE_DIR 
    AntTweakBar_LIBRARY_DEBUG
    AntTweakBar_LIBRARY_RELEASE)
    
IF(ANTTWEAKBAR_FOUND)
    set(AntTweakBar_LIBRARY optimized ${AntTweakBar_LIBRARY_RELEASE} debug ${AntTweakBar_LIBRARY_DEBUG} CACHE STRING "")
    mark_as_advanced(AntTweakBar_LIBRARY_RELEASE AntTweakBar_LIBRARY_DEBUG AntTweakBar_BINARY_RELEASE)
ENDIF()