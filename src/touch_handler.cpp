#include "touch_handler.h"

TouchLib touch(Wire, PIN_IIC_SDA, PIN_IIC_SCL, CTS328_SLAVE_ADDRESS, PIN_TOUCH_RES);

// hw_timer_t *hwTimer = NULL;

// void IRAM_ATTR updateTimer()
// {
//     TouchHandler::update();
// }


Fingers TouchHandler::touchCurrent = Fingers{{UINT16_MAX, UINT16_MAX}, {UINT16_MAX, UINT16_MAX}};
Fingers TouchHandler::touchLast = Fingers{{UINT16_MAX, UINT16_MAX}, {UINT16_MAX, UINT16_MAX}};
Gesture TouchHandler::gesture = {GestureType::NONE, {UINT16_MAX, UINT16_MAX}, {UINT16_MAX, UINT16_MAX}};
Gesture TouchHandler::gestureOut = {GestureType::NONE, {UINT16_MAX, UINT16_MAX}, {UINT16_MAX, UINT16_MAX}};
State TouchHandler::state = State::START;
uint16_t TouchHandler::touchDownCounter = 0;
Gesture gestureEmpty = {GestureType::NONE, {UINT16_MAX, UINT16_MAX}, {UINT16_MAX, UINT16_MAX}}; 
bool gestureAcknoleged = false;
void TouchHandler::init(){

    if (!touch.init()) {
        debugln("Touch IC not found");
        return;
    }
    
    // uint8_t timer_id = 0;
    // uint16_t prescaler = 80; // Between 0 and 65 535
    // int threshold = 800; // 10 ms (800 * (1/80M) = 10 ms) // 64 bits value 

    // hwTimer = timerBegin(timer_id, prescaler, true);
    // timerAttachInterrupt(hwTimer, &updateTimer, true);
    // timerAlarmWrite(hwTimer, threshold, true);
    // timerAlarmEnable(hwTimer);

}

bool isValidTouch(Point a)
{
    return a.x != UINT16_MAX && a.y != UINT16_MAX;
}
Distance distance(Fingers current, Fingers last)
{
    Distance dis; 
    dis.x = abs(current.a.x - current.b.x) - abs(last.a.x - last.b.x);
    dis.y = abs(current.a.y - current.b.y) - abs(last.a.y - last.b.y);
    return dis;
} 

bool distanceLargerThanLast(Fingers current, Fingers last)
{
    Distance dis = distance(current, last);
    return dis.x >= 0 && dis.y >= 0;
}
bool distanceGreaterThan(Point a, Point b, uint16_t distance)
{
    return (abs(a.x - b.x) > distance || abs(a.y - b.y) > distance);
}
bool shown = false;
void TouchHandler::update() 
{
    touchCurrent.a = {UINT16_MAX, UINT16_MAX};
    touchCurrent.b = {UINT16_MAX, UINT16_MAX};
    if (touch.read())
    {
        touchCurrent.a = {touch.getPoint(0).x, touch.getPoint(0).y};
        
        if(touch.getPointNum() >= 2)
        {
            touchCurrent.b = {touch.getPoint(1).x, touch.getPoint(1).y};
        }  
        
        if(!shown)
        {
            debugln("td");
            shown = true;
        }
    }
    
    TouchHandler::calculateGesture();

    touchLast = touchCurrent;
}

