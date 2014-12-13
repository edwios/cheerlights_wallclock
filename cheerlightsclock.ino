// This #include statement was automatically added by the Spark IDE.
#include "neopixel/neopixel.h"

#define version 114

/****************************************************************************
*   Cheerlights LED strip using 1m x 60  WB2813b LED strip                  *
*   A local OLED display is used to show the current color                  *
****************************************************************************/
extern char* itoa(int a, char* buffer, unsigned char radix);

// Time for each Deep sleep (in ms)
#define PERIOD 300000
// Time between each rainbow (in ms)
#define RAINBOWTIME 30000
// How long shall we show the rainbow (in ms)
#define RAINBOWLENGTH 5000
// Set UPDATE2CLOUD to true if you want the measurement sent to the cloud
#define UPDATE2CLOUD false 
// Thingspeak API WRITE key
#define THINGSPEAK_API_WRITEKEY "THINGSPEAK_API_WRITEKEY"

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D4
#define PIXEL_COUNT 60
#define PIXEL_TYPE WS2812B

#define BLACK 0
#define RED strip.Color(255,0,0)
#define GREEN strip.Color(0,255,0)
#define BLUE strip.Color(0,0,255)
#define CYAN strip.Color(0,255,255)
#define WHITE strip.Color(255,255,255)
#define WARMWHITE strip.Color(0xFD,0xF5,0xE6)
#define PURPLE strip.Color(128,0,128)
#define MAGENTA strip.Color(255,0,255)
#define YELLOW strip.Color(255,255,0)
#define ORANGE strip.Color(0xFF, 0xA5, 0)
#define PINK strip.Color(0xFF, 0xC0, 0xCB)
#define OLDLACE strip.Color(0xFD,0xF5,0xE6)

#define CLBLACK 0
#define CLRED 1
#define CLGREEN 2
#define CLBLUE 3
#define CLCYAN 4
#define CLWHITE 5
#define CLWARMWHITE 6
#define CLPURPLE 7
#define CLMAGENTA 8
#define CLYELLOW 9
#define CLORANGE 10
#define CLPINK 11
#define CLOLDLACE 12

// Cheerlights Clock Face
#define CLOCKFACE0024 strip.Color(0X80,0x80,0x10)
#define CLOCKFACEQUADS strip.Color(0x40,0x40,0)

#define SOUNDSENSORPIN D3

/* ======================= LED Strip prototypes =============================== */

