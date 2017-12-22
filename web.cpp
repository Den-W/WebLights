/*  WebLights v1.08 by VDG
 *  This project designed for ESP8266 chip. Use it to control up to 256 LED strip on base of WS2811 chip.
 *  Copyright (c) by Denis Vidjakin, 
 *  
 *  https://github.com/Den-W/WebLights
 *  http://mysku.ru/blog/aliexpress/50036.html
 *  https://www.instructables.com/id/WebLights-Take-Control-Over-Your-New-Year-Lights/ 
 *  
 *  WEB related stuff and pages data
 */
#include "main.h"

const char P_Set[] PROGMEM = R"(
[rc:0k]
)";

const char P_Main[] PROGMEM = R"(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<head><title>Web Lights</title><meta http-equiv="Content-Type" content="text/html; charset=win1251">
<meta name=viewport content="width=device-width, initial-scale=1.1">
<style>
h4{text-align: center;background: orange;margin: -2px 0 1em 0;}
body {font-size:14px;}
label {float:left; padding-right:10px;}
.fld{clear:both; text-align:right; line-height:25px;}
.frm{float:left;border: 2px solid #634f36;background: #f3f0e9;padding: 5px;margin:3px;}
</style>
<script type="text/javascript">
 window.addEventListener("load",function(e)
 { document.getElementById("btWC").addEventListener("click",btc_WC);   
   document.getElementById("btU").addEventListener("click",btc_U);   
   document.getElementById("btD").addEventListener("click",btc_D);   
 })
 function btc_WC(){ TxRq("c","fWC" ); }
 function btc_U(){ TxRq("e","fU" ); }
 function btc_D(){ TxRq("e","fD" ); }
 
 function TxRq(url,fName,id)
 { var xhr = new XMLHttpRequest();
   xhr.open('POST',url,false);
   var fData = new FormData(document.forms.namedItem(fName));
   xhr.send(fData);
   if (xhr.status != 200) alert( "TxRq error - " + xhr.status + ': ' + xhr.statusText );
   else { if (id) document.getElementById(id).innerHTML = xhr.responseText; }
 }
</script></head><body>
<div class="frm"><h4>WebLights info</h4>
<table width="100%" cellspacing="0" cellpadding="4">
<tr><td align="left" width="60">Address</td><td>"@IP@"</td></tr>
<tr><td align="left">IR Code</td><td>"@IRC@"</td></tr>
"@BMP@"</table><hr/><h4>WebLights config</h4>
<form name="fWC" method="POST">
 <div class="fld"><label for="cM">WiFi Mode</label><select name="cM"><option value="0" "@SA@">Access Point</option><option value="1" "@SC@">Client</option></select></div>
 <div class="fld"><label for="cN">Name</label><input type="text" name="cN" maxlength="31" value="@NAME@"></div>
 <div class="fld"><label for="cP">Password</label><input type="text" name="cP" maxlength="31" value="@PASS@"></div>
 <div class="fld"><label for="cL">LED Num</label><input type="text" name="cL" max="256" value="@LED@"></div>
 <div class="fld"><label for="cB">LED Brightness</label><input type="text" name="cB" min="1" max="100" value="@BRG@"></div>
 <div class="fld"><label for="cO">LED order</label><select name="cO">"@ML@"</select></div><br/>
 <div class="fld"><label for="cY">LED Play Mode</label><select name="cY"><option value="0" "@sS@">Script</option><option value="1" "@sL@">BMP: All</option><option value="2""@sO@">BMP: One</option></select></div> 
 <div class="fld"><label for="cU">IR 0001 (Prev)</label><input type="text" name="cU" maxlength=4 value="@IRU@"></div>
 <div class="fld"><label for="cD">IR 0002 (Next)</label><input type="text" name="cD" maxlength=4 value="@IRD@"></div> 
 <div style="float:right;"><button id="btWC">Set params</button></div>
</form> 
<br/><hr/><a href="ps">Script</a>&emsp;<a href="pf">Files</a>     "@PNB@"
</div>
</body></html>
)";

