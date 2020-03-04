//
// Created by david on 3/3/20.
//

#ifndef CONTRA_BOXCOLLIDER_H
#define CONTRA_BOXCOLLIDER_H

#include "CollideComponent.h"
#include "../../kernel/box.h"
#include "../../kernel/game_object.h"

class BoxCollider : public CollideComponent {
protected:
    Box m_box;

    void GetOccupiedCells(Grid::CellsSquare &square) override;

    bool IsColliding(const CollideComponent *other) override;

public:
    virtual void
    Create(BaseScene *scene, GameObject *go,
           int local_top_left_x, int local_top_left_y, int width, int height, int layer, int checkLayer) {
        Create(scene, go, {
                local_top_left_x,
                local_top_left_y,
                local_top_left_x + width,
                local_top_left_y + height
        }, layer, checkLayer);
    }

    virtual void
    Create(BaseScene *scene, GameObject *go, Box box, int layer, int checkLayer) {
        CollideComponent::Create(scene, go, layer, checkLayer);
        m_box = box;
    }

    void ChangeBox(const Box &box) {
        m_box = box;
    }

    void Update(float dt) override;

    [[nodiscard]] float AbsoluteTopLeftX() const {
        return float(go->position.x) + float(m_box.top_left_x);
    }

    [[nodiscard]] float AbsoluteTopLeftY() const {
        return float(go->position.y) + float(m_box.top_left_y);
    }

    [[nodiscard]] float AbsoluteBottomRightX() const {
        return float(go->position.x) + float(m_box.bottom_right_x);
    }

    [[nodiscard]] float AbsoluteBottomRightY() const {
        return float(go->position.y) + float(m_box.bottom_right_y);
    }
};

#endif //CONTRA_BOXCOLLIDER_H
