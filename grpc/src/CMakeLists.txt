# Generate the *.cc, *.h files from the protocol buffers, the Arax client API
# as well as the tests

# This CMakeLists is largely inspired by the gRPC C++ examples in the gRPC Github repository
# https://github.com/grpc/grpc/blob/master/examples/cpp/route_guide/CMakeLists.txt

# Author: Michael Michaelou
# Github Username: MichaelMichael1997

cmake_minimum_required(VERSION 3.13)

# project name
project(g_arax VERSION 1.0.0)

message(STATUS "${PROJECT_NAME} ${PROJECT_VERSION}")

set(ARAX ../../build)
set(CONTROLLER ../../controller/build)

if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
  message(STATUS "BUILD TYPE: DEBUG")
  set(DEBUG 1)
endif()

# -- Add compile options --
add_compile_options(-Wall -Wextra -pedantic -g)

# We nedd to generate the c++ files from the protocol buffers first
# Find the necessary packages
find_package(arax CONFIG REQUIRED)
find_library(ARAX_LIB arax_st)

message(STATUS "FOUND ARAX LIB ${ARAX_LIB}")

if(arax_FOUND)
    message(STATUS "Found libarax")
endif()

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

find_package(Threads)

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
message(STATUS "Found gRPC C++ plugin ${_GRPC_CPP_PLUGIN_EXECUTABLE}")

# proto files
get_filename_component(protos "../protos/arax.proto" ABSOLUTE)
get_filename_component(protos_path "${protos}" PATH)

message(STATUS "Protocol Buffers Path: ${protos_path}")

# Generated sources
set(protos_srcs "${CMAKE_CURRENT_BINARY_DIR}/../generated/arax.pb.cc")
set(protos_hdrs "${CMAKE_CURRENT_BINARY_DIR}/../generated/arax.pb.h")
set(grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/../generated/arax.grpc.pb.cc")
set(grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/../generated/arax.grpc.pb.h")

add_custom_command(
    OUTPUT "${protos_srcs}" "${protos_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}/../generated"
         --cpp_out "${CMAKE_CURRENT_BINARY_DIR}/../generated"
         -I "${protos_path}"
         --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
         "${protos}"
    DEPENDS "${protos}"
  )
  
# Include the build/include Arax directory in the build path
# include_directories(../../build/include)
# include_directories(../../build/src)
# include_directories(../../controller/build/include)
# include_directories(../../controller/src)


# Declare the library target
add_library(${PROJECT_NAME} SHARED
	    arax_grpc_client/arax_grpc_client.cpp
	    generated/arax.grpc.pb.cc
	    generated/arax.pb.cc
        ${ARAX}/src
        ${CONTROLLER}/src

	   )

# Configure the directories to search for header files
target_include_directories(${PROJECT_NAME} PRIVATE arax_grpc_client)
target_include_directories(${PROJECT_NAME} PRIVATE generated)
target_include_directories(${PROJECT_NAME} PRIVATE ${ARAX}/build/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CONTROLLER}/build/include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${ARAX_LIB}
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
        )
target_link_libraries(${PROJECT_NAME} PRIVATE arax)

# Set the version properly
# set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

# Set the shared object version property to the project's major version
# set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

# Set the public header property to the one with the actual API.
# set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER client/client.h)

# -- Build the tests --
add_subdirectory(testing)

# TODO: Later maybe install library system wide
