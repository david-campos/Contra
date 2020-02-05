//
// Created by david on 4/2/20.
//

#ifndef CONTRA_ANIMATIONRENDERER_H
#define CONTRA_ANIMATIONRENDERER_H


#include "component.h"

/**
 * Component that allows to render animations from a single spritesheet.
 * The component expects the frames of each animation to be horizontal,
 * contiguous and have fixed size.
 */
class AnimationRenderer : public RenderComponent {
public:
    /** Set to true to horizontally mirror the sprite, it will only work if a mirror sprite was created */
    bool mirrorHorizontal;
    struct Animation {
        /** Top-left corner of the first frame of the animation inside the sprite */
        int start_x, start_y;
        /** 1/FPS (seconds per frame) */
        float speed;
        /** Total frames */
        short frames;
        /** Frames width and height */
        int frame_w, frame_h;
        /** Anchor that defines where in the frame the object position is */
        int anchor_x, anchor_y;
        /** Name to obtain the animation */
        std::string name;
        /** Whether the animation loops or stops at frame 0 each time is played */
        bool loop;
    };

    // Adds an animation to the renderer and returns the index
    int AddAnimation(Animation animation);

    /** Plays the indicated animation */
    void PlayAnimation(int index);
    /**
     * If the animation is not being played right now, it changes
     * the current animation to it but pausing it in the first frame.
     */
    void CurrentAndPause(int index);

    /** Plays the animation if it was stopped */
    void Play();
    /** Pauses the animation */
    void Pause();
    /** Stops the animation if it was playing */
    void Stop();

    /** Find an animation by name, -1 indicates the animation was not found */
    int FindAnimation(std::string name);

    /** Indicates if the renderer is playing an animation */
    bool IsPlaying() { return playing; }

    /** Indicates if the renderer is playing the given animation */
    bool IsPlaying(int index) { return IsPlaying() && IsCurrent(index); }

    /** Indicates if the animation is the currently selected */
    bool IsCurrent(int animationIndex);

    void Update(float dt) override;
private:
    std::vector<AnimationRenderer::Animation> m_animations;
    Animation *m_currentAnimation = nullptr;
    float m_currentTime;
    bool playing;
};


#endif //CONTRA_ANIMATIONRENDERER_H
