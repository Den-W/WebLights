/*  WebLights v1.02 by VDG
 *  This project designed for ESP8266 chip. Use it to control up to 256 LED strip on base of WS2811 chip.
 *  Copyright (c) by Denis Vidjakin, 
 *  
 *  https://github.com/Den-W/WebLights
 *  http://mysku.ru/blog/aliexpress/50036.html
 *  https://www.instructables.com/id/WebLights-Take-Control-Over-Your-New-Year-Lights/ 
 *  
 *  LED control procedures
 */
#include "main.h"

// @A-Z -> 0-27
byte  ChrTo27( char ch ) 
{   if( ch < 0x40 || ch > 0x5A ) 
      return 0;
    return ch - 0x40;    
}

//-------------------------------------------------------------------------------

// @A-za-z0-9 -> 0-63
byte  ChrTo64( char ch )
{   long  i = 0;
    if( ch >= 0x40 && ch <= 0x5A ) i = ch - 0x40;
    else if( ch >= 0x60 && ch <= 0x7A ) i = (ch - 0x60) + 27;
         else if( ch >= '0' && ch <= '9' ) i = (ch - '0') + 27 + 27;                
    return i;
}

//-------------------------------------------------------------------------------

// ASCII str -> int as HEX
word  HexToVal( const char *Str, int Sz )
{   char  *e, Tb[8];
    memcpy( Tb, Str, Sz );
    Tb[Sz] = 0;    
    return strtol( Tb, &e, 16 );
}

//-------------------------------------------------------------------------------

// ASCII str -> color
uByteLong  ChrToClr( const char *p )
{ uByteLong v;
  
  if( *p == '=' )  // Get current color
  { RgbColor c = gD.mLeds.GetPixelColor( HexToVal( p+1, 2 ) );
    v.uB[0] = c.R;
    v.uB[1] = c.G;
    v.uB[2] = c.B;
  } else
  {
    v.uB[0] = ChrTo64( p[0] ) << 2; // R
    v.uB[1] = ChrTo64( p[1] ) << 2; // G
    v.uB[2] = ChrTo64( p[2] ) << 2; // B
  }
 v.uB[3] = 0;
 return v;
}
 
//-------------------------------------------------------------------------------

int   CGlobalData::LedCallSub( const char *SubId2, byte RootCtx )
{ char  *p, Tb[16] = "[00";

  Tb[1] = SubId2[0];
  Tb[2] = SubId2[1];
    
  p = strstr( mScr.c_str(), Tb );
  if( !p ) return 0; // Subroutine not found
  if( mCtxCur >= CALL_SIZE-1 ) return 0;
  mCtxCur++;
  mCtx[ mCtxCur ].mPosScr = p - mScr.c_str() + 3;
  mCtx[ mCtxCur ].mRootCtx = RootCtx;
  return mCtxCur;
}

//-------------------------------------------------------------------------------

// Set pixel color and select next LED  00RRGGBB
void  CGlobalData::LedSetPxl( uByteLong clr ) 
{ TStack  &s = mCtx[mCtxCur];
  int     i = s.mPosLed;

  switch( s.mNxtMode )
  { case '>': mLeds.ShiftRight( 1, i, i+s.mNxtStep ); break;
    case '<': mLeds.ShiftLeft( 1, i, i+s.mNxtStep ); i += s.mNxtStep; break;
  }

  mLeds.SetPixelColor( i, RgbColor( clr.uB[1], clr.uB[0], clr.uB[2] ) );
  
  switch( s.mNxtMode )
  { default: if( !s.mNxtStep ) s.mNxtStep = 1;
    case '+': s.mPosLed += s.mNxtStep; break;
    case '-': s.mPosLed -= s.mNxtStep; break;
  }
  if( s.mPosLed > mLedCount ) s.mPosLed = mLedCount;
}

//-------------------------------------------------------------------------------

