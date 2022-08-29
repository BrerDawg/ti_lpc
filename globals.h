/*
Copyright (C) 2018 BrerDawg

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

//globals.h
//v1.01			
 
#ifndef globals_h
#define globals_h


#define _FILE_OFFSET_BITS 64			//large file handling, must be before all #include...
//#define _LARGE_FILES

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <wchar.h>
#include <algorithm>



//linux code
#ifndef compile_for_windows

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
//#include <X11/Xaw/Form.h>								//64bit
//#include <X11/Xaw/Command.h>							//64bit

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



#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Input_Choice.H>
#include <FL/Fl_Toggle_Button.H>




#include "GCProfile.h"


using namespace std;

#define cnsAppName "ti_lpc"
#define cnsAppWndName "ti_lpc"

#define cns_build_date build_date       //this holds the build date e.g: 2016 Mar 23, obtained automatically by a shell cmd and sets '-Dbuild_date' option in Makefile, you could manually use e.g: -Dbuild_date="\"2016 Mar 23\""



#define cns_help_filename "help.txt"			// !! don't use white spaces

//linux code
#ifndef compile_for_windows
	#define cns_open_editor "open_editor.sh"
	#define cns_open_audio_editor "open_audio_editor.sh"
#endif

//windows code
#ifdef compile_for_windows
	#define cns_open_editor "open_editor.bat"
	#define cns_open_audio_editor "open_audio_editor.bat"
#endif



//linux code
#ifndef compile_for_windows
	#define include_jack					//see also 'b_use_jack'
	#define include_pulseaudio
#endif

#ifdef include_jack
	#include <jack/jack.h>
#endif


#define def_VU0 3276					//default value e.g: for 16 bit audio at -20dBFS = 3276



typedef double flts;					//define numerical calc precision: float/double


#define rom_size_max (16384 * 2)					//up to a 32KB vsm
#define cn_bufsz (48000 * 30)						//30 secs at 48KHz


struct colref
{
int r, g, b;
};

static colref col_blk =   { 0,    0,    0 	};
static colref col_bkgd =   { 64,    64,    64 	};
static colref col_yel = 	{ 255,   255,   0 	};
static colref col_red =	{ 255,   0,     0	};
static colref col_mag =	{ 255,   0,     255	};
static colref col_grey = { 200,   200,   200	};
static colref col_wht =	{ 255,   255,   255	};



/*
struct st_cplex_tag
{
double real;
double imag;
};
*/





class mytexteditor : public Fl_Text_Editor
{
private:
bool left_button;
int wstart;
int wend;

public:											//public functions
mytexteditor( int X, int Y, int W, int H, const char* l );


private:										//private functions
int handle( int );

};




class mytexteditor2 : public Fl_Text_Editor
{
private:
bool left_button;
int wstart;
int wend;

public:											//public functions
mytexteditor2( int X, int Y, int W, int H, const char* l );


private:										//private functions
int handle( int );

};

#endif
