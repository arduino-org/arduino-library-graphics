#include "stm32f4xx_hal.h"
#include "star_ts.h"

#define TS_I2C_ADDRESS          ((uint16_t)0x54)
// TODO: move this defines to common IRQ code
#define STAR_TS_IRQ_PRI         5
#define STAR_TS_IRQ_SUB         0

// #ifdef BOARD_DISCO469
// #define TS_INT_PIN                      ((uint32_t)GPIO_PIN_5)
// #define TS_INT_GPIO_PORT                ((GPIO_TypeDef*)GPIOJ)
// #define TS_INT_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOJ_CLK_ENABLE()
// #define TS_INT_GPIO_CLK_DISABLE()       __HAL_RCC_GPIOJ_CLK_DISABLE()
// #define TS_INT_EXTI_IRQn                EXTI9_5_IRQn
// #define TS_INT_IRQ_HANDLER              __irq_exti9_5
// #else
#define TS_INT_PIN                      ((uint32_t)GPIO_PIN_0)
#define TS_INT_GPIO_PORT                ((GPIO_TypeDef*)GPIOI)
#define TS_INT_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOI_CLK_ENABLE()
#define TS_INT_GPIO_CLK_DISABLE()       __HAL_RCC_GPIOI_CLK_DISABLE()
#define TS_INT_EXTI_IRQn                EXTI0_IRQn
#define TS_INT_IRQ_HANDLER              __irq_exti0
//#endif

static TS_DrvTypeDef *ts_driver;
static uint8_t ts_orientation;
static uint8_t I2C_Address = 0;

#if 0
const char * ts_event_string_tab[TOUCH_EVENT_NB_MAX] = {
    "None",
    "Press down",
    "Lift up",
    "Contact"
};

const char * ts_gesture_id_string_tab[GEST_ID_NB_MAX] = {
    "None",
    "Move Up",
    "Move Right",
    "Move Down",
    "Move Left",
    "Zoom In",
    "Zoom Out"
};
#endif

static uint8_t GetState(TS_StateTypeDef *TS_State)
{
    uint8_t ts_status = TS_OK;
    static uint32_t _x[TS_MAX_NB_TOUCH] = { 0, 0 };
    static uint32_t _y[TS_MAX_NB_TOUCH] = { 0, 0 };
    uint16_t tmp;
    uint16_t Raw_x[TS_MAX_NB_TOUCH];
    uint16_t Raw_y[TS_MAX_NB_TOUCH];
    uint16_t xDiff;
    uint16_t yDiff;
    uint32_t index;
#if (TS_MULTI_TOUCH_SUPPORTED == 1)
    uint32_t weight = 0;
    uint32_t area = 0;
    uint32_t event = 0;
#endif

    TS_State->touchDetected = ts_driver->DetectTouch(I2C_Address);
    if (TS_State->touchDetected) {
        for (index=0; index < TS_State->touchDetected; index++) {
            ts_driver->GetXY(I2C_Address, &(Raw_x[index]), &(Raw_y[index]));
            if (ts_orientation & TS_SWAP_XY) {
                tmp = Raw_x[index];
                Raw_x[index] = Raw_y[index];
                Raw_y[index] = tmp;
            }
            if (ts_orientation & TS_SWAP_X)
                Raw_x[index] = FT_6206_MAX_WIDTH - 1 - Raw_x[index];
            if (ts_orientation & TS_SWAP_Y)
                Raw_y[index] = FT_6206_MAX_HEIGHT - 1 - Raw_y[index];

            xDiff = Raw_x[index] > _x[index]? (Raw_x[index] - _x[index]): (_x[index] - Raw_x[index]);
            yDiff = Raw_y[index] > _y[index]? (Raw_y[index] - _y[index]): (_y[index] - Raw_y[index]);

            if ((xDiff + yDiff) > 5) {
                _x[index] = Raw_x[index];
                _y[index] = Raw_y[index];
            }

            TS_State->touchX[index] = _x[index];
            TS_State->touchY[index] = _y[index];

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
            ft6x06_TS_GetTouchInfo(I2C_Address, index, &weight, &area, &event);
            TS_State->touchWeight[index] = weight;
            TS_State->touchArea[index]   = area;
            switch(event) {
                case FT6206_TOUCH_EVT_FLAG_PRESS_DOWN  :
                    TS_State->touchEventId[index] = TOUCH_EVENT_PRESS_DOWN;
                    break;
                case FT6206_TOUCH_EVT_FLAG_LIFT_UP :
                    TS_State->touchEventId[index] = TOUCH_EVENT_LIFT_UP;
                    break;
                case FT6206_TOUCH_EVT_FLAG_CONTACT :
                    TS_State->touchEventId[index] = TOUCH_EVENT_CONTACT;
                    break;
                case FT6206_TOUCH_EVT_FLAG_NO_EVENT :
                    TS_State->touchEventId[index] = TOUCH_EVENT_NO_EVT;
                    break;
                default :
                    ts_status = TS_ERROR;
                    break;
            }
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */
        }

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
        ts_status = STAR_TS_Get_GestureId(TS_State);
#endif
    }

    return ts_status;
}

