#include <r65emu.h>
#include <r6502.h>
#include <acia.h>
#include <via.h>
#include <debugging.h>
#include "roms/osi_bas.h"

Memory memory;
r6502 cpu(memory);
Machine machine(cpu);

static void irq_handler(bool irq) {
	if (irq)
		cpu.raise(0);
}

class SerialAcia: public Memory::Device {
public:
	SerialAcia(): Memory::Device(8192) {}

	void init() {
		_acia.register_framing_handler([](uint32_t cfg) {
#if DEBUGGING != DEBUG_NONE
			DBG_EMU(printf("framing: %x\r\n", cfg));
#else
			Serial.begin(TERMINAL_SPEED, cfg);
#endif
		});
		_acia.register_read_data_handler([]() {
			uint8_t b = Serial.read();
			DBG_EMU(printf("read: %x\r\n", b));
			if (b == 0x0e)		// ^N
				machine.reset();
			else if (b == 0x08)	// BS
				b = '_';
			return b;
		});
		_acia.register_write_data_handler([](uint8_t b) {
			DBG_EMU(printf("write: %x\r\n", b));
			if (b == '_') {
				Serial.write(0x08);
				Serial.write(' ');
				Serial.write(0x08);
				return;
			}
			Serial.write(b);
		});
		_acia.register_can_rw_handler([](void) {
			uint8_t s = 0;
			if (Serial.available() > 0) s++;
			if (Serial.availableForWrite() > 0) s += 2;
			DBG_EMU(printf("can_rw: %x\r\n", s));
			return s;
		});
		_acia.register_irq_handler(irq_handler);
	}

	void poll() { _acia.poll_for_interrupt(); }

	virtual void operator=(uint8_t b) { _acia.write(_acc, b); }
	virtual operator uint8_t() { return _acia.read(_acc); }

private:
	ACIA _acia;

} acia;

class ViaDevice: public Memory::Device {
public:
	ViaDevice(): Memory::Device(8192) {}

	void init() {
		_via.register_irq_handler(irq_handler);
	}

	void poll() { _via.tick(); }

	virtual void operator=(uint8_t b) { _via.write(_acc, b); }
	virtual operator uint8_t() { return _via.read(_acc); }

private:
	VIA _via;
} via;

prom basic(osi_bas, 16384);
ram<> pages[32];

void setup() {

	machine.init();

        for (unsigned i = 0; i < 32; i++)
                memory.put(pages[i], i * ram<>::page_size);

	memory.put(via, 0x8000);
	memory.put(acia, 0xa000);
        memory.put(basic, 0xc000);

	via.init();
	acia.init();

	machine.reset();
}

void loop() {

	acia.poll();
	via.poll();
	machine.run();
}