// Main script processing functions
void  CGlobalData::LedProcess( void ) 
{ int           i, n, pass;
  const char    *p, *e;
  char          c, Tb[32];
  unsigned long clr, l2;
  uByteLong     u;  
  TStack        &s = mCtx[mCtxCur];
  
  if( mbFirstPass ) // Call constructor [** if present
  { mbFirstPass = false;
    p = strstr( mScr.c_str(), "[**" );
    if( p ) mCtx[0].mPosScr = p - mScr.c_str() + 3;  
  }

  if( s.mPosScr < 0 || s.mPosScr > mScr.length() ) s.mPosScr = 0;
  p = mScr.c_str() + s.mPosScr;

  for( pass=0; pass<128; pass++ )
  {
    switch( *p )
    { case 0: // End of script. Restart
        p = mScr.c_str();
        break;

      case '!': // !xx LED address
        switch( p[1] )
        { case '*': // set current LED from variable
              s.mPosLed = p[2] == '*' ? mLedCount : mVars[ ChrTo64( p[2] ) ]; 
              break;
          case '@': // put current LED to variable
              mVars[ ChrTo64( p[2] ) ] = s.mPosLed; 
              break;
          case '#': // Clear all LED
              s.mPosLed = 0;
              for( i=0; i <= mLedCount; i++ ) mLeds.SetPixelColor( i, RgbColor( 0, 0, 0 ) );  
              break;
          default: s.mPosLed = HexToVal( p+1, 2 ); // Absolute address
        }
        
      case ':': // :LL Label - just skip it
        p += 3;
        continue;

      case '#': // Operations #zcxx
        i = ChrTo64( p[1] );
        c = p[2];
        if( p[3] != '*' ) n = HexToVal( p+3, 2 ); // Constant
        else {  // Variable 
                if( p[4] == '*' ) n = mLedCount+1;
                else n = mVars[ ChrTo64( p[4] ) ];
             }        
        p += 5;
        if( p[-4] == '#' ) // Set global mode
        { switch( c )
          { case '+': 
            case '-': 
            case '>': 
            case '<': s.mNxtMode = c;
                      s.mNxtStep = n;
                      break;
            //case '!': SPIFFS.format();
          }          
          continue;
        }
        switch( c )
        { case '=': mVars[i]  = n; continue; // Set Var
          case '+': mVars[i] += n; continue; // add Var
          case '-': mVars[i] -= n; continue; // sub Var
          case '*': mVars[i] *= n; continue; // mul Var
          case '/': mVars[i] /= n; continue; // div Var
          case '%': mVars[i] %= n; continue; // div Var
          case '<': mVars[i] <<= n; continue; // RollLeft Var
          case '>': mVars[i] >>= n; continue; // RollRight Var
          case '&': mVars[i] &= n; continue; // and Var
          case '|': mVars[i] |= n; continue; // or Var
          case '^': mVars[i] ^= n; continue; // xor Var
          case 'P': s.mPosLed = mVars[i]; continue; // Set cur led
          case 'G': mVars[i] = s.mPosLed; continue; // Get cur led
          case '?': if( mVars[i] < n ) s.mCmpRes = 1;  // compare Var 
                    else if( mVars[i] > n ) s.mCmpRes = 2;
                         else s.mCmpRes = 0;
                    continue;
          case 'F': if( i > 26 ) continue;            // Get fader value
                    if( mFdr[i].mInterval == 0 ) continue; // Fader is not active
                    clr = millis();
                    if( clr > mFdr[i].mStop ) mFdr[i].mStart = 0; // Fader end
                    if( mFdr[i].mStart == 0 ) // fader restart
                    { mFdr[i].mStart = clr;
                      mFdr[i].mStop  = clr + mFdr[i].mInterval;
                    }
                    clr = clr - mFdr[i].mStart;
                    clr <<= 8;
                    clr /= mFdr[i].mInterval;
                                        
                    u.uB[0] = mFdr[i].mClrStart[0] + ((clr*mFdr[i].mClrDiff[0])>>8);
                    u.uB[1] = mFdr[i].mClrStart[1] + ((clr*mFdr[i].mClrDiff[1])>>8);
                    u.uB[2] = mFdr[i].mClrStart[2] + ((clr*mFdr[i].mClrDiff[2])>>8);
                    u.uB[3] = 0;
                    for( ; n>0; n-- ) LedSetPxl( u );
                    continue;
                    
          case 'R': for( ; n>0; n-- ) mVars[(i++) & 0x3F] = random(255);
                    continue;
          case 'S':
                    u.uB[0] = mVars[i];         // R
                    u.uB[1] = mVars[(i+1)&0x3F];// G
                    u.uB[2] = mVars[(i+2)&0x3F];// B
                    u.uB[3] = 0;
                    for( ; n>0; n-- ) LedSetPxl( u );
                    continue;
                    
          case 'T': // Get RGB to 3 variables
                {   RgbColor c = gD.mLeds.GetPixelColor( s.mPosLed );
                    mVars[i] = c.G;         // R
                    mVars[(i+1)&0x3F] = c.B;// G
                    mVars[(i+2)&0x3F] = c.R;// B
                    continue;
                }
        }
        continue;

      case '$': // Set LED to VarA, VarB, VarC $abc        
        u.uB[0] = mVars[ ChrTo64( p[1] ) ];
        u.uB[1] = mVars[ ChrTo64( p[2] ) ];
        u.uB[2] = mVars[ ChrTo64( p[3] ) ];
        u.uB[3] = 0;
        LedSetPxl( u );
        p += 4;
        continue;
      
      case '^': // Operations with Timers ^Zpddd
        i = ChrTo27( p[1] );
        memcpy( Tb, p+3, 3 );
        Tb[3] = 0;
        clr = atoi(Tb);
        p += 6;
        switch( p[-4] )
        { case 'R':
            mTmr[i].mStart = millis();
            continue;
          case 'S':
            clr *= 1000;
        }
        mTmr[i].mTreshold = clr;
        continue;
        
      case '?': // Conditional jumps ?cLL
        c = p[1];
        p += 4;
        switch( c )
        { default: 
             if( c < 0x40 || c > 0x5A ) continue;
             // Check timer
             i = ChrTo27( c );
             if( millis() <  mTmr[i].mStart + mTmr[i].mTreshold ) continue;
             mTmr[i].mStart = millis();
             break;
          case '+': break; // Unconditional jump
          case '>': if( s.mCmpRes != 2 ) continue;
                    break;
          case '<': if( s.mCmpRes != 1 ) continue;
                    break;
          case '=': if( s.mCmpRes != 0 ) continue;
                    break;
          case '!': if( s.mCmpRes == 0 ) continue;
                    break;
        }

        memcpy( Tb+1, p-2, 2 );
        Tb[0] = ':';
        Tb[3] = 0;
        e = strstr( mScr.c_str(), Tb );
        if( !e ) break; // Label not found
        p = e;
        continue;

      case '>': // Call subroutine. >mLL
        c = p[1];
        p += 4;
        switch( c )
        { default: 
             if( c < 0x40 || c > 0x5A ) continue; // not a timer ID
             // Set timer call
             i = ChrTo27( c ); 
             mTmr[i].mSubProc[0] = p[-2];
             mTmr[i].mSubProc[1] = p[-1];
             continue;
          case '>': if( s.mCmpRes != 2 ) continue; // Call if >
                    break;
          case '<': if( s.mCmpRes != 1 ) continue;  // Call if <
                    break;
          case '=': if( s.mCmpRes != 0 ) continue;  // Call if ==
                    break;
          case '!': if( s.mCmpRes == 0 ) continue;  // Call if !=
          case '+': break;
        }
        
        LedCallSub( p-2, s.mRootCtx );
        break; 
        
      case '[': // SubProgram start. Skip it
        p = strchr( p, ']' );
        if( p ) p++; else p = mScr.c_str();
        continue;
        
      case ']': // SubProgram end. return
        if( !mCtxCur ) p = mScr.c_str(); // return from Constructor [** call. Go to start of Scr
        else mCtxCur--; // Return from subroutine
        break;

      case '*': // Fader *Zptttbbbccc changing color from bbb to ccc
        i = ChrTo27( p[1] ); // Fader ID
        p += 12;
        if( i > 53 ) continue;
        if( i > 27 )
        { i -= 27;
          mFdr[i].mMode = 0x01; // BiDirection fader
        } else mFdr[i].mMode = 0; // SingleDirection fader
        
        memcpy( Tb, p-9, 3 );
        Tb[3] = 0;
        clr = atoi(Tb);
        if( p[-10] == 'S' ) clr *= 1000;
        mFdr[i].mInterval = clr; // Fader interval
       
        // start color
        u = ChrToClr( p-6 );
        mFdr[i].mClrStart[0] = u.uB[0];
        mFdr[i].mClrStart[1] = u.uB[1];
        mFdr[i].mClrStart[2] = u.uB[2];

        // End color
        u = ChrToClr( p-3 );
        mFdr[i].mClrDiff[0] = u.uB[0] - mFdr[i].mClrStart[0];
        mFdr[i].mClrDiff[1] = u.uB[1] - mFdr[i].mClrStart[1];
        mFdr[i].mClrDiff[2] = u.uB[2] - mFdr[i].mClrStart[2];
        continue;
              
      case '~': // ~tttFlName; play BMP file.
        e = strchr( p, ';' );
        if( !e )
        { p = mScr.c_str(); // Format error
          break;
        }
        i = LedBmpOpen( atoi( p+1 ), p+4 );
        p = e;
        if( i <= 0 ) p++;
        
      case ';': // waiting for end of file
        if( LedBmpPlay() <= 0 ) p++;
        break;

      case '(': // (LLxxxxcc) External events - just skip it
        p = strchr( p, ')' );
        if( p ) p++; else p = mScr.c_str();
        continue;

      case '<': // <FileName> Load and execute FileName. if not exists - load cur.scr
	      strcpy( Tb, "/cur.scr" );
        e = strchr( p, '>' );
	      if( e )
        { n = e - p - 1;
          if( n > 30 ) n = 30;
          memcpy( Tb, p+1, n );
          Tb[n] = 0;                 
        }
        { File  f = SPIFFS.open( Tb, "r");  
          if( !f ) ESP.restart();
          Rst();
          mScr = f.readString();
          f.close();
        }        
        return; // New file loaded. Play it.
            
      default:  // Set absolute pixel colour
        if( *p <= ' ' ) 
        { p++;
          break;
        }        
        LedSetPxl( ChrToClr( p ) );
        p += 3;
        continue;
    }
    break;
  }
  
  s.mPosScr = p - mScr.c_str();
  if( s.mRootCtx != 0 )  return; // Some CoRoutine already active

  while( 1 )  
  { if( ++mTmrCur > 26 )
    { mTmrCur = 0;
      break;
    }
    if( mTmr[mTmrCur].mSubProc[0] <= '!' ) continue; // No timer-activated subproc
    if( millis() < mTmr[mTmrCur].mStart + mTmr[mTmrCur].mTreshold ) continue;
    mTmr[mTmrCur].mStart = millis();    
    LedCallSub( mTmr[mTmrCur].mSubProc, 1 );
    break;
  }
}

