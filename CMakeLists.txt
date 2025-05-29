cmake_minimum_required(VERSION 3.10)
project(SMA_T6_Project)

# 외부 라이브러리 경로 설정 (사용자 환경에 맞게 설정됨)
set(RAPIDJSON_INCLUDE_DIR "/home/linuxbrew/.linuxbrew/include") # RapidJSON 헤더 경로

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost 라이브러리 찾기
find_package(Boost 1.74.0 REQUIRED COMPONENTS system)

if(Boost_FOUND)
    message(STATUS "Boost found. Include Dirs: ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS}) # 전역으로 Boost 헤더 경로 추가 
else()
    message(FATAL_ERROR "Boost not found!")
endif()


if(UNIX)
    set(CMAKE_EXECUTABLE_SUFFIX ".out")
endif()

find_package(Threads REQUIRED)

# 기본 Include 경로 설정
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external/rapidjson/include 
)

# 커버리지 옵션 (기존 유지)
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
  message(STATUS "Coverage instrumentation enabled")
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# Domain Layer
add_library(domain STATIC
    src/domain/drink.cpp
    src/domain/inventory.cpp
    src/domain/order.cpp
    src/domain/prepaymentCode.cpp
    src/domain/vendingMachine.cpp
)
target_include_directories(domain PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Network Layer
add_library(network STATIC
    src/network/MessageSender.cpp
    src/network/MessageReceiver.cpp
    src/network/MessageSerializer.cpp
    src/network/PaymentCallbackReceiver.cpp
)
target_include_directories(network PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(network PUBLIC
    ${Boost_LIBRARIES}
    Threads::Threads
)

# Persistence Layer
add_library(persistence STATIC
    src/persistence/DrinkRepository.cpp       
    src/persistence/inventoryRepository.cpp   
    src/persistence/OrderRepository.cpp       
    src/persistence/OvmAddressRepository.cpp
    src/persistence/prepayCodeRepository.cpp  
)
target_include_directories(persistence PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(persistence PUBLIC domain) 

# Application Logic Layer (Service & Presentation)
add_library(application STATIC
    src/service/DistanceService.cpp           
    src/service/ErrorService.cpp
    src/service/InventoryService.cpp
    src/service/MessageService.cpp
    src/service/OrderService.cpp
    src/service/PrepaymentService.cpp
    src/service/UserProcessController.cpp
    src/presentation/UserInterface.cpp
)
target_include_directories(application PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(application PUBLIC
    domain
    network     
    persistence
    ${Boost_LIBRARIES}
    Threads::Threads
)

# Main Executable
add_executable(MyApp
    src/main.cpp
)
target_link_libraries(MyApp PRIVATE
    application      
    ${Boost_LIBRARIES} 
    Threads::Threads
)

# GoogleTest Option 
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
  if(NOT TARGET gtest)
    add_subdirectory(lib/googletest EXCLUDE_FROM_ALL)
  endif()

  add_executable(RunTests
    tests/test_person.cpp
    tests/inventoryRepository_test.cpp
    tests/inventory_test.cpp
    tests/prepaymentCode_test.cpp
    tests/order_test.cpp
    tests/prepaymentCodeRepository_test.cpp
  )
  target_link_libraries(RunTests PRIVATE application gtest gtest_main Threads::Threads)
  add_test(NAME GoogleTests COMMAND RunTests)

  add_executable(MessageTests
    tests/network/msg_sender_test_1.cpp
    tests/network/test_message_service_broadcast_stock.cpp
  )
  target_link_libraries(MessageTests PRIVATE application gtest gtest_main Threads::Threads Boost::system) 
  add_test(NAME MessageTests COMMAND MessageTests)
endif()

# Static Analysis: cppcheck 
find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
if(CPPCHECK_EXECUTABLE)
  message(STATUS "Found cppcheck: ${CPPCHECK_EXECUTABLE}")
  add_custom_target(
    cppcheck
    COMMAND ${CPPCHECK_EXECUTABLE}
      --enable=all --std=c++17 --language=c++
      --suppress=missingIncludeSystem --suppress=unusedFunction
      --verbose --force --inline-suppr
      --output-format=sarif --output-file=cppcheck_report.sarif
      -I include
      -I ${Boost_INCLUDE_DIRS} 
      src tests
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Running cppcheck..."
    VERBATIM
  )
endif()

# Coverage Report: gcovr 
find_program(GCOVR_EXECUTABLE NAMES gcovr)
if(ENABLE_COVERAGE AND GCOVR_EXECUTABLE)
  message(STATUS "Found gcovr: ${GCOVR_EXECUTABLE}")
  add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage_report
    COMMAND ${CMAKE_COMMAND} --build . --target RunTests   
    COMMAND ${CMAKE_COMMAND} --build . --target MessageTests 
    COMMAND ${GCOVR_EXECUTABLE}
      -r ${PROJECT_SOURCE_DIR}
      --filter "${PROJECT_SOURCE_DIR}/src/.*"
      --exclude "${PROJECT_SOURCE_DIR}/lib/.*"
      --exclude "${PROJECT_SOURCE_DIR}/tests/.*"
      --exclude "${PROJECT_SOURCE_DIR}/external/.*"
      --html-details ${CMAKE_BINARY_DIR}/coverage_report/coverage.html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating coverage report with gcovr..."
    VERBATIM
  )
endif()