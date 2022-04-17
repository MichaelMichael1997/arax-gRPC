option(COVERAGE "Enable coverage reports" OFF)

if(COVERAGE)
  set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_C_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage ${CMAKE_EXE_LINKER_FLAGS}")
  set(CMAKE_C_OUTPUT_EXTENSION_REPLACE ON)
  set(BUILD_TESTS 1)
endif()

if(COVERAGE)
  add_custom_target(
    coverage_pre
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND make -j
    COMMAND -make -j test
    COMMAND -rm -Rf coverage
    COMMAND mkdir coverage
    COMMAND
      /usr/bin/gcovr -p -d --gcov-ignore-parse-errors --exclude-directories
      'tests' -r ${CMAKE_CURRENT_SOURCE_DIR}/ --html-title
      'VineTalk Coverage Report' --html --html-details --html-self-contained -o coverage/coverage.html
      -s
    COMMAND sed -i.bak 's/GCC Code/VineTalk/g' coverage/*.html)
  add_custom_target(
    coverage
    DEPENDS coverage_pre
    COMMENT
      "Coverage results at: ${CMAKE_CURRENT_BINARY_DIR}/coverage/coverage.html")

  add_custom_target(cov DEPENDS coverage)
endif()
