/*
Copyright (C) 2019 BrerDawg

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

//audio_formats.h
//v1.05	---- 05-jun-2017
//v1.08

//!! see usage examples in 'audio_formats.cpp'
   
#ifndef audio_formats_h
#define audio_formats_h



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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
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



#include "GCProfile.h"
//#include "sdr_global.h"                 //this holds the flts type def, e.g: typedef double flts;



using namespace std;

#define cn_malloc_granularity 16 * 1024 * 1024			//used for malloc storage method (non vector based)
#define cn_malloc_max_size 256 * 1024 * 1024 + 1		//put a limit on how big realloc storage can grow (space for at least 33554432 doubles[assuming 8 bytes per double], @48KHz amounts to ~699 secs [11 mins:39 secs] )


#define cn_audio_formats_buf_siz 1024 * 1024 * 16      //make sure this is divisible by 16 so double prec(64 bit) stereo samples (128 bit total [16 bytes])
                                            //occupy a buffer read in whole units, ie: only whole stereo samples will fit in a single buffer fread(..)

typedef double flts;				//define numerical calc precision: float/double


enum en_audio_formats
{
en_af_unknown,					//unknown audio format
en_af_raw_pcm,                  //int 16 bit
en_af_raw_single_prec,          //float 32 bit floating point
en_af_raw_double_prec,          //double 64 bit floating point
en_af_sun,                      //encoding 1 of ulaw,   encoding 3 for 16 bit PCM
en_af_wav_pcm,					//only 16 bit PCM format supported
en_af_aiff,
};





//---------------------------------
//simple wave file RIFF format support (only PCM format supported at present)
#define cn_riff_hdr_ckid 0							    //+0 = 0
#define cn_riff_hdr_cksize 4							//+0 = 4
#define cn_riff_hdr_waveid 8							//+0 = 8


#define cn_riff_fmt_chunk_ckid 0						//+12 = 12
#define cn_riff_fmt_chunk_cksize 4						//+12 = 16
#define cn_riff_fmt_chunk_wformat_tag 8				    //+12 = 20
#define cn_riff_fmt_chunk_channels 10					//+12 = 22
#define cn_riff_fmt_chunk_samp_per_sec 12				//+12 = 24
#define cn_riff_fmt_chunk_avg_bytes_per_sec 16			//+12 = 28
#define cn_riff_fmt_chunk_block_align 20				//+12 = 32
#define cn_riff_fmt_chunk_bits_per_samp 22				//+12 = 34

//if 'cn_riff_fmt_chunk_cksize' has 16 stored in it the below defines are not used, the data chunk follows immediately
//#define cn_riff_fmt_chunk_cbsize 24					//+12 = 36
//#define cn_riff_fmt_chunk_valid_bits_per_samp 26		//+12 = 38
//#define cn_riff_fmt_chunk_chan_mask 28				//+12 = 42
//#define cn_riff_fmt_chunk_sub_format 32				//+12 = 44


#define cn_riff_data_chunk_ckid 0					    //+12 = 36
#define cn_riff_data_chunk_cksize 4				        //+12 = 40
//---------------------------------







//---------------------------------
//simple aiff file format support (only PCM format supported at present)
#define cn_aiff_form_ckid 0                     //4 FORM
#define cn_aiff_form_cksize 4                   //4
#define cn_aiff_aiff_ckid 8                     //4 AIFF
#define cn_aiff_comm_ckid 12                    //4 COMM
#define cn_aiff_comm_cksize 16                  //4
#define cn_aiff_comm_channels 20                //2
#define cn_aiff_comm_sample_frames 22           //4
#define cn_aiff_comm_sample_size 26             //2
#define cn_aiff_comm_sample_srate 28            //10 apple SANE, 80 bit IEEE Standard 754 floating point number, long double, big endian
#define cn_aiff_comm_ssnd_ckid 38               //4 SSND
#define cn_aiff_comm_ssnd_cksize 42             //4
#define cn_aiff_comm_ssnd_offset 46             //4  this is not used and is usually 0
#define cn_aiff_comm_ssnd_block_size 50         //4  this is not used and is usually 0
#define cn_aiff_comm_ssnd_sample_data 54        //4 this is where the data samples start, big endian pcm



/* aiff file hex dump, 48000Hz, 2 channel, 16 bit (file length: 1997774 bytes, big endian, holds 1997728 - 8 = 1997720 samples )

0000000000000000: 46 4f 52 4d 00 1e 7b c6 41 49 46 46 43 4f 4d 4d 	FORM..{.AIFFCOMM
0000000000000010: 00 00 00 12 00 02 00 07 9e e6 00 10 40 0e bb 80 	............@...
0000000000000020: 00 00 00 00 00 00 53 53 4e 44 00 1e 7b a0 00 00 	......SSND..{...
0000000000000030: 00 00 00 00 00 00 00 9c 00 00 00 a3 00 00 00 a1 	................
0000000000000040: 00 00 00 b2 00 00 00 ab 00 00 00 a0 00 00 00 89 	................
0000000000000050: 00 00 00 81 00 00 00 58 00 00 00 62 00 00 00 55 	.......X...b...U
0000000000000060: 00 00 00 31 00 00 00 2a 00 00 00 2b 00 00 00 26 	...1...*...+...&
0000000000000070: 00 00 00 1a 00 00 00 27 00 00 00 54 00 00 00 6b 	.......'...T...k
0000000000000080: 00 00 00 5d 00 00 00 5f 00 00 00 57 00 00 00 56 	...]..._...W...V
0000000000000090: 00 00 00 44 00 00 00 4a 00 00 00 49 00 00 00 24 	...D...J...I...$
00000000000000a0: 00 00 00 21 00 00 00 24 00 00 00 2c 00 00 00 27 	...!...$...,...'
00000000000000b0: 00 00 00 30 00 00 00 4a 00 00 00 5a 00 00 00 4d 	...0...J...Z...M
00000000000000c0: 00 00 00 40 00 00 00 44 00 00 00 44 00 00 00 54 	...@...D...D...T
00000000000000d0: 00 00 00 5b 00 00 00 5c 00 00 00 77 00 00 00 85 	...[...\...w....
00000000000000e0: 00 00 00 74 00 00 00 59 00 00 00 4d 00 00 00 59 	...t...Y...M...Y
00000000000000f0: 00 00 00 6a 00 00 00 72 00 00 00 71 00 00 00 82 	...j...r...q....
0000000000000100: 00 00 00 80 00 00 00 7f 00 00 00 77 00 00 00 64 	..........w...d
0000000000000110: 00 00 00 69 00 00 00 59 00 00 00 68 00 00 00 54 	...i...Y...h...T
0000000000000120: 00 00 00 36 00 00 00 2d 00 00 00 33 00 00 00 59 	...6...-...3...Y
0000000000000130: 00 00 00 5c 00 00 00 5c 00 00 00 51 00 00 00 53 	...\...\...Q...S
0000000000000140: 00 00 00 58 00 00 00 63 00 00 00 71 00 00 00 61 	...X...c...q...a
0000000000000150: 00 00 00 45 00 00 00 46 00 00 00 45 00 00 00 22 	...E...F...E..."
0000000000000160: 00 00 00 1a 00 00 00 1a 00 00 00 4e 00 00 00 6a 	...........N...j
0000000000000170: 00 00 00 67 00 00 00 6e 00 00 00 68 00 00 00 69 	...g...n...h...i
0000000000000180: 00 00 00 54 00 00 00 4e 00 00 00 54 00 00 00 54 	...T...N...T...T
0000000000000190: 00 00 00 56 00 00 00 53 00 00 00 43 00 00 00 3c 	...V...S...C...<
00000000000001a0: 00 00 00 51 00 00 00 78 00 00 00 90 00 00 00 7c 	...Q...x.......|
00000000000001b0: 00 00 00 52 00 00 00 54 00 00 00 58 00 00 00 3d 	...R...T...X...=
00000000000001c0: 00 00 00 3a 00 00 00 42 00 00 00 88 00 00 00 9d 	...:...B........
00000000000001d0: 00 00 00 87 00 00 00 84 00 00 00 75 00 00 00 7f 	...........u...
00000000000001e0: 00 00 00 6d 00 00 00 71 00 00 00 69 00 00 00 65 	...m...q...i...e
00000000000001f0: 00 00 00 61 00 00 00 64 00 00 00 86 00 00 00 88 	...a...d........
0000000000000200: 00 00 00 84 00 00 00 82 00 00 00 81 00 00 00 73	...............s

*/
//---------------------------------


















