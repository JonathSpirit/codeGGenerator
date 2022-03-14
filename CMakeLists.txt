cmake_minimum_required(VERSION 3.10)

#Project
project(codeGGenerator VERSION 0.2)

#Policy
cmake_policy(SET CMP0076 NEW) #target_sources

#Enabling CTest
enable_testing()

#Variables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)

#Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#Copy example folder
file(COPY "example/" DESTINATION "example/")

#Set cache entry
set(TARGET_ARCH "" CACHE STRING "Set architecture type (32 or 64 or nothing (auto))")

#Not defined = default to empty
if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "")
endif()

#Set compiler 32/64/(auto) flag
set(TARGET_ARCH_FLAG "")

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if (TARGET_ARCH MATCHES "32")
        set(TARGET_ARCH_FLAG "-m32")
    elseif (TARGET_ARCH MATCHES "64")
        set(TARGET_ARCH_FLAG "-m64")
    else()
        set(TARGET_ARCH_FLAG "")
    endif()
endif()

#Configure header file
configure_file(CMakeConfig.hpp.in CMakeConfig.hpp)

#C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Set compiler options
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # using Clang
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${TARGET_ARCH_FLAG}")
    
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-static -O3")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # using Visual Studio C++
    set(CMAKE_CXX_FLAGS "/EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    # using GCC
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${TARGET_ARCH_FLAG}")
    
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-static -O3 -s")
endif()

#Executable
add_executable(${PROJECT_NAME})

#Includes path
target_include_directories(${PROJECT_NAME} PUBLIC "include/")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

#Source files
target_sources(${PROJECT_NAME} PUBLIC "src/main.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_variable.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_value.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_target.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_stringDecomposer.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_string.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_macro.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_keyword.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_instruction.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_console.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_compilerData.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_address.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_readableBus.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_fileReader.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_function.cpp")
target_sources(${PROJECT_NAME} PUBLIC "src/C_reserved.cpp")

#Header files
target_sources(${PROJECT_NAME} PUBLIC "include/main.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_variable.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_value.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_target.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_stringDecomposer.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_string.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_macro.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_keyword.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_instruction.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_console.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_compilerData.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_address.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_readableBus.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_fileReader.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_function.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_reserved.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_error.hpp")
target_sources(${PROJECT_NAME} PUBLIC "include/C_bus.hpp")

#Add test
add_test(NAME "CompilingTestFile" COMMAND ${PROJECT_NAME} "--in=example/test")
add_test(NAME "CompilingModelFile" COMMAND ${PROJECT_NAME} "--in=example/model")
add_test(NAME "CompilingUartTestFile" COMMAND ${PROJECT_NAME} "--in=example/uart_test")
