#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

#define HEIGHT 32
#define WIDTH 64
#define DELAY_VALUE 8
typedef unsigned short word;
typedef unsigned char byte;

int sizeMod = 10;
word opcode;
byte memory[4096];
byte V[16];
byte sound_timer;
byte delay_timer;
byte key[16];
byte quit = 0;
byte pixels[64*32];
byte draw;

word I;
word pc;
word stack[16];
word sp;
byte font[80] =
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

SDL_Window * window;
SDL_Surface * screenSurface;

void init();
void emulateCycle();
void test();
void printAll();
void initIO();
int loadGame(char * game);
int updateKey();
void closeSDL();
void clearScreen();
void drawSprite(word X, word Y, word rows);
void updateGraphics();

int main(int argc, char **argv)
{
	if(argc!=2)
	{
		printf("Not a valid input.\n");
		return 0;
	}
	if (! loadGame(argv[1]))
	{
		return;
	}
	init();
	initIO();
	do{
		 emulateCycle();
		 if (draw){
		 	SDL_Delay(DELAY_VALUE);	

		 	updateGraphics();
		 } 

	}while(!updateKey());
 	closeSDL();
}

void initIO()
{
	window = NULL;
	screenSurface = NULL;

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }

    else{
        //Create window
        window = SDL_CreateWindow( "CHIP-8 EMULATOR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH*sizeMod, HEIGHT*sizeMod, SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get window surface
            screenSurface = SDL_GetWindowSurface( window );

		}
		updateGraphics();


		return;

    }
}

int loadGame(char * game)
{
	FILE * fp;
	fp = fopen(game,"r");
	if (fp == NULL) 
	{
		printf("Not a valid game\n");
		return 0;
	}
	int i = 0;
	while(1){ 
		if(feof(fp))
			break;
		memory[0x200 + i] = fgetc(fp);
		i++;
	}
	fclose(fp);
	return 1;
}

void  closeSDL()
{
	SDL_FreeSurface(screenSurface);
	screenSurface=NULL;
	SDL_DestroyWindow(window);
	window = NULL;
	SDL_Quit();

}

void test()
{
	printAll();
	emulateCycle();
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
	int i = 0;
	for (i;i<80;i++)
	{
		memory[i] = font[i];
	}
	sound_timer=0;
	delay_timer = 0;
	for (i = 0;i<16;i++)
		key[i]=0;

	return;
}

void clearScreen()
{
	int i = 0;
	for(i;i<64*32;i++)
	{
		pixels[i] = 0;
	}
	draw = 1;
	return;
}

void drawSprite(word X, word Y, word rows)
{
	int i;
	int j;
	int coord;
	byte line;
	V[0xF] = 0;
	for (i = 0; i<rows; i++)
	{
		line = memory[I+i];
		for (j =0; j<8;j++)
		{
			coord = ((V[X]+j)%64) + ((V[Y]+i)%32)*64;
			if (pixels[coord] == 1)
			{
				if(line>>7)
				{
					V[0xF]=1;
					pixels[coord]=0;
				}
			}
			else
			{
				if(line>>7)
				{
					pixels[coord]=1;
				}
			}
			line = line<<1;

		}
	}
	draw = 1;
	return;
}

void updateGraphics()
{
	Uint32 black = SDL_MapRGB(screenSurface->format,0,0,0);
	Uint32 white = SDL_MapRGB(screenSurface->format,255,255,255);
	int x = 0;
	int y = 0;
	int i = 0;
	for (y = 0; y<HEIGHT *sizeMod;y++ )
	{
		for(x = 0; x<WIDTH * sizeMod; x++)
		{
			i = y/10*64+x/10;
			Uint32 * target = (Uint8*) screenSurface->pixels + y * screenSurface->pitch + x * sizeof *target;
			if(pixels[i]==0) *target = black;
			else *target = white;			
		}
	}
	SDL_UpdateWindowSurface(window);
	draw = 0;
	return;

}

void emulateCycle()
{
	opcode = memory[pc] << 8 | memory[pc+1];
	word X = (opcode & 0x0F00)>>8;
	word Y = (opcode & 0x00F0)>>4;
	byte i = 0;
	byte pressed = 0;
	switch (opcode & 0xF000)
	{
		case 0x0000:
			switch (opcode & 0x000F)
			{
				case 0x0000:		//x00E0 clears screen
					clearScreen();
					pc+=2;
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

		case 0xD000:				//xDXYN draws sprite. check wiki for details. https://en.wikipedia.org/wiki/CHIP-8
			drawSprite(X,Y, opcode & 0x000F);
			pc+=2;
		break;

		case 0xE000:
			switch(opcode & 0x000F)
			{
				case 0x000E:		//xEX9E skips next instr if key stored in VX is pressed
					if(key[V[X]]) pc+=4;
					else pc+=2;
				break;

				case 0x0001:		//xEXA1 skips next instr if key stored in Vx isnt pressed
					if(key[V[X]]) pc+=2;
					else pc+=4;
				break;
			}
		break;

		case 0xF000:
			switch(opcode & 0x00FF)
			{
				case 0x0007:		//xFX07 sets VX to value of delay timer
					V[X]=delay_timer;
					pc+=2;
				break;

				case 0x000A:		//xFX0A waits for key press then stores in VX
					for (i =0;i<16;i++)
					{
						if (key[i]==1)
						{
							pressed = 1;
							V[X] = i;
						}
					} 
					if(pressed) pc+=2;
					else return;	
					SDL_Delay(10);

				break;

				case 0x0015:		//xFX15 sets delay time to Vx
					delay_timer = V[X];
					pc+=2;
				break;

				case 0x0018:		//xFX18 sets sound timer to VX
					sound_timer=V[X];
					pc+=2;
				break;

				case 0x001E:		//FX1E adds VX to I
					I+=V[X];
					pc+=2;
				break;

				case 0x0029:		//FX29 sets I to memory location of FONT letter in VX
					I = V[X] * 5;
					pc+=2;
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
	if(delay_timer>0) delay_timer--;
	if(sound_timer!=0)
	{
		sound_timer--;
		//PLAYYYYY SOUNNNNNND
	}
	

}

int updateKey()
{
	SDL_Event e;
	while(SDL_PollEvent( &e) !=0)
	{
		if(e.type == SDL_QUIT)
			return 1;
	}
	const Uint8* state = SDL_GetKeyboardState(NULL);
	key[0] = state[SDL_SCANCODE_X];
	key[1]= state[SDL_SCANCODE_1];
	key[2] = state[SDL_SCANCODE_2];
	key[3]= state[SDL_SCANCODE_3];
	key[4] = state[SDL_SCANCODE_Q];
	key[5]= state[SDL_SCANCODE_W];
	key[6] = state[SDL_SCANCODE_E];
	key[7]= state[SDL_SCANCODE_A];
	key[8] = state[SDL_SCANCODE_S];
	key[9]= state[SDL_SCANCODE_D];
	key[10] = state[SDL_SCANCODE_Z];
	key[11]= state[SDL_SCANCODE_C];
	key[12] = state[SDL_SCANCODE_4];
	key[13]= state[SDL_SCANCODE_R];		
	key[14] = state[SDL_SCANCODE_F];
	key[15]= state[SDL_SCANCODE_V];	
	return 0;			
}