void colorAll(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
uint32_t Wheel(byte WheelPos);

/* ======================= Other definables ============ */

// Pin out defs
int led = D7;


// Thingspeak.com API
TCPClient client;
const char * WRITEKEY = THINGSPEAK_API_WRITEKEY;
const char * serverName = "api.thingspeak.com";
IPAddress server = {184,106,153,149};

// 1.3" 12864 OLED with SH1106 and Simplified Chinese IC for Spark Core
int Rom_CS = A2;
unsigned long  fontaddr=0;
char dispCS[32];

// LED strip demo params
int demo = 2;
int speed = 20;
long lastread = 0;

// Cheerlights
uint32_t CheerLightsColor=0;
int CheerLightsENum=CLBLACK;
bool colorChanged=false;

// Cheerlights clock
int i_hour = 0;
int i_minute = 0;
int i_second = 0;
int offset = 0;

// Clapping
int SoundSensor = SOUNDSENSORPIN;
int debounce = 0;
bool showTime = false;

/****************************************************************************
*****************************************************************************
****************************   OLED Driver  *********************************
*****************************************************************************
****************************************************************************/


/*****************************************************************************
 Funtion    :   OLED_WrtData
 Description:   Write Data to OLED
 Input      :   byte8 ucCmd  
 Output     :   NONE
 Return     :   NONE
*****************************************************************************/
void transfer_data_lcd(byte ucData)
{
   Wire.beginTransmission(0x78 >> 1);     
   Wire.write(0x40);      //write data
   Wire.write(ucData);
   Wire.endTransmission();
}

/*****************************************************************************
 Funtion    :   OLED_WrCmd
 Description:   Write Command to OLED
 Input      :   byte8 ucCmd  
 Output     :   NONE
 Return     :   NONE
*****************************************************************************/
void transfer_command_lcd(byte ucCmd)
{
   Wire.beginTransmission(0x78 >> 1);            //Slave address,SA0=0
   Wire.write(0x00);      //write command
   Wire.write(ucCmd); 
   Wire.endTransmission();
}


/* OLED Initialization */
void initial_lcd()
{
  digitalWrite(Rom_CS, HIGH);
  Wire.begin();
  delay(20);        
  transfer_command_lcd(0xAE);   //display off
  transfer_command_lcd(0x20); //Set Memory Addressing Mode  
  transfer_command_lcd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
  transfer_command_lcd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7
  transfer_command_lcd(0xc8); //Set COM Output Scan Direction
  transfer_command_lcd(0x00);//---set low column address
  transfer_command_lcd(0x10);//---set high column address
  transfer_command_lcd(0x40);//--set start line address
  transfer_command_lcd(0x81);//--set contrast control register
  transfer_command_lcd(0x7f);
  transfer_command_lcd(0xa1);//--set segment re-map 0 to 127
  transfer_command_lcd(0xa6);//--set normal display
  transfer_command_lcd(0xa8);//--set multiplex ratio(1 to 64)
  transfer_command_lcd(0x3F);//
  transfer_command_lcd(0xa4);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
  transfer_command_lcd(0xd3);//-set display offset
  transfer_command_lcd(0x00);//-not offset
  transfer_command_lcd(0xd5);//--set display clock divide ratio/oscillator frequency
  transfer_command_lcd(0xf0);//--set divide ratio
  transfer_command_lcd(0xd9);//--set pre-charge period
  transfer_command_lcd(0x22); //
  transfer_command_lcd(0xda);//--set com pins hardware configuration
  transfer_command_lcd(0x12);
  transfer_command_lcd(0xdb);//--set vcomh
  transfer_command_lcd(0x20);//0x20,0.77xVcc
  transfer_command_lcd(0x8d);//--set DC-DC enable
  transfer_command_lcd(0x14);//
  transfer_command_lcd(0xaf);//--turn on oled panel 

}

void lcd_address(byte page,byte column)
{

  transfer_command_lcd(0xb0 + column);   /* Page address */
  transfer_command_lcd((((page + 1) & 0xf0) >> 4) | 0x10);  /* 4 bit MSB */
  transfer_command_lcd(((page + 1) & 0x0f) | 0x00); /* 4 bit LSB */ 
}

void clear_screen()
{
  unsigned char i,j;
  digitalWrite(Rom_CS, HIGH); 
  for(i=0;i<8;i++)
  {
    transfer_command_lcd(0xb0 + i);
    transfer_command_lcd(0x00);
    transfer_command_lcd(0x10);
    for(j=0;j<132;j++)
    {
        transfer_data_lcd(0x00);
    }
  }

}

void display_128x64(byte *dp)
{
  unsigned int i,j;
  for(j=0;j<8;j++)
  {
    lcd_address(0,j);
    for (i=0;i<132;i++)
    { 
      if(i>=2&&i<130)
      {
          // Write data to OLED, increase address by 1 after each byte written
        transfer_data_lcd(*dp);
        dp++;
      }
    }

  }

}



void display_graphic_16x16(unsigned int page,unsigned int column,byte *dp)
{
  unsigned int i,j;

  digitalWrite(Rom_CS, HIGH);   
  for(j=2;j>0;j--)
  {
    lcd_address(column,page);
    for (i=0;i<16;i++)
    { 
      transfer_data_lcd(*dp);
      dp++;
    }
    page++;
  }
}


void display_graphic_8x16(unsigned int page,byte column,byte *dp)
{
  unsigned int i,j;
  
  for(j=2;j>0;j--)
  {
    lcd_address(column,page);
    for (i=0;i<8;i++)
    { 
      // Write data to OLED, increase address by 1 after each byte written
      transfer_data_lcd(*dp);
      dp++;
    }
    page++;
  }

}


/*
    Display a 5x7 dot matrix, ASCII or a 5x7 custom font, glyph, etc.
*/
    
void display_graphic_5x7(unsigned int page,byte column,byte *dp)
{
  unsigned int col_cnt;
  byte page_address;
  byte column_address_L,column_address_H;
  page_address = 0xb0 + page - 1;// 
  
  
  
  column_address_L =(column&0x0f);  // -1
  column_address_H =((column>>4)&0x0f)+0x10;
  
  transfer_command_lcd(page_address);     /*Set Page Address*/
  transfer_command_lcd(column_address_H); /*Set MSB of column Address*/
  transfer_command_lcd(column_address_L); /*Set LSB of column Address*/
  
  for (col_cnt=0;col_cnt<6;col_cnt++)
  { 
    transfer_data_lcd(*dp);
    dp++;
  }
}

/**** Send command to Character ROM ***/
void send_command_to_ROM( byte datu )
{
  SPI.transfer(datu);
}

/**** Read a byte from the Character ROM ***/
byte get_data_from_ROM( )
{
  byte ret_data=0;
  ret_data = SPI.transfer(255);
  return(ret_data);
}


/* 
*     Read continuously from ROM DataLen's bytes and 
*     put them into pointer pointed to by pBuff
*/

void get_n_bytes_data_from_ROM(byte addrHigh,byte addrMid,byte addrLow,byte *pBuff,byte DataLen )
{
  byte i;
  digitalWrite(Rom_CS, LOW);
  delayMicroseconds(100);
  send_command_to_ROM(0x03);
  send_command_to_ROM(addrHigh);
  send_command_to_ROM(addrMid);
  send_command_to_ROM(addrLow);

  for(i = 0; i < DataLen; i++ ) {
       *(pBuff+i) =get_data_from_ROM();
  }
  digitalWrite(Rom_CS, HIGH);
}


/******************************************************************/

void display_string_5x7(byte y,byte x,const char *text)
{
  unsigned char i= 0;
  unsigned char addrHigh,addrMid,addrLow ;
  while((text[i]>0x00))
  {
    
    if((text[i]>=0x20) &&(text[i]<=0x7e)) 
    {           
      unsigned char fontbuf[8];     
      fontaddr = (text[i]- 0x20);
      fontaddr = (unsigned long)(fontaddr*8);
      fontaddr = (unsigned long)(fontaddr+0x3bfc0);     
      addrHigh = (fontaddr&0xff0000)>>16;
      addrMid = (fontaddr&0xff00)>>8;
      addrLow = fontaddr&0xff;

      get_n_bytes_data_from_ROM(addrHigh,addrMid,addrLow,fontbuf,8);/*取8个字节的数据，存到"fontbuf[32]"*/
      
      display_graphic_5x7(y,x+1,fontbuf);/*显示5x7的ASCII字到LCD上，y为页地址，x为列地址，fontbuf[]为数据*/
      i+=1;
      x+=6;
    }
    else
    i++;  
  }
  
}

void showClock(byte y)
{
    noInterrupts();
        String s = Time.timeStr().substring(4,19);
        char v[17];
        s.toCharArray(v, 16);
        sprintf(dispCS, "%s    ", v);
        display_string_5x7(y,20,dispCS);
    interrupts();
}

void refreshClock()
{
    showClock(1);
}


/****************************************************************************
*****************************************************************************
****************************  Data Upload   *********************************
*****************************************************************************
****************************************************************************/

void sendToThingSpeak(const char * key, String mesg)
{
    client.stop();
    String outMesg = String("field1="+mesg);
    RGB.control(true);
    RGB.color(0,255,0);
    if (client.connect(server, 80)) {
        client.print("POST /update");
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(serverName);
        client.println("User-Agent: Spark");
        client.println("Connection: close");
        client.print("X-THINGSPEAKAPIKEY: ");
        client.println(key);
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-length: ");
        client.println(outMesg.length());
        client.println();
        client.print(outMesg);
        client.flush();
        RGB.control(false);
    } 
    else{
        RGB.color(255,0,0);
    }
}


int myVersion(String command)
{
    return version;
}

/****************************************************************************
*****************************************************************************
*************************  LED Strip Driver *********************************
*****************************************************************************
****************************************************************************/


Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);


