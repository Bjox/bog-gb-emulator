#include "BogCPU.h"
#include <iostream>


BogCPU::BogCPU()
{
	std::cout << "CPU created." << std::endl;
	reset();
}


void BogCPU::reset()
{
	// reset running state
	running = false;

	// clear addr.space
	memset(mem, 0, sizeof(mem));

	// set registers to 0
	memset(&F, 0, sizeof(word)*8);
	SP = 0;
	PC = 0;

	// reset cycles counter
	cycles = 0;

	// copy boot rom to beginning of addr.space
	memcpy(mem, BOOT_ROM, sizeof(BOOT_ROM));
}


void BogCPU::run()
{
	running = true;
	byte opcode;

	try {

		for (;;) { // Main loop


			/*
			1. read next opcode from ram.
				2. execute instruction.
				3. increment PC.
				4. increment cycle counter.
			5. check interrupt cycle counter.
			*/

			opcode = mem[PC++];

			switch (opcode) {

			case NOP:
				cycles += 4;
				break;

			case LD_SP_D16:
				load(&SP, DWORD);
				PC += 2;
				cycles += 12;
				break;

			case LD_HL_D16:
				load(HL, DWORD);
				PC += 2;
				cycles += 12;
				break;

			case LD_PHLM_A:
				load(ptr(HL), &A, WORD);
				--(*HL);
				cycles += 8;
				break;

			case XOR_A:
				A ^= A;
				clearFlags();
				setFlag(Z_FLAG);
				cycles += 4;
				break;

			case PREFIX_CB:
				cycles += 4;
				opcode = mem[PC++];
				cb(opcode);
				break;

			default:
				throw opcode;
			}

			if (cycles >= interrupt_period) {
				cycles -= interrupt_period;

				// TODO: handle interrupt

				if (!running) break;
			}

		}

	}
	catch (dword opcode) {
		printf("Opcode 0x%04X at 0x%04X not supported!\n", opcode, PC - 1);
	}
	catch (byte opcode) {
		printf("Opcode 0x%02X at 0x%04X not supported!\n", opcode, PC - 1);
	}
}


void BogCPU::cb(byte opcode)
{
	byte Lnibble = opcode >> 4;
	byte Rnibble = opcode & 0xF;

	byte     op1 = 2 * (Lnibble & 3) + (Rnibble >> 3);
	dword*   op2 = regPtrArray[Rnibble & 7];

	opcode = CB_EXT_OPS[Rnibble >> 3][Lnibble];

	byte Fbackup = F;
	bool HLop = !((Rnibble + 2) & 7); // true if the operation involves (HL)
	byte* valuePtr = HLop ? ptr(op2) : (byte*)op2;
	cycles += 4 * (1 + HLop); // inc cycles

	switch (opcode) {
	// ALL INSTRUCTIONS HERE ARE 1 BYTE AND 4/8 CYCLES
	// DO NOT INC CYCLES

	case RLC:
		*valuePtr = _rotl8(*valuePtr, 1);
		clearFlags();
		F = (*valuePtr & 1) << C_FLAG; // handle carry flag
		setFlag(Z_FLAG, *valuePtr == 0); // handle zero flag
		break;

	case RL:
		clearFlags();
		F = (*valuePtr >> 7) << C_FLAG;
		*valuePtr <<= 1;
		*valuePtr |= (Fbackup >> C_FLAG) & 1;
		setFlag(Z_FLAG, *valuePtr == 0); // handle zero flag
		break;

	case SLA:
		clearFlags();
		F = (*valuePtr >> 7) << C_FLAG;
		*valuePtr <<= 1;
		setFlag(Z_FLAG, *valuePtr == 0); // handle zero flag
		break;

	case SWAP:
		clearFlags();
		*valuePtr = *valuePtr >> 4 | *valuePtr << 4;
		setFlag(Z_FLAG, *valuePtr == 0); // handle zero flag
		break;

	case BIT:
		F &= 0x10; // clear all flags except carry
		setFlag(H_FLAG);
		F |= (~*valuePtr >> op1 & 1) << Z_FLAG;
		break;

	/*case RES:

		break;

	case SET:

		break;*/

	case RRC:
		*valuePtr = _rotr8(*valuePtr, 1);
		clearFlags();
		F = (*valuePtr >> 7) << C_FLAG; // handle carry flag
		setFlag(Z_FLAG, *valuePtr == 0); // handle zero flag
		break;

	case RR:
		clearFlags();
		F = (*valuePtr & 1) << C_FLAG;
		*valuePtr >>= 1;
		*valuePtr |= (Fbackup << 3) & 0x80;
		setFlag(Z_FLAG, *valuePtr == 0); // handle zero flag
		break;

	case SRA:
		clearFlags();
		//F = (*valuePtr & 1) << C_FLAG; // NB. Should SRA set carry flag?
		*valuePtr = ((*valuePtr & 0x7F) >> 1) | (*valuePtr & 0x80);
		setFlag(Z_FLAG, *valuePtr == 0); // handle zero flag
		break;

	case SRL:
		clearFlags();
		F = (*valuePtr & 1) << C_FLAG;
		*valuePtr >>= 1;
		setFlag(Z_FLAG, *valuePtr == 0); // handle zero flag
		break;


	default:
		throw (dword)(PREFIX_CB << 8 | Rnibble | Lnibble << 4);
	}
}


void BogCPU::printRegisters()
{
	printf("AF = 0x%04X, Flags = %s %s %s %s\n", (int)*AF, isSet(Z_FLAG) ? "Z" : "", isSet(N_FLAG) ? "N" : "", isSet(H_FLAG) ? "H" : "", isSet(C_FLAG) ? "C" : "");
	printf("BC = 0x%04X\n", (int)*BC);
	printf("DE = 0x%04X\n", (int)*DE);
	printf("HL = 0x%04X\n", (int)*HL);
	printf("SP = 0x%04X\n", (int)SP);
	printf("PC = 0x%04X\n", (int)PC);
}