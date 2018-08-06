SRCDIR=src
INCDIR=include
OBJDIR=build
BINDIR=bin

F_CPU=16000000UL
CFLAGS=-mmcu=atmega328p -I$(INCDIR) -Wall -DF_CPU=$(F_CPU) -Os -DPIN='"$(pin)"'
LFLAGS=$(CFLAGS)
AVRDUDE_FLAGS=-p atmega328p -carduino -P /dev/$(port)

CROSS_COMPILE=avr-
CC=$(CROSS_COMPILE)gcc
OBJCOPY=$(CROSS_COMPILE)objcopy
AVRDUDE=avrdude

all:	$(BINDIR)/gsm800.hex

$(BINDIR)/gsm800.hex: $(BINDIR)/gsm800.elf | $(BINDIR)
	$(OBJCOPY) -R .eeprom -O ihex $< $@

$(BINDIR)/gsm800.eep: $(BINDIR)/gsm800.elf | $(BINDIR)
	$(OBJCOPY) -j .eeprom -O ihex $< $@

$(BINDIR)/gsm800.elf:	$(OBJDIR)/main.o $(OBJDIR)/uart.o | $(BINDIR)
	$(CC) $(LFLAGS) -o $@ $^

$(OBJDIR)/main.o:	$(SRCDIR)/main.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(OBJDIR)/uart.o:	$(SRCDIR)/uart.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	@rm -rf $(OBJDIR) $(BINDIR)

program:	$(BINDIR)/gsm800.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$^

initialize:	$(BINDIR)/gsm800.eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U eeprom:w:$^

$(OBJDIR):
	@mkdir $@

$(BINDIR):
	@mkdir $@
