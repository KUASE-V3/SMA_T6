cmake_minimum_required(VERSION 3.10)
project(MyProject)

# C++17 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Unix 기반 시스템에서는 실행 파일 이름에 .out postfix 부여
if(UNIX)
    set(CMAKE_EXECUTABLE_SUFFIX ".out")
endif()

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
  message(STATUS "Coverage instrumentation enabled")
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# 헤더 파일 디렉터리 추가
include_directories(${PROJECT_SOURCE_DIR}/include)

# 공통 모듈을 라이브러리로 생성 (module.cpp)
add_library(MyLibrary STATIC src/person.cpp)
target_include_directories(MyLibrary PUBLIC ${PROJECT_SOURCE_DIR}/include)

# 메인 애플리케이션 생성 (main.cpp는 라이브러리의 함수 호출)
add_executable(MyApp src/main.cpp)
target_link_libraries(MyApp MyLibrary)

# 테스트 빌드를 선택적으로 활성화하는 옵션 추가
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
  # GoogleTest 서브모듈 추가 (테스트 빌드는 기본 ALL에서 제외)
  add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)
  include_directories(${PROJECT_SOURCE_DIR}/lib/googletest/googletest/include)
  
  # 테스트 실행 파일 생성 (test_main.cpp 포함) - 기본 빌드에서 제외
  add_executable(RunTests tests/test_person.cpp)
  set_target_properties(RunTests PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(RunTests MyLibrary gtest_main gtest pthread)
  
  enable_testing()
  add_test(NAME GoogleTests COMMAND RunTests)
  
  # "googletest"라는 custom target을 만들어 필요 시 테스트를 따로 빌드할 수 있도록 함
  add_custom_target(googletest DEPENDS RunTests)
endif()

# cppcheck 실행 커스텀 타깃 추가
find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
if(CPPCHECK_EXECUTABLE)
  message(STATUS "Found cppcheck: ${CPPCHECK_EXECUTABLE}")
  add_custom_target(
    cppcheck
    COMMAND ${CPPCHECK_EXECUTABLE}
      --enable=all
      --std=c++17
      --language=c++
      --suppress=missingIncludeSystem
      --verbose
      --output-format=sarif
      --output-file=cppcheck_report.sarif
      -Iinclude
      src include
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Running cppcheck..."
    VERBATIM
  )
endif()

# GCover
find_program(GCOVR_EXECUTABLE NAMES gcovr)
if(GCOVR_EXECUTABLE)
  message(STATUS "Found gcovr: ${GCOVR_EXECUTABLE}")
  add_custom_target(coverage
    DEPENDS googletest
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
    COMMAND ${GCOVR_EXECUTABLE}
      -r ${PROJECT_SOURCE_DIR}
      --filter "${PROJECT_SOURCE_DIR}/src/.*|${PROJECT_SOURCE_DIR}/include/.*"
      --filter "${PROJECT_SOURCE_DIR}/tests/.*"
      --exclude "${PROJECT_SOURCE_DIR}/lib/.*"
    COMMAND ${GCOVR_EXECUTABLE}
      -r ${PROJECT_SOURCE_DIR}
      --filter "${PROJECT_SOURCE_DIR}/src/.*|${PROJECT_SOURCE_DIR}/include/.*"
      --filter "${PROJECT_SOURCE_DIR}/tests/.*"
      --exclude "${PROJECT_SOURCE_DIR}/lib/.*"
      --html-details
      --output ${CMAKE_BINARY_DIR}/coverage/coverage.html
    COMMAND ${GCOVR_EXECUTABLE}
      -r ${PROJECT_SOURCE_DIR}
      --filter "${PROJECT_SOURCE_DIR}/src/.*|${PROJECT_SOURCE_DIR}/include/.*"
      --filter "${PROJECT_SOURCE_DIR}/tests/.*"
      --exclude "${PROJECT_SOURCE_DIR}/lib/.*"
      --coveralls
      --output ${CMAKE_BINARY_DIR}/coverage.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage report with gcovr..."
    VERBATIM
  )
endif()