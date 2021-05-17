#include <TVout.h>
#include <fontALL.h>
#include <Keypad.h>
#include "Games.h"
#include "Fonts.h"

#define uint8_t u8
#define uint16_t u16

#define ROWS 4
#define COLS 4

u8 reset_counter = 0;

const byte keys[ROWS][COLS] = {
  {0x1, 0x2, 0x3, 0xC},
  {0x4, 0x5, 0x6, 0xD},
  {0x7, 0x8, 0x9, 0xE},
  {0xA, 0xFF, 0xB, 0xF}
};

const byte rowPins[ROWS] = {28, 36, 46, 52};
const byte colPins[COLS] = {30, 40, 48, 24};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

TVout tv;
u8 buf[2048];

u8 key;
u8 memory[4096];
u8 registers[16];
u8 key_inputs[16];
u8 stack_p = 0;
u16 stack[16];
u16 opcode = 0;
u16 index = 0;
u16 pc = 0x200;
u16 test;

u8 vx = 0;
u8 vy = 0;
u8 kk = 0;
u16 nnn = 0;

u8 dt = 0;
u8 st = 0;

void reset_state()
{
  for (u16 i = 0; i < 64 * 32; i++) buf[i] = 0;
  for (u8 i = 0; i < 16; i++) registers[i] = 0;
  for (u8 i = 0; i < 16; i++) key_inputs[i] = 0;
  stack_p = 0;
  for (u8 i = 0; i < 16; i++) stack[i] = 0;
  index = 0;
  pc = 0x200;
  vx = 0;
  vy = 0;
  kk = 0;
  nnn = 0;
  dt = 0;
  st = 0;
}

