cmake_minimum_required(VERSION 3.10)
project(SMA_T6_Project)

#유스케이스 테스트 추가
enable_testing()  # (한 번만 선언, 보통 맨 위에)
add_test(NAME UC01Test COMMAND runUC01Test.out)
add_test(NAME UC04Test COMMAND runUC04Test.out)
add_test(NAME UC05Test COMMAND runUC05Test.out)
add_test(NAME UC06Test COMMAND runUC06Test.out)
add_test(NAME UC07Test COMMAND runUC07Test.out)
add_test(NAME UC08Test COMMAND runUC08Test.out)
add_test(NAME UC10Test COMMAND runUC10Test.out)
add_test(NAME UC11Test COMMAND runUC11Test.out)
add_test(NAME UC12Test COMMAND runUC12Test.out)
add_test(NAME UC13Test COMMAND runUC13Test.out)
add_test(NAME UC14Test COMMAND runUC14Test.out)
add_test(NAME UC15Test COMMAND runUC15Test.out)
add_test(NAME UC16Test COMMAND runUC16Test.out)
add_test(NAME UC17Test COMMAND runUC17Test.out)





# �ܺ� ���̺귯�� ��� ���� (����� ȯ�濡 �°� ������)
set(RAPIDJSON_INCLUDE_DIR "/home/linuxbrew/.linuxbrew/include") # RapidJSON ��� ���


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost ���̺귯�� ã��
find_package(Boost 1.74.0 REQUIRED COMPONENTS system)

if(Boost_FOUND)
    message(STATUS "Boost found. Include Dirs: ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS}) # �������� Boost ��� ��� �߰� 

else()
    message(FATAL_ERROR "Boost not found!")
endif()


if(UNIX)
    set(CMAKE_EXECUTABLE_SUFFIX ".out")
endif()

find_package(Threads REQUIRED)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/external/rapidjson/include 
  /home/linuxbrew/.linuxbrew/include
)


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

#UC 01, 10~ 17 System Test Case
add_executable(runUC01Test 
tests/UC01.cpp
src/service/InventoryService.cpp
src/service/ErrorService.cpp)

target_link_libraries(runUC01Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC01Test PRIVATE include)
target_compile_options(runUC01Test PRIVATE --coverage)
target_link_options(runUC01Test PRIVATE --coverage)

add_executable(runUC04Test 
tests/UC04.cpp)

target_link_libraries(runUC04Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC04Test PRIVATE include)
target_compile_options(runUC04Test PRIVATE --coverage)
target_link_options(runUC04Test PRIVATE --coverage)

add_executable(runUC05Test 
tests/UC05.cpp)

target_link_libraries(runUC05Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC05Test PRIVATE include)
target_compile_options(runUC05Test PRIVATE --coverage)
target_link_options(runUC05Test PRIVATE --coverage)

add_executable(runUC06Test 
tests/UC06.cpp)

target_link_libraries(runUC06Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC06Test PRIVATE include)
target_compile_options(runUC06Test PRIVATE --coverage)
target_link_options(runUC06Test PRIVATE --coverage)

add_executable(runUC07Test 
tests/UC07.cpp)

target_link_libraries(runUC07Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC07Test PRIVATE include)
target_compile_options(runUC07Test PRIVATE --coverage)
target_link_options(runUC07Test PRIVATE --coverage)

add_executable(runUC08Test 
tests/UC08.cpp)

target_link_libraries(runUC08Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC08Test PRIVATE include)
target_compile_options(runUC08Test PRIVATE --coverage)
target_link_options(runUC08Test PRIVATE --coverage)

add_executable(runUC11Test 
tests/UC11.cpp)

target_link_libraries(runUC11Test
    domain
    persistence
    application
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC11Test PRIVATE include)
target_compile_options(runUC11Test PRIVATE --coverage)
target_link_options(runUC11Test PRIVATE --coverage)

add_executable(runUC10Test 
tests/UC10.cpp
src/service/DistanceService.cpp)

