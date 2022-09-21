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

//portions of this code come from the 'Speech library for Arduino' project:  https://github.com/going-digital/Talkie

//ti_lpc.cpp
//v1.01 	01-jun-2018			//
//v1.02 	12-may-2019			//started to incorperate rtaudio
//v1.03 	31-may-2019			//updated to mgraph v1.13
//v1.04 	25-oct-2020			//further fixes
//v1.05 	18-sep-2022			//added more features, such as auto address stepping with delta and delay options, see 'addr_step_auto_val_delay',  'addr_step_auto_val_delta'



#include "ti_lpc.h"


//refer: https://en.wikipedia.org/wiki/Texas_Instruments_LPC_Speech_Chips
//refer: http://www.stuartconner.me.uk/ti_portable_speech_lab/ti_portable_speech_lab.htm

//rom dump site
//refer: http://seanriddle.com/speakandspell.html


//mame coeffs
//refer: https://github.com/mamedev/mame/blob/master/src/devices/sound/tms5110r.hxx

//description of rom and hardware - good
//refer: http://furrtek.free.fr/index.php?a=speakandspell&ss=6&i=2

//talkie coeff discussion
//refer: https://github.com/going-digital/Talkie/issues/6


//pitch details from patent 4331836
//page 22(right) mentions pitch interpolator
//page 12(right) mentions pitch register increment details


//tms5220 emulator for beeb - 
//http://sam.aiki.info/code/beebem-pnd/beebem-0.0.13-x/src/speech.cc

//https://www.google.com.au/search?biw=1746&bih=915&ei=g9kAW63hJ4Op0gTi2IfYBA&q=beeb+emulator+Palazzolo+tms5220&oq=beeb+emulator+Palazzolo+tms5220&gs_l=psy-ab.3...16768.17286.0.17636.4.4.0.0.0.0.0.0..0.0....0...1.1.64.psy-ab..4.0.0....0.20mkwRhrpGY


//global vars
Fl_Double_Window* wndMain;
PrefWnd* pref_wnd=0;
PrefWnd* font_pref_wnd=0;
GCLed *led_addr_step0;

//Fl_Value_Slider *fvs_aud_gain;
string csIniFilename;
int iBorderWidth;
int iBorderHeight;
int maximize_boundary_x, maximize_boundary_y, maximize_boundary_w, maximize_boundary_h;     //resize()
//int restore_size_x, restore_size_y, restore_size_w, restore_size_h;

int gi0,gi1,gi2,gi3,gi4;
double gd0,gd1,gd2,gd3,gd4;
string app_path;						//path this app resides on
string dir_seperator="\\";				//assume dos\windows folder directory seperator
string sglobal;
string sg_col1, sg_col2;
int gi;
int font_num = cnFontEditor;
int font_size = cnSizeEditor;
//static Fl_Font extra_font;
unsigned long long int ns_tim_start1;	//a ns timer start val
double perf_period;						//used for windows timings in timing_start( ), timing_stop( )
string pref_aud_editor;
string slpc_bytes;


int iAppExistDontRun = 1;	//if set, app probably exist and user doesn't want to run another
							//set it to assume we don't want to run another incase user closes
							//wnd without pressing either 'Run Anyway' or 'Don't Run, Exit' buttons


#ifndef compile_for_windows
Display *gDisp;
#endif

//windows code
#ifdef compile_for_windows
PROCESS_INFORMATION processInformation;
#endif

bool inited = 0;

string slast_filename;
string slast_binary_file;
string slast_rom0_filename;
string slast_rom1_filename;

extern unsigned char phrom_rom[16][16384];
int stop_lpc_addr = 0;
int last_render_rom_byte_cnt = 0;							//how many rom byte consumed by the last rendering

bool my_verbose = 0;
int audio_hardware_bits_max_integer = 32767;
int audio_hardware_bits_min_integer = -32768;
int lev_clip_flag;
int lev_clip_ch;
int lev_clip_polarity;
double lev_clip_val;


//linux code
#ifndef compile_for_windows
	bool b_use_jack = 1;							// see also in 'globals.h': #define include_jack
#endif

//windows code
#ifdef compile_for_windows
	bool b_use_jack = 0;							// see also in 'globals.h': #define include_jack
#endif

int pref_use_jack = b_use_jack;

unsigned int framecnt = 4096;
double VU0 = def_VU0;													//integer value req to show 0 VU, e.g: for 16 bit audio at -20dBFS = 3276

#ifdef include_jack
	cjack myjack;											//jack obj
	st_jack_tag st_jack;									//jack variables
#endif

flts *inbuf1, *inbuf2;
flts *outbuf1, *outbuf2;

#ifdef include_jack
	sample_t *jinbuf[ 8 ];
	sample_t *joutbuf[ 8 ];
#endif


float lpc_srate = 8000;												//srate the rom was encoded with


//int srate = cn_srate;
float frame_time = 25e-3;

int generated_samp_cnt = 0;

double twopi = 2.0 * M_PI;



st_fir fir1, fir2;

int dbg_dump = 10;

int bufcnt = 0;
int imin = 0;
int imax = 0;

int lpccnt = 1;										//set this to show first byte has been counted

audio_formats af, af2;
st_audio_formats_tag saf;
unsigned int last_say_offset = 0;



uint8_t periodCounter;
float x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10;
float chirp_step;

Talkie talk;
bool bperiod_6bits = 0;


uint8_t *vsm = 0;											//holds a voice synth memory rom 

float *bufsamp0 = 0;
float *bufsamp1 = 0;
int bufsamp_sz = cn_bufsz;
//int mode = 1;
//int aud_pos = 0;


//function protos
void cb_user1(void *o, int row,int ctrl);
void cb_user2(void *o, int row,int ctrl);
void update_fonts();
void DoQuit();

bool process_audio( int16_t* buf, int bufsz, int &bufptr, bool showframes, bool tmc_0280, float interp_step );
bool process_audio_not_tmc0280( int16_t* buf, int bufsz, int &bufptr, bool showframes );
bool process_audio_tmc0280( int16_t* buf, int16_t* bf2, int16_t* bf3, int bufsz, bool showframes, float interp_step, bool newframe, float chirp_sub_steps, bool skip_interp, float chirp_sub_modulo );
bool build_rom_word_addr_list( int offs, string fname );
int str_to_hex_buf( string ss, uint16_t *bf, int bfsz  );
int str_to_decimal_buf( string ss, int16_t *bf, int bfsz );

void stop_audio();
bool start_audio( void* obj );

void cb_pref(Fl_Widget*w, void* v);
void cb_font_pref(Fl_Widget*w, void* v);
void cb_btAbout(Fl_Widget *, void *);
void cb_help(Fl_Widget *, void *);
void cb_open_audio_editor(Fl_Widget *, void *);
void cb_bt_quit(Fl_Widget *, void *);
void cb_open_file(Fl_Widget *, void *);
void cb_save_file(Fl_Widget *, void *);




int16_t buf[ cn_bufsz ];
int16_t buf2[ cn_bufsz ];
int16_t buf3[ cn_bufsz ];
int buf4[ cn_bufsz ];

float fbuf1[ cn_bufsz ];
float fbuf2[ cn_bufsz ];

int16_t from_k0, from_k1, from_k2, from_k3, from_k4, from_k5, from_k6, from_k7, from_k8, from_k9;
int16_t tgt_k0, tgt_k1, tgt_k2, tgt_k3, tgt_k4, tgt_k5, tgt_k6, tgt_k7, tgt_k8, tgt_k9;
/*
uint8_t tmsEnergy[0x10] = {0x00,0x02,0x03,0x04,0x05,0x07,0x0a,0x0f,0x14,0x20,0x29,0x39,0x51,0x72,0xa1,0xff};
uint8_t tmsPeriod[0x40] = {0x00,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2D,0x2F,0x31,0x33,0x35,0x36,0x39,0x3B,0x3D,0x3F,0x42,0x45,0x47,0x49,0x4D,0x4F,0x51,0x55,0x57,0x5C,0x5F,0x63,0x66,0x6A,0x6E,0x73,0x77,0x7B,0x80,0x85,0x8A,0x8F,0x95,0x9A,0xA0};
int16_t tmsK1[0x20]     = {0x82C0,0x8380,0x83C0,0x8440,0x84C0,0x8540,0x8600,0x8780,0x8880,0x8980,0x8AC0,0x8C00,0x8D40,0x8F00,0x90C0,0x92C0,0x9900,0xA140,0xAB80,0xB840,0xC740,0xD8C0,0xEBC0,0x0000,0x1440,0x2740,0x38C0,0x47C0,0x5480,0x5EC0,0x6700,0x6D40};
int16_t tmsK2[0x20]     = {0xAE00,0xB480,0xBB80,0xC340,0xCB80,0xD440,0xDDC0,0xE780,0xF180,0xFBC0,0x0600,0x1040,0x1A40,0x2400,0x2D40,0x3600,0x3E40,0x45C0,0x4CC0,0x5300,0x5880,0x5DC0,0x6240,0x6640,0x69C0,0x6CC0,0x6F80,0x71C0,0x73C0,0x7580,0x7700,0x7E80};
int8_t tmsK3[0x10]      = {0x92,0x9F,0xAD,0xBA,0xC8,0xD5,0xE3,0xF0,0xFE,0x0B,0x19,0x26,0x34,0x41,0x4F,0x5C};
int8_t tmsK4[0x10]      = {0xAE,0xBC,0xCA,0xD8,0xE6,0xF4,0x01,0x0F,0x1D,0x2B,0x39,0x47,0x55,0x63,0x71,0x7E};
int8_t tmsK5[0x10]      = {0xAE,0xBA,0xC5,0xD1,0xDD,0xE8,0xF4,0xFF,0x0B,0x17,0x22,0x2E,0x39,0x45,0x51,0x5C};
int8_t tmsK6[0x10]      = {0xC0,0xCB,0xD6,0xE1,0xEC,0xF7,0x03,0x0E,0x19,0x24,0x2F,0x3A,0x45,0x50,0x5B,0x66};
int8_t tmsK7[0x10]      = {0xB3,0xBF,0xCB,0xD7,0xE3,0xEF,0xFB,0x07,0x13,0x1F,0x2B,0x37,0x43,0x4F,0x5A,0x66};
int8_t tmsK8[0x08]      = {0xC0,0xD8,0xF0,0x07,0x1F,0x37,0x4F,0x66};
int8_t tmsK9[0x08]      = {0xC0,0xD4,0xE8,0xFC,0x10,0x25,0x39,0x4D};
int8_t tmsK10[0x08]     = {0xCD,0xDF,0xF1,0x04,0x16,0x20,0x3B,0x4D};
*/

//T0280A 0281,  year: ~1978
//see: https://github.com/mamedev/mame/blob/master/src/devices/sound/tms5110r.hxx
int16_t tmsEnergy_0280[ 16 ] = { 0, 0, 1, 1, 2, 3, 5, 7, 10, 15, 21, 30, 43, 61, 86, 0 };
int16_t tmsPeriod_0280[ 64 ] = { 0, 41, 43, 45, 47, 49, 51, 53, 55, 58, 60, 63, 66, 70, 73, 76, 79, 83, 87, 90, 94, 99, 103, 107, 112, 118, 123, 129, 134, 140, 147, 153 };
//coeffs are multiplied by 512
int16_t tms_k0_0280[ 32 ] = { -501, -497, -493, -488, -480, -471, -460, -446, -427, -405, -378, -344, -305, -259, -206, -148, -86, -21, 45, 110, 171, 227, 277, 320, 357, 388, 413, 434, 451, 464, 474, 498 };
int16_t tms_k1_0280[ 32 ] = { -349, -328, -305, -280, -252, -223, -192, -158, -124, -88, -51, -14, 23, 60, 97, 133, 167, 199, 230, 259, 286, 310, 333, 354, 372, 389, 404, 417, 429, 439, 449, 506 };
int16_t tms_k2_0280[ 16 ] = { -397, -365, -327, -282, -229, -170, -104, -36, 35, 104, 169, 228, 281, 326, 364, 396 };
int16_t tms_k3_0280[ 16 ] = { -369, -334, -293, -245, -191, -131, -67, -1, 64, 128, 188, 243, 291, 332, 367, 397 };
int16_t tms_k4_0280[ 16 ] = { -319, -286, -250, -211, -168, -122, -74, -25, 24, 73, 121, 167, 210, 249, 285, 318 };
int16_t tms_k5_0280[ 16 ] = { -290, -252, -209, -163, -114, -62, -9, 44, 97, 147, 194, 238, 278, 313, 344, 371 };
int16_t tms_k6_0280[ 16 ] = { -291, -256, -216, -174, -128, -80, -31, 19, 69, 117, 163, 206, 246, 283, 316, 345 };
int16_t tms_k7_0280[ 8 ] = { -218, -133, -38, 59, 152, 235, 305, 361 };
int16_t tms_k8_0280[ 8 ] = { -226, -157, -82, -3, 76, 151, 220, 280 };
int16_t tms_k9_0280[ 8 ] = { -179, -122, -61, 1, 62, 123, 179, 231  };



//refer https://github.com/mamedev/mame/blob/master/src/devices/sound/tms5110r.hxx

//#define CHIRP_SIZE 41
//int8_t chirp[CHIRP_SIZE] = {0x00,0x2a,0xd4,0x32,0xb2,0x12,0x25,0x14,0x02,0xe1,0xc5,0x02,0x5f,0x5a,0x05,0x0f,0x26,0xfc,0xa5,0xa5,0xd6,0xdd,0xdc,0xfc,0x25,0x2b,0x22,0x21,0x0f,0xff,0xf8,0xee,0xed,0xef,0xf7,0xf6,0xfa,0x00,0x03,0x02,0x01};

#define CHIRP_SIZE_0280 50
//int8_t chirp_0280[ CHIRP_SIZE_0280 ] = { 0x00, 0x2a, 0xd4, 0x32, 0xb2, 0x12, 0x25, 0x14, 0x02, 0xe1, 0xc5, 0x02, 0x5f, 0x5a, 0x05, 0x0f, 0x26, 0xfc, 0xa5, 0xa5, 0xd6, 0xdd, 0xdc, 0xfc, 0x25, 0x2b, 0x22, 0x21, 0x0f, 0xff, 0xf8, 0xee, 0xed, 0xef, 0xf7, 0xf6, 0xfa, 0x00, 0x03, 0x02, 0x01, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

