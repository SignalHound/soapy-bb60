###############################################################
# Build Soapy SDR support module for Signal Hound BB60C Devices
###############################################################

cmake_minimum_required(VERSION 2.8.7)

project(SoapyBB60 CXX)

find_package(SoapySDR "0.4.0" NO_MODULE REQUIRED)
if (NOT SoapySDR_FOUND)
    message(FATAL_ERROR "Soapy SDR development files not found...")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

find_package(LibBB60)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${BB60_INCLUDE_DIRS})

#enable c++11 features
if(CMAKE_COMPILER_IS_GNUCXX)
    #C++11 is a required language feature for this project
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_STD_CXX11)
    if(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else(HAS_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()
    #Thread support enabled (not the same as -lpthread)
    list(APPEND BB60_LIBS -pthread)
    #disable warnings for unused parameters
    add_definitions(-Wno-unused-parameter)
endif(CMAKE_COMPILER_IS_GNUCXX)

list(APPEND BB60C_LIBS ${BB60C_LIBRARIES})

SOAPY_SDR_MODULE_UTIL(
    TARGET bb60Support
    SOURCES
    	src/SoapyBB60.hpp
        src/Registration.cpp
        src/Settings.cpp
        src/Streaming.cpp
        src/Sensors.cpp
    LIBRARIES
        ${BB60C_LIBS}
)
