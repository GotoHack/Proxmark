//-----------------------------------------------------------------------------
// The main application code. This is the first thing called after start.c
// executes.
// Jonathan Westhues, Mar 2006
//-----------------------------------------------------------------------------
#include <proxmark3.h>
#include "apps.h"

// The large multi-purpose buffer, typically used to hold A/D samples,
// maybe pre-processed in some way.
DWORD BigBuf[3072];

//=============================================================================
// A buffer where we can queue things up to be sent through the FPGA, for
// any purpose (fake tag, as reader, whatever). We go MSB first, since that
// is the order in which they go out on the wire.
//=============================================================================

BYTE ToSend[256];
int ToSendMax;
static int ToSendBit;

void ToSendReset(void)
{
    ToSendMax = -1;
    ToSendBit = 8;
}

void ToSendStuffBit(int b)
{
    if(ToSendBit >= 8) {
        ToSendMax++;
        ToSend[ToSendMax] = 0;
        ToSendBit = 0;
    }

    if(b) {
        ToSend[ToSendMax] |= (1 << (7 - ToSendBit));
    }

    ToSendBit++;

    if(ToSendBit >= sizeof(ToSend)) {
        ToSendBit = 0;
        DbpString("ToSendStuffBit overflowed!");
    }
}

//=============================================================================
// Debug print functions, to go out over USB, to the usual PC-side client.
//=============================================================================

void DbpString(char *str)
{
    UsbCommand c;
    c.cmd = CMD_DEBUG_PRINT_STRING;
    c.ext1 = strlen(str);
    memcpy(c.d.asBytes, str, c.ext1);

    UsbSendPacket((BYTE *)&c, sizeof(c));
    // TODO fix USB so stupid things like this aren't req'd
    SpinDelay(50);
}

void DbpIntegers(int x1, int x2, int x3)
{
    UsbCommand c;
    c.cmd = CMD_DEBUG_PRINT_INTEGERS;
    c.ext1 = x1;
    c.ext2 = x2;
    c.ext3 = x3;

    UsbSendPacket((BYTE *)&c, sizeof(c));
    // XXX
    SpinDelay(50);
}

void AcquireRawAdcSamples125k(BOOL at134khz)
{
    BYTE *dest = (BYTE *)BigBuf;
    int n = sizeof(BigBuf);
    int i;

    if(at134khz) {
        FpgaWriteConfWord(0x00);
    } else {
        FpgaWriteConfWord(0x08);
    }

    // Connect the A/D to the peak-detected low-frequency path.
    SetAdcMuxFor(GPIO_MUXSEL_LOPKD);

    // Give it a bit of time for the resonant antenna to settle. 
    SpinDelay(50);

    // Now set up the SSC to get the ADC samples that are now streaming at us.
    FpgaSetupSsc();

    i = 0;
    for(;;) {
        if(SSC_STATUS & (SSC_STATUS_TX_READY)) {
            SSC_TRANSMIT_HOLDING = 0x43;
            LED_D_ON();
        }
        if(SSC_STATUS & (SSC_STATUS_RX_READY)) {
            dest[i] = (BYTE)SSC_RECEIVE_HOLDING;
            i++;
            LED_D_OFF();
            if(i >= n) {
                break;
            }
        }
    }
    DbpIntegers(dest[0], dest[1], at134khz);
}

//-----------------------------------------------------------------------------
// Read an ADC channel and block till it completes, then return the result
// in ADC units (0 to 1023). Also a routine to average sixteen samples and
// return that.
//-----------------------------------------------------------------------------
static int ReadAdc(int ch)
{
    DWORD d;

    ADC_CONTROL = ADC_CONTROL_RESET;
    ADC_MODE = ADC_MODE_PRESCALE(32) | ADC_MODE_STARTUP_TIME(16) |
        ADC_MODE_SAMPLE_HOLD_TIME(8);
    ADC_CHANNEL_ENABLE = ADC_CHANNEL(ch);

    ADC_CONTROL = ADC_CONTROL_START;
    while(!(ADC_STATUS & ADC_END_OF_CONVERSION(ch)))
        ;
    d = ADC_CHANNEL_DATA(ch);

    return d;
}
static int AvgAdc(int ch)
{
    int i;
    int a = 0;

    for(i = 0; i < 32; i++) {
        a += ReadAdc(ch);
    }

    return (a + 15) >> 5;
}