int8_t chirp_0280[ CHIRP_SIZE_0280 + 2] = { 0, 42, 212, 50, 178, 18, 37, 20, 2, 225, 197, 2, 95, 90, 5, 15, 38, 252, 165, 165, 214, 221, 220, 252, 37, 43, 34, 33, 15, 255, 248, 238, 237, 239, 247, 246, 250, 0, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int ichirp_size = CHIRP_SIZE_0280;								//this will change id user modifiesthe chirp byte string in text edit


mystr tim1;

struct st_osc_params_tag							//a pointer to this is stored in stuct passed to audio proc callback, see 'st_rtaud_arg_tag.usr_ptr' 
{
float freq0;
float gain0;

float freq1;	
float gain1;
} st_osc_params;


rtaud rta;																//rtaudio
st_rtaud_arg_tag st_rta_arg;											//this is used in audio proc callback to work out chan in/out counts
double theta0 = 0;
double theta1 = 0;
double theta0_inc;
double theta1_inc;
#define twopi (M_PI * 2)

int dbg_cnt = 0;

int addr_step_auto_state = 0;	//0: stopped,  1: start sounding,  2: sounding,  3: sounding finished, 4: apply delay,  5: apply delta check addr valid - else goto stop,  6: goto step 1
int addr_step_auto_count = 0;
float addr_step_auto_val_delay = cn_timer_period;
int addr_step_auto_val_delta = 1;
float addr_step_auto_delay_sum = 0;

vector<int>vaddr_hist;
int addr_hist_pos = 0;



//----------------------------- Main Menu --------------------------
Fl_Menu_Item menuitems[] =
{
	{ "&File",              0, 0, 0, FL_SUBMENU },
		{ "&OpenFile...", FL_CTRL + 'o'	, (Fl_Callback *)cb_open_file, 0 },
		{ "&SaveFile...", FL_CTRL + 's'	, (Fl_Callback *)cb_save_file, 0, FL_MENU_DIVIDER },
		{ "E&xit", FL_CTRL + 'q'	, (Fl_Callback *)cb_bt_quit, 0 },
		{ 0 },

#ifndef compile_for_windows
	{ "&Edit", 0, 0, 0, FL_SUBMENU },
//		{ "&Undo",  FL_CTRL + 'z', (Fl_Callback *)cb_edit_undo},
//		{ "&Redo",  FL_CTRL + 'y', (Fl_Callback *)cb_edit_redo, 0, FL_MENU_DIVIDER },
//		{ "Cu&t",  FL_CTRL + 'x', (Fl_Callback *)cb_edit_cut },
//		{ "&Copy",  FL_CTRL + 'c', (Fl_Callback *)cb_edit_copy },
//		{ "Past&e",  FL_CTRL + 'v', (Fl_Callback *)cb_edit_paste, 0, FL_MENU_DIVIDER },
		{ "&Preferences..",  0, (Fl_Callback *)cb_pref},
//		{ "&Font Pref..",  0, (Fl_Callback *)cb_font_pref},	
		{ 0 },
#endif

	{ "&Help", 0, 0, 0, FL_SUBMENU },
		{ "'help.txt'", 0				, (Fl_Callback *)cb_help, 0, FL_MENU_DIVIDER  },
		{ "&About", 0				, (Fl_Callback *)cb_btAbout, 0 },
		{ 0 },


	{ 0 }
};
//--------------------------------------------------------------------















//----------------------------------------------------------

mytexteditor::mytexteditor( int X, int Y, int W, int H, const char* l = 0 ) : Fl_Text_Editor( X, Y, W, H, l )
{
left_button = 0;
wstart = 0;
wend = 0;
id0 = 0;
}




int mytexteditor::handle( int e )
{
string s1;
mystr m1;

bool need_redraw = 0;
bool dont_pass_on = 0;

/*
if ( e == FL_MOVE)
	{
//	if( left_button )
		{
		if( ( wstart ) | ( wend ) )
			{
			buffer()->select( wstart, wend );

			s1 = buffer()->selection_text();

			double dd;
			sscanf( s1.c_str(), "%lf", &dd );
	printf("text num: '%s', num: %.17e\n", s1.c_str(), dd );

	//		((mywnd*)parent())->paste_num( dd );
			((mywnd*)parent())->paste_str_num( s1 );

			need_redraw = 0;
			dont_pass_on = 1;
			}
		}
	}
*/




if ( e == FL_PUSH )
	{
	Fl_Text_Editor::handle( e );			//call base function now so it will process mouse click as below code needs it

	if( Fl::event_button() == 1 )
		{
		left_button = 1;
		need_redraw = 1;
//		dont_pass_on = 1;			//note base function already called above
		}

	if( Fl::event_button() == 3 )
		{
		wstart = 0;
		wend = 0;
//		buffer()->highlight( wstart, wend );

		int start, end;
	//	buffer()->selection_position( &start, &end );

		int cursor_pos = insert_position();

		printf("te push: %d, %d, %d\n", start, end, cursor_pos );
		
		char *sz = buffer()->line_text(cursor_pos);
		m1 = sz;
		m1.FindReplace( s1, "0x", "", 0);

		m1 = s1;	
		m1.FindReplace( s1, "\r", "", 0);

		m1 = s1;	
		m1.FindReplace( s1, "\n", "", 0);

		printf("line: '%s'\n", s1.c_str() );
		
		if( s1.find( ":" ) != string::npos )			//comment prefixed?
			{
			m1 = s1;
			m1.cut_just_past_first_find_and_keep_right( s1, ":", 0 );
			}
		say_lpc_str( s1 );

		need_redraw = 1;
//		dont_pass_on = 1;			//note base function already called above
		}
	}


if ( e == FL_RELEASE )
	{
	if( Fl::event_button() == 1 )
		{
		left_button = 0;
		}

	need_redraw = 1;
//	dont_pass_on = 1;
	}



if ( need_redraw ) redraw();


if( dont_pass_on ) return 1;

return Fl_Text_Editor::handle(e);

}


//----------------------------------------------------------



int addr_history_last = 0;

void addr_add_history( int addr )
{
if( addr_history_last == addr ) return;
addr_history_last = addr;

vaddr_hist.push_back( addr );
addr_hist_pos = vaddr_hist.size() - 1;
}



//----------------------------------------------------------

mytexteditor2::mytexteditor2( int X, int Y, int W, int H, const char* l = 0 ) : Fl_Text_Editor( X, Y, W, H, l )
{
left_button = 0;
wstart = 0;
wend = 0;

id0 = 0;
}




int mytexteditor2::handle( int e )
{
string s1, s_addr;

bool need_redraw = 0;
bool dont_pass_on = 0;

/*
if ( e == FL_MOVE)
	{
//	if( left_button )
		{
		if( ( wstart ) | ( wend ) )
			{
			buffer()->select( wstart, wend );

			s1 = buffer()->selection_text();

			double dd;
			sscanf( s1.c_str(), "%lf", &dd );
	printf("text num: '%s', num: %.17e\n", s1.c_str(), dd );

	//		((mywnd*)parent())->paste_num( dd );
			((mywnd*)parent())->paste_str_num( s1 );

			need_redraw = 0;
			dont_pass_on = 1;
			}
		}
	}
*/




if ( e == FL_PUSH )
	{
	Fl_Text_Editor::handle( e );			//call base function now so it will process mouse click as below code needs it

	if( Fl::event_button() == 1 )
		{
		left_button = 1;
		}

	if( Fl::event_button() == 3 )
		{
		wstart = 0;
		wend = 0;
		buffer()->highlight( wstart, wend );

		int start, end;
	//	buffer()->selection_position( &start, &end );

		int cursor_pos = insert_position();

		printf("mytexteditor2::handle() - te push: %d, %d, %d\n", start, end, cursor_pos );



		//mouse clicked, extract a rom address
		unsigned int addr;
		if( buffer()->length() > 0 )
			{
			char cc = buffer()->char_at( cursor_pos );

			if( cc <= ' ' ) goto skip_num_extraction;		//rule out as a possible number
			if( cc == '\t' ) goto skip_num_extraction;
			if( cc > 0x7f ) goto skip_num_extraction;



			int ii = 0;
			for( ii = cursor_pos;  ii >= 0; ii-- )
				{
				cc = buffer()->char_at( ii );

				if( cc == 'a' ) continue;
				if( cc == 'A' ) continue;
				if( cc == 'b' ) continue;
				if( cc == 'B' ) continue;
				if( cc == 'c' ) continue;
				if( cc == 'C' ) continue;
				if( cc == 'd' ) continue;
				if( cc == 'D' ) continue;
				if( cc == 'e' ) continue;
				if( cc == 'E' ) continue;
				if( cc == 'f' ) continue;
				if( cc == 'F' ) continue;

				if( ( cc >= '0' ) & ( cc <= '9' ) ) continue;
				break;
				}

			ii += 1;
			if( ii >= buffer()->length() ) ii = buffer()->length() - 1;
			if( ii < 0 ) ii = 0;

			wstart = ii;
			wend = wstart + 1;

			for( ii = cursor_pos;  ii < buffer()->length(); ii++ )
				{
				cc = buffer()->char_at( ii );
	//			if( cc == '-' ) continue;
	//			if( cc == '+' ) continue;
	//			if( cc == '.' ) continue;
				if( cc == 'a' ) continue;
				if( cc == 'A' ) continue;
				if( cc == 'b' ) continue;
				if( cc == 'B' ) continue;
				if( cc == 'c' ) continue;
				if( cc == 'C' ) continue;
				if( cc == 'd' ) continue;
				if( cc == 'D' ) continue;
				if( cc == 'e' ) continue;
				if( cc == 'E' ) continue;
				if( cc == 'f' ) continue;
				if( cc == 'F' ) continue;

				if( ( cc >= '0' ) & ( cc <= '9' ) ) continue;
				break;
				}

			wend = ii;

			printf("mytexteditor2::handle() - ws: %d, %d\n", wstart, wend );

	//		buffer()->highlight( wstart, wend );
			buffer()->select( wstart, wend );
			s1 = buffer()->selection_text();

			s_addr = s1;
			printf("mytexteditor2::handle() - str: '%s'\n", s1.c_str() );
			sscanf( s1.c_str(), "%x", &addr );
			printf("mytexteditor2::handle() - addr: %x\n", addr );
			}


		if( ( wstart ) || ( wend ) )
			{
			buffer()->select( wstart, wend );

			s1 = buffer()->selection_text();

			printf("mytexteditor2::handle() - num: '%s', num: %04x\n", s1.c_str(), addr );

			printf("mytexteditor2::handle() - vaddr_hist.size() %d, addr %04x\n", vaddr_hist.size(), addr );
			addr_add_history( addr );

			talk.say_tmc0580( vsm, addr );


			if( id0 == 1 )			//NOTE!!!!!!: id0 is set in fluid's gui, see eg: 'tb_wordlist = new Fl_Text_Buffer;' which is defined in fluid
				{
				fi_romaddr->value( s_addr.c_str() );	
				}
			need_redraw = 0;
			dont_pass_on = 1;
			}

	skip_num_extraction:
		int ii = 123;			//dummy
		}

//	need_redraw = 1;
//	dont_pass_on = 1;			//note base function already called above
	}



if ( e == FL_RELEASE )
	{
	if( Fl::event_button() == 1 )
		{
		left_button = 0;
		}

	need_redraw = 1;
	dont_pass_on = 1;
	}



if ( need_redraw ) redraw();


if( dont_pass_on ) return 1;

return Fl_Text_Editor::handle(e);

}


//----------------------------------------------------------



















//console printf(...) like function
//e.g. cslpf( "\nTesing int=%05d, float=%f str=%s\n",z,(float)4.567,"I was Here" );
string sdebug;							//holds all cslpf output 

void cslpf( const char *fmt,... )
{
string s1;

int buf_size =  10 * 1024 * 1024;
char* buf = new char[ buf_size ];				//create a data buffer



if( buf == 0 )
	{
	printf("cslpf() - no memory allocated!!!!!!!!!!!!!!!!!!!!!!!\n");
	return;
	}

string s;
va_list ap;

va_start( ap, fmt );
vsnprintf( buf, buf_size - 1, fmt, ap );
if( 1 )
	{
	printf( buf );
	}


//prevent string growing anywhere near 'max_size()' and causing a std::__throw_length_error(char const*)
unsigned int len = sdebug.length();
unsigned int max = sdebug.max_size();
unsigned int limit = (double)max / 1.25;

//printf("cslpf() - culling %d\n", (int)limit );

if( len > limit )
	{
	s1 = sdebug.substr( max - limit, len - ( max - limit ) );	//cut off begining section of str
	sdebug = s1;
	printf("cslpf() - culling begining of str to avoid hitting string::max_size()\n");
	}
sdebug += buf;								//for diag

va_end(ap);

delete buf;

}







void set_new_srate( int sr )
{

if( (sr >= 2000 )&( sr <= 192000) ) srate = sr;

}



void set_new_srate_au( int sr )
{

if( (sr >= 2000 )&( sr <= 192000) ) srate_au = sr;

}




void stop_play()
{
mode = 0;
}






//returns 1 if addr still valid, else 0
bool modify_addr( int dir )
{
int ii;

bool ret = 1;

string s1 = fi_romaddr->value();

sscanf( s1.c_str(), "%x", &ii );

ii += dir;

if( ii < 0 ) 
	{
	ii = 0;
	ret = 0;
	}

if( ii >= rom_size_max ) 
	{
	ii = rom_size_max - 1;
	ret = 0;
	}

strpf( s1, "%04x", ii );
fi_romaddr->value( s1.c_str() );

last_say_offset = ii;

return ret;
}














//----------------------------------------------------------------------------------
//this is the callback that is called by buttons that specified is in - 
//definition ....pref_wnd->sc.cb=(void*)cb_user;
//'*o' is the PrefWnd* ptr 
//'row' is the row the control lives on, 0 is first row
//'ctrl' is the number of the controlon that row, 0 is first control
void cb_user1(void *o, int row,int ctrl)
{
PrefWnd *w = (PrefWnd *) o;


unsigned int id = w->ctrl_list[ row ][ ctrl ].uniq_id;  //get user id for controll making this call
/*
if( id == 9 )                                   //first led control?
    {
    val[ 0 ][ 5 ]++;
    if( val[ 0 ][ 5 ] >=3 ) val[ 0 ][ 5 ] = 0;  //cycle led states
    
    GCLed *o = (GCLed *)w->ctrl_ptrs[ row ][ ctrl ];
    
    o->ChangeCol( val[ 0 ][ 5 ] );              //change led state
    }
*/

printf("\ncb_user1() - Ping by Control on Row=%d at control count Ctrl=%d on this row, uniq_id: %d\n", row, ctrl, id );

}











void cb_pref_use_jack( void *o, int row, int ctrl )
{
PrefWnd *w = (PrefWnd *) o;
string s1;

unsigned int id = w->ctrl_list[ row ][ ctrl ].uniq_id;  //get user id for controll making this call

if( id == 1 )
    {
	printf("pref_use_jack: %d\n", pref_use_jack );
	if( b_use_jack == pref_use_jack ) 
		{
		if( !b_use_jack ) strpf( s1, "Already using alsa, nothing will be changed." );
		else strpf( s1, "Already using jack, nothing will be changed." );
		fl_alert( s1.c_str(), 0 );
		return;
		}

	if( !pref_use_jack ) strpf( s1, "Will switch to alsa on restart of this app, you need to be sure system has alsa available." );
	else strpf( s1, "Will switch to jack on restart of this app, you need to be sure system has jack available." );
	fl_alert( s1.c_str(), 0 );
	return;
	}

}











void make_pref_wnd()
{
sControl sc;

if(pref_wnd==0)
	{
	pref_wnd = new PrefWnd(wndMain->x()+20,wndMain->y()+20,730,330,"Preferences","Settings","PrefWndPos");
	}
else{
	pref_wnd->Show(1);
	return;
	}



pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

pref_wnd->sc.type=cnStaticTextPref;
pref_wnd->sc.x=200;
pref_wnd->sc.y=0;
pref_wnd->sc.w=150;
pref_wnd->sc.h=20;
pref_wnd->sc.label="Nothing added here as yet";
pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd->sc.label_align = FL_ALIGN_LEFT;
pref_wnd->sc.tooltip="";						//tool tip
pref_wnd->sc.options="";	//menu button drop down options
pref_wnd->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd->sc.textfont=-1;//fl_font();
pref_wnd->sc.textsize=-1;//fl_size();
pref_wnd->sc.section="Settings";
pref_wnd->sc.key="pref_static_text_dummy";
pref_wnd->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd->sc.def="0";							//default to use if ini value not avail
pref_wnd->sc.iretval=(int*)-1;						//address of int to be modified, -1 means none
pref_wnd->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd->sc.cb = cb_user1;					//address of a callback if any, 0 means none
pref_wnd->sc.dynamic = 0;						//allow immediate dynamic change of user var
pref_wnd->sc.uniq_id = 0;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd->AddControl();

pref_wnd->CreateRow(25);			//specify optional row height

/*
pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

pref_wnd->sc.type=cnMenuChoicePref;
pref_wnd->sc.x=140;
pref_wnd->sc.y=0;
pref_wnd->sc.w=150;
pref_wnd->sc.h=20;
pref_wnd->sc.label="Initial Execution:";
pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd->sc.label_align = FL_ALIGN_LEFT;
pref_wnd->sc.tooltip="";						//tool tip
pref_wnd->sc.options="&None,&Step into main(...),&Run";	//menu button drop down options
pref_wnd->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd->sc.textfont=-1;//fl_font();
pref_wnd->sc.textsize=-1;//fl_size();
pref_wnd->sc.section="Settings";
pref_wnd->sc.key="InitExec";
pref_wnd->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd->sc.def="0";							//default to use if ini value not avail
pref_wnd->sc.iretval=&gi0;						//address of int to be modified, -1 means none
pref_wnd->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd->sc.cb = cb_user1;					//address of a callback if any, 0 means none
pref_wnd->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd->sc.uniq_id = 0;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd->AddControl();

pref_wnd->CreateRow(25);			//specify optional row height


pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

pref_wnd->sc.type=cnCheckPref;
pref_wnd->sc.x=200;
pref_wnd->sc.y=0;
pref_wnd->sc.w=300;
pref_wnd->sc.h=20;
pref_wnd->sc.label="some check pref";
pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd->sc.label_align = FL_ALIGN_LEFT;
pref_wnd->sc.tooltip="a tooltip";						//tool tip
pref_wnd->sc.options="";						//menu button drop down options
pref_wnd->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd->sc.textfont=4;
pref_wnd->sc.textsize=12;
pref_wnd->sc.section="Settings";
pref_wnd->sc.key="WatchToFront";
pref_wnd->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd->sc.def="0";							//default to use if ini value not avail
pref_wnd->sc.iretval=&gi1;						//address of int to be modified, -1 means none
pref_wnd->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
pref_wnd->sc.cb = cb_user1;					//address of a callback if any, 0 means none
pref_wnd->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd->sc.uniq_id = 1;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd->AddControl();

pref_wnd->CreateRow(25);					//specify optional row height
*/



/*
pref_wnd->ClearToDefCtrl();			//this will clear sc struct to safe default values

pref_wnd->sc.type = cnCheckPref;
pref_wnd->sc.x = 80;
pref_wnd->sc.y = 0;
pref_wnd->sc.w = 20;
pref_wnd->sc.h = 20;
pref_wnd->sc.label = "UseJack";
pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd->sc.label_align = FL_ALIGN_LEFT;
pref_wnd->sc.tooltip = "Switch to between alsa and jack audio systems, will occur on next restart of app.";						//tool tip
pref_wnd->sc.options = "";						//menu button drop down options
pref_wnd->sc.labelfont = -1;					//-1 means use fltk default
pref_wnd->sc.labelsize = -1;				    //-1 means use fltk default
pref_wnd->sc.textfont = 4;
pref_wnd->sc.textsize = 12;
pref_wnd->sc.ctrl_style = 0;                    //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round
pref_wnd->sc.section = "Pref";
pref_wnd->sc.key = "use_jack";
pref_wnd->sc.keypostfix = -1;					//ini file Key post fix
pref_wnd->sc.def = "1";							//default to use if ini value not avail
pref_wnd->sc.iretval = &pref_use_jack;			//address of int to be modified, -1 means none
pref_wnd->sc.sretval = (string*)-1;				//address of string to be modified, -1 means none
pref_wnd->sc.cb = cb_pref_use_jack;				//address of a callback if any, 0 means none
pref_wnd->sc.dynamic = 1;
pref_wnd->sc.uniq_id = 1;              //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd->AddControl();

pref_wnd->CreateRow();							//specify optional row height
*/


/*
pref_wnd->ClearToDefCtrl();					//this will clear sc struct to safe default values

pref_wnd->sc.type=cnInputPref;
pref_wnd->sc.x=110;
pref_wnd->sc.y=0;
pref_wnd->sc.w=450;
pref_wnd->sc.h=20;
pref_wnd->sc.label="Audio Editor:";
pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd->sc.label_align = FL_ALIGN_LEFT;
pref_wnd->sc.tooltip="Enter a path/filename of audio editor to use";				//tool tip
pref_wnd->sc.options="";						//menu button drop down options
pref_wnd->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd->sc.textfont=-1;
pref_wnd->sc.textsize=-1;
pref_wnd->sc.section="Settings";
pref_wnd->sc.key="AEditor";
pref_wnd->sc.keypostfix=-1;						//ini file Key post fix
pref_wnd->sc.def="mhwaveedit";							//default to use if ini value not avail
pref_wnd->sc.iretval=(int*)-1;						//address of int to be modified, -1 means none
pref_wnd->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
pref_wnd->sc.sretval=&pref_aud_editor;			//address of string to be modified, -1 means none
pref_wnd->sc.cb = cb_user1;						//address of a callback if any, 0 means none
pref_wnd->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd->sc.uniq_id = 3;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd->AddControl();

pref_wnd->CreateRow(25);						//specify optional row height
*/

/*
pref_wnd->sc.type=cnGCColColour;
pref_wnd->sc.x=70;
pref_wnd->sc.y=2;
pref_wnd->sc.w=84;
pref_wnd->sc.h=20;
pref_wnd->sc.label="Colour1";
pref_wnd->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
pref_wnd->sc.label_align = FL_ALIGN_LEFT;
pref_wnd->sc.tooltip="Set r,g,b colour value"; //tool tip
pref_wnd->sc.options="";						//menu button drop down options
pref_wnd->sc.labelfont=-1;						//-1 means use fltk default
pref_wnd->sc.labelsize=-1;						//-1 means use fltk default
pref_wnd->sc.textfont=4;
pref_wnd->sc.textsize=12;
pref_wnd->sc.section="Settings";
pref_wnd->sc.key="Colour1";
pref_wnd->sc.keypostfix=-1;					//ini file Key post fix
pref_wnd->sc.def="255,255,0";						//default to use if ini value not avail
pref_wnd->sc.iretval=(int*)-1;			       	//address of int to be modified, -1 means none
pref_wnd->sc.sretval=&sg_col1;				//address of string to be modified, -1 means none
pref_wnd->sc.cb = cb_user1;					//address of a callback if any, 0 means none
pref_wnd->sc.dynamic = 1;						//allow immediate dynamic change of user var
pref_wnd->sc.uniq_id = 7;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
pref_wnd->AddControl();

pref_wnd->CreateRow(25);					//specify optional row height
*/
pref_wnd->End();								//do end for all windows

}





void cb_pref(Fl_Widget*w, void* v)
{
if(pref_wnd) pref_wnd->Show(1);
}














//this is the callback that is called by buttons that specified is in - 
//definition ....pref_wnd->sc.cb=(void*)cb_user;
//'*o' is the PrefWnd* ptr 
//'row' is the row the control lives on, 0 is first row
//'ctrl' is the number of the controlon that row, 0 is first control
void cb_user3(void *o, int row,int ctrl)
{
PrefWnd *w = (PrefWnd *) o;

unsigned int id = w->ctrl_list[ row ][ ctrl ].uniq_id;

printf("\ncb_user3() - Ping by Control on Row=%d at control count Ctrl=%d on this row, uniq_id: %d\n", row, ctrl, id );

update_fonts();
}





void cb_font_pref(Fl_Widget*w, void* v)
{
if(font_pref_wnd)
	{
	font_pref_wnd->hide();
	font_pref_wnd->Show(0);
	}
}







void update_fonts()
{
//wndMain->redraw();
//fi_unicode->textfont( font_num );
//fi_unicode->textsize( font_size );
//fi_unicode->redraw();

fl_message_font( (Fl_Font) font_num, font_size );

}












void make_font_pref_wnd()
{
sControl sc;


if( font_pref_wnd == 0 )
	{
	font_pref_wnd = new PrefWnd(wndMain->x()+20,wndMain->y()+20,400,115,"Font Preference","Settings","FontPrefWndPos");
	}
else{
	font_pref_wnd->Show(0);
	return;
	}

PrefWnd* o=font_pref_wnd;



string fnames;
string fsizes;



int maxfont = Fl::set_fonts("*");
string s;

for (int i = 0; i < maxfont; i++)
	{
    int t;
    const char *name = Fl::get_font_name((Fl_Font)i,&t);
//    printf("%d: %s\n",i,name);
    strpf(s,"%02d: %s",i,name);
    fnames+=s;
    fnames+=",";
	}


for (int i = 0; i <= 72; i++)
	{
	string s;
	
	strpf(s,"%d",i);
	fsizes+=s;
	fsizes+=",";
	}






// -- dont need to do the below manual default load as "ClearToDefCtrl()" does this for you --

o->ClearToDefCtrl();			//this will clear sc struct to safe default values
o->AddControl();
o->CreateRow(10);				//specify optional row height


o->ClearToDefCtrl();			//this will clear sc struct to safe default values

o->sc.type=cnMenuChoicePref;
o->sc.x=105;
o->sc.y=0;
o->sc.w=250;
o->sc.h=25;
o->sc.label="Font Type:";
o->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
o->sc.label_align = FL_ALIGN_LEFT;
o->sc.tooltip="";						//tool tip
o->sc.options=fnames;	//menu button drop down options
o->sc.labelfont=-1;						//-1 means use fltk default
o->sc.labelsize=-1;						//-1 means use fltk default
o->sc.textfont=-1;
o->sc.textsize=-1;
o->sc.section="Settings";
o->sc.key="FontTypeEditor";
o->sc.keypostfix=-1;					//ini file Key post fix
o->sc.def="0";							//default to use if ini value not avail
o->sc.iretval=&font_num;				//address of int to be modified, -1 means none
o->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
o->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
o->sc.cb=cb_user3;						//address of a callback if any, 0 means none
o->sc.dynamic=1;						//allow immediate dynamic change of user var
o->sc.uniq_id = 0;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
o->AddControl();

o->CreateRow(30);						//specify optional row height

o->ClearToDefCtrl();					//this will clear sc struct to safe default values

o->sc.type=cnMenuChoicePref;
o->sc.x=105;
o->sc.y=0;
o->sc.w=50;
o->sc.h=25;
o->sc.label="Font Size";
o->sc.label_type = FL_NORMAL_LABEL;      //label effects such a FL_EMBOSSED_LABEL, FL_ENGRAVED_LABEL, FL_SHADOW_LABEL
o->sc.label_align = FL_ALIGN_LEFT;
o->sc.tooltip="This is My Tool Tip";	//tool tip
o->sc.options=fsizes;					//menu button drop down options
o->sc.labelfont=-1;						//-1 means use fltk default
o->sc.labelsize=-1;						//-1 means use fltk default
o->sc.textfont=-1;
o->sc.textsize=-1;
o->sc.section="Settings";
o->sc.key="FontSizeEditor";
o->sc.keypostfix=-1;					//ini file Key post fix
o->sc.def="12";							//default to use if ini value not avail
o->sc.iretval=&font_size;			//address of int to be modified, -1 means none
o->sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
o->sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
o->sc.dynamic=1;						//allow immediate dynamic change of user var
o->sc.cb=cb_user3;						//address of a callback if any, 0 means none
o->sc.uniq_id = 1;                      //allows identification of an actual control, rather that using its row and column values, don't use 0xffffffff
o->AddControl();

o->CreateRow(30);						//specify optional row height



o->End();								//do end for all windows

}








//----------------------------------------------------------------------------------




























void LoadSettings(string csIniFilename)
{
string s1, csTmp;
mystr m1;
int x,y,cx,cy;

GCProfile p(csIniFilename);
x=p.GetPrivateProfileLONG("Settings","WinX",100);
y=p.GetPrivateProfileLONG("Settings","WinY",100);
cx=p.GetPrivateProfileLONG("Settings","WinCX",750);
cy=p.GetPrivateProfileLONG("Settings","WinCY",550);

if( x < 0 ) x = 0;
if( y < 0 ) y = 0;

cx = 980;
cy = 940;

wndMain->position( x , y );	
//wndMain->size( cx , cy );	


p.GetPrivateProfileStr( "Settings", "last_filename", "", &slast_filename );
p.GetPrivateProfileStr( "Settings", "last_binary_file", "", &slast_binary_file );



p.GetPrivateProfileStr( "Settings", "last_rom0_filename", "tmc0351n2l.vsm", &slast_rom0_filename );
fi_romfname->value( slast_rom0_filename.c_str() );

p.GetPrivateProfileStr( "Settings", "last_rom1_filename", "tmc0352n2l.vsm", &slast_rom1_filename );
fi_romfname1->value( slast_rom1_filename.c_str() );

s1 = fi_au_fname->value();
p.GetPrivateProfileStr( "Settings", "au_fname", "zz_audio.au", &s1 );
fi_au_fname->value( s1.c_str() );


p.GetPrivateProfileStr( "Settings", "rom_addr", "510", &s1  );
fi_romaddr->value( s1.c_str() );

x=p.GetPrivateProfileLONG("Settings","aud_gain", 50 );
fvs_aud_gain->value( x );
fvs_aud_gain->do_callback();

x=p.GetPrivateProfileLONG("Settings","au_aud_gain", 50 );
fvs_au_aud_gain->value( x );
fvs_au_aud_gain->do_callback();


x=p.GetPrivateProfileLONG("Settings","srate", 48000 );
fi_srate->value( x );
fi_srate->do_callback();


x=p.GetPrivateProfileLONG("Settings","srate_au", 48000 );
fi_srate_au->value( x );
fi_srate_au->do_callback();


p.GetPrivateProfileStr( "Settings", "lpc_data", "paste your vocal lpc hex strings here, see 'help.txt' or select Help from menu\n\nspell: 0E,DC,52,1E,41,02,6B,64,44,80,01,02,D1,B8,5B,70,AC,8E,16,91,6B,F9,78,55,22,A6,DB,F2,0D,37,DE,7C,CB,2D,A7,39,1D,67,1A,77,4E,0B,D2,2F,D9,77,9E,9A,A5,EE,A9,F4,3A,29,D0,F8,24,5B,7C,FD,31,9E,E6,27,59,DD,A6,B4,DC,55,EC,B8,49,72,3B,11,AA,AA,AD,78", &s1 );
m1 = s1;
m1.EscToStr();						       				//convert 'c escaped' to multiline
tb_lpcdata->text( m1.szptr() );


p.GetPrivateProfileStr( "Settings", "chirp", "press buttons on right, try: tms5100", &s1 );
m1 = s1;
m1.EscToStr();						       				//convert 'c escaped' to multiline
tb_chirp->text( m1.szptr() );




p.GetPrivateProfileStr( "Settings", "lpc_bytes", "2B,E5,64,A6,DD,73,6F,BA,E9,E6,53,B2,53,F5,9E,3A,B7,DC,72,CB,29,D5,09,5B,6F,ED,5B,6E,B9,F9,86,ED,A6,EB,B6,DA,6C,AD,79,A2,C0,58,EE,55,91", &s1 );
m1 = s1;
m1.EscToStr();						       				//convert 'c escaped' to multiline
fi_lpc_hex->value( m1.szptr() );


load_settings_gph0( csIniFilename );


p.GetPrivateProfileStr("Settings","address_list", "", &s1 );
m1 = s1;
m1.FindReplace( s1, ",", "\n", 0 );
tb_romaddr->text( s1 .c_str() );



p.GetPrivateProfileStr("Settings","address_list", "", &s1 );
m1 = s1;
m1.FindReplace( s1, ",", "\n", 0 );
tb_romaddr->text( s1 .c_str() );


p.GetPrivateProfileStr("Settings","address_auto_delta", "1", &s1 );
fi_addr_step_delta->value( s1.c_str() );


p.GetPrivateProfileStr("Settings","address_auto_delay", "1.5", &s1 );
fi_addr_step_delay->value( s1.c_str() );

p.GetPrivateProfileStr("Settings","fi_addr_dec_inc_val", "8", &s1 );
fi_addr_dec_inc_val->value( s1.c_str() );



if(pref_wnd!=0) pref_wnd->Load(p);
if(font_pref_wnd!=0) font_pref_wnd->Load(p);

//load_recent_list( csIniFilename );

//b_use_jack = pref_use_jack;
}











void SaveSettings(string csIniFilename)
{
string s1;
mystr m1;

//save_recent_list( csIniFilename );	//do this first so ini is saved before below GCProfile instance is started
//									//this ensures ini is written out to by rlist.save_settings()

{	//scope for GCProfile to save and end of this scope
GCProfile p(csIniFilename);

p.WritePrivateProfileLONG("Settings","WinX", wndMain->x() - iBorderWidth );
p.WritePrivateProfileLONG("Settings","WinY", wndMain->y() - iBorderHeight);
p.WritePrivateProfileLONG("Settings","WinCX", wndMain->w() );
p.WritePrivateProfileLONG("Settings","WinCY", wndMain->h() );


p.WritePrivateProfileStr( "Settings", "last_filename", slast_filename );
p.WritePrivateProfileStr( "Settings", "last_binary_file", slast_binary_file );


slast_rom0_filename = fi_romfname->value();
p.WritePrivateProfileStr("Settings","last_rom0_filename", slast_rom0_filename );

slast_rom1_filename = fi_romfname1->value();
p.WritePrivateProfileStr("Settings","last_rom1_filename", slast_rom1_filename );



s1 = fi_au_fname->value();
p.WritePrivateProfileStr("Settings","au_fname", s1 );


p.WritePrivateProfileLONG("Settings","aud_gain", fvs_aud_gain->value() );

p.WritePrivateProfileLONG("Settings","au_aud_gain", fvs_au_aud_gain->value() );

p.WritePrivateProfileStr( "Settings", "rom_addr", fi_romaddr->value() );

p.WritePrivateProfileLONG( "Settings", "srate", srate );

p.WritePrivateProfileLONG( "Settings", "srate_au", srate_au );

m1 = tb_lpcdata->text();
m1.StrToEscMostCommon1();										//convert multiline to 'c escaped'
p.WritePrivateProfileStr("Settings","lpc_data", m1.str() );


m1 = tb_chirp->text();
m1.StrToEscMostCommon1();										//convert multiline to 'c escaped'
p.WritePrivateProfileStr("Settings","chirp", m1.str() );


m1 = fi_lpc_hex->value();
m1.StrToEscMostCommon1();										//convert multiline to 'c escaped'
p.WritePrivateProfileStr("Settings","lpc_bytes", m1.str() );



m1 = tb_romaddr->text();
m1.FindReplace( s1, "\n", ",", 0 );
p.WritePrivateProfileStr("Settings","address_list", s1 );

s1 = fi_addr_step_delta->value();
p.WritePrivateProfileStr("Settings","address_auto_delta", s1 );

s1 = fi_addr_step_delay->value();
p.WritePrivateProfileStr("Settings","address_auto_delay", s1 );

s1 = fi_addr_dec_inc_val->value();
p.WritePrivateProfileStr("Settings","fi_addr_dec_inc_val", s1 );


if(pref_wnd!=0) pref_wnd->Save(p);
if(font_pref_wnd!=0) font_pref_wnd->Save(p);
}


{
save_settings_gph0( csIniFilename );
}
}




void DoQuit()
{
stop_audio();

SaveSettings(csIniFilename);

if(pref_wnd!=0) delete pref_wnd;

if(vsm) free(vsm);
if(bufsamp0) free(bufsamp0);
if(bufsamp1) free(bufsamp1);

delete_filter( fir1 );
delete_filter( fir2 );

exit(0);
}











//linux code
#ifndef compile_for_windows

//execute shell cmd
int RunShell(string sin)
{

printf("\nRunShell() - '%s'\n", sin.c_str() );

if ( sin.length() == 0 ) return 0;

//make command to cd working dir to app's dir and execute app (params in "", incase of spaces)
//strpf(csCmd,"cd \"%s\";\"%s\" \"%s\"",csPath.c_str(),sEntry[iEntryNum].csStartFunct.c_str(),csFile.c_str());

pid_t child_pid;

child_pid=fork();		//create a child process	

if(child_pid==-1)		//failed to fork?
	{
	printf("\nRunShell() failed to fork\n");
	return 0;
	}

if(child_pid!=0)		//parent fork? i.e. child pid is avail
	{
	int status;
	printf("\nwaitpid: %d, RunShell start\n",child_pid);	

	while(1)
		{
		waitpid(child_pid,&status,0);		//wait for return val from child so a zombie process is not left in system
		printf("\nwaitpid %d RunShell stop\n",child_pid);
		if(WIFEXITED(status)) break;		//confirm status returned shows the child terminated
		}	
	}
else{					//child fork (0) ?
//	printf("\nRunning Shell: %s\n",csCmd.c_str());
	printf("\nRunShell system cmd started: %s\n",sin.c_str());	
	system(sin.c_str());
	printf("\nRunShell system cmd finished \n");	
	exit(1);
	}
return 1;
}

#endif











//windows code
#ifdef compile_for_windows

//execute shell cmd as a process that can be monitored
int RunShell( string sin )
{
BOOL result;
wstring ws1;

printf("\nRunShell() - '%s'\n", sin.c_str() );

if ( sin.length() == 0 ) return 0;


mystr m1 = sin;

m1.mbcstr_wcstr( ws1 );	//convert utf8 string to windows wchar string array


memset(&processInformation, 0, sizeof(processInformation));


STARTUPINFOW StartInfoW; 							// name structure
memset(&StartInfoW, 0, sizeof(StartInfoW));
StartInfoW.cb = sizeof(StartInfoW);

StartInfoW.wShowWindow = SW_HIDE;

result = CreateProcessW( NULL, (WCHAR*)ws1.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfoW, &processInformation);

if ( result == 0)
	{
	
	return 0;
	}

return 1;



//bkup_filelist_SEI[ 0 ].cbSize = sizeof( bkup_filelist_SEI[ 0 ] ); 
//bkup_filelist_SEI[ 0 ].lpVerb = "open"; 
//bkup_filelist_SEI[ 0 ].lpFile = sin.c_str(); 
//bkup_filelist_SEI[ 0 ].lpParameters= 0; 
//bkup_filelist_SEI[ 0 ].nShow = SW_HIDE; 
//bkup_filelist_SEI[ 0 ].fMask = SEE_MASK_NOCLOSEPROCESS; 

//ShellExecuteEx( &bkup_filelist_SEI[ 0 ] );     //execute batch file



//WCHAR cmd[] = L"cmd.exe /c pause";
//LPCWSTR dir = L"c:\\";
//STARTUPINFOW si = { 0 };
//si.cb = sizeof(si);
//PROCESS_INFORMATION pi;

//STARTUPINFO StartInfo; 							// name structure
//PROCESS_INFORMATION ProcInfo; 						// name structure
//memset(&ProcInfo, 0, sizeof(ProcInfo));				// Set up memory block
//memset(&StartInfo, 0 , sizeof(StartInfo)); 			// Set up memory block
//StartInfo.cb = sizeof(StartInfo); 					// Set structure size

//int res = CreateProcess( NULL, (char*)sin.c_str(), 0, 0, TRUE, 0, NULL, NULL, &StartInfo, &ProcInfo );

}

#endif






/*
void open_audio_editor()
{

open_shell( pref_aud_editor, fi_au_fname->value() );
}
*/






//open preference specified editor with supplied fname as parameter 
void open_shell( string app, string fname )
{
string s1;

//pref_aud_editor = "mhwaveedit";

app = pref_aud_editor;

s1 = cns_run_shell;
s1 = "\"";
s1 += pref_aud_editor;
s1 += "\"";
s1 += " ";
s1 += "\"";
s1 += fname;
s1 += "\"&";


//linux code
#ifndef compile_for_windows
RunShell( s1 );
#endif



//windows code
#ifdef compile_for_windows
wstring ws1;
mystr m1 = s1;

m1.mbcstr_wcstr( ws1 );	//convert utf8 string to windows wchar string array

//WCHAR cmd[] = L"cmd.exe /c pause";
//LPCWSTR dir = L"c:\\";
//STARTUPINFOW si = { 0 };
//si.cb = sizeof(si);
//PROCESS_INFORMATION pi;

STARTUPINFOW StartInfoW; 							// name structure
PROCESS_INFORMATION ProcInfo; 						// name structure
memset(&ProcInfo, 0, sizeof(ProcInfo));				// Set up memory block
memset(&StartInfoW, 0 , sizeof(StartInfoW)); 		// Set up memory block
StartInfoW.cb = sizeof(StartInfoW); 				// Set structure size

int res = CreateProcessW(NULL, (WCHAR*)ws1.c_str(), 0, 0, TRUE, CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &StartInfoW, &ProcInfo );

#endif
}









//make sure 'picked' is not the same string as 'pathfilename'
//string 'picked' is loaded with selected path and filename, on 'OK' 
//on 'Cancel' string 'picked' is set to a null string
//returns 1 if 'OK', else 0
//set 'type' to Fl_File_Chooser::CREATE to allow a new filename to be entered

//linux code
#ifndef compile_for_windows
bool my_file_chooser( string &picked, const char* title, const char* pat, const char* start_path_filename, int type, Fl_Font fnt = -1, int fntsze = -1 )
{
picked = "";

//show file chooser
Fl_File_Chooser fc	( 	 start_path_filename,		// directory
						 pat,                       // filter
						 Fl_File_Chooser::SINGLE | type,   // chooser type
						 title						// title
					);

if ( fnt != -1 )fc.textfont( fnt );
if ( fntsze != -1 )fc.textsize( fntsze );

fc.show();

while( fc.shown() )
	{
	Fl::wait();
	}


if( fc.value() == 0 ) return 0;


picked = fc.value();

//windows code
//#ifdef compile_for_windows
//make the slash suit Windows OS
//mystr m1;
//m1 = fc.value();
//m1.FindReplace( picked, "/", "\\",0);
//#endif


return 1;
}
#endif












//windows code
#ifdef compile_for_windows
bool my_file_chooser( string &picked, const char* title, const char* pat, const char* start_path_filename, int type, Fl_Font fnt = -1, int fntsze = -1 )
{
OPENFILENAME ofn;
char szFile[ 8192 ];
string fname;

mystr m1;

m1 = start_path_filename;

m1.ExtractFilename( dir_seperator[ 0 ],  fname );		 //remove path from filename

strncpy( szFile, fname.c_str(), sizeof( szFile ) );		//put supplied fname as default

memset( &ofn, 0, sizeof( ofn ) );

ofn.lStructSize = sizeof ( ofn );
ofn.hwndOwner = NULL ;
ofn.lpstrFile = szFile ;
//ofn.lpstrFile[ 0 ] = '\0';
ofn.nMaxFile = sizeof( szFile );
ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = 0;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = start_path_filename ;
ofn.lpstrTitle = title;
ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR ;

if( type == Fl_File_Chooser::CREATE )
	{
	if( GetSaveFileName( &ofn ) )
		{
		picked = szFile;
		return 1;
		}
	}
else{
	if( GetOpenFileName( &ofn ) )
		{
		picked = szFile;
		return 1;
		}
	}
return 0;
}
#endif








void cb_bt_quit(Fl_Widget *, void *)
{
DoQuit();
}








void cb_open_file(Fl_Widget *, void *)
{
string s1, s2;

s1 = slast_filename;

mystr m1;

unsigned long long int filesz;
if( !m1.filesize( s1, filesz ) )				//need this for windows, else dialog does not get shown
	{
	s1 = "";
	}

if( my_file_chooser( s2, "Open File?", "*", s1.c_str(), 0, font_num, font_size ) )
	{
 //   mystr m1 = s2;
//    m1.ExtractFilename( dir_seperator[ 0 ], s2 );
 

//	cslpf( "You selected file: '%s'\n", s2.c_str() );

	LoadSettings( s2 );

	slast_filename = s2;
	
	build_rom_word_addr_list( 0x0000, slast_rom0_filename );
	build_rom_word_addr_list( 0x4000, slast_rom1_filename );
	say_lpc_str( fi_lpc_hex->value() );
	}
}





void cb_save_file(Fl_Widget *, void *)
{
string s1, s2;

s1 = slast_filename;

if( my_file_chooser( s2, "Save File?", "*", s1.c_str(), Fl_File_Chooser::CREATE, font_num, font_size ) )
	{

	SaveSettings( s2 );

	slast_filename = s2;
	}
}





bool open_file( string &ss )
{
mystr m1;
string s1, s2;

s1 = ss;


unsigned long long int filesz;
if( !m1.filesize( s1, filesz ) )				//need this for windows, else dialog does not get shown
	{
	s1 = "";
	}

if( my_file_chooser( s2, "Select File?", "*", s1.c_str(), 0, font_num, font_size ) )
	{

//	cslpf( "You selected file: '%s'\n", s2.c_str() );


	ss = s2;
	return 1;
	}

return 0;
}













bool select_rom_file()
{
	
if( open_file( slast_rom0_filename ) )
	{
	fi_romfname->value( slast_rom0_filename.c_str() );		
	build_rom_word_addr_list( 0x0000, slast_rom0_filename );
	}
}





bool select_rom1_file()
{
if( open_file( slast_rom1_filename ) )
	{
	fi_romfname1->value( slast_rom1_filename.c_str() );		
	build_rom_word_addr_list( 0x4000, slast_rom1_filename );
	}
}





void cb_btAbout(Fl_Widget *, void *)
{
string s1, st;

Fl_Window *wnd = new Fl_Window(wndMain->x()+20,wndMain->y()+20,500,100);
wnd->label("About");
Fl_Input *teText = new Fl_Input(10,10,wnd->w()-20,wnd->h()-20,"");
teText->type(FL_MULTILINE_OUTPUT);
teText->textsize(12);

strpf( s1, "%s,  %s,  Built: %s\n", cnsAppWndName, "v1.05", cns_build_date );
st += s1;


strpf( s1, "\nTexas Instruments (Speak & Spell) Linear Predictive Decoder Utility" );
st += s1;


//strpf(s,"%s, v1.05, Mar 2011\n\nBasic app skeleton foundation to build onto...",cnsAppWndName);
teText->value(st.c_str());
wnd->end();

#ifndef compile_for_windows
wnd->set_modal();
#endif

wnd->show();

}





void cb_help(Fl_Widget *, void *)
{

string pathname;
pathname = '\"';					//incase path has white spaces

//linux code
#ifndef compile_for_windows
	pathname += app_path;
	pathname += dir_seperator;
#endif

pathname += cns_open_editor;
pathname += '\"';					//incase path has white spaces
pathname += " ";
pathname += cns_help_filename;

printf( "cb_help() - '%s'\n", pathname.c_str() );

stop_audio();							//need this for some reason, else button stays pressed and mouse clicks don't respond, also without this 'rta.stop_stream()' is somehow called (due to 'rta' obj being destroyed), maybe audio proc zombify prob due to shell cmd taking time?
RunShell( pathname );					//do both shell cmds
start_audio( 0 );

}






void cb_open_audio_editor_actual()
{

string pathname;
pathname = '\"';					//incase path has white spaces

//linux code
#ifndef compile_for_windows
	pathname += app_path;
	pathname += dir_seperator;
#endif

pathname += cns_open_audio_editor;
pathname += '\"';					//incase path has white spaces
pathname += " ";
pathname += fi_au_fname->value();

printf( "cb_open_audio_editor_actual() - '%s'\n", pathname.c_str() );

stop_audio();							//need this for some reason, else button stays pressed and mouse clicks don't respond, also without this 'rta.stop_stream()' is somehow called (due to 'rta' obj being destroyed), maybe audio proc zombify prob due to shell cmd taking time?
RunShell( pathname );					//do both shell cmds
start_audio( 0 );

}






void cb_bt_play_lpc_file_actual()
{
uint8_t *bf = new uint8_t[ cn_bufsz ];

string s1, s2, st;
mystr m1;

s1 = slast_binary_file;
unsigned long long int filesz;
if( !m1.filesize( s1, filesz ) )				//need this for windows, else dialog does not get shown
	{
	s1 = "";
	}


if( my_file_chooser( s2, "Open File?", "*", s1.c_str(), 0, font_num, font_size ) )
	{
//    m1.ExtractFilename( dir_seperator[ 0 ], s2 );
 

//	cslpf( "You selected file: '%s'\n", s2.c_str() );

	slast_binary_file = s2;

	FILE *fp = fopen( s2.c_str(), "rb" );

	if( fp == 0 )
		{
		printf("cb_bt_play_lpc_file_actual() - failed to open file: '%s'\n", s2.c_str() );

		delete[] bf;
		return;
		}

	int read = fread( bf, 1, cn_bufsz, fp );

//	if( read != cn_bufsz )
//		{
//		printf("cb_bt_play_lpc_file_actual() - read: %d bytes, requested: %d, file: '%s'\n", read, count, s2.c_str() );
//		}

	printf("cb_bt_play_lpc_file_actual() - read: %d bytes, file: '%s'\n", read, s2.c_str() );

	fclose( fp );

	m1.array_uint8_t_to_hex_str( bf, read, 1, "prefix:", "0x", ",", s1 );
	
	fi_lpc_hex->value( s1.c_str() );
	say_lpc_str( fi_lpc_hex->value() );
	}

return;
}








void cb_bt_tms5100_actual()
{
string s1;

//s1 = "chirp_hx=00,2a,d4,32,b2,12,25,14,02,e1,c5,02,5f,5a,05,0f,26,fc,a5,a5,d6,dd,dc,fc,25,2b,22,21,0f,ff,f8,ee,ed,ef,f7,f6,fa,00,03,02,01,00,00,00,00,00,00,00,00,00";

s1 = "processor=tms5100\n";
s1 += "chirp=0, 42, 212, 50, 178, 18, 37, 20, 2, 225, 197, 2, 95, 90, 5, 15, 38, 252, 165, 165, 214, 221, 220, 252, 37, 43, 34, 33, 15, 255, 248, 238, 237, 239, 247, 246, 250, 0, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0";
s1 += "\n";
s1 += "energy=0, 0, 1, 1, 2, 3, 5, 7, 10, 15, 21, 30, 43, 61, 86, 0";
s1 += "\n";
s1 += "pitch_count=32";
s1 += "\n";
s1 += "pitch=0, 41, 43, 45, 47, 49, 51, 53, 55, 58, 60, 63, 66, 70, 73, 76, 79, 83, 87, 90, 94, 99, 103, 107, 112, 118, 123, 129, 134, 140, 147, 153";
s1 += "\n";
s1 += "k0=-501, -497, -493, -488, -480, -471, -460, -446, -427, -405, -378, -344, -305, -259, -206, -148, -86, -21, 45, 110, 171, 227, 277, 320, 357, 388, 413, 434, 451, 464, 474, 498";
s1 += "\n";
s1 += "k1=-349, -328, -305, -280, -252, -223, -192, -158, -124, -88, -51, -14, 23, 60, 97, 133, 167, 199, 230, 259, 286, 310, 333, 354, 372, 389, 404, 417, 429, 439, 449, 506";
s1 += "\n";
s1 += "k2=-397, -365, -327, -282, -229, -170, -104, -36, 35, 104, 169, 228, 281, 326, 364, 396";
s1 += "\n";
s1 += "k3=-369, -334, -293, -245, -191, -131, -67, -1, 64, 128, 188, 243, 291, 332, 367, 397";
s1 += "\n";
s1 += "k4=-319, -286, -250, -211, -168, -122, -74, -25, 24, 73, 121, 167, 210, 249, 285, 318";
s1 += "\n";
s1 += "k5=-290, -252, -209, -163, -114, -62, -9, 44, 97, 147, 194, 238, 278, 313, 344, 371";
s1 += "\n";
s1 += "k6=-291, -256, -216, -174, -128, -80, -31, 19, 69, 117, 163, 206, 246, 283, 316, 345";
s1 += "\n";
s1 += "k7=-218, -133, -38, 59, 152, 235, 305, 361";
s1 += "\n";
s1 += "k8=-226, -157, -82, -3, 76, 151, 220, 280";
s1 += "\n";
s1 += "k9=-179, -122, -61, 1, 62, 123, 179, 231";
s1 += "\n";

tb_chirp->text( s1.c_str() );
say_lpc_str( fi_lpc_hex->value() );

//talk.say_repeat();
}



//0280_2801 chirp
//0, 41, 43, 45, 47, 49, 51, 53, 55, 58, 60, 63, 66, 70, 73, 76, 79, 83, 87, 90, 94, 99, 103, 107, 112, 118, 123, 129, 134, 140, 147, 153

//2802 chirp
//0, 16, 18, 19, 21, 24, 26, 28, 31, 35, 37, 42, 44, 47, 50, 53, 56, 59, 63, 67, 71, 75, 79, 84, 89, 94, 100, 106, 112, 126, 141, 150

//5110
//0, 15, 16, 17, 19, 21, 22, 25, 26, 29, 32, 36, 40, 42, 46, 50, 55, 60, 64, 68, 72, 76, 80, 84, 86, 93, 101, 110, 120, 132, 144, 159


void cb_bt_tms5110_actual()
{
string s1;

//s1 = "chirp_hx=00,2a,d4,32,b2,12,25,14,02,e1,c5,02,5f,5a,05,0f,26,fc,a5,a5,d6,dd,dc,fc,25,2b,22,21,0f,ff,f8,ee,ed,ef,f7,f6,fa,00,03,02,01,00,00,00,00,00,00,00,00,00";

s1 = "processor=tms5110\n";
s1 += "chirp=0, 16, 18, 19, 21, 24, 26, 28, 31, 35, 37, 42, 44, 47, 50, 53, 56, 59, 63, 67, 71, 75, 79, 84, 89, 94, 100, 106, 112, 126, 141, 150";
s1 += "\n";
s1 += "energy=0, 1, 2, 3, 4, 6, 8, 11, 16, 23, 33, 47, 63, 85, 114, 0";
s1 += "\n";
s1 += "pitch_count=32";
s1 += "\n";
s1 += "pitch=0, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 44, 46, 48, 50, 52, 53, 56, 58, 60, 62, 65, 68, 70, 72, 76, 78, 80, 84, 86, 91, 94, 98, 101, 105, 109, 114, 118, 122, 127, 132, 137, 142, 148, 153, 159";
s1 += "\n";
s1 += "k0=-501, -498, -497, -495, -493, -491, -488, -482, -478, -474, -469, -464, -459, -452, -445, -437, -412, -380, -339, -288, -227, -158, -81, -1, 80, 157, 226, 287, 337, 379, 411, 436";
s1 += "\n";
s1 += "k1=-328, -303, -274, -244, -211, -175, -138, -99, -59, -18, 24, 64, 105, 143, 180, 215, 248, 278, 306, 331, 354, 374, 392, 408, 422, 435, 445, 455, 463, 470, 476, 506";
s1 += "\n";
s1 += "k2=-441, -387, -333, -279, -225, -171, -117, -63, -9, 45, 98, 152, 206, 260, 314, 368";
s1 += "\n";
s1 += "k3=-328, -273, -217, -161, -106, -50, 5, 61, 116, 172, 228, 283, 339, 394, 450, 506";
s1 += "\n";
s1 += "k4=-328, -282, -235, -189, -142, -96, -50, -3, 43, 90, 136, 182, 229, 275, 322, 368";
s1 += "\n";
s1 += "k5=-256, -212, -168, -123, -79, -35, 10, 54, 98, 143, 187, 232, 276, 320, 365, 409";
s1 += "\n";
s1 += "k6=-308, -260, -212, -164, -117, -69, -21, 27, 75, 122, 170, 218, 266, 314, 361, 409";
s1 += "\n";
s1 += "k7=-256, -161, -66, 29, 124, 219, 314, 409";
s1 += "\n";
s1 += "k8=-256, -176, -96, -15, 65, 146, 226, 307";
s1 += "\n";
s1 += "k9=-205, -132, -59, 14, 87, 160, 234, 307";
s1 += "\n";

tb_chirp->text( s1.c_str() );
say_lpc_str( fi_lpc_hex->value() );
//talk.say_repeat();
}








void cb_bt_tms5200_actual()
{
string s1;

s1 = "processor=tms5200\n";
s1 += "chirp_hx=0x00, 0x03, 0x0f, 0x28, 0x4c, 0x6c, 0x71, 0x50, 0x25, 0x26, 0x4c, 0x44, 0x1a, 0x32, 0x3b, 0x13, 0x37, 0x1a, 0x25, 0x1f, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00";
s1 += "\n";
s1 += "energy=0, 1, 2, 3, 4, 6, 8, 11, 16, 23, 33, 47, 63, 85, 114, 0";
s1 += "\n";
s1 += "pitch_count=64";
s1 += "\n";
s1 += "pitch=0, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 34, 36, 38, 40, 41, 43, 45, 48, 49, 51, 54, 55, 57, 60, 62, 64, 68, 72, 74, 76, 81, 85, 87,  90, 96, 99,103, 107, 112, 117, 122, 127, 133, 139, 145, 151, 157, 164, 171, 178, 186, 194, 202, 211";
s1 += "\n";
s1 += "k0=-501, -498, -495, -490, -485, -478, -469, -459,-446, -431, -412, -389, -362, -331, -295, -253, -207, -156, -102, -45, 13, 70, 126, 179, 228, 272, 311, 345, 374, 399, 420, 437";
s1 += "\n";
s1 += "k1=-376, -357, -335, -312, -286, -258, -227, -195, -161, -124, -87, -49, -10,  29,  68, 106, 143, 178, 212, 243, 272, 299, 324, 346, 366, 384, 400, 414, 427, 438, 448, 506";
s1 += "\n";
s1 += "k2=-407, -381, -349, -311, -268, -218, -162, -102, -39, 25, 89, 149, 206, 257, 302, 341";
s1 += "\n";
s1 += "k3=-290, -252, -209, -163, -114, -62, -9, 44, 97, 147, 194, 238, 278, 313, 344, 371";
s1 += "\n";
s1 += "k4=-318, -283, -245, -202, -156, -107, -56, -3, 49, 101, 150, 196, 239, 278, 313, 344";
s1 += "\n";
s1 += "k5=-193, -152, -109, -65, -20,  26, 71, 115, 158, 198, 235, 270, 301, 330, 355, 377";
s1 += "\n";
s1 += "k6=-254, -218, -180, -140, -97, -53, -8, 36, 81, 124, 165, 204, 240, 274, 304, 332";
s1 += "\n";
s1 += "k7=-205, -112, -10, 92, 187, 269, 336, 387";
s1 += "\n";
s1 += "k8=-249, -183, -110, -32, 48, 126, 198, 261";
s1 += "\n";
s1 += "k9=-190, -133, -73, -10, 53, 115, 173, 227";
s1 += "\n";

tb_chirp->text( s1.c_str() );
say_lpc_str( fi_lpc_hex->value() );
//talk.say_repeat();
}




void cb_bt_tms5220_actual()
{
string s1;

//s1 = "chirp_hx=00,2a,d4,32,b2,12,25,14,02,e1,c5,02,5f,5a,05,0f,26,fc,a5,a5,d6,dd,dc,fc,25,2b,22,21,0f,ff,f8,ee,ed,ef,f7,f6,fa,00,03,02,01,00,00,00,00,00,00,00,00,00";

s1 = "processor=tms5220\n";
s1 += "chirp_hx=0x00, 0x03, 0x0f, 0x28, 0x4c, 0x6c, 0x71, 0x50, 0x25, 0x26, 0x4c, 0x44, 0x1a, 0x32, 0x3b, 0x13, 0x37, 0x1a, 0x25, 0x1f, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00";
s1 += "\n";
s1 += "energy=0,  1,  2,  3,  4,  6,  8, 11, 16, 23, 33, 47, 63, 85, 114, 0";
s1 += "\n";
s1 += "pitch_count=64";
s1 += "\n";
s1 += "pitch=0, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 44, 46, 48, 50, 52, 53, 56, 58, 60, 62, 65, 68, 70, 72, 76, 78, 80, 84, 86, 91, 94, 98, 101, 105, 109, 114, 118, 122, 127, 132, 137, 142, 148, 153, 159";
s1 += "\n";
s1 += "k0=-501, -498, -497, -495, -493, -491, -488, -482, -478, -474, -469, -464, -459, -452, -445, -437, -412, -380, -339, -288, -227, -158, -81, -1, 80, 157, 226, 287, 337, 379, 411, 436";
s1 += "\n";
s1 += "k1=-328, -303, -274, -244, -211, -175, -138, -99, -59, -18, 24, 64, 105, 143, 180, 215, 248, 278, 306, 331, 354, 374, 392, 408, 422, 435, 445, 455, 463, 470, 476, 506";
s1 += "\n";
s1 += "k2=-441, -387, -333, -279, -225, -171, -117, -63, -9, 45, 98, 152, 206, 260, 314, 368";
s1 += "\n";
s1 += "k3=-328, -273, -217, -161, -106, -50, 5, 61, 116, 172, 228, 283, 339, 394, 450, 506";
s1 += "\n";
s1 += "k4=-328, -282, -235, -189, -142, -96, -50, -3, 43, 90, 136, 182, 229, 275, 322, 368";
s1 += "\n";
s1 += "k5=-256, -212, -168, -123, -79, -35, 10, 54, 98, 143, 187, 232, 276, 320, 365, 409";
s1 += "\n";
s1 += "k6=-308, -260, -212, -164, -117, -69, -21, 27, 75, 122, 170, 218, 266, 314, 361, 409";
s1 += "\n";
s1 += "k7=-256, -161, -66, 29, 124, 219, 314, 409";
s1 += "\n";
s1 += "k8=-256, -176, -96, -15, 65, 146, 226, 307";
s1 += "\n";
s1 += "k9=-205, -132, -59, 14, 87, 160, 234, 307";
s1 += "\n";

tb_chirp->text( s1.c_str() );
say_lpc_str( fi_lpc_hex->value() );
//talk.say_repeat();
}











void cb_fi_lpc_hex_actual()
{
}




void cb_fi_lpc_decimal_actual()
{
}



void cb_bt_play_lpc_hex_actual()
{

string s1, s2, st;
mystr m1;

uint16_t bf[8192];

s2 = fi_lpc_hex->value();
m1 = s2;

//m1 = "01, af, ff, 1, 256, 257";
int cnt = m1.LoadArray_hex_uint16_t( bf, 8192, ',' );

if( cnt != 0 )
	{
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		strpf( s1, "%d, ", bf[i] );
		st += s1;
		}
//	printf("got: %d\n", cnt );

	fi_lpc_decimal->value( st.c_str() );
	say_lpc_str( fi_lpc_hex->value() );
	}

}