int demos(String cmd) 
{
    if (cmd.equals("1")) {
        demo = 1;
    }
    if (cmd.equals("2")) {
        demo = 2;
    }
    if (cmd.equals("3")) {
        demo = 3;
    }
    if (cmd.equals("4")) {
        demo = 4;
    }
    if (cmd.equals("5")) {
        demo = 5;
    }
    if (cmd.equals("6")) {
        demo = 6;
    }
    if (cmd.substring(0,1).equals("S")) {
        speed = cmd.substring(1).toInt();
    }
    return 0;
}

int cheerlights(String cmd) 
{
    uint32_t c=0;
    
    if (cmd.equalsIgnoreCase("red")) {
        // Red
        c = RED;
        CheerLightsENum = CLRED;
    } else if (cmd.equalsIgnoreCase("green")) {
        // Green
        c = GREEN;
        CheerLightsENum = CLGREEN;
    } else if (cmd.equalsIgnoreCase("blue")) {
        // Blue
        c = BLUE;
        CheerLightsENum = CLBLUE;
    } else if (cmd.equalsIgnoreCase("cyan")) {
        // cyan
        c = CYAN;
        CheerLightsENum = CLCYAN;
    } else if (cmd.equalsIgnoreCase("white")) {
        // white
        c = WHITE;
        CheerLightsENum = CLWHITE;
    } else if (cmd.equalsIgnoreCase("warmwhite")) {
        // warmwhite
        c = WARMWHITE;
        CheerLightsENum = CLWARMWHITE;
    } else if (cmd.equalsIgnoreCase("purple")) {
        // purple
        c = PURPLE;
        CheerLightsENum = CLPURPLE;
    } else if (cmd.equalsIgnoreCase("magenta")) {
        // magenta
        c = MAGENTA;
        CheerLightsENum = CLMAGENTA;
    } else if (cmd.equalsIgnoreCase("yellow")) {
        // yellow
        c = YELLOW;
        CheerLightsENum = CLYELLOW;
    } else if (cmd.equalsIgnoreCase("orange")) {
        // orange
        c = ORANGE;
        CheerLightsENum = CLORANGE;
    } else if (cmd.equalsIgnoreCase("pink")) {
        // pink
        c = PINK;
        CheerLightsENum = CLPINK;
    } else if (cmd.equalsIgnoreCase("oldlace")) {
        // oldlace
        c = OLDLACE;
        CheerLightsENum = CLOLDLACE;
    } else if (cmd.equalsIgnoreCase("black")) {
        // oldlace
        c = BLACK;
        CheerLightsENum = CLBLACK;
    }
    if (c >= 0) { 
        CheerLightsColor = c;
        colorChanged = true;
        lastread = millis(); // reset rainbow counter to prevent rainbox from showing now
    }
    cmd.toUpperCase();
    cmd.toCharArray(dispCS, 16);
    display_string_5x7(4,1,"                  ");
    display_string_5x7(4,1,dispCS);
    return 1;
}



