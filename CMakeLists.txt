cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME lab1) # Set this to your main .cpp file's name, this will be the name of the output executable as well

project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_compile_definitions(_HAS_CXX17=1) # thank you microsoft

# Find GLFW and GLM (system packages)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Find all files recursively in the libraries folder
file(GLOB_RECURSE HEADERS libraries/*.h)
file(GLOB_RECURSE SOURCES_C libraries/*.c)
file(GLOB_RECURSE SOURCES_CPP libraries/*.cpp)

set(HEADERS
    # add your additional headers here
    ${SOURCES_H}
)

set(SOURCES 
    # add your additional sources here
    ${PROJECT_NAME}.cpp # Your main .cpp file should have the same name as the project name
    ${SOURCES_CPP}
    ${SOURCES_C}
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE 
    .
    libraries
    libraries/glad/include
)

# Link against GLFW and GLM
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm)