void cb_bt_play_lpc_decimal_actual()
{

string s1, s2, st;
mystr m1;

int8_t bf[8192];

s2 = fi_lpc_decimal->value();
m1 = s2;

//m1 = "01, af, ff, 1, 256, 257";
int cnt = m1.LoadArray_decimal_int8_t( bf, 8192, ',' );

if( cnt != 0 )
	{
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		strpf( s1, "%02x, ", (uint8_t)bf[i] );
		st += s1;
	printf("cb_bt_play_lpc_decimal_actual() - [%03d] = %02x\n", i, (uint8_t)bf[i] );
		}
	printf("cb_bt_play_lpc_decimal_actual() - byte cnt: %d\n", cnt );

	fi_lpc_hex->value( st.c_str() );
	say_lpc_str( fi_lpc_hex->value() );
	}

}




void cb_bt_addr_plus_last_actual( Fl_Widget *, void * )
{
modify_addr( last_render_rom_byte_cnt );
addr_add_history( last_say_offset );

//talk.say_tmc0580( vsm, last_render_rom_byte_cnt );			//'that is correct'

talk.say_repeat();
}





void cb_bt_addr_enter_actual( Fl_Widget *, void * )
{
string s1;

s1 = fi_romaddr->value();

tb_romaddr->append( s1.c_str() );
tb_romaddr->append( "\n" );

//vaddr.push_back(s1);
}









void cb_fi_addr_step_combo( Fl_Widget *w, void *v )
{
string s1;

int which = (intptr_t)v;

printf( "cb_fi_addr_step_combo() - %d\n",  which );		

if( which == 0 )
	{
	addr_step_auto_state = 0;
	fi_addr_step_count->value("000000");
	}
	
if( which == 1 )
	{
	addr_step_auto_state = 1;
	addr_step_auto_count = 0;
	}

}





