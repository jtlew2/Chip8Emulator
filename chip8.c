#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned short word;
typedef unsigned char byte;

word opcode;
byte memory[4096];
byte V[16];

word I;
word pc;
word stack[16];
word sp;
byte chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void init();
void emulateCycle();
void test();
void printAll();

int main(int argc, char **argv)
{
	init();
	test();
}

void test()
{
	memory[pc] = 0xA2;
	memory[pc+1] = 0x15;
	printAll();
	emulateCycle();
	printAll();


}
void printAll()
{
	printf("curPC: x%x\n", pc);
	printf("cur Opcode: x%x%x\n", memory[pc], memory[pc+1]);
	printf("curSP: %d\n", sp);
	printf("cur I: x%x\n",I );
	int i;
	for(i = 0; i<16; i++)
	{
		printf("V[%d]: x%x\n", i, V[i]);
	}



}

void init()
{
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;
	srand(time(NULL));


}
void emulateCycle()
{
	opcode = memory[pc] << 8 | memory[pc+1];
	word X = (opcode & 0x0F00)>>8;
	word Y = (opcode & 0x00F0)>>4;
	byte i = 0;
	switch (opcode & 0xF000)
	{
		case 0x0000:
			switch (opcode & 0x000F)
			{
				case 0x0000:		//x00E0 clears screen NEED TO DO
				break;

				case 0x000E:		//x00EE returns from subroutine
					sp--;
					pc = stack[sp]+2;
				break;
			}
		break;

		case 0x1000:				//x1NNN jumps to address NNN
			pc = opcode & 0x0FFF;
		break;

		case 0x2000:				//x2NNN calls subroutine at NNN
			stack[sp]=pc;
			sp++;
			pc = opcode & 0x0FFF;
		break;		

		case 0x3000:				//x3XNN skips next instruction if Vx equals NN

			if (V[X] == (opcode & 0x00FF)) pc+=4;
			else pc+=2;
		break;

		case 0x4000:				//x4XNN skips next instr if Vx doesnt equal NN
			if (V[X] != (opcode & 0x00FF)) pc+=4;
			else pc+=2;
		break;

		case 0x5000:				//x5XY0 skips next instruction if Vx = Vy
			if (V[X] == V[Y]) pc+=4;
			else pc+=2;
		break;

		case 0x6000:				//x6XNN sets Vx to NN
			V[X] = opcode & 0x00FF;
			pc+=2;
		break;	
		
		case 0x7000:				//x7XNN adds NN to VX
			V[X]+= opcode & 0x00FF;
			pc+=2;
		break;

		case 0x8000:
			switch(opcode & 0x000F)
			{
				case 0x0000:		//x8XY0 sets vx to value of vy
					V[X] = V[Y];
					pc+=2;
				break;

				case 0x0001:		//x8XY1 sets Vx to vx OR vy
					V[X] = V[X] | V[Y];
					pc+=2;
				break;

				case 0x0002:		//x8XY2 sets Vx to Vx AND Vy
					V[X] = V[X] & V[Y];
					pc+=2;
				break;

				case 0x0003:		//x8XY3 sets Vx to Vx XOR Vy
					V[X] = V[X] ^ V[Y];
					pc+=2;
				break;

				case 0x0004:		//x8XY4 adds Vy to Vx.  VF is set to 1 when theres carry and 0 otherwise
					if (V[Y] > (0xFF - V[X])) V[0xF] = 1;
					else V[0xF] = 0;
					V[X] += V[Y];
					pc+=2;
				break;

				case 0x0005:		//x8XY5 Vy is subtracted from Vx. Vf set to 0 for a borrow. 1 otherwise 
					if(V[Y] > V[X]) V[0xF]= 0;
					else V[0xF] = 1;
					V[X]-=V[Y];
					pc+=2;
				break;

				case 0x0006:		//x8XY6 shift Vx right by 1. VF set to val of LSB of VX before shift
					V[0xF] = V[X] & 0x01;
					V[X] = V[X] >>1;
					pc+=2;
				break;

				case 0x0007:		//x8XY7 Vx = Vy-Vx VF sets to 0 when borrow, 1 otherwise
					if (V[X] > V[Y]) V[0xF] = 0;
					else V[0xF] = 1;
					V[X] = V[Y] -V[X];
					pc+=2;
				break;

				case 0x000E:		//x8XYE shift VX left by 1. VF set to val of MSB of VX before shift.
					V[0xF] = (V[X] & 0x80) >> 7;
					V[X] = V[X] << 1;
					pc+=2;
				break;
			}		
		break;

		case 0x9000:				//x9XY0 skips instruction if VX != Vy
			if(V[X] != V[Y]) pc+=4;
			else pc+=2;
		break;

		case 0xA000:				//xANNN sets I to NNN
			I = opcode & 0x0FFF;
			pc+=2;
		break;

		case 0xB000:				//BNNN jump to NNN + V0
			pc = opcode & 0x0FFF + V[0];
		break;

		case 0xC000:				//xCXNN sets VX to result of bitwise AND of NN and randNum
			V[X] = (opcode & 0x00FF) & (rand() & 0x00FF);
			pc+=2;
		break;

		case 0xD000:				//SPRITES NEED TO DOOOOOOOOOOO
		break;

		case 0xE000:
			switch(opcode & 0x000F)
			{
				case 0x000E:		//xEX9E skips next instr if key stored in VX is pressed
				break;

				case 0x0001:		//xEXA1 skips next instr if key stored in Vx isnt pressed
				break;
			}
		break;

		case 0xF000:
			switch(opcode & 0x00FF)
			{
				case 0x0007:		//xFX07 sets VX to value of delay timer

				break;

				case 0x000A:		//xFX0A waits for key press then stores in VX
				
				break;

				case 0x0015:		//xFX15 sets delay time to Vx

				break;

				case 0x0018:		//xFX18 sets sound timer to VX

				break;

				case 0x001E:		//FX1E adds VX to I
					I+=V[X];
					pc+=2;
				break;

				case 0x0029:		//FONT STUFF TO DO

				break;

				case 0x0033:		//stores binary coded deciaml of VX to address stored in I. stores MS in I and LS in I +2
					memory[I] = V[X]/100;
					memory[I+1] = (V[X]/10) % 10;
					memory[I+2] = V[X] % 10;
					pc +=2;
				break;

				case 0x0055:		//stores V0 through Vx in memory startting at addrees I
					for (i=0;i<=X;i++)
					{
						memory[I+i]=V[i];
					}
					pc+=2;
				break;

				case 0x0065:		//fills V0 through Vx with val from memory starting at I
					for (i=0;i<=X;i++)
					{
						V[i] = memory[I+i];
					}	
					pc+=2;				
				break;
			}
		break;	

		default:
			printf("Not a valid OPCODE\n");
		break;																		
	}
}