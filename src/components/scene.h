//
// Created by david on 22/2/20.
//

#ifndef CONTRA_SCENE_H
#define CONTRA_SCENE_H

#include <memory>
#include <queue>
#include "../kernel/game_object.h"
#include "../kernel/avancezlib.h"
#include "../consts.h"
#include "collision/grid.h"

class BaseScene : public GameObject {
protected:
    std::queue<std::pair<GameObject *, int>> game_objects_to_add;
    std::unique_ptr<Sprite> m_background;
    AvancezLib *m_engine;
    Vector2D m_camera;
    std::set<GameObject *> *game_objects[RENDERING_LAYERS];
    Grid m_grid;
    Vector2D m_animationShift;
    float m_time = 0.f;
    float m_animationShiftTime;
    std::unique_ptr<Music> m_music;
public:
    BaseScene() {
        for (int i = 0; i < RENDERING_LAYERS; i++) {
            game_objects[i] = new std::set<GameObject *>();
        }
    }

    ~BaseScene() {
        for (int i = 0; i < RENDERING_LAYERS; i++) {
            delete game_objects[i];
        }
    }

    void Create(AvancezLib *engine, const char *background_path, const char *music_path = nullptr,
                const Vector2D anim_shift = Vector2D(0, 0), const float anim_shift_time = 0.2f) {
        GameObject::Create();
        m_engine = engine;
        m_animationShift = anim_shift;
        m_animationShiftTime = anim_shift_time;
        if (background_path != nullptr) {
            m_background.reset(m_engine->createSprite(background_path));
        }
        if (music_path != nullptr) {
            m_music.reset(m_engine->createMusic(music_path));
        }
        m_camera = Vector2D(0, 0);
    }

    void Update(float dt) override {
        GameObject::Update(dt);

        m_time += dt;

        if (m_background) {
            // Draw background (smoothing the zoom)
            int camera_without_zoom_x = int(floorf(m_camera.x / PIXELS_ZOOM));
            int shift_x = -int(roundf(m_camera.x - camera_without_zoom_x * PIXELS_ZOOM));
            if (camera_without_zoom_x < 0) {
                shift_x -= camera_without_zoom_x * PIXELS_ZOOM;
                camera_without_zoom_x = 0;
            }
            int camera_without_zoom_y = int(floorf(m_camera.y / PIXELS_ZOOM));
            int shift_y = -int(roundf(m_camera.y - camera_without_zoom_y * PIXELS_ZOOM));

            bool use_animation_shift = fmod(m_time, 2 * m_animationShiftTime) < m_animationShiftTime;

            // We need to add an extra pixel if we shift slightly bc if not we will have black pixels
            // the reason why we don't do it ALWAYS is because if the background image has
            // the exact same height as the window (scaled by PIXELS_ZOOM), trying to get 1px more
            // will deform the image, this is not necessary if there is no shifting so we just avoid it.
            m_background->draw(shift_x, shift_y,
                    WINDOW_WIDTH + (shift_x == 0 ? 0 : PIXELS_ZOOM),
                    WINDOW_HEIGHT + (shift_y == 0 ? 0 : PIXELS_ZOOM),
                    camera_without_zoom_x + (use_animation_shift ? m_animationShift.x : 0),
                    camera_without_zoom_y + (use_animation_shift ? m_animationShift.y : 0),
                    WINDOW_WIDTH / PIXELS_ZOOM + (shift_x == 0 ? 0 : 1),
                    WINDOW_HEIGHT / PIXELS_ZOOM + (shift_y == 0 ? 0 : 1));
        }

        m_grid.ClearCollisionCache(); // Clear collision cache
        for (const auto &layer: game_objects) {
            // Update objects which are enabled and not to be removed
            for (auto *game_object : *layer)
                if (game_object->IsEnabled() && !game_object->IsMarkedToRemove())
                    game_object->Update(dt);
        }
        // Delete objects marked to remove
        for (const auto &layer: game_objects) {
            std::set<GameObject *>::iterator it = layer->begin();
            while (it != layer->end()) {
                auto *game_object = *it;
                if (game_object->IsMarkedToRemove()) {
                    it = layer->erase(it);
                    game_object->UnmarkToRemove();
                    if (game_object->onRemoval == DESTROY) {
                        game_object->Destroy();
                    }
                } else {
                    it++;
                }
            }
        }
        // Add new ones
        while (!game_objects_to_add.empty()) {
            std::pair<GameObject *, int> next = game_objects_to_add.front();
            game_objects_to_add.pop();
            game_objects[next.second]->insert(next.first);
        }

    }

    void FadeOutMusic(int ms = 1000) {
        m_engine->FadeOutMusic(ms);
    }

    void Destroy() override {
        GameObject::Destroy();
        while (!game_objects_to_add.empty()) {
            game_objects_to_add.front().first->Destroy();
            game_objects_to_add.pop();
        }
        for (const auto &layer: game_objects) {
            for (auto game_object : *layer)
                game_object->Destroy();
        }
        m_background.reset();
    }

    /**
     * Adds a game object to the queue to be added before the next frame
     * @param game_object
     * @param layer
     */
    void AddGameObject(GameObject *const game_object, const int layer) {
        game_objects_to_add.push(std::pair<GameObject *, int>(game_object, layer));
    }

    /**
     * Just a convenient method in case someone is confused about how to safely remove objects.
     * Marks the object to be removed, GameObject::MarkToRemove can safely be used instead
     * @param game_object
     */
    void RemoveGameObject(GameObject *game_object) { game_object->MarkToRemove(); }

    /**
     * Removes immediately the object from the layer.
     * @warning Prefer Level::RemoveGameObject instead, as immediately removing an
     * object while iterating over the layer can cause memory issues.
     * @param game_object
     * @param layer
     */
    void RemoveImmediately(GameObject *game_object, const int layer) {
        game_objects[layer]->erase(game_object);
        if (game_object->onRemoval == DESTROY) {
            game_object->Destroy();
        }
    }

    /**
     * @return The x position of the camera, this corresponds to the exact left side of the same.
     */
    float GetCameraX() const {
        return m_camera.x;
    }

    /**
     * @return The y position of the camera, this corresponds to the exact top side of the same
     */
    float GetCameraY() const {
        return m_camera.y;
    }

    Grid *GetGrid() {
        return &m_grid;
    }

    [[nodiscard]] AvancezLib *GetEngine() const {
        return m_engine;
    }

    void Receive(Message m) override {
        Send(m); // Bubble up
    }
};

#endif //CONTRA_SCENE_H