void cb_bt_addr_step_combo( Fl_Widget *w, void *v )
{
string s1;

int which = (intptr_t)v;

printf( "cb_bt_addr_step_combo() - %d\n",  which );		

if( which == 0 )
	{
	addr_step_auto_state = 1;
	fi_addr_step_count->value("000000");
	}
	
if( which == 1 )
	{
	addr_step_auto_state = 0;
	}

}







void cb_bt_addr_dec_inc_val( Fl_Widget *w, void *v )
{
string s1;

int which = (intptr_t)v;

printf( "cb_bt_addr_dec_inc_val() - %d\n",  which );		


s1 = fi_addr_dec_inc_val->value();
int ii = 0;
sscanf( s1.c_str(), "%d", &ii );

if( which == 0 )
	{
	ii = -ii;
	modify_addr( ii );
	addr_add_history( last_say_offset );
	talk.say_repeat();
	}
	
if( which == 1 )
	{
	modify_addr( ii );
	addr_add_history( last_say_offset );
	talk.say_repeat();
	}

}















void cb_bt_addr_hist_prev_next( Fl_Widget *w, void *v )
{
string s1;

int which = (intptr_t)v;

printf( "cb_bt_addr_hist_prev_next() - %d\n",  which );

if( which == 0 )
	{
	addr_hist_pos--;
	if( addr_hist_pos < 0 ) addr_hist_pos = 0;
	
//printf( "cb_bt_addr_hist_prev_next() - vaddr_hist.size() %d, addr_hist_pos %d\n",  vaddr_hist.size(), addr_hist_pos );
	if( addr_hist_pos < vaddr_hist.size() ) 
		{
		int addr = vaddr_hist[addr_hist_pos];
		strpf( s1, "%04x", addr );
	
		fi_romaddr->value( s1.c_str() );
		talk.say_tmc0580( vsm, addr );

printf( "cb_bt_addr_hist_prev_next() - vaddr_hist.size() %d, addr %04x\n",  vaddr_hist.size(), addr );
		}
	}

if( which == 1 )
	{
	addr_hist_pos++;
	if( addr_hist_pos >= vaddr_hist.size() ) addr_hist_pos = vaddr_hist.size() - 1;
	if( addr_hist_pos < 0 ) addr_hist_pos = 0;
	
//printf( "cb_bt_addr_hist_prev_next() - vaddr_hist.size() %d, addr_hist_pos %d\n",  vaddr_hist.size(), addr_hist_pos );
	if( addr_hist_pos < vaddr_hist.size() ) 
		{
		int addr = vaddr_hist[addr_hist_pos];
		strpf( s1, "%04x", addr );
		
		fi_romaddr->value( s1.c_str() );
		talk.say_tmc0580( vsm, addr );
printf( "cb_bt_addr_hist_prev_next() - vaddr_hist.size() %d, addr %04x\n",  vaddr_hist.size(), addr );
		}
	
	}
}













void cb_btRunAnyway(Fl_Widget *w, void* v)
{
Fl_Widget* wnd=(Fl_Widget*)w;
Fl_Window *win;
win=(Fl_Window*)wnd->parent();

iAppExistDontRun = 0;
win->~Fl_Window();
}






void cb_btDontRunExit(Fl_Widget* w, void* v)
{
Fl_Widget* wnd=(Fl_Widget*)w;
Fl_Window *win;
win=(Fl_Window*)wnd->parent();

win->~Fl_Window();
}






void cb_fi_lpc_chirp( Fl_Widget *w, void *v )
{
}







//linux code
#ifndef compile_for_windows 

//gets its ID, -- fixed memory leak using XFetchName (used XFree) 01-10-10
int FindWindowID(string csName,Display *display,Window &wid)
{
Window root, parent, *children;
unsigned int numWindows = 0;
int iRet=0;

//*display = XOpenDisplay(NULL);
//gDisp = XOpenDisplay(NULL);

//if(cnShowFindResults) printf("\nDispIn %x\n",display);

XQueryTree(gDisp, RootWindow(gDisp,0), &root, &parent, &children, &numWindows);

int i = 0;
for(i=0; i < numWindows ; i++)
	{
//	char *name;
	Window root2, parent2, *children2;
//	XFetchName(display, children[i], &name);

	
	unsigned int numWindows2 = 0;

//	if(cnShowFindResults) if(name) printf("Window name: %s\n", name);

	XQueryTree(display, children[i], &root2, &parent2, &children2, &numWindows2);
	for(int j=0; j < numWindows2 ; j++)
		{
		char *name;
		XFetchName(display, children2[j], &name);

		
//		unsigned int numWindows2 = 0;
//		Window root2, parent2, *children2;
//		XQueryTree(display, RootWindow(display,0), children[i], &parent2, &children2, &numWindows2);
		 
		if(name) 
			{
//		if(cnShowFindResults) printf("    Window2 name: %s  Id=%x\n", name2,children2[j]);

//printf( "win name: '%s'\n", name );

			if(strcmp(csName.c_str(),name)==0)
				{
//				if(cnShowFindResults) printf("Found It................\n");
//				XMoveWindow(display, children2[j], -100, -100);
//				XMoveWindow(display, children2[j], -100, -100);
//				XMoveWindow(display, children2[j], -100, -100);
//				XResizeWindow(display, children2[j], 1088, 612+22);
//				XMoveWindow(*display, children2[j], 1100, 22);
				wid=children2[j];
//				gw=children2[j];
				iRet=1;
//				return 0;
//				if(iRet)
//					{
//					printf("\n\nTrying to Move %x  %x\n\n",gDisp, gw);
//					XMoveWindow(gDisp, gw, 700, 22);
//					return 1;
//					}
				}
			XFree(name);
			}
		}
	 if(children2) XFree(children2);
	}

if(children) XFree(children);
return  iRet;
}

#endif




void BringWindowToFront(string csTitle)
{

//linux code
#ifndef compile_for_windows
Window wid;
if(FindWindowID(csTitle,gDisp,wid))
	{
	XUnmapWindow(gDisp, wid);
	XMapWindow(gDisp, wid);
	XFlush(gDisp);
	}
#endif


//windows code
#ifdef compile_for_windows
HWND hWnd;
//csAppName.LoadString(IDS_APPNAME);

hWnd = FindWindow( 0, cnsAppName );

if( hWnd )
	{
	::BringWindowToTop( hWnd );
//	::SetForegroundWindow( hWnd );
//	::PostMessage(hWnd,WM_MAXIMIZE,0,0);
	::ShowWindow( hWnd, SW_RESTORE );
	}
#endif

}







//linux code
#ifndef compile_for_windows 

//test if window with csAppName already exists, if so create inital main window with
//two options to either run app, or to exit.
//if no window with same name exists returns 0
//if 'exit' option chosen, exit(0) is called and no return happens
//if 'run anyway' option is chosen, returns 1
int CheckInstanceExists(string csAppName)
{
string csTmp;

gDisp = XOpenDisplay(NULL);

Window wid;


if(FindWindowID(csAppName,gDisp,wid))		//a window with same name exists?
	{
	BringWindowToFront( csAppName );

	XCloseDisplay(gDisp);		//added this to see if valgrind showed improvement - it didn't

	Fl_Window *wndInitial = new Fl_Window(50,50,330,90);
	wndInitial->label("Possible Instance Already Running");
	
	Fl_Box *bxHeading = new Fl_Box(10,10,200, 15, "Another Window With Same Name Was Found.");
	bxHeading->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

	strpf(csTmp,"App Name: '%s'",csAppName.c_str()); 
	Fl_Box *bxAppName = new Fl_Box(10,30,200, 15,csTmp.c_str());
	bxAppName->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

	Fl_Button *btRunAnyway = new Fl_Button(25,55,130,25,"Run App Anyway");
	btRunAnyway->labelsize(12);
	btRunAnyway->callback(cb_btRunAnyway,0);

	Fl_Button *btDontRunExit = new Fl_Button(btRunAnyway->x()+btRunAnyway->w()+15,55,130,25,"Don't Run App, Exit");
	btDontRunExit->labelsize(12);
	btDontRunExit->callback(cb_btDontRunExit,0);

	wndInitial->end();
	wndInitial->show();
	
	Fl::run();

	return iAppExistDontRun;
	}
else iAppExistDontRun=0;

XCloseDisplay(gDisp);		//added this to see if valgrind showed improvement - it didn't

return iAppExistDontRun;

}

#endif














//windows code
#ifdef compile_for_windows 

//test if window with csAppName already exists, if so create inital main window with
//two options to either run app, or to exit.
//if no window with same name exists returns 0
//if 'exit' option chosen, exit(0) is called and no return happens
//if 'run anyway' option is chosen, returns 1
int CheckInstanceExists( string csAppName )
{
string csTmp;

HWND hWnd;
//csAppName.LoadString(IDS_APPNAME);

hWnd = FindWindow( 0, csAppName.c_str() );

if( hWnd )
	{
	BringWindowToFront( csAppName );
//	::BringWindowToTop( hWnd );
//::SetForegroundWindow( hWnd );
//::PostMessage(hWnd,WM_MAXIMIZE,0,0);
//	::ShowWindow( hWnd, SW_RESTORE );
Sleep(1000);

	Fl_Window *wndInitial = new Fl_Window(50,50,330,90);
	wndInitial->label("Possible Instance Already Running");
	
	Fl_Box *bxHeading = new Fl_Box(10,10,200, 15, "Another Window With Same Name Was Found.");
	bxHeading->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

	strpf(csTmp,"App Name: '%s'",csAppName.c_str()); 
	Fl_Box *bxAppName = new Fl_Box(10,30,200, 15,csTmp.c_str());
	bxAppName->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

	Fl_Button *btRunAnyway = new Fl_Button(25,55,130,25,"Run App Anyway");
	btRunAnyway->labelsize(12);
	btRunAnyway->callback(cb_btRunAnyway,0);

	Fl_Button *btDontRunExit = new Fl_Button(btRunAnyway->x()+btRunAnyway->w()+15,55,130,25,"Don't Run App, Exit");
	btDontRunExit->labelsize(12);
	btDontRunExit->callback(cb_btDontRunExit,0);

	wndInitial->end();
	wndInitial->show();
	wndInitial->hide();
	wndInitial->show();
	Fl::run();

	return iAppExistDontRun;
	}
else iAppExistDontRun = 0;

return iAppExistDontRun;
}

#endif















/*
//find this apps path and prefix it to supplied filename
void MakeIniPathFilename(string csFilename,string &csPathFilename)
{

//linux code
#ifndef compile_for_windows

//get the actual path this app lives in
#define MAXPATHLEN 1025   // make this larger if you need to

int length;
char fullpath[MAXPATHLEN];

// /proc/self is a symbolic link to the process-ID subdir
// of /proc, e.g. /proc/4323 when the pid of the process
// of this program is 4323.
//
// Inside /proc/<pid> there is a symbolic link to the
// executable that is running as this <pid>.  This symbolic
// link is called "exe".
//
// So if we read the path where the symlink /proc/self/exe
// points to we have the full path of the executable.


length = readlink("/proc/self/exe", fullpath, sizeof(fullpath));
 
// Catch some errors:
if (length < 0)
	{
 
	syslog(LOG_ALERT,"Error resolving symlink /proc/self/exe.\n");
	fprintf(stderr, "Error resolving symlink /proc/self/exe.\n");
	exit(0);
	}

if (length >= MAXPATHLEN)
	{
	syslog(LOG_ALERT, "Path too long. Truncated.\n");
	fprintf(stderr, "Path too long. Truncated.\n");
	exit(0);
	}

//I don't know why, but the string this readlink() function 
//returns is appended with a '@'

fullpath[length] = '\0';      // Strip '@' off the end


//printf("Full path is: %s\n", fullpath);
//syslog(LOG_ALERT,"Full path is: %s\n", fullpath);

string csTmp;

csTmp=fullpath;
size_t found=csTmp.rfind("/");
if (found!=string::npos) csPathFilename=csTmp.substr(0,found);
//syslog(LOG_ALERT,"Path only is: %s\n", csPathFilename.c_str());

csPathFilename+='/';
csPathFilename+=csFilename;	
#endif



//windows code
#ifdef compile_for_windows 
csTmp = GetCommandLine();

size_t found = csTmp.rfind( dir_seperator );
if ( found != string::npos ) csPathFilename  =csTmp.substr( 0, found );

csPathFilename += dir_seperator;
csPathFilename+=csFilename;	


csPathFilename = csFilename;
#endif

}

*/






//extract command line details from windud

//from GCCmdLine::GetAppName()
//eg: prog.com											//no path
//eg. "c:\dos\edit prog.com"							//with path\prog in quotes
//eg. "c:\dos\edit prog.com" c:\dos\junk.txt			//with path\prog in quotes and path\file
//eg. c\dos\edit.com /p /c		(as in screen-savers)	//path\prog and params no quotes

void get_cmd_line( string cmdline, string &path, string &appname, vector<string> &vparams )
{
string stmp;
char ch;

path = "";
appname = "";
vparams.clear();
bool in_str = 0;
bool in_quote = 0;
bool beyond_app_name = 0;
//bool app_name_in_quotes = 0;

//cmdline = "c:/dos/edit prog.com";
//cmdline = "\"c:/dos/edit prog.com\" hello 123";
//cmdline = "c:/dos/edit.com hello 123";

int len =  cmdline.length();

if( len == 0 ) return;

for( int i = 0; i < len; i++ )
	{
	ch = cmdline[ i ];
	
	if( ch == '\"' )									//quote?
		{
		if( in_quote )
			{
			in_quote = 0;								//if here found closing quote
			goto got_param;
			}
		else{
			in_quote = 1;
			}
		}
	else{
		if( ch == ' ' )									//space?
			{
			if( !in_quote )				//if not in quote and space must be end of param
				{
				if( in_str ) goto got_param;
				}
			}
		else{
			in_str = 1;
			}

		if( in_str ) stmp += ch;
		}

	continue;

	got_param:

	in_str = 0;
	if ( beyond_app_name == 0 )					//store where approp
		{
		path = stmp;
		beyond_app_name = 1;
		}
	else{
		//store if not just a space
		if( stmp.compare( " " ) != 0 ) vparams.push_back( stmp );
		}

	stmp = "";
	}


//if here end of params reached, store where approp
if ( beyond_app_name == 0 )
	{
	path = stmp;
	}
else{
	vparams.push_back( stmp );
	}




appname = path;

len = path.length();
if( len == 0 ) return;

int pos = path.rfind( dir_seperator );

if( pos == string::npos )					//no directory path found?
	{
	path = "";	
	return;
	}

if( ( pos + 1 ) < len ) appname = path.substr( pos + 1,  pos + 1 - len );	//extract appname
path = path.substr( 0,  pos );								//extract path


//windows code
#ifdef compile_for_windows 
#endif

}









//find this apps path
void get_app_path( string &path_out )
{
string s1, path;
mystr m1;


//linux code
#ifndef compile_for_windows

//get the actual path this app lives in
#define MAXPATHLEN 1025   // make this larger if you need to

int length;
char fullpath[MAXPATHLEN];

// /proc/self is a symbolic link to the process-ID subdir
// of /proc, e.g. /proc/4323 when the pid of the process
// of this program is 4323.
//
// Inside /proc/<pid> there is a symbolic link to the
// executable that is running as this <pid>.  This symbolic
// link is called "exe".
//
// So if we read the path where the symlink /proc/self/exe
// points to we have the full path of the executable.


length = readlink("/proc/self/exe", fullpath, sizeof(fullpath));
 
// Catch some errors:
if (length < 0)
	{
	syslog(LOG_ALERT,"Error resolving symlink /proc/self/exe.\n");
	fprintf(stderr, "Error resolving symlink /proc/self/exe.\n");
	exit(0);
	}

if (length >= MAXPATHLEN)
	{
	syslog(LOG_ALERT, "Path too long. Truncated.\n");
	fprintf(stderr, "Path too long. Truncated.\n");
	exit(0);
	}

//I don't know why, but the string this readlink() function 
//returns is appended with a '@'

fullpath[length] = '\0';      // Strip '@' off the end


//printf("Full path is: %s\n", fullpath);
//syslog(LOG_ALERT,"Full path is: %s\n", fullpath);

path = fullpath;
size_t found = path.rfind( "/" );
if ( found != string::npos ) path_out = path.substr( 0, found );
//syslog(LOG_ALERT,"Path only is: %s\n", csPathFilename.c_str());

#endif



//windows code
#ifdef compile_for_windows 
UINT i,uiLen;                    //eg. c\dos\edit.com /p /c		(as in screen-savers)
bool bQuotes;
string csCmdLineStr;


//----------------------------
//from GCCmdLine::GetAppName()
//eg: prog.com											//no path
//eg. "c:\dos\edit prog.com"							//with path\prog in quotes
//eg. "c:\dos\edit prog.com" c:\dos\junk.txt			//with path\prog in quotes and path\file
//eg. c\dos\edit.com /p /c		(as in screen-savers)	//path\prog and params no quotes
csCmdLineStr = GetCommandLine();

//csCmdLineStr = "skeleton.exe abc";
//printf("csCmdLineStr= '%s'\n", csCmdLineStr.c_str() );


string appname;
vector<string> vparams;
get_cmd_line( csCmdLineStr, path_out, appname, vparams  );

#endif


printf( "csPathFilename= %s\n", path_out.c_str() );

}














//audio card will call this when it has audio data to offer - audio data from mic/line i/p, from A/D
int cb_audio_process_in( unsigned int bufid, unsigned int frames, void* arg )
{

if( !b_use_jack )
	{
	inbuf1 = audio_card_get_inbuf( 0 );
	inbuf2 = audio_card_get_inbuf( 1 );

	if( inbuf1 ) audio_card_prepare_in( 0 );
	if( inbuf2 ) audio_card_prepare_in( 1 );
	}

for( int i = 0; i < frames; i++ )							//audio buf load
	{
	float fl, fr;
	
	if( !b_use_jack )
		{
		fl = inbuf1[ i ];
		fr = inbuf2[ i ];
		}
	else{
		#ifdef include_jack
		fl = jinbuf[ 0 ][ i ];
		fr = jinbuf[ 1 ][ i ];
		#endif
		}
	}

}










bool flag_stop_audio = 0;

//-------------------- realtime audio proc -----------------------------
int cb_audio_proc_rtaudio( void *bf_out, void *bf_in, int frames, double streamTime, RtAudioStreamStatus status, void *arg_in )
{
bool vb = 1;

double dt = tim1.time_passed( tim1.ns_tim_start );
tim1.time_start( tim1.ns_tim_start );

if( !(dbg_cnt%40) )
	{
//	if(vb) printf( "cb_audio_proc_rtaudio() - dt: %f, frames: %d\n", dt, frames );
	}
	

if ( status ) std::cout << "cb_audio_proc_rtaudio() - Stream over/underflow detected." << std::endl;



float *bfin = (float *) bf_in;											//depends on which 'audio_format' data type, refer: 'RtAudioFormat'
float *bfout = (float *) bf_out;


st_rtaud_arg_tag *arg = (st_rtaud_arg_tag*) arg_in;

st_osc_params_tag *usr = (st_osc_params_tag*) arg->usr_ptr;


theta0_inc = usr->freq0 * twopi / arg->srate;
theta1_inc = usr->freq1 * twopi / arg->srate;


//printf( "inout()- frames %d\n ",frames );
//printf( "inout()- arg->channels %d\n ",arg->ch_outs );
//return 0;

dbg_cnt++;
if( !(dbg_cnt%40) )
	{
//printf( "cb arg: %08x\n", (unsigned int)arg );
//	printf( "inout()- arg.bufsiz: %d\n", arg->bufsiz );
//	if(vb) printf( "cb_audio_proc_rtaudio()- arg.ch_ins: %d\n", arg->ch_ins );
//	if(vb) printf( "cb_audio_proc_rtaudio()- arg.ch_outs: %d\n",arg->ch_outs );
//	if(vb) printf( "cb_audio_proc_rtaudio()- arg.audio_format: 0x%08x\n\n",arg->audio_format );
	}

float gain = aud_gain / 100.0;

int idx_in = 0;
int idx_out = 0;

//below handles a max 2 channels of either/or input and output
for( int i = 0; i < frames; i++ )
	{
	if( 0 )				//test tone out ?
		{
		float in0, in1, tone;
		
		in0 = in1 = 0;
		
		if ( arg->ch_ins != 0 ) in0 = 0.2 * bfin[ idx_in++ ];			//input samples?
		if ( arg->ch_ins == 2 ) in1 = 0.2 * bfin[ idx_in++ ];
		
		if ( arg->ch_outs != 0 )										//output?
			{
			tone = in0 + sin( theta0 );
			bfout[ idx_out++ ] = tone * usr->gain0;
			
			theta0 += theta0_inc;
			if( theta0 >= twopi ) theta0 -= twopi;
			}

		if ( arg->ch_outs == 2 )										//2nd output?
			{
			tone = in1 + sin( theta1 );									//mix
			bfout[ idx_out++ ] = tone * usr->gain1;
			
			theta1 += theta1_inc;
			if( theta1 >= twopi ) theta1 -= twopi;
			}
		}
	else{			//lpc out ?
		float lft, rght;
		if( mode )														//playing?
			{
			//do loop recue if req
			if( aud_pos < generated_samp_cnt  )
				{
				lft = gain * bufsamp0[ aud_pos ];
				rght =  gain * bufsamp1[ aud_pos++ ];
				flag_stop_audio = 0;
				}
			else{
				lft = 0;
				rght = 0;
	//			printf("mute_it\n");
				flag_stop_audio = 1;													//flag audio output to be stopped
				}

			if ( arg->ch_outs != 0 ) bfout[ idx_out++ ] = lft;
			if ( arg->ch_outs == 2 ) bfout[ idx_out++ ] = rght;
//			printf("here0\n");
			}
		else{
//			printf("here1\n");
			if ( arg->ch_outs != 0 ) bfout[ idx_out++ ] = 0;
			if ( arg->ch_outs == 2 ) bfout[ idx_out++ ] = 0;
			}	
		}
	}

if(flag_stop_audio)
	{
	flag_stop_audio = 0;
	mode = 0;
	aud_pos = 0;
printf("cb_audio_proc_rtaudio() - flag_stop_audio %d\n", flag_stop_audio );
	}

//  unsigned int *bytes = (unsigned int *) data;
//  memcpy( outputBuffer, inputBuffer, *bytes );
 return 0;
}

//----------------------------------------------------------------------













double theta2 = 0;


//audio card will call this when more audio sample data is needed - audio data to line/spkr o/p, to D/A
int cb_audio_process_out( unsigned int bufid, unsigned int frames, void *arg )
{

#ifdef include_jack
	st_jack_tag *st_args = (st_jack_tag*) arg;
#endif
//printf("arg: %d, %x\n", (int)st_args->user_arg, (int)st_args->outbuf1 );

//return 1;

//timer1.time_start( timer1.ns_tim_start );

if( !b_use_jack )
	{
	outbuf1 = audio_card_get_outbuf( 0 );
	outbuf2 = audio_card_get_outbuf( 1 );
	}
else{
	}
//printf( "aud_ptr: %u\n", aud_ptr );


float cos1;	

theta1_inc = 400.0 * twopi / srate;

//float aud_gain = 0.6;

float gain = aud_gain / 100.0;

bool aud_mute = 0;

if( aud_mute ) gain = 0;


float lft = 0;
float rght = 0;
int aud_channels = 2;

if( mode )														//playing?
	{
	for( int i = 0; i < frames; i++ )							//audio buf load
		{

		if( aud_pos < generated_samp_cnt  )
			{
			lft = bufsamp0[ aud_pos ];
			rght = bufsamp1[ aud_pos++ ];
			flag_stop_audio = 0;
			}
		else{
//			printf("mute_it\n");
			flag_stop_audio = 1;													//flag audio output to be stopped
			//goto mute_it;
			}

		if( 0 )													//tone
			{
			cos1 = cos( theta2 );	

			lft = cos1 * 0.5;
			rght = cos1 * 0.5;

			theta2 += theta1_inc;
			if( theta2 >= twopi ) theta2 -= twopi;
			}

		if( aud_channels != 2 )									//mono?
			{
			rght = lft;
			}

bool mix_in = 0;

	if( mix_in )
		{
		if( !b_use_jack )
			{
			lft += inbuf1[ i ];
			rght += inbuf2[ i ];
			}
		else{
			#ifdef include_jack
			lft += jinbuf[ 0 ][ i ];
			rght += jinbuf[ 1 ][ i ];
			#endif
			}
		}

		if( !b_use_jack )
			{
			outbuf1[ i ] = lft * gain;
			outbuf2[ i ] = rght * gain;
			}
		else{
			#ifdef include_jack
			joutbuf[ 0 ][ i ] = lft * gain;
			joutbuf[ 1 ][ i ] = rght * gain;
			#endif
			}
		}
	}
else{
mute_it:															//stopped
	for ( unsigned int  i = 0; i < frames; i++ )
		{
		if( !b_use_jack )
			{
			outbuf1[ i ] = 0;
			outbuf2[ i ] = 0;
			}
		else{
			#ifdef include_jack
			joutbuf[ 0 ][ i ] = 0;
			joutbuf[ 1 ][ i ] = 0;
			#endif
			}
		}
	}


if( !b_use_jack )
	{
	audio_card_prepare_out( 0 );									//load audio
	audio_card_prepare_out( 1 );
	
	}


if(flag_stop_audio)
	{
	flag_stop_audio = 0;
	mode = 0;
	aud_pos = 0;
printf("cb_audio_process_out() - flag_stop_audio %d\n", flag_stop_audio );
	}
//printf("cos1: %f\n", theta1 );
}






void stop_audio()
{
cslpf( "stop_audio()\n" );

rta.stop_stream();
}






void stop_audio_old()
{
cslpf( "stop_audio()\n" );

if( b_use_jack )
	{
	#ifdef include_jack
	myjack.stop_jack();
	#endif
	}
else{
	audio_card_close();
	}
}




