cmake_minimum_required(VERSION 3.10)
project(MyProject)

find_package(Threads REQUIRED)

# C++17 ï¿½ê¶—ï¿½ìŠœ
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Unix æ¹²ê³•ì»? ï¿½ë–†ï¿½ë’ªï¿½ë?¥ï¿½ë¿‰ï¿½ê½Œï¿½?’— ï¿½ë–ï¿½ë»¾ ï¿½ë™†ï¿½ì”ª ï¿½ì” ?”±ê¾©ë¿‰ .out postfix ?ºï¿½ï¿½ë¿?
if(UNIX)
    set(CMAKE_EXECUTABLE_SUFFIX ".out")
endif()

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
  message(STATUS "Coverage instrumentation enabled")
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# ï¿½ë¿¤ï¿½ëœ‘ ï¿½ë™†ï¿½ì”ª ï¿½ëµ’ï¿½ì †ï¿½ê½£?”±ï¿? ?•°ë¶½ï¿½ï¿?
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external/rapidjson/include
)


# ??¨ë“¯?„» ï§â‘¤ë±?ï¿½ì“£ ï¿½ì”ªï¿½ì” ?‡‰?š®?œ­?”±?‰ì¤ˆ ï¿½ê¹®ï¿½ê½¦ (module.cpp)
add_library(MyLibrary STATIC src/person.cpp)
target_include_directories(MyLibrary PUBLIC ${PROJECT_SOURCE_DIR}/include)

# MessageSerializer Àü¿ë ¶óÀÌºê·¯¸®
add_library(serializer STATIC
    src/network/MessageSerializer.cpp
)
target_include_directories(serializer PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
# Network ¶óÀÌºê·¯¸®
# -> MessageSender, MessageReceiver, PaymentCallbackReceiver ±¸Çö¸¸
add_library(network STATIC
    src/network/MessageSender.cpp
    src/network/MessageReceiver.cpp
    src/network/PaymentCallbackReceiver.cpp
)
target_include_directories(network PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
# Persistence ·¹ÀÌ¾î (OvmAddressRepository)
add_library(persistence STATIC
    src/persistence/OvmAddressRepository.cpp
)
target_include_directories(persistence PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(network
    PRIVATE
        serializer
        Threads::Threads
)

# ¾ÖÇÃ¸®ÄÉÀÌ¼Ç ½ÇÇà ÆÄÀÏ
add_executable(MyApp
    src/main.cpp
)
target_link_libraries(MyApp
    PRIVATE
        MyLibrary
        serializer
        network
)
# Áø¼Ö Å×½ºÆ®¿ë
add_executable(TestNetwork
    tests/test_network.cpp
)
target_link_libraries(TestNetwork
    PRIVATE
        persistence
        network
        Threads::Threads
)


# ï¿½ë?’ï¿½?’ªï¿½ë“ƒ ?®?š®ë±¶ç‘œï¿? ï¿½ê½‘ï¿½ê¹®ï¿½ìŸ»ï¿½ì‘æ¿¡ï¿½ ï¿½ì†¢ï¿½ê½¦ï¿½ì†•ï¿½ë¸¯ï¿½ë’— ï¿½ìƒƒï¿½ë?? ?•°ë¶½ï¿½ï¿?
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
  # GoogleTest ï¿½ê½Œ?‡‰?š®?ˆï¿½ë±¢ ?•°ë¶½ï¿½ï¿? (ï¿½ë?’ï¿½?’ªï¿½ë“ƒ ?®?š®ë±¶ï¿½?’— æ¹²ê³•?‚¯ ALLï¿½ë¿‰ï¿½ê½Œ ï¿½ì £ï¿½ì‡…)
  add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)
  include_directories(${PROJECT_SOURCE_DIR}/lib/googletest/googletest/include)
  
  # ï¿½ë?’ï¿½?’ªï¿½ë“ƒ ï¿½ë–ï¿½ë»¾ ï¿½ë™†ï¿½ì”ª ï¿½ê¹®ï¿½ê½¦ (test_main.cpp ï¿½ë£·ï¿½ë¸¿) - æ¹²ê³•?‚¯ ?®?š®ë±¶ï¿½ë¿‰ï¿½ê½? ï¿½ì £ï¿½ì‡…
  add_executable(RunTests tests/test_person.cpp)
  set_target_properties(RunTests PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(RunTests MyLibrary gtest_main gtest pthread)
  
  enable_testing()
  add_test(NAME GoogleTests COMMAND RunTests)
  
  # "googletest"ï¿½ì”ªï¿½ë’— custom targetï¿½ì“£ ï§ëš®ë±¾ï¿½ë¼? ï¿½ë¸˜ï¿½ìŠ‚ ï¿½ë–† ï¿½ë?’ï¿½?’ªï¿½ë“ƒ?‘œï¿? ï¿½ëµ²æ¿¡ï¿½ ?®?š®ë±¶ï¿½ë¸? ï¿½ë‹” ï¿½ì—³ï¿½ë£„æ¿¡ï¿½ ï¿½ë¸¿
  add_custom_target(googletest DEPENDS RunTests)
endif()

# cppcheck ï¿½ë–ï¿½ë»¾ ??Œã…¼?’ªï¿½ï¿½ï¿? ï¿½ï¿½ï¿½æºï¿? ?•°ë¶½ï¿½ï¿?
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