void MeasureAntennaTuning(void)
{
// Impedances are Zc = 1/(j*omega*C), in ohms
#define LF_TUNING_CAP_Z     1273        // 1 nF @ 125 kHz
#define HF_TUNING_CAP_Z     90          // 130 pF @ 13.56 MHz

    int vLf125, vLf134, vHf;   // in mV

    UsbCommand c;

    // Let the FPGA drive the low-frequency antenna around 125 kHz.
    FpgaWriteConfWord(0x08);
    SpinDelay(20);
    vLf125 = AvgAdc(4);
    // Vref = 3.3 V, and a 41:1 voltage divider on the input
    vLf125 = (3300 * vLf125) >> 10;
    vLf125 = vLf125 * 41;

    // Let the FPGA drive the low-frequency antenna around 134 kHz.
    FpgaWriteConfWord(0x00);
    SpinDelay(20);
    vLf134 = AvgAdc(4);
    // Vref = 3.3 V, and a 41:1 voltage divider on the input
    vLf134 = (3300 * vLf134) >> 10;
    vLf134 = vLf134 * 41;

    // Let the FPGA drive the high-frequency antenna around 13.56 MHz.
    FpgaWriteConfWord(0x60);
    SpinDelay(20);
    vHf = AvgAdc(5);
    // Vref = 3.3 V, and an 11:1 voltage divider on the input
    vHf = (3300 * vHf) >> 10;
    vHf = vHf * 11;

    c.cmd = CMD_MEASURED_ANTENNA_TUNING;
    c.ext1 = (vLf125 << 0) | (vLf134 << 16);
    c.ext2 = vHf;
    c.ext3 = (LF_TUNING_CAP_Z << 0) | (HF_TUNING_CAP_Z << 16);
    UsbSendPacket((BYTE *)&c, sizeof(c));
}

void SimulateTagLowFrequency(int period)
{
    int i;
    BYTE *tab = (BYTE *)BigBuf;

    FpgaWriteConfWord(0x20);

    PIO_ENABLE = (1 << GPIO_SSC_DOUT) |
                 (1 << GPIO_SSC_CLK);

    PIO_OUTPUT_ENABLE = (1 << GPIO_SSC_DOUT);
    PIO_OUTPUT_DISABLE = (1 << GPIO_SSC_CLK);

#define SHORT_COIL()    LOW(GPIO_SSC_DOUT)
#define OPEN_COIL()     HIGH(GPIO_SSC_DOUT)

    i = 0;
    for(;;) {
        while(!(PIO_PIN_DATA_STATUS & (1<<GPIO_SSC_CLK))) {
            if(!(PIO_PIN_DATA_STATUS & (1<<GPIO_BUTTON))) {
                return;
            }
            WDT_HIT();
        }

        LED_D_ON();
        if(tab[i]) {
            OPEN_COIL();
        } else {
            SHORT_COIL();
        }
        LED_D_OFF();

        while(PIO_PIN_DATA_STATUS & (1<<GPIO_SSC_CLK)) {
            if(!(PIO_PIN_DATA_STATUS & (1<<GPIO_BUTTON))) {
                return;
            }
            WDT_HIT();
        }

        i++;
        if(i == period) i = 0;
    }
}

void SimulateTagHfListen(void)
{
    BYTE *dest = (BYTE *)BigBuf;
    int n = sizeof(BigBuf);
    BYTE v = 0;
    int i;
    int p = 0;

    // We're using this mode just so that I can test it out; the simulated
    // tag mode would work just as well and be simpler.
    FpgaWriteConfWord(
        FPGA_MAJOR_MODE_HF_READER_RX_XCORR | FPGA_HF_READER_RX_XCORR_848_KHZ |
        FPGA_HF_READER_RX_XCORR_SNOOP);

    // We need to listen to the high-frequency, peak-detected path.
    SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

    FpgaSetupSsc();

    i = 0;
    for(;;) {
        if(SSC_STATUS & (SSC_STATUS_TX_READY)) {
            SSC_TRANSMIT_HOLDING = 0xff;
        }
        if(SSC_STATUS & (SSC_STATUS_RX_READY)) {
            BYTE r = (BYTE)SSC_RECEIVE_HOLDING;

            v <<= 1;
            if(r & 1) {
                v |= 1;
            }
            p++;

            if(p >= 8) {
                dest[i] = v;
                v = 0;
                p = 0;
                i++;

                if(i >= n) {
                    break;
                }
            }
        }
    }
    DbpString("simulate tag (now type bitsamples)");
}

