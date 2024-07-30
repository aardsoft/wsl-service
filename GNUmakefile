ifeq ($(toolchain), msvc)
CROS=
CC=cl
MC=mc
RC=rc /nologo
DLLTOOL=
else
CROSS=x86_64-w64-mingw32-
CC=gcc
MC=windmc
RC=windres
DLLTOOL=dlltool
endif
Q=@
#LDFLAGS=-lwslapi.a
#LDFLAGS=-lpopt.dll
CFLAGS=-DUNICODE -D_UNICODE
WIXL=wixl
WIXLFLAGS=-a x64
LIBINTL_DLL=gettext/x86_64/usr/bin/libintl-8.dll
POPT_DLL=build/popt/src/libpopt.dll
INCLUDES=-Ipopt/src/ -Ibuild
OUT=build

$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))

all: $(OUT)/wsl-templates.h $(OUT)/wsl-service-events.dll $(OUT)/wsl-tool.exe $(OUT)/wsl-service.exe $(OUT)/wsl-service.msi $(OUT)/doc/html/index.html

CORE_OBJECTS=$(OUT)/wsl-launcher.o $(OUT)/wsl-log.o $(OUT)/wslapi.a
ALL_HEADERS=wsl-launcher.h wsl-log.h wslapi.h
ALL_SOURCES=wsl-service.c wsl-tool.c wsl-launcher.c wsl-log.c

$(POPT_DLL):
	$(Q)echo "> popt"
	$(Q)rm -Rf build/popt
	$(Q)cmake --toolchain ../toolchain.cmake -B build/popt -S popt && cmake --build build/popt

copy-mingw-dlls:
	$(Q)for f in $(LIBINTL_DLL); do cp $$f build/; done
	$(Q)for f in $(POPT_DLL); do cp $$f build/; done

$(OUT)/wsl-templates.h: start-service-template.sh stop-service-template.sh
	$(Q)echo "> $@"
	$(Q)./make-template-header.sh

$(OUT)/doc/html/index.html: $(ALL_HEADERS) $(ALL_SOURCES)
	$(Q)doxygen doc/Doxyfile

$(OUT)/%.a: %.def
	$(Q)echo "DLLTOOL $@"
	$(Q)$(CROSS)$(DLLTOOL) -d $< -l $@

$(OUT)/%.o: %.c
	$(Q)echo "CC $@"
	$(Q)$(CROSS)$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUT)/%.ro: $(OUT)/%.rc
	$(Q)echo "RC $@"
ifeq ($(toolchain), msvc)
	$(Q)$(CROSS)$(RC) $(subst /,\,$<)
#	$(Q)$(CROSS)$(CVTRES) $<
else
	$(Q)$(CROSS)$(RC) $< $@
endif

#$(OUT)/%.h: $(OUT)/%.rc
# 	$(Q)echo "RC $@"
# 	$(Q)$(CROSS)$(RC) $< $@

$(OUT)/%.rc: %.mc
	$(Q)echo "MC $@"
	$(Q)$(CROSS)$(MC) -r $(OUT) -h $(OUT) $<

$(OUT)/wsl-service-events.dll: $(OUT)/wsl-service-events.ro
	$(Q)$(CROSS)$(CC) -o $@ -shared $<

$(OUT)/wsl-tool.exe: $(POPT_DLL) $(OUT)/wsl-tool.o $(CORE_OBJECTS)
	$(Q)echo "LD $@"
	$(Q)$(CROSS)$(CC) $(INCLUDES) -o $@ $^ $(LIBS) $(LDFLAGS)

$(OUT)/wsl-service.exe: $(OUT)/wsl-service.o $(CORE_OBJECTS)
	$(Q)echo "LD $@"
	$(Q)$(CROSS)$(CC) $(LDFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

clean:
	$(Q)echo "Cleaning up"
	$(Q)rm -Rf $(OUT)/*.bin $(OUT)/*.exe $(OUT)/*.rc $(OUT)/*.dll $(OUT)/*.o $(OUT)/*.ro $(OUT)/*.a $(OUT)/*.msi $(OUT)//*.h $(OUT)/*.res $(OUT)/popt

$(OUT)/wsl-service.msi: $(OUT)/wsl-service.exe $(OUT)/wsl-tool.exe $(OUT)/wsl-service-events.dll wsl-service.wxs copy-mingw-dlls
	$(Q)echo "MSI $@"
	$(Q)$(WIXL) $(WIXLFLAGS) wsl-service.wxs -o $@
