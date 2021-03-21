#include <TVout.h>
#include <fontALL.h>

TVout TV;

void setup() {
  TV.begin(NTSC, 128, 64);
  TV.select_font(font6x8);
  TV.println("CHIP-8");
}

void loop() {
  
}
