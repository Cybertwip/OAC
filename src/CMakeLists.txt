set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)

# opengl library wrapper
set(GLCPP_SRC
    glcpp/window.cpp
    glcpp/camera.cpp
    glcpp/application.hpp
)
if(APPLE)
  SET(CMAKE_OSX_DEPLOYMENT_TARGET 12.4)
endif()

add_library(glcpp STATIC ${GLCPP_SRC})

set(LIBs STB_IMAGE imgui glad SmallFBX jsoncpp_static)

if(MSVC)
   target_compile_options(glcpp PUBLIC  "/utf-8")
  if (CMAKE_BUILD_TYPE MATCHES Release)
    target_link_options(glcpp PUBLIC "/INCREMENTAL" "/SUBSYSTEM:windows" "/ENTRY:mainCRTStartup")
 endif()
  target_link_libraries(glcpp PUBLIC ${LIBs} glm)
  target_include_directories(glcpp PUBLIC ./ ./glcpp ../ ${CMAKE_HOME_DIRECTORY}/external)


elseif(WIN32)
    target_compile_options(glcpp PRIVATE -Wall -Werror -std=c++17 -static)

    target_include_directories(glcpp PUBLIC ./ ./glcpp ../ ${CMAKE_HOME_DIRECTORY}/external)
    target_link_libraries(glcpp PUBLIC ${LIBs} glm -lpthread -lgdi32 -lopengl32 -static)
else()
    target_compile_options(glcpp PRIVATE -Wall -Werror -std=c++17)

    target_include_directories(glcpp PUBLIC ./ ./glcpp ../ ${CMAKE_HOME_DIRECTORY}/external)
    target_link_libraries(glcpp PUBLIC ${LIBs} glm::glm -ldl -lpthread)
endif()

message("==========================================================")
# python
# add_executable(test12  cpython/main.cpp cpython/py_manager.cpp)
# target_link_libraries(test12 PUBLIC Python3::Python pybind11::embed)
# add_library(py  cpython/py_manager.cpp)
# target_link_libraries(py PUBLIC Python3::Python pybind11::embed)

# new
set(test_src
    graphics/mesh.cpp
    graphics/opengl/gl_shader.cpp
    graphics/opengl/gl_mesh.cpp
    graphics/opengl/framebuffer.cpp
    graphics/opengl/image.cpp
    graphics/post_processing.cpp

    graphics/shader.h
    graphics/mesh.h
    graphics/opengl/gl_shader.h
    graphics/opengl/gl_mesh.h
    graphics/opengl/framebuffer.h
    graphics/opengl/image.h
    graphics/post_processing.h

    util/log.h
    util/utility.h

    entity/entity.cpp
    entity/components/transform_component.cpp
    entity/components/animation_component.cpp
    entity/components/pose_component.cpp
    entity/components/renderable/armature_component.cpp
    entity/components/renderable/mesh_component.cpp
    
    entity/entity.h
    entity/components/component.h
    entity/components/transform_component.h
    entity/components/animation_component.h
    entity/components/pose_component.h
    entity/components/renderable/mesh_component.h
    entity/components/renderable/armature_component.h

    resources/exporter.cpp
    resources/exporter.h
    resources/importer.cpp
    resources/importer.h
    resources/model.cpp
    resources/model.h
    resources/shared_resources.cpp
    resources/shared_resources.h

    animation/animator.cpp
    animation/sequence_animator.cpp
    animation/animation.cpp
    animation/json_animation.cpp
    animation/assimp_animation.cpp
    animation/bone.cpp
    animation/retargeter.cpp

    animation/animator.h
    animation/sequence_animator.h
    animation/animation.h
    animation/assimp_animation.h
    animation/json_animation.h
    animation/bone.h
    animation/retargeter.h
)
add_library(test STATIC ${test_src})


if(MSVC)
target_compile_options(test PUBLIC  "/utf-8")
endif()
target_link_libraries(test PUBLIC ${LIBs} glm)
target_include_directories(test PUBLIC ./
 ./entity/
  ./entity/components/
   ./entity/components/renderable 
   ./graphics 
   ./graphics/opengl 
   ./util
   ./animation
   ./resources
   ../ ${CMAKE_HOME_DIRECTORY}/external)


message("==========================================================")

set(BUNDLE)
set(APP_ICON)
if(APPLE)
    set(BUNDLE MACOSX_BUNDLE)

    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)

    # And this part tells CMake where to find and install the file itself
    set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/../AppIcon.icns)
    set_source_files_properties(${APP_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")

endif()

add_executable(OAC 
${BUNDLE}
${APP_ICON}
./main.cpp 
 
./UI/hierarchy_layer.cpp
./UI/component_layer.cpp
./UI/scene_layer.cpp
./UI/scene_layer.h
./UI/text_edit_layer.cpp
./UI/timeline_layer.cpp
./UI/main_layer.cpp
./UI/main_layer.h
./UI/imgui_helper.cpp
./UI/ui_context.h

event/event_history.h
event/event_history.cpp

./app.cpp
./scene/scene.hpp
./scene/main_scene.cpp
)

target_include_directories(OAC  PUBLIC ../external/smallfbx)
target_include_directories(test  PUBLIC ../external/smallfbx)

if(MSVC)
  target_compile_options(OAC PUBLIC   "/utf-8")#  target_link_options(OAC PUBLIC "/INCREMENTAL")
  if (CMAKE_BUILD_TYPE  MATCHES Release)
      target_link_options(OAC PUBLIC "/INCREMENTAL" "/SUBSYSTEM:windows" "/ENTRY:mainCRTStartup")
  endif()
#   target_link_libraries(OAC  glcpp test py)
target_link_libraries(OAC  glcpp test )
elseif(WIN32)
    target_compile_options(OAC  PRIVATE -Wall -Werror -std=c++17 -static)
    target_link_libraries(OAC  glcpp)
    target_include_directories(OAC  PUBLIC ./)
else()
# target_link_libraries(OAC  glcpp test py)
    target_link_libraries(OAC  glcpp test)
endif()

message("==========================================================")

# Set the resource files to be included in the bundle
set(RESOURCE_DIRECTORIES
    resources
)
set(RESOURCE_FILES
    imgui.ini
    LICENSE
)


if(APPLE)
    
    # Copy resource files to the binary directory
    foreach(resource_file ${RESOURCE_DIRECTORIES})
        add_custom_command(TARGET OAC POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/${resource_file} $<TARGET_FILE_DIR:OAC>/../Resources/${resource_file})
    endforeach()

    foreach(resource_file ${RESOURCE_FILES})
        add_custom_command(TARGET OAC POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/${resource_file} $<TARGET_FILE_DIR:OAC>/../Resources/${resource_file})
    endforeach()


    # Set macOS bundle properties
    set_target_properties(OAC PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist  # Provide your Info.plist file
    )

        # Disable code signing for the entire project
    set_target_properties(OAC PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    )
    
    # Set release build type and enable optimizations
    set(CMAKE_BUILD_TYPE Release)

else()
    foreach(resource_file ${RESOURCE_DIRECTORIES})
        add_custom_command(TARGET OAC POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/${resource_file} $<TARGET_FILE_DIR:OAC>/${resource_file})
    endforeach()

    foreach(resource_file ${RESOURCE_FILES})
        add_custom_command(TARGET OAC POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/${resource_file} $<TARGET_FILE_DIR:OAC>/${resource_file})
    endforeach()

endif()