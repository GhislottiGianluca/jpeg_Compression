cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

project(jpeg_compression VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

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


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(jpeg_compression
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET jpeg_compression APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(jpeg_compression SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(jpeg_compression
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(jpeg_compression PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
add_dependencies(jpeg_compression project_fftw)

set_target_properties(jpeg_compression PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS jpeg_compression
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(jpeg_compression)
endif()
