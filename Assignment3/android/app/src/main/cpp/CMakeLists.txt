cmake_minimum_required(VERSION 3.22.1)
project("glpiframeworkintro")

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT ANDROID)
    message(FATAL_ERROR "This CMakeLists.txt is for Android builds only.")
endif()

add_definitions(-DANDROID)

set(SCENE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Scene)

# GLM is header-only; fetch it independently for the Android build so this
# module never depends on a prior desktop configure step.
include(FetchContent)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# Android Programming Quiz 3: renderer split into Scene3D (Fan + Ground) and
# SceneHUD (buttons + speed bar) -- see Scene/Renderer.cpp.
add_library(glpiframeworkintro SHARED
        NativeTemplate.cpp
        ${SCENE_DIR}/Renderer.cpp
        ${SCENE_DIR}/Transform.cpp
        ${SCENE_DIR}/Fan.cpp
        ${SCENE_DIR}/Ground.cpp
        ${SCENE_DIR}/Scene3D.cpp
        ${SCENE_DIR}/SceneHUD.cpp
)

target_include_directories(glpiframeworkintro PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SCENE_DIR}
)

target_link_libraries(glpiframeworkintro glm::glm)

find_library(log-lib     log)
find_library(android-lib android)
find_library(glesv3-lib  GLESv3)
find_library(egl-lib     EGL)

target_link_libraries(glpiframeworkintro
        ${log-lib}
        ${android-lib}
        ${glesv3-lib}
        ${egl-lib}
)
