cmake_minimum_required(VERSION 3.22.1)

## 官方标准配置
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_STANDARD 20)  # 使用 C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 确保 C++20 是必需的
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -Wall")
set(GLM_DIR "${CMAKE_SOURCE_DIR}/libs/glm")
set(ANDROID_PLATFORM android-29)
include_directories(${GLM_DIR})

##ANDROID_PLATFORM_LEVEL=18
add_definitions("-DDYNAMIC_ES3")
set(OPENGL_LIB GLESv3)

add_library(native-color
        SHARED
        native_color.cpp)

add_library(native-color-lib
        SHARED
        native_color_lib.cpp
        RenderVideo.cpp)

target_link_libraries(native-color
        ${OPENGL_LIB}
        android
        EGL
        log
        m)

target_link_libraries(native-color-lib
        ${OPENGL_LIB}
        android
        EGL
        log
        mediandk
        m)
