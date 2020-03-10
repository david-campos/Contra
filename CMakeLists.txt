cmake_minimum_required(VERSION 3.15)

project(Contra)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if (MINGW)
    set(SDL2_PATH "${CMAKE_SOURCE_DIR}/cmake/mingw/SDL2-2.0.10/i686-w64-mingw32")
    set(SDL2_TTF_PATH "${CMAKE_SOURCE_DIR}/cmake/mingw/SDL2_ttf-2.0.15/i686-w64-mingw32")
    set(SDL2_IMAGE_PATH "${CMAKE_SOURCE_DIR}/cmake/mingw/SDL2_image-2.0.5/i686-w64-mingw32")
    set(SDL2_MIXER_PATH "${CMAKE_SOURCE_DIR}/cmake/mingw/SDL2_mixer-2.0.4/i686-w64-mingw32")
    add_subdirectory(third-party/yaml-cpp)
endif ()

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)

add_executable(Contra main.cpp src/kernel/avancezlib.cpp src/kernel/game_object.cpp src/kernel/object_pool.h src/kernel/vector2D.h src/components/render/AnimationRenderer.cpp src/components/render/AnimationRenderer.h src/contra/entities/Player.cpp src/contra/entities/Player.h src/contra/components/floor.h src/contra/components/Gravity.cpp src/contra/components/Gravity.h src/components/render/SimpleRenderer.h src/contra/entities/bullets.h src/contra/entities/canons.cpp src/contra/entities/canons.h src/components/collision/grid.cpp src/contra/entities/weapons.h src/contra/entities/enemies.cpp src/contra/entities/enemies.h src/contra/level/level.cpp src/contra/level/level.h src/contra/level/yaml_converters.h src/contra/entities/pickups.h src/contra/entities/pickup_types.h src/contra/entities/exploding_bridge.h src/contra/entities/defense_wall.h src/contra/menus.h src/components/scene.h src/contra/menus.cpp src/contra/game.cpp src/contra/player_stats.h src/contra/level/level_component.h src/components/render/RenderComponent.h src/components/collision/CollideComponent.h src/components/collision/CollideComponent.cpp src/components/collision/BoxCollider.h src/components/collision/BoxCollider.cpp src/kernel/box.h src/contra/level/scrolling_level.h src/contra/level/scrolling_level.cpp src/contra/level/level_factory.h src/contra/level/perspective_level.h src/contra/level/perspective_level.cpp src/contra/entities/perspective/cores.h src/contra/entities/explosion.h src/components/sound_effect.h src/contra/hittable.h src/contra/entities/perspective/pers_enemies.h src/contra/level/perspective_const.h src/contra/entities/perspective/exploding_pill.h src/contra/entities/weapon_types.h src/contra/entities/perspective/darr.h src/contra/entities/perspective/garmakilma.h src/contra/entities/perspective/hidden_destroyable.h)

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/data)
file(COPY data DESTINATION .)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIR}
        ${SDL2_MIXER_INCLUDE_DIR}
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${SDL2_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_IMAGE_LIBRARY}
        ${SDL2_MIXER_LIBRARY}
        yaml-cpp
        )

