#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <stack>
#include <fstream>
#include <map>

// A little hack with one-header libs
#pragma warning(disable : 4996)
#include "stb_image.h"
#include "stb_image_write.h"

#include "defGameEngine.h"

// Original keyboard:
/*
*	123C
*	456D
*	789E
*	A0BF
*/

class Chip8
{
public:
	Chip8();
	~Chip8();

	// There's also a monochrome 64x32 pixels display
	static constexpr uint8_t DISPLAY_WIDTH = 64;
	static constexpr uint8_t DISPLAY_HEIGHT = 32;

	std::array<uint8_t, DISPLAY_WIDTH * DISPLAY_HEIGHT> display;

	void clock();

	bool load_rom(const std::string& filename);
	void reset();

	std::map<uint16_t, std::string> disassemble(uint16_t start, uint16_t end);

	static def::GameEngine* engine;

public:
	// 0x000 <= x <= 0x1FF - used by interpreter
	// (e.g. for storing sprite data)
	// 0x200 <= x <= 0xFFF - used by most programs
	std::array<uint8_t, 0x1000> ram;

	static constexpr uint16_t PROGRAM_BEGIN = 0x200;
	static constexpr uint16_t INTERPRETER_END = 0x1FF;

	// registers[0xF] is not used by any program,
	// because it's used as a flag by some instructions
	std::array<uint8_t, 16> registers;

	// Used to store memory addresses
	// (only 12 rightmost bits are used)
	uint16_t i;

	// Used for delay and sound timers
	uint8_t delay = 0;
	uint8_t sound = 0;

	// Previously executed instruction
	uint16_t last_inst = 0x0;

	// Program counter, that's used for selecting
	// instructions
	uint16_t pc = PROGRAM_BEGIN;

	// Store addresses that should be returned to when
	// subroutine is finished
	std::stack<uint16_t> stack;
	std::array<bool, 16> keys;

	static constexpr uint16_t OPCODES_PER_FRAME = 6;

	void OP_0NNN(); void OP_00E0(); void OP_00EE(); void OP_1NNN(); void OP_2NNN();
	void OP_3XNN(); void OP_4XNN(); void OP_5XY0(); void OP_6XNN(); void OP_7XNN();
	void OP_8XY0(); void OP_8XY1(); void OP_8XY2(); void OP_8XY3(); void OP_8XY4();
	void OP_8XY5(); void OP_8XY6(); void OP_8XY7(); void OP_8XYE(); void OP_9XY0();
	void OP_ANNN(); void OP_BNNN(); void OP_CXNN(); void OP_DXYN(); void OP_EX9E();
	void OP_EXA1(); void OP_FX07(); void OP_FX0A(); void OP_FX15(); void OP_FX18();
	void OP_FX1E(); void OP_FX29(); void OP_FX33(); void OP_FX55(); void OP_FX65();

	struct Instruction
	{
		std::string name;
		void (Chip8::*operate)(void) = nullptr;
	};

	// Info about instructions (used for disassembling)
	// Order of instructions is reserved
	std::vector<Instruction> instructions;

	static uint8_t lookup_key(uint32_t key);

	void make_sound() const;
	uint16_t read();

	uint8_t get_x() const;
	uint8_t get_y() const;

	Instruction& decode(uint16_t inst);
	
};

