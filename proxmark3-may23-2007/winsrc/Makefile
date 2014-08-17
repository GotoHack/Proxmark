BASE_DEFS   = /D_WIN32_WINNT=0x501 /DISOLATION_AWARE_ENABLED /D_WIN32_IE=0x600 /DWIN32_LEAN_AND_MEAN /DWIN32 /D_MT
BASE_CFLAGS = /W3 /nologo -I..\common\win32

DEFINES  = $(BASE_DEFS)
CFLAGS   = $(BASE_CFLAGS) /Zi /MT

OBJDIR = obj

OBJS   = $(OBJDIR)\prox.obj \
         $(OBJDIR)\gui.obj \
         $(OBJDIR)\command.obj

LIBS   = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib setupapi.lib

HEADERS = prox.h

all: $(OBJDIR)/prox.exe
    cp $(OBJDIR)/prox.exe .
    copy prox.exe c:\bin
    prox gui

clean:
    rm -f obj/*

$(OBJDIR)/prox.exe: $(OBJS)
    $(CC) $(DEFINES) $(CFLAGS) -Fe$(OBJDIR)/prox.exe $(OBJS) $(LIBS)

$(OBJS): $(@B).cpp $(HEADERS)
    $(CC) $(CFLAGS) $(DEFINES) -c -Fo$(OBJDIR)/$(@B).obj $(@B).cpp
