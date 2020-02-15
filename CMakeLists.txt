cmake_minimum_required(VERSION 3.15)

project(Contra)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
#find_package(glm REQUIRED)
#find_package(GLEW REQUIRED)
#find_package(OpenGL REQUIRED)

add_executable(Contra main.cpp avancezlib.cpp component.cpp game_object.cpp object_pool.h vector2D.h AnimationRenderer.cpp AnimationRenderer.h Player.cpp Player.h floor.h Gravity.cpp Gravity.h SimpleRenderer.h bullets.h canons.cpp canons.h grid_cell.cpp weapons.h enemies.cpp enemies.h level.cpp level.h yaml_converters.h pickups.h)

file(COPY data DESTINATION .)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIR}
#        ${GLM_INCLUDE_DIRS}
        #        ${GLEW_INCLUDE_DIRS}
        #        ${OPENGL_INCLUDE_DIR}
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${SDL2_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_IMAGE_LIBRARY}
        yaml-cpp
        #        ${GLEW_LIBRARIES}
        #        ${OPENGL_LIBRARY}
        )

