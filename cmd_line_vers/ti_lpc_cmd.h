//ti_lpc_cmd.h

//v1.01


#ifndef ti_lpc_cmd_h
#define ti_lpc_cmd_h

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <string>
#include <vector>
#include <wchar.h>
#include <stdlib.h>
#include <math.h>




//linux code
#ifndef compile_for_windows

//#include <X11/Intrinsic.h>
//#include <X11/StringDefs.h>
//#include <X11/Shell.h>
//#include <X11/Xaw/Form.h>
//#include <X11/Xaw/Command.h>

#define _FILE_OFFSET_BITS 64			//large file handling
//#define _LARGE_FILES
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>		//MakeIniPathFilename(..) needs this
#endif


//windows code
#ifdef compile_for_windows
#include <windows.h>
#include <process.h>
#include <winnls.h>
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <conio.h>

#define WC_ERR_INVALID_CHARS 0x0080		//missing from gcc's winnls.h
#endif



#include "audio_formats.h"



using namespace std;



#define pi ((float)M_PI)
#define twopi (2*(float)M_PI)


#define cns_build_date build_date

#define rom_size_max (16384 * 2)										//up to a 32KB vsm
#define cn_bufsz (48000 * 30)						//30 secs at 48KHz





enum en_mode_user_tag
{
en_mu_none,
en_mu_romlist,
en_mu_render,
en_mu_rendaddrfileline,
en_mu_rendaddrfileseq,
en_mu_rendstrfileline,
en_mu_rendstrfileseq,
en_mu_cleanbrace,
en_mu_cleanquote,
};





class Talkie
{
private:
uint8_t* ptrAddr;
uint8_t ptrBit;
int8_t setup;
public:
bool rev_rom; 					//reset this, to stop reversal of rom bytes just after they are read and before applying to synth code (reverse is set by default)
bool show_lpc;
bool show_frames;
bool tmc0280;					//circa: 1978 lpc synthesis chip

public:
Talkie();
void say(uint8_t* address);
void say_repeat();
void say_tmc0580( uint8_t *buf_lpc, unsigned int offset );
uint8_t rev(uint8_t a);

private:
void setPtr(uint8_t* addr);
uint8_t getBits(uint8_t bits);
};



#endif
