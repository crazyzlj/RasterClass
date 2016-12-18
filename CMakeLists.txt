# CMake build for Windows, Linux, and macOS
# Updated by Liang-Jun Zhu
# zlj@lreis.ac.cn
cmake_minimum_required(VERSION 2.8)
project(RasterClass)
IF (COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0015 NEW)
ENDIF (COMMAND CMAKE_POLICY)
SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})
################# OS platform   ##################
if (WIN32)
    add_definitions(-Dwindows)
    if (MSVC)
        add_definitions(-DMSVC)
    endif ()
elseif (APPLE)
    add_definitions(-Dmacos)
else ()
    add_definitions(-Dlinux)
endif ()
################# Find packages ##################
# GDAL is required
IF (WIN32)
    SET(GDAL_DIR $ENV{GDAL_DIR})
    SET(GDAL_DIR $ENV{GDAL_DIR})
    SET(GDAL_INCLUDE_DIR ${GDAL_DIR}\\include)
    SET(GDAL_LIBRARIES ${GDAL_DIR}\\lib)
    SET(GDAL_BIN_DIR ${GDAL_DIR}\\bin)
    SET(GDAL_LIBRARY ${GDAL_LIBRARIES}\\gdal_i.lib)
ELSE ()
    # CMake will find GDAL and get GDAL_INCLUDE_DIR and GDAL_LIBRARY.
    find_package(GDAL REQUIRED)
ENDIF ()
# Bson and MongoC are optional
include(FindBson.cmake)
include(FindMongoC.cmake)
################# Packages found ##################
set(SOURCE_FILES main.cpp clsRasterData.cpp)

set(UTILS_INC ../UtilsClass)
set(UTILS_FILES ../UtilsClass/utils.cpp ../UtilsClass/util.cpp ../UtilsClass/StringList.cpp ../UtilsClass/ModelException.cpp)
if (BSON_FOUND AND MONGOC_FOUND)
    set(MONGO_INC ../MongoUtilClass)
    set(MONGO_FILES ${MONGO_INC}/MongoUtil.cpp)
    include_directories(${GDAL_INCLUDE_DIR} ${BSON_INCLUDE_DIR} ${MONGOC_INCLUDE_DIR} ${UTILS_INC} ${MONGO_INC})
    add_executable(RasterClass ${SOURCE_FILES} ${UTILS_FILES} ${MONGO_FILES})
    target_link_libraries(RasterClass ${BSON_LIBRARIES} ${MONGOC_LIBRARIES} ${GDAL_LIBRARY})
else ()
    include_directories(${GDAL_INCLUDE_DIR} ${UTILS_INC})
    add_executable(RasterClass ${SOURCE_FILES} ${UTILS_FILES})
    target_link_libraries(RasterClass ${GDAL_LIBRARY})
endif ()
install(TARGETS ${RasterClass} DESTINATION bin)
