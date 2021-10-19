/*
 * keycodes from winuser.h, ac constants for c language
 * 
 *
 */

#ifndef _WINUSER_H
#ifndef _KEYCODES_H

#define _KEYCODES_H

const int VK_LBUTTON=1;
const int VK_RBUTTON=2;
const int VK_CANCEL=3;
const int VK_MBUTTON=4;
#if (_WIN32_WINNT >= 0x0500)
const int VK_XBUTTON1=5;
const int VK_XBUTTON2=6;
#endif
const int VK_BACK=8;
const int VK_TAB=9;
const int VK_CLEAR=12;
const int VK_RETURN=13;
const int VK_SHIFT=16;
const int VK_CONTROL=17;
const int VK_MENU=18;
const int VK_PAUSE=19;
const int VK_CAPITAL=20;
const int VK_KANA=0x15;
const int VK_HANGEUL=0x15;
const int VK_HANGUL=0x15;
const int VK_JUNJA=0x17;
const int VK_FINAL=0x18;
const int VK_HANJA=0x19;
const int VK_KANJI=0x19;
const int VK_ESCAPE=0x1B;
const int VK_CONVERT=0x1C;
const int VK_NONCONVERT=0x1D;
const int VK_ACCEPT=0x1E;
const int VK_MODECHANGE=0x1F;
const int VK_SPACE=32;
const int VK_PRIOR=33;
const int VK_NEXT=34;
const int VK_END=35;
const int VK_HOME=36;
const int VK_LEFT=37;
const int VK_UP=38;
const int VK_RIGHT=39;
const int VK_DOWN=40;
const int VK_SELECT=41;
const int VK_PRINT=42;
const int VK_EXECUTE=43;
const int VK_SNAPSHOT=44;
const int VK_INSERT=45;
const int VK_DELETE=46;
const int VK_HELP=47;
const int VK_LWIN=0x5B;
const int VK_RWIN=0x5C;
const int VK_APPS=0x5D;
const int VK_SLEEP=0x5F;
const int VK_NUMPAD0=0x60;
const int VK_NUMPAD1=0x61;
const int VK_NUMPAD2=0x62;
const int VK_NUMPAD3=0x63;
const int VK_NUMPAD4=0x64;
const int VK_NUMPAD5=0x65;
const int VK_NUMPAD6=0x66;
const int VK_NUMPAD7=0x67;
const int VK_NUMPAD8=0x68;
const int VK_NUMPAD9=0x69;
const int VK_MULTIPLY=0x6A;
const int VK_ADD=0x6B;
const int VK_SEPARATOR=0x6C;
const int VK_SUBTRACT=0x6D;
const int VK_DECIMAL=0x6E;
const int VK_DIVIDE=0x6F;
const int VK_F1=0x70;
const int VK_F2=0x71;
const int VK_F3=0x72;
const int VK_F4=0x73;
const int VK_F5=0x74;
const int VK_F6=0x75;
const int VK_F7=0x76;
const int VK_F8=0x77;
const int VK_F9=0x78;
const int VK_F10=0x79;
const int VK_F11=0x7A;
const int VK_F12=0x7B;
const int VK_F13=0x7C;
const int VK_F14=0x7D;
const int VK_F15=0x7E;
const int VK_F16=0x7F;
const int VK_F17=0x80;
const int VK_F18=0x81;
const int VK_F19=0x82;
const int VK_F20=0x83;
const int VK_F21=0x84;
const int VK_F22=0x85;
const int VK_F23=0x86;
const int VK_F24=0x87;
const int VK_NUMLOCK=0x90;
const int VK_SCROLL=0x91;
const int VK_LSHIFT=0xA0;
const int VK_RSHIFT=0xA1;
const int VK_LCONTROL=0xA2;
const int VK_RCONTROL=0xA3;
const int VK_LMENU=0xA4;
const int VK_RMENU=0xA5;
#if (_WIN32_WINNT >= 0x0500)
const int VK_BROWSER_BACK=0xA6;
const int VK_BROWSER_FORWARD=0xA7;
const int VK_BROWSER_REFRESH=0xA8;
const int VK_BROWSER_STOP=0xA9;
const int VK_BROWSER_SEARCH=0xAA;
const int VK_BROWSER_FAVORITES=0xAB;
const int VK_BROWSER_HOME=0xAC;
const int VK_VOLUME_MUTE=0xAD;
const int VK_VOLUME_DOWN=0xAE;
const int VK_VOLUME_UP=0xAF;
const int VK_MEDIA_NEXT_TRACK=0xB0;
const int VK_MEDIA_PREV_TRACK=0xB1;
const int VK_MEDIA_STOP=0xB2;
const int VK_MEDIA_PLAY_PAUSE=0xB3;
const int VK_LAUNCH_MAIL=0xB4;
const int VK_LAUNCH_MEDIA_SELECT=0xB5;
const int VK_LAUNCH_APP1=0xB6;
const int VK_LAUNCH_APP2=0xB7;
#endif
const int VK_OEM_1=0xBA;
#if (_WIN32_WINNT >= 0x0500)
const int VK_OEM_PLUS=0xBB;
const int VK_OEM_COMMA=0xBC;
const int VK_OEM_MINUS=0xBD;
const int VK_OEM_PERIOD=0xBE;
#endif
const int VK_OEM_2=0xBF;
const int VK_OEM_3=0xC0;
const int VK_OEM_4=0xDB;
const int VK_OEM_5=0xDC;
const int VK_OEM_6=0xDD;
const int VK_OEM_7=0xDE;
const int VK_OEM_8=0xDF;
#if (_WIN32_WINNT >= 0x0500)
const int VK_OEM_102=0xE2;
#endif
const int VK_PROCESSKEY=0xE5;
#if (_WIN32_WINNT >= 0x0500)
const int VK_PACKET=0xE7;
#endif
const int VK_ATTN=0xF6;
const int VK_CRSEL=0xF7;
const int VK_EXSEL=0xF8;
const int VK_EREOF=0xF9;
const int VK_PLAY=0xFA;
const int VK_ZOOM=0xFB;
const int VK_NONAME=0xFC;
const int VK_PA1=0xFD;
const int VK_OEM_CLEAR=0xFE;
const int TME_HOVER=1;
const int TME_LEAVE=2;
const int TME_QUERY=0x40000000;
const int TME_CANCEL=0x80000000;
const int HOVER_DEFAULT=0xFFFFFFFF;
const int MK_LBUTTON=1;
const int MK_RBUTTON=2;
const int MK_SHIFT=4;
const int MK_CONTROL=8;
const int MK_MBUTTON=16;
#if(_WIN32_WINNT >= 0x0500)
const int MK_XBUTTON1=32;
const int MK_XBUTTON2=64;
#endif

#endif
#endif

