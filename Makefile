ALL=makecards

MORE=

CCOPTS=-I. -D_GNU_SOURCE -g -Wall -funsigned-char -IAXL -IQR -IReedsol -IImage -Iflytools/include -g
OPTS=-LAXL ${CCOPTS}

.PHONY: clean update uncrustify

all: $(ALL)

AXL/axl.o: AXL/axl.c
	make -C AXL
QR/iec18004.o: QR/iec18004.c
	make -C QR
Reedsol/reedsol.o: Reedsol/reedsol.c
	make -C Reedsol
Image/image.o: Image/image.c
	make -C Image
1dbar/1dbar.o: 1dbar/1dbar.c
	make -C 1dbar 1dbar.o

flytools/build/libflytools.a: flytools/Makefile flytools/src/*.c flytools/include/*.h
	$(MAKE) -C flytools

clean:
	rm -f $(ALL) $(MORE) makecourt court.h

update:
	git submodule update --init --recursive --remote
	git commit -a -m "Library update"

uncrustify: .uncrustify.cfg makecards.c makecourt.c
	-uncrustify --replace --no-backup --mtime -c $?

makecards: makecards.c Makefile AXL/axl.o QR/iec18004.o 1dbar/1dbar.o Image/image.o Reedsol/reedsol.o court.h flytools/build/libflytools.a
	cc -O -o $@ $< ${OPTS} -lpopt -DMAIN AXL/axl.o QR/iec18004.o 1dbar/1dbar.o Image/image.o Reedsol/reedsol.o flytools/build/libflytools.a -lcurl -lz

makecourt: makecourt.c Makefile AXL/axl.o flytools/build/libflytools.a
	cc -O -o $@ $< ${OPTS} -lpopt -DMAIN AXL/axl.o flytools/build/libflytools.a -lcurl

SVGFILES := $(wildcard svg/??.svg)
court.h: makecourt ${SVGFILES}
	./makecourt
