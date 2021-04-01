#include <TVout.h>
#include <fontALL.h>
#include <Vector.h>

#define DEBUG true

#define uint8_t u8
#define uint16_t u16

#define REGISTERS 16
#define MEMORY 4096
//#define DISPLAY 2048
#define STACK 16

TVout TV;

u8 maze[38] = {
  0x60, 0x00, 0x61, 0x00, 0xa2, 0x22, 0xc2, 0x01, 0x32, 0x01, 0xa2, 0x1e, 0xd0, 0x14, 0x70, 0x04,
  0x30, 0x40, 0x12, 0x04, 0x60, 0x00, 0x71, 0x04, 0x31, 0x20, 0x12, 0x04, 0x12, 0x1c, 0x80, 0x40,
  0x20, 0x10, 0x20, 0x40, 0x80, 0x10
};

struct Chip8
{
  u8 memory[MEMORY];
  u8 registers[REGISTERS];
  char displayBuffer[DISPLAY];
  u16 stack[STACK];
  u16 opcode = 0;
  u16 index = 0;
  u16 pc = 0x200;

  bool EXIT = false;
  bool shouldDraw = DEBUG;

  void loadGame(uint8_t* src)
  {
    memset(this->memory, 0, MEMORY);
    memset(this->registers, 0, REGISTERS);
    memset(this->displayBuffer, WHITE, 2048);
    memset(this->stack, 0, STACK);
    memcpy(src, &this->memory[0x200], sizeof(src));
  }

  void draw()
  {
    for (u8 i = 0; i < 64; i++)
    {
      for (u8 j = 0; j < 32; j++)
      {
        TV.set_pixel(i, j, this->displayBuffer[j * 64 + i]);
      }
    }
  }

  void cycle()
  {
    
  }
};

void setup() {
  TV.begin(NTSC, 128, 32);
  TV.select_font(font6x8);
  Serial.begin(9600);
}

Chip8* chip8;
void loop() {
  chip8->loadGame(maze);
  
//  chip8->draw();
//  chip8->displayBuffer[0] = WHITE;
  chip8->draw();
  for (u8 i = 0; i < 64; i++)
  {
    for (u8 j = 0; j < 32; j++)
    {
      Serial.print(chip8->displayBuffer[j * 64 + i]);
      Serial.print(" ");
    }
    Serial.println("\n");
  }
  while(1){};

  delete chip8;
  chip8 = NULL;
}
