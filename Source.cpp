#include "Chip8.h"

class Sample : public def::GameEngine
{
public:
	Sample()
	{
		SetTitle("Sample");
	}

	Chip8 emu;
	
	static constexpr int32_t TILE_SIZE = 6;

	std::map<uint16_t, std::string> disassembled;

protected:
	std::string hex(uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	void DrawRam(int x, int y, uint16_t addr, int rows, int columns)
	{
		uint16_t pc = emu.pc;

		int ram_x = x, ram_y = y;
		for (int row = 0; row < rows; row++)
		{
			std::string offset = "$" + hex(addr, 4) + ":";
			for (int col = 0; col < columns; col++)
			{
				offset += " " + hex(emu.read(), 2);
				addr += 1;
			}

			DrawString(ram_x, ram_y, offset);
			ram_y += 10;
		}

		emu.pc = pc;
	}

	void DrawCpu(int x, int y)
	{
		DrawString(x, y, "REGISTERS:", def::WHITE);
		y += 6;

		for (int i = 0; i < 16; i++)
			DrawString(x, y += 10, "V[" + hex(i, 1) + "]: $" + hex(emu.registers[i], 2), def::WHITE);
	
		DrawString(x, y += 10, "V[I]: $" + hex(emu.i, 2), def::WHITE);

		DrawString(x, y += 16, "PC:    $" + hex(emu.pc, 2), def::WHITE);
		DrawString(x, y += 10, "DELAY: $" + hex(emu.delay, 2), def::WHITE);
		DrawString(x, y += 10, "SOUND: $" + hex(emu.sound, 2), def::WHITE);
	}

	void DrawCode(int x, int y, int lines)
	{
		DrawString(x, y, "CODE:", def::WHITE);

		y += 16;

		auto it_a = disassembled.find(emu.pc);
		int line_y = (lines >> 1) * 10 + y;
		if (it_a != disassembled.end())
		{
			DrawString(x, line_y, it_a->second, def::CYAN);

			while (line_y < (lines * 10) + y)
			{
				line_y += 10;
				if (++it_a != disassembled.end())
					DrawString(x, line_y, it_a->second);
			}
		}

		it_a = disassembled.find(emu.pc);
		line_y = (lines >> 1) * 10 + y;
		if (it_a != disassembled.end())
		{
			while (line_y > y)
			{
				line_y -= 10;
				if (--it_a != disassembled.end())
					DrawString(x, line_y, (*it_a).second);
			}
		}
	}

	void DrawKeys(int x, int y)
	{
		DrawString(x, y, "KEYS:", def::WHITE);

		DrawString(x, y += 16, "$0 - X", emu.keys[0x0] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$1 - K1", emu.keys[0x1] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$2 - K2", emu.keys[0x2] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$3 - K3", emu.keys[0x3] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$4 - Q", emu.keys[0x4] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$5 - W", emu.keys[0x5] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$6 - E", emu.keys[0x6] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$7 - A", emu.keys[0x7] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$8 - S", emu.keys[0x8] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$9 - D", emu.keys[0x9] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$A - Z", emu.keys[0xA] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$B - C", emu.keys[0xB] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$C - K4", emu.keys[0xC] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$D - R", emu.keys[0xD] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$E - F", emu.keys[0xE] ? def::GREEN : def::RED);
		DrawString(x, y += 10, "$F - V", emu.keys[0xF] ? def::GREEN : def::RED);
	}

	bool OnUserCreate() override
	{
		Chip8::engine = this;

		emu.load_rom("roms/breakout.ch8");
		
		disassembled = emu.disassemble(0x000, 0xFFF);

		return true;
	}

	bool OnUserUpdate(float deltaTime) override
	{
		//if (GetKey(def::Key::ENTER).pressed)
		emu.clock();

		if (GetKey(def::Key::SPACE).pressed)
		{
			emu.reset();
			emu.load_rom("roms/breakout.ch8");
			disassembled = emu.disassemble(0x000, 0xFFF);
		}

		Clear(def::DARK_BLUE);
		
		// Draw chip8 display pixels
		for (uint8_t y = 0; y < Chip8::DISPLAY_HEIGHT; y++)
			for (uint8_t x = 0; x < Chip8::DISPLAY_WIDTH; x++)
			{
				uint8_t col = emu.display[y * Chip8::DISPLAY_WIDTH + x];
				FillRectangle(
					(2 + x) * TILE_SIZE, (20 + y) * TILE_SIZE,
					TILE_SIZE, TILE_SIZE,
					col == 1 ? def::BLACK : def::WHITE
				);
			}

		// Draw disassembled info

		DrawCpu(432, 10);
		DrawCode(532, 10, 26);
		DrawKeys(432, 270);

		return true;
	}
};

int main()
{
	Sample demo;

	demo.Construct(680, 480, 2, 2, false, true);
	demo.Run();

	return 0;
}