const char P_Btn[] PROGMEM = R"(
<div style="float:right;"><form name="fD" method="POST" id="btD"><input name="ev" type="hidden" value="D"><input type="submit" value="Next"></form></div>
<div style="float:right;"><form name="fU" method="POST" id="btU"><input name="ev" type="hidden" value="U"><input type="submit" value="Prev">&emsp;</form></div>
)";

//-------------------------------------------------------------------------------------------

const char P_Scr[] PROGMEM = R"(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<head><title>Web Lights</title><meta http-equiv="Content-Type" content="text/html; charset=win1251">
<meta name=viewport content="width=device-width, initial-scale=1.0">
<style>
h4{text-align: center;background: orange;margin: -2px 0 1em 0;}
body {font-size:14px;}
label {float:left; padding-right:10px;}
.fld{clear:both; text-align:right; line-height:25px;}
.frm{float:left;border: 2px solid #634f36;background: #f3f0e9;padding: 5px;margin:3px;}
</style>
<script type="text/javascript">
 window.addEventListener("load",function(e)
 { document.getElementById("btSC").addEventListener("click",btc_SC);   
   document.getElementById("btE").addEventListener("click",btc_E);
   document.getElementById("btU").addEventListener("click",btc_U);   
   document.getElementById("btD").addEventListener("click",btc_D);   
 })
 function btc_SC(){ TxRq("s","fSC" ); } 
 function btc_E(){ TxRq("e","fE" ); }
 function btc_U(){ TxRq("e","fU" ); }
 function btc_D(){ TxRq("e","fD" ); }
 
 function TxRq(url,fName,id)
 { var xhr = new XMLHttpRequest();
   xhr.open('POST',url,false);
   var fData = new FormData(document.forms.namedItem(fName));   
   xhr.send(fData);
   if (xhr.status != 200) alert( "TxRq error - " + xhr.status + ': ' + xhr.statusText );
   else { if (id) document.getElementById(id).innerHTML = xhr.responseText; }
 }
</script></head><body>
<div class="frm" style="clear:both;"><h4>WebLights LED Script</h4>
<table><tr>
<td><form name="fSC" method="POST">
<textarea style="width:99%" name="Scr" cols="60" rows="20">"@SCR@"</textarea><br/><br/>
<div style="float:left;"><a href="/">Config</a>&emsp;<a href="pf">Files</a></div>
<div style="float:right;">&emsp;<button id="btSC">Save</button></div>
</form>
"@PNB@"</td>
<td valign="bottom"><form name="fE" method="post">"@BTNS@"&emsp;<button id="btE">Activate</button></form></td>
</tr></table>
</div>
</body></html>
)";

//-------------------------------------------------------------------------------------------

