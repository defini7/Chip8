#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define DGE_APPLICATION
#include "Chip8.h"

def::GameEngine* Chip8::engine = nullptr;

Chip8::Chip8()
{
	using a = Chip8;
	instructions =
	{
		{ "??           ", &a::OP_0NNN }, { "CLS          ", &a::OP_00E0 }, { "RET          ", &a::OP_00EE }, { "JMP          ", &a::OP_1NNN }, { "CALL NNN     ", &a::OP_2NNN },
		{ "SE VX, NN    ", &a::OP_3XNN }, { "SNE VX, NN   ", &a::OP_4XNN }, { "SE VX, VY    ", &a::OP_5XY0 }, { "LD VX, NN    ", &a::OP_6XNN }, { "ADD VX, NN   ", &a::OP_7XNN },
		{ "LD VX, VY    ", &a::OP_8XY0 }, { "OR VX, VY    ", &a::OP_8XY1 }, { "AND VX, VY   ", &a::OP_8XY2 }, { "XOR VX, VY   ", &a::OP_8XY3 }, { "ADD VX, VY   ", &a::OP_8XY4 },
		{ "SUB VX, VY   ", &a::OP_8XY5 }, { "SHR VX {, VY}", &a::OP_8XY6 }, { "SUBN VX, VY  ", &a::OP_8XY7 }, { "SHL VX {, VY}", &a::OP_8XYE }, { "SNE VX, VY   ", &a::OP_9XY0 },
		{ "LD I, NNN    ", &a::OP_ANNN }, { "JMP V0, NNN  ", &a::OP_BNNN }, { "RND VX, NN   ", &a::OP_CXNN }, { "DRW VX, VY, N", &a::OP_DXYN }, { "SKP VX       ", &a::OP_EX9E },
		{ "SKNP VX      ", &a::OP_EXA1 }, { "LD VX, DT    ", &a::OP_FX07 }, { "LD VX, K     ", &a::OP_FX0A }, { "LD DT, VX    ", &a::OP_FX15 }, { "LD ST, VX    ", &a::OP_FX18 },
		{ "ADD I, VX    ", &a::OP_FX1E }, { "LD F, VX     ", &a::OP_FX29 }, { "LD B, VX     ", &a::OP_FX33 }, { "LD [I], VX   ", &a::OP_FX55 }, { "LD VX, [I]   ", &a::OP_FX65 }
	};

	reset();
}

Chip8::~Chip8()
{
}

std::map<uint16_t, std::string> Chip8::disassemble(uint16_t start, uint16_t end)
{
	std::map<uint16_t, std::string> lines;

	auto hex = [](uint32_t n, uint8_t d)
	{
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4)
			s[i] = "0123456789ABCDEF"[n & 0xF];
		return s;
	};

	pc = start;

	while (pc <= end)
	{
		uint16_t line_addr = pc;
		lines[line_addr] = "$" + hex(pc, 4) + ": " + decode(read()).name;
	}

	pc = PROGRAM_BEGIN;
	return lines;
}

void Chip8::clock()
{
	if (delay > 0) delay--;
	if (sound > 0)
	{
		sound--;
		make_sound();
	}

	keys[0x1] = engine->GetKey(def::Key::K1).held;
	keys[0x2] = engine->GetKey(def::Key::K2).held;
	keys[0x3] = engine->GetKey(def::Key::K3).held;
	keys[0xC] = engine->GetKey(def::Key::K4).held;
	keys[0x4] = engine->GetKey(def::Key::Q).held;
	keys[0x5] = engine->GetKey(def::Key::W).held;
	keys[0x6] = engine->GetKey(def::Key::E).held;
	keys[0xD] = engine->GetKey(def::Key::R).held;
	keys[0x7] = engine->GetKey(def::Key::A).held;
	keys[0x8] = engine->GetKey(def::Key::S).held;
	keys[0x9] = engine->GetKey(def::Key::D).held;
	keys[0xE] = engine->GetKey(def::Key::F).held;
	keys[0xA] = engine->GetKey(def::Key::Z).held;
	keys[0x0] = engine->GetKey(def::Key::X).held;
	keys[0xB] = engine->GetKey(def::Key::C).held;
	keys[0xF] = engine->GetKey(def::Key::V).held;

	for (uint16_t _i = 0; _i < OPCODES_PER_FRAME; _i++)
	{
		// Execute opcode
		last_inst = read();

		// Decode instruction and call
		// associated method
		(this->*decode(last_inst).operate)();
	}
}

