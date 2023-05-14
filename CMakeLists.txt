cmake_minimum_required(VERSION 3.10)

# project details
project(PSIApp VERSION 1.0)
set(SENDER_EXEC_NAME psi_sender)
set(RECEIVER_EXEC_NAME psi_receiver)
set(LIBRARY_NAME psi_app_lib)
set(LIBRARY_NAME_SHARED psi_app_lib_shared)

# packages
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(Boost)
include(Cryptopp)
include(CTest)
include(Doctest)
include(Documentation)
include(Warnings)
include(Curses)

# add shared libraries
set(SOURCES_SHARED
  src-shared/circuit.cxx
  src-shared/messages.cxx
  src-shared/logger.cxx
  src-shared/util.cxx)
add_library(${LIBRARY_NAME_SHARED} ${SOURCES_SHARED})
target_include_directories(${LIBRARY_NAME_SHARED} PUBLIC ${PROJECT_SOURCE_DIR}/include-shared)
target_link_libraries(${LIBRARY_NAME_SHARED} PUBLIC doctest)
target_link_libraries(${LIBRARY_NAME_SHARED} PRIVATE cryptopp)
target_link_libraries(${LIBRARY_NAME_SHARED} PRIVATE cryptopp-shared)
target_link_libraries(${LIBRARY_NAME_SHARED} PRIVATE ${Boost_LIBRARIES})
target_link_libraries(${LIBRARY_NAME_SHARED} PRIVATE ${CURSES_LIBRARIES})

# add student libraries
set(SOURCES
  src/pkg/psi_sender.cxx
  src/pkg/psi_receiver.cxx
  src/drivers/cli_driver.cxx
  src/drivers/crypto_driver.cxx
  src/drivers/network_driver.cxx)
add_library(${LIBRARY_NAME} ${SOURCES})
target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include-shared ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LIBRARY_NAME_SHARED})

# add psi sender executables
add_executable(${SENDER_EXEC_NAME} src/cmd/psi_sender.cxx)
target_link_libraries(${SENDER_EXEC_NAME} PRIVATE ${LIBRARY_NAME})

# add psi receiver executables
add_executable(${RECEIVER_EXEC_NAME} src/cmd/psi_receiver.cxx)
target_link_libraries(${SENDER_EXEC_NAME} PRIVATE ${LIBRARY_NAME})

# add ot test executables
# if ( "$ENV{CS1515_TA_MODE}" STREQUAL "on" )
#   add_executable(${OTTEST_EXEC_NAME} src-ta/cmd/ot_test.cxx)
#   target_link_libraries(${OTTEST_EXEC_NAME} PRIVATE ${LIBRARY_NAME_TA})
# else()
#   add_executable(${OTTEST_EXEC_NAME} src/cmd/ot_test.cxx)
#   target_link_libraries(${OTTEST_EXEC_NAME} PRIVATE ${LIBRARY_NAME})
# endif()

# properties
set_target_properties(
  ${LIBRARY_NAME}
  ${SENDER_EXEC_NAME}
  ${RECEIVER_EXEC_NAME}
    PROPERTIES
      CXX_STANDARD 20
      CXX_STANDARD_REQUIRED YES
      CXX_EXTENSIONS YES
)

# add tests
# add_subdirectory(test)
# add_custom_target(check ./test.sh)
