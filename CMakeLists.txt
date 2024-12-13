cmake_minimum_required(VERSION 3.16)
project(AutomatedUSDTesting CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point to the USD prebuilt directory
set(USD_ROOT "${CMAKE_SOURCE_DIR}/usd")

# Include directories for USD
include_directories("${USD_ROOT}/include")

# Link directories for USD
link_directories("${USD_ROOT}/lib")

# Find all USD libraries dynamically
file(GLOB USD_LIBS "${USD_ROOT}/lib/libusd*.so")

# Find Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
include_directories(${Python3_INCLUDE_DIRS})
link_directories(${Python3_LIBRARY_DIRS})

# Add the main executable
add_executable(usdTestRunner src/main.cpp)

# Link against USD and Python libraries
target_link_libraries(usdTestRunner ${USD_LIBS} ${Python3_LIBRARIES})

# On Linux/macOS, ensure that the RPATH is set so that the binary can find the USD libraries at runtime:
set_target_properties(usdTestRunner PROPERTIES
    BUILD_RPATH "${USD_ROOT}/lib"
    INSTALL_RPATH "${USD_ROOT}/lib"
)