//-------------------------------------------------------------------------------

int   CGlobalData::LedBmpOpen( int To, const char *FlName ) 
{ int         i, j, n;
  const char  *e;
  byte        Tb[800];  
  // File play start
  mBmpCurY = 0;
  mTmrBmp.mStart = 0;
  mTmrBmp.mTreshold = To;
  if( !mTmrBmp.mTreshold ) goto gStop;

  e = strchr( FlName, ';' );
  if( e ) n = e - FlName;
  else n = strlen( FlName );
  
  if( n > 32 ) n = 32;
  memcpy( Tb+1, FlName, n );
  Tb[0] = '/';
  Tb[n+1] = 0;

  mFlBmp = SPIFFS.open( (char*)Tb, "r");
  if( mFlBmp == 0 ) // Can't open file. Go to next cmd
        return -1;

  mFlBmp.seek( 0, SeekSet );
  i = mFlBmp.read( Tb, 14 ); // WINBMPFILEHEADER
  if( i != 14 )//|| Tb[0] != 0x42 || Tb[1] != 0x4D ) // Format error. Go to next cmd
  {   
gStop:    mFlBmp.close();
          return 0;
  }
  i = Tb[10] + 256L*Tb[11] - 14;
  if( i > 256 ) goto gStop; // WINxxBITMAPHEADER size error
  n = mFlBmp.read( Tb, i ); // WINxxBITMAPHEADER
  if( n != i ) goto gStop; // WINxxBITMAPHEADER read error

  mBmpPic = 14 + i; // Start of pic data
  mBmpLineSize = (3*mBmpX + 3) & 0xFFFC; // Length of one X line in bytes
      
  switch( Tb[0] )
  {  default: goto gStop; // WINxxBITMAPHEADER format error
     case 0x12: // WIN2XBITMAPHEADER      
          mBmpX = Tb[4] + 256*Tb[5];
          mBmpY = Tb[6] + 256*Tb[7];
          if( Tb[10] != 24 ) goto gStop; // WINxxBITMAPHEADER format error
          break;
     case 0x28: // WIN3XBITMAPHEADER
          mBmpX = Tb[4] + 256*Tb[5];
          mBmpY = Tb[8] + 256*Tb[9];
          if( Tb[14] != 24 ) goto gStop; // WINxxBITMAPHEADER format error
          break;
  }
  
  // All Ok. May play. FilePos on the start of BMP data
  return 1;
}