const char P_File[] PROGMEM = R"(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN">
<head><title>Web Lights</title><meta http-equiv="Content-Type" content="text/html; charset=win1251">
<meta name=viewport content="width=device-width, initial-scale=1.0">
<style>
h4{text-align: center;background: orange;margin: -2px 0 1em 0;}
body {font-size:14px;}
label {float:left; padding-right:10px;}
.fld{clear:both; text-align:right; line-height:25px;}
.frm{float:left;border: 2px solid #634f36;background: #f3f0e9;padding: 5px;margin:3px;}
</style>
<script type="text/javascript">
 window.addEventListener("load",function(e)
 { document.getElementById("btFL").addEventListener("click",btc_FL);
 })
 function btc_FL(){ TxRq("f","fFL" ); }
 
 function TxRq(url,fName,id)
 { var xhr = new XMLHttpRequest();
   xhr.open('POST',url,false);
   var fData = new FormData(document.forms.namedItem(fName));   
   xhr.send(fData);
   if (xhr.status != 200) alert( "TxRq error - " + xhr.status + ': ' + xhr.statusText );
   else { if (id) document.getElementById(id).innerHTML = xhr.responseText; }
 }
</script></head><body>
<div class="frm"><h4>WebLights Files "@FR@"</h4>
<form name="fFL" method="POST"><table>"@FILE@"
<tr><td colspan=2><hr />
 <div class="fld"><label for="fN">New filename</label><input type="text" name="fN" maxlength="32" ></div><br/>
 <div class="fld"><label for="fB">Upload file</label><input type="file" name="fB" formenctype="multipart/form-data"></div><br/>
 <div class="fld">
  <input type="radio" name="fO" value="L">Scr Load<input type="radio" name="fO" value="S">Scr Save
  <input type="radio" name="fO" value="P">Play<br/><input type="radio" name="fO" value="R">Rename
  <input type="radio" name="fO" value="D">Delete<input type="radio" name="fO" value="U">Upload<br/>
 </div></td></tr>
<tr><td><a href="/">Config</a>&emsp;<a href="ps">Script</a></td>
    <td align="right"><button id="btFL">Exec</button>
</td></tr></table>
</form></div>
</body></html>
)";

//-----------------------------------------------------------------------------

void  ShowArgs( const char *Msg )
{   Serial.print("\nUrl:" ); Serial.print( gD.mSrv.uri() ); Serial.print("  " ); Serial.print( Msg );
    for ( int n = 0; n < gD.mSrv.args(); n++ ) 
     { Serial.print("\n"); Serial.print( n ); Serial.print(":"); 
       Serial.print(gD.mSrv.argName(n)); Serial.print("="); Serial.print(gD.mSrv.arg(n)); 
     }
}

//-----------------------------------------------------------------------------

