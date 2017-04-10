/*  WebLights v1.04 by VDG
 *  This project designed for ESP8266 chip. Use it to control up to 256 LED strip on base of WS2811 chip.
 *  Copyright (c) by Denis Vidjakin, 
 *  
 *  https://github.com/Den-W/WebLights
 *  http://mysku.ru/blog/aliexpress/50036.html
 *  https://www.instructables.com/id/WebLights-Take-Control-Over-Your-New-Year-Lights/
 
 *  03.03.2017 v1.01 created by VDG
 *  05.03.2017 v1.02 Corrected bug in "BMP One" + play selected file procedure.
 *  10.04.2017 v1.04 LED RGB color order added
 *  
 *  Main module
 */
 
#include "main.h"
 
CGlobalData gD;

//-----------------------------------------------------------------------------

// Button handlers
void hBtSingle()  { gD.mKey = 0x01; }
void hBtDouble()  { gD.mKey = 0x02; }
void hBtLongSt()  { gD.mToLp = millis(); }
void hBtLongEn()  { gD.mKey = 0x03; }

void setup(void) 
{ gD.Start();
}

void loop(void) 
{ gD.Run();
}

//-----------------------------------------------------------------------------
 
void  CGlobalData::Start(void) 
{ 
  FlashRd();  // Read data from eeprom
  Serial.begin( 115200 );
  if( !mLedCount ) mLedCount = 49;
  mLeds.SetMaxPxl( mLedCount+2 );

  mBt.setClickTicks( 300 );
  mBt.attachClick( hBtSingle );
  mBt.attachDoubleClick( hBtDouble );
  mBt.attachLongPressStart( hBtLongSt );  
  mBt.attachLongPressStart( hBtLongEn );  
  
  randomSeed( analogRead(0) );
  pinMode( PIN_LED, OUTPUT );  // Set OnBoad LED as an output
  mBlinkMode = 0x03;
  BlinkerSet( 100, 1 );
  
  { bool res = SPIFFS.begin();    // Try to load FS
    Serial.print("\nWebLights v1.01. SPIFFS:" ); Serial.print( res ? "Ok":"Fail" );    
    
    for( int i=millis()+2000; i > millis(); ) { mBt.tick(); Blinker(); }
    if( gD.mToLp ) res = false; // current button signal.    

    if( res == false )  // First start probably. Try to format.
    { Serial.print("->formatting...");
      BlinkerSet( 500, 1 );
      res = SPIFFS.format();
      Serial.print( res ? "Ok":"Fail" );
    }
  }

  // Load current script
  mFl = SPIFFS.open( "/cur.scr", "r");  
  if( mFl ) 
  { mScr = mFl.readString();
    mFl.close();
  }
  // If empty - set default
  if( !mScr.length() )
    mScr = "[Tz#B?00?!zL#A?**?<z1#CR03#B=01?+zE:z1!*A@@@#CS02#A+01?+zE:zL#A-01#A?00?>z2#CR03#B=00:z2!*A#CS02@@@:zE]"
           "[**#CR03^ZM050>ZTz!31C@C]";

  mIrda.enableIRIn(); // Start the IR receiver
  mLeds.Begin();
  pinMode( PIN_LED, OUTPUT );  // Set OnBoad LED as an output  
  WebInit();
}

//-----------------------------------------------------------------------------

void  CGlobalData::Run(void) 
{ mKey = 0;
  mBt.tick();

  if ( mIrda.decode(&mIrdaRes) ) 
  { char  Tb[16];
    mIrCommand = mIrdaRes.value & 0xFFFF;
    mIrda.resume(); // Receive the next value
    sprintf( Tb, "@%04X", mIrCommand );
    if( mLedMode )
    { // BMP play mode
      if( !strcmp( Tb+1, mIr_Up ) ) mKey = 0x02;
      else if( !strcmp( Tb+1, mIr_Dn ) ) mKey = 0x01;
           else LedBmpFileChg( 0, Tb );
    } else
    { // Script mode. Find event by code
      const char *p = mScr.c_str();
      while( p && *p )
      { p = strchr( p, '(' ); // (LLxxxxcc) External events 
        if( !p++ ) break;
        if( memcmp( p+2, Tb+1, 4 ) ) continue;
        LedCallSub( p, 2 );
        break;
      }      
    }
  }

  switch( mKey )
  { case 0x01: // Button press - Down
      LedBmpFileChg( 1, 0 );
      break;
    case 0x02: // Button Double press - Up
      LedBmpFileChg( -1, 0 );
      break;
    case 0x03: // Button Long press
      { int l = millis() - mToLp;
        if( l > 10*1000 ) break; // Too long
        if( l < 4*1000 ) break; // Too short
        BlinkerSet( 100, 1 );
        Defaults();
        FlashWr();
        break;    
      }
  }

  if( mLedMode == 0 ) LedProcess(); // Script processor
  else { if( LedBmpPlay() <= 0 ) 
            LedBmpFileChg( 0, 0 );
       }

  mLeds.Show();
  mSrv.handleClient();
  if( !mWF_Mode ) mDns.processNextRequest(); 
  Blinker();
}

//-----------------------------------------------------------------------------

void  CGlobalData::BlinkerSet( int Ms, int bOn )
{ 
  mBlinkMs = millis() + Ms;
  mBlinkLed = bOn;
  digitalWrite( PIN_LED, mBlinkLed ? LOW:HIGH );  
}

void  CGlobalData::Blinker(void) 
{ static byte  BlinkOn[4] =  {0, 9, 20, 9};
  static byte  BlinkOff[4] = {0, 6, 50, 1};

  if( millis() < mBlinkMs ) return;
  
  if( mBlinkLed ) 
  { BlinkerSet( BlinkOff[mBlinkMode&0x03]*100L, 0 );
  } else BlinkerSet( BlinkOn[mBlinkMode&0x03], 1 );
}

//-----------------------------------------------------------------------------

byte crc8( byte crc, byte ch ) 
{  
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = crc ^ ch;
      crc >>= 1;
      if (mix & 0x01 ) crc ^= 0x8C;
      ch >>= 1;
  }
  return crc;
}

//-----------------------------------------------------------------------------

void CGlobalData::FlashRd(void)
{ int   i;
  
  EEPROM.begin(&_Wr-&_St+8);
  
  _Wr = 0;
  for( i=0; i < &_Wr - &_St; i++ )
  { (&_St)[i] = EEPROM.read( i );
    _Wr = crc8( _Wr, (&_St)[i] );
     yield();
  }
  
  i = EEPROM.read( i ) & 0xFF;  
  if( _Wr != i ) Defaults(); // CRC failed. Reset data.
  _Wr = 0;
  EEPROM.end();
}

//-----------------------------------------------------------------------------

void CGlobalData::FlashWr(void)
{ int   i;
  EEPROM.begin(&_Wr-&_St+8);
  _Wr = 0;
  for( i=0; i < &_Wr - &_St; i++ )
  { EEPROM.write( i, (&_St)[i] );
    _Wr = crc8( _Wr, (&_St)[i] );
    yield();
  }
  EEPROM.write( i, _Wr );
  EEPROM.commit();
  EEPROM.end();
  _Wr = 0;
}


