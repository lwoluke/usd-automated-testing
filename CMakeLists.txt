cmake_minimum_required(VERSION 3.16)
project(AutomatedUSDTesting CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define NOMINMAX to prevent Windows headers from defining max/min macros
add_definitions(-DNOMINMAX)

# Define USD_ROOT based on the platform if not provided
if(NOT DEFINED USD_ROOT)
    if(WIN32)
        set(USD_ROOT "C:/Users/lwolu/OneDrive/Documents/Coding/dev/usd-automated-testing/usd") # Update as needed
    else()
        set(USD_ROOT "/usr/local/USD")
    endif()
endif()

message("Using USD_ROOT=${USD_ROOT}")

# Include directories for USD
include_directories("${USD_ROOT}/include")

# Link directories for USD
link_directories("${USD_ROOT}/lib")

# Find USD libraries based on the platform
if(WIN32)
    # On Windows, look for .lib files (import libraries)
    file(GLOB USD_LIBS "${USD_ROOT}/lib/*.lib")
else()
    # On Unix-like systems, look for .so files
    file(GLOB USD_LIBS "${USD_ROOT}/lib/libusd*.so")
endif()

# Find Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})
link_directories(${Python3_LIBRARY_DIRS})

# Suppress warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-deprecated -Wno-pragmas -DTBB_SUPPRESS_DEPRECATED_MESSAGES=1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-deprecated-declarations -DTBB_SUPPRESS_DEPRECATED_MESSAGES=1)
endif()

# Add the main executable
add_executable(usdTestRunner src/main.cpp)

# Link against USD and Python libraries
target_link_libraries(usdTestRunner ${USD_LIBS} ${Python3_LIBRARIES})

# Set RPATH for Unix-like systems
if(NOT WIN32)
    set_target_properties(usdTestRunner PROPERTIES
        BUILD_RPATH "${USD_ROOT}/lib"
        INSTALL_RPATH "${USD_ROOT}/lib"
    )
endif()
