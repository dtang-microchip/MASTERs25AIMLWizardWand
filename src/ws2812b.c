// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include "definitions.h"                // SYS function prototypes
#include <stdio.h>
#include <xc.h>
#include "ws2812b.h"


DMA_Descriptor_TypeLocal DMA_Descriptor_WB = {
    (volatile uint16_t) 0,
    (volatile uint16_t) 0,
    (volatile uint32_t) 0,
    (volatile uint32_t) 0,
    (volatile uint32_t) 0
}
SECTION_DMAC_DESCRIPTOR;

__attribute__((__aligned__(16)))
DMA_Descriptor_TypeLocal DMA_Descriptor = {
    (volatile uint16_t) 0,
    (volatile uint16_t) 0,
    (volatile uint32_t) 0,
    (volatile uint32_t) 0,
    (volatile uint32_t) 0
}
SECTION_DMAC_DESCRIPTOR;

WS2812_TYPE ws2812;

//SECTION_DMAC_DESCRIPTOR;

void ws2812_ConstructTable(void) {
    uint8_t i, j;
    memset(&ws2812.PWM[3 * WS2812_NUM], 0, WS2812_RESET_PAD);
    for (i = 0; i < WS2812_NUM * 3; i++) {
        for (j = 0; j < 8; j++) {
            if ((ws2812.color.array[i] & (0x80 >> j)) == 0) {
                ws2812.PWM[(i * 8) + j] = WS2812_DC_ZERO;
            } else {
                ws2812.PWM[(i * 8) + j] = WS2812_DC_ONE;
            }
        }
    }
}

void initTimerDMA(void) {
    DMAC_REGS->DMAC_CTRL &= !DMAC_CTRL_DMAENABLE_Msk;
    DMAC_REGS->DMAC_CTRL &= !DMAC_CTRL_CRCENABLE_Msk;

    DMA_Descriptor.BTCTRL = (volatile uint16_t) DMAC_BTCTRL_STEPSIZE(0) | DMAC_BTCTRL_STEPSEL_SRC | DMAC_BTCTRL_DSTINC(0) | DMAC_BTCTRL_SRCINC(1) | DMAC_BTCTRL_BEATSIZE(0) | DMAC_BTCTRL_BLOCKACT(0) | DMAC_BTCTRL_EVOSEL(1) | DMAC_BTCTRL_VALID(1);
    DMA_Descriptor.BTCNT = (volatile uint16_t) sizeof (ws2812.PWM) - 1;
    DMA_Descriptor.SRCADDR = ((volatile uint32_t) & ws2812.PWM[1]) + DMA_Descriptor.BTCNT;
    DMA_Descriptor.DSTADDR = (volatile uint32_t) & TCC0_REGS->TCC_CCB[0];
    DMA_Descriptor.DESCADDR = (volatile uint32_t) 0;

    DMAC_REGS->DMAC_BASEADDR = (volatile uint32_t) & DMA_Descriptor;
    DMAC_REGS->DMAC_WRBADDR = (volatile uint32_t) & DMA_Descriptor_WB;
    DMAC_REGS->DMAC_CHID = 0;
    DMAC_REGS->DMAC_CTRL = DMAC_CTRL_LVLEN0_Msk;
    DMAC_REGS->DMAC_CHCTRLB = DMAC_CHCTRLB_TRIGACT_BEAT | DMAC_CHCTRLB_TRIGSRC(0x0D) | DMAC_CHCTRLB_LVL(0) | DMAC_CHCTRLB_CMD_RESUME;
    DMAC_REGS->DMAC_CHCTRLA |= DMAC_CHCTRLA_ENABLE_Msk;
    DMAC_REGS->DMAC_CTRL |= DMAC_CTRL_DMAENABLE_Msk;
    TCC0_REGS->TCC_CC[0] = 0;
    TCC0_PWMStart();
}

void ws2812_Update(void) {
    if (((DMAC_REGS->DMAC_CHINTFLAG & DMAC_CHINTFLAG_TCMPL_Msk) != 0) && ws2812.update == 1) {
        TCC0_PWMStop();
        TCC0_PWM24bitCounterSet(0);
        DMAC_REGS->DMAC_CHID = 0;
        DMAC_REGS->DMAC_CTRL &= !DMAC_CTRL_DMAENABLE_Msk;
        ws2812.update = 0;
        ws2812_ConstructTable();
        DMA_Descriptor.BTCTRL = (volatile uint16_t) DMAC_BTCTRL_STEPSIZE(0) | DMAC_BTCTRL_STEPSEL_SRC | DMAC_BTCTRL_DSTINC(0) | DMAC_BTCTRL_SRCINC(1) | DMAC_BTCTRL_BEATSIZE(0) | DMAC_BTCTRL_BLOCKACT(0) | DMAC_BTCTRL_EVOSEL(1) | DMAC_BTCTRL_VALID(1);
        DMA_Descriptor.BTCNT = (volatile uint16_t) sizeof (ws2812.PWM) - 1;
        DMA_Descriptor.SRCADDR = ((volatile uint32_t) & ws2812.PWM[1]) + DMA_Descriptor.BTCNT;
        DMA_Descriptor.DSTADDR = (volatile uint32_t) & TCC0_REGS->TCC_CCB[0];
        DMA_Descriptor.DESCADDR = (volatile uint32_t) 0;
        DMAC_REGS->DMAC_BASEADDR = (volatile uint32_t) & DMA_Descriptor;
        DMAC_REGS->DMAC_WRBADDR = (volatile uint32_t) & DMA_Descriptor_WB;
        DMAC_REGS->DMAC_CTRL = DMAC_CTRL_LVLEN0_Msk;
        DMAC_REGS->DMAC_CHCTRLB = DMAC_CHCTRLB_TRIGACT_BEAT | DMAC_CHCTRLB_TRIGSRC(0x0D) | DMAC_CHCTRLB_LVL(0) | DMAC_CHCTRLB_CMD_RESUME;
        TCC0_REGS->TCC_CC[0] = 0;
        while (TCC0_REGS->TCC_SYNCBUSY & (TCC_SYNCBUSY_CC0_Msk));
        TCC0_REGS->TCC_CCB[0] = ws2812.PWM[0];
        while (TCC0_REGS->TCC_SYNCBUSY & (TCC_SYNCBUSY_CCB0_Msk));
        TCC0_PWMStart();
        DMAC_REGS->DMAC_CHCTRLA |= DMAC_CHCTRLA_ENABLE_Msk;
        DMAC_REGS->DMAC_CTRL |= DMAC_CTRL_DMAENABLE_Msk;
    }
}

