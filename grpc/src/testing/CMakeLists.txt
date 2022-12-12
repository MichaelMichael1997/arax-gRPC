# Build the tests
# find_library(NOOPLIB nooplib ${CMAKE_CURRENT_BINARY_DIR} nooplib)

# Set the path to the Arax controller include directory
set(ARAX_CONTROLLER_PATH ${CMAKE_CURRENT_BINARY_DIR}/../../../../controller)

add_library(nooplib SHARED noop_grpc.cpp)
target_include_directories(nooplib PRIVATE ${ARAX_CONTROLLER_PATH}/include)
target_compile_definitions(nooplib PRIVATE BUILD_SO)
target_link_libraries(nooplib arax)

message(STATUS "ARAX CONTROLLER PATH: ${ARAX_CONTROLLER_PATH}")

# Create the executable and create link the library
foreach(_target simple_grpc clean_arax data noop_grpc)
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
            nooplib
            # ${NOOPLIB}
			      ${_REFLECTION}
			      ${_GRPC_GRPCPP}
			      ${_PROTOBUF_LIBPROTOBUF}
			      ${ARAX_API}
			     )
endforeach()
