BINARIES := button.bin rtc.bin

# $(call loadaddr,MAPFILE,SEGMENT)
loadaddr = $(shell sed -n 's/^CODE[[:space:]]\+\([0-9A-Fa-f]\{6\}\).*/\1/p' $(1))

all: $(BINARIES)

srec: $(foreach b,$(BINARIES), $(b:.bin=.srec))

clean:
	rm -f *.bin *.srec *.map

%.o: %.s
	ca65 -o $@ $<

%.bin %.map: %.o %.cfg
	ld65 -C $*.cfg -o $@ -m $*.map $<

%.srec: %.bin %.map
	srec_cat $< -binary -offset 0x$(call loadaddr,$*.map) -o $@