void TouchHandler::calculateGesture()
{
    switch(state)
    {
        case State::START: //check if a finger or both are down
            if(isValidTouch(touchCurrent.a))
            {
                if(!isValidTouch(touchCurrent.b))
                {
                    gesture.start = touchCurrent.a;
                    switchState(State::FINGER_DOWN);
                }
                else
                {
                    gesture.start.x = 0;
                    gesture.start.y = 0;
                    switchState(State::TWO_FINGER_DOWN);
                }
                
            }
            break;
        case State::FINGER_DOWN: //if finger is down check if it is a tap, slide or hold
            if(isValidTouch(touchCurrent.a))
            {
                if(isValidTouch(touchCurrent.b)) //check for 2 fingers
                {
                    gesture.start.x = 0;
                    gesture.start.y = 0;
                    switchState(State::TWO_FINGER_DOWN);
                    break;   
                }
                if(isValidTouch(touchLast.a))
                {
                    if(distanceGreaterThan(touchCurrent.a, touchLast.a, MIN_DISTANCE_SLIDE))
                    {
                        gesture.start = touchCurrent.a;
                        switchState(State::SLIDE);
                    }
                    touchDownCounter++;

                    if(touchDownCounter >= MIN_HOLD_TIME)
                    {
                        gesture.start = touchCurrent.a;
                        switchState(State::HOLDING_DOWN);
                        switchGestureType(GestureType::DRAG);
                    }
                }
            }
            else 
            {
                if(touchDownCounter >= MIN_TAP_TIME) // 
                {
                    
                    gesture.end = touchLast.a;
                    switchGestureType(GestureType::TAP);
                }
                // else
                // {
                //     switchGestureType(GestureType::START);
                // }
                switchState(State::START);
            }
            break;
        case State::SLIDE:
            if(!isValidTouch(touchCurrent.a) || !distanceGreaterThan(touchCurrent.a, gesture.start, MIN_DISTANCE_SLIDE))
            {
                gesture.end = touchLast.a;
                switchGestureType(GestureType::SWIPE);
                switchState(State::START);
            }
            break;
        case State::HOLDING_DOWN:
            if(!isValidTouch(touchCurrent.a))
            {
                gesture.end = touchLast.a;
                switchGestureType(GestureType::DROP);
                switchState(State::START);
            }
            break;
        case State::TWO_FINGER_DOWN:
            if(isValidTouch(touchCurrent.a) && isValidTouch(touchCurrent.b) && 
            isValidTouch(touchLast.a) && isValidTouch(touchLast.b))
            {
                Distance dis = distance(touchCurrent, touchLast);
                gesture.start.x += abs(dis.x);
                gesture.start.y += abs(dis.y);
                 
                if(dis.x >= 0 && dis.y >=0)
                {
                    switchGestureType(GestureType::EXPAND);
                    switchState(State::EXPANDING);
                }
                else
                {
                    switchGestureType(GestureType::PINCH);
                    switchState(State::PINCHING);
                }
            }
            break;
        case State::EXPANDING:
            if(isValidTouch(touchCurrent.a) && isValidTouch(touchCurrent.b) && 
            isValidTouch(touchLast.a) && isValidTouch(touchLast.b))
            {
                Distance dis = distance(touchCurrent, touchLast);
                if(dis.x >= 0 && dis.y >=0)
                {   
                    gesture.start.x += abs(dis.x);
                    gesture.start.y += abs(dis.y);
                }
                else
                {
                    switchGestureType(GestureType::EXPAND);
                    switchState(State::START);
                }
            }
            else
            {
                switchGestureType(GestureType::EXPAND);
                switchState(State::START);
            }

            break;
        case State::PINCHING:

            if(isValidTouch(touchCurrent.a) && isValidTouch(touchCurrent.b) && 
            isValidTouch(touchLast.a) && isValidTouch(touchLast.b))
            {
                Distance dis = distance(touchCurrent, touchLast);
                if(dis.x < 0 && dis.y < 0)
                {   
                    gesture.start.x += abs(dis.x);
                    gesture.start.y += abs(dis.y);
                }
                else
                {
                    switchGestureType(GestureType::PINCH);
                    switchState(State::START);
                }
            }
            else
            {
                switchGestureType(GestureType::PINCH);
                switchState(State::START);
            }
            break;
    }
    
}

void TouchHandler::switchState(State newState)
{
    touchDownCounter = 0;
    state = newState;
    debug("ns ");
    switch (newState)
    {
        case State::START:
            debugln("st");
            break;
        case State::FINGER_DOWN:
            debugln("fd");
            break;
        case State::SLIDE:
            debugln("sl");
            break;
        case State::HOLDING_DOWN:
            debugln("hd");
            break;
        case State::TWO_FINGER_DOWN:
            debugln("2fd");
            break;
        case State::EXPANDING:
            debugln("ex");
            break;
        case State::PINCHING:
            debugln("pi");
            break;
    }
    
}
void TouchHandler::switchGestureType(GestureType type)
{
    gesture.type = type;
    debug("G ");
    switch (type)
    {
        case GestureType::NONE:
            debugln("no ");
            break;
        case GestureType::TAP:
            debug("tap ");
            break;
        case GestureType::SWIPE:
            debug("swipe ");
            break;
        case GestureType::DRAG:
            debug("drag ");
            break;
        case GestureType::DROP:
            debug("drop ");
            break;
        case GestureType::PINCH:
            debug("pinch ");
            break;
        case GestureType::EXPAND:
            debug("expand ");
            break;
    }
    if(type != GestureType::NONE)
    {
        debug(gesture.start.x);
        debug(",");
        debug(gesture.start.y);
        debug("->");
        debug(gesture.end.x);
        debug(",");
        debugln(gesture.end.y);
    }
    gestureOut = gesture;
    gestureAcknoleged = false;
}


Gesture TouchHandler::getGesture() {

    if(!gestureAcknoleged)
    {
        gestureAcknoleged = true;
        return gestureOut;
    }
    
    return gestureEmpty;

}
