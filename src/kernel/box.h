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
};

#endif //CONTRA_BOX_H
