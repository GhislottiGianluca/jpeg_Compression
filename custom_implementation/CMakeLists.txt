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

add_library(fftw STATIC IMPORTED)
set(lib_fftw_name ${CMAKE_STATIC_LIBRARY_PREFIX}fftw3${CMAKE_STATIC_LIBRARY_SUFFIX})
set_target_properties(fftw PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_BINARY_DIR}/fftw/install/lib/${lib_fftw_name})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/fftw/install/include)

add_executable(custom main.cpp timer.cpp)
add_dependencies(custom project_fftw)

TARGET_LINK_LIBRARIES(custom fftw)