cmake_minimum_required(VERSION 3.10)
project(SMA_T6_Project)


set(RAPIDJSON_INCLUDE_DIR "/home/linuxbrew/.linuxbrew/include")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BOOST_ROOT /opt/homebrew)
set(BOOST_INCLUDEDIR /opt/homebrew/include)
set(BOOST_LIBRARYDIR /opt/homebrew/lib)

if(UNIX)
    set(CMAKE_EXECUTABLE_SUFFIX ".out")
endif()

find_package(Threads REQUIRED 
            Boost REQUIRED COMPONENTS system)

# ───────────── Include Paths ─────────────
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external/rapidjson/include
  ${Boost_INCLUDE_DIRS}
)

# ───────────── Option: Coverage ─────────────
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
  message(STATUS "Coverage instrumentation enabled")
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# ───────────── Domain Layer -────────────
add_library(domain STATIC 
    src/domain/inventory.cpp
    src/domain/order.cpp
    src/domain/prepaymentCode.cpp
    src/domain/drink.cpp
    src/domain/vendingMachine.cpp
)
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
    src/presentation/UserInterface.cpp 

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

  # ───────────── 기본 테스트 실행 파일 ─────────────
  add_executable(RunTests
    tests/test_person.cpp
    tests/inventoryRepository_test.cpp
    tests/inventory_test.cpp
    tests/prepaymentCode_test.cpp
    tests/order_test.cpp
    tests/prepaymentCodeRepository_test.cpp
  )
  set_target_properties(RunTests PROPERTIES EXCLUDE_FROM_ALL TRUE)
  target_link_libraries(RunTests PRIVATE application gtest_main gtest pthread)
  add_test(NAME GoogleTests COMMAND RunTests)

  # ───────────── Message 관련 세분화 테스트 ─────────────
  add_executable(MessageTests
    tests/network/msg_sender_test_1.cpp
    tests/network/msg_sender_test_2.cpp
    tests/network/msg_service_test_1.cpp
    tests/network/msg_service_test_2.cpp
    tests/network/msg_service_test_3.cpp
    tests/network/msg_service_test_4.cpp
    tests/network/msg_service_test_5.cpp
    tests/network/test_MessageReceiver_subscribe.cpp
    tests/network/test_MessageSender_basicSend.cpp
    tests/network/test_MessageService_broadcast.cpp
    tests/network/test_message_service_broadcast_stock.cpp
  )
  target_include_directories(MessageTests
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/googletest/googletest/include
  )
  target_link_libraries(MessageTests
    PRIVATE application domain network persistence gtest gtest_main pthread
  )
  add_test(NAME MessageTests COMMAND MessageTests)

  add_custom_target(googletest DEPENDS RunTests MessageTests)
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
