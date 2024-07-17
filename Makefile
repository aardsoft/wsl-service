#!include <win32.mak>

MC=mc
RC=rc
DLLTOOL=dlltool
Q=@
#LDFLAGS=-lwslapi.a
#LDFLAGS=-lpopt.dll
CFLAGS=-DUNICODE -D_UNICODE
MINGW_DLLS=libintl-8.dll
POPT_DLL=popt/build/src/libpopt.dll
INCLUDES=-Ipopt/src/

all: wsl-templates.h wsl-service-events.dll wsl-tool.exe wsl-service.exe wsl-service.msi

CORE_OBJECTS=wsl-launcher.o wsl-log.o wslapi.a

.SUFFIXES: .mc

$(POPT_DLL):
	$(Q)echo "> popt"
	$(Q)cmake --toolchain ../toolchain.cmake -B build/popt -S popt && cmake --build build/popt

wsl-templates.h: start-service-template.sh stop-service-template.sh
	$(Q)echo "> $@"
	$(Q)./make-template-header.sh

%.a: %.def
	$(Q)echo "DLLTOOL $@"
	$(Q)$(DLLTOOL) -d $< -l $@

.c.obj:
	$(Q)echo "CC $@"
	$(CC) $(CFLAGS) $(CVARS) $(INCLUDES) $*.c

.rc.ro:
	$(Q)echo "RC $@"
	$(Q)$(RC) $*.rc

%.h: %.rc
	$(Q)echo "RC $@"
	$(Q)$(RC) $< $@

.mc.rc:
	$(Q)echo "MC $@"
	$(Q)$(MC) $*.mc

wsl-service-events.dll: wsl-service-events.ro
	$(Q)$(CC) -o $@ -shared $<

wsl-tool.exe: $(POPT_DLL) wsl-tool.o $(CORE_OBJECTS)
	$(Q)echo "LD $@"
	$(Q)$(CC) $(INCLUDES) -o $@ $** $(LIBS) $(LDFLAGS)

wsl-service.exe: wsl-service.o $(CORE_OBJECTS)
	$(Q)echo "LD $@"
	$(Q)$(CC) $(LDFLAGS) $(INCLUDES) -o $@ $** $(LIBS)

clean:
	$(Q)echo "Cleaning up"
	$(Q)rm -Rf *.bin *.exe *.rc *.dll *.o *.ro *.a *.msi popt/build
