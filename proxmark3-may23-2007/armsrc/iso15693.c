//-----------------------------------------------------------------------------
// Routines to support ISO 15693. This includes both the reader software and
// the `fake tag' modes, but at the moment I've implemented only the reader
// stuff, and that barely.
// Jonathan Westhues, split Nov 2006
//-----------------------------------------------------------------------------
#include <proxmark3.h>
#include "apps.h"

//-----------------------------------------------------------------------------
// Map a sequence of octets (~layer 2 command) into the set of bits to feed
// to the FPGA, to transmit that command to the tag.
//-----------------------------------------------------------------------------
static void CodeIso15693AsReader(BYTE *cmd, int n)
{
    int i, j;

    ToSendReset();

    // Give it a bit of slack at the beginning
    for(i = 0; i < 24; i++) {
        ToSendStuffBit(1);
    }

    ToSendStuffBit(0);
    ToSendStuffBit(1);
    ToSendStuffBit(1);
    ToSendStuffBit(1);
    ToSendStuffBit(1);
    ToSendStuffBit(0);
    ToSendStuffBit(1);
    ToSendStuffBit(1);
    for(i = 0; i < n; i++) {
        for(j = 0; j < 8; j += 2) {
            int these = (cmd[i] >> j) & 3;
            switch(these) {
                case 0:
                    ToSendStuffBit(1);
                    ToSendStuffBit(0);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    break;
                case 1:
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(0);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    break;
                case 2:
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(0);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    break;
                case 3:
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(1);
                    ToSendStuffBit(0);
                    break;
            }
        }
    }
    ToSendStuffBit(1);
    ToSendStuffBit(1);
    ToSendStuffBit(0);
    ToSendStuffBit(1);

    // And slack at the end, too.
    for(i = 0; i < 24; i++) {
        ToSendStuffBit(1);
    }
}

//-----------------------------------------------------------------------------
// The CRC used by ISO 15693.
//-----------------------------------------------------------------------------
static WORD Crc(BYTE *v, int n)
{
    DWORD reg;
    int i, j;

    reg = 0xffff;
    for(i = 0; i < n; i++) {
        reg = reg ^ ((DWORD)v[i]);
        for (j = 0; j < 8; j++) {
            if (reg & 0x0001) {
                reg = (reg >> 1) ^ 0x8408;
            } else {
                reg = (reg >> 1);
            }
        }
    }

    return ~reg;
}

//-----------------------------------------------------------------------------
// Encode (into the ToSend buffers) an identify request, which is the first
// thing that you must send to a tag to get a response.
//-----------------------------------------------------------------------------
static void BuildIdentifyRequest(void)
{
    BYTE cmd[5];

    WORD crc;
    // one sub-carrier, inventory, 1 slot, fast rate
    cmd[0] = (1 << 2) | (1 << 5) | (1 << 1);
    // inventory command code
    cmd[1] = 0x01;
    // no mask
    cmd[2] = 0x00;
    crc = Crc(cmd, 3);
    cmd[3] = crc & 0xff;
    cmd[4] = crc >> 8;

    CodeIso15693AsReader(cmd, sizeof(cmd));
}

//-----------------------------------------------------------------------------
// Start to read an ISO 15693 tag. We send an identify request, then wait
// for the response. The response is not demodulated, just left in the buffer
// so that it can be downloaded to a PC and processed there.
//-----------------------------------------------------------------------------
void AcquireRawAdcSamplesIso15693(void)
{
    int c = 0;
    BYTE *dest = (BYTE *)BigBuf;
    int getNext = 0;

    SBYTE prev = 0;

    BuildIdentifyRequest();

    SetAdcMuxFor(GPIO_MUXSEL_HIPKD);

    // Give the tags time to energize
    FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);
    SpinDelay(100);

    // Now send the command
    FpgaSetupSsc();
    FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_TX);

    c = 0;
    for(;;) {
        if(SSC_STATUS & (SSC_STATUS_TX_READY)) {
            SSC_TRANSMIT_HOLDING = ToSend[c];
            c++;
            if(c == ToSendMax+3) {
                break;
            }
        }
        if(SSC_STATUS & (SSC_STATUS_RX_READY)) {
            volatile DWORD r = SSC_RECEIVE_HOLDING;
            (void)r;
        }
        WDT_HIT();
    }

    FpgaWriteConfWord(FPGA_MAJOR_MODE_HF_READER_RX_XCORR);

    c = 0;
    getNext = FALSE;
    for(;;) {
        if(SSC_STATUS & (SSC_STATUS_TX_READY)) {
            SSC_TRANSMIT_HOLDING = 0x43;
        }
        if(SSC_STATUS & (SSC_STATUS_RX_READY)) {
            SBYTE b;
            b = (SBYTE)SSC_RECEIVE_HOLDING;

            // The samples are correlations against I and Q versions of the
            // tone that the tag AM-modulates, so every other sample is I,
            // every other is Q. We just want power, so abs(I) + abs(Q) is
            // close to what we want.
            if(getNext) {
                SBYTE r;

                if(b < 0) {
                    r = -b;
                } else {
                    r = b;
                }
                if(prev < 0) {
                    r -= prev;
                } else {
                    r += prev;
                }

                dest[c++] = (BYTE)r;

                if(c >= 2000) {
                    break;
                }
            } else {
                prev = b;
            }

            getNext = !getNext;
        }
    }
}