bool start_audio( void* obj )
{
string s1;

cslpf( "start_audio()\n" );

rta.verbose = 1;

//rtaudio_probe();									//dump a list of devices to console


RtAudio::StreamOptions options;

options.streamName = cnsAppName;
options.numberOfBuffers = 2;						//for jack (at least) this is hard set by jack's settings and can't be changed via this parameter

options.flags = 0; 	//0 means interleaved, use oring options, refer 'RtAudioStreamFlags': RTAUDIO_NONINTERLEAVED, RTAUDIO_MINIMIZE_LATENCY, RTAUDIO_HOG_DEVICE, RTAUDIO_SCHEDULE_REALTIME, RTAUDIO_ALSA_USE_DEFAULT, RTAUDIO_JACK_DONT_CONNECT
					// !!! when using RTAUDIO_JACK_DONT_CONNECT, you can create any number of channels, you can't do this if auto connecting as 'openStream()' will fail if there is not enough channel mating ports  

options.priority = 0;								//only used with flag 'RTAUDIO_SCHEDULE_REALTIME'



uint16_t device_num_out = 0;						//use 0 for default device to be used
int channels_out = 2;								//if auto connecting (not RTAUDIO_JACK_DONT_CONNECT), there must be enough mathing ports or 'openStream()' will fail
int first_chan_out = 0;

int srate = 48000;
uint16_t frames = 2048;
unsigned int audio_format = RTAUDIO_FLOAT32;		//see rtaudio docs 'RtAudioFormat' for supported format types, adj audio proc code to suit

st_osc_params.freq0 = 200;							//set up some audio proc callback user params
st_osc_params.gain0 = 0.1;

st_osc_params.freq1 = 600;
st_osc_params.gain1 = 0.1;
st_rta_arg.usr_ptr = (void*)&st_osc_params;



if( !rta.start_stream_out( device_num_out, channels_out, first_chan_out, srate, frames, audio_format, &options, &st_rta_arg, (void*)cb_audio_proc_rtaudio ) )		//output only
	{
	printf("failed to open audio device!!!!\n" );	
	}
else{
	printf("audio out device %d opened, srate is: %d, framecnt: %d\n", device_num_out, rta.get_srate(), rta.get_framecnt()  );							 //output only	
	}

//printf("\npress a key\n" );	
//getchar();											//wait here and do nothing, audio proc will be generating audio samples
return 1;
}






bool start_audio_old( void* obj )
{
string s1;

cslpf( "start_audio()\n" );


if( b_use_jack)
	{
	#ifdef include_jack

	//load up a struct with jack vars, this will be sent to callback
	st_jack.cjack_obj = &myjack;
	st_jack.inbuf[ 0 ] = &jinbuf[ 0 ];						//one for each chan
	st_jack.inbuf[ 1 ] = &jinbuf[ 1 ];
	st_jack.inbuf[ 2 ] = &jinbuf[ 2 ];
	st_jack.outbuf[ 0 ] = &joutbuf[ 0 ];					//one for each chan
	st_jack.outbuf[ 1 ] = &joutbuf[ 1 ];
	st_jack.outbuf[ 2 ] = &joutbuf[ 2 ];

	st_jack.cb_ptr_process_in = cb_audio_process_in;		//user audio sample process callback, MUST SET to zero if no callback to occur
	st_jack.cb_ptr_process_out = cb_audio_process_out;		//user audio sample process callback
	st_jack.user_arg = (void*)1234567;						//any (void*) can go here

	if( !myjack.start_jack( "MyClient", 3, 3, 1, framecnt, &st_jack ) )
		{
		cslpf("start_audio() - failed to start jack\n" );
		strpf( s1, "Failed to open audio hardware, check: 'jackd -d alsa [-r samplerate]' server is running,\nor de-select jack in preferences for alsa to be used\n" );
		fl_alert( s1.c_str(), 0 );
		return 0;
		}
	#else
		cslpf("start_audio() - jack audio was not compiled, set: '#define include_jack', and recompile,\nor de-select jack in preferences for alsa to be used\n" );
		strpf( s1, "Failed to open audio hardware, jack has not been compiled into this app,\nset: '#define include_jack' and recompile,\nor de-select jack in preferences for alsa to be used" );
		fl_alert( s1.c_str(), 0 );
		return 0;
	#endif
	}
else{
	printf("srate0: %d\n", srate );
	if ( !audio_card_open_out( srate, 2, 16, framecnt, cb_audio_process_out, (void*)obj ) )
		{
		cslpf( "start_audio() - can't init audio card for output - failed\n" );
		strpf( s1, "Failed to open audio hardware, check alsa is not busy and restart app: /dev/dsp/,\nif you've compiled jack into this app with: '#define include_jack',\nand jack is running, select jack in preferences" );
		fl_alert( s1.c_str(), 0 );
		return 0;
		}


	if ( !audio_card_open_in( srate, 2, 16, framecnt, cb_audio_process_in, (void*)obj ) )
		{
		cslpf( "start_audio() - can't init audio card for input - failed\n" );
		strpf( s1, "Failed to open audio hardware, check alsa is not busy and restart app: /dev/dsp/" );
		fl_alert( s1.c_str(), 0 );
		return 0;
		}
	}
	unsigned int sr;

	if( !b_use_jack)
		{
		if( !audio_card_get_srate( sr ) )
			{
			cslpf( "start_audio() - can't get audio card srate - failed\n" );
			strpf( s1, "Can't get audio card srate - failed" );
			fl_alert( s1.c_str(), 0 );
			return 0;
			}
		}
	else{
	#ifdef include_jack
		sr = myjack.get_srate();
	#endif
		}

cslpf( "audio hardware samplerate: %d\n", sr );

if( sr != srate )
	{
	cslpf( "audio samplerate mismatch, pc set to: %d, audio file has: %d\nexpect problems,\nif running jack, try: 'jackd -d alsa -r samplerate'\n", sr, srate );
	strpf( s1, "audio samplerate mismatch, pc set to: %d, audio file has: %d,\nexpect problems,\nif running jack, try: 'jackd -d alsa -r samplerate'", sr, srate );
	fl_alert( s1.c_str(), 0 );
	}
//	srate = sr;
audio_card_set_vu0( VU0 );
	

return 1;
}



















Talkie::Talkie() 
{
rev_rom = 1;
show_lpc = 0;
show_frames = 0;
tmc0280 = 0;
}


uint8_t *last_ptraddr;


void Talkie::setPtr(uint8_t* addr) 
{
ptrAddr = addr;
ptrBit = 0;
lpccnt = 1;												//set this to show first byte has been counted
	
last_ptraddr = ptrAddr - 1;								//set up a flag to determine when a new lpc memory location is accessed, pick any lower value initially

if(show_lpc) printf( "setPtr: %02x\n", *ptrAddr );
}



// The ROMs used with the TI speech were serial, not byte wide.
// Here's a handy routine to flip ROM data which is usually reversed.
//i.e: 0x80 becomes: 0x01, if 'rev_rom' is set

uint8_t Talkie::rev(uint8_t a) 
{
if( !rev_rom ) return a;

// 76543210
a = (a>>4) | (a<<4); // Swap in groups of 4
// 32107654
a = ((a & 0xcc)>>2) | ((a & 0x33)<<2); // Swap in groups of 2
// 10325476
a = ((a & 0xaa)>>1) | ((a & 0x55)<<1); // Swap bit pairs
// 01234567
return a;
}







uint8_t Talkie::getBits(uint8_t bits) 
{
uint8_t value;
uint16_t data;
	
if(show_lpc) printf( "rom addr: %04x %02x\n", ptrAddr - vsm, *ptrAddr );



if( ptrAddr != last_ptraddr )				//new addr loc?
	{
//	string s1;
//	if( slpc_bytes.length() == 0 ) strpf( s1, "%02X", *ptrAddr );
//	else  strpf( s1, ",%02X", *ptrAddr );
//	slpc_bytes += s1;

//	last_ptraddr = ptrAddr;
	}

data = rev(*ptrAddr)<<8;
if (ptrBit+bits > 8)
	{
	data |= rev(*(ptrAddr+1));
	}

data <<= ptrBit;
value = data >> (16-bits);
ptrBit += bits;
if (ptrBit >= 8)
	{
	ptrBit -= 8;
	ptrAddr++;
	lpccnt++;

	string s1;
	strpf( s1, ",%02X", *ptrAddr );
	slpc_bytes += s1;
	}
return value;
}





/*

uint8_t Talkie::getBits_old(uint8_t bits) 
{
uint8_t value;
uint16_t data;
	
if(show_lpc) printf( "rom addr: %04x %02x\n", ptrAddr - vsm, *ptrAddr );



if( ptrAddr != last_ptraddr )				//new addr loc?
	{
	string s1;
	if( slpc_bytes.length() == 0 ) strpf( s1, "%02X", *ptrAddr );
	else  strpf( s1, ",%02X", *ptrAddr );
	slpc_bytes += s1;

	last_ptraddr = ptrAddr;
	}

data = rev(*ptrAddr)<<8;
if (ptrBit+bits > 8)
	{
	data |= rev(*(ptrAddr+1));
	}

data <<= ptrBit;
value = data >> (16-bits);
ptrBit += bits;
if (ptrBit >= 8)
	{
	ptrBit -= 8;
	ptrAddr++;
	lpccnt++;
	}
return value;
}
*/








int phrase_cnt = 0;

bool process_audio( int16_t* buf, int bufsz, int &bufptr, bool showframes, bool tmc_0280, float interp_step ) 
{
if( tmc_0280 ) 
	{
//	return process_audio_tmc0280( buf, bufsz, showframes, interp_step );
	}
else{
//	return process_audio_not_tmc0280( buf, bufsz, bufptr, showframes );
	}
}












void Talkie::say_repeat() 
{
say_tmc0580( vsm, last_say_offset );
}











fast_mgraph fgph;

static uint16_t synthRand = 1;
int frame_cnt = 0;
float time_cur = 0;
int temp_smple_cnt = 0;







void say_lpc_bytes( uint8_t *bf, int cnt )
{
if( cnt != 0 )
	{
	talk.say_tmc0580( bf, 0 );
	}
}






//comma delimited
void say_lpc_str( string ss )
{
string s1;
mystr m1;

s1 = ss;
if( ss.find( ":" ) != string::npos )			//comment prefixed?
	{
	m1 = ss;
	m1.cut_just_past_first_find_and_keep_right( s1, ":", 0 );
	}

uint8_t bf[8192];

m1 = s1;

//m1 = "01, af, ff, 1, 256, 257";
int cnt = m1.LoadArray_hex_uint8_t( bf, 8192, ',' );

if( cnt != 0 )
	{
	talk.say_tmc0580( bf, 0 );
	printf("got: %d\n", cnt );
	}

}




/*

void dec_to_hex()
{
string s1;
mystr m1;
s1 = "-397, -365, -327, -282, -229, -170, -104, -36, 35, 104, 169, 228, 281, 326, 364, 396";

int16_t ibf[ 256 ];
int cnt = str_to_decimal_buf( s1, ibf, 256 );

printf("\nhex string: " );
for( int i = 0; i < cnt; i++ )							//store bytes
	{
	printf("%04x,", (uint16_t)ibf[i] );
	}

printf("  |---done" );

getchar();
}
*/











//collect user specified vals for: chirp, energy, pitch, lattice iir coeffs k0-->k9

//either decimal or hex supported, e.g:

//"chirp=0, 42, 212, 50, 178, 18, 37, 20, 2, 225, 197, 2, 95, 90, 5, 15, 38, 252, 165, 165, 214, 221, 220, 252, 37, 43, 34, 33, 15, 255, 248, 238, 237, 239, 247, 246, 250, 0, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0"
//"chirp_hx=00,2a,d4,32,b2,12,25,14,02,e1,c5,02,5f,5a,05,0f,26,fc,a5,a5,d6,dd,dc,fc,25,2b,22,21,0f,ff,f8,ee,ed,ef,f7,f6,fa,00,03,02,01,00,00,00,00,00,00,00,00,00"


/* 
chirp=0, 42, 212, 50, 178, 18, 37, 20, 2, 225, 197, 2, 95, 90, 5, 15, 38, 252, 165, 165, 214, 221, 220, 252, 37, 43, 34, 33, 15, 255, 248, 238, 237, 239, 247, 246, 250, 0, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0
energy=0, 0, 1, 1, 2, 3, 5, 7, 10, 15, 21, 30, 43, 61, 86, 0
pitch_count=32
pitch=0, 41, 43, 45, 47, 49, 51, 53, 55, 58, 60, 63, 66, 70, 73, 76, 79, 83, 87, 90, 94, 99, 103, 107, 112, 118, 123, 129, 134, 140, 147, 153
k0=-501, -497, -493, -488, -480, -471, -460, -446, -427, -405, -378, -344, -305, -259, -206, -148, -86, -21, 45, 110, 171, 227, 277, 320, 357, 388, 413, 434, 451, 464, 474, 498
k1=-349, -328, -305, -280, -252, -223, -192, -158, -124, -88, -51, -14, 23, 60, 97, 133, 167, 199, 230, 259, 286, 310, 333, 354, 372, 389, 404, 417, 429, 439, 449, 506
k2=-397, -365, -327, -282, -229, -170, -104, -36, 35, 104, 169, 228, 281, 326, 364, 396
k3=-369, -334, -293, -245, -191, -131, -67, -1, 64, 128, 188, 243, 291, 332, 367, 397
k4=-319, -286, -250, -211, -168, -122, -74, -25, 24, 73, 121, 167, 210, 249, 285, 318
k5=-290, -252, -209, -163, -114, -62, -9, 44, 97, 147, 194, 238, 278, 313, 344, 371
k6=-291, -256, -216, -174, -128, -80, -31, 19, 69, 117, 163, 206, 246, 283, 316, 345
k7=-218, -133, -38, 59, 152, 235, 305, 361
k8=-226, -157, -82, -3, 76, 151, 220, 280
k9=-179, -122, -61, 1, 62, 123, 179, 231
*/

void load_chip_params()
{
string s1;
mystr m1;

int16_t bf[ 256 ];
uint16_t uibf[ 256 ];
string ss = tb_chirp->text();

m1 = ss;

bool vb = 0;

//dec_to_hex();


//----
if( m1.ExtractParamVal_with_delimit( "chirp=", "\n", s1 ) )
	{
	if(vb)printf("chirp= '%s'\n", s1.c_str() );
	
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > CHIRP_SIZE_0280 )
		{
		printf("load_chip_params() - too many vals, max is %d, chirp vals found: %d\n", CHIRP_SIZE_0280, cnt );
		cnt = CHIRP_SIZE_0280;		
		}

	printf("load_chip_params() - chirp vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		chirp_0280[i] = bf[i];
		}

	ichirp_size = cnt;

	if(vb)printf("cnt: %d, chirp_0280[]= %d %d\n", cnt, chirp_0280[0],chirp_0280[1] );
	}


if( m1.ExtractParamVal_with_delimit( "chirp_hx=", "\n", s1 ) )
	{
	if(vb)printf("chirp_hx= '%s'\n", s1.c_str() );
	
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > CHIRP_SIZE_0280 )
		{
		printf("load_chip_params() - too many vals, max is %d, chirp vals found: %d\n", CHIRP_SIZE_0280, cnt );
		cnt = CHIRP_SIZE_0280;		
		}

	printf("load_chip_params() - chirp vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		chirp_0280[i] = uibf[i];
//	printf("%d, ", uibf[i] );
		}
//	printf(" |--done\n" );


	ichirp_size = cnt;

	if(vb)printf("cnt: %d, chirp_0280[]= %02x %02x\n", cnt, chirp_0280[0],chirp_0280[1] );
	}
//----



//----
if( m1.ExtractParamVal_with_delimit( "energy=", "\n", s1 ) )
	{
	if(vb)printf("energy= '%s'\n", s1.c_str() );	
	
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, energy vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - energy vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tmsEnergy_0280[i] = bf[i];
		}

	if(vb)printf("cnt: %d, tmsEnergy_0280[]= %d %d\n", cnt, tmsEnergy_0280[0],tmsEnergy_0280[1] );
	}

if( m1.ExtractParamVal_with_delimit( "energy_hx=", "\n", s1 ) )
	{
	if(vb)printf("energy_hx= '%s'\n", s1.c_str() );	
	
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, energy vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - energy vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tmsEnergy_0280[i] = uibf[i];
		}

	if(vb)printf("cnt: %d, tmsEnergy_0280[]= %02x %02x\n", cnt, tmsEnergy_0280[0],tmsEnergy_0280[1] );
		
	}
//----
	


int ipitch_count = 32;

//----
if( m1.ExtractParamVal_with_delimit( "pitch_count=", "\n", s1 ) )
	{
//	printf("pitch_count= '%s'\n", s1.c_str() );
	
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt >= 1 )
		{
		ipitch_count = bf[0];
		if( ipitch_count == 64 ) 
			{
			bperiod_6bits = 1;
			ck_pitch_6bits->value(1);
			}
		else{
			bperiod_6bits = 0;
			ck_pitch_6bits->value(0);
			}

		printf("pitch_count= %d\n", ipitch_count );
		}
	}
//----


