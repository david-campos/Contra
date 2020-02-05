//
// Created by david on 4/2/20.
//

#ifndef CONTRA_PLAYER_H
#define CONTRA_PLAYER_H

#define PLAYER_SPEED 100

#include "component.h"
#include "game_object.h"
#include "AnimationRenderer.h"

class Player: public GameObject {

};

class PlayerControl: public Component {
public:
    void Init() override;
    void Update(float dt) override;
private:
    AnimationRenderer* m_animator;
    float m_shootDowntime;
    bool m_facingRight;
    int m_idleAnim, m_upAnim, m_crawlAnim,
    m_runAnim, m_runUpAnim, m_runDownAnim,
    m_jumpAnim, m_dieAnim, m_runShootAnim,
    m_splashAnim, m_swimAnim, m_diveAnim,
    m_swimShootAnim, m_swimShootDiagonalAnim,
    m_swimShootUpAnim;
};

#endif //CONTRA_PLAYER_H
