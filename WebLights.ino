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
  
  randomSeed( analogRead(0) );
  pinMode( 2, OUTPUT );  // Set OnBoad LED as an output
  
  { bool res = SPIFFS.begin();    // Try to load FS
    Serial.print("\nWebLights v1.01. SPIFFS:" ); Serial.print( res ? "Ok":"Fail" );
    
    delay( 5 );
    if( digitalRead( PIN_BUTTON ) == false ) res = false; // current button signal.    

    if( res == false )  // First start probably. Try to format.
    { Serial.print("->formatting...");
      res = SPIFFS.format();
      Serial.print( res ? "Ok":"Fail" );
    }
  }

  mBt.setClickTicks( 300 );
  mBt.attachClick( hBtSingle );
  mBt.attachDoubleClick( hBtDouble );
  mBt.attachLongPressStart( hBtLongSt );  
  mBt.attachLongPressStart( hBtLongEn );  

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
  WebInit();
Serial.print( mBmpFile );  Serial.print( "  " );  
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
        // Do hardware reset
        digitalWrite( 2, LOW );
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
  if( bOn ) 
  { digitalWrite( 2, LOW );
    mBlinkMode |= 0x10;
  } else
  { digitalWrite( 2, HIGH );
    mBlinkMode &= ~0x10;
  } 
}

void  CGlobalData::Blinker(void) 
{ static byte  BlinkOn[4] =  {0, 9, 50,10};
  static byte  BlinkOff[4] = {0, 5,  9,50};

  if( millis() < mBlinkMs ) return;
  
  if( mBlinkMode & 0x10 ) 
  { BlinkerSet( BlinkOff[mBlinkMode&0x07]*100, 0 );
    if( mBlinkMode & 0x20 ) mBlinkMode = 0;
  } else BlinkerSet( BlinkOn[mBlinkMode&0x07], 1 );
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