void UsbPacketReceived(BYTE *packet, int len)
{
    UsbCommand *c = (UsbCommand *)packet;

    switch(c->cmd) {
        case CMD_ACQUIRE_RAW_ADC_SAMPLES_125K:
            AcquireRawAdcSamples125k(c->ext1);
            break;

        case CMD_ACQUIRE_RAW_ADC_SAMPLES_ISO_15693:
            AcquireRawAdcSamplesIso15693();
            break;

        case CMD_ACQUIRE_RAW_ADC_SAMPLES_ISO_14443:
            AcquireRawAdcSamplesIso14443(c->ext1);
            break;

        case CMD_SNOOP_ISO_14443:
            SnoopIso14443();
            break;

        case CMD_SIMULATE_TAG_HF_LISTEN:
            SimulateTagHfListen();
            break;

        case CMD_SIMULATE_TAG_ISO_14443:
            SimulateIso14443Tag();
            break;

        case CMD_MEASURE_ANTENNA_TUNING:
            MeasureAntennaTuning();
            break;

        case CMD_DOWNLOAD_RAW_ADC_SAMPLES_125K:
        case CMD_DOWNLOAD_RAW_BITS_TI_TYPE: {
            UsbCommand n;
            if(c->cmd == CMD_DOWNLOAD_RAW_ADC_SAMPLES_125K) {
                n.cmd = CMD_DOWNLOADED_RAW_ADC_SAMPLES_125K;
            } else {
                n.cmd = CMD_DOWNLOADED_RAW_BITS_TI_TYPE;
            }
            n.ext1 = c->ext1;
            memcpy(n.d.asDwords, BigBuf+c->ext1, 12*sizeof(DWORD));
            UsbSendPacket((BYTE *)&n, sizeof(n));
            break;
        }
        case CMD_DOWNLOADED_SIM_SAMPLES_125K: {
            BYTE *b = (BYTE *)BigBuf;
            memcpy(b+c->ext1, c->d.asBytes, 48);
            break;
        }
        case CMD_SIMULATE_TAG_125K:
            LED_A_ON();
            SimulateTagLowFrequency(c->ext1);
            LED_A_OFF();
            break;

        case CMD_SETUP_WRITE:
        case CMD_FINISH_WRITE:
            USB_D_PLUS_PULLUP_OFF();
            SpinDelay(1000);
            SpinDelay(1000);
            RSTC_CONTROL = RST_CONTROL_KEY | RST_CONTROL_PROCESSOR_RESET;
            for(;;) {
                // We're going to reset, and the bootrom will take control.
            }
            break;

        default:
            DbpString("unknown command");
            break;
    }
}

void AppMain(void)
{   
    SpinDelay(100);

    UsbStart();

    // The FPGA gets its clock from us, from PCK0, so set that up. We supply
    // a 48 MHz clock, so divide the 96 MHz PLL clock by two.
    PIO_PERIPHERAL_B_SEL = (1 << GPIO_PCK0);
    PIO_DISABLE = (1 << GPIO_PCK0);
    PMC_SYS_CLK_ENABLE = PMC_SYS_CLK_PROGRAMMABLE_CLK_0;
    PMC_PROGRAMMABLE_CLK_0 = PMC_CLK_SELECTION_PLL_CLOCK |
        PMC_CLK_PRESCALE_DIV_4;
    PIO_OUTPUT_ENABLE = (1 << GPIO_PCK0);

    // Load the FPGA image, which we have stored in our flash.
    FpgaDownloadAndGo();

    for(;;) {
        UsbPoll(FALSE);
        WDT_HIT();
    }
}

void SpinDelay(int ms)
{
    int ticks = (48000*ms) >> 10;

    // Borrow a PWM unit for my real-time clock
    PWM_ENABLE = PWM_CHANNEL(0);
    // 48 MHz / 1024 gives 46.875 kHz
    PWM_CH_MODE(0) = PWM_CH_MODE_PRESCALER(10);
    PWM_CH_DUTY_CYCLE(0) = 0;
    PWM_CH_PERIOD(0) = 0xffff;

    WORD start = (WORD)PWM_CH_COUNTER(0);

    for(;;) {
        WORD now = (WORD)PWM_CH_COUNTER(0);
        if(now == (WORD)(start + ticks)) {
            return;
        }
        WDT_HIT();
    }
}
