//-----------------------------------------------------------------------------
// Definitions internal to the app source.
// Jonathan Westhues, Aug 2005
//-----------------------------------------------------------------------------

#ifndef __APPS_H
#define __APPS_H

/// appmain.c
void AppMain(void);
void DbpIntegers(int a, int b, int c);
void DbpString(char *str);
void SpinDelay(int ms);
void ToSendStuffBit(int b);
void ToSendReset(void);
extern int ToSendMax;
extern BYTE ToSend[];
extern DWORD BigBuf[];


/// fpga.c
void FpgaWriteConfWord(BYTE v);
void FpgaDownloadAndGo(void);
void FpgaSetupSsc(void);
void FpgaSetupSscDma(BYTE *buf, int len);
void SetAdcMuxFor(int whichGpio);

// Definitions for the FPGA configuration word.
#define FPGA_MAJOR_MODE_LF_READER           (0<<5)
#define FPGA_MAJOR_MODE_LF_SIMULATOR        (1<<5)
#define FPGA_MAJOR_MODE_HF_READER_TX        (2<<5)
#define FPGA_MAJOR_MODE_HF_READER_RX_XCORR  (3<<5)
#define FPGA_MAJOR_MODE_HF_SIMULATOR        (4<<5)
#define FPGA_MAJOR_MODE_OFF                 (7<<5)
// Options for the LF reader
#define FPGA_LF_READER_USE_125_KHZ          (1<<3)
// Options for the HF reader, tx to tag
#define FPGA_HF_READER_TX_SHALLOW_MOD       (1<<0)
// Options for the HF reader, correlating against rx from tag
#define FPGA_HF_READER_RX_XCORR_848_KHZ     (1<<0)
#define FPGA_HF_READER_RX_XCORR_SNOOP       (1<<1)
// Options for the HF simulated tag, how to modulate
#define FPGA_HF_SIMULATOR_NO_MODULATION     (0<<0)
#define FPGA_HF_SIMULATOR_MODULATE_BPSK     (1<<0)


/// iso14443.h
void SimulateIso14443Tag(void);
void AcquireRawAdcSamplesIso14443(DWORD parameter);
void SnoopIso14443(void);


/// iso15693.h
void AcquireRawAdcSamplesIso15693(void);


/// util.h
int strlen(char *str);
void *memcpy(void *dest, const void *src, int len);
void *memset(void *dest, int c, int len);
int memcmp(const void *av, const void *bv, int len);


#endif
