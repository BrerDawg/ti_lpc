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

//wfm.h
//v1.01

#ifndef wfm_h
#define wfm_h




#define _FILE_OFFSET_BITS 64			//large file handling, must be before all #include...
//#define _LARGE_FILES

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <string>
#include <vector>
#include <wchar.h>
#include <algorithm>
#include <math.h>
#include <complex>

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

#include "globals.h"
#include "GCProfile.h"
#include "ti_lpc.h"
#include "fluid.h"

//linux code
#ifndef compile_for_windows

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
//#include <X11/Xaw/Form.h>
//#include <X11/Xaw/Command.h>

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

using namespace std;


extern mgraph *gph0;
extern vector<float> gph0_vx;
extern vector<float> gph0_vamp0;
extern vector<float> gph0_vamp1;
extern vector<float> gph0_vamp2;
extern vector<float> gph0_vamp3;
extern vector<float> gph0_vamp4;
extern double gph0_scalex;
extern int gph0_keya;
extern int gph0_keyx;
extern int gph0_keyy;

//extern float gph0_posx;
//extern float gph0_samp_sel;
//extern float gph0_hov_samp;


void add_graphs( Fl_Window* o );
void update_gph0_user_obj();
void update_gph0( vector<float> &v0, vector<float> &v1, vector<float> &v2 );
void save_settings_gph0( string ini );
void load_settings_gph0( string ini );

#endif
