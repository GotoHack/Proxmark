#include <proxmark3.h>

extern void CallRam(void);


static void ConfigClocks(void)
{
    volatile int i;

    // we are using a 16 MHz crystal as the basis for everything
    PMC_SYS_CLK_ENABLE = PMC_SYS_CLK_PROCESSOR_CLK | PMC_SYS_CLK_UDP_CLK;

    PMC_PERIPHERAL_CLK_ENABLE =
        (1<<PERIPH_PIOA) |
        (1<<PERIPH_ADC) |
        (1<<PERIPH_SPI) |
        (1<<PERIPH_SSC) |
        (1<<PERIPH_PWMC) |
        (1<<PERIPH_UDP);

    PMC_MAIN_OSCILLATOR = PMC_MAIN_OSCILLATOR_ENABLE |
        PMC_MAIN_OSCILLATOR_STARTUP_DELAY(0x40);

    // minimum PLL clock frequency is 80 MHz in range 00 (96 here so okay)
    PMC_PLL = PMC_PLL_DIVISOR(2) | PMC_PLL_COUNT_BEFORE_LOCK(0x40) |
        PMC_PLL_FREQUENCY_RANGE(0) | PMC_PLL_MULTIPLIER(12) |
        PMC_PLL_USB_DIVISOR(1);

    // let the PLL spin up, plenty of time
    for(i = 0; i < 1000; i++)
        ;

    PMC_MASTER_CLK = PMC_CLK_SELECTION_SLOW_CLOCK | PMC_CLK_PRESCALE_DIV_2;

    for(i = 0; i < 1000; i++)
        ;

    PMC_MASTER_CLK = PMC_CLK_SELECTION_PLL_CLOCK | PMC_CLK_PRESCALE_DIV_2;

    for(i = 0; i < 1000; i++)
        ;
}


void CMain(void)
{
    int i;

    volatile DWORD *src = (volatile DWORD *)0x200;
    volatile DWORD *dest = (volatile DWORD *)0x00200000;

    ConfigClocks();

    for(i = 0; i < 1024; i++) {
        *dest++ = *src++;
        WDT_HIT();
    }

    CallRam();
}
