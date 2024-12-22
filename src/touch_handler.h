#pragma once

#include "TouchLib.h"
#include "common.h"

#define MAX_TOUCH_QUEUE_POINTS 50
#define UPDATE_MS 10
#define MIN_DISTANCE_SLIDE 2
#define MIN_TAP_TIME 30 / UPDATE_MS
#define MIN_HOLD_TIME 1000 / UPDATE_MS
enum GestureType {
    NONE,
    TAP,
    SWIPE,
    DRAG,
    DROP,
    PINCH,
    EXPAND
};

enum State{
    START,
    FINGER_DOWN,
    SLIDE,
    HOLDING_DOWN,
    TWO_FINGER_DOWN,
    EXPANDING,
    PINCHING
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

struct Distance 
{
    int x, y;
};

class TouchHandler {
private:
    static Fingers touchCurrent;
    static Fingers touchLast;
    static uint16_t touchDownCounter;
    static Gesture gesture;
    static State state;
    static Gesture gestureOut;

public:
    static void init();
    static void update();
    static void calculateGesture();
    static Gesture getGesture();
    static void switchState(State newState);
    static void switchGestureType(GestureType newGesture);
};
