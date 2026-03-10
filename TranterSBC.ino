#include <r65emu.h>
#include <r6502.h>
#include <acia.h>
#include <via.h>
#include <debugging.h>
#include "roms/osi_bas.h"

Memory memory;
r6502 cpu(memory);
Arduino machine(cpu);
ACIA acia;
VIA via;
prom basic(osi_bas, 16384);
ram<> pages[32];

static void irq_handler(bool irq) {
	if (irq)
		cpu.raise(0);
}

static void acia_init() {
	acia.register_framing_handler([](uint8_t) {
#if DEBUGGING == DEBUG_NONE
		Serial.begin(TERMINAL_SPEED, SERIAL_8N1);
#endif
	});
	acia.register_read_data_handler([]() {
		uint8_t b = Serial.read();
		DBG_EMU("read: %02x", b);
		if (b == 0x0e)		// ^N
			machine.reset();
		else if (b == 0x08)	// BS
			b = '_';
		return b;
	});
	acia.register_write_data_handler([](uint8_t b) {
		DBG_EMU("write: %02x", b);
		if (b == '_') {
			Serial.write(0x08);
			Serial.write(' ');
			Serial.write(0x08);
			return;
		}
		Serial.write(b);
	});
	acia.register_can_rw_handler([](void) {
		uint8_t s = 0;
		if (Serial.available() > 0) s++;
		if (Serial.availableForWrite() > 0) s += 2;
		DBG_EMU("can_rw: %x", s);
		return s;
	});
	acia.register_irq_handler(irq_handler);
	machine.register_pollable(acia);
}

// Lilygo TTGO
#define BUTTON_PIN	36
#define LED_PIN		14

static void via_init() {
	via.register_irq_handler(irq_handler);
	via.register_porta_input_handler([]() {
		return (uint8_t)!digitalRead(BUTTON_PIN);
	});
	via.register_porta_output_handler([](uint8_t b) {
		digitalWrite(LED_PIN, (b & 0x80)? LOW: HIGH);
	});
}

void setup() {

	pinMode(LED_PIN, OUTPUT);
	pinMode(BUTTON_PIN, INPUT);

	digitalWrite(LED_PIN, LOW);

	machine.begin();

	for (unsigned i = 0; i < 32; i++)
		memory.put(pages[i], i * ram<>::page_size);

	memory.put(via, 0x8000, 8192);
	memory.put(acia, 0xa000, 8192);
        memory.put(basic, 0xc000);

	via_init();
	acia_init();

	machine.reset();
}

void loop() {

	machine.run();
}
