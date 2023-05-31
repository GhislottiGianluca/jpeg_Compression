cmake_minimum_required(VERSION 3.5)


include(ExternalProject)

MESSAGE(STATUS "Trying to install fftw...")

ExternalProject_Add(project_fftw
        #GIT_REPOSITORY https://github.com/FFTW/fftw3
        URL http://www.fftw.org/fftw-3.3.2.tar.gz
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/fftw
        CONFIGURE_COMMAND
        ${CMAKE_CURRENT_BINARY_DIR}/fftw/src/project_fftw/configure
        --prefix=${CMAKE_CURRENT_BINARY_DIR}/fftw/install
        INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/fftw/install
        )


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/fftw/install/include)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/fftw/install/lib)

add_executable(custom main.cpp timer.cpp)
add_dependencies(custom project_fftw)

TARGET_LINK_LIBRARIES(custom fftw3)