void Chip8::make_sound() const
{
	// TODO: Make sound
}

bool Chip8::load_rom(const std::string& filename)
{
	reset();
	OP_00E0();

	FILE* rom = fopen(filename.c_str(), "rb");
	if (!rom) return false;

	fread(&ram[PROGRAM_BEGIN], ram.size() - 1, 1, rom);
	fclose(rom);

	return true;
}

void Chip8::reset()
{
	i = 0x0;
	pc = PROGRAM_BEGIN;

	delay = 0;
	sound = 0;

	keys.fill(false);

	registers.fill(0x0);
	ram.fill(0);

	// Fill RAM with sprites i.e. numbers
	ram =
	{
		0xF0,0x90,0x90,0x90,0xF0,
		0x20,0x60,0x20,0x20,0x70,
		0xF0,0x90,0x90,0x90,0xF0,
		0xF0,0x10,0xF0,0x10,0xF0,
		0x90,0x90,0xF0,0x10,0x10,
		0xF0,0x80,0xF0,0x10,0xF0,
		0xF0,0x80,0xF0,0x90,0xF0,
		0xF0,0x10,0x20,0x40,0x40,
		0xF0,0x90,0xF0,0x90,0xF0,
		0xF0,0x90,0xF0,0x10,0xF0,
		0xF0,0x90,0xF0,0x90,0x90,
		0xE0,0x90,0xE0,0x90,0xE0,
		0xF0,0x80,0x80,0x80,0xF0,
		0xE0,0x90,0x90,0x90,0xE0,
		0xF0,0x80,0xF0,0x80,0xF0,
		0xF0,0x80,0xF0,0x80,0x80
	};
}

uint8_t Chip8::lookup_key(uint32_t key)
{
	switch (key)
	{
	case def::Key::K1: return 0x1;
	case def::Key::K2: return 0x2;
	case def::Key::K3: return 0x3;
	case def::Key::K4: return 0xC;
	case def::Key::Q: return 0x4;
	case def::Key::W: return 0x5;
	case def::Key::E: return 0x6;
	case def::Key::R: return 0xD;
	case def::Key::A: return 0x7;
	case def::Key::S: return 0x8;
	case def::Key::D: return 0x9;
	case def::Key::F: return 0xE;
	case def::Key::Z: return 0xA;
	case def::Key::X: return 0x0;
	case def::Key::C: return 0xB;
	case def::Key::V: return 0xF;
	}

	return -1;
}

uint8_t Chip8::get_x() const
{
	return (last_inst & 0x0F00) >> 8;
}

uint8_t Chip8::get_y() const
{
	return (last_inst & 0x00F0) >> 4;
}

