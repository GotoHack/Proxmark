
CC	  = arm-elf-gcc
AS	  = arm-elf-as
LD	  = arm-elf-ld
OBJCOPY = arm-elf-objcopy

OBJDIR  = obj

INCLUDE = -I../include

INCLUDES = ../include/proxmark3.h ../include/at91sam7s128.h ../include/config_gpio.h ../include/usb_cmd.h apps.h

CFLAGS  = -O6 -c $(INCLUDE) -Wall

OBJ =   $(OBJDIR)/start.o \
        $(OBJDIR)/appmain.o \
        $(OBJDIR)/fpga.o \
        $(OBJDIR)/fpgaimg.o \
        $(OBJDIR)/iso14443.o \
        $(OBJDIR)/iso15693.o \
        $(OBJDIR)/util.o

OBJCOMMON = \
        $(OBJDIR)/usb.o

all: osimage.s19

$(OBJDIR)/osimage.s19: $(OBJ) $(OBJCOMMON)
    @echo obj/osimage.s19
	@$(LD) -g -Tldscript -o $(OBJDIR)/osimage.elf $(OBJ) $(OBJCOMMON) C:/gnuarm/lib/gcc/arm-elf/3.4.1/interwork/libgcc.a
	@$(OBJCOPY) -Osrec --srec-forceS3 $(OBJDIR)/osimage.elf $(OBJDIR)/osimage.s19

osimage.s19: $(OBJDIR)/osimage.s19
    @echo osimage.s19
	@cat $(OBJDIR)/osimage.s19 > osimage.s19

$(OBJ): $(@B).c $(INCLUDES)
    @echo $(@B).c
	@$(CC) $(CFLAGS) -mthumb -mthumb-interwork $(@B).c -o $(OBJDIR)/$(@B).o

$(OBJCOMMON): ../common/$(@B).c $(INCLUDES)
    @echo $(@B).c
	@$(CC) $(CFLAGS) -mthumb -mthumb-interwork ../common/$(@B).c -o $(OBJDIR)/$(@B).o

clean:
	del /q obj\*.o
	del /q obj\*.elf
	del /q obj\*.s19
	del /q *.s19
