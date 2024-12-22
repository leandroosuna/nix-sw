#pragma once

#include "TouchLib.h"
#include "common.h"

#define MAX_TOUCH_QUEUE_POINTS 50
#define UPDATE_MS 10
#define TAP_THRESHOLD 20 / UPDATE_MS

enum GestureType {
    NONE,
    TAP,
    SWIPE,
    LONG_PRESS,
    DRAG_DROP,
    PINCH,
    EXPAND
};
struct Point{uint16_t x; uint16_t y;};

struct Gesture {
    GestureType type;
    Point start;
    Point end;
};

struct Fingers
{
    Point a;
    Point b;
};




class TouchHandler {
private:
    static uint16_t x1, y1, x2, y2;
    static bool touched;
    static uint8_t touchQueueIndex;
    static Gesture gesture;

public:
    static void init();
    static void update();
    static void calculateGesture();
    static Gesture getGesture();
};
