#pragma once

#include <cmath>

class Vector2D {
public:
    double x;
    double y;

    Vector2D operator+(const Vector2D &rhs) {
        return Vector2D(x + rhs.x, y + rhs.y);
    }

    Vector2D operator-(const Vector2D &rhs) {
        return Vector2D(x - rhs.x, y - rhs.y);
    }

    Vector2D operator*(const Vector2D &rhs) {
        return Vector2D(x * rhs.x, y * rhs.y);
    }

    Vector2D operator/(const double &rhs) {
        return Vector2D(x / rhs, y / rhs);
    }

    Vector2D operator*(const double &rhs) {
        return Vector2D(x * rhs, y * rhs);
    }

    double dotProduct(const Vector2D &rhs) {
        return x * rhs.x + y * rhs.y;
    }

    [[nodiscard]] Vector2D rotate(const float angle) const {
        float cos = cosf(angle);
        float sin = sinf(angle);
        return Vector2D(
                cos * x - sin * y,
                sin * x + cos * y
        );
    }

    [[nodiscard]] double magnitude() const {
        return sqrt(x * x + y * y);
    }

    [[nodiscard]] double magnitudeSqr() const {
        return x * x + y * y;
    }

    [[nodiscard]] Vector2D normalise() const {
        auto len = magnitude();
        return Vector2D(x / len, y / len);
    }

    [[nodiscard]] Vector2D perpendicular() const {
        return Vector2D(-y, x);
    }

    Vector2D(double x, double y) {
        this->x = x;
        this->y = y;
    }

    Vector2D() {
        this->x = 0;
        this->y = 0;
    }

    float distance(const Vector2D &vector2D) {
        return (*this - vector2D).magnitude();
    }
};