Chip8::Instruction& Chip8::decode(uint16_t inst)
{
	switch (inst & 0xF000)
	{
	case 0x0000:
	{
		switch (inst & 0xF)
		{
		case 0x0: return instructions[1];
		case 0xE: return instructions[2];
		}
	}
	break;

	case 0x1000: return instructions[3];
	case 0x2000: return instructions[4];
	case 0x3000: return instructions[5];
	case 0x4000: return instructions[6];
	case 0x5000: return instructions[7];
	case 0x6000: return instructions[8];
	case 0x7000: return instructions[9];

	case 0x8000:
	{
		switch (inst & 0xF)
		{
		case 0x0: return instructions[10];
		case 0x1: return instructions[11];
		case 0x2: return instructions[12];
		case 0x3: return instructions[13];
		case 0x4: return instructions[14];
		case 0x5: return instructions[15];
		case 0x6: return instructions[16];
		case 0x7: return instructions[17];
		case 0xE: return instructions[18];
		}
	}
	break;

	case 0x9000: return instructions[19];
	case 0xA000: return instructions[20];
	case 0xB000: return instructions[21];
	case 0xC000: return instructions[22];
	case 0xD000: return instructions[23];

	case 0xE000:
	{
		switch (inst & 0xF)
		{
		case 0xE: return instructions[24];
		case 0x1: return instructions[25];
		}
	}
	break;

	case 0xF000:
	{
		switch (inst & 0xFF)
		{
		case 0x07: return instructions[26];
		case 0x0A: return instructions[27];
		case 0x15: return instructions[28];
		case 0x18: return instructions[29];
		case 0x1E: return instructions[30];
		case 0x29: return instructions[31];
		case 0x33: return instructions[32];
		case 0x55: return instructions[33];
		case 0x65: return instructions[34];
		}
	}
	break;

	}

	return instructions[0];
}

uint16_t Chip8::read()
{
	return (ram[pc++] << 8) | ram[pc++];
}

void Chip8::OP_0NNN()
{
	// Does nothing
}

// CLS
void Chip8::OP_00E0()
{
	display.fill(0);
}

// RET
void Chip8::OP_00EE()
{
	pc = stack.top();
	stack.pop();
}

// JMP
void Chip8::OP_1NNN()
{
	pc = last_inst & 0x0FFF;
}

// CALL NNN
void Chip8::OP_2NNN()
{
	stack.push(pc);
	pc = last_inst & 0x0FFF;
}

// SE VX, NN
void Chip8::OP_3XNN()
{
	uint8_t nn = last_inst & 0x00FF;

	if (registers[get_x()] == nn)
		pc += 2;
}

// SNE VX, NN
void Chip8::OP_4XNN()
{
	uint8_t nn = last_inst & 0x00FF;

	if (registers[get_x()] != nn)
		pc += 2;
}

// SE VX, VY
void Chip8::OP_5XY0()
{
	if (registers[get_x()] == registers[get_y()])
		pc += 2;
}

// LD VX, NN
void Chip8::OP_6XNN()
{
	registers[get_x()] = last_inst & 0x00FF;
}

// ADD VX, NN
void Chip8::OP_7XNN()
{
	registers[get_x()] += last_inst & 0x00FF;
}

// LD VX, VY
void Chip8::OP_8XY0()
{
	registers[get_x()] = registers[get_y()];
}

// OR VX, VY
void Chip8::OP_8XY1()
{
	registers[get_x()] |= registers[get_y()];
}

// AND VX, VY
void Chip8::OP_8XY2()
{
	registers[get_x()] &= registers[get_y()];
}

// XOR VX, VY
void Chip8::OP_8XY3()
{
	registers[get_x()] ^= registers[get_y()];
}

// ADD VX, VY
void Chip8::OP_8XY4()
{
	uint8_t x = get_x();
	uint16_t res = registers[x] + registers[get_y()];

	registers[0xF] = uint8_t(res > 0xFF);
	registers[x] = res;
}

// SUB VX, VY
void Chip8::OP_8XY5()
{
	uint8_t x = get_x();
	uint16_t res = registers[x] - registers[get_y()];

	registers[0xF] = uint8_t(res > 0xFF);
	registers[x] = res;
}

// SHR VX {, VY}
void Chip8::OP_8XY6()
{
	uint8_t x = get_x();
	registers[0xF] = registers[x] & 0x1;
	registers[x] = registers[get_y()] >> 1;
}

