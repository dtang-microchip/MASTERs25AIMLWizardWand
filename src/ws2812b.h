/* 
 * File:   ws2812b.h
 * Author: C07445
 *
 * Created on January 16, 2020, 3:24 PM
 */

#ifndef WS2812B_H
#define	WS2812B_H

#ifdef	__cplusplus
extern "C" {
#endif
/* This define can be altered to represent the number of LEDs*/    
#define WS2812_NUM 8

    
#define WS2812_DC_ZERO  16
#define WS2812_DC_ONE  32
#define WS2812_RESET_PAD 45

typedef struct {
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} WS2812_COLOR;

typedef union {
    WS2812_COLOR led[WS2812_NUM];
    uint8_t array[3 * WS2812_NUM];
} LED_TYPE;

typedef struct {
    uint8_t PWM[(8 * 3 * WS2812_NUM) + WS2812_RESET_PAD + 1];
    LED_TYPE color;
    uint8_t update : 1;
} WS2812_TYPE;

typedef struct {
    volatile uint16_t BTCTRL;
    volatile uint16_t BTCNT;
    volatile uint32_t SRCADDR;
    volatile uint32_t DSTADDR;
    volatile uint32_t DESCADDR;
} DMA_Descriptor_TypeLocal;

void ws2812_Update(void);
void initTimerDMA(void);

typedef struct {
    int32_t green;
    int32_t red;
    int32_t blue;
} WS2812_FADE;

typedef struct {
    WS2812_FADE old;
    WS2812_FADE new;
    int32_t time;
    int32_t count;
    int32_t index;
} TARGET_LED_FADE;

typedef struct {
    WS2812_FADE color;
    uint32_t time;
} FADE_TABLE;
#ifdef	__cplusplus
}
#endif

#endif	/* WS2812B_H */