target_link_libraries(runUC10Test gtest gtest_main pthread)
target_include_directories(runUC10Test PRIVATE include)
target_compile_options(runUC10Test PRIVATE --coverage)
target_link_options(runUC10Test PRIVATE --coverage)

add_executable(runUC12Test 
tests/UC12.cpp)

target_link_libraries(runUC12Test
    domain
    persistence
    application
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC12Test PRIVATE include)
target_compile_options(runUC12Test PRIVATE --coverage)
target_link_options(runUC12Test PRIVATE --coverage)

add_executable(runUC13Test 
tests/UC13.cpp)

target_link_libraries(runUC13Test
    domain
    persistence
    application
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC13Test PRIVATE include)
target_compile_options(runUC13Test PRIVATE --coverage)
target_link_options(runUC13Test PRIVATE --coverage)

add_executable(runUC14Test 
tests/UC14.cpp)

target_link_libraries(runUC14Test
    domain
    persistence
    application
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC14Test PRIVATE include)
target_compile_options(runUC14Test PRIVATE --coverage)
target_link_options(runUC14Test PRIVATE --coverage)

add_executable(runUC15Test 
tests/UC15.cpp)

target_link_libraries(runUC15Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC15Test PRIVATE include)
target_compile_options(runUC15Test PRIVATE --coverage)
target_link_options(runUC15Test PRIVATE --coverage)

add_executable(runUC16Test 
tests/UC16.cpp)

target_link_libraries(runUC16Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC16Test PRIVATE include)
target_compile_options(runUC16Test PRIVATE --coverage)
target_link_options(runUC16Test PRIVATE --coverage)

add_executable(runUC17Test 
tests/UC17.cpp)

target_link_libraries(runUC17Test
    domain
    persistence
    application
    network
    gtest 
    gtest_main 
    pthread
    ${Boost_LIBRARIES}
    Threads::Threads)
target_include_directories(runUC17Test PRIVATE include)
target_compile_options(runUC17Test PRIVATE --coverage)
target_link_options(runUC17Test PRIVATE --coverage)


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

  #add_executable(RunTests
    #tests/test_person.cpp
    #tests/inventoryRepository_test.cpp
    #tests/inventory_test.cpp
    #tests/prepaymentCode_test.cpp
    #tests/order_test.cpp
    #tests/prepaymentCodeRepository_test.cpp
  #)
  #target_link_libraries(RunTests PRIVATE application gtest gtest_main Threads::Threads)
  #add_test(NAME GoogleTests COMMAND RunTests)

  #add_executable(MessageTests
    #tests/network/msg_sender_test_1.cpp
    #tests/network/test_message_service_broadcast_stock.cpp
  #)
  #target_link_libraries(MessageTests PRIVATE application gtest gtest_main Threads::Threads Boost::system) 
  #add_test(NAME MessageTests COMMAND MessageTests)
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


option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
  # ... coverage 타겟 정의 ...
# Coverage Report: gcovr 
find_program(GCOVR_EXECUTABLE NAMES gcovr)
if(ENABLE_COVERAGE AND GCOVR_EXECUTABLE)
  message(STATUS "Found gcovr: ${GCOVR_EXECUTABLE}")
  add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage_report
    COMMAND ${CMAKE_COMMAND} --build . --target runUC01Test
    COMMAND ./runUC01Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC04Test
    COMMAND ./runUC04Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC05Test
    COMMAND ./runUC05Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC06Test
    COMMAND ./runUC06Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC07Test
    COMMAND ./runUC07Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC08Test
    COMMAND ./runUC08Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC11Test
    COMMAND ./runUC11Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC10Test
    COMMAND ./runUC10Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC12Test
    COMMAND ./runUC12Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC13Test
    COMMAND ./runUC13Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC14Test
    COMMAND ./runUC14Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC15Test
    COMMAND ./runUC15Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC16Test
    COMMAND ./runUC16Test.out
    COMMAND ${CMAKE_COMMAND} --build . --target runUC17Test
    COMMAND ./runUC17Test.out
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




endif()




