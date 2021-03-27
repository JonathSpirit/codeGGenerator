cmake_minimum_required(VERSION 3.10)

#Project
project(codeGGenerator)

#Enabling CTest
enable_testing()

#Policy
cmake_policy(SET CMP0076 NEW) #target_sources

#Variables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release)

#Copy example folder
file(COPY "example/" DESTINATION "example/")

#Set cache entry
set(TARGET_ARCH "" CACHE STRING "Set architecture type (32 or 64)")

#Not defined = default to empty
if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH "")
endif()

#Set compiler 32/64 flag
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

#Project name
project(codeGGenerator VERSION 0.1)

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
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # using Visual Studio C++
    set(CMAKE_CXX_FLAGS "/EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
else()
    # using GCC
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${TARGET_ARCH_FLAG}")
    
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -s")
endif()

#Executable
add_executable(${PROJECT_NAME})

#Includes path
target_include_directories(${PROJECT_NAME} PUBLIC "include/")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")

#Sources file
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

#Add test
add_test(NAME "CompilingTestFile" COMMAND ${PROJECT_NAME} "--in=example/test")