// SUBN VX, VY
void Chip8::OP_8XY7()
{
	uint8_t x = get_x(), y = get_y();

	registers[0xF] = uint8_t(registers[y] > registers[x]);
	registers[x] = registers[y] - registers[x];
}

// SHL VX {, VY}
void Chip8::OP_8XYE()
{
	uint8_t x = get_x();

	registers[0xF] = registers[x] & 0x80;
	registers[x] = registers[get_y()] << 1;
}

// SNE VX, VY
void Chip8::OP_9XY0()
{
	if (registers[get_x()] != registers[get_y()])
		pc += 2;
}

// LD I, NNN
void Chip8::OP_ANNN()
{
	i = last_inst & 0x0FFF;
}

// JMP V0, NNN
void Chip8::OP_BNNN()
{
	pc = registers[0x0] + (last_inst & 0x0FFF);
}

// RND VX, NN
void Chip8::OP_CXNN()
{
	registers[get_x()] = rand() & (last_inst & 0x00FF);
}

// DRW VX, VY, N
void Chip8::OP_DXYN()
{
	uint8_t n = last_inst & 0x000F;

	uint8_t x = registers[get_x()] % DISPLAY_WIDTH;
	uint8_t y = registers[get_y()] % DISPLAY_HEIGHT;

	registers[0xF] = 0x0;

	for (int row = 0; row < n; row++)
	{
		uint8_t bits = ram[i + row];
		uint8_t py = (y + row) % DISPLAY_HEIGHT;

		for (int col = 0; col < 8; col++)
		{
			uint8_t px = (x + col) % DISPLAY_WIDTH;
			uint8_t& c = display[py * DISPLAY_WIDTH + px];
			
			if (bits & (0x1 << (7 - col)))
			{
				if (c == 1)
				{
					c = 0;
					registers[0xF] = 1;
				}
				else
					c = 1;
			}

			if (px == DISPLAY_WIDTH - 1)
				break;
		}

		if (py == DISPLAY_HEIGHT - 1)
			break;
	}
}

// SKP VX
void Chip8::OP_EX9E()
{
	if (keys[registers[get_x()]])
		pc += 2;
}

// SKNP VX
void Chip8::OP_EXA1()
{
	if (!keys[registers[get_x()]])
		pc += 2;
}

// LD VX, DT
void Chip8::OP_FX07()
{
	registers[get_x()] = delay;
}

// LD VX, K
void Chip8::OP_FX0A()
{
	uint8_t k = lookup_key(engine->AnyKey());

	if (k == 255U)
	{
		// Go back to the previous instruction
		// So on the next decoding iteration
		// the same instruction will be executed
		pc -= 2;
	}
	else
	{
		registers[get_x()] = k;
	}
}

// LD DT, VX
void Chip8::OP_FX15()
{
	delay = registers[get_x()];
}

// LD ST, VX
void Chip8::OP_FX18()
{
	sound = registers[get_x()];
}

// ADD I, VX
void Chip8::OP_FX1E()
{
	i += registers[get_x()];
}

// LD F, VX
void Chip8::OP_FX29()
{
	// Each font has a length of 5 bytes
	i = registers[get_x()] * 0x05;
}

// LD B, VX
void Chip8::OP_FX33()
{
	uint8_t x = get_x();

	ram[i + 0x0] = registers[x] / 100;
	ram[i + 0x1] = (registers[x] / 10) % 10;
	ram[i + 0x2] = registers[x] % 10;
}

// LD [I], VX
void Chip8::OP_FX55()
{
	uint8_t x = get_x();

	for (uint8_t reg = 0; reg <= x; reg++)
		ram[i + reg] = registers[reg];

	i += x + 1;
}

// LD VX, [I]
void Chip8::OP_FX65()
{
	uint8_t x = get_x();

	for (uint8_t reg = 0; reg <= x; reg++)
		registers[reg] = ram[i + reg];
	
	i += x + 1;
}