void  CGlobalData::Pgm2Str( String &sPg, PGM_P content )
{   int     n;
    char    contentUnit[400], Tb[128], *p, *e;
    PGM_P   contentNext;

    BlinkerSet( 500, 1 );
    while( true )
    {   // due to the memccpy signature, lots of casts are needed
        memset( contentUnit, 0, sizeof(contentUnit) );
        contentNext = (PGM_P)memccpy_P((void*)contentUnit, (PGM_VOID_P)content, 0, sizeof(contentUnit)-1 );
        
        content += sizeof(contentUnit)-1;

        while( 1 )
        { p = strstr( contentUnit, "\"@" );
          if( p ) *p = 0;          
          sPg += contentUnit;
          if( !p ) break;
          p += 2;
          e = strstr( p, "@\"" );
          if( !e ) break;
          n = e - p;
          if( n > sizeof(Tb)-1 ) n = sizeof(Tb)-1;
          memcpy( Tb, p, n ); Tb[n] = 0;
          strcpy( contentUnit, e+2 );

          if( !strcmp( Tb, "NAME" ) ) // WiFi network SSID
          {   sPg += mWF_Id;
              continue;
          }

          if( !strcmp( Tb, "PASS" ) ) // WiFi password
          {   sPg += mWF_Pwd; 
              continue;
          }
          if( !strcmp( Tb, "IP" ) ) // Current TCP address
          {   sPg += WiFi.localIP().toString();
              continue;
          }
          if( !strcmp( Tb, "BMP" ) ) // Current TCP address
          {   if( !mLedMode ) continue;            
              sPg += "<tr><td align=\"left\">BMP file</td><td>";
              sPg += mBmpFile;
              sPg += "</td></tr>";
              continue;
          }
          if( !strcmp( Tb, "LED" ) )  // Last IR command
          {   sprintf( Tb, "%d", mLedCount+1 );
              sPg += Tb;
              continue;
          }
          if( !strcmp( Tb, "BRG" ) )  // Last IR command
          {   sprintf( Tb, "%d", mLedBrightness );
              sPg += Tb;
              continue;
          }
          if( !strcmp( Tb, "ML" ) )
          { const char *Nm[] = { "RGB","RBG","GRB","GBR","BRG","BGR",0};
            for( n=0; Nm[n]; n++ )
            { sprintf( Tb, "<option value=\"%d\" %s>%s</option>", n, n==gD.mLedOrder ? "selected":"", Nm[n] );
              sPg += Tb;
            }
              continue;
          }
          if( !strcmp( Tb, "SCR" ) )  // Current script
          {   sPg += mScr;          
              continue;
          }
          if( !strcmp( Tb, "IRC" ) )  // Last IR command
          {   sprintf( Tb, "%04X", mIrCommand );
              sPg += Tb;
              continue;
          }
          if( !strcmp( Tb, "SA" ) )  // Access point
          {   if( mWF_Mode == 0 ) sPg += "selected";
              continue;
          }
          if( !strcmp( Tb, "SC" ) )  // Client
          {   if( mWF_Mode == 1 ) sPg += "selected";
              continue;
          }
          if( !strcmp( Tb, "sS" ) )  // Script
          {   if( mLedMode == 0 ) sPg += "selected";
              continue;
          }          
          if( !strcmp( Tb, "sL" ) )  // BMP all
          {   if( mLedMode == 1 ) sPg += "selected";
              continue;
          }
          if( !strcmp( Tb, "sO" ) )  // BMP one
          {   if( mLedMode == 2 ) sPg += "selected";
              continue;
          }
          if( !strcmp( Tb, "IRU" ) )  // IR 0001 Up
          {   sPg += mIr_Up;
              continue;
          }
          if( !strcmp( Tb, "IRD" ) )  // IR 0001 Up
          {   sPg += mIr_Dn;
              continue;
          }
          if( !strcmp( Tb, "PNB" ) ) // Add Prev/Next buttons
          { Pgm2Str( sPg, P_Btn );
            continue;
          }
          if( !strcmp( Tb, "FR" ) )  // Fileinfo data
          {   FSInfo fs_info;
              SPIFFS.info(fs_info);
              sprintf( Tb, "(free %d)", fs_info.totalBytes - fs_info.usedBytes );
              sPg += Tb;
              continue;
          }
          
          if( !strcmp( Tb, "BTNS" ) ) // IR\WEB activated subprograms (LLxxxxc…c)
          { n = 0;
            p = (char*)mScr.c_str();
            while( 1 )
            { p = strchr( p, '(' );
              if( !p ) break;
              p += 1;
              e = strchr( p, ')' );
              if( !e ) break;
              n = e - p;
              if( n < 8 ) continue;
              if( n >= sizeof(Tb)-10 ) n = sizeof(Tb)-10;
              sPg += "<input type=\"radio\" name=\"ev\"value=";
              sprintf( Tb, "\"%c%c\">", p[0], p[1] );
              sPg += Tb;
              memcpy( Tb, p+6, n ); Tb[n-6] = 0;
              sPg += Tb;
              sPg += "<br/>";
            }
            continue;
          }
          if( !strcmp( Tb, "FILE" ) )  // File list
          { n = 0;
            Dir dir = SPIFFS.openDir("/");
            while( dir.next() )
            { if( dir.fileName().length() < 2 ) continue;
              File f = dir.openFile("r");             
              if( n++ == 0 ) sPg += "<tr>";
              sPg += "<td><input type=\"radio\" name=\"fS\" value=\"" + dir.fileName().substring(1) + "\">";
              sPg += dir.fileName().substring(1) + " / " + f.size() + "</td>";
              if( n >= 2 ) { sPg += "</tr>"; n = 0; }
            }
            continue;
          }
        }
        if( contentNext ) break;
    }
}

//-------------------------------------------------------------------------------

void CGlobalData::WebTxPage( int b200, PGM_P content )
{   String  sPg;
    Pgm2Str( sPg, content );
    mSrv.send( b200 ? 200:404, "text/html", sPg );
}

