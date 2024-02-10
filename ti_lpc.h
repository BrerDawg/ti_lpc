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

//ti_lpc.h
//v1.07			




#ifndef ti_lpc_h
#define ti_lpc_h


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
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <math.h>

#define __STDC_FORMAT_MACROS 1		//needed this for msys/mingw
#include <inttypes.h>				//for PRIu64, e.g: 	printf( "sample_count: %" PRIu64 " bytes", sample_cnt ); 	//where 'sample_cnt' is a: uint64_t

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


#include "globals.h"
#include "GCProfile.h"
#include "pref.h"
#include "audio_formats.h"
#include "pcaudio.h"
#include "synth_eng.h"
#include "filter_code.h"
#include "fluid.h"
#include "mgraph.h"
#include "wfm.h"
#include "gc_srateconv.h"
#include "gc_rtaudio.h"


using namespace std;


#define cnFontEditor 4
#define cnSizeEditor 12

#define cnGap 2
#define cnCslHei 125

#define cn_timer_period (0.05f)

//linux code
#ifndef compile_for_windows
#define LLU "llu"
#endif


//windows code
#ifdef compile_for_windows
#define LLU "I64u"
#endif




//linux code
#ifndef compile_for_windows
#define cns_run_shell "run_shell_cmd.sh"		// !! don't use white spaces
#endif

//windows code
#ifdef compile_for_windows
#define cns_run_shell "run_shell_cmd.bat"		// !! don't use white spaces
#endif





extern string pref_aud_editor;
extern string pref_au_filename;
extern void open_shell( string app, string fname );
extern uint8_t *vsm;
extern bool select_rom_file();
extern bool select_rom1_file();
extern void set_new_srate( int sr );
extern void set_new_srate_au( int sr );
extern void stop_play();
extern void cb_help(Fl_Widget *, void *);
extern bool  modify_addr( int dir );
extern void say_lpc_str( string ss );
extern void say_lpc_bytes( uint8_t *buf, int cnt );
extern bool bperiod_6bits;
void cb_fi_lpc_chirp( Fl_Widget *w, void *v );
void cb_bt_tms5100_actual();
void cb_bt_tms5110_actual();
void cb_bt_tms5200_actual();
void cb_bt_tms5220_actual();
void cb_bt_addr_plus_last_actual( Fl_Widget *, void * );
void cb_bt_addr_enter_actual( Fl_Widget *, void * );
void cb_fi_lpc_hex_actual();
void cb_fi_lpc_decimal_actual();
void cb_bt_play_lpc_hex_actual();
void cb_bt_play_lpc_decimal_actual();
void cb_open_audio_editor_actual();
void cb_bt_play_lpc_file_actual();
void cb_fi_addr_step_combo( Fl_Widget *w, void *v );
void cb_bt_addr_step_combo( Fl_Widget *w, void *v );
void cb_bt_addr_dec_inc_val( Fl_Widget *w, void *v );
void cb_bt_addr_hist_prev_next( Fl_Widget *w, void *v );
void addr_add_history( int addr );
void cb_bt_hex_byte_text_file( Fl_Widget *w, void *v );
void cb_bt_tms_code_tables_text_file( Fl_Widget *w, void *v );




extern unsigned int last_say_offset;

void open_audio_editor();

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

extern Talkie talk;


#endif