//----
if( m1.ExtractParamVal_with_delimit( "pitch=", "\n", s1 ) )
	{
	if(vb)printf("pitch= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	
	if( cnt > 64 )
		{
		printf("load_chip_params() - too many vals, max is 64, pitch vals found: %d\n", cnt );
		cnt = 64;
		}

	printf("load_chip_params() - pitch vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tmsPeriod_0280[i] = bf[i];
		}
	if(vb)printf("cnt: %d, tmsPeriod_0280[]= %d %d\n", cnt, tmsPeriod_0280[0],tmsPeriod_0280[1] );

	}




if( m1.ExtractParamVal_with_delimit( "pitch_hx=", "\n", s1 ) )
	{
	if(vb)printf("pitch_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 64 )
		{
		printf("load_chip_params() - too many vals, max is 64, pitch vals found: %d\n", cnt );
		cnt = 64;
		}

	printf("load_chip_params() - pitch vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tmsPeriod_0280[i] = uibf[i];
		printf("%02x,", tmsPeriod_0280[i] );
		}
	
	if(vb)printf("cnt: %d, tmsPeriod_0280[]= %02x %02x\n", cnt, tmsPeriod_0280[0],tmsPeriod_0280[1] );
	}
//----


//----
if( m1.ExtractParamVal_with_delimit( "k0=", "\n", s1 ) )
	{
	if(vb)printf("k0= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 32 )
		{
		printf("load_chip_params() - too many vals, max is 32, k0 vals found: %d\n", cnt );
		cnt = 32;
		}

	printf("load_chip_params() - k0 vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k0_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k0_0280[]= %d %d\n", cnt, tms_k0_0280[0],tms_k0_0280[1] );
		
	}


if( m1.ExtractParamVal_with_delimit( "k0_hx=", "\n", s1 ) )
	{
	if(vb)printf("k0_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 32 )
		{
		printf("load_chip_params() - too many vals, max is 32, k0 vals found: %d\n", cnt );
		cnt = 32;
		}

	printf("load_chip_params() - k0 vals fetched: %d\n", cnt );
	
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k0_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k0_0280[]= %02x %02x\n", cnt, tms_k0_0280[0],tms_k0_0280[1] );
	}
//----

//----
if( m1.ExtractParamVal_with_delimit( "k1=", "\n", s1 ) )
	{
	if(vb)printf("k1= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 32 )
		{
		printf("load_chip_params() - too many vals, max is 32, k1 vals found: %d\n", cnt );
		cnt = 32;
		}

	printf("load_chip_params() - k1 vals fetched: %d\n", cnt );
	
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k1_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k1_0280[]= %d %d\n", cnt, tms_k1_0280[0],tms_k1_0280[1] );
	
	}


if( m1.ExtractParamVal_with_delimit( "k1_hx=", "\n", s1 ) )
	{
	if(vb)printf("k1_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 32 )
		{
		printf("load_chip_params() - too many vals, max is 32, k1 vals found: %d\n", cnt );
		cnt = 32;
		}

	printf("load_chip_params() - k1 vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k1_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k1_0280[]= %02x %02x\n", cnt, tms_k1_0280[0],tms_k1_0280[1] );
		
	}
//----




//----
if( m1.ExtractParamVal_with_delimit( "k2=", "\n", s1 ) )
	{
	if(vb)printf("k2= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k2 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k2 vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k2_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k2_0280[]= %d %d\n", cnt, tms_k2_0280[0],tms_k2_0280[1] );
		
	}


if( m1.ExtractParamVal_with_delimit( "k2_hx=", "\n", s1 ) )
	{
	if(vb)printf("k2_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k2 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k2 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k2_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k2_0280[]= %02x %02x\n", cnt, tms_k2_0280[0],tms_k2_0280[1] );
	
	}
//----







//----
if( m1.ExtractParamVal_with_delimit( "k3=", "\n", s1 ) )
	{
	if(vb)printf("k3= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k3 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k3 vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k3_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k3_0280[]= %d %d\n", cnt, tms_k3_0280[0],tms_k3_0280[1] );
		
	}


if( m1.ExtractParamVal_with_delimit( "k3_hx=", "\n", s1 ) )
	{
	if(vb)printf("k3_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k3 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k3 vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k3_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k3_0280[]= %02x %02x\n", cnt, tms_k3_0280[0],tms_k3_0280[1] );
	
	}
//----




//----
if( m1.ExtractParamVal_with_delimit( "k4=", "\n", s1 ) )
	{
	if(vb)printf("k4= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k4 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k4 vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k4_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k4_0280[]= %d %d\n", cnt, tms_k4_0280[0],tms_k4_0280[1] );
		
	}


if( m1.ExtractParamVal_with_delimit( "k4_hx=", "\n", s1 ) )
	{
	if(vb)printf("k4_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k4 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k4 vals fetched: %d\n", cnt );

	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k4_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k4_0280[]= %02x %02x\n", cnt, tms_k4_0280[0],tms_k4_0280[1] );
	}
//----



//----
if( m1.ExtractParamVal_with_delimit( "k5=", "\n", s1 ) )
	{
	if(vb)printf("k5= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k5 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k5 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k5_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k5_0280[]= %d %d\n", cnt, tms_k5_0280[0],tms_k5_0280[1] );
		
	}


if( m1.ExtractParamVal_with_delimit( "k5_hx=", "\n", s1 ) )
	{
	if(vb)printf("k5_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k5 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k5 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k5_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k5_0280[]= %02x %02x\n", cnt, tms_k5_0280[0],tms_k5_0280[1] );
	
	}
//----




//----
if( m1.ExtractParamVal_with_delimit( "k6=", "\n", s1 ) )
	{
	if(vb)printf("k6= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k6 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k6 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k6_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k6_0280[]= %d %d\n", cnt, tms_k6_0280[0],tms_k6_0280[1] );

	}


if( m1.ExtractParamVal_with_delimit( "k6_hx=", "\n", s1 ) )
	{
	if(vb)printf("k6_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 16 )
		{
		printf("load_chip_params() - too many vals, max is 16, k6 vals found: %d\n", cnt );
		cnt = 16;
		}

	printf("load_chip_params() - k6 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k6_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k4_0280[]= %02x %02x\n", cnt, tms_k6_0280[0],tms_k6_0280[1] );
	
	}
//----



//----
if( m1.ExtractParamVal_with_delimit( "k7=", "\n", s1 ) )
	{
	if(vb)printf("k7= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 8 )
		{
		printf("load_chip_params() - too many vals, max is 8, k7 vals found: %d\n", cnt );
		cnt = 8;
		}

	printf("load_chip_params() - k7 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k7_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k7_0280[]= %d %d\n", cnt, tms_k7_0280[0],tms_k7_0280[1] );
	
	}


if( m1.ExtractParamVal_with_delimit( "k7_hx=", "\n", s1 ) )
	{
	if(vb)printf("k7_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 8 )
		{
		printf("load_chip_params() - too many vals, max is 8, k7 vals found: %d\n", cnt );
		cnt = 8;
		}

	printf("load_chip_params() - k7 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k7_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k7_0280[]= %02x %02x\n", cnt, tms_k7_0280[0],tms_k7_0280[1] );
		
	}
//----






//----
if( m1.ExtractParamVal_with_delimit( "k8=", "\n", s1 ) )
	{
	if(vb)printf("k8= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 8 )
		{
		printf("load_chip_params() - too many vals, max is 8, k8 vals found: %d\n", cnt );
		cnt = 8;
		}

	printf("load_chip_params() - k8 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k8_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k8_0280[]= %d %d\n", cnt, tms_k8_0280[0],tms_k8_0280[1] );
		
	}


if( m1.ExtractParamVal_with_delimit( "k8_hx=", "\n", s1 ) )
	{
	if(vb)printf("k8_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 8 )
		{
		printf("load_chip_params() - too many vals, max is 8, k8 vals found: %d\n", cnt );
		cnt = 8;
		}

	printf("load_chip_params() - k8 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k8_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k8_0280[]= %02x %02x\n", cnt, tms_k8_0280[0],tms_k8_0280[1] );
		
	}
//----





//----
if( m1.ExtractParamVal_with_delimit( "k9=", "\n", s1 ) )
	{
	if(vb)printf("k9= '%s'\n", s1.c_str() );
		
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > 8 )
		{
		printf("load_chip_params() - too many vals, max is 8, k9 vals found: %d\n", cnt );
		cnt = 8;
		}

	printf("load_chip_params() - k9 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k9_0280[i] = bf[i];
		}
	if(vb)printf("s1: '%s'\n", s1.c_str() );
	if(vb)printf("cnt: %d, tms_k9_0280[]= %d %d\n", cnt, tms_k9_0280[0],tms_k9_0280[1] );
		
	}


if( m1.ExtractParamVal_with_delimit( "k9_hx=", "\n", s1 ) )
	{
	if(vb)printf("k9_hx= '%s'\n", s1.c_str() );
		
	int cnt = str_to_hex_buf( s1, uibf, 256 );

	if( cnt > 8 )
		{
		printf("load_chip_params() - too many vals, max is 8, k9 vals found: %d\n", cnt );
		cnt = 8;
		}

	printf("load_chip_params() - k8 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k9_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k9_0280[]= %02x %02x\n", cnt, tms_k9_0280[0],tms_k9_0280[1] );
		
	}
//----

}






void update_gphs()
{
update_gph0( gph0_vamp0, gph0_vamp1, gph0_vamp2 );
}




int last_adress = 0;


void Talkie::say_tmc0580( uint8_t *buf_lpc, unsigned int offset )
{
bool vb_dbg = 0;
string s1, st;
mystr m1;
slpc_bytes = "";

string fname = "zzdbg_dump.txt";
int tclock = 0;						//just a handy marker value

bufcnt = 0;
int bufptr = 0;
int start_lpc_addr;
int ptr_8KHz = 0;

int smpl_cnt = 0;
int smpl_tot = 0;
float sub_time = 0;


theta1 = 0;

generated_samp_cnt = 0;
frame_cnt = 0;

int ending_cnt = -1;

af2.clear_ch0();
af2.clear_ch1();
af2.clear_ch0();
af2.clear_ch1();

mode = 0;
int period_cnt = 0;
x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = 0;
chirp_step = 0;


uint8_t from_energy, cur_energy, tgt_energy;
uint8_t from_period, cur_period, tgt_period;

int16_t cur_k0, cur_k1, cur_k2, cur_k3, cur_k4, cur_k5, cur_k6, cur_k7, cur_k8, cur_k9;


float interp_mix = 0;							//changes from 0.0->1.0
float impulse_sig;

int energy_idx = 0;

int phs0 = 0;			//phase counter 1->2
int phs1 = 0;			//0->12 sub multiple as driven by phs0
int phs2 = 0;			//0->7, sub multiple as driven by phs1, interpolation is control by this val


saf.encoding = 3;
saf.offset = 0;
saf.format = en_af_wav_pcm;
saf.channels = 1;
saf.srate = 8000;

//af.load( "", "zzcorrect0_48000.wav", 32768, saf );

//if( !af.load( "", "/home/gc/Desktop/sdb2/mame_ubuntu/mame/zzgc_mame_chirp_more_gain.wav", 32768, saf ) )
//if( !af.load( "", "./zzgc_mame.wav", 32768, saf ) )
//	{
//	printf("failed af.load()\n");
//	exit(0);
//	}





load_chip_params();


// 


bool vb = 0;
static uint8_t nextPwm;
float u0,u1,u2,u3,u4,u5,u6,u7,u8,u9,u10;
float yy;

float inp[ 8000 ];
float koef[ 10 ];
float ycalc[ 10 ];
float gstate[ 10 + 1 ];
float outp[ 8000 ];


if( !inited ) return;

if( offset >= rom_size_max )
	{
	printf("say_tmc0580() - address out of range: %d (0x%x)\n", offset, offset );
	return;
	}

last_say_offset = offset;

uint8_t *addr = buf_lpc + offset;

//memset( bufsamp0, 0, bufsamp_sz * sizeof(float) );
//memset( bufsamp1, 0, bufsamp_sz * sizeof(float) );

for( int i = 0; i < bufsamp_sz; i++ )
	{
	bufsamp0[ i ] = 0;					//clear buf
	bufsamp1[ i ] = 0;
	}

dbg_dump = 0;


if (!setup)
	{
	setup = 1;
	}

setPtr( addr );

start_lpc_addr = (int)ptrAddr;
stop_lpc_addr = start_lpc_addr;


strpf( s1, "%02X", *ptrAddr );						//store initial byte starting from, see also 'getBits()', where further bytes are collected
slpc_bytes += s1;



//strpf( s1, "%04x", start_lpc_addr );
//printf("start_lpc_addr: %04x \n",start_lpc_addr );
//getchar();
//printf("start lpc[%03d]: %02x \n", stop_lpc_addr - start_lpc_addr, *ptrAddr );

bool skip_interp = 0;
bool first = 1;

bool now_voiced = 0;								//used to detect change between voice/unvoiced frame, used for interpolation muting
bool now_silence = 0;
bool last_voiced = 0;
bool last_silence = 0;


synthRand = 1;				//force random noise seed to commence from  the same value - for debug purposes

//extern vector<float> gph0_vx;
vector<float> fgph_vx;
vector<float> fgph_vy0;
vector<float> fgph_vy1;
vector<float> fgph_vy2;

gph0_vx.clear();
gph0_vamp0.clear();
gph0_vamp1.clear();
gph0_vamp2.clear();
gph0_vamp3.clear();
gph0_vamp4.clear();

//if( last_silence );
int k0_idx, k1_idx, k2_idx, k3_idx, k4_idx, k5_idx, k6_idx, k7_idx, k8_idx, k9_idx;

tgt_energy = tgt_period = tgt_k9 = tgt_k8 = tgt_k7 = tgt_k6 = tgt_k5 = tgt_k4 = tgt_k3 = tgt_k2 = tgt_k1 = tgt_k0 = 0;

m1.appendfile_str( fname, 1, 1, "" );			//clear dump file


//render loop
while(1)
	{
	uint8_t repeat;
	
	//move target->from vals
	from_energy = cur_energy = tgt_energy;		//new frame - update from and cur params
	from_period = cur_period = tgt_period;
	from_k9 = cur_k9 = tgt_k9;
	from_k8 = cur_k8 = tgt_k8;
	from_k7 = cur_k7 = tgt_k7;
	from_k6 = cur_k6 = tgt_k6;
	from_k5 = cur_k5 = tgt_k5;
	from_k4 = cur_k4 = tgt_k4;
	from_k3 = cur_k3 = tgt_k3;
	from_k2 = cur_k2 = tgt_k2;
	from_k1 = cur_k1 = tgt_k1;
	from_k0 = cur_k0 = tgt_k0;

	//read new frame params
	if( ending_cnt < 0 ) energy_idx = getBits(4);
	if (energy_idx == 0) 						//silence frame?
		{
		tgt_energy = 0;
		from_energy = 0;
		}
	else{
		if( ( energy_idx == 0xf ) && ( ending_cnt < 0 ) )				//stop frame?
			{
			ending_cnt = 2;						//allow lattice iir empty out and to settle to zero after hitting last frame
printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!energy_idx == 0xf\n");
			tgt_energy = tgt_period = tgt_k9 = tgt_k8 = tgt_k7 = tgt_k6 = tgt_k5 = tgt_k4 = tgt_k3 = tgt_k2 = tgt_k1 = tgt_k0 = 0;
			}

		if( energy_idx != 0xf )
			{
			//get frame params
			tgt_energy = tmsEnergy_0280[ energy_idx ];
			repeat = getBits(1);

			int period_idx;
			if( bperiod_6bits ) period_idx = getBits(6);					//energy will be 6 bits?
			else period_idx = getBits(5);
			
			tgt_period = tmsPeriod_0280[period_idx];					//pitch, 32 levels
//if(vb_dbg)printf("tgt_energy : idx: %02d, val=%03d\n", energy_idx, tgt_energy );
//if(vb_dbg)printf("tgt_period : idx: %02d, val=%03d\n", period_idx, tgt_period );


			if( first ) 												//first frame?
				{
				from_energy = cur_energy = tgt_energy;
				from_period = cur_period = tgt_period;
				}
			
//			bool unvoiced = 0;
//			if( cur_period == 0 ) unvoiced = 1;
			
			if (!repeat) 					//not a repeat frame?, get new coeff val
				{
				//every frames has 4 coeffs, voiced frames (period!=0) have additional 6 coeffs collected further below
				k0_idx = getBits(5);
				tgt_k0 = tms_k0_0280[k0_idx];							//32 levels
//if(vb_dbg)printf("tgt_k0 : k0_idx: %02d, kval=%03d\n", k0_idx, tgt_k0 );

				k1_idx = getBits(5);
				tgt_k1 = tms_k1_0280[k1_idx];							//32 levels
//if(vb_dbg)printf("tgt_k1 : k1_idx: %02d, kval=%03d\n", k1_idx, tgt_k1 );

				k2_idx = getBits(4);
				tgt_k2 = tms_k2_0280[k2_idx];							//16 levels
//if(vb_dbg)printf("tgt_k2 : k2_idx: %02d, kval=%03d\n", k2_idx, tgt_k2 );

				k3_idx = getBits(4);
				tgt_k3 = tms_k3_0280[k3_idx];							//16 levels
//if(vb_dbg)printf("tgt_k3 : k3_idx: %02d, kval=%03d\n", k3_idx, tgt_k3 );

				if( tgt_period )									//pitch is not zero?, voiced frames have 6 extra coeffs
					{
					k4_idx = getBits(4);
					tgt_k4 = tms_k4_0280[k4_idx];					//16 levels
//if(vb_dbg)printf("tgt_k4 : k4_idx: %02d, kval=%03d\n", k4_idx, tgt_k4 );

					k5_idx = getBits(4);
					tgt_k5 = tms_k5_0280[k5_idx];					//16 levels
//if(vb_dbg)printf("tgt_k5 : k5_idx: %02d, kval=%03d\n", k5_idx, tgt_k5 );

					k6_idx = getBits(4);
					tgt_k6 = tms_k6_0280[k6_idx];					//16 levels
//if(vb_dbg)printf("tgt_k6 : k6_idx: %02d, kval=%03d\n", k6_idx, tgt_k6 );

					k7_idx = getBits(3);
					tgt_k7 = tms_k7_0280[k7_idx];					//8 levels
//if(vb_dbg)printf("tgt : k7_idx: %02d, kval=%03d\n", k7_idx, tgt_k7 );

					k8_idx = getBits(3);
					tgt_k8 = tms_k8_0280[k8_idx];					//8 levels
//if(vb_dbg)printf("tgt_k8 : k8_idx: %02d, kval=%03d\n", k8_idx, tgt_k8 );

					k9_idx = getBits(3);
					tgt_k9 = tms_k9_0280[k9_idx];					//8 levels
//if(vb_dbg)printf("tgt_k9: k9_idx: %02d, kval=%03d\n", k9_idx, tgt_k9 );
					}
				else{
					tgt_k4 = 0;										//zero when not there is no pitch (unvoiced)
					tgt_k5 = 0;
					tgt_k6 = 0;
					tgt_k7 = 0;
					tgt_k8 = 0;
					tgt_k9 = 0;
					}
				}
			}
		}

	if(1)printf("frame_cnt[%03d]: engy_idx: %d, engy: %d, period: %d, rpt: %d, ending_cnt: %d\n", frame_cnt, energy_idx, tgt_energy, tgt_period, repeat, ending_cnt );

	if( first ) 					//first frame?
		{
		from_energy = cur_energy = tgt_energy;
		from_period = cur_period = tgt_period;
		from_k0 = cur_k0 = tgt_k0;
		from_k1 = cur_k1 = tgt_k1;
		from_k2 = cur_k2 = tgt_k2;
		from_k3 = cur_k3 = tgt_k3;
		from_k4 = cur_k4 = tgt_k4;
		from_k5 = cur_k5 = tgt_k5;
		from_k6 = cur_k6 = tgt_k6;
		from_k7 = cur_k7 = tgt_k7;
		from_k8 = cur_k8 = tgt_k8;
		from_k9 = cur_k9 = tgt_k9;
		first = 0;
		}

	//interpolation skip logic
	skip_interp = 0;

	if(!use_interp) skip_interp = 1;

	if( cur_period != 0 ) now_voiced = 1;
	else now_voiced = 0;

	if( cur_energy == 0 ) now_silence = 1;
	else now_silence = 0;

	if( now_voiced & ( !last_voiced ) ) skip_interp = 1;
	if( last_voiced & ( !now_voiced ) ) skip_interp = 1;

	if( now_silence & ( !last_silence ) ) skip_interp = 1;
	if( last_silence & ( !now_silence ) ) skip_interp = 1;



	float chirp_sub_modulo = 1.0/lpc_srate;
	float chirp_sub_steps = 1.0/lpc_srate;

	float raw_stimul;
//	uint16_t energy_interp;

	int interp_granularity = 8;										//ti_lpc used 8 steps per frame
	int interp_cur;
	int smple_per_frame = lpc_srate * frame_time;
//	int interp_frmesz = smple_per_frame / interpstep;

	for( int i = 0; i < cn_bufsz; i++ ) buf[ i ] = 0;				//clear buf


	double theta1_inc = 400 * twopi / lpc_srate;
	float sin1;
	bool new_frame = 1;
	bool interp_frame = 0;
	interp_cur = 0;
	
	
	for( int ii = 0; ii < smple_per_frame; ii++ )
		{
		sin1 = sin( theta1 );

		theta1 += theta1_inc;
		if( theta1 >= twopi ) theta1 -= twopi;

//		bufsamp0[ ptr ] = sin1;										//store audio samples
//		bufsamp1[ ptr++ ] = sin1; 

		gph0_vx.push_back( smpl_cnt * (1.0 / lpc_srate) );				//make x-axis vals
		fgph_vx.push_back( smpl_cnt * (1.0 / lpc_srate) );				//make x-axis vals
//		gph0_vamp0.push_back( sin1 );
//		gph0_vamp1.push_back( ii / 32768.0 );
		gph0_vamp4.push_back( new_frame );
		smpl_cnt++;
		smpl_tot++;
		
		sub_time += chirp_sub_steps;
		
		bool update_interp = 0;
		
		//calc interpolation factor
		if( ( ii != 0 ) & ( !(ii % ( smple_per_frame / interp_granularity )) ) )		//8 times per frame
			{
			update_interp = 1;
//			cur_k9 = tgt_k9;
			interp_mix = interp_cur / (interp_granularity - 1.0);	//changes from 0.0->1.0, in 8 steps
			interp_cur++;
			if( interp_cur >= interp_granularity  ) interp_cur = 0;
			}


		if( !use_interp ) interp_mix = 0;
		if ( skip_interp ) interp_mix = 0;

		if( new_frame ) impulse_sig = 0.4;							//just for checking lattice iir

		//calc a new lpc decoder sample
		if ( sub_time >= chirp_sub_modulo )							//time to the lpc calcs, running at: lpc_srate
			{
			sub_time -= chirp_sub_modulo;
			temp_smple_cnt++;


//printf("\ninterp_step: %.4f\n", interp_step );


			// ---- calc a lpc decoder sample ----
//			int period_interp;

//			period_interp = (1.0 - interp_mix) * last_period  +  interp_mix * period;		//do a linear interp mix

//			energy_interp = (1.0 - interp_mix) * last_energy  +  interp_mix * energy;		//do a linear interp mix


			if( cur_period)
				{
				//voiced, glottal pitch chirp stimulus
				if( period_cnt < 41 )
					{
					u10 = (float)((float)( (int8_t*)chirp_0280)[period_cnt] / 256.0) * ( cur_energy / 256.0);
//printf("VC: %06d: %02d %02d %02d chirp_0280[%02d]: val=%05d\n", tclock, phs0, phs1, phs2, period_cnt, chirp_0280[period_cnt] );
					}
				else{
					u10 = 0;
//printf("VC: %06d: %02d %02d %02d chirp_0280[%02d]: val=%05d (>=41) \n", tclock, phs0, phs1, phs2, period_cnt, 0 );
					}


				int chirp_cnt = ichirp_size;

				if (period_cnt >= ( cur_period - 1 ) )
					{
//if(vb_dbg)printf("clear - period_cnt: %02d, cur_period: %02d, chirp: %03d  %f\n", period_cnt, cur_period, chirp_0280[period_cnt], u10 );
					period_cnt = 0;
					}
				else{
					period_cnt++;
//if(vb_dbg)printf("count - period_cnt: %02d, cur_period: %02d, chirp: %03d  %f\n", period_cnt, cur_period, chirp_0280[period_cnt], u10 );
					}
				}
			else{
				//unvoiced consonants, fricative etc - white noise stimulus
				synthRand = (synthRand >> 1) ^ ((synthRand & 1) ? 0xB800 : 0);
				int wnoise = (synthRand & 1) ? cur_energy : -cur_energy;
				u10 = wnoise;
				
				u10 /= 2048.0;
//if(vb_dbg)printf("WN: %06d: %02d %02d %02d: val=%05d\n", tclock, phs0, phs1, phs2, synthRand );
				}



//u10/=1.0;


		raw_stimul = u10;

		int8_t tms_interp_shift[8] = { 0, 3, 3, 3, 2, 2, 1, 1 };			//for simple divide


		//generate interpolated params using deltas and shifters
		float aa = 0;
		int interp_shift = tms_interp_shift[ interp_cur ];
		if( update_interp )
			{
		//	int interp_val = cur_k9 + ( (tgt_k9 - cur_k9 ) >> interp_shift );
			
		//float aa = last_k9 >> 3;
		//printf("ii_interp: %05d, frme: %d, interp_cur: %d, interp_shift: %d, f: %d, c: %d, t: %d, %f\n", ii, frame_cnt, interp_cur, interp_shift, from_k9, cur_k9, tgt_k9, aa);
		//	cur_k9 = interp_val;


			int interp_val = cur_energy + ( (tgt_energy - cur_energy ) >> interp_shift );
			cur_energy = interp_val;

			interp_val = cur_period + ( (tgt_period - cur_period ) >> interp_shift );
			cur_period = interp_val;

			interp_val = cur_k9 + ( (tgt_k9 - cur_k9 ) >> interp_shift );
			cur_k9 = interp_val;

			interp_val = cur_k8 + ( (tgt_k8 - cur_k8 ) >> interp_shift );
			cur_k8 = interp_val;

			interp_val = cur_k7 + ( (tgt_k7 - cur_k7 ) >> interp_shift );
			cur_k7 = interp_val;

			interp_val = cur_k6 + ( (tgt_k6 - cur_k6 ) >> interp_shift );
			cur_k6 = interp_val;

			interp_val = cur_k5 + ( (tgt_k5 - cur_k5 ) >> interp_shift );
			cur_k5 = interp_val;

			interp_val = cur_k4 + ( (tgt_k4 - cur_k4 ) >> interp_shift );
			cur_k4 = interp_val;

			interp_val = cur_k3 + ( (tgt_k3 - cur_k3 ) >> interp_shift );
			cur_k3 = interp_val;

			interp_val = cur_k2 + ( (tgt_k2 - cur_k2 ) >> interp_shift );
			cur_k2 = interp_val;

			interp_val = cur_k1 + ( (tgt_k1 - cur_k1 ) >> interp_shift );
			cur_k1 = interp_val;

			interp_val = cur_k0 + ( (tgt_k0 - cur_k0 ) >> interp_shift );
			cur_k0 = interp_val;

			update_interp = 0;

			if( use_interp )
				{
				cur_energy = from_energy;
				cur_period = from_period;

				cur_k9 = from_k9;
				cur_k8 = from_k8;
				cur_k7 = from_k7;
				cur_k6 = from_k6;
				cur_k5 = from_k5;
				cur_k4 = from_k4;
				cur_k3 = from_k3;
				cur_k2 = from_k2;
				cur_k1 = from_k1;
				cur_k0 = from_k0;
				}
		//if(vb_dbg)printf("tot: %06d, cnt: %03d, ii_interp: %05d, frme: %03d, energy: %03d, period: %03d, k9:%03d, k8:%03d, k7:%03d, k6:%03d, k5:%03d, k4:%03d, k3:%03d, k2:%03d, k1:%03d, k0:%03d\n", smpl_tot, smpl_cnt, ii, frame_cnt, cur_energy, cur_period, cur_k9, cur_k8, cur_k7, cur_k6, cur_k5, cur_k4, cur_k3, cur_k2, cur_k1, cur_k0 );
			}
		else{
		//if(vb_dbg)printf("tot: %06d, cnt: %03d, ii_interp: %05d, frme: %03d, energy: %03d, period: %03d, k9:%03d, k8:%03d, k7:%03d, k6:%03d, k5:%03d, k4:%03d, k3:%03d, k2:%03d, k1:%03d, k0:%03d\n", smpl_tot, smpl_cnt, ii, frame_cnt, cur_energy, cur_period, cur_k9, cur_k8, cur_k7, cur_k6, cur_k5, cur_k4, cur_k3, cur_k2, cur_k1, cur_k0 );
		//printf("ii_not   : %05d, frme: %d, interp_cur: %d, interp_shift: %d, f: %d, c: %d, t: %d, %f\n", ii, frame_cnt, interp_cur, interp_shift, from_k9, cur_k9, tgt_k9, aa);
			}
		//			k0 = (1.0 - interp_mix) * last_k0  +  interp_mix * cur_k0;				//do a linear interp mix
		//			k1 = (1.0 - interp_mix) * last_k1  +  interp_mix * cur_k1;
		//			k2 = (1.0 - interp_mix) * last_k2  +  interp_mix * cur_k2;
		//			k3 = (1.0 - interp_mix) * last_k3  +  interp_mix * cur_k3;
		//			k4 = (1.0 - interp_mix) * last_k4  +  interp_mix * cur_k4;
		//			k5 = (1.0 - interp_mix) * last_k5  +  interp_mix * cur_k5;
		//			k6 = (1.0 - interp_mix) * last_k6  +  interp_mix * cur_k6;
		//			k7 = (1.0 - interp_mix) * last_k7  +  interp_mix * cur_k7;
		//			k8 = (1.0 - interp_mix) * last_k8  +  interp_mix * cur_k8;
		//			k9 = (1.0 - interp_mix) * last_k9  + interp_mix * cur_k9;




			
		float mkfract = 512.0;
		
		if( new_frame )
			{
	/*
			st = "";
			strpf( s1, "\n\n------ new_frame ------!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" ); 	
			st += s1;
			strpf( s1, "repeat: %d\n", repeat );
			st += s1;
			strpf( s1, "period: %d\n", cur_period );
			st += s1;
			strpf( s1, "interp_mix: %f\n", interp_mix );
			st += s1;
			strpf( s1, "cur_k9: %f, idx: %d\n", (float)cur_k9 / mkfract, k9_idx ); 
			st += s1;
			strpf( s1, "cur_k8: %f, idx: %d\n", (float)cur_k8 / mkfract, k8_idx ); 
			st += s1;
			strpf( s1, "cur_k7: %f, idx: %d\n", (float)cur_k7 / mkfract, k7_idx ); 
			st += s1;
			strpf( s1, "cur_k6: %f, idx: %d\n", (float)cur_k6 / mkfract, k6_idx ); 
			st += s1;
			strpf( s1, "cur_k5: %f, idx: %d\n", (float)cur_k5 / mkfract, k5_idx ); 
			st += s1;
			strpf( s1, "cur_k4: %f, idx: %d\n", (float)cur_k4 / mkfract, k4_idx ); 
			st += s1;
			strpf( s1, "cur_k3: %f, idx: %d\n", (float)cur_k3 / mkfract, k3_idx ); 
			st += s1;
			strpf( s1, "cur_k2: %f, idx: %d\n", (float)cur_k2 / mkfract, k2_idx ); 
			st += s1;
			strpf( s1, "cur_k1: %f, idx: %d\n", (float)cur_k1 / mkfract, k1_idx ); 
			st += s1;
			strpf( s1, "cur_k0: %f, idx: %d\n", (float)cur_k0 / mkfract, k0_idx ); 
			st += s1;
	*/
	//				m1.appendfile_str( fname, 1, 0, st );
			}
		else{
			interp_frame = 1;
			}

		if( interp_frame )
			{
	/*
			st = "";
			strpf( s1, "------- interp ------\n" );
			st += s1;
			strpf( s1, "repeat: %d\n", repeat );
			st += s1;
			strpf( s1, "period: %d\n", cur_period );
			st += s1;
			strpf( s1, "interp_mix: %f\n", interp_mix );
			st += s1;
			strpf( s1, "tclk: %06d, from:k9: %f, k8: %f, k7: %f, k6: %f, k5: %f, k5: %f, k3: %f, k2: %f, k1: %f, k0: %f\n", tclock, (float)from_k9, (float)from_k8, (float)from_k7, (float)from_k6, (float)from_k5, (float)from_k4, (float)from_k3, (float)from_k2, (float)from_k1, (float)from_k0 ); 
			st += s1;
			strpf( s1, "tclk: %06d, to:  k9: %f, k8: %f, k7: %f, k6: %f, k5: %f, k5: %f, k3: %f, k2: %f, k1: %f, k0: %f\n", tclock, (float)tgt_k9, (float)tgt_k8, (float)tgt_k7, (float)tgt_k6, (float)tgt_k5, (float)tgt_k4, (float)tgt_k3, (float)tgt_k2, (float)tgt_k1, (float)tgt_k0 ); 
			st += s1;
			strpf( s1, "tclk: %06d, mix: k9: %d, k8: %d, k7: %d, k6: %d, k5: %d, k5: %d, k3: %d, k2: %d, k1: %d, k0: %d\n", tclock, cur_k9, cur_k8, cur_k7, cur_k6, cur_k5, cur_k4, cur_k3, cur_k2, cur_k1, cur_k0 ); 
			st += s1;
	//				strpf( s1, "tclk: %06d,      k9: %f, k8: %f, k7: %f, k6: %f, k5: %f, k5: %f, k3: %f, k2: %f, k1: %f, k0: %f\n", tclock, (float)k9 / mkfract, (float)k8 / mkfract, (float)k7 / mkfract, (float)k6 / mkfract, (float)k5 / mkfract, (float)k4 / mkfract, (float)k3 / mkfract, (float)k2 / mkfract, (float)k1 / mkfract, (float)k0 / mkfract ); 
	//				st += s1;
	//				m1.appendfile_str( fname, 1, 0, st );
	*/
			}

		//lattice iir forward path - vocal tract's spectral formant shaper,
		//approximates sound wave transmission/reflectance through abutted cylinders of various radii 
		u9 = u10 - ( ( (float)cur_k9 / mkfract) ) * x9;
		u8 = u9 - ( ( (float)cur_k8 / mkfract) ) * x8;
		u7 = u8 - ( ( (float)cur_k7 / mkfract) ) * x7;
		u6 = u7 - ( ( (float)cur_k6 / mkfract) ) * x6;
		u5 = u6 - ( ( (float)cur_k5 / mkfract) ) * x5;
		u4 = u5 - ( ( (float)cur_k4 / mkfract) ) * x4;
		u3 = u4 - ( ( (float)cur_k3 / mkfract) ) * x3;
		u2 = u3 - ( ( (float)cur_k2 / mkfract) ) * x2;
		u1 = u2 - ( ( (float)cur_k1 / mkfract) ) * x1;
		u0 = u1 - ( ( (float)cur_k0 / mkfract) ) * x0;

		//clamp
		if (u0 > 1.0) u0 = 1.0;
		if (u0 < -1.0) u0 = -1.0;

		//lattice iir reverse path
		x9 = x8 + ( ( (float)cur_k8 / mkfract) ) * u8;
		x8 = x7 + ( ( (float)cur_k7 / mkfract) ) * u7;
		x7 = x6 + ( ( (float)cur_k6 / mkfract) ) * u6;
		x6 = x5 + ( ( (float)cur_k5 / mkfract) ) * u5;
		x5 = x4 + ( ( (float)cur_k4 / mkfract) ) * u4;
		x4 = x3 + ( ( (float)cur_k3 / mkfract) ) * u3;
		x3 = x2 + ( ( (float)cur_k2 / mkfract) ) * u2;
		x2 = x1 + ( ( (float)cur_k1 / mkfract) ) * u1;
		x1 = x0 + ( ( (float)cur_k0 / mkfract) ) * u0;
		x0 = u0;


//	if( std::isnan(raw_stimul) )
//		{
//		raw_stimul = 0;
//		}


		u0 /= 1.0;
		
		if( !use_filter ) u0 = raw_stimul;

bool use_sine = 0;

		if( use_sine ) u0 = sin1 * 0.5;

		//------------------------------------
		}

//		bufsamp0[ ptr ] = u0;										//store audio samples
//		bufsamp1[ ptr ] = u0; 

//	if( std::isnan(u0) )
//		{
//		u0 = 0;
//		}


u0 *= 1.5;

		fbuf1[ ptr_8KHz ] = u0;										//store 8KHz sample for samplerate converter to later process
		ptr_8KHz++;

		af.push_ch0( u0 );
		af.push_ch1( u0 );
		
//		if( af.sizech0 >  )
			{
			
			}
			
//		gph0_vamp0.push_back( sin1 );
		gph0_vamp0.push_back( u0 );
//		printf("u0 %f  raw_stimul %f  new_frame %f\n", u0, raw_stimul, new_frame / 10.0 );
		
		gph0_vamp1.push_back( raw_stimul );
//		gph0_vamp1.push_back( sin1 * 0.25 + 0.1 );
		gph0_vamp2.push_back( new_frame / 10.0 );
		
//		fgph_vy0.push_back( u0 );
//		fgph_vy1.push_back( raw_stimul );

		new_frame = 0;
		interp_frame = 0;

		//next phase
		phs0++;
		if( phs0 == 2 )
			{
			phs0 = 0;
			phs1++;
			}

		if( phs1 == 13 )
			{
			phs1 = 0;
			phs2++;
			}

		if( phs2 == 8 )
			{
			phs2 = 0;
			}
		tclock++;
		}


	if( cur_period != 0 ) last_voiced = 1;
	else last_voiced = 0;

	if( cur_energy == 0 ) last_silence = 1;
	else last_silence = 0;

	time_cur += frame_time;
	frame_cnt++;

	if( ending_cnt > 0 )													//in process of ending?, allow lattice iir to empty out
		{
		ending_cnt--;
		printf("ending_cnt: %d\n", ending_cnt);
		if( ending_cnt == 0 ) break;
		}
	}

printf( "tot_frames: %d, tot_time: %f, temp_smple_cnt: %d\n", frame_cnt, time_cur, temp_smple_cnt );

fi_lpc_hex->value( slpc_bytes.c_str() );


/*
gph0_vamp4.clear();
int ssshift = 278;
int ishift = ssshift;

for( int i = 0; i < gph0_vx.size(); i++ )
	{
	
	if( ishift >= saf.vch0.size() )
		{
//		gph0_vamp4.push_back( 0 );
		continue;
		}
		
//	gph0_vamp4.push_back( saf.vch0[ishift] / 30.0 );
	gph0_vamp4.push_back( saf.vch0[ishift] / 30.0 );
	ishift++;
	}
*/


fgph_vx.clear();
fgph_vy0.clear();
fgph_vy1.clear();


// ---- sample rate conversion from lpc rom's 8KHz srate to PC's soundcard srate and .au srate ----
float srconv_ratio = (float)lpc_srate / srate;
float cur_sample = 0;

float nyquist = srate * 0.55;									//assume up converting the srate
int fir_wndw = 64;
if( srate <= lpc_srate )
	{
	nyquist = srate * 0.375;
	fir_wndw = 256;
	}

printf("sample rate conversion 8KHz --> %d, samples: %f, nyquist: %f\n", srate, (ptr_8KHz * (1.0/srconv_ratio)), nyquist );

//samples for pc audio
for( int i = 0; i < (ptr_8KHz * (1.0/srconv_ratio)); i++ )
	{
//usage --->		//float qdss_resample_float( float x, float *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth )
	float srconv_val = qdss_resample_float( cur_sample, fbuf1, ptr_8KHz, nyquist, srate, fir_wndw );


//		bool end_reached;
//usage --->		//float farrow_resample_float( float sampleIndexOutput, float *bf, unsigned int bfsize, bool &end_reached )
//		float srconv_val = farrow_resample_float(cur_sample,  fbuf1, ptr_8KHz, end_reached );

	cur_sample += srconv_ratio;

	if(  i < bufsamp_sz )
		{
		bufsamp0[ i ] = srconv_val;									//store pc audio samples
		bufsamp1[ i ] = srconv_val; 
		generated_samp_cnt++;
		}

	if( srate == srate_au )										//pc audio and .au srates the same?
		{
		af2.push_ch0( srconv_val * ( au_aud_gain / 100.0 ) );								//samples for .au audio file
		af2.push_ch1( srconv_val * ( au_aud_gain / 100.0 ) );
		}
	
	fgph_vx.push_back( i * 1.0 / srate );
	fgph_vy0.push_back( srconv_val );
	if( !(i%1000) ) printf(".");
	}

printf("\n");

if( srate != srate_au )											//pc audio and .au srates the different?
	{
	srconv_ratio = (float)lpc_srate / srate_au;
	cur_sample = 0;
	nyquist = srate_au * 0.55;									//assume up converting the srate
	fir_wndw = 64;
	if( srate_au <= lpc_srate )
		{
		nyquist = srate_au * 0.375;
		fir_wndw = 256;
		}

printf("sample rate conversion .au file: 8KHz --> %d, samples: %f, nyquist: %f\n", srate_au, (ptr_8KHz * (1.0/srconv_ratio)), nyquist );

	//samples for .au audio file
	for( int i = 0; i < (ptr_8KHz * (1.0/srconv_ratio)); i++ )
		{
//usage --->		//float qdss_resample_float( float x, float *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth )
		float srconv_val = qdss_resample_float( cur_sample, fbuf1, ptr_8KHz, nyquist, srate_au, fir_wndw );

//		bool end_reached;
//usage --->		//float farrow_resample_float( float sampleIndexOutput, float *bf, unsigned int bfsize, bool &end_reached )
//		float srconv_val = farrow_resample_float(cur_sample,  fbuf1, ptr_8KHz, end_reached );
	
		//bool end_reached;
		//float farrow_resample_float( float sampleIndexOutput, float *bf, unsigned int bfsize, bool &end_reached )
		//float srconv_val = farrow_resample_float(cur_sample,  ptr_8KHz, cn_bufsz, end_reached );

		cur_sample += srconv_ratio;

		af2.push_ch0( srconv_val * ( au_aud_gain / 100.0 ) );
		af2.push_ch1( srconv_val * ( au_aud_gain / 100.0 ) );
		if( !(i%1000) ) printf(".");
		}
	}
//--------------------------------------------------------------------------------------------------------

printf("\n");

//printf("\nnyquist: %f, fir_wndw: %d\n", nyquist, fir_wndw );
saf.encoding = 3;
saf.offset = 0;
saf.format = en_af_sun;
saf.channels = 2;
saf.srate = srate_au;

s1 = fi_au_fname->value();
af2.save_malloc( "", s1, 32767, saf );


update_gphs();

fgph.scale_y( 1.0, 1.0, 1.0, 0.1 );
fgph.shift_y( 0.0, -0.1, -0.10, -0.2 );
fgph.font_size( 9 );
fgph.set_sig_dig( 2 );
fgph.sample_rect_hints_distancex = 0;
fgph.sample_rect_hints_distancey = 0;

//fgph.plotxy( gph0_vx, gph0_vamp0, gph0_vamp1, gph0_vamp2, gph0_vamp4,"brwn", "drkg", "drkcy", "drkr", "ofw", "drkb", "drkgry", "trace 1", "trace 2", "trace 3", "trace 4"  );
//fgph.plotxy( gph0_vx, gph0_vamp0, gph0_vamp1 );
//fgph.plotxy( fgph_vx, fgph_vy0, fgph_vy1 );

fgph.plotxy( fgph_vx, fgph_vy0, "drky", "ofw", "drkb", "blk", "pc srate" );	//3 traces cols, bkgd col, axis col, text col, and trace colour coded labels, see defined colours: 'user_col'

//fgph.plotxy( gph0_vx, gph0_vamp2 );
//fgph.plot( 0,gph0_vamp2 );

aud_pos = 0;												//start audio from beginning

if(show_lpc) printf("\n");



stop_lpc_addr = (int)ptrAddr;
last_render_rom_byte_cnt = lpccnt;//stop_lpc_addr - start_lpc_addr + 1;
printf("stop  lpc[%03d]: %02x  lpccnt: %d, (addr delta: %d)\n", last_render_rom_byte_cnt, *ptrAddr, lpccnt, last_render_rom_byte_cnt );		



phrase_cnt++;

saf.encoding = 3;
saf.offset = 0;
saf.format = en_af_sun;
saf.channels = 2;
saf.srate = lpc_srate;

af.save_malloc( "", "zz_cummulative.au", 32767, saf );

mode = 1;
aud_pos = 0;

strpf( s1, "%04x", offset + last_render_rom_byte_cnt );
fi_addr_snd_end->value( s1.c_str() );
}







//
//convert comma delimited hex string and store in bf[]
//returns bytes stored
//e.g: "01, af, ff, 0x01,0x80,0x7f, 7f, 0x01, 7fff, 8000"
int str_to_hex_buf( string ss, uint16_t *bf, int bfsz  )
{
string s1;
mystr m1;

m1 = ss;
m1.FindReplace( s1, "0x", "", 0);

m1 = s1;	
m1.FindReplace( s1, "\r", "", 0);

m1 = s1;	
m1.FindReplace( s1, "\n", "", 0);
m1 = s1;


//printf("str_to_hex_buf() - line: '%s'\n", s1.c_str() );


//m1 = "01, af, ff, 1, 256, 257";
int cnt = m1.LoadArray_hex_uint16_t( bf, bfsz, ',' );

return cnt;
}








//
//convert comma delimited decimal string and store in bf[]
//returns bytes stored
//e.g: "01, 02, -25, -32768, 32767"
int str_to_decimal_buf( string ss, int16_t *bf, int bfsz  )
{
string s1;
mystr m1;

m1 = ss;
m1.FindReplace( s1, "\r", "", 0);

m1 = s1;	
m1.FindReplace( s1, "\n", "", 0);
m1 = s1;


//printf("str_to_hex_buf() - line: '%s'\n", s1.c_str() );


//m1 = "01, af, ff, 1, 256, 257";
int cnt = m1.LoadArray_decimal_int16_t( bf, bfsz, ',' );

return cnt;
}












void lattice_iir( int cnt, float inp[], int kcnt, float koef[], float gstate[], float outp[] )
{

for( int n = 0; n < cnt; n++ )
	{
	outp[ n ] = inp[ n ];
	for( int i = 0; i < kcnt; i++ )
		{
		outp[ n ] = outp[ n ] - koef[ i ] * gstate[ i ];				//fwd
		gstate[ i + 1 ] = koef[ i ] * outp[ n ] + gstate[ i ];			//rev
		}

//	for( int i = 0; i < kcnt; i++ )
		{
		}
	}

}








/*
float yy0 = 0;
float yy1 = 0;

bool process_audio_tmc0280_hold( int16_t* buf, int16_t* bf2, int bufsz, bool showframes, float interp_step, bool newframe ) 
{
bool vb = 0;
  static uint8_t nextPwm;
  static uint8_t periodCounter;
  static float x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10;
  float u0,u1,u2,u3,u4,u5,u6,u7,u8,u9,u10;

int synthPeriod_interp = 0;

int16_t grrr;

if( synthPeriod == 0 ) interp_step = 1.0;					//disable interpolation during un-voiced frames

//interp_step = 1.0;
//interp_step = 0.0;

//printf("synthPeriod: %d, synthPeriodLast: %d, interp_step: %f\n", (int)synthPeriod, (int)synthPeriodLast, interp_step );
synthPeriod_interp = (1.0 - interp_step) * synthPeriodLast  +  interp_step * synthPeriod;		//do a linear interp mix

uint16_t synthEnergy_interp = (1.0 - interp_step) * synthEnergyLast  +  interp_step * synthEnergy;		//do a linear interp mix

//printf("synthK1Last: %d, synthK1: %d ", synthK1Last, synthK1 );

if( ( synthPeriod_interp == 11 ) & (dbg_dump != 0 ) ) dbg_dump = 0;






float K1, K2, K3, K4, K5, K6, K7, K8, K9, K10;


//printf("interp_step: %.3f, K1: %f ", interp_step, K1 );
K1 = (1.0 - interp_step) * synthK1Last  +  interp_step * synthK1;				//do a linear interp mix
K2 = (1.0 - interp_step) * synthK2Last  +  interp_step * synthK2;
K3 = (1.0 - interp_step) * synthK3Last  +  interp_step * synthK3;
K4 = (1.0 - interp_step) * synthK4Last  +  interp_step * synthK4;
K5 = (1.0 - interp_step) * synthK5Last  +  interp_step * synthK5;
K6 = (1.0 - interp_step) * synthK6Last  +  interp_step * synthK6;
K7 = (1.0 - interp_step) * synthK7Last  +  interp_step * synthK7;
K8 = (1.0 - interp_step) * synthK8Last  +  interp_step * synthK8;
K9 = (1.0 - interp_step) * synthK9Last  +  interp_step * synthK9;
K10 = (1.0 - interp_step) * synthK10Last  +  interp_step * synthK10;

/*
koef[ 0 ] = K1;
koef[ 1 ] = K2;
koef[ 2 ] = K3;
koef[ 3 ] = K4;
koef[ 4 ] = K5;
koef[ 5 ] = K6;
koef[ 6 ] = K7;
koef[ 7 ] = K8;
koef[ 8 ] = K9;
koef[ 9 ] = K10;
*/

/*
koef[ 9 ] = K1;
koef[ 8 ] = K2;
koef[ 7 ] = K3;
koef[ 6 ] = K4;
koef[ 5 ] = K5;
koef[ 4 ] = K6;
koef[ 3 ] = K7;
koef[ 2 ] = K8;
koef[ 1 ] = K9;
koef[ 0 ] = K10;
*/






/*
//printf("bufsz: %04d\n", bufsz );
printf("synthPeriod_interp: %d\n", synthPeriod_interp );
//int pitch_modulo = 0;
for( int i = 0; i < bufsz; i++ )
	{
	if(showframes)printf("data:\n" );
	if (synthPeriod) 
		{
		// Voiced source
//synthEnergy_interp = 128;
		
		if( periodCounter < 41 )
			{
if(dbg_dump<10) printf("%02d~%.2f ", (int)periodCounter, u10);
			u10 = ((float)( (int8_t*)chirp_0280)[periodCounter] / 256.0) * (synthEnergy_interp / 256.0);
//periodCounter++;
			}
		else{
if(dbg_dump<10) printf("c%02d ", (int)periodCounter);
			u10 = 0;
//periodCounter=0;
			}

		int chirp_cnt = CHIRP_SIZE_0280;

//		pitch_modulo++;
//		if( pitch_modulo >= 1 )
//			{
//			pitch_modulo = 0;
//			}

		if (periodCounter >= synthPeriod_interp )
			{
if(dbg_dump<10)  printf("clear %02d %02d__", periodCounter, synthPeriod_interp );
			periodCounter = 0;
			}
		else{
			periodCounter++;
			}
		}
	else{
		// Unvoiced source
		static uint16_t synthRand = 1;
		synthRand = (synthRand >> 1) ^ ((synthRand & 1) ? 0xB800 : 0);
		u10 = (synthRand & 1) ? synthEnergy_interp : -synthEnergy_interp;

		u10 /= 256.0;

		}

if( newframe )
	{
newframe = 0;
	bf2[ i ] = 0.5 * 1024.0;
if( synthPeriod == 0 ) bf2[ i ] = 0.4 * 1024.0;
	}
else{
	bf2[ i ] = u10 * 1024.0;
	}

	inp[ i ] = u10;

	float sin1;
	if( 0 )
		{
//		sin1= sin( theta1 );

//		theta1 += theta1_inc;
//		if( theta1 >= twopi ) theta1 -= twopi;
		u0 = sin1;
		}


float yy;


float a1 = -1.287;
float a2 = 0.8282;


yy = u0 / 4.0 -a1 * yy0 - a2 * yy1;

yy1 = yy0;
yy0 = yy;


//	buf[ i ] = yy * 1500.0;

	if( u0 < imin ) imin = u0;
	if( u0 > imax ) imax = u0;

//	if(showframes)printf("[%05d]: %02x\n", bufptr - 1, u0 );
//	if(showframes)printf("%02x ", (int)u0 );
//	if( !(bufptr % 100) ) if(vb) printf("\n");
//	if( bufptr >= bufsz )
//		{
	//getchar();
//		return 1;
//		}
	}


lattice_iir( bufsz, inp, 10, koef, gstate, outp );

for( int i = 0; i < bufsz; i++ )
	{
	buf[ i ] = outp[ i ] * 1.0;
	}


if(showframes)printf( "\n" );
return 1;
}













/*
//float yy0 = 0;
//float yy1 = 0;

bool process_audio_tmc0280( int16_t* buf, int16_t* bf2, int16_t* bf3, int bufsz, bool showframes, float interp_step, bool newframe, float chirp_sub_steps, bool skip_interp, float chirp_sub_modulo ) 
{
bool vb = 0;
  static uint8_t nextPwm;
  float u0,u1,u2,u3,u4,u5,u6,u7,u8,u9,u10;
		float yy;



int synthPeriod_interp = 0;

int16_t grrr;

//if( synthPeriod == 0 ) interp_step = 1.0;					//disable interpolation during un-voiced frames

if(skip_interp) interp_step = 1.0;


//interp_step = 1.0;
//interp_step = 0.0;

//printf("synthPeriod: %d, synthPeriodLast: %d, interp_step: %f\n", (int)synthPeriod, (int)synthPeriodLast, interp_step );
synthPeriod_interp = (1.0 - interp_step) * synthPeriodLast  +  interp_step * synthPeriod;		//do a linear interp mix

uint16_t synthEnergy_interp = (1.0 - interp_step) * synthEnergyLast  +  interp_step * synthEnergy;		//do a linear interp mix

//printf("synthK1Last: %d, synthK1: %d ", synthK1Last, synthK1 );

if( ( synthPeriod_interp == 11 ) & (dbg_dump != 0 ) ) dbg_dump = 0;

//printf("\ninterp_step: %.4f\n", interp_step );
//printf("skip_interp: %d\n", skip_interp );
//printf("synthEnergy_interp: %d, synthPeriod_interp: %d\n", synthEnergy_interp, synthPeriod_interp );
//int pitch_modulo = 0;




float raw_stimul;

bool first = 1;
for( int i = 0; i < bufsz; i++ )
	{
//printf("chirp_sub_modulo: %f\n", chirp_sub_modulo );
//printf("chirp_sub_steps: %f\n", chirp_sub_steps );
//printf("chirp_step: %f\n", chirp_step );
	if(showframes)printf("data:\n" );
	if( ( chirp_step >= chirp_sub_modulo ) | first )							//chirp_sub_modulo: 1/8000, chirp_sub_steps: 1/srate 
		{
		first = 0;
		if (synthPeriod)
			{
	//printf("chirp_sub_steps: %f, chirp_step: %f\n", chirp_sub_steps, chirp_step );
			// Voiced source
	//synthEnergy_interp = 128;


			if( periodCounter < 41 )
				{
//		if(dbg_dump<10) printf("%02d~%.2f ", (int)periodCounter, u10);
				u10 = (float)((float)( (int8_t*)chirp_0280)[periodCounter] / 256.0) * (synthEnergy_interp / 256.0);
	//periodCounter++;
				}
			else{
//		if(dbg_dump<10) printf("c%02d ", (int)periodCounter);
				u10 = 0;
	//periodCounter=0;
				}

			int chirp_cnt = CHIRP_SIZE_0280;

	//		pitch_modulo++;
	//		if( pitch_modulo >= 1 )
	//			{
	//			pitch_modulo = 0;
	//			}

			if (periodCounter >= synthPeriod_interp )
				{
	if(dbg_dump<10)  printf("clear %02d %02d__", periodCounter, synthPeriod_interp );
				periodCounter = 0;
				}
			else{
				periodCounter++;
				}
			}
		else{
			// Unvoiced source
			synthRand = (synthRand >> 1) ^ ((synthRand & 1) ? 0xB800 : 0);
			u10 = (synthRand & 1) ? synthEnergy_interp : -synthEnergy_interp;

			u10 /= 2048.0;
			}

//if( dbg_cnt == 0 ) u10 = 0.5;							//impulse lattice iir check
//else u10 = 0.0;

		raw_stimul = u10;


		float K1, K2, K3, K4, K5, K6, K7, K8, K9, K10;

		koef[ 0 ] = K1;
		koef[ 1 ] = K2;
		koef[ 2 ] = K3;
		koef[ 3 ] = K4;
		koef[ 4 ] = K5;
		koef[ 5 ] = K6;
		koef[ 6 ] = K7;
		koef[ 7 ] = K8;
		koef[ 8 ] = K9;
		koef[ 9 ] = K10;

	//printf("interp_step: %.3f, K1: %f ", interp_step, K1 );
		K1 = (1.0 - interp_step) * synthK1Last  +  interp_step * synthK1;				//do a linear interp mix
		K2 = (1.0 - interp_step) * synthK2Last  +  interp_step * synthK2;
		K3 = (1.0 - interp_step) * synthK3Last  +  interp_step * synthK3;
		K4 = (1.0 - interp_step) * synthK4Last  +  interp_step * synthK4;
		K5 = (1.0 - interp_step) * synthK5Last  +  interp_step * synthK5;
		K6 = (1.0 - interp_step) * synthK6Last  +  interp_step * synthK6;
		K7 = (1.0 - interp_step) * synthK7Last  +  interp_step * synthK7;
		K8 = (1.0 - interp_step) * synthK8Last  +  interp_step * synthK8;
		K9 = (1.0 - interp_step) * synthK9Last  +  interp_step * synthK9;
		K10 = (1.0 - interp_step) * synthK10Last  +  interp_step * synthK10;

		  // Lattice filter forward path
		  u9 = u10 - ( ( (float)K10 / 512.0) ) * x9;
		  u8 = u9 - ( ( (float)K9 / 512.0) ) * x8;
		  u7 = u8 - ( ( (float)K8 / 512.0) ) * x7;
		  u6 = u7 - ( ( (float)K7 / 512.0) ) * x6;
		  u5 = u6 - ( ( (float)K6 / 512.0) ) * x5;
		  u4 = u5 - ( ( (float)K5 / 512.0) ) * x4;
		  u3 = u4 - ( ( (float)K4 / 512.0) ) * x3;
		  u2 = u3 - ( ( (float)K3 / 512.0) ) * x2;
		  u1 = u2 - ( ( (float)K2 / 512.0) ) * x1;
		  u0 = u1 - ( ( (float)K1 / 512.0) ) * x0;

		  // Output clamp
		  if (u0 > 1.0) u0 = 1.0;
		  if (u0 < -1.0) u0 = -1.0;
		  
		  // Lattice filter reverse path
		  x9 = x8 + ( ( (float)K9 / 512.0) ) * u8;
		  x8 = x7 + ( ( (float)K8 / 512.0) ) * u7;
		  x7 = x6 + ( ( (float)K7 / 512.0) ) * u6;
		  x6 = x5 + ( ( (float)K6 / 512.0) ) * u5;
		  x5 = x4 + ( ( (float)K5 / 512.0) ) * u4;
		  x4 = x3 + ( ( (float)K4 / 512.0) ) * u3;
		  x3 = x2 + ( ( (float)K3 / 512.0) ) * u2;
		  x2 = x1 + ( ( (float)K2 / 512.0) ) * u1;
		  x1 = x0 + ( ( (float)K1 / 512.0) ) * u0;
		  x0 = u0;

if( !use_filter ) u0 = raw_stimul;

		//  nextPwm = (u0>>2)+0x80;

		double theta1_inc = 400 * twopi / srate;
		float sin1;
		if( 0 )
			{
//			sin1= sin( theta1 );

//			theta1 += theta1_inc;
//			if( theta1 >= twopi ) theta1 -= twopi;
			u0 = sin1;
			}


//		float a1 = -1.287;
//		float a2 = 0.8282;


//		yy = u0 / 4.0 -a1 * yy0 - a2 * yy1;

//		yy1 = yy0;
//		yy0 = yy;
//printf("chirp_step1: %f\n", chirp_step );
		chirp_step -= chirp_sub_modulo;
//printf("chirp_step2: %f\n",chirp_step );
		}
	else{
	
//buf4[ i ] = 0;
		chirp_step += chirp_sub_steps;
		}
buf4[ i ] = chirp_step * 32767.0 * 20;
buf4[ i ] = i;

	if( newframe )
		{
	newframe = 0;

		bf2[ i ] = 0.5 * 1024.0;								//wfm marker for debugging
		if( synthPeriod == 0 ) bf2[ i ] = 0.4 * 1024.0;
//bf2[ i ] = u10 * 1024.0;
		}
	else{
		bf2[ i ] = periodCounter * 50.0;
		}

bf3[ i ] = raw_stimul * 512.0;

//if( synthPeriod == 0 ) bf2[ i ] = 256;
//else bf2[ i ] = 0;


	buf[ i ] = u0 * 1500.0;

	if( u0 < imin ) imin = u0;
	if( u0 > imax ) imax = u0;

//	if(showframes)printf("[%05d]: %02x\n", bufptr - 1, u0 );
//	if(showframes)printf("%02x ", (int)u0 );
//	if( !(bufptr % 100) ) if(vb) printf("\n");
//	if( bufptr >= bufsz )
//		{
	//getchar();
//		return 1;
//		}
dbg_cnt++;
	}

if(showframes)printf( "\n" );
return 1;
}
*/












/*

int dbg_cnt = 0;
double theta1 = 0;

//float yy0 = 0;
//float yy1 = 0;

bool process_audio_tmc0280( int16_t* buf, int16_t* bf2, int16_t* bf3, int bufsz, bool showframes, float interp_step, bool newframe, float chirp_sub_steps, bool skip_interp, float chirp_sub_modulo ) 
{
bool vb = 0;
  static uint8_t nextPwm;
  float u0,u1,u2,u3,u4,u5,u6,u7,u8,u9,u10;
		float yy;



int synthPeriod_interp = 0;

int16_t grrr;

//if( synthPeriod == 0 ) interp_step = 1.0;					//disable interpolation during un-voiced frames

if(skip_interp) interp_step = 1.0;


//interp_step = 1.0;
//interp_step = 0.0;

//printf("synthPeriod: %d, synthPeriodLast: %d, interp_step: %f\n", (int)synthPeriod, (int)synthPeriodLast, interp_step );
synthPeriod_interp = (1.0 - interp_step) * synthPeriodLast  +  interp_step * synthPeriod;		//do a linear interp mix

uint16_t synthEnergy_interp = (1.0 - interp_step) * synthEnergyLast  +  interp_step * synthEnergy;		//do a linear interp mix

//printf("synthK1Last: %d, synthK1: %d ", synthK1Last, synthK1 );

if( ( synthPeriod_interp == 11 ) & (dbg_dump != 0 ) ) dbg_dump = 0;

//printf("\ninterp_step: %.4f\n", interp_step );
//printf("skip_interp: %d\n", skip_interp );
//printf("synthEnergy_interp: %d, synthPeriod_interp: %d\n", synthEnergy_interp, synthPeriod_interp );
//int pitch_modulo = 0;




float raw_stimul;

bool first = 1;
for( int i = 0; i < bufsz; i++ )
	{
//printf("chirp_sub_modulo: %f\n", chirp_sub_modulo );
//printf("chirp_sub_steps: %f\n", chirp_sub_steps );
//printf("chirp_step: %f\n", chirp_step );
	if(showframes)printf("data:\n" );
	if( ( chirp_step >= chirp_sub_modulo ) | first )							//chirp_sub_modulo: 1/8000, chirp_sub_steps: 1/srate 
		{
		first = 0;
		if (synthPeriod)
			{
	//printf("chirp_sub_steps: %f, chirp_step: %f\n", chirp_sub_steps, chirp_step );
			// Voiced source
	//synthEnergy_interp = 128;


			if( periodCounter < 41 )
				{
//		if(dbg_dump<10) printf("%02d~%.2f ", (int)periodCounter, u10);
				u10 = (float)((float)( (int8_t*)chirp_0280)[periodCounter] / 256.0) * (synthEnergy_interp / 256.0);
	//periodCounter++;
				}
			else{
//		if(dbg_dump<10) printf("c%02d ", (int)periodCounter);
				u10 = 0;
	//periodCounter=0;
				}

			int chirp_cnt = CHIRP_SIZE_0280;

	//		pitch_modulo++;
	//		if( pitch_modulo >= 1 )
	//			{
	//			pitch_modulo = 0;
	//			}

			if (periodCounter >= synthPeriod_interp )
				{
	if(dbg_dump<10)  printf("clear %02d %02d__", periodCounter, synthPeriod_interp );
				periodCounter = 0;
				}
			else{
				periodCounter++;
				}
			}
		else{
			// Unvoiced source
			synthRand = (synthRand >> 1) ^ ((synthRand & 1) ? 0xB800 : 0);
			u10 = (synthRand & 1) ? synthEnergy_interp : -synthEnergy_interp;

			u10 /= 2048.0;
			}

//if( dbg_cnt == 0 ) u10 = 0.5;							//impulse lattice iir check
//else u10 = 0.0;

		raw_stimul = u10;

		inp[ i ] = u10;


		float K1, K2, K3, K4, K5, K6, K7, K8, K9, K10;

		koef[ 0 ] = K1;
		koef[ 1 ] = K2;
		koef[ 2 ] = K3;
		koef[ 3 ] = K4;
		koef[ 4 ] = K5;
		koef[ 5 ] = K6;
		koef[ 6 ] = K7;
		koef[ 7 ] = K8;
		koef[ 8 ] = K9;
		koef[ 9 ] = K10;

	//printf("interp_step: %.3f, K1: %f ", interp_step, K1 );
		K1 = (1.0 - interp_step) * synthK1Last  +  interp_step * synthK1;				//do a linear interp mix
		K2 = (1.0 - interp_step) * synthK2Last  +  interp_step * synthK2;
		K3 = (1.0 - interp_step) * synthK3Last  +  interp_step * synthK3;
		K4 = (1.0 - interp_step) * synthK4Last  +  interp_step * synthK4;
		K5 = (1.0 - interp_step) * synthK5Last  +  interp_step * synthK5;
		K6 = (1.0 - interp_step) * synthK6Last  +  interp_step * synthK6;
		K7 = (1.0 - interp_step) * synthK7Last  +  interp_step * synthK7;
		K8 = (1.0 - interp_step) * synthK8Last  +  interp_step * synthK8;
		K9 = (1.0 - interp_step) * synthK9Last  +  interp_step * synthK9;
		K10 = (1.0 - interp_step) * synthK10Last  +  interp_step * synthK10;

		  // Lattice filter forward path
		  u9 = u10 - ( ( (float)K10 / 512.0) ) * x9;
		  u8 = u9 - ( ( (float)K9 / 512.0) ) * x8;
		  u7 = u8 - ( ( (float)K8 / 512.0) ) * x7;
		  u6 = u7 - ( ( (float)K7 / 512.0) ) * x6;
		  u5 = u6 - ( ( (float)K6 / 512.0) ) * x5;
		  u4 = u5 - ( ( (float)K5 / 512.0) ) * x4;
		  u3 = u4 - ( ( (float)K4 / 512.0) ) * x3;
		  u2 = u3 - ( ( (float)K3 / 512.0) ) * x2;
		  u1 = u2 - ( ( (float)K2 / 512.0) ) * x1;
		  u0 = u1 - ( ( (float)K1 / 512.0) ) * x0;

		  // Output clamp
		  if (u0 > 1.0) u0 = 1.0;
		  if (u0 < -1.0) u0 = -1.0;
		  
		  // Lattice filter reverse path
		  x9 = x8 + ( ( (float)K9 / 512.0) ) * u8;
		  x8 = x7 + ( ( (float)K8 / 512.0) ) * u7;
		  x7 = x6 + ( ( (float)K7 / 512.0) ) * u6;
		  x6 = x5 + ( ( (float)K6 / 512.0) ) * u5;
		  x5 = x4 + ( ( (float)K5 / 512.0) ) * u4;
		  x4 = x3 + ( ( (float)K4 / 512.0) ) * u3;
		  x3 = x2 + ( ( (float)K3 / 512.0) ) * u2;
		  x2 = x1 + ( ( (float)K2 / 512.0) ) * u1;
		  x1 = x0 + ( ( (float)K1 / 512.0) ) * u0;
		  x0 = u0;

if( !use_filter ) u0 = raw_stimul;

		//  nextPwm = (u0>>2)+0x80;

		double theta1_inc = 400 * twopi / srate;
		float sin1;
		if( 0 )
			{
//			sin1= sin( theta1 );

//			theta1 += theta1_inc;
//			if( theta1 >= twopi ) theta1 -= twopi;
			u0 = sin1;
			}


//		float a1 = -1.287;
//		float a2 = 0.8282;


//		yy = u0 / 4.0 -a1 * yy0 - a2 * yy1;

//		yy1 = yy0;
//		yy0 = yy;
//printf("chirp_step1: %f\n", chirp_step );
		chirp_step -= chirp_sub_modulo;
//printf("chirp_step2: %f\n",chirp_step );
		}
	else{
	
//buf4[ i ] = 0;
		chirp_step += chirp_sub_steps;
		}
buf4[ i ] = chirp_step * 32767.0 * 20;
buf4[ i ] = i;

	if( newframe )
		{
	newframe = 0;

		bf2[ i ] = 0.5 * 1024.0;								//wfm marker for debugging
		if( synthPeriod == 0 ) bf2[ i ] = 0.4 * 1024.0;
//bf2[ i ] = u10 * 1024.0;
		}
	else{
		bf2[ i ] = periodCounter * 50.0;
		}

bf3[ i ] = raw_stimul * 512.0;

//if( synthPeriod == 0 ) bf2[ i ] = 256;
//else bf2[ i ] = 0;


	buf[ i ] = u0 * 1500.0;

	if( u0 < imin ) imin = u0;
	if( u0 > imax ) imax = u0;

//	if(showframes)printf("[%05d]: %02x\n", bufptr - 1, u0 );
//	if(showframes)printf("%02x ", (int)u0 );
//	if( !(bufptr % 100) ) if(vb) printf("\n");
//	if( bufptr >= bufsz )
//		{
	//getchar();
//		return 1;
//		}
dbg_cnt++;
	}

if(showframes)printf( "\n" );
return 1;
}
*/












bool load_audio( string fname )
{
//string slast_SF2_file_list_path = "/mnt/home/PuppyLinux/Packages/fluid_sf2";


printf("load_audio() - loading audio file: '%s'\n", fname.c_str() );

af.verb = 1;                                //store progress in: string 'log'
af.log_to_printf = 1;                       //show to console aswell

en_audio_formats fmt;

if( !af.find_file_format( "", fname, fmt ) )
	{
	printf("load_audio() - unable to determine audio file format: '%s'\n", fname.c_str() );
	return 0;
	}
else{
	printf("load_audio() - determined audio file format is: %d, '%s'\n", (int)fmt, fname.c_str() );
	}


saf.format = fmt;

if( !af.load_malloc( "", fname, 0, saf ) )
	{
	printf("load_audio() - unable to read audio file: '%s'\n", fname.c_str() );
	return 0;
	}
else{
	printf( "load_audio() - read audio file: '%s'\n", fname.c_str() );
	printf( "load_audio() - saf.srate: %d\n", saf.srate ); 
	printf( "load_audio() - saf.channels: %d\n", saf.channels ); 
	printf( "load_audio() - saf.format: %d\n", saf.format ); 
	printf( "load_audio() - saf.encoding: %d\n", saf.encoding ); 
	printf( "load_audio() - saf.is_big_endian: %d\n", saf.is_big_endian ); 
	}

int srate;

srate = saf.srate;


uint64_t aud_len = af.sizech0;
//if( af.sizech1 < af.sizech0 ) aud_len = af.sizech1;

int aud_channels = saf.channels;


printf("load_audio() - channels: %d, srate: %d\n", aud_channels,  srate );
printf("load_audio() - sample count: %" PRIu64 ", length (secs): %f\n", aud_len, (float)aud_len / srate );

//saf.encoding = 1;
//saf.offset = 0;
//saf.format = en_af_sun;
//saf.channels = 1;
//saf.srate = 48000;
//af.save( "", "junk.au", 32767, saf );

return 1;
}




bool load_vsm( uint8_t *vsmem, string fname, int count )
{

FILE *fp = fopen( fname.c_str(), "rb" );

if( fp == 0 )
	{
	printf("load_vsm() - failed to open file: '%s'\n", fname.c_str() );
	return 0;
	}

int read = fread( vsmem, 1, count, fp );

if( read != count )
	{
	printf("load_vsm() - read: %d bytes, requested: %d, file: '%s'\n", read, count, fname.c_str() );
	}

printf("load_vsm() - read: %d bytes, file: '%s'\n", read, fname.c_str() );

fclose( fp );


/*
//---- save a bit reversed version ----- (not really useful as I think the rom dumps are not reversed)
Talkie o;

fp = fopen( "zzrev_vsmrom.bin", "wb" );

uint8_t buf[ 10 ];
for( int i = 0; i < count; i++ )
	{
	buf[ 0 ] = o.rev( vsmem[ i ] );
	fwrite( buf, 1, 1, fp );
	}
fclose( fp );
*/

//-------------------------------------

return 1;
}








/*
void create_filters()
{
fir1.created = 0;
fir1.bypass = 0;

fir2.created = 0;
fir2.bypass = 0;



if( !create_filter_from_file( fir1, "fir_fs8k_lpf3k5.txt" ) )
	{
	printf("create_filters() - failed to create fir1\n" );
	exit( 0 );
	}

if( !create_filter_from_file( fir2, "fir_fs8k_lpf3k5.txt" ) )
	{
	printf("create_filters() - failed to create fir2\n" );
	exit( 0 );
	}

}
*/













void cb_wndmain(Fl_Widget*, void* v)
{
Fl_Window* wnd = (Fl_Window*)v;

//wndMain->iconize();
//wndMain->hide();
//wndMain->show();

//do_quit_via_timer = 1;
DoQuit();

}









//get a rom address ptr for spec offs
uint16_t get_ptr( uint8_t *rom_addr, int offs )
{
	
uint16_t ui = vsm[ offs ];									//get lpc's data addr
ui |= vsm[ offs + 1 ] << 8;
return ui;
}






//make list: of rom ptr addresses paired with their alpha/phrase/word
bool build_rom_word_addr_list( int offs, string fname )
{
string s1, s2, st;

if( !load_vsm( vsm + offs, fname, rom_size_max ) ) return 0;

uint16_t ui;

//letters
int pp = 0xc;
//pp = 0x8;
for( int i = 0; i < 26; i++ )
	{
	ui = get_ptr( vsm, pp );						//get lpc's data addr
	pp += 2;
	
	char cc = i + 0x41;
	strpf( s1, "%04x %c\n", ui, cc );
	st += s1;
	}

st += "-----------------\n";


ui = vsm[ pp ];									//get lpc's data addr
ui |= vsm[ pp + 1 ] << 8;
pp += 2;

strpf( s1, "%04x beep\n", ui );
st += s1;

st += "-----------------\n";

//numbers 0->9
for( int i = 0; i <10; i++ )
	{
	ui = get_ptr( vsm, pp );						//get lpc's data addr
	
	pp += 2;										//get lpc's data addr

	strpf( s1, "%04x %c\n", ui, (char)(i + 0x30) );
	st += s1;
	}


ui = get_ptr( vsm, pp );						//get lpc's data addr
pp += 2;

strpf( s1, "%04x 10\n", ui );
st += s1;

st += "-----------------\n";


ui = get_ptr( vsm, pp );						//get lpc's data addr
pp += 2;

strpf( s1, "%04x that is correct\n", ui );
st += s1;



ui = get_ptr( vsm, pp );						//get lpc's data addr
pp += 2;

strpf( s1, "%04x you are correct\n", ui );
st += s1;


ui = get_ptr( vsm, pp );						//get lpc's data addr
pp += 2;

strpf( s1, "%04x that is right\n", ui );
st += s1;


ui = get_ptr( vsm, pp );						//get lpc's data addr
pp += 2;

strpf( s1, "%04x you are right\n", ui );
st += s1;



//printf("pp: %04x\n", pp );

int p2 = get_ptr( vsm, pp );					//get rom ptr
ui = get_ptr( vsm, p2 );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x wrong\n", ui );
st += s1;




p2 = get_ptr( vsm, pp );						//get rom ptr
ui = get_ptr( vsm, p2 );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x that is incorrect\n", ui );
st += s1;




p2 = get_ptr( vsm, pp );						//get rom ptr
ui = get_ptr( vsm, p2 );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x spell\n", ui );
st += s1;




p2 = get_ptr( vsm, pp );						//get rom ptr
ui = get_ptr( vsm, p2 );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x now spell\n", ui );
st += s1;




p2 = get_ptr( vsm, pp );						//get rom ptr
ui = get_ptr( vsm, p2 );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x next spell\n", ui );
st += s1;




p2 = get_ptr( vsm, pp );						//get rom ptr
ui = get_ptr( vsm, p2 );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x now try\n", ui );
st += s1;




p2 = get_ptr( vsm, pp );						//get rom ptr
ui = get_ptr( vsm, p2 );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x try\n", ui );
st += s1;




ui = get_ptr( vsm, pp );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x say it\n", ui );
st += s1;




ui = get_ptr( vsm, pp );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x I win\n", ui );
st += s1;




ui = get_ptr( vsm, pp );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x you win\n", ui );
st += s1;



p2 = get_ptr( vsm, pp );						//get rom ptr
ui = get_ptr( vsm, p2 );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x here is your score\n", ui );
st += s1;


ui = get_ptr( vsm, pp );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x perfect score\n", ui );
st += s1;



ui = get_ptr( vsm, pp );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x tones\n", ui );
st += s1;



ui = get_ptr( vsm, pp );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x tones\n", ui );
st += s1;


ui = get_ptr( vsm, pp );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x tones\n", ui );
st += s1;


ui = get_ptr( vsm, pp );						//get lpc's data addr

pp += 2;

strpf( s1, "%04x tones\n", ui );
st += s1;


st += "-----------------\n";
//tb_romaddr->text( st.c_str() );

bool vb = 1;


//the four word lists

for( int k = 0; k < 4; k++ )									//cycle 4 word lists
	{
	int word_cnt = vsm[ k ];
	int word_list_addr = vsm[ 4 + k * 2 ];						//start of a word list
	word_list_addr |= vsm[ 4 + k * 2 + 1 ]<<8;


	for( int i = 0; i < word_cnt; i += 2 )
		{
		int word_ptr = vsm[ word_list_addr + i ];				//get curr word's ptr addr
		word_ptr |= vsm[ word_list_addr + i + 1 ]<<8;			


int text_addr = word_ptr;

		
		s1 = "";		
		for( int j = 0; j < 8; j++ )							//max of a letters
			{

			int byt = vsm[ word_ptr + j ];		
			int asc = byt & 0x3f;
			asc += 0x41;										//make ascii letter
			if( asc == '[' ) asc  = '\'';						//as in COULDN[T
			s1 += asc;
			if( byt & 0x40 )									//end of the word's spelling
				{
				int lpc_ptr = vsm[ word_ptr + j + 1 ];			//get curr word's lpc data addr
				lpc_ptr |= vsm[ word_ptr + j + 2 ]<<8;			
			
				strpf( s2, "%04x ", lpc_ptr );
				st += s2;
				st += s1;
				st += "\n";
				break;
				}
			}
if(vb)printf("word list %d, text_addr: 0x%04x, lpc_addr: %s  text: %s\n", k, text_addr, s2.c_str(), s1.c_str() );
		}


	st += "-----------------\n";
	}
tb_wordlist->text( st.c_str() );

return 0;
}





void cb_timer1(void *)
{
string s1;
mystr m1;


Fl::repeat_timeout( cn_timer_period, cb_timer1 );




//printf( "cb_timer1()\n" );

// --- do transitional states ----
if( addr_step_auto_state == 0 )
	{
	led_addr_step0->ChangeCol( 0 );
	
	addr_step_auto_delay_sum = 0;		//zero delay sum

	goto done_auto_state;
	}

if( addr_step_auto_state == 1 )
	{
	s1 = fi_addr_step_delay->value();
	sscanf( s1.c_str(), "%f", &addr_step_auto_val_delay );
	if( addr_step_auto_val_delay < cn_timer_period ) addr_step_auto_val_delay = cn_timer_period;
	if( addr_step_auto_val_delay > 10 ) addr_step_auto_val_delay = 10;
	

	s1 = fi_addr_step_delta->value();
	sscanf( s1.c_str(), "%d", &addr_step_auto_val_delta );
	if( addr_step_auto_val_delta == 0 ) addr_step_auto_val_delta = 1;
	if( addr_step_auto_val_delta < -256 ) addr_step_auto_val_delta = -256;
	if( addr_step_auto_val_delta > 256 ) addr_step_auto_val_delta = 256;

	addr_step_auto_delay_sum = 0;		//zero delay sum

	led_addr_step0->ChangeCol( 1 );
	addr_step_auto_state = 2;
	goto done_auto_state;
	}

if( addr_step_auto_state == 2 )
	{
	addr_step_auto_delay_sum = 0;		//zero delay sum

	talk.say_repeat();					//sound speech
	
	addr_step_auto_state = 3;
	goto done_auto_state;
	}

if( addr_step_auto_state == 3 )			//delay starts counting up when here
	{
	if( mode == 0 ) 
		{
		addr_step_auto_state = 4;
		led_addr_step0->ChangeCol( 2 );
		}
	goto done_auto_state;
	}

if( addr_step_auto_state == 4 )
	{
	if( addr_step_auto_delay_sum > addr_step_auto_val_delay )
		{
		addr_step_auto_state = 5;
		}
	goto done_auto_state;
	}
	
if( addr_step_auto_state == 5 )
	{
	addr_step_auto_delay_sum = 0;		//zero delay sum

	if( !modify_addr( addr_step_auto_val_delta ) )		//addr limits reached ?
		{
		addr_step_auto_state = 0;	
		}
	else{
		addr_step_auto_state = 6;
		}
		
	addr_add_history( last_say_offset );
	goto done_auto_state;
	}

if( addr_step_auto_state == 6 )
	{
	addr_step_auto_delay_sum = 0;		//zero delay sum

	addr_step_auto_count++;
	
	addr_step_auto_state = 1;
	
	goto done_auto_state;
	}

done_auto_state:
//----------------------------

//printf( "state %d, mode: %d, sum %f\n", addr_step_auto_state, mode, addr_step_auto_delay_sum );




addr_step_auto_delay_sum += cn_timer_period;

strpf( s1, "%06d", addr_step_auto_count );
fi_addr_step_count->value( s1.c_str() );

if( vaddr_hist.size() == 0 )
	{
	strpf( s1, "addr history (0/0):" );
	}
else{
	strpf( s1, "addr history (%d/%d):", addr_hist_pos+1, vaddr_hist.size() );
	}
bx_addr_hist_label->copy_label( s1.c_str() );
return;
}







void graph_nulls()
{

for( int i = 0; i < 1000; i++ )
	{
	gph0_vamp0.push_back( 0 );
		
	gph0_vamp1.push_back( 0 );
	gph0_vamp2.push_back( 0 );
	}

update_gphs();
}







int main(int argc, char **argv)
{
string s1, fname, dir_sep;
bool add_ini = 1;									//assume need to add ini extension	

Fl::visual( FL_RGB );


Fl::scheme("plastic");								//optional

fname = cnsAppName;							//assume ini file will have same name as app
dir_sep = "";								//assume no path specified, so no / or \ (dos\windows)



//test if window with same name found and ask if still to run this -
// will return 1 if user presses 'Don't Run, Exit' button
if( CheckInstanceExists( cnsAppWndName ) ) return 0;

//linux code
#ifndef compile_for_windows
dir_seperator = "/";									//use unix folder directory seperator
#endif


dir_sep = dir_seperator;



//windows code
//attach a command line console, so printf works
#ifdef compile_for_windows
int hCrt;
FILE *hf;

AllocConsole();
hCrt = _open_osfhandle( (long) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
hf = _fdopen( hCrt, "w" );
*stdout = *hf;
setvbuf( stdout, NULL, _IONBF, 0 );
#endif


get_app_path( app_path );				//get app's path


//handle command line params
printf("\n\nti_lpc app\n\n");
    printf("~~~~~~~~~~~~");


printf("\nSpecify no switches to use config with app's filename in app's dir\n");
printf("\nSpecify '--cf ffff' to use config with filename 'ffff'\n");


if( argc == 1 )
	{
	printf( "-- Using app's dir for config storage.\n" );
	}


if( argc == 3 )
	{
	if( strcmp( argv[ 1 ], "--cf" ) == 0 )
		{
		printf( "-- Using specified filename for config storage.\n" );
		fname = argv[ 2 ];
		app_path = "";
		dir_sep = "";								//no directory seperator
		add_ini = 0;								//user spec fname don't add ini ext
		}
	}





if( app_path.length() == 0 )
	{
	csIniFilename = app_path + fname;					//make config file pathfilename
	}
else{
	csIniFilename = app_path + dir_sep + fname;			//make config file pathfilename
	}
if( add_ini ) csIniFilename += ".ini";                  //need ini if no user spcified fname





printf("\n\n-> Config pathfilename is:'%s', this will be used for config recall and saving.\n",csIniFilename.c_str());


wndMain = main_window();//new dble_wnd( 50, 50, 820, 750 );

add_graphs( wndMain );

//wndMain->label( cnsAppWndName );
wndMain->callback((Fl_Callback *)cb_wndmain, wndMain);

//menu bar
//meMain = new Fl_Menu_Bar(0, 0, wndMain->w(), 25);
meMain->textsize(12);
meMain->copy( menuitems, wndMain );



led_addr_step0 = new GCLed( bx_addr_step_led->x(), bx_addr_step_led->y(), 12, 12, "");
led_addr_step0->labelsize( 9 );	
led_addr_step0->align( FL_ALIGN_LEFT );
led_addr_step0->tooltip( "address auto stepping state\nred: stopped,\nyel: sounding,\ngreen: stepping address" );	
led_addr_step0->led_style = cn_gcled_style_square;
led_addr_step0->SetColIndex( 0, 255, 80, 80 );	
led_addr_step0->SetColIndex( 1, 255, 165, 80 );	
led_addr_step0->SetColIndex( 2, 80, 255, 80 );	
led_addr_step0->ChangeCol( 0 );

wndMain->add( led_addr_step0 );
bx_addr_step_led->hide();

//led_addr_step0->callback( cb_led0, 0 );



make_pref_wnd();
//make_pref2_wnd();
make_font_pref_wnd();


LoadSettings(csIniFilename); 


 
fi_addr_step_count->value("?");


b_use_jack =  pref_use_jack;

aud_pos = 0;
//aud_gain = 0.5;
mode = 1;
use_interp = 1;
ck_interp->value(1);

use_filter = 1;
ck_filter->value(1);









vsm = (uint8_t*) malloc( rom_size_max );
bufsamp0 = (float*) malloc( bufsamp_sz * sizeof(float) );
bufsamp1 = (float*) malloc( bufsamp_sz * sizeof(float) );


build_rom_word_addr_list( 0x0000, slast_rom0_filename );
build_rom_word_addr_list( 0x4000, slast_rom1_filename );


start_audio( wndMain );



wndMain->show(argc, argv);

graph_nulls();				//put somthing in graphs so on exit the traces will have their y-axis offset stored incase no sounding was made before exit
 
 
//extern void test_fast_mgraph();
//test_fast_mgraph();


//tms_test();
//stop_audio();
//exit(0);

//create_filters();

inited = 1;

//if( !load_vsm( vsm, "tmc0351n2l.vsm", rom_size_max ) ) return 1;
//if( !load_vsm( vsm, "tmc0352n2l.vsm", rom_size_max ) ) return 1;
//if( !load_vsm( vsm, "cd2319raw.bin", rom_size_max ) ) return 1;
//if( !load_vsm( vsm, "cd2319.bin", rom_size_max ) ) return 1;


talk.rev_rom = 1;
talk.tmc0280 = 1;


int last_addr = 0xf03;
s1 = fi_romaddr->value();
sscanf( s1.c_str(),"%x", &last_addr );

//say_lpc_str( fi_lpc_hex->value() );

Fl::add_timeout( cn_timer_period, cb_timer1 );		//update controls, post queued messages

int iAppRet=Fl::run();

return 0;
}





