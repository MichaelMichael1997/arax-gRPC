# Build the testing executables
cmake_minimum_required(VERSION 3.5.1)

project(arax_api_testing VERSION 1.0.0)

set(ARAX_CONTROLLER_PATH ../../../controller/include)
message(STATUS "ARAX CONTROLLER PATH: ${ARAX_CONTROLLER_PATH}")

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

find_package(Threads)

# Add compile options
add_compile_options(-Wall -Wextra -pedantic)

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

list(APPEND CMAKE_PREFIX_PATH "../build")

# Find the arax_api library
find_library(ARAX_API arax_api)

add_library(testlib EXCLUDE_FROM_ALL SHARED data.cpp)
target_include_directories(testlib PRIVATE ${ARAX_CONTROLLER_PATH}/include)
target_include_directories(testlib PRIVATE ../../../build/include)
target_compile_definitions(testlib PRIVATE BUILD_SO)
target_link_libraries(testlib arax)

# Create the executable and create link the library
foreach(_target simple simple_grpc clean_arax data noop_grpc)
	add_executable(${_target}
			"${_target}.cpp" 
		      )
	# include path to arax include files
	target_include_directories(${_target} PRIVATE ../../../build/include)
  target_include_directories(${_target} PRIVATE ${ARAX_CONTROLLER_PATH}/include)	
  target_compile_definitions(${_target} PRIVATE BUILD_MAIN)
	target_link_libraries(${_target}
			      arax
			      pthread
			      ${_REFLECTION}
			      ${_GRPC_GRPCPP}
			      ${_PROTOBUF_LIBPROTOBUF}
			      ${ARAX_API}
			     )
  add_dependencies(${_target} testlib)
endforeach()
























