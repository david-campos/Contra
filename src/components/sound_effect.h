//
// Created by david on 4/3/20.
//

#ifndef CONTRA_SOUND_EFFECT_H
#define CONTRA_SOUND_EFFECT_H

#include "../kernel/component.h"
#include "../kernel/avancezlib.h"

class SoundEffectComponent : public Component {
private:
    bool m_playOnStart;
    bool m_firstFrame;
    int m_times;
    SoundEffect *m_sound;
public:
    /**
     * @warning the component will not destroy the sound when destroyed, make sure your sounds are deleted when
     * necessary!
     */
    void Create(BaseScene *scene, GameObject *go, SoundEffect *sound, int timesToPlay = 1, bool playOnStart = true) {
        Component::Create(scene, go);
        m_sound = sound;
        m_times = timesToPlay;
        m_playOnStart = playOnStart;
    }

    void Init() override {
        Component::Init();
        m_firstFrame = true;
    }

    void Update(float dt) override {
        if (m_playOnStart && m_firstFrame) {
            Play();
            m_firstFrame = false;
        }
    }

    void Play() {
        if (m_sound) m_sound->Play(m_times);
    }
};

#endif //CONTRA_SOUND_EFFECT_H
