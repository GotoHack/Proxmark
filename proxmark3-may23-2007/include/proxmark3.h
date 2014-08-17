//-----------------------------------------------------------------------------
// Definitions of interest to most of the software for this project.
// Jonathan Westhues, Mar 2006
//-----------------------------------------------------------------------------

#ifndef __PROXMARK3_H
#define __PROXMARK3_H

// Might as well have the hardware-specific defines everywhere.
#include <at91sam7s128.h>

#include <config_gpio.h>
#define LOW(x)      PIO_OUTPUT_DATA_CLEAR = (1 << (x))
#define HIGH(x)     PIO_OUTPUT_DATA_SET = (1 << (x))

typedef unsigned long DWORD;
typedef signed long SDWORD;
typedef int BOOL;
typedef unsigned char BYTE;
typedef signed char SBYTE;
typedef unsigned short WORD;
typedef signed short SWORD;
#define TRUE 1
#define FALSE 0

#include <usb_cmd.h>

#define PACKED __attribute__((__packed__))

#define USB_D_PLUS_PULLUP_ON() { \
        PIO_OUTPUT_DATA_SET = (1<<GPIO_USB_PU); \
        PIO_OUTPUT_ENABLE = (1<<GPIO_USB_PU); \
    }
#define USB_D_PLUS_PULLUP_OFF() PIO_OUTPUT_DISABLE = (1<<GPIO_USB_PU)

#define LED_A_ON()          PIO_OUTPUT_DATA_SET = (1<<GPIO_LED_A)
#define LED_A_OFF()         PIO_OUTPUT_DATA_CLEAR = (1<<GPIO_LED_A)
#define LED_B_ON()          PIO_OUTPUT_DATA_SET = (1<<GPIO_LED_B)
#define LED_B_OFF()         PIO_OUTPUT_DATA_CLEAR = (1<<GPIO_LED_B)
#define LED_C_ON()          PIO_OUTPUT_DATA_SET = (1<<GPIO_LED_C)
#define LED_C_OFF()         PIO_OUTPUT_DATA_CLEAR = (1<<GPIO_LED_C)
#define LED_D_ON()          PIO_OUTPUT_DATA_SET = (1<<GPIO_LED_D)
#define LED_D_OFF()         PIO_OUTPUT_DATA_CLEAR = (1<<GPIO_LED_D)

//--------------------------------
// USB declarations

void UsbSendPacket(BYTE *packet, int len);
BOOL UsbPoll(BOOL blinkLeds);
void UsbStart(void);

// This function is provided by the apps/bootrom, and called from UsbPoll
// if data are available.
void UsbPacketReceived(BYTE *packet, int len);


#endif