//-------------------------------------------------------------------------------

int   CGlobalData::LedBmpPlay( void ) 
{ int         i, j, n;
  const char  *e;
  byte        Tb[800];  

  // Play file. Check timer    
  if( millis() <  mTmrBmp.mStart + mTmrBmp.mTreshold ) 
      return 1; // Delay is not up yet
      
  mTmrBmp.mStart = millis();
  if( mBmpCurY >= mBmpY || !mFlBmp ) // End of file
  {   
gStop:    mFlBmp.close();
          return 0;
  }

  // Read data block        
  i = mBmpPic + mBmpLineSize * (mBmpY- ++mBmpCurY); // Start of Y line in bytes
  mFlBmp.seek( i, SeekSet );

  i = mBmpX;
  if( i > 256 ) i = 256;
  i *= 3;
  n = mFlBmp.read( Tb, i );  // BGR X line
  if( n != i ) goto gStop; // EOF reached

  for( i=j=n=0; i<mLedCount; i++,n++ )
  {  mLeds.SetPixelColor( i, RgbColor( Tb[j+1], Tb[j+2], Tb[j] ) );
     j += 3;
     if( n >= mBmpX ) n = j = 0; // Repeat picture to the rest of strip
  }
  return 1;
}

//-------------------------------------------------------------------------------

