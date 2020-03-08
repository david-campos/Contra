//
// Created by david on 3/3/20.
//

#ifndef CONTRA_BOX_H
#define CONTRA_BOX_H

struct Box {
    int top_left_x;
    int top_left_y;
    int bottom_right_x;
    int bottom_right_y;

    Box operator*(const int &rhs) const {
        return Box{
                top_left_x * rhs,
                top_left_y * rhs,
                bottom_right_x * rhs,
                bottom_right_y * rhs
        };
    }

    int width() const { return bottom_right_x - top_left_x; }

    int height() const { return bottom_right_y - top_left_y; }

    Vector2D center() const { return Vector2D(width() / 2, height() / 2); }
};

#endif //CONTRA_BOX_H
