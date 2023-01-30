# Author: Michael Michaelou
# Github Username: MichaelMichael1997

cmake_minimum_required(VERSION 3.13)

project(arax_server VERSION 1.0.0)

message(STATUS "${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Generated with build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-Wall -Wextra -pedantic -g)

find_package(arax CONFIG REQUIRED)
find_package(Threads)

# -- Uncomment if gRPC added as dependency with Fetch Content
# set(GRPC_DEPS ../build/_deps)
# set(GRPC_BUILD ../build/_deps/grpc-build)
# set(GRPC_SUBBUILD ../build/_deps/grpc-subbuild)
# set(GRPC_SRC ../build/_deps/grpc-src)

# add_subdirectory(${GRPC_SRC} ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)

# -----------------------------------------------------------

# -- Uncomment if gRPC installed --
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${grpc_VERSION}")
# ---------------------------------

include_directories("${CMAKE_CURRENT_BINARY_DIR}/../../generated")
include_directories(../../../build/include) # Arax Library

# Generated sources
set(protos_srcs "${CMAKE_CURRENT_BINARY_DIR}/../../generated/arax.pb.cc")
set(protos_hdrs "${CMAKE_CURRENT_BINARY_DIR}/../../generated/arax.pb.h")
set(grpc_srcs   "${CMAKE_CURRENT_BINARY_DIR}/../../generated/arax.grpc.pb.cc")
set(grpc_hdrs   "${CMAKE_CURRENT_BINARY_DIR}/../../generated/arax.grpc.pb.h")

# grpc_protos
add_library(grpc_protos
            ${protos_srcs}
            ${protos_hdrs}
            ${grpc_srcs}
            ${grpc_hdrs}
  )

add_executable(server server.cpp)
 target_link_libraries(server
                        arax
                        grpc_protos
                        pthread
                        ${_PROTOBUF_LIBPROTOBUF}
                        ${_REFLECTION}
                        ${_GRPC_GRPCPP}
)
