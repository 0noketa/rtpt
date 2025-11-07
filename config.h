/* 
 * File:   config.h
 *
 * Created on 2021/05/19, 20:48
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
    
#include <xc.h>

#define IS_18F452_family ( \
        (_18F452) || (_18LF452) \
        || (_18F442) || (_18LF442) \
        || (_18F252) || (_18LF252) \
        || (_18F242) || (_18LF242) \
    )
#define IS_18F4520_family ( \
        (_18F4520) || (_18LF4520) \
        || (_18F4420) || (_18LF4420) \
        || (_18F2520) || (_18LF2520) \
        || (_18F2420) || (_18LF2420) \
    )
#define IS_18F45J10_family ( \
        (_18F44J10) || (_18LF44J10) \
        || (_18F45J10) || (_18LF45J10) \
        || (_18F24J10) || (_18LF24J10) \
        || (_18F25J10) || (_18LF25J10) \
    )
#define IS_18F46J50_family ( \
        (_18F44J50) || (_18LF44J50) \
        || (_18F45J50) || (_18LF45J50) \
        || (_18F46J50) || (_18LF46J50) \
        || (_18F24J50) || (_18LF24J50) \
        || (_18F25J50) || (_18LF25J50) \
        || (_18F26J50) || (_18LF26J50) \
    )
#define IS_18F14K50_family ( \
        (_18F14K50) || (_18LF14K50) \
        || (_18F13K50) || (_18LF13K50) \
    )
#define IS_18F47Q10_family ( \
        (_18F45Q10) \
        || (_18F46Q10) \
        || (_18F47Q10) \
        || (_18F24Q10) \
        || (_18F25Q10) \
        || (_18F26Q10) \
        || (_18F27Q10) \
    )
#define IS_16F785_family ( \
        (_16F785) \
    )
#define IS_12F675_family ( \
        (_12F675) \
        || (_12F629) \
    )
#define IS_12F1840_family ( \
        (_12F1822) || (_12LF1822) \
        || (_12F1840) || (_12LF1840) \
        || (_16F1823) || (_16LF1823) \
        || (_16F1824) || (_16LF1824) \
        || (_16F1825) || (_16LF1825) \
        || (_16F1826) || (_16LF1826) \
        || (_16F1827) || (_16LF1827) \
        || (_16F1828) || (_16LF1828) \
        || (_16F1829) || (_16LF1829) \
    )
#define IS_12F1572_family ( \
        (_12F1571) || (_12LF1571) \
        || (_12F1572) || (_12LF1572) \
    )

// no serial
#define IS_12F322_family ( \
        (_12F320) || (_12LF320) \
        || (_12F322) || (_12LF322) \
    )
#define IS_16F505_family ( \
        (_16F505) || (_12F507) || (_12F508) \
    )

#define IS_18F16Q40_family ( \
    (_18F14Q40) \
    || (_18F15Q40) \
    || (_18F16Q40) \
    || (_18F04Q40) \
    || (_18F05Q40) \
    || (_18F06Q40) \
)



#if IS_18F4520_family
//// external
// 40MHz (OSC = ECIO6)
//#define _XTAL_FREQ 40000000
// 12MHz (OSC = ECIO6)
//#define _XTAL_FREQ 12000000
//// internal
// 8MHz (OSC = INTIO67)
#define _XTAL_FREQ 8000000
// 1MHz (OSC = INTIO67)
//#define _XTAL_FREQ 1000000
// 250kHz (OSC = INTIO67)
//#define _XTAL_FREQ 250000
// 125kHz (OSC = INTIO67)
//#define _XTAL_FREQ 125000
// 31kHz (OSC = INTIO67)
//#define _XTAL_FREQ 31000
#elif IS_18F46J50_family
// 8MHz (OSC = INTIO67)
#define _XTAL_FREQ 8000000
#elif IS_12F675_family
//// external
// 20MHz (OSC = EC)
//#define _XTAL_FREQ 20000000
// 12MHz (OSC = EC)
//#define _XTAL_FREQ 12000000
//// internal
// 4MHz (OSC = INTOSC)
#define _XTAL_FREQ 4000000
#elif defined(IS_16F505_family)
//// external (16F505 only)
// 20MHz (OSC = EC)
//#define _XTAL_FREQ 20000000
// 12MHz (OSC = EC)
//#define _XTAL_FREQ 12000000
//// internal
// 4MHz (OSC = INTOSC)
#define _XTAL_FREQ 4000000
#endif

#if IS_18F4520_family || IS_18F46J50_family
#define HZ_TO_IRCF(hz) ( \
      hz >= 8000000 ? 7 \
    : hz >= 4000000 ? 6 \
    : hz >= 2000000 ? 5 \
    : hz >= 1000000 ? 4 \
    : hz >=  500000 ? 3 \
    : hz >=  250000 ? 2 \
    : hz >=  125000 ? 1 \
    : hz >=   31000 ? 0 \
    : 0)
#endif
#if IS_12F1840_family || IS_12F1572_family
// HF
#define HZ_TO_IRCF(hz) ( \
      hz >=16000000 ? 15 \
    : hz >= 8000000 ? 14 \
    : hz >= 4000000 ? 13 \
    : hz >= 2000000 ? 12 \
    : hz >= 1000000 ? 11 \
    : hz >=  500000 ? 10 \
    : hz >=  250000 ? 9 \
    : hz >=  125000 ? 8 \
    : hz >=   31250 ? 2 \
    : 2)
// MF
//#define HZ_TO_IRCF(hz) ( \
//      hz >=  500000 ? 7 \
//    : hz >=  250000 ? 6 \
//    : hz >=  125000 ? 5 \
//    : hz >=   62500 ? 4 \
//    : hz >=   31250 ? 3 \
//    : 3)
// LF
//#define HZ_TO_IRCF(hz) ( \
//    : hz >=   31000 ? 1 \
//    : 0)
#endif

#if IS_12F322_family
#define HZ_TO_IRCF(hz) ( \
      hz >=16000000 ? 7 \
    : hz >= 8000000 ? 6 \
    : hz >= 4000000 ? 5 \
    : hz >= 2000000 ? 4 \
    : hz >= 1000000 ? 3 \
    : hz >=  500000 ? 2 \
    : hz >=  250000 ? 1 \
    : hz >=   31000 ? 0 \
    : 0)
#endif

#define KHZ_TO_IRCF(khz) HZ_TO_IRCF(khz ## 000)
#define MHZ_TO_IRCF(mhz) KHZ_TO_IRCF(mhz ## 000)

#if IS_18F4520_family
#define delay_ms(x) do { \
    uint8_t ircf0 = OSCCONbits.IRCF; \
    OSCCONbits.IRCF = KHZ_TO_IRCF(31000); \
    TMR0ON = 0; \
    T0CS = 0; \
    T08BIT = 0; \
    T0PS = 4; \
    TMR0IE = 1; \
    uint16_t n = (unsigned long)(x) * (31000/32000.0); \
    TMR0L = n & 0xFF; \
    TMR0H = n >> 8; \
    TMR0IE = 1; \
    TMR0ON = 1; \
    SLEEP(); \
    TMR0ON = 0; \
    OSCCONbits.IRCF = ircf0; \
} while(0)
#else
#define delay_ms(x) do { \
    uint8_t ircf0 = OSCCONbits.IRCF; \
    OSCCONbits.IRCF = KHZ_TO_IRCF(31000); \
    _delay((unsigned long)((x) * (31000/4000.0))); \
    OSCCONbits.IRCF = ircf0; \
} while(0)
#endif
#define delay_x_s(x) do { \
    uint8_t ircf0 = OSCCONbits.IRCF; \
    OSCCONbits.IRCF = KHZ_TO_IRCF(31000); \
    for (int i = x; i--;) _delay((unsigned long)(1000 * (31000/4000.0))); \
    OSCCONbits.IRCF = ircf0; \
} while(0)
#define delay_x_10s(x) do { \
    uint8_t ircf0 = OSCCONbits.IRCF; \
    OSCCONbits.IRCF = KHZ_TO_IRCF(31000); \
    for (int i = x; i--;) _delay((unsigned long)(10000 * (31000/4000.0))); \
    OSCCONbits.IRCF = ircf0; \
} while(0)


    
#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

