//-----------------------------------------------------------------------------
// Routines to load the FPGA image, and then to configure the FPGA's major
// mode once it is configured.
//
// Jonathan Westhues, April 2006
//-----------------------------------------------------------------------------
#include <proxmark3.h>
#include "apps.h"

//-----------------------------------------------------------------------------
// Set up the synchronous serial port, with the one set of options that we
// always use when we are talking to the FPGA. Both RX and TX are enabled.
//-----------------------------------------------------------------------------
void FpgaSetupSsc(void)
{
    // First configure the GPIOs, and get ourselves a clock.
    PIO_PERIPHERAL_A_SEL = (1 << GPIO_SSC_FRAME) |
                           (1 << GPIO_SSC_DIN)   |
                           (1 << GPIO_SSC_DOUT)  |
                           (1 << GPIO_SSC_CLK);
    PIO_DISABLE = (1 << GPIO_SSC_DOUT);

    PMC_PERIPHERAL_CLK_ENABLE = (1 << PERIPH_SSC);

    // Now set up the SSC proper, starting from a known state.
    SSC_CONTROL = SSC_CONTROL_RESET;

    // RX clock comes from TX clock, RX starts when TX starts, data changes
    // on RX clock rising edge, sampled on falling edge
    SSC_RECEIVE_CLOCK_MODE = SSC_CLOCK_MODE_SELECT(1) | SSC_CLOCK_MODE_START(1);

    // 8 bits per transfer, no loopback, MSB first, 1 transfer per sync
    // pulse, no output sync, start on positive-going edge of sync
    SSC_RECEIVE_FRAME_MODE = SSC_FRAME_MODE_BITS_IN_WORD(8) |
        SSC_FRAME_MODE_MSB_FIRST | SSC_FRAME_MODE_WORDS_PER_TRANSFER(0);
    
    // clock comes from TK pin, no clock output, outputs change on falling
    // edge of TK, start on rising edge of TF
    SSC_TRANSMIT_CLOCK_MODE = SSC_CLOCK_MODE_SELECT(2) |
        SSC_CLOCK_MODE_START(5);

    // tx framing is the same as the rx framing
    SSC_TRANSMIT_FRAME_MODE = SSC_RECEIVE_FRAME_MODE;

    SSC_CONTROL = SSC_CONTROL_RX_ENABLE | SSC_CONTROL_TX_ENABLE;
}

//-----------------------------------------------------------------------------
// Set up DMA to receive samples from the FPGA. We will use the PDC, with
// a single buffer as a circular buffer (so that we just chain back to
// ourselves, not to another buffer). The stuff to manipulate those buffers
// is in apps.h, because it should be inlined, for speed.
//-----------------------------------------------------------------------------
void FpgaSetupSscDma(BYTE *buf, int len)
{
    PDC_RX_POINTER(SSC_BASE) = (DWORD)buf;
    PDC_RX_COUNTER(SSC_BASE) = len;
    PDC_RX_NEXT_POINTER(SSC_BASE) = (DWORD)buf;
    PDC_RX_NEXT_COUNTER(SSC_BASE) = len;

    PDC_CONTROL(SSC_BASE) = PDC_RX_ENABLE;
}

//-----------------------------------------------------------------------------
// Download the FPGA image stored in flash (slave serial).
//-----------------------------------------------------------------------------
void FpgaDownloadAndGo(void)
{
    extern const DWORD FpgaImage[];
    extern const DWORD FpgaImageLen;
    
    int i, j;

    PIO_OUTPUT_ENABLE = (1 << GPIO_FPGA_ON);
    PIO_ENABLE = (1 << GPIO_FPGA_ON);
    PIO_OUTPUT_DATA_SET = (1 << GPIO_FPGA_ON);

    SpinDelay(50);

    LED_D_ON();

    HIGH(GPIO_FPGA_NPROGRAM);
    LOW(GPIO_FPGA_CCLK);
    LOW(GPIO_FPGA_DIN);
    PIO_OUTPUT_ENABLE = (1 << GPIO_FPGA_NPROGRAM)   |
                        (1 << GPIO_FPGA_CCLK)       |
                        (1 << GPIO_FPGA_DIN);
    SpinDelay(1);

    LOW(GPIO_FPGA_NPROGRAM);
    SpinDelay(50);
    HIGH(GPIO_FPGA_NPROGRAM);


    for(i = 0; i < FpgaImageLen; i++) {
        DWORD v = FpgaImage[i];
        for(j = 0; j < 32; j++) {
            if(v & 0x80000000) {
                HIGH(GPIO_FPGA_DIN);
            } else {
                LOW(GPIO_FPGA_DIN);
            }
            HIGH(GPIO_FPGA_CCLK);
            LOW(GPIO_FPGA_CCLK);
            v <<= 1;
        }
    }

    LED_D_OFF();
}

//-----------------------------------------------------------------------------
// Write the FPGA setup word (that determines what mode the logic is in, read
// vs. clone vs. etc.).
//-----------------------------------------------------------------------------
void FpgaWriteConfWord(BYTE v)
{
    int j;

    PIO_OUTPUT_ENABLE = (1 << GPIO_MOSI) | (1 << GPIO_SPCK) | (1 << GPIO_NCS);

    // Might as well just bit-bang, for only a few bits
    LOW(GPIO_MOSI);
    LOW(GPIO_SPCK);
    LOW(GPIO_NCS);

    for(j = 0; j < 8; j++) {
        if(v & 0x80) {
            HIGH(GPIO_MOSI);
        } else {
            LOW(GPIO_MOSI);
        }
        HIGH(GPIO_SPCK);
        LOW(GPIO_SPCK);
        v <<= 1;
    }

    HIGH(GPIO_NCS);
    HIGH(GPIO_SPCK);
    LOW(GPIO_SPCK);
}

//-----------------------------------------------------------------------------
// Set up the CMOS switches that mux the ADC: four switches, independently
// closable, but should only close one at a time. Not an FPGA thing, but
// the samples from the ADC always flow through the FPGA.
//-----------------------------------------------------------------------------
void SetAdcMuxFor(int whichGpio)
{
    PIO_OUTPUT_ENABLE = (1 << GPIO_MUXSEL_HIPKD) |
                        (1 << GPIO_MUXSEL_LOPKD) |
                        (1 << GPIO_MUXSEL_LORAW) |
                        (1 << GPIO_MUXSEL_HIRAW);

    PIO_ENABLE        = (1 << GPIO_MUXSEL_HIPKD) |
                        (1 << GPIO_MUXSEL_LOPKD) |
                        (1 << GPIO_MUXSEL_LORAW) |
                        (1 << GPIO_MUXSEL_HIRAW);

    LOW(GPIO_MUXSEL_HIPKD);
    LOW(GPIO_MUXSEL_HIRAW);
    LOW(GPIO_MUXSEL_LORAW);
    LOW(GPIO_MUXSEL_LOPKD);

    HIGH(whichGpio);
}
