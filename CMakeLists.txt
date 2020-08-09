project(gzat)
# Include header directories
include_directories("include")

# Include headers and sources
# Sources
file(GLOB sub_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "./source/*.cpp")

# Headers
file(GLOB sub_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "./include/*.hpp")

# Testers
file(GLOB sub_testers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "./test/*.cpp")

# Add to list of sources
set(sources ${sources} ${sub_sources})
set(headers ${headers} ${sub_headers})
set(testers ${testers} ${sub_testers})

# Set compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fPIC -Werror -fsanitize=address")

# Compile library
add_library(
    ${PROJECT_NAME}
    STATIC
    ${sources} 
    ${headers}
)

# Compile test
add_executable(
    ${PROJECT_NAME}_test
    ${testers}
    ${headers}
)

target_link_libraries(
    ${PROJECT_NAME}_test
    ${PROJECT_NAME}
    gtest
    gtest_main
)