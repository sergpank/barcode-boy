/*
  Hello, World! example
  June 11, 2015
  Copyright (C) 2015 David Martinez
  All rights reserved.
  This code is the most basic barebones code for writing a program for Arduboy.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/

#include <Arduboy2.h>

// make an instance of arduboy used for many functions
Arduboy2 boy;

byte prevButtonsState = 0x01;

const byte BAR_LENGTH = 52;

const byte CARDS_NR = 7; // total number of cards
const char* CARDS[][2] = {{"KOPEIKA", "2041892343958"},
  {"POLIMED",   "2100006671734"},
  {"FISHKA",    "1000331133919"},
  {"EPICENTRK", "2714705924353"},
  {"AMIC",      "2504807470984"},
  {"METRO",     "0000000000000"},
  {"LEROY_M",   "0000000000000"}
};
const byte CARDS_ON_SCREEN = 4;

const char MENU = 'm';
const char CARD = 'c';
char mode = MENU;

byte firstCardIndex = 0; // show cards starting from this index
byte curCardNr = 0; // number of current card

// This function runs once in your game.
// use it for anything that needs to be set only once in your game.
void setup()
{
  // initiate arduboy instance
  boy.boot();

  // here we set the framerate to 1, we do not need to run at
  // default 60 and it saves us battery life
  boy.setFrameRate(15);
  boy.setTextSize(2);

  Serial.begin(9600); // without that crap image on display does not get updated!
  //    while (!Serial)
  //    {
  //      ; // wait for serial port to connect. Needed for native USB port only
  //    }
  //
  //    for (byte i = 0; i < CARDS_NR; i++)
  //    {
  //      Serial.print(CARDS[i][0]);
  //      Serial.print(" :: ");
  //      Serial.println(CARDS[i][1]);
  //    }
}


// our main game loop, this runs once every cycle/frame.
// this is where our game logic goes.
void loop()
{
  // pause render until it's time for the next frame
  if (!(boy.nextFrame()))
  {
    return;
  }

  byte buttonsState = boy.buttonsState();

  Serial.println(buttonsState, BIN);

  // first we clear our screen to white
  if (buttonsState != prevButtonsState)
  {
    //boy.clear();
    
    if (mode == MENU)
    {
      boy.setTextSize(2);
      boy.clear();
      drawMenu();
    }
    else
    {
      boy.setTextSize(1);
      boy.clear();
      drawCard();
      if (buttonsState == A_BUTTON) buttonsState = 0;
    }
    
    prevButtonsState = buttonsState;
    
    boy.display();
  }
}

void drawCard()
{
  if (boy.pressed(B_BUTTON))
  {
    mode = MENU;
    return;
  }
  drawBarCode(CARDS[curCardNr][1]);
}

void drawMenu()
{
  if (boy.pressed(A_BUTTON))
  {
    mode = CARD;
    return;
  }

  if (boy.pressed(DOWN_BUTTON) && (curCardNr < CARDS_NR - 1)) {
    //while(boy.pressed(DOWN_BUTTON)) {};
    curCardNr++;
    if (curCardNr >= firstCardIndex + CARDS_ON_SCREEN) {
      firstCardIndex++;
    }
  }

  if (boy.pressed(UP_BUTTON) && curCardNr > 0) {
    //while (boy.pressed(UP_BUTTON)) {};
    curCardNr--;
    if (curCardNr < firstCardIndex) {
      firstCardIndex--;
    }
  }

  //drawBarCode("2041892343958");
  for (int i = firstCardIndex; i < firstCardIndex + CARDS_NR - 1; i++)
  {
    boy.print(i + 1);
    if (i == curCardNr) {
      boy.print(">");
    } else {
      boy.print(" ");
    }
    boy.println(CARDS[i][0]);
  }
}

void drawBarCode(String code)
{
  drawBars(code);
  drawNumbers(code);
}

void drawNumbers(String code)
{
  int cursorX = 9;
  int cursorY = 55;
  boy.setCursor(cursorX, cursorY);
  boy.print(code[0]);

  boy.setCursor(cursorX += (5 + 7), cursorY);
  for (int i = 1; i <= 6; i++)
  {
    boy.print(code[i]);
    boy.setCursor(cursorX += 7, cursorY);
  }
  boy.setCursor(cursorX += 5, cursorY);
  for (int i = 7; i <= 12; i++)
  {
    boy.print(code[i]);
    boy.setCursor(cursorX += 7, cursorY);
  }
}

void drawBars(String code)
{
  String encoding = getEncoding(code);

  byte barX = 17;
  for (byte i = 0; i <= 13; i++)
  {
    if (i == 0 || i == 13)
    {
      drawBorderMarker(barX, i == 0);
      barX += 3;
      // first digit of barcode is checsum as is not encoded
      continue;
    }

    if (i == 7)
    {
      //      Serial.println("drawing center");
      drawCenterMarker(barX);
      barX += 5;
    }

    byte digit = toInt(code.charAt(i));
    byte digitCode = getDigitCode(digit, encoding, i - 1, barX);
    encodeDigit(digitCode, barX);
    barX += 7;
  }
}

byte getDigitCode(byte digit, String encoding, byte index, byte barX)
{
  // First Digit       0          1          2          3          4          5          6          7          8          9
  byte lCode[]     = { 0b0001101, 0b0011001, 0b0010011, 0b0111101, 0b0100011, 0b0110001, 0b0101111, 0b0111011, 0b0110111, 0b0001011 };
  byte gCode[]     = { 0b0100111, 0b0110011, 0b0011011, 0b0100001, 0b0011101, 0b0111001, 0b0000101, 0b0010001, 0b0001001, 0b0010111 };
  byte rCode[]     = { 0b1110010, 0b1100110, 0b1101100, 0b1000010, 0b1011100, 0b1001110, 0b1010000, 0b1000100, 0b1001000, 0b1110100 };

  if (index < 6)
  {
    switch (encoding.charAt(index))
    {
      case 'L' : return lCode[digit];
      case 'G' : return gCode[digit];
    }
  }
  else
  {
    return rCode[digit];
  }
}

void encodeDigit(byte digitCode, byte barX)
{
  //  Serial.print("Bits = ");
  byte mask = 0b01000000;
  for (byte i = 0; i < 7; i++)
  {
    bool b = digitCode & (mask >> i);
    boy.drawLine(barX + i, 1, barX + i, BAR_LENGTH, b ? BLACK : WHITE);
  }
}

String getEncoding(String code)
{
  int firstDigit = toInt(code.charAt(0));
  switch (firstDigit)
  {
    case 0 : return "LLLLLL";
    case 1 : return "LLGLGG";
    case 2 : return "LLGGLG";
    case 3 : return "LLGGGL";
    case 4 : return "LGLLGG";
    case 5 : return "LGGLLG";
    case 6 : return "LGGGLL";
    case 7 : return "LGLGLG";
    case 8 : return "LGLGGL";
    case 9 : return "LGGLGL";
  }
}

void drawBorderMarker(byte barX, boolean isFirst)
{
  if (isFirst)
    for (byte i = 1; i <= 10; i++)
      boy.drawLine(barX - i, 1, barX - i, BAR_LENGTH, WHITE);

  boy.drawLine(barX, 1, barX, 64, BLACK); barX++;
  boy.drawLine(barX, 1, barX, 64, WHITE); barX++;
  boy.drawLine(barX, 1, barX, 64, BLACK); barX;

  if (!isFirst)
    for (byte i = 1; i <= 10                                                                                                                                                                                                    ; i++)
      boy.drawLine(barX + i, 1, barX + i, BAR_LENGTH, WHITE);
}

void drawCenterMarker(byte barX)
{
  boy.drawLine(barX, 1, barX, 64, WHITE);
  boy.drawLine(barX + 1, 1, barX + 1, 64, BLACK);
  boy.drawLine(barX + 2, 1, barX + 2, 64, WHITE);
  boy.drawLine(barX + 3, 1, barX + 3, 64, BLACK);
  boy.drawLine(barX + 4, 1, barX + 4, 64, WHITE);
}

int toInt(char c)
{
  return c - '0';
}
