#include "touch_handler.h"

TouchLib touch(Wire, PIN_IIC_SDA, PIN_IIC_SCL, CTS328_SLAVE_ADDRESS, PIN_TOUCH_RES);

// hw_timer_t *hwTimer = NULL;

// void IRAM_ATTR updateTimer()
// {
//     TouchHandler::update();
// }

Fingers touchQueue[MAX_TOUCH_QUEUE_POINTS]; 

uint16_t TouchHandler::x1;
uint16_t TouchHandler::y1;
uint16_t TouchHandler::x2;
uint16_t TouchHandler::y2;
bool TouchHandler::touched;
uint8_t TouchHandler::touchQueueIndex;
Gesture TouchHandler::gesture;


void TouchHandler::init(){
    x1 = UINT16_MAX;
    y1 = UINT16_MAX;
    x2 = UINT16_MAX;
    y2 = UINT16_MAX;
    touched = false;
    touchQueueIndex = 0;
    gesture = {GestureType::NONE, {UINT16_MAX, UINT16_MAX}, {UINT16_MAX, UINT16_MAX}};

    if (!touch.init()) {
        debugln("Touch IC not found");
        return;
    }
    
    for(int i = 0; i< MAX_TOUCH_QUEUE_POINTS; i++){
        touchQueue[i] = {UINT16_MAX, UINT16_MAX};
    }

    uint8_t timer_id = 0;
    uint16_t prescaler = 80; // Between 0 and 65 535
    int threshold = 800; // 10 ms (800 * (1/80M) = 10 ms) // 64 bits value 

    // hwTimer = timerBegin(timer_id, prescaler, true);
    // timerAttachInterrupt(hwTimer, &updateTimer, true);
    // timerAlarmWrite(hwTimer, threshold, true);
    // timerAlarmEnable(hwTimer);

}
bool distanceGreaterThan(Point a, Point b, uint16_t distance)
{
    if(a.x == UINT16_MAX || b.x == UINT16_MAX)
    {
        return true;
    }

    return (abs(a.x - b.x) > distance || abs(a.y - b.y) > distance);
}
bool shown = false;
void TouchHandler::update() {
    if (touch.read())
    {
        x1 = touch.getPoint(0).x;
        y1 = touch.getPoint(0).y;
        
        if(touch.getPointNum() >= 2)
        {
            x2 = touch.getPoint(1).x;
            y2 = touch.getPoint(1).y;
        }  
        else
        {
            x2 = UINT16_MAX;
            y2 = UINT16_MAX;
        } 
        touchQueue[touchQueueIndex] = {x1, y1, x2, y2};

        touched = true;
        if(!shown)
        {
            debugln("td");
            shown = true;
        }
    }
    else
    {
        touchQueue[touchQueueIndex] = {UINT16_MAX, UINT16_MAX};
    }

    TouchHandler::calculateGesture();
    touchQueueIndex++;
    touchQueueIndex%=MAX_TOUCH_QUEUE_POINTS;
    
}



void TouchHandler::calculateGesture()
{
    gesture = {GestureType::NONE, {UINT16_MAX, UINT16_MAX}, {UINT16_MAX, UINT16_MAX}};
    
    Point a = touchQueue[touchQueueIndex].a;
    Point b = touchQueue[touchQueueIndex].b;
    
    
    

    if(a.x == UINT16_MAX && a.y == UINT16_MAX && touched) //lifted finger after touch
    {
        uint8_t prevIndex;
        bool isTap = true;

        Point prevPoint = touchQueue[touchQueueIndex - 1].a;
        for(int i = 0; i < TAP_THRESHOLD; i++)
        {
            prevIndex = (touchQueueIndex - 2 - i); //%MAX_TOUCH_QUEUE_POINTS;
            
            if(distanceGreaterThan(prevPoint, touchQueue[prevIndex].a, 5))
            {
                isTap = false;
                break;
            }            

        }
        if(isTap)
        {
            gesture = {GestureType::TAP, prevPoint, prevPoint};
            debug("T ");
            debug(prevPoint.x);
            debug(",");
            debugln(prevPoint.y);
        }
        else
            gesture = {GestureType::NONE, {UINT16_MAX, UINT16_MAX}, {UINT16_MAX, UINT16_MAX}};

        int c = 0;
        for(int i = 0; i < MAX_TOUCH_QUEUE_POINTS; i++)
        {
            if(touchQueue[i].a.x == UINT16_MAX)
            {
                c++;
            }
        }
        if(c == MAX_TOUCH_QUEUE_POINTS)
        {
            touched = false;
            shown = false;
            debugln("tu");
        }
    } 
    else
    {
        
    }



}

Gesture TouchHandler::getGesture() {

    return gesture;
}