/* ======================= extra-examples.cpp ======================== */


// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
//               note: if not specified, D2 is selected for you.
// Parameter 3 = pixel type [ WS2812, WS2812B, WS2811, TM1803 ]
//               note: if not specified, WS2812B is selected for you.
//               note: RGB order is automatically applied to WS2811,
//                     WS2812/WS2812B/TM1803 is GRB order.
//
// 800 KHz bitstream 800 KHz bitstream (most NeoPixel products ...
//                         ... WS2812 (6-pin part)/WS2812B (4-pin part) )
//
// 400 KHz bitstream (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//                   (Radio Shack Tri-Color LED Strip - TM1803 driver
//                    NOTE: RS Tri-Color LED's are grouped in sets of 3)


// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait) {
  uint16_t i;
  
  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}

// Fill the dots one after the other with a color, wait (ms) after each one
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
        if (i != i_hour && i != i_minute && i != i_second)
          strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout, then wait (ms)
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 1 cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
        if (i != i_hour && i != i_minute && i != i_second)
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

int setOffset(String cmd) {
    long ofs = cmd.toInt();
    if (ofs >=0 && ofs < 60) {
        offset = (int)ofs;
    }
    EEPROM.write(1, (byte)offset);
    return offset;
}

void showClock(uint32_t hc, uint32_t mc, uint32_t sc, uint32_t dc, bool keep) {
    int minute = Time.minute();
    i_second = (Time.second() + offset) % 60;
    i_minute = (minute + offset) % 60;
    i_hour = ((Time.hourFormat12() * 5) + (minute * 5)/60 + offset )% 60 ;
    int c = 0;
    int j = 0;
    
    for(int i=0; i< strip.numPixels(); i++) {
        j = dc; // set default to default color
        // Draw clock face
        if (i == offset || i == ((59 + offset)%60) || i == ((offset + 1)%60))
            j = CLOCKFACE0024;
        else if (i == ((15+offset)%60) || i == ((30+offset)%60) || i == ((45+offset)%60))
            j = CLOCKFACEQUADS;
        if (i == i_hour)    
            c = hc;
        else if (i == i_minute)
            c = mc;
        else if (i == i_second)
            c = sc;
        else {
            if (keep)
                c = strip.getPixelColor(i);
            else
                c = j;
        }
        strip.setPixelColor(i, c);
    }
    strip.show();    
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void clapHandler() {
    // Handle sound input
    // Must not have any processing here as it will cause the Core to hang
//    detachInterrupt(SoundSensor);
    debounce++;
    if (debounce == 1) {
        showTime = true;
    }
}


/****************************************************************************
*****************************************************************************
**************************  Initialization  *********************************
*****************************************************************************
****************************************************************************/

// App init
// Return 0 on error

int init() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    return 1;
}


