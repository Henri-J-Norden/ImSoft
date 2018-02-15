#include "softraster.h"
#include "imgui.h"
//#define ESP32
#include "SPI.h"
#include <TFT_22_ILI9225.h>

//#define PRINT_ATLAS
#define SMALL_ATLAS
#include "fontAtlas.h"

const uint8_t HSPICLK = 14;
const uint8_t HSPICS0 = 15;
const uint8_t HSPIMISO = 12;
const uint8_t HSPIMOSI = 13;
const uint8_t HSPIHD = 4;
const uint8_t HSPIWP = 2;

const uint8_t ADC2CH1 = 0; 
const uint8_t BOOTBTN = ADC2CH1;
const uint8_t LEDPIN = 16;

const uint8_t TFTLED = 25;
const uint8_t TFTRST = 26;
const uint8_t TFTRS = 27;
const uint8_t TFTCS = HSPICS0; 
const uint8_t TFTCLK = HSPICLK; 
const uint8_t TFTSDI = HSPIMOSI; 

#define TFTX 220
#define TFTY 176

texture_t screenBuffer;

TFT_22_ILI9225 tft = TFT_22_ILI9225(TFTRST, TFTRS, TFTCS, TFTSDI, TFTCLK, TFTLED, 128);

void updateScreen()
{
  tft.drawBitmap(screenBuffer.tex16.col, screenBuffer.tex16.w, screenBuffer.tex16.h);
}

unsigned long drawTime;
unsigned long rasterTime;

void renderFunc(ImDrawData* drawData)
{
  screen_t screen;
  screen.w = TFTX;
  screen.h = TFTY;
  screen.buffer = &screenBuffer;

  rasterTime = millis();
  Softraster::renderDrawLists(drawData, &screen);
  rasterTime = millis() - rasterTime;

  drawTime = millis();
  updateScreen();
  drawTime = millis() - drawTime;
}

void setup()
{
  Serial.begin(115200);

  //SPI.setClockDivider(SPI_CLOCK_DIV2);
  //SPI.begin();
  
  tft.begin();
  tft.setFont(Terminal6x8);
  tft.setOrientation(3);
  digitalWrite(TFTLED, HIGH);
    
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize.x = TFTX;
  io.DisplaySize.y = TFTY;
  io.RenderDrawListsFn = renderFunc;
  
  #ifdef SMALL_ATLAS
  io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight | ImFontAtlasFlags_NoMouseCursors;
  #endif

  uint8_t* pixels;
  int width, height;
  io.Fonts->GetTexDataAsAlpha8(&pixels, &(width), &(height));
  fontAtlas.tex8.w = width;
  fontAtlas.tex8.h = height;

  #ifdef PRINT_ATLAS
  Serial.print("{");
  for(int x = 0; x < fontAtlas.tex8.w; x++)
  {
    for(int y = 0; y < fontAtlas.tex8.h; y++)
    {
      Serial.print("0x");
      Serial.print(pixels[x + y*fontAtlas.tex8.w], HEX);
      Serial.print(", ");
    }
    Serial.println("");
  }
  Serial.print("}");
  #endif

  ImGui::MemFree(pixels);

  //delay(1000);

  fontAtlas.pre_init(COLOR8);
  for(size_t i = 0; i < fontAtlas.tex8.w; i++)
  {
    fontAtlas.ctex8.col[i] = &(fontAtlasPixels[i * fontAtlas.tex8.h]);
  }

  screenBuffer.init(TFTX, TFTY, COLOR16);
  
  io.Fonts->TexID = (void*)&fontAtlas;
}

float f = 0.0f;
float time = 0.0f;

void loop()
{
  ImGuiIO& io = ImGui::GetIO();
  io.DeltaTime = 1.0f/60.0f;
  //io.MousePos = mouse_pos;
  //io.MouseDown[0] = mouse_button_0;
  //io.MouseDown[1] = mouse_button_1;
  //[0.0f - 1.0f]
  io.NavFlags |= ImGuiNavFlags_EnableGamepad;
  io.NavInputs[ImGuiNavInput_Activate] = 0.0f;    // activate / open / toggle / tweak value       // e.g. Circle (PS4), A (Xbox), B (Switch), Space (Keyboard)
  io.NavInputs[ImGuiNavInput_Cancel] = 0.0f;      // cancel / close / exit                        // e.g. Cross  (PS4), B (Xbox), A (Switch), Escape (Keyboard)
  io.NavInputs[ImGuiNavInput_Input] = 0.0f;       // text input / on-screen keyboard              // e.g. Triang.(PS4), Y (Xbox), X (Switch), Return (Keyboard)
  io.NavInputs[ImGuiNavInput_Menu] = 0.0f;        // tap: toggle menu / hold: focus, move, resize // e.g. Square (PS4), X (Xbox), Y (Switch), Alt (Keyboard)
  io.NavInputs[ImGuiNavInput_DpadLeft] = 0.0f;    // move / tweak / resize window (w/ PadMenu)    // e.g. D-pad Left/Right/Up/Down (Gamepads), Arrow keys (Keyboard)
  io.NavInputs[ImGuiNavInput_DpadRight] = 0.0f;
  io.NavInputs[ImGuiNavInput_DpadUp] = 0.0f;
  io.NavInputs[ImGuiNavInput_DpadDown] = 0.0f;
  io.NavInputs[ImGuiNavInput_TweakSlow] = 0.0f;   // slower tweaks                                // e.g. L1 or L2 (PS4), LB or LT (Xbox), L or ZL (Switch)
  io.NavInputs[ImGuiNavInput_TweakFast] = 0.0f;   // faster tweaks                                // e.g. R1 or R2 (PS4), RB or RT (Xbox), R or ZL (Switch)

  ImGui::NewFrame();
  
  f += 0.05;
  if(f > 1.0f) f = 0.0f;
  
  float deltaTime = (millis()/1000.0f) - time;
  time += deltaTime;
  float temp = drawTime / 1000.0f;
  float temp2 = rasterTime / 1000.0f;
  
  deltaTime -= (temp + temp2);
  
  ImGui::SliderFloat("float", &deltaTime, 0.0f, 10.0f);
  
  ImGui::SliderFloat("float1", &temp, 0.0f, 10.0f);

  ImGui::SliderFloat("float2", &temp2, 0.0f, 10.0f);
  
  ImGui::SliderFloat("float3", &f, 0.0f, 1.0f);

  screenBuffer.clear();
  
  ImGui::Render();
}

