# Build the server executable

# This CMakeLists is largely inspired by the gRPC C++ examples in the gRPC Github repository
# https://github.com/grpc/grpc/blob/master/examples/cpp/route_guide/CMakeLists.txt

# Author: Michael Michaelou
# Github Username: MichaelMichael1997

cmake_minimum_required(VERSION 3.13)

# project name
project(arax_server VERSION 1.0.0)

message(STATUS "${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Generated with build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 11)

# Set DEBUG flag
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Add compile options
add_compile_options(-Wall -Wextra -pedantic)

# Find the neccessary packages
find_package(arax CONFIG REQUIRED)
find_package(Threads)

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

include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../generated")

# Include the build/include directory in the build path
include_directories(../../../build/include)


# Generated sources
set(protos_srcs "${CMAKE_CURRENT_BINARY_DIR}/../../generated/arax.pb.cc")
set(protos_hdrs "${CMAKE_CURRENT_BINARY_DIR}/../../generated/arax.pb.h")
set(grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/../../generated/arax.grpc.pb.cc")
set(grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/../../generated/arax.grpc.pb.h")

# grpc_protos
add_library(grpc_protos
            ${protos_srcs}
            ${protos_hdrs}
            ${grpc_srcs}
            ${grpc_hdrs}
  )

# Targets (Client/Server)
 foreach(_target server)
   add_executable(${_target}
     "${_target}.cpp" "${CPP_SOURCES}"
    )
 target_link_libraries(${_target}
                        arax
                        grpc_protos
                        pthread
                        ${_REFLECTION}
                        ${_GRPC_GRPCPP}
                        ${_PROTOBUF_LIBPROTOBUF}
    )
endforeach()
