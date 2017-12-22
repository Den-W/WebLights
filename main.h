/*  WebLights v1.08 by VDG
 *  This project designed for ESP8266 chip. Use it to control up to 256 LED strip on base of WS2811 chip.
 *  Copyright (c) by Denis Vidjakin, 
 *  
 *  https://github.com/Den-W/WebLights
 *  http://mysku.ru/blog/aliexpress/50036.html
 *  https://www.instructables.com/id/WebLights-Take-Control-Over-Your-New-Year-Lights/ 
 *  
 *  !!!!!!!!!!!!! 
 *  !!! Important notice: Some problem may arise when you power ESP8266 from USB and LED strip from external PSU. 
 *  !!! LED may do not work correctly. Switch power line of ESP8266 from USB to LED PSU before looking for results. 
 *  !!!!!!!!!!!!!
 */
 
#include <Arduino.h>
#include <stdio.h>
#include <EEPROM.h>
#include "OneButton.h"
#include <FS.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <DNSServer.h>

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

#define PIN_BUTTON  D2  // I GPIO4  - D2  Button
#define PIN_LED     D4  // O GPIO2  - D4  OnBoard LED
#define PIN_IRDA    D5  // I GPIO14 - D5  IRDA receiver

#define CALL_SIZE   16  // Stack call depth

typedef unsigned long DWORD;

typedef union
{ long  uL;
  byte  uB[4];  // RGB?
} uByteLong;

typedef struct 
{   long      mStart;       // Fader start millis()
    long      mStop;        // Fader stop millis()
    long      mInterval;    // Fader interval
    short     mClrStart[3]; // RGB - start
    short     mClrDiff[3];  // RGB - end
    byte      mMode;        // 0x01: 0x00:SingleDirection, 0x01:BiDirection
} TFader;

typedef struct 
{   long      mStart;       // timer start millis()
    long      mTreshold;    // timer stop millis()
    char      mSubProc[2];  // Procedure to call
} TTimer;

typedef struct 
{   byte      mCmpRes;      // 0:==, 1:<, 2:>
    byte      mNxtMode;     // How to move PosLed after set
    byte      mNxtStep;     // Move step. 1 by default
    byte      mPosLed;      // current LED position
    int       mPosScr;      // Program Counter - Offset in mScr
    byte      mRootCtx;     // Thread context 0:main, 1:Timer, 2:Event
} TStack;

class CGlobalData
{   
  public:
    
    byte    _St;
    byte      mWF_Mode;         // 0: AccessPoint, 1:Client
    byte      mLedCount;        // Led strip size (1). FF = 256 LEDs in strip.    
    byte      mLedMode;         // Led play mode (1). 0:Script, 1:Play all BMP, 2:PlaySelectedBmp.
    byte      mLedOrder;        // 0-7
    byte      mLedBrightness;   // 1-100
    char      mWF_Id[32];       // WiFi SSID
    char      mWF_Pwd[32];      // WiFi password
    char      mIr_Up[6];        // IR 0001 Up
    char      mIr_Dn[6];        // IR 0002 Down
    char      mBmpFile[32];     // Current play file
    byte    _Wr;                // Data between _St and _Wr stored in flash memory. _Wr == crc8 of block
    
    byte      mKey;             // Pressed key    
    long      mFlSize;
    long      mToLp;            // LongPress start    
    byte      mbFirstPass;      // true on first pass
    byte      mBlinkMode;
    byte      mBlinkLed;
    byte      mLedR, mLedG, mLedB; // LED offsets 0,1,2
    unsigned short     mIrCommand;       // Last IR command.
    short     mCtxCur;          // Current context
    long      mBlinkMs;
    int       mBmpLineSize;
    short     mBmpPic, mBmpX, mBmpY, mBmpCurY; // BMP play variables
    TTimer    mTmrBmp;
    TStack    mCtx[CALL_SIZE];
    short     mTmrCur;          // Current context
    TTimer    mTmr[27];     
    TFader    mFdr[27];    
    byte      mVars[65];        // @A-Z'a-z + LedNumber    
    byte      mBrightness[256];
    byte    _En;

    String            mScr;     // Current script
    File              mFl;      // upload file handler
    File              mFlBmp;   // BMP file handler
    OneButton         mBt;      // Button handlers
    IRrecv            mIrda;
    decode_results    mIrdaRes;
    ESP8266WebServer  mSrv;     // http server
    IPAddress         mIP;
    DNSServer         mDns;
    
    class CLeds : public NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> 
    { public: 
              CLeds( void ) : NeoPixelBus(256){}
        // Very bad hack in SetMaxPxl()! But I am too lazy to create NeoPixelBus() dynamically.
        // It is possible totally remove SetMaxPxl() and work with maximum LED length 256.
        void  SetMaxPxl( int MaxPxl ) { *((uint16_t*)&_countPixels) = MaxPxl; } 
    }                 mLeds;

    CGlobalData() : mSrv(80), mBt(PIN_BUTTON, true), mIrda(PIN_IRDA)
    { Defaults();
    }
              
    void  Start(void);
    void  Run(void);
    void  Blinker(void);
    void  BlinkerSet( int Ms, int bOn );
    void  FlashRd(void);
    void  FlashWr(void);    
    int   LedBmpOpen( int To, const char *Name );
    int   LedBmpPlay( void );
    void  LedBmpStop( void ) { mFlBmp.close(); };    
    int   LedBmpFileChg( int Mov, const char *Evt );
    void  LedProcess( void );
    void  LedSetPxl( uByteLong clr );
    int   LedCallSub( const char *SubId2, byte RootCtx ); // 0:NotFound\NoStackSpace

    void  WebInit( void );    
    void  WebTxPage( int b200, PGM_P content );
    void  Pgm2Str( String &s, PGM_P content );
    
    void  Defaults( void )
          { memset( &_St, 0, &_En-&_St );
            strcpy( mWF_Id,  "WebLights" );  // default WiFi SSID
            strcpy( mWF_Pwd, "weblights" );  // default WiFi password
            mLedCount = 50-1;                // default strip size = 50
            mLedOrder = 2;
            Brightness( 100 );
            Rst();
          }  
            
    void  Rst( void )
        {   int i;
            mFlBmp.close();
            memset( &_Wr, 0, &_En-&_Wr );

            mbFirstPass = true;
            for( i=0; i<CALL_SIZE; i++ ) mCtx[i].mNxtStep = 1;
            for( i=0; i <= mLedCount; i++ ) mLeds.SetPixelColor( i, RgbColor( 0, 0, 0 ) );  
            
            switch( mLedOrder & 0x07 )
            { default:  mLedR = 0; mLedG = 1; mLedB = 2; break;
              case 1:   mLedR = 0; mLedG = 2; mLedB = 1; break;
              case 2:   mLedR = 1; mLedG = 0; mLedB = 2; break;
              case 3:   mLedR = 2; mLedG = 0; mLedB = 1; break;
              case 4:   mLedR = 1; mLedG = 2; mLedB = 0; break;
              case 5:   mLedR = 2; mLedG = 1; mLedB = 0; break;
            }
            digitalWrite( PIN_LED, HIGH );
            Brightness( mLedBrightness );
        }
        
  void  Brightness( byte br )        
  { int i;

    if( br < 1 ) br = 1;
    if( br > 100 ) br = 100;
    
    mLedBrightness = br;
  
    for( i = 0; i < 256; i++ )
    { mBrightness[i] = (i*br) / 100;
    }
    
  }
};

extern CGlobalData gD;

