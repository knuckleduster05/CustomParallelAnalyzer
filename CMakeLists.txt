cmake_minimum_required (VERSION 3.11)
project(simple_parallel_analyzer)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES 
src/ParallelCSAnalyzer.cpp
src/ParallelCSAnalyzer.h
src/ParallelCSAnalyzerResults.cpp
src/ParallelCSAnalyzerResults.h
src/ParallelCSAnalyzerSettings.cpp
src/ParallelCSAnalyzerSettings.h
src/ParallelCSSimulationDataGenerator.cpp
src/ParallelCSSimulationDataGenerator.h
)

add_analyzer_plugin(simple_parallel_analyzer SOURCES ${SOURCES})