int   CGlobalData::LedBmpFileChg( int Mov, const char *Evt )
{ if( !mLedMode ) return 0;

  int     i = 0;
  String  sPrev, sNext, sFirst, sLast;
  const char *p;
  Dir dir = SPIFFS.openDir("/");
  
  BlinkerSet( 100, 1 );   
  if( Evt && *Evt  ) 
  { sNext = Evt;
    goto gOpn;    
  }

  while( dir.next() )
  { const String &sn = dir.fileName();
    if( sn.length() < 5 ) continue;
    if( !strstr( sn.c_str(), ".bmp" ) && !strstr( sn.c_str(), ".BMP" ) )    
      continue;

    if( !sFirst.length() ) sFirst = sn;
      
    if( mBmpFile[0] ) 
    { if( sn == mBmpFile ) 
        { i++;
          continue;
        } 
        if( !i ) 
        { sPrev = sn;
          continue;
        }
    }

    if( i == 1 ) { i++; sNext = sn; }
    sLast = sn;
  }

  if( !Mov )
  { // Set name according to mLedMode
    if( mBmpFile[0] && mLedMode == 2 ) sNext = mBmpFile;    
    if( sNext.length() == 0 ) sNext = sFirst;    
  } else if( Mov < 0 )
         { if( sPrev.length() ) sNext = sPrev;
           else sNext = sLast;
         } else if( sNext.length() == 0 ) 
                  sNext = sFirst;

gOpn:    
  if( sNext.length() == 0 ) return 0;
  strcpy( mBmpFile, sNext.c_str() );         

  p = strchr( mBmpFile, '#' );
  if( !p++ ) p = "100";
  i = atoi( p );
  if( i < 10 ) i = 10;

  if( mFlBmp ) mFlBmp.close();          

  if( LedBmpOpen( i, mBmpFile+1 ) > 0 ) 
      return 1;
  
  mBmpFile[0] = 0;
  return 0;  
}

//-------------------------------------------------------------------------------


