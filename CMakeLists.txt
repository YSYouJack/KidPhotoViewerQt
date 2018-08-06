cmake_minimum_required(VERSION 3.1.0)

project(KidPhotoViewerQt)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

# Populate a CMake variable with the sources
set(SRCS
    exif.cpp
    KidPhotoViewer.cpp
    main.cpp
)

if (MSVC OR CMAKE_GENERATOR STREQUAL Xcode)
	set(SRCS ${SRCS} exif.h KidPhotoViewer.h)
endif ()


add_executable(KidPhotoViewerQt WIN32 ${SRCS})

# Use the Widgets module from Qt 5
target_link_libraries(KidPhotoViewerQt Qt5::Widgets)