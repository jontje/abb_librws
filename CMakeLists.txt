cmake_minimum_required(VERSION 3.5)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/ExtractMetaData.cmake)
project(${EXTRACTED_NAME} VERSION ${EXTRACTED_VERSION} LANGUAGES CXX)

#######################
## Find dependencies ##
#######################
##--------------------##
## POCO C++ Libraries ##
##--------------------##
# We need at least 1.4.3 because of WebSocket support.
find_package(Poco 1.4.3 REQUIRED COMPONENTS Foundation Net Util XML)

###############
## Pre-build ##
###############
include(GNUInstallDirs)
include(GenerateExportHeader)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(NOT DEFINED BUILD_SHARED_LIBS)
  option(BUILD_SHARED_LIBS "Build dynamically-linked binaries" ON)
endif()

###########
## Build ##
###########
set(
  SRC_FILES
    src/rws_client.cpp
    src/rws_common.cpp
    src/rws_interface.cpp
    src/rws_poco_client.cpp
    src/rws_rapid.cpp
    src/rws_state_machine_interface.cpp
)

add_library(${PROJECT_NAME} ${SRC_FILES})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

generate_export_header(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  Poco::Foundation
  Poco::Net
  Poco::Util
  Poco::XML
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(${PROJECT_NAME} PUBLIC "ABB_LIBRWS_STATIC_DEFINE")
endif()

################
## Post-build ##
################
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/PostBuild.cmake)