//-------------------------------------------------------------------------------

void handle_root() 
{ //ShowArgs( "- Root" ); // Debug WEB output
  gD.WebTxPage( true, P_Main );
}

void handle_scr() 
{ // ShowArgs( "- scr" );  // Debug WEB output
  gD.WebTxPage( true, P_Scr );
}

void handle_files() 
{ // ShowArgs( "- files" );  // Debug WEB output
  gD.WebTxPage( true, P_File );
}

//-------------------------------------------------------------------------------

int   strcpymax( char *To, const String &From, int Max )
{ int i = From.length();
  if( !i ) return 0;
  if( i >= Max ) i = Max-1;
  memcpy( To, From.c_str(), i );  
  To[i] = 0;
  return i;
}

void handle_cf() 
{ gD.mWF_Mode = atoi( gD.mSrv.arg("cM").c_str() );
  gD.mLedMode = atoi( gD.mSrv.arg("cY").c_str() );
  gD.mLedOrder = atoi( gD.mSrv.arg("cO").c_str() );
  gD.mLedCount = atoi( gD.mSrv.arg("cL").c_str() )-1;
  gD.mLedBrightness = atoi( gD.mSrv.arg("cB").c_str() );
  if( !gD.mLedCount ) gD.mLedCount = 50;
  strcpymax( gD.mWF_Id,   gD.mSrv.arg("cN"), sizeof(gD.mWF_Id) );
  strcpymax( gD.mWF_Pwd,  gD.mSrv.arg("cP"), sizeof(gD.mWF_Pwd) );
  strcpymax( gD.mIr_Up,   gD.mSrv.arg("cU"), sizeof(gD.mIr_Up) );
  strcpymax( gD.mIr_Dn,   gD.mSrv.arg("cD"), sizeof(gD.mIr_Dn) );

  gD.Brightness( gD.mLedBrightness );
  
  gD.FlashWr();
  // ShowArgs( "-WebCfg" );  // Debug WEB output
  gD.WebTxPage( true, P_Set );
  Serial.print( "[Restart]" );
  delay( 2000 );
  ESP.restart();
}

//-------------------------------------------------------------------------------

 void handle_ev() 
{ String s = gD.mSrv.arg("ev"); 

  if( s.length() > 0 ) 
  { if( gD.mLedMode == 0 ) gD.LedCallSub( s.c_str(), 2 );
    else { if( s == "D" ) gD.LedBmpFileChg( 1, 0 );
           if( s == "U" ) gD.LedBmpFileChg( -1, 0 );
         }
  }
  // ShowArgs( "- Event" );  // Debug WEB output
  gD.WebTxPage( true, P_Set );  
}

//-------------------------------------------------------------------------------

void handle_sc() 
{ String s = gD.mSrv.arg("Scr"); 
  if( s.length() > 0 ) 
  {   gD.Rst();
      gD.mScr = s;

      SPIFFS.remove( "/cur.scr" );
      File  f = SPIFFS.open( "/cur.scr", "w");
      if( f ) 
      { f.write( (byte*)s.c_str(), s.length() );
        f.close();
      }
  }
  // ShowArgs( "- Script" );  // Debug WEB output
  gD.WebTxPage( true, P_Set );  
}
//-------------------------------------------------------------------------------

void handle_frx() 
{ HTTPUpload& upload = gD.mSrv.upload();  
  switch( upload.status )
  { case UPLOAD_FILE_START:
      { String  s = "/" + gD.mSrv.arg("fS");
        if( s.length() < 2 ) s = "/" + gD.mSrv.arg("fN");
        if( s.length() < 2 ) s = "/" + upload.filename;
        gD.mFl.close();
        SPIFFS.remove( s );
        gD.mFl = SPIFFS.open( s, "w" );
        gD.mFlSize = 0;
        break;
      }
    case UPLOAD_FILE_WRITE:
        if( !gD.mFl ) break;
        gD.mFlSize += upload.currentSize;
        gD.mFl.write( upload.buf, upload.currentSize );
        break;
    case UPLOAD_FILE_END:
        gD.mFl.close();
        break;    
  }
  yield();
}
    