// System init
void setup()
{
    Time.zone(8);
  // Register a Spark variable here
    Spark.function("version", myVersion);
    Spark.function("cheerlights", cheerlights);
    Spark.function("setoffset", setOffset);
    Spark.variable("clcolor", &CheerLightsENum, INT);

    pinMode(led, OUTPUT);
    pinMode(SoundSensor, INPUT);

    SPI.begin();
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE3);
    SPI.setClockDivider(SPI_CLOCK_DIV8);
    digitalWrite(Rom_CS, HIGH);

    initial_lcd();  
    clear_screen();    //clear all dots
    digitalWrite(led, LOW);
    display_string_5x7(4,1,"(c)2014 ioStation");
    sprintf(dispCS, "Version: %d", version);
    display_string_5x7(6,1,dispCS);
    delay(1000);
    clear_screen();    //clear all dots
    sprintf(dispCS, "v%d", version);
    display_string_5x7(7,1,dispCS);
    if (!init()) {
        display_string_5x7(7,1,"Error: init()");
    }
    lastread = millis();
    colorChanged = true;
    offset = (int)EEPROM.read(1);
    showTime = false;
//    attachInterrupt(SoundSensor, clapHandler, RISING);
}

/****************************************************************************
*****************************************************************************
****************************  Main  Loop  ***********************************
*****************************************************************************
****************************************************************************/

void loop()
{
    refreshClock();
    showClock(RED, BLUE, GREEN, CheerLightsColor, false);
    if ((i_second + offset) % 60 == 0) {
        rainbow(16);
    } else {
        delay(200);
    }
    if (showTime) {
        showTime = false;
        rainbowCycle(5);
    }
    if (i_second % 5 == 0) {
        debounce = 0;
//        attachInterrupt(SoundSensor, clapHandler, RISING);
    }
}





