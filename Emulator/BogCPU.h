#pragma once
#include "Global.h"
#include "Bootrom.h"

#define interrupt_period 1000

#define WORD      1
#define DWORD     2

// FLAG BIT POSITIONS
#define C_FLAG    4
#define H_FLAG    5
#define N_FLAG    6
#define Z_FLAG    7

// OPCODES
#define NOP       0x00 // NOP
#define LD_SP_D16 0x31 // LD SP,d16
#define LD_HL_D16 0x21 // LD HL,d16
#define LD_PHLM_A 0x32 // LD (HL-),A  
#define XOR_A     0xAF // XOR A
#define JR_NZ_R8  0x20 // JR NZ,r8
#define PREFIX_CB 0xCB // PREFIX CB

// CB EXTENSION
#define RLC       0    // Rotate left
#define RL        1    // Rotate left through C flag
#define SLA       2    // Shift left
#define SWAP      3    // Swap high and low nibble
#define BIT       4    // Check if n'th bit is set
#define RES       5    // Reset n'th bit
#define SET       6    // Set n'th bit
#define RRC       7    // Rotate right
#define RR        8    // Rotate right through C flag
#define SRA       9    // Shift right (preserve sign)
#define SRL       10   // Shift right (reset bit 7)


// TODO optimize
const byte CB_EXT_OPS[2][16]
{
	{ RLC,RL,SLA,SWAP,BIT,BIT,BIT,BIT,RES,RES,RES,RES,SET,SET,SET,SET },
	{ RRC,RR,SRA,SRL ,BIT,BIT,BIT,BIT,RES,RES,RES,RES,SET,SET,SET,SET }
};


class BogCPU
{

public:
	byte mem[65536]; // 64KB address space

	// Stack pointer
	dword SP;
	// Program counter
	dword PC;
	// Register
	word F, A, C, B, E, D, L, H;
	// Register
	dword *AF = (dword*)&F, *BC = (dword*)&C, *DE = (dword*)&E, *HL = (dword*)&L;
	// Register pointers, in order: B, C, D, E, H, L, HL, A
	dword* regPtrArray[8]{ (dword*)&B, (dword*)&C, (dword*)&D, (dword*)&E, (dword*)&H, (dword*)&L, HL, (dword*)&A }; // DO NOT CHANGE ELEMENT ORDER

	bool running;
	uint32_t cycles;

	BogCPU();

	void reset();
	void run();

	// Returns a pointer to an addr.space location. 
	byte* ptr(dword* Addr);
	// Loads Size number of bytes from Src to Dst.
	void load(void* Dst, void* Src, size_t Size);
	// Loads Size number of bytes from mem, starting at current PC, into Dst.
	void load(void* Dst, rsize_t Size);
	// Sets a flag.
	void setFlag(size_t flag);
	// Sets a flag if the LSB of value is set. Does not clear the flag if value == 0.
	void setFlag(size_t flag, byte value);
	// Clears a flag.
	void clearFlag(size_t flag);
	// Clear all flags.
	void clearFlags();
	// Returns the state of a flag.
	bool isSet(size_t flag);
	// Prints the content of all registers.
	void printRegisters();

	void cb(byte opcode) throw();
private:
	

};


inline byte* BogCPU::ptr(dword* Addr)
{
	return mem + *Addr;
}


inline void BogCPU::load(void* Dst, void* Src, size_t Size)
{
	memcpy(Dst, Src, Size);
}


inline void BogCPU::load(void* Dst, rsize_t Size)
{
	load(Dst, mem+PC, Size);
}


inline void BogCPU::setFlag(size_t flag)
{
	F |= 1 << flag;
}


inline void BogCPU::setFlag(size_t flag, byte value)
{
	F |= (value & 1) << flag;
}


inline void BogCPU::clearFlag(size_t flag)
{
	F &= ~(1 << flag);
}


inline void BogCPU::clearFlags()
{
	//F &= 0xF;
	F ^= F; // NB. whole F reg is reset
}


inline bool BogCPU::isSet(size_t flag)
{
	return (F & (1 << flag)) != 0;
}

