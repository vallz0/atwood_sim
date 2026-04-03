#pragma once

struct State
{
    float y;
    float v;
};

float acceleration(float m1, float m2, float g);
State stateAt(float t, float y0, float v0, float a);
