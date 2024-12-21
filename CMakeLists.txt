# This CMakeLists.txt configures a project for automated USD (Universal Scene Description) testing.
# It sets up a C++ project that interfaces with USD libraries and Python.
# Prerequisites:
#   - CMake 3.16 or higher
#   - C++ compiler with C++17 support
#   - USD libraries installed
#   - Python 3 with development headers

# Specify minimum CMake version required
cmake_minimum_required(VERSION 3.16)

# Define project name and language
project(AutomatedUSDTesting CXX)

# Configure C++17 as the required standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prevent Windows.h from defining min/max macros that could conflict with std::min/std::max
add_definitions(-DNOMINMAX)

# Set default USD installation directory based on platform
# Can be overridden with -DUSD_ROOT=/path/to/usd during cmake configuration
if(NOT DEFINED USD_ROOT)
    if(WIN32)
        set(USD_ROOT "C:/Users/lwolu/OneDrive/Documents/Coding/dev/usd-automated-testing/usd") # Default Windows path
    else()
        set(USD_ROOT "/usr/local/USD") # Default Linux path
    endif()
endif()

# Display the USD installation path being used
message("Using USD_ROOT=${USD_ROOT}")

# Add USD header files to include path
include_directories("${USD_ROOT}/include")

# Add USD library directory to linker search path
link_directories("${USD_ROOT}/lib")

# Automatically detect USD libraries based on platform
if(WIN32)
    # Windows uses .lib files as import libraries
    file(GLOB USD_LIBS "${USD_ROOT}/lib/*.lib")
else()
    # Linux systems use .so shared libraries
    file(GLOB USD_LIBS "${USD_ROOT}/lib/libusd*.so")
endif()

# Configure Python dependencies
# Requires Python 3 with development components
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})
link_directories(${Python3_LIBRARY_DIRS})

# Configure compiler-specific warning suppressions
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC-specific warning suppressions
    add_compile_options(-Wno-deprecated -Wno-pragmas -DTBB_SUPPRESS_DEPRECATED_MESSAGES=1)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Clang-specific warning suppressions
    add_compile_options(-Wno-deprecated-declarations -DTBB_SUPPRESS_DEPRECATED_MESSAGES=1)
endif()

# Define the main executable target
add_executable(usdTestRunner src/main.cpp)

# Link the executable with USD and Python libraries
target_link_libraries(usdTestRunner ${USD_LIBS} ${Python3_LIBRARIES})

# Configure runtime library search path (RPATH) for Linux systems
# This ensures the executable can find USD libraries at runtime
if(NOT WIN32)
    set_target_properties(usdTestRunner PROPERTIES
        BUILD_RPATH "${USD_ROOT}/lib"
        INSTALL_RPATH "${USD_ROOT}/lib"
    )
endif()