//holds one audio file's data    !! see usage examples in 'audio_formats.cpp'
struct st_audio_formats_tag
{
string path;
string fname;

en_audio_formats format;
 
unsigned int srate;
unsigned int encoding;                               //for sav_sun(), load_sun(): encoding 1 = 8 bit u-law, 1 or 2 channel,  encoding 3 = 16 bit pcm, 1 or 2 channel
unsigned int channels;
unsigned int offset;

bool is_big_endian;
bool bits_per_sample;

vector<double> vch0;
vector<double> vch1;


};



class audio_formats
{
private:
int req_scale_down_by_peak_int_val;
int req_peak_int_val;

en_audio_formats fp_audio_format;					//used with fp based methods
int fp_srate;
int fp_channels;
int fp_bytes_written;
int fp_offset;
int fp_encoding;
int fp_is_big_endian;

public:
bool verb;
string log;                                         //holds a cumulative number of string showing progress and any errors
bool log_to_printf;                                 //if to issue a printf as well
string sid;											//a string name to help debug objs

unsigned int allocsz_ch0, allocsz_ch1;				//used for malloc methods
unsigned int sizech0, sizech1;
double *pch0, *pch1;

FILE *fileptr;

string pathname;									//used with fp based methods
uint64_t file_size;									
unsigned int buf_size;
unsigned char *ucbf;
double *bf0;										
double *bf1;
float *f_bf0;										
float *f_bf1;

public:
audio_formats();
~audio_formats();

int load( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );


bool save( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
bool save_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
bool sample_buf_load_flts( st_audio_formats_tag &af, flts* fltsbuf, unsigned int sample_cnt_to_load, unsigned int &actually_loaded );
bool sample_buf_load_flts_malloc( st_audio_formats_tag &af, flts* fltsbuf, unsigned int sample_cnt_to_load, unsigned int &actually_loaded );
bool make_tone( double amp0, double freq0, double phase0, double amp1, double freq1, double phase1, double dur, st_audio_formats_tag &af );
bool srate_change( st_audio_formats_tag &afout );
bool normalise( double max_val, st_audio_formats_tag &af );
bool find_file_format( string path, string fname, en_audio_formats &fmt );

bool push_ch0( double dd );
bool push_ch1( double dd );
bool clear_ch0();
bool clear_ch1();
bool copypush_ch0( double *pch, unsigned int count );
bool copypush_ch1( double *pch, unsigned int count );


bool fir_filter( int which_chan, vector<double> &vcoeff );

int fp_open_double( string path, string fname, unsigned int bfsz, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int fp_read_double( unsigned int count, st_audio_formats_tag &af );

int fp_create_double( string path, string fname, unsigned int bfsz, int peak_int_val, st_audio_formats_tag &af );
int fp_write_double( unsigned int count, st_audio_formats_tag &af );

int fp_open_float( string path, string fname, unsigned int bfsz, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int fp_read_float( unsigned int count, st_audio_formats_tag &af );

int fp_create_float( string path, string fname, unsigned int bfsz, int peak_int_val, st_audio_formats_tag &af );
int fp_write_float( unsigned int count, st_audio_formats_tag &af );


private:
int load_raw_pcm( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_raw_pcm_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_raw_single_prec( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_raw_single_prec_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_raw_double_prec( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_raw_double_prec_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
bool save_raw( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
bool save_raw_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
int load_sun( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_sun_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
bool save_sun( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
bool save_sun_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
int load_wav( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_wav_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
bool save_wav( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
bool save_wav_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
int load_aiff( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
int load_aiff_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af );
bool save_aiff( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
bool save_aiff_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af );
bool find_in_file( FILE *fp, string fname, unsigned int &found_at_pos );



unsigned char linear2ulaw( int sample );
int ulaw2linear( unsigned char ulawbyte );
void logpf( const char *fmt,... );

void fp_write_close_free_mem();

};




#endif