void handle_fl() 
{  File   f;
   String s = gD.mSrv.arg("fO") + " ",
          sSel = "/" + gD.mSrv.arg("fS"),
          sNm = "/" + gD.mSrv.arg("fN");

  switch( s[0] )
  { case 'L': // Load to current script
        if( sSel.length() < 2 ) break;
        f = SPIFFS.open( sSel, "r");
        if( f ) 
        { gD.mScr = f.readString();
          gD.mbFirstPass = 1;
          f.close();          
        }
      break;

    case 'S': // Save current script      
      s = sNm;
      if( s.length() < 2 ) s = sSel;
      if( s.length() < 2 ) break;
      SPIFFS.remove( s );
      f = SPIFFS.open( s, "w");
      if( f ) 
      {   f.write( (byte*)gD.mScr.c_str(), gD.mScr.length() );
          f.close();
      }
      break;

    case 'P': // Play file
      if( gD.mLedMode == 0 ) break;
      if( sSel.length() < 2 ) break;
      if( strstr( sSel.c_str(), ".bmp" ) || strstr( sSel.c_str(), ".BMP" ) )
      { gD.LedBmpFileChg( 0, sSel.c_str() );
        if( gD.mFlBmp ) gD.FlashWr();
      }
      break;
      
    case 'D': // Delete file
      if( sSel.length() < 2 ) break;
      SPIFFS.remove( sSel );
      break;

    case 'R': // Rename file      
      if( sNm.length() < 2 ) break;
      if( sSel.length() < 2 ) break;
      SPIFFS.remove( sNm );
      SPIFFS.rename( sSel, sNm );
      break;
  }
  // ShowArgs( "-File" );  // Debug WEB output
  gD.WebTxPage( true, P_Set );  
}

//-------------------------------------------------------------------------------

void  CGlobalData::WebInit( void )
{ char    Tb[128];

  mBlinkMode = 0x01;
  BlinkerSet( 500, 1 );
  wifi_station_set_hostname("weblights.wl");

  sprintf( Tb, "\nSSID:%s, Pwd:%s, ", mWF_Id, mWF_Pwd );
  Serial.print( Tb );  
  
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  delay(100);
  // Connect to WiFi network
  if( !mWF_Mode )
  { // Create access point
    IPAddress ip(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);  // set gateway to match your wifi network
    IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your wifi network
    WiFi.config(ip, gateway, subnet);
    Serial.print( "Mode:AccessPoint 192.168.1.1/255.255.255.0/192.168.1.1, IP:" );
    WiFi.softAP( mWF_Id, mWF_Pwd );
    mIP = WiFi.softAPIP();
    mDns.setTTL(300);
    mDns.start( 53, "*", mIP);
  } else 
  { Serial.print( "Mode:Client, IP:" );    
    WiFi.begin( mWF_Id, mWF_Pwd);
        
    // Wait for connection   
    while (WiFi.status() != WL_CONNECTED) 
    { Blinker();
      delay( 5 );
    }
    mIP = WiFi.localIP();
  }

  Serial.print( mIP );
  
   mBlinkMode = 0x02;
   mSrv.onNotFound(handle_root); //send main page to any request
   mSrv.on("/c", handle_cf); // set config
   mSrv.on("/s", handle_sc); // set script
   mSrv.on("/f", HTTP_POST, handle_fl, handle_frx ); //handle file uploads
   mSrv.on("/e", handle_ev); // events
   mSrv.on("/pf", handle_files);  // page-file
   mSrv.on("/ps", handle_scr);    // page-scr
   
   mSrv.begin();
}

//-------------------------------------------------------------------------------

