//
// Created by david on 6/2/20.
//

#ifndef CONTRA_SIMPLERENDERER_H
#define CONTRA_SIMPLERENDERER_H

#include <utility>

#include "component.h"
#include "consts.h"

class SimpleRenderer : public RenderComponent {
private:
    int m_srcX, m_srcY, m_width, m_height, m_anchorX, m_anchorY;
public:
    void Update(float dt) override {
        if (!go->IsEnabled())
            return;

        sprite->draw((int) round(go->position.x - level->GetCameraX()) - m_anchorX * PIXELS_ZOOM,
                (int) round(go->position.y) - m_anchorY * PIXELS_ZOOM,
                m_width * PIXELS_ZOOM, m_height * PIXELS_ZOOM,
                m_srcX, m_srcY, m_width, m_height);
    }

    void Create(Level *level, GameObject *go,
                std::shared_ptr<Sprite> sprite, int srcX, int srcY, int width, int height,
                int anchorX, int anchorY) {
        RenderComponent::Create(level, go, std::move(sprite));
        m_srcX = srcX;
        m_srcY = srcY;
        m_width = width;
        m_height = height;
        m_anchorX = anchorX;
        m_anchorY = anchorY;
    }

    void ChangeCoords(int srcX, int srcY, int width, int height, int anchorX, int anchorY) {
        m_srcX = srcX;
        m_srcY = srcY;
        m_width = width;
        m_height = height;
        m_anchorX = anchorX;
        m_anchorY = anchorY;
    }
};

#endif //CONTRA_SIMPLERENDERER_H
