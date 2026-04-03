#include "physics.hpp"

float acceleration(float m1, float m2, float g) {
    return (m2 - m1) * g / (m1 + m2);
}

State stateAt(float t, float y0, float v0, float a) {
    State s;
    s.y = y0 + v0*t + 0.5f * a*t*t; 
    s.v = v0 + a*t;
    return s;
}