u8 reverse_byte(u8 b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void clear_keyboard()
{
  for (u8 i = 0; i < 16; i++)
  {
    key_inputs[i] = 0;
  }
}

void display_sprite(u8 x, u8 y, u8 n)
{
  for (u8 i = y; i < (y + n); i++)
  {
    for (u8 j = x; j < (x + 8); j++)
    {
      if (j >= 0 && j < 64 && i >= 0 && i < 32)
      {
        tv.draw_rect(j * 2, i * 2 + 16, 1, 1, buf[i * 64 + j]);
      }
    }
  }
}

void fill_screen()
{
  tv.clear_screen();
}

void execute()
{
  u8 sprite_lines, sprite_data;
  u8 screen_x, screen_y;
  
  switch(opcode & 0xf000)
  {
    case 0x0000:
      switch (kk)
      {
        case 0x00e0:
          for (u16 i = 0; i < 64 * 32; i++)
          {
              buf[i] = BLACK;
          }
          fill_screen();
          delay(5);
        break;
        case 0x0ee:
          pc = stack[--stack_p];
        break;
      }
    break;
    case 0x1000:
      pc = nnn;
    break;
    case 0x2000:
      stack[stack_p++] = pc;
      pc = nnn;
    break;
    case 0x3000:
      if (registers[vx] == kk)
      {
        pc += 2;
      }
    break;
    case 0x4000:
      if (registers[vx] != kk)
      {
        pc += 2;
      }
    break;
    case 0x5000:
      if (registers[vx] == registers[vy])
      {
        pc += 2;
      }
    break;
    case 0x6000:
      registers[vx] = kk;
    break;
    case 0x7000:
      registers[vx] += kk;
    break;
    case 0x8000:
      switch (opcode & 0x000f)
      {
        case 0x0:
          registers[vx] = registers[vy];
        break;
        case 0x1:
          registers[vx] |= registers[vy];
        break;
        case 0x2:
          registers[vx] &= registers[vy];
        break;
        case 0x3:
          registers[vx] ^= registers[vy];
        break;
        case 0x4:
          test = registers[vx] + registers[vy];
          registers[0xf] = test > 0xff;
          registers[vx] = test;
        break;
        case 0x5:
          registers[0xf] = registers[vx] > registers[vy];
          registers[vx] -= registers[vy];
        break;
        case 0x6:
          registers[0xf] = registers[vx] & 0x1;
          registers[vx] >>= 1;
        break;
        case 0x7:
          test = registers[vy] - registers[vx];
          registers[0xf] = registers[vy] > registers[vx];
          registers[vx] = test;
        break;
        case 0xe:
          registers[0xf] = ((registers[vx] & 0x80) >> 7) == 1;
          registers[vx] <<= 1;
        break;
      }
    break;
    case 0x9000:
      if (registers[vx] != registers[vy])
      {
        pc += 2;
      }
    break;
    case 0xa000:
      index = nnn;
    break;
    case 0xb000:
      pc = nnn + registers[0x0];
    break;
    case 0xc000:
      registers[vx] = random(0, 256) & kk;
    break;
    case 0xd000:
      sprite_lines = opcode & 0x000f;
      registers[0xf] = 0x0;
   
      for (screen_y = 0; screen_y < sprite_lines; screen_y++)
      {
        sprite_data = memory[index + screen_y];
        int32_t row = (registers[vy] + screen_y) % 32;

        for (screen_x = 0; screen_x < 8; screen_x++)
        {
          u8 b = (sprite_data & 0x80) >> 7;
          u8 col = (registers[vx] + screen_x) % 64;
          int32_t offset = row * 64 + col;

          if (b)
          {
            if (buf[offset] != BLACK)
            {
              buf[offset] = BLACK;
              registers[0xf] = 1;
            }
            else
            {
              buf[offset] = WHITE;
            }
          }

          sprite_data <<= 1;
        }
      }
      display_sprite(registers[vx], registers[vy], sprite_lines);
      delay(5);
    break;
    case 0xe000:
      switch (kk)
      {
        case 0x009e:
          if (key_inputs[registers[vx]])
          {
            pc += 2;
          }
        break;
        case 0x00a1:
          if (!key_inputs[registers[vx]])
          {
            pc += 2;
          }
        break;
      }
    break;
    case 0xf000:
      switch (kk)
      {
        case 0x0007:
          registers[vx] = dt;
        break;
        case 0x000a:          
          for (u8 i = 0; i < 16; i++)
          {
            if (key_inputs[i])
            {
                registers[vx] = i;
                return;
            }
          }
          pc -= 2;
        break;
        case 0x015:
          dt = registers[vx];
        break;
        case 0x018:
          st = registers[vx];
        break;
        case 0x01e:
          index += registers[vx];
        break;
        case 0x0029:
          index = 5 * registers[vx];
        break;
        case 0x0033:
          memory[index] = registers[vx] / 100;
          memory[index + 1] = (registers[vx] / 10) % 10;
          memory[index + 2] = registers[vx] % 10;
        break;
        case 0x055:
          for (u8 i = 0; i <= vx; i++)
            memory[index + i] = registers[i];
        break;
        case 0x065:
          for (u8 i = 0; i <= vx; i++)
            registers[i] = memory[index + i];
        break;
      }
    break;
  }
}

void cycle()
{
  opcode = 0;
  opcode |= memory[pc];
  opcode <<= 8;
  opcode |= memory[pc + 1];

  pc += 2;
  vx = (opcode & 0x0f00) >> 8;
  vy = (opcode & 0x00f0) >> 4;
  kk = opcode & 0x00ff;
  nnn = opcode & 0x0fff;

  key = keypad.getKey();

  if (key == 0xF) reset_counter += 1;
  if (reset_counter == 3)
  {
    reset_counter = 0;
    reset_state();
    tv.clear_screen();
    interface();
  }
  
  if (key != NO_KEY) {
    if (key == 0xFF) {
      key_inputs[0] = 1;
      key = 0;
    }
    key_inputs[key] = 1;
  }

  if (keypad.getState() == RELEASED)
  {
    clear_keyboard();    
  }

  execute();

  
  if (dt > 0)
  {
    dt--;
  }
  
  if (st > 0)
  {
    st--;
    if (st == 0)
    {
      tv.delay(100);
      tv.tone(500, 100);
    }
  }
}

void load_game(u8 game)
{
  for (u16 i = 0; i < games_sizes[game]; i++) // load game
    memory[0x200 + i] = pgm_read_byte(games_pointers[game] + i);
}

void interface()
{
  for (u8 i = 0; i < GAMES_COUNT; i++)
  {
    tv.print(i + 1);
    tv.print(". ");
    tv.println(games_names[i]);
  }
  
  while (1)
  {    
    key = keypad.getKey();

    if (key >= 1 && key <= 10) {
      tv.println("suka");
      load_game(key - 1);
      tv.clear_screen();
      break;
    }
  }
}

void setup() {
  tv.begin(NTSC,128,96);
  tv.select_font(font8x8);
    
  for (u8 i = 0; i < 80; i++) // load fonts
    memory[i] = pgm_read_byte(fonts + i);
  
  randomSeed(analogRead(0));

  tv.println(44, 32, "CHIP-8");
  delay(3000);
  tv.clear_screen();
  tv.select_font(font6x8);
  
  interface();
}

void loop() {  
  cycle();
}