#ifdef ENABLE_STAR_TS_IRQ
// TODO: rename symbols
static volatile uint8_t newEvent = 0;
static TS_StateTypeDef tsState;
static uint8_t tsStatus;
void STAR_TS_IrqCallback(void)
{
    static uint8_t irqTsStatus;
    static TS_StateTypeDef irqTsState;

    // Always flush event fifo
    irqTsStatus = GetState(&irqTsState);

    // If last event has been processed
    // pass new event
    if (!newEvent) {
        tsStatus = irqTsStatus;
        memcpy(&tsState, &irqTsState, sizeof(TS_StateTypeDef));
        newEvent = 1;
    }
}

// TODO: move in common code
void TS_INT_IRQ_HANDLER(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(TS_INT_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(TS_INT_PIN);
        STAR_TS_IrqCallback();
    }
}

static uint8_t IrqConfig(void)
{
    GPIO_InitTypeDef gpio_init_structure;
    uint8_t ts_status = TS_OK;

    TS_INT_GPIO_CLK_ENABLE();

    gpio_init_structure.Pin = TS_INT_PIN;
    gpio_init_structure.Pull = GPIO_PULLUP;
    gpio_init_structure.Mode = GPIO_MODE_IT_FALLING;
    gpio_init_structure.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(TS_INT_GPIO_PORT, &gpio_init_structure);

    HAL_NVIC_SetPriority((IRQn_Type)(TS_INT_EXTI_IRQn), STAR_TS_IRQ_PRI,
            STAR_TS_IRQ_SUB);
    HAL_NVIC_EnableIRQ((IRQn_Type)(TS_INT_EXTI_IRQn));

    ts_driver->EnableIT(I2C_Address);

    return ts_status;
}
#endif // ENABLE_STAR_TS_IRQ

uint8_t STAR_TS_Init(uint16_t ts_SizeX, uint16_t ts_SizeY)
{
    uint8_t ts_status = TS_OK;

    ft6x06_ts_drv.Init(0);

    if (ft6x06_ts_drv.ReadID(TS_I2C_ADDRESS) == FT6206_ID_VALUE) {
        ts_driver = &ft6x06_ts_drv;

        I2C_Address = TS_I2C_ADDRESS;

        if (ts_SizeX < ts_SizeY)
            ts_orientation = TS_SWAP_NONE;
        else
            ts_orientation = TS_SWAP_XY | TS_SWAP_Y;

        if (ts_status == TS_OK) {
            ts_driver->Reset(I2C_Address);
            ts_driver->Start(I2C_Address);
#ifdef ENABLE_STAR_TS_IRQ
            IrqConfig();
#endif
        }
    }
    else
        ts_status = TS_DEVICE_NOT_FOUND;

    return ts_status;
}

uint8_t STAR_TS_GetState(TS_StateTypeDef *TS_State)
{
    uint8_t ts_status = TS_OK;

#ifdef ENABLE_STAR_TS_IRQ
    memcpy(TS_State, &tsState, sizeof(TS_StateTypeDef));
    ts_status = tsStatus;
    newEvent = 0;
#else
    ts_status = GetState(TS_State);
#endif

    return ts_status;
}

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
uint8_t STAR_TS_Get_GestureId(TS_StateTypeDef *TS_State)
{
    uint32_t gestureId = 0;
    uint8_t  ts_status = TS_OK;

    ft6x06_TS_GetGestureID(I2C_Address, &gestureId);

    switch (gestureId) {
        case FT6206_GEST_ID_NO_GESTURE :
            TS_State->gestureId = GEST_ID_NO_GESTURE;
            break;
        case FT6206_GEST_ID_MOVE_UP :
            TS_State->gestureId = GEST_ID_MOVE_UP;
            break;
        case FT6206_GEST_ID_MOVE_RIGHT :
            TS_State->gestureId = GEST_ID_MOVE_RIGHT;
            break;
        case FT6206_GEST_ID_MOVE_DOWN :
            TS_State->gestureId = GEST_ID_MOVE_DOWN;
            break;
        case FT6206_GEST_ID_MOVE_LEFT :
            TS_State->gestureId = GEST_ID_MOVE_LEFT;
            break;
        case FT6206_GEST_ID_ZOOM_IN :
            TS_State->gestureId = GEST_ID_ZOOM_IN;
            break;
        case FT6206_GEST_ID_ZOOM_OUT :
            TS_State->gestureId = GEST_ID_ZOOM_OUT;
            break;
        default :
            ts_status = TS_ERROR;
            break;
    }

    return ts_status;
}

uint8_t STAR_TS_ResetTouchData(TS_StateTypeDef *TS_State)
{
    uint8_t ts_status = TS_ERROR;
    uint32_t index;

    if (TS_State != (TS_StateTypeDef *)NULL) {
        TS_State->gestureId = GEST_ID_NO_GESTURE;
        TS_State->touchDetected = 0;

        for(index = 0; index < TS_MAX_NB_TOUCH; index++) {
            TS_State->touchX[index]       = 0;
            TS_State->touchY[index]       = 0;
            TS_State->touchArea[index]    = 0;
            TS_State->touchEventId[index] = TOUCH_EVENT_NO_EVT;
            TS_State->touchWeight[index]  = 0;
        }

        ts_status = TS_OK;
    }

    return ts_status;
}
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */
