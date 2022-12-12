# Build the tests
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
endforeach()

