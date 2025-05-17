cmake_minimum_required(VERSION 3.10)
project(SMA_T6_Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX)
    set(CMAKE_EXECUTABLE_SUFFIX ".out")
endif()

find_package(Threads REQUIRED)

# ───────────── Include Paths ─────────────
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external/rapidjson/include
)

# ───────────── Option: Coverage ─────────────
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
  message(STATUS "Coverage instrumentation enabled")
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# ───────────── Domain Layer (Header-Only) ─────────────
add_library(domain INTERFACE)
target_include_directories(domain INTERFACE ${PROJECT_SOURCE_DIR}/include)

# ───────────── Network Layer ─────────────
add_library(network STATIC
    src/network/MessageSender.cpp
    src/network/MessageReceiver.cpp
    src/network/MessageSerializer.cpp
    src/network/PaymentCallbackReceiver.cpp
)
target_include_directories(network PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ───────────── Persistence Layer ─────────────
add_library(persistence STATIC
    src/persistence/OvmAddressRepository.cpp
    src/persistence/prepayCodeRepository.cpp
    src/persistence/inventoryRepository.cpp
)
target_include_directories(persistence PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ───────────── Application Logic Layer ─────────────
add_library(application STATIC
    src/service/ErrorService.cpp
    src/service/InventoryService.cpp
    src/service/MessageService.cpp
    src/service/OrderService.cpp
    src/service/PrepaymentService.cpp
    src/service/UserProcessController.cpp
)
target_include_directories(application PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(application
    PRIVATE
        domain
        network
        persistence
        Threads::Threads
)

# ───────────── Main Executable ─────────────
add_executable(MyApp
    src/main.cpp
)
target_link_libraries(MyApp
    PRIVATE
        application
        network
        persistence
        domain
        Threads::Threads
)



# ───────────── GoogleTest Option ─────────────
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
  add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)
  include_directories(${PROJECT_SOURCE_DIR}/lib/googletest/googletest/include)

  add_executable(RunTests tests/test_person.cpp)
  set_target_properties(RunTests PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(RunTests PRIVATE application gtest_main gtest pthread)

  enable_testing()
  add_test(NAME GoogleTests COMMAND RunTests)

  add_custom_target(googletest DEPENDS RunTests)
endif()

# ───────────── Static Analysis: cppcheck ─────────────
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

# ───────────── Coverage Report: gcovr ─────────────
find_program(GCOVR_EXECUTABLE NAMES gcovr)
if(GCOVR_EXECUTABLE)
  message(STATUS "Found gcovr: ${GCOVR_EXECUTABLE}")
  add_custom_target(coverage
    DEPENDS googletest
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
    COMMAND ${GCOVR_EXECUTABLE}
      -r ${PROJECT_SOURCE_DIR}
      --filter "${PROJECT_SOURCE_DIR}/(src|include|tests)/.*"
      --exclude "${PROJECT_SOURCE_DIR}/lib/.*"
      --html-details
      --output ${CMAKE_BINARY_DIR}/coverage/coverage.html
    COMMAND ${GCOVR_EXECUTABLE}
      -r ${PROJECT_SOURCE_DIR}
      --filter "${PROJECT_SOURCE_DIR}/(src|include|tests)/.*"
      --exclude "${PROJECT_SOURCE_DIR}/lib/.*"
      --coveralls
      --output ${CMAKE_BINARY_DIR}/coverage.json
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage report with gcovr..."
    VERBATIM
  )
endif()
