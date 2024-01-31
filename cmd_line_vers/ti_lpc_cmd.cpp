/*
Copyright (C) 2024 BrerDawg

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

//ti_lpc_cmd.cpp

//v1.01		31-jan-2024			//



#include "ti_lpc_cmd.h"

int exit_code;

float srate_lpc = 8000;
float srate_wav = 48000;
float lpc_srate = srate_lpc;											//srate the rom was encoded with
float srate = srate_wav;
float srate_au = srate_wav;
float au_aud_gain = 75.0f;
int srate_user = srate_wav;




string slast_rom0_filename = "tmc0351n2l.vsm";
string slast_rom1_filename = "tmc0352n2l.vsm";
string fname_au = "zzzout.wav";
string fname_tms_chip = "tms5100.txt";
string fnamein0 = "zzzfile_in.txt";										//user specified filename for input, e.g: for c-code encoded string needing to be cleaned up
string fnameout0 = "zzzaddr_list.txt";									//user specified filename for output, e.g: for rom list dump
string fnamelineindex = "zzzline_index.txt";
string fname_last_utter_str = "zzzlast_utter_strhex.txt";			//holds the last hex byte string that was sounded

audio_formats af;
audio_formats af0;
audio_formats af1;
audio_formats af2;
st_audio_formats_tag saf;
st_audio_formats_tag saf0;
st_audio_formats_tag saf1;

uint8_t *vsm = 0;											//holds a voice synth memory rom 

Talkie talk;

bool bperiod_6bits = 0;
string slpc_bytes;
int bufcnt = 0;
int generated_samp_cnt = 0;
int frame_cnt = 0;
uint8_t periodCounter;
float x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10;
float chirp_step;
unsigned int last_say_offset = 0;
float *bufsamp0 = 0;
float *bufsamp1 = 0;
int bufsamp_sz = cn_bufsz;
int stop_lpc_addr = 0;
static uint16_t synthRand = 1;
bool use_interp = 1;
float frame_time = 25e-3;
int temp_smple_cnt = 0;
bool use_filter = 1;
float time_cur = 0;
int last_render_rom_byte_cnt = 0;
int lpccnt = 0;
int phrase_cnt = 0;
en_mode_user_tag mode_user = en_mu_none;
int mode = 0;
int snd_end_addr;

uint64_t aud_pos = 0;

bool verbose = 0;


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

uint8_t chirp_0280[ CHIRP_SIZE_0280 + 2] = { 0, 42, 212, 50, 178, 18, 37, 20, 2, 225, 197, 2, 95, 90, 5, 15, 38, 252, 165, 165, 214, 221, 220, 252, 37, 43, 34, 33, 15, 255, 248, 238, 237, 239, 247, 246, 250, 0, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int ichirp_size = CHIRP_SIZE_0280;								//this will change id user modifiesthe chirp byte string in text edit










//function prototypes
double qdss_resample( double x, double *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth );
float qdss_resample_float( float x, float *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth );
int str_to_decimal_buf( string ss, int16_t *bf, int bfsz );
int str_to_hex_buf( string ss, uint16_t *bf, int bfsz );












bool open_audio_file( string sfname )
{
af0.verb = 1;                                	//store progress in: string 'log'
af0.log_to_printf = 1;                       	//show to console as well


if( !af0.find_file_format( "", sfname, saf0.format ) )
	{
	printf("open_audio_file() - failed to detct audio file format: '%s'\n", sfname.c_str());
	return 0;
	}

if( !af0.load_malloc( "", sfname, 0, saf0 ) )
	{
	printf("open_audio_file() - failed af.load() '%s'\n", sfname.c_str());
	return 0;
	}

float srate = saf0.srate;


return 1;
}










//--- converted from Basic language code ----
// refer: http://www.nicholson.com/rhn/dsp.html
//
// rem:  use 'baudline' to assess quality

// this function can be used in sample rate conversion code,
// it generates one intermediate sample from a supplied collection of original samples, it calcs this sample
// using a windowed sinc that implements a lowpass fir. The position of the new sample point is defined by the 'x'
// value. If 'x' is 50.5 then the fir is centered at that location and the original samples either side of this (see 'wnwdth')
// are convolved with fir to produce the new sample value.

// x - set 'x' to the position of the sample to generate, the repeated calling of this function and the amount of 'x' stepping between
// calls will determine the resultant sample rate.
// if 'x' stepping is: 2, you will be up-converting source srate by 2,   (e.g: 48KHz->96KHz, use fmax around 24KHz)  
// if 'x' stepping is: 0.25, you will be down-converting source srate by 4,	(e.g: 48KHz->12KHz, use fmax around 5.5KHz)

// fsrate - is the sample rate of the source signal.
// fmax - is cutoff freq of interpolation filter (the 'on the fly' calc'd windowed sinc filter), set this to less than half the desired srate,
//        so if down-converting 48KHz to 24KHz, set fmax to say 11KHz, a safety margin is req, and is related to how big the 'wnwdth' has been set to,
//		  if up-converting 11KHz to 24KHz, set fmax to say 12KHz, a safety margin here is not so important as the original signal would
//		  only contain freqs to 12KHz in anycase. 
//		  
 
// wnwdth - is number of calc'd coeffs(taps) to use for the fir interpolation filter, bigger 'wnwdth' gives a quicker cutoff transition and
//			better antialias lowpass filtering, but it increases execution times, try: 32, 64, 200(there is no need for it to be a factor of two)

// buf - 	is a ptr to an buf holding source samples, best results are obtained if the samples are either side of
//			the sample to generate, and that there are enough samples to match the fir coeff count spec with: wnwdth
// bufsz -	size of the buf

//
// Ron Nicholson's QDSS ReSampler cookbook recipe
// QDSS = Quick, Dirty, Simple and Short
// Version 0.1b - 2007-Aug-01
// Copyright 2007 Ronald H. Nicholson Jr.
// No warranties implied.  Error checking, optimization, and
// quality assessment of the "results" is left as an exercise
// for the student.
// (consider this code Open Source under a BSD style license)
// IMHO. YMMV.  http://www.nicholson.com/rhn/dsp.html

// QDSS Windowed-Sinc ReSampling subroutine in Basic
// This function can also be used for interpolation of FFT results

// function parameters
// 		x		= 	new sample point location (relative to old indexes)
//             		(e.g. every other integer for 0.5x decimation)
//  	buf	= original data array
//  	bufsz	= size of data array
//  	fmax	= low pass filter cutoff frequency
//  	fsrate		= sample rate
//  	wnwdth	= width of windowed Sinc used as the low pass filter

//  	resamp() returns a filtered new sample point

// Notes:

// fmax should be less than half of fsrate (src srate), and less than half of new_fsrate (the reciprocal of the x step size).
// Filter quality increases with a larger window width. The wider the window, the closer fmax can approach half of fsr or new_fsrate.
// Several operations inside the FOR loop can be pre-calculated.
// There are more optimal windows than the von Hann window.
// If the x step size is rational the same Window and Sinc values will be recalculated repeatedly.
// Therefore these values can either be cached, or pre-calculated and stored in a table (polyphase interpolation); or
// interpolated from a smaller pre-calculated table; or computed from a set of low-order polynomials fitted to each section or
// lobe between zero-crossings of the windowed Sinc (Farrow). (Performance optimization is left as an exercise for the student). 




double qdss_resample( double x, double *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth )
{
double r_g,r_w,r_a,r_snc,r_y;												//some local variables
int i, j;

if( wnwdth <= 0 ) return 0;
if( bufsz < ( wnwdth / 2 ) ) return 0;

r_g = 2.0 * fmax / fsrate;													//calc gain correction factor

r_y = 0;
for ( i = -( wnwdth / 2 ); i <= ( wnwdth / 2 - 1 ); i++ ) 					//for 1 window width
	{
    j = (int) x + i;          												//calc input sample index

    r_w = 0.5 - 0.5 * cos( twopi * ( 0.5 + ( j - x ) / wnwdth ) );			//make a von hann sample, will be used taper sinc's length

    r_a = twopi * ( j - x ) * fmax / fsrate;								//curr sinc location
    r_snc = 1;
	if ( r_a != 0 ) r_snc = sin( r_a ) / r_a;								//make a sinc (sin x/x) lpf sample

//printf("i: %d, r_w: %f, r_a: %f, r_snc: %f\n", i, r_w, r_a, r_snc );

    if ( ( j >= 0 ) & ( j < bufsz ) )										//src sample avail?
		{
		r_y = r_y + r_g * r_w * r_snc * buf[ j ];							//convolve/mac: first: apply von hann tapering to sinc, then both these to a src sample (adj gain as well)
		}
 	}
return r_y;                  												//new filtered intermediate sample

}

//------------------









float qdss_resample_float( float x, float *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth )
{
double r_g,r_w,r_a,r_snc,r_y;												//some local variables
int i, j;

if( wnwdth <= 0 ) return 0;
if( bufsz < ( wnwdth / 2 ) ) return 0;

r_g = 2.0 * fmax / fsrate;													//calc gain correction factor

r_y = 0.0f;
for ( i = -( wnwdth / 2 ); i <= ( wnwdth / 2 - 1 ); i++ ) 					//for 1 window width
	{
    j = (int) x + i;          												//calc input sample index

    r_w = 0.5 - 0.5 * cos( twopi * ( 0.5 + ( j - x ) / wnwdth ) );			//make a von hann sample, will be used taper sinc's length

    r_a = twopi * ( j - x ) * fmax / fsrate;								//curr sinc location
    r_snc = 1;
	if ( r_a != 0 ) r_snc = sin( r_a ) / r_a;								//make a sinc (sin x/x) lpf sample

//printf("i: %d, r_w: %f, r_a: %f, r_snc: %f\n", i, r_w, r_a, r_snc );

    if ( ( j >= 0 ) & ( j < bufsz ) )										//src sample avail?
		{
		r_y = r_y + r_g * r_w * r_snc * buf[ j ];							//convolve/mac: first: apply von hann tapering to sinc, then both these to a src sample (adj gain as well)
		}
 	}
return r_y;                  												//new filtered intermediate sample

}






















float theta0 = 0;
float theta1 = 0;






void test_tone()
{
af0.verb = 1;                                	//store progress in: string 'log'
af0.log_to_printf = 1;                       	//show to console as well


saf0.srate = srate_lpc;
saf0.format = en_af_wav_pcm;
saf0.encoding = 0;
saf0.channels = 2;
saf0.offset = 0;
saf0.is_big_endian = 0;
saf0.bits_per_sample = 16;


float freq0 = 200.0f;
float freq1 = 400.0f;

float time_per_samp = 1.0f/srate_lpc;

float theta_inc0 = freq0 * twopi * time_per_samp;
float theta_inc1 = freq1 * twopi * time_per_samp;

float dur = 2;

int smpl_cnt = dur * srate_lpc;

for( int i = 0; i < smpl_cnt; i++ )
	{

	float amp0 = 0.5f;
	


	float f0 = amp0 * sinf( theta0 );
	float f1 = amp0 * sinf( theta1 );


	theta0 += theta_inc0;
	if( theta0 >= twopi ) theta0 -= twopi;
	
	theta1 += theta_inc1;
	if( theta1 >= twopi ) theta1 -= twopi;

	af0.push_ch0( f0 );
	af0.push_ch1( f0 );
	}

/*
string path = "";
string fname = "zztone.wav";
if( af0.save_malloc( path, fname, 32767, saf0 ) )
	{
	printf( "saved wav file (%d samples/chan): '%s'\n", smpl_cnt, fname.c_str() );	
	}
else{
	printf( "failed to save wav file: '%s'\n", fname.c_str() );	
	}
*/



int upsample_ratio = srate_wav / srate_lpc;

float step = 1.0f/upsample_ratio;
float pos = 0;
for( int i = 0; i < af0.sizech0*upsample_ratio; i++ )
	{
	
	float f0 = qdss_resample( pos, af0.pch0, af0.sizech0, srate_lpc/2, srate_lpc, 32 );
	
	af1.push_ch0( f0 );
	af1.push_ch1( f0 );

	pos += step;
	}
	
//printf( "pos %f\n", pos );

saf1.srate = srate_wav;
saf1.format = en_af_wav_pcm;
saf1.encoding = 0;
saf1.channels = 2;
saf1.offset = 0;
saf1.is_big_endian = 0;
saf1.bits_per_sample = 16;

string path = "";
string fname = "zztone.wav";
if( af1.save_malloc( path, fname, 32767, saf1 ) )
	{
	printf( "saved wav file (%d samples/chan): '%s'\n", af1.sizech0, fname.c_str() );	
	}
else{
	printf( "failed to save wav file: '%s'\n", fname.c_str() );	
	}

}
































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








bool load_chip_params( string fname_in )
{
string s1;
mystr m1;

int16_t bf[ 256 ];
uint16_t uibf[ 256 ];


//string ss = tb_chirp->text();											//ti_lpc_cmd mod

//m1 = ss;																//ti_lpc_cmd mod

if( !m1.readfile( fname_in, 100000 ) )							//ti_lpc_cmd mod
	{
	printf("load_chip_params() - failed to read '%s'\n", fname_in.c_str() );
	return 0;	
	}

bool vb = verbose;

//dec_to_hex();


//----
if( m1.ExtractParamVal_with_delimit( "chirp=", "\n", s1 ) )
	{
	if(vb)printf("chirp= '%s'\n", s1.c_str() );
	
	int cnt = str_to_decimal_buf( s1, bf, 256 );

	if( cnt > CHIRP_SIZE_0280 )
		{
		if(vb)printf("load_chip_params() - too many vals, max is %d, chirp vals found: %d\n", CHIRP_SIZE_0280, cnt );
		cnt = CHIRP_SIZE_0280;		
		}

	if(vb)printf("load_chip_params() - chirp vals fetched: %d\n", cnt );
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
		if(vb)printf("load_chip_params() - too many vals, max is %d, chirp vals found: %d\n", CHIRP_SIZE_0280, cnt );
		cnt = CHIRP_SIZE_0280;		
		}

	if(vb)printf("load_chip_params() - chirp vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - energy vals fetched: %d\n", cnt );

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

	if(vb)printf("load_chip_params() - energy vals fetched: %d\n", cnt );

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
			//ck_pitch_6bits->value(1);									//ti_lpc_cmd mod
			}
		else{
			bperiod_6bits = 0;
			//ck_pitch_6bits->value(0);									//ti_lpc_cmd mod
			}

		if(vb)printf("pitch_count= %d\n", ipitch_count );
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

	if(vb)printf("load_chip_params() - pitch vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - pitch vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k0 vals fetched: %d\n", cnt );

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

	if(vb)printf("load_chip_params() - k0 vals fetched: %d\n", cnt );
	
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

	if(vb)printf("load_chip_params() - k1 vals fetched: %d\n", cnt );
	
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

	if(vb)printf("load_chip_params() - k1 vals fetched: %d\n", cnt );

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

	if(vb)printf("load_chip_params() - k2 vals fetched: %d\n", cnt );

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

	if(vb)printf("load_chip_params() - k2 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k3 vals fetched: %d\n", cnt );

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

	if(vb)printf("load_chip_params() - k3 vals fetched: %d\n", cnt );

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

	if(vb)printf("load_chip_params() - k4 vals fetched: %d\n", cnt );

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

	if(vb)printf("load_chip_params() - k4 vals fetched: %d\n", cnt );

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

	if(vb)printf("load_chip_params() - k5 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k5 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k6 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k6 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k7 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k7 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k8 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k8 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k9 vals fetched: %d\n", cnt );
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

	if(vb)printf("load_chip_params() - k8 vals fetched: %d\n", cnt );
	for( int i = 0; i < cnt; i++ )							//store bytes
		{
		tms_k9_0280[i] = uibf[i];
		}
	if(vb)printf("cnt: %d, tms_k9_0280[]= %02x %02x\n", cnt, tms_k9_0280[0],tms_k9_0280[1] );
		
	}
//----
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



























int last_adress = 0;

//portions of this code originate from Talkie prg on github
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

//mode = 0;																//ti_lpc_cmd mod
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





//load_chip_params();														//ti_lpc_cmd mod	


// 


bool vb = verbose;
static uint8_t nextPwm;
float u0,u1,u2,u3,u4,u5,u6,u7,u8,u9,u10;
float yy;

float inp[ 8000 ];
float koef[ 10 ];
float ycalc[ 10 ];
float gstate[ 10 + 1 ];
float outp[ 8000 ];


//if( !inited ) return;													//ti_lpc_cmd mod	

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

//dbg_dump = 0;															//ti_lpc_cmd mod	


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

bool plot_first_time_flag = 1;			//initial plot call flag

//extern vector<float> gph0_vx;
vector<float> fgph_vx;
vector<float> fgph_vy0;
vector<float> fgph_vy1;
vector<float> fgph_vy2;

//gph0_vx.clear();														//ti_lpc_cmd mod
//gph0_vamp0.clear();
//gph0_vamp1.clear();
//gph0_vamp2.clear();
//gph0_vamp3.clear();
//gph0_vamp4.clear();

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
if(vb)printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!energy_idx == 0xf\n");
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

	if(vb)printf("frame_cnt[%03d]: engy_idx: %d, engy: %d, period: %d, rpt: %d, ending_cnt: %d\n", frame_cnt, energy_idx, tgt_energy, tgt_period, repeat, ending_cnt );

	if(!vb)printf("frame_cnt[%03d]\r", frame_cnt);
	
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

//		gph0_vx.push_back( smpl_cnt * (1.0 / lpc_srate) );				//make x-axis vals			//ti_lpc_cmd mod
		fgph_vx.push_back( smpl_cnt * (1.0 / lpc_srate) );				//make x-axis vals
//		gph0_vamp0.push_back( sin1 );
//		gph0_vamp1.push_back( ii / 32768.0 );
//		gph0_vamp4.push_back( new_frame );								//ti_lpc_cmd mod
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
//  		gph0_vamp0.push_back( u0 );										//ti_lpc_cmd mod
//		printf("u0 %f  raw_stimul %f  new_frame %f\n", u0, raw_stimul, new_frame / 10.0 );
		
//  		gph0_vamp1.push_back( raw_stimul );								//ti_lpc_cmd mod
//		gph0_vamp1.push_back( sin1 * 0.25 + 0.1 );
//  		gph0_vamp2.push_back( new_frame / 10.0 );						//ti_lpc_cmd mod
		
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
		if(vb)printf("ending_cnt: %d\n", ending_cnt);
		if( ending_cnt == 0 ) break;
		}
	}

printf( "render tot_frames: %d, tot_time: %f, 8KHz smple cnt: %d\n", frame_cnt, time_cur, temp_smple_cnt );

//fi_lpc_hex->value( slpc_bytes.c_str() );								//ti_lpc_cmd mod
if(vb)printf("Talkie::say_tmc0580() - slpc_bytes:\n'%s'\n", slpc_bytes.c_str() );

m1 = slpc_bytes;
m1.writefile( fname_last_utter_str );


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

int sr_out_sample_cont = (ptr_8KHz * (1.0/srconv_ratio));

printf("sample rate conversion 8KHz --> %d, sample cnt: %d, nyquist: %f\n", (int)srate, sr_out_sample_cont, nyquist );

//samples for pc audio
for( int i = 0; i < sr_out_sample_cont; i++ )
	{
//usage --->		//float qdss_resample_float( float x, float *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth )
//	float srconv_val = gc_srateconv_code::qdss_resample_float( cur_sample, fbuf1, ptr_8KHz, nyquist, srate, fir_wndw );				//ti_lpc_cmd mod
	float srconv_val = qdss_resample_float( cur_sample, fbuf1, ptr_8KHz, nyquist, srate, fir_wndw );								//ti_lpc_cmd mod


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

printf("sample rate conversion .au file: 8KHz --> %d, samples: %f, nyquist: %f\n", (int)srate_au, (ptr_8KHz * (1.0/srconv_ratio)), nyquist );

	//samples for .au audio file
	for( int i = 0; i < (ptr_8KHz * (1.0/srconv_ratio)); i++ )
		{
//usage --->		//float qdss_resample_float( float x, float *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth )
//		float srconv_val = gc_srateconv_code::qdss_resample_float( cur_sample, fbuf1, ptr_8KHz, nyquist, srate_au, fir_wndw );			//ti_lpc_cmd mod
		float srconv_val = qdss_resample_float( cur_sample, fbuf1, ptr_8KHz, nyquist, srate_au, fir_wndw );								//ti_lpc_cmd mod

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

//s1 = fi_au_fname->value();											//ti_lpc_cmd mod
s1 = fname_au;															//ti_lpc_cmd mod
af2.save_malloc( "", s1, 32767, saf );


//update_gphs();														//ti_lpc_cmd mod

/*																		//ti_lpc_cmd mod
if( plot_first_time_flag ) 
	{
//	fgph.scale_y( 1.0, 1.0, 1.0, 0.1 );
//	fgph.shift_y( 0.0, -0.1, -0.10, -0.2 );
	fgph.font_size( 9 );
	fgph.set_sig_dig( 2 );
	fgph.sample_rect_hints_distancex = 0;
	fgph.sample_rect_hints_distancey = 0;
//	fgph.yunits_perpxl[0] = 0.02;
//	fgph.max_defl_y[ 0 ] = 2.5;

	fgph.set_trc_label1( 0, "pc srate" );
	plot_first_time_flag = 0;
	}
*/

//fgph.plotxy( gph0_vx, gph0_vamp0, gph0_vamp1, gph0_vamp2, gph0_vamp4,"brwn", "drkg", "drkcy", "drkr", "ofw", "drkb", "drkgry", "trace 1", "trace 2", "trace 3", "trace 4"  );
//fgph.plotxy( gph0_vx, gph0_vamp0, gph0_vamp1 );
//fgph.plotxy( fgph_vx, fgph_vy0, fgph_vy1 );

//fgph.plotxy( fgph_vx, fgph_vy0, "drky", "ofw", "drkb", "blk", "pc srate" );	//3 traces cols, bkgd col, axis col, text col, and trace colour coded labels, see defined colours: 'user_col'


//            fgph.plotxy_vfloat_1( fgph_vx, fgph_vy0 );	//3 traces cols, bkgd col, axis col, text col, and trace colour coded labels, see defined colours: 'user_col'     //ti_lpc_cmd mod

//fgph.plotxy( gph0_vx, gph0_vamp2 );
//fgph.plot( 0,gph0_vamp2 );

aud_pos = 0;												//start audio from beginning

if(show_lpc) printf("\n");



stop_lpc_addr = (int)ptrAddr;
last_render_rom_byte_cnt = lpccnt;//stop_lpc_addr - start_lpc_addr + 1;
if(vb)printf("stop  lpc[%03d]: %02x  lpccnt: %d, (addr delta: %d)\n", last_render_rom_byte_cnt, *ptrAddr, lpccnt, last_render_rom_byte_cnt );		



phrase_cnt++;

saf.encoding = 3;
saf.offset = 0;
saf.format = en_af_sun;
saf.channels = 2;
saf.srate = lpc_srate;

//af.save_malloc( "", "zz_cummulative.au", 32767, saf );				//ti_lpc_cmd mod

mode = 1;
aud_pos = 0;

strpf( s1, "%04x", offset + last_render_rom_byte_cnt );

snd_end_addr = offset + last_render_rom_byte_cnt;						//ti_lpc_cmd mod

//fi_addr_snd_end->value( s1.c_str() );									//ti_lpc_cmd mod
}





//
//convert comma delimited hex string and store in bf[]
//returns bytes stored
//e.g: "01, af, ff, 0x01,0x80,0x7f, 7f, 0x01, 7fff, 8000"
int str_to_hex_buf( string ss, uint16_t *bf, int bfsz )
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



























void say_lpc_bytes( uint8_t *bf, int cnt )
{
if( cnt != 0 )
	{
	talk.say_tmc0580( bf, 0 );
	}
}






//comma delimited
void say_lpc_str( string ss, bool is_dec )
{
bool vb = verbose;
string s1;
mystr m1;



m1 = ss;
m1.FindReplace( s1, "0x", "", 0);

m1 = ss;
m1.FindReplace( s1, "\r", "", 0);

m1 = s1;	
m1.FindReplace( s1, "\n", "", 0);

m1 = s1;
m1.FindReplace( s1, " ", "", 0);

ss = s1;

if( ss.find( ":" ) != string::npos )			//comment prefixed?
	{
	m1 = ss;
	m1.cut_just_past_first_find_and_keep_right( s1, ":", 0 );
	}

uint8_t bf[8192];


m1 = s1;



int cnt = 0;

if( is_dec )
	{
	cnt = m1.LoadArray_decimal_uint8_t( bf, 8192, ',' );
	}
else{
	cnt = m1.LoadArray_hex_uint8_t( bf, 8192, ',' );
	}



if(vb)
	{
	printf( "say_lpc_str() - processed str (obtained %d bytes):\n", cnt );
	for( int i = 0; i < cnt; i++ )
		{
		if( is_dec ) printf( "%d,", bf[i] );
		else printf( "%02x,", bf[i] );
		}	
	printf( "\n" );
	}



if( cnt != 0 )
	{
	talk.say_tmc0580( bf, 0 );
	if(vb)printf("say_lpc_str() - got: %d\n", cnt );
	}

}














































bool load_vsm( uint8_t *vsmem, string fname, int count )
{
bool vb = verbose;

FILE *fp = fopen( fname.c_str(), "rb" );

if( fp == 0 )
	{
	if(vb)printf("load_vsm() - failed to open file: '%s'\n", fname.c_str() );
	return 0;
	}

int read = fread( vsmem, 1, count, fp );

if( read != count )
	{
	if(vb)printf("load_vsm() - read: %d bytes, requested: %d, file: '%s'\n", read, count, fname.c_str() );
	}

if(vb)printf("load_vsm() - read: %d bytes, file: '%s'\n", read, fname.c_str() );

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



















//get a rom address ptr for spec offs
uint16_t get_ptr( uint8_t *rom_addr, int offs )
{
	
uint16_t ui = vsm[ offs ];									//get lpc's data addr
ui |= vsm[ offs + 1 ] << 8;
return ui;
}






//make list: of rom ptr addresses paired with their alpha/phrase/word
bool build_rom_word_addr_list( int offs, string fname, string &slist )
{
bool vb = verbose;
bool bdash = 0;

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

if(bdash) st += "-----------------\n";


ui = vsm[ pp ];									//get lpc's data addr
ui |= vsm[ pp + 1 ] << 8;
pp += 2;

strpf( s1, "%04x beep\n", ui );
st += s1;

if(bdash) st += "-----------------\n";

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

if(bdash) st += "-----------------\n";


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


if(bdash) st += "-----------------\n";
//tb_romaddr->text( st.c_str() );



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
//if(vb)printf("word list %d, text_addr: 0x%04x, lpc_addr: %s  text: %s\n", k, text_addr, s2.c_str(), s1.c_str() );
		}


if(bdash) 	st += "-----------------\n";
	}



if(vb)printf("word list:\n");
if(vb)printf( "%s", st.c_str() );

slist = st;
return 1;
}








bool cleanup_ccode_snipet_file( string fname_in, string &sout, char enclosing_left, char enclosing_right )
{
bool vb = verbose;

string s1, s2, st;
mystr m1;



if( !m1.readfile( fname_in, 100000 ) )
	{
	if(vb)printf("cleanup_ccode_snipet_file() - failed to read '%s'\n", fname_in.c_str() );
	return 0;	
	}



vector<string> vstr;
m1.LoadVectorStrings( vstr, '\n' );

for( int i = 0; i < vstr.size(); i++ )
	{
	
	m1 = vstr[i];
	
	s2 = enclosing_left;
	if( !m1.cut_at_end_of_first_find_and_keep_right( s1, s2, 0 ) ) continue;

	m1 = s1;

	s2 = enclosing_right;
	m1.cut_at_first_find( s1, s2, 0 );
	
	m1 = s1;
	m1.FindReplace( s1, "\r", "", 0);

	m1 = s1;
	m1.FindReplace( s1, "0x", "", 0);

	m1 = s1;
	m1.FindReplace( s1, " ", "", 0);

	if( s1.length() != 0 )
		{
		st += s1;
	
		st += "\n";
		if(vb)printf("cleanup_ccode_snipet_file() - %03d:'%s'\n", i, s1.c_str() );
		}
	}

sout = st;
return 1;
}








bool get_line_at_index( string fname_in, unsigned int line_idx, string &sout )
{
bool vb = verbose;
string s1, s2, st;
mystr m1;


if( !m1.readfile( fname_in, 100000 ) )
	{
	if(vb)printf("get_line_at_index() - failed to read '%s'\n", fname_in.c_str() );
	return 0;	
	}


vector<string> vstr;
m1.LoadVectorStrings( vstr, '\n' );

if( line_idx >= vstr.size() ) 
	{
	if(vb)printf("get_line_at_index() - line_idx %d is beyond last line %d\n", line_idx, vstr.size() );
	return 0;
	}

sout = vstr[ line_idx ];

return 1;
}





void show_usage()
{
printf("Texas Instruments LPC Voice Synthesizer Decoder (e.g: Speak and Spell) v1.01\n");
printf("\n");
printf("Minimal parameter checks are performed, params are delimited by spaces, therefore, do not use spaces in filenames or strings on the command line.\n");
printf("\n");
printf("Anything before a colon is ignored when strings are rendered, this allows a convenient name to be at head of each string.\n");
printf("\n");
printf("Some commands use default values if none are specified.\n");
printf("\n");
printf("Pick the chip def file which sounds best.\n");
printf("\n");
printf("A hex string file is generated after render of audio, named: 'zzzlast_utter_strhex.txt'\n");
printf("\n");
printf("Ubuntu users could add this to end of command to hear audio: && aplay zzzout.wav\n");
printf("\n");
printf("Invocation Examples:\n");
printf("\n");
printf("ti_lpc_cmd -help\n");
printf("ti_lpc_cmd --help\n");
printf("\n");
printf("ti_lpc_cmd mode=romlist rom0=tmc0351n2l.vsm rom1=tmc0352n2l.vsm fnameout=zzzaddr_list.txt			//dump list of words found in rom to text file (default fname is zzzaddr_list.txt), this list can be wrong for many roms as their organisation is unknown\n");
printf("																									//dump formatted as  'address' 'word'   like so:            6a8d isle\n");
printf("																									//see 'mode=rendaddrfileseq' option for sequential playing of this file\n");
printf("\n");
printf("ti_lpc_cmd mode=romlist rom0=tmc0351n2l.vsm rom1=tmc0352n2l.vsm fnameout=zzzaddr_list.txt verb=on 	//with verbose enabled for detailed output\n");
printf("\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5100.txt rom0=tmc0351n2l.vsm rom1=tmc0352n2l.vsm wav=zzzout.wav gain=75 addr=12a1	 	//render using chip def t'ms5100.txt' and rom hex address 12a1 to audio file with gain of 75% (gain default is 75, and limits are between 0-->300)\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5100.txt rom0=tmc0351n2l.vsm rom1=tmc0352n2l.vsm addr=1600	 				//render using rom hex address 1600 to default audio file named 'zzzout.wav'\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5100.txt rom0=tmc0351n2l.vsm rom1=tmc0352n2l.vsm srate=8000 addr=1198	 	//render and rom hex address 1198 to audio file with samplerate of 8KHz (srate default is 48KHz, limits are between 4000-->192000)\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5200.txt wav=zzzout.wav str=45,36,126,14,3,255...7,40 						//render decimal string to audio file (no roms req, strings must not have spaces)\n");
printf("ti_lpc_cmd mode=render chip=tms5100.txt str=isle:45,36,126,14,3,255...12,255 						//render decimal string to audio file (str has word label prefixed before colon and is ignored)\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5220.txt strhex=45,36,AE,D5,0x56,A7...,AE,D5 						//render hex string to audio file\n");
printf("\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5220.txt filt=off strhex=45,36,AE,D5,0x56,A7...03,DF 				//render hex string to audio file (without lattice filtering)\n");
printf("\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5100.txt strhex=4D,E6,C8,C2,91,8F,BB,9C,72,89,98,84,35,7D,D5,12,D6,98,5A,FA,E2,C1,94,71,0F,B7,D9,8D,09,9F,61,66,B7,13,E3,D3,4D,BC,1F,05,46,9E,AB,65,AD,0E,86,5E,53,F5,2A,EB,BD,CB,F4,B8,9B,96,14,9F,4B,52,2D,C4,B5,2B,CF,74,8A,8B,62,11,1D,EB,D4,E4,CC,34,6B,D2,69,C5,85,9B,D6,A5,3B,CF,8E,BE,4C,E5,A6,CE,9B,6F,78,C9,6B,08,94,FB,FE,F2,2B,CF,D9,54,F7,B4,F4,00\n");
printf("\n");
printf("\n");
printf("\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5100.txt str=isle:69,171,54,174,213,86,167,62,202,212,42,238,150,115,213,85,87,95,115,156,107,145,30,39,251,4,159,52,163,198,206,137,41,154,165,95,236,19,115,114,13,207,39,55,222,126,70,50,25,41,250,250,140,32,178,154,125,243,154,137,123,143,112,239,54,19,243,57,165,222,105,70,26,59,130,187,243,172,115,204,64,162,67,68,74,159,118,62,0,0,149,171,54,174,213,86,167,62,202,212,42,238,150,115,213,85,87,95,115,156,107,145,30,39,251,4,159,52,163,198,206,137,41,154,165,95,236,19,115,114,13,207\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5220.txt strhex=twenty:0x01,0x98,0xD1,0xC2,0x00,0xCD,0xA4,0x32,0x20,0x79,0x13,0x04,0x28,0xE7,0x92,0xDC,0x70,0xCC,0x5D,0xDB,0x76,0xF3,0xD2,0x32,0x0B,0x0B,0x5B,0xC3,0x2B,0xCD,0xD4,0xDD,0x23,0x35,0xAF,0x44,0xE1,0xF0,0xB0,0x6D,0x3C,0xA9,0xAD,0x3D,0x35,0x0E,0xF1,0x0C,0x8B,0x28,0xF7,0x34,0x01,0x68,0x22,0xCD,0x00,0xC7,0xA4,0x04,0xBB,0x32,0xD6,0xAC,0x56,0x9C,0xDC,0xCA,0x28,0x66,0x53,0x51,0x70,0x2B,0xA5,0xBC,0x0D,0x9A,0xC1,0xEB,0x14,0x73,0x37,0x29,0x19,0xAF,0x33,0x8C,0x3B,0xA7,0x24,0xBC,0x42,0xB0,0xB7,0x59,0x09,0x09,0x3C,0x96,0xE9,0xF4,0x58,0xFF,0x0F0x01,0x98,0xD1,0xC2,0x00,0xCD,0xA4,0x32,0x20,0x79,0x13,0x04,0x28,0xE7,0x92,0xDC,0x70,0xCC,0x5D,0xDB,0x76,0xF3,0xD2,0x32,0x0B,0x0B,0x5B,0xC3,0x2B,0xCD,0xD4,0xDD,0x23,0x35,0xAF,0x44,0xE1,0xF0,0xB0,0x6D,0x3C,0xA9,0xAD,0x3D,0x35,0x0E,0xF1,0x0C,0x8B,0x28,0xF7,0x34,0x01,0x68,0x22,0xCD,0x00,0xC7,0xA4,0x04,0xBB,0x32,0xD6,0xAC,0x56,0x9C,0xDC,0xCA,0x28,0x66,0x53,0x51,0x70,0x2B,0xA5,0xBC,0x0D,0x9A,0xC1,0xEB,0x14,0x73,0x37,0x29,0x19,0xAF,0x33,0x8C,0x3B,0xA7,0x24,0xBC,0x42,0xB0,0xB7,0x59,0x09,0x09,0x3C,0x96,0xE9,0xF4,0x58,0xFF,0x0F\n");
printf("\n");
printf("\n");
printf("\n");
printf("ti_lpc_cmd mode=render chip=tms5220.txt strhex=afternoon:0xC7,0xCE,0xCE,0x3A,0xCB,0x58,0x1F,0x3B,0x07,0x9D,0x28,0x71,0xB4,0xAC,0x9C,0x74,0x5A,0x42,0x55,0x33,0xB2,0x93,0x0A,0x09,0xD4,0xC5,0x9A,0xD6,0x44,0x45,0xE3,0x38,0x60,0x9A,0x32,0x05,0xF4,0x18,0x01,0x09,0xD8,0xA9,0xC2,0x00,0x5E,0xCA,0x24,0xD5,0x5B,0x9D,0x4A,0x95,0xEA,0x34,0xEE,0x63,0x92,0x5C,0x4D,0xD0,0xA4,0xEE,0x58,0x0C,0xB9,0x4D,0xCD,0x42,0xA2,0x3A,0x24,0x37,0x25,0x8A,0xA8,0x8E,0xA0,0x53,0xE4,0x28,0x23,0x26,0x13,0x72,0x91,0xA2,0x76,0xBB,0x72,0x38,0x45,0x0A,0x46,0x63,0xCA,0x69,0x27,0x39,0x58,0xB1,0x8D,0x60,0x1C,0x34,0x1B,0x34,0xC3,0x55,0x8E,0x73,0x45,0x2D,0x4F,0x4A,0x3A,0x26,0x10,0xA1,0xCA,0x2D,0xE9,0x98,0x24,0x0A,0x1E,0x6D,0x97,0x29,0xD2,0xCC,0x71,0xA2,0xDC,0x86,0xC8,0x12,0xA7,0x8E,0x08,0x85,0x22,0x8D,0x9C,0x43,0xA7,0x12,0xB2,0x2E,0x50,0x09,0xEF,0x51,0xC5,0xBA,0x28,0x58,0xAD,0xDB,0xE1,0xFF,0x030xC7,0xCE,0xCE,0x3A,0xCB,0x58,0x1F,0x3B,0x07,0x9D,0x28,0x71,0xB4,0xAC,0x9C,0x74,0x5A,0x42,0x55,0x33,0xB2,0x93,0x0A,0x09,0xD4,0xC5,0x9A,0xD6,0x44,0x45,0xE3,0x38,0x60,0x9A,0x32,0x05,0xF4,0x18,0x01,0x09,0xD8,0xA9,0xC2,0x00,0x5E,0xCA,0x24,0xD5,0x5B,0x9D,0x4A,0x95,0xEA,0x34,0xEE,0x63,0x92,0x5C,0x4D,0xD0,0xA4,0xEE,0x58,0x0C,0xB9,0x4D,0xCD,0x42,0xA2,0x3A,0x24,0x37,0x25,0x8A,0xA8,0x8E,0xA0,0x53,0xE4,0x28,0x23,0x26,0x13,0x72,0x91,0xA2,0x76,0xBB,0x72,0x38,0x45,0x0A,0x46,0x63,0xCA,0x69,0x27,0x39,0x58,0xB1,0x8D,0x60,0x1C,0x34,0x1B,0x34,0xC3,0x55,0x8E,0x73,0x45,0x2D,0x4F,0x4A,0x3A,0x26,0x10,0xA1,0xCA,0x2D,0xE9,0x98,0x24,0x0A,0x1E,0x6D,0x97,0x29,0xD2,0xCC,0x71,0xA2,0xDC,0x86,0xC8,0x12,0xA7,0x8E,0x08,0x85,0x22,0x8D,0x9C,0x43,0xA7,0x12,0xB2,0x2E,0x50,0x09,0xEF,0x51,0xC5,0xBA,0x28,0x58,0xAD,0xDB,0xE1,0xFF,0x03.0xC7,0xCE,0xCE,0x3A,0xCB,0x58,0x1F,0x3B,0x07,0x9D,0x28,0x71,0xB4,0xAC,0x9C,0x74,0x5A,0x42,0x55,0x33,0xB2,0x93,0x0A,0x09,0xD4,0xC5,0x9A,0xD6,0x44,0x45,0xE3,0x38,0x60,0x9A,0x32,0x05,0xF4,0x18,0x01,0x09,0xD8,0xA9,0xC2,0x00,0x5E,0xCA,0x24,0xD5,0x5B,0x9D,0x4A,0x95,0xEA,0x34,0xEE,0x63,0x92,0x5C,0x4D,0xD0,0xA4,0xEE,0x58,0x0C,0xB9,0x4D,0xCD,0x42,0xA2,0x3A,0x24,0x37,0x25,0x8A,0xA8,0x8E,0xA0,0x53,0xE4,0x28,0x23,0x26,0x13,0x72,0x91,0xA2,0x76,0xBB,0x72,0x38,0x45,0x0A,0x46,0x63,0xCA,0x69,0x27,0x39,0x58,0xB1,0x8D,0x60,0x1C,0x34,0x1B,0x34,0xC3,0x55,0x8E,0x73,0x45,0x2D,0x4F,0x4A,0x3A,0x26,0x10,0xA1,0xCA,0x2D,0xE9,0x98,0x24,0x0A,0x1E,0x6D,0x97,0x29,0xD2,0xCC,0x71,0xA2,0xDC,0x86,0xC8,0x12,0xA7,0x8E,0x08,0x85,0x22,0x8D,0x9C,0x43,0xA7,0x12,0xB2,0x2E,0x50,0x09,0xEF,0x51,0xC5,0xBA,0x28,0x58,0xAD,0xDB,0xE1,0xFF,0x03\n");
printf("\n");
printf("\n");
printf("ti_lpc_cmd mode=cleanbrace fnamein=Vocab_US_Large.cpp fnameout=Vocab_US_Large_str_clean.txt				//clean a file keeping only what's within braces, useful for extracing strings embeded in c code such as found in Talkie prj on github\n");
printf("\n");
printf("ti_lpc_cmd mode=cleanquote fnamein=a_quote_hex_string_file.txt fnameout=a_hex_string_file.txt			//clean a file keeping only what's within quotes\n");
printf("\n");
printf("\n");
printf("\n");
printf("\n");
printf("\n");
printf("ti_lpc_cmd mode=rendaddrfileseq chip=tms5100.txt rom0=tmc0351n2l.vsm rom1=tmc0352n2l.vsm fnamein=zzzaddr_list.txt line=0	 		//render rom addr (from spec fnamein) using spec line index (will store line index incremented by 1 in 'zzzline_index.txt'), zero based index, so 0 is the first line in file, see also 'mode=romlist' option\n");
printf("\n");
printf("ti_lpc_cmd mode=rendaddrfileseq chip=tms5100.txt rom0=tmc0351n2l.vsm rom1=tmc0352n2l.vsm fnamein=zzzaddr_list.txt	 				//render rom addr (from spec fnamein) using a line index from file 'zzzline_index.txt', will incremented this index and store back to same 'zzzline_index.txt', useful to sequentially progress though a long list of addresses in a file\n");
printf("\n");
printf("ti_lpc_cmd mode=rendaddrfileseq chip=tms5100.txt rom0=tmc0351n2l.vsm rom1=tmc0352n2l.vsm fnamein=zzzaddr_list.txt step=-2	 		//render rom addr (from spec fnamein) using a line index from file 'zzzline_index.txt', will decremented this index twice and store back to same 'zzzline_index.txt', useful to sequentially regress though a long list of addresses in a file\n");
printf("\n");
printf("\n");
printf("\n");
printf("ti_lpc_cmd mode=rendstrfileseq chip=tms5220.txt fnamein=Vocab_US_Large_str_clean.txt line=0	 			//render hex string(from spec fnamein) at line index (will store line index incremented by 1 in 'zzzline_index.txt'), zero based index, so 0 is the first line in file\n");
printf("\n");
printf("ti_lpc_cmd mode=rendstrfileseq chip=tms5220.txt fnamein=Vocab_US_Large_str_clean.txt	 				//render hex string(from spec fnamein) using a line index from file 'zzzline_index.txt', will incremented this index and store back to same 'zzzline_index.txt', useful to sequentially progress though a long list of string in a file, set line index with 'mode=rendstrfileline' option\n");
printf("\n");
printf("ti_lpc_cmd mode=rendstrfileseq chip=tms5220.txt fnamein=Vocab_US_Large_str_clean.txt step=-3	 		//render hex string(from spec fnamein) using a line index from file 'zzzline_index.txt', will decremented this index trice and store back to same 'zzzline_index.txt', useful to sequentially regress though a long list of string in a file, set line index with 'mode=rendstrfileline' option\n");
printf("\n");
}	




void usage_help_c_string_generator( string fname_in, string fname_write )
{
string s1, st;
mystr m1;


if( !m1.readfile( fname_in, 100000 ) )
	{
	printf("usage_help_c_string_generator() - failed to read file: '%s'\n", fname_in.c_str() );
	return;
	}

vector<string> vstr;
m1.LoadVectorStrings( vstr, '\n' );

printf("usage_help_c_string_generator() - read %d lines\n", (int)vstr.size() );

for( int i = 0; i < vstr.size(); i++ )
	{
	s1 = vstr[i];
	
	
	m1 = s1;
	m1.FindReplace( s1, "\\", "\\\\", 0 );								//handle a backslash, do this before quote handling just below as that code also adds slashes which confuses this handling
	
	m1 = s1;
	m1.FindReplace( s1, "\"", "\\\"", 0 );								//handle a quote
	
	st += "printf(\"";
	st += s1;
	st += "\\n\");";
	st += '\n';
	}

m1 = st;


if( !m1.writefile( fname_write ) )
	{
	printf("usage_help_c_string_generator() - failed to write file: '%s'\n", fname_write.c_str() );
	return;
	}

}






int main(int argc, char **argv)
{
string s1, st, sequ, ss, str_lpc, strhex_lpc, slist;
mystr m1;

//usage_help_c_string_generator( "ti_lpc_cmd_help.txt", "zzzusage.txt" );
//return 0;


//test_tone();

bool vb = verbose;

exit_code = 0;															//assume no error

bool param_rom0 = 0;
bool param_rom1 = 0;
bool param_addr = 0;
bool param_str = 0;
bool param_strhex = 0;
bool param_strfile = 0;
bool param_fnamein0 = 0;
bool param_lineidx = 0;

int addr, line_idx, step_direc;
addr = 0;
line_idx = 0;
step_direc = 1;

if(vb)printf("argc %d\n", argc );


vsm = (uint8_t*) malloc( rom_size_max );
bufsamp0 = (float*) malloc( bufsamp_sz * sizeof(float) );
bufsamp1 = (float*) malloc( bufsamp_sz * sizeof(float) );


	
if( argc < 2 )
	{
	show_usage();
	exit_code = 1;	
	goto do_exit;
	}

for( int i = 1; i < argc; i++ )
	{
	if(vb)printf("[%d] '%s'\n", i, argv[i] );
	st += argv[i];
	st += ",";
	}

//m1 = st;
//vector<string> vstr;
//m1.LoadVectorStrings( vstr, ' ' );


m1 = st;
float fv;
int iv;
unsigned int ui;


if( m1.ExtractParamVal_with_delimit( "-help", ",", sequ ) )
	{
	show_usage();
	goto do_exit;
	}

if( m1.ExtractParamVal_with_delimit( "--help", ",", sequ ) )
	{
	show_usage();
	goto do_exit;
	}


if( m1.ExtractParamVal_with_delimit( "verb=", ",", sequ ) )
	{
	if( sequ.compare( "on" ) == 0 )
		{
		verbose = 1;
		vb = verbose;	
		}
	if(vb)printf( "verb=%d \n", verbose );
	}

if( m1.ExtractParamVal_with_delimit( "mode=", ",", sequ ) )
	{
//	m2 = sequ;
//	m2.EscToStr();
	if(vb)printf( "param: mode='%s'\n", sequ.c_str() );

	if( sequ.compare( "romlist" ) == 0 )
		{
		mode_user = en_mu_romlist;
		}

	if( sequ.compare( "render" ) == 0 )
		{
		mode_user = en_mu_render;
		}

	if( sequ.compare( "rendaddrfileline" ) == 0 )
		{
		mode_user = en_mu_rendaddrfileline;
		}

	if( sequ.compare( "rendaddrfileseq" ) == 0 )
		{
		mode_user = en_mu_rendaddrfileseq;
		}

	if( sequ.compare( "rendstrfileline" ) == 0 )
		{
		mode_user = en_mu_rendstrfileline;
		}

	if( sequ.compare( "rendstrfileseq" ) == 0 )
		{
		mode_user = en_mu_rendstrfileseq;
		}

	if( sequ.compare( "cleanbrace" ) == 0 )
		{
		mode_user = en_mu_cleanbrace;
		}

	if( sequ.compare( "cleanquote" ) == 0 )
		{
		mode_user = en_mu_cleanquote;
		}
	}

if( ( mode_user != en_mu_romlist ) && ( mode_user != en_mu_render ) && ( mode_user != en_mu_rendaddrfileline ) && ( mode_user != en_mu_rendaddrfileseq ) && ( mode_user != en_mu_rendstrfileline ) && ( mode_user != en_mu_rendstrfileseq ) && ( mode_user != en_mu_cleanbrace ) && ( mode_user != en_mu_cleanquote ) )
	{
	printf( "ERROR: unknow mode specified.\n" );
	exit_code = 1;
	goto do_exit;
	}

if( m1.ExtractParamVal_with_delimit( "chip=", ",", sequ ) )
	{
	fname_tms_chip = sequ;
	if(vb)printf( "param: chip='%s'\n", sequ.c_str() );
	}

if( m1.ExtractParamVal_with_delimit( "rom0=", ",", sequ ) )
	{
	param_rom0 = 1;
	slast_rom0_filename = sequ;
	if(vb)printf( "param: rom0='%s'\n", sequ.c_str() );
	}

if( m1.ExtractParamVal_with_delimit( "rom1=", ",", sequ ) )
	{
	param_rom1 = 1;
	slast_rom1_filename = sequ;
	if(vb)printf( "param: rom1='%s'\n", sequ.c_str() );
	}


if( m1.ExtractParamVal_with_delimit( "gain=", ",", sequ ) )
	{
	sscanf( sequ.c_str(), "%f" , &fv );
	
	au_aud_gain = fv;
	if( au_aud_gain < 0 ) au_aud_gain = 100;
	if( au_aud_gain > 300 ) au_aud_gain = 300;
	
	if(vb)printf( "gain='%s'  %f\n", sequ.c_str(), fv );
	}


if( m1.ExtractParamVal_with_delimit( "addr=", ",", sequ ) )
	{
	param_addr = 1;
	sscanf( sequ.c_str(), "%x" , &iv );
	addr = iv;
	
	if(vb)printf( "addr='%s'  %x (hex)\n", sequ.c_str(), iv );
	}

if( m1.ExtractParamVal_with_delimit( "filt=", ",", sequ ) )
	{	
	if( sequ.compare( "off" ) == 0 )
		{
		use_filter = 0;	
		}
	if(vb)printf( "filt='%s'  %d (hex)\n", sequ.c_str(), use_filter );
	}



if( m1.ExtractParamVal_with_delimit( "str=", ".", sequ ) )
	{
	param_str = 1;
	str_lpc = sequ;
	
	if(vb)printf( "str='%s'\n", sequ.c_str() );
	}

if( m1.ExtractParamVal_with_delimit( "strhex=", ".", sequ ) )
	{
	param_strhex = 1;
	strhex_lpc = sequ;
	if(vb)printf( "strhex='%s'\n", sequ.c_str() );
	}

//if( m1.ExtractParamVal_with_delimit( "strfile=", ",", sequ ) )
//	{
//	param_strfile = 1;
//	strfile_lpc = sequ;
//	if(vb)printf( "strfile='%s'\n", sequ.c_str() );
//	}




if( m1.ExtractParamVal_with_delimit( "wav=", ",", sequ ) )
	{
	fname_au = sequ;
	if(vb)printf( "wav='%s'\n", sequ.c_str() );
	}


if( m1.ExtractParamVal_with_delimit( "fnamein=", ",", sequ ) )
	{
	param_fnamein0 = 1;
	
	fnamein0 = sequ;
	if(vb)printf( "fnamein='%s'\n", sequ.c_str() );
	}


if( m1.ExtractParamVal_with_delimit( "fnameout=", ",", sequ ) )
	{
	fnameout0 = sequ;
	if(vb)printf( "fnameout='%s'\n", sequ.c_str() );
	}


if( m1.ExtractParamVal_with_delimit( "line=", ",", sequ ) )
	{
	param_lineidx = 1;
	sscanf( sequ.c_str(), "%d" , &iv );
	
	if( iv < 0 ) iv = 0;
	line_idx = iv;

	if(vb)printf( "line=%d\n", iv );
	}

if( m1.ExtractParamVal_with_delimit( "step=", ",", sequ ) )
	{
	sscanf( sequ.c_str(), "%d" , &iv );
	
	step_direc = iv;

	if(vb)printf( "step_direc=%d\n", iv );
	}


if( m1.ExtractParamVal_with_delimit( "srate=", ",", sequ ) )
	{
	sscanf( sequ.c_str(), "%d" , &iv );
	
	srate_user = iv;

	if( srate_user < 4000 ) srate_user = 4000;

	if( srate_user > 192000 ) srate_user = 192000;
	
	srate_wav = srate = srate_au = srate_user;

	if(vb)printf( "srate=%d\n", srate_user );
	}


if( ( param_rom0 == 0 ) && ( param_rom1 == 1 ) )
	{
	printf( "ERROR: must specify rom0 if specifying rom1.\n" );
	exit_code = 1;
	goto do_exit;
	}



if( !load_chip_params( fname_tms_chip ) )
	{
	printf( "ERROR: failed to read tms chip def file: '%s'\n", fname_tms_chip.c_str() );
	exit_code = 1;
	goto do_exit;
	}


if( param_rom0 )
	{
	if( build_rom_word_addr_list( 0x0000, slast_rom0_filename, s1 ) == 0 )
		{
		printf( "ERROR: failed to read rom: '%s'\n", slast_rom0_filename.c_str() );
		exit_code = 1;
		goto do_exit;
		}
	
	slist = s1;
	}

if( param_rom1 )
	{
	if( build_rom_word_addr_list( 0x4000, slast_rom1_filename, s1 ) == 0 )
		{
		printf( "ERROR: failed to read rom: '%s'\n", slast_rom1_filename.c_str() );
		exit_code = 1;
		goto do_exit;
		}
	slist = s1;
	}




//----
if( mode_user == en_mu_romlist )
	{
	if( param_rom0 == 0 )
		{
		printf( "ERROR: need a rom specified for romlist to be shown.\n" );
		exit_code = 1;
		goto do_exit;
		}

	printf( "rom list:\n%s\n", slist.c_str() );
	
//	st = slist0;
//	st += slist1;
	
	m1 = slist;
	
	if ( m1.writefile( fnameout0 ) == 0 )
		{
		printf( "ERROR: could not write list to file: '%s'\n", fnameout0.c_str() );
		exit_code = 1;
		goto do_exit;
		}
	else{
		printf( "list written to file: '%s'\n", fnameout0.c_str() );
		exit_code = 0;
		goto do_exit;
		}
	
	goto do_exit;
	}
//----





//----
if( ( mode_user == en_mu_cleanbrace ) || ( mode_user == en_mu_cleanquote ) )
	{
	char enclosing_left = '?';
	char enclosing_right = '?';
	
	if( param_fnamein0 == 0 )
		{
		printf( "ERROR: need to specify fnamein.\n" );
		exit_code = 1;
		goto do_exit;
		}


	if( mode_user == en_mu_cleanbrace )
		{
		enclosing_left = '{';
		enclosing_right = '}';
		}

	if( mode_user == en_mu_cleanquote )
		{
		enclosing_left = '"';
		enclosing_right = '"';
		}

	if( !cleanup_ccode_snipet_file( fnamein0, s1, enclosing_left, enclosing_right ) )
		{
		printf( "ERROR: failed to clean up file: '%s'\n", fnamein0.c_str() );
		exit_code = 1;
		goto do_exit;
		}

	m1 = s1;
	if ( m1.writefile( fnameout0 ) == 0 )
		{
		printf( "ERROR: could not write cleanup to file: '%s'\n", fnameout0.c_str() );
		exit_code = 1;
		goto do_exit;
		}
	else{
		printf( "clean between %c %c written to file: '%s'\n", enclosing_left, enclosing_right, fnameout0.c_str() );
		exit_code = 0;
		goto do_exit;
		}
	
	goto do_exit;
	}
//----






//----
if( ( mode_user == en_mu_rendaddrfileseq ) )
	{
	if( param_fnamein0 == 0 )
		{
		printf( "ERROR: need to specify a fnamein file containing hex addresses.\n" );
		exit_code = 1;
		goto do_exit;
		}

	if( param_rom0 == 0 )
		{
		printf( "ERROR: can't continue without specifying a rom.\n" );
		exit_code = 1;
		goto do_exit;
		}


	if( param_lineidx == 0 )											//no user spec line index ?
		{
		if( !get_line_at_index( fnamelineindex, 0, s1 ) )
			{
			printf( "NOTE: could not get line index number from file: '%s', recreated file with line index set to zero.\n", fnamelineindex.c_str() );
			s1 = "0\n";
			m1 = s1;
			if( !m1.writefile( fnamelineindex ) )
				{
				printf( "NOTE: could not save line index 0 to file: '%s'\n", fnamelineindex.c_str() );
				}
			exit_code = 1;
			goto do_exit;
			}
		
		if( s1.length() != 0 )											//valid line index in file?
			{
			sscanf( s1.c_str(), "%d", &iv );
			if( iv < 0 ) iv = 0;
			
			line_idx = iv;

			int new_idx = line_idx + step_direc;						//inc/dec line index

			if( new_idx < 0 ) new_idx = 0;

			if( step_direc < 0 )
				{
				line_idx = new_idx;
				}
		
			strpf( s1, "%d", new_idx );
			m1 = s1;
		
			if( !m1.writefile( fnamelineindex ) )
				{
				printf( "NOTE: could not save line index %d to file: '%s'\n", new_idx, fnamelineindex.c_str() );
				}
			}
		else{
			printf( "NOTE: could not get line index from file: '%s', recreated file with line index set to zero.\n", fnamelineindex.c_str() );
			s1 = "0\n";
			m1 = s1;
			if( !m1.writefile( fnamelineindex ) )
				{
				printf( "NOTE: could not save line index 0 to file: '%s'\n", fnamelineindex.c_str() );
				}
			}
		}
	else{
//printf( "here 2 s1 '%s'\n", s1.c_str() );

		int new_idx = line_idx + step_direc;							//inc/dec line index
			
		if( new_idx < 0 ) new_idx = 0;

		strpf( s1, "%d", new_idx );										
		m1 = s1;

		if( !m1.writefile( fnamelineindex ) )
			{
			printf( "NOTE: could not save line index %d to file: '%s'\n", line_idx, fnamelineindex.c_str() );
			}
		}
	
	printf( "getting line at index %d, from file: '%s'.\n", line_idx, fnamein0.c_str() );

	if( !get_line_at_index( fnamein0, line_idx, s1 ) )
		{
		printf( "ERROR: could not get hex address at line %d from file: '%s'.\n", line_idx, fnamein0.c_str() );
		exit_code = 1;
		goto do_exit;
		}

	
	if( s1.length() == 0 )
		{
		printf( "ERROR: line %d was empty, from file: '%s'.\n", line_idx, fnamein0.c_str() );
		exit_code = 1;
		goto do_exit;
		}
		
		
	line_idx = iv;
	sscanf( s1.c_str(), "%x", &iv );									//get addr
	
	printf( "line %d has addr %x\n", line_idx, iv );
	
	if( iv < 0 )
		{
		printf( "ERROR: address obtain from line %d  was negative from file: '%s'.\n", line_idx, fnamein0.c_str() );
		exit_code = 1;
		goto do_exit;
		}

	int max_addr = rom_size_max/2;
	if( param_rom1 == 1 )
		{
		max_addr = rom_size_max;
		}

		
	if( iv >= max_addr )
		{
		printf( "ERROR: address obtain from line %d was: %x Hx and is out of rom address range (0-->%x Hx), from file: '%s'.\n", line_idx, iv, max_addr, fnamein0.c_str() );
		exit_code = 1;
		goto do_exit;
		}

	talk.say_tmc0580( vsm, iv );
	

	goto do_exit;
	}
//----













//----
if( ( mode_user == en_mu_rendstrfileseq ) )
	{
	if( param_fnamein0 == 0 )
		{
		printf( "ERROR: need to specify a fnamein file containing hex strings.\n" );
		exit_code = 1;
		goto do_exit;
		}


	if( param_lineidx == 0 )											//no user spec line index ?
		{
		if( !get_line_at_index( fnamelineindex, 0, s1 ) )
			{
			printf( "NOTE: could not get line index number from file: '%s', recreated file with line index set to zero.\n", fnamelineindex.c_str() );
			s1 = "0\n";
			m1 = s1;
			if( !m1.writefile( fnamelineindex ) )
				{
				printf( "NOTE: could not save line index 0 to file: '%s'\n", fnamelineindex.c_str() );
				}
			exit_code = 1;
			goto do_exit;
			}
		
		if( s1.length() != 0 )											//valid line index in file?
			{
			sscanf( s1.c_str(), "%d", &iv );
			if( iv < 0 ) iv = 0;
			
			line_idx = iv;

			int new_idx = line_idx + step_direc;						//inc/dec line index

			if( new_idx < 0 ) new_idx = 0;

			if( step_direc < 0 )
				{
				line_idx = new_idx;
				}

			strpf( s1, "%d", new_idx );
			m1 = s1;
		
			if( !m1.writefile( fnamelineindex ) )
				{
				printf( "NOTE: could not save line index %d to file: '%s'\n", new_idx, fnamelineindex.c_str() );
				}
			}
		else{
			printf( "NOTE: could not get line index from file: '%s', recreated file with line index set to zero.\n", fnamelineindex.c_str() );
			s1 = "0\n";
			m1 = s1;
			if( !m1.writefile( fnamelineindex ) )
				{
				printf( "NOTE: could not save line index 0 to file: '%s'\n", fnamelineindex.c_str() );
				}
			}
		}
	else{
//printf( "here 2 s1 '%s'\n", s1.c_str() );

		int new_idx = line_idx + step_direc;							//inc/dec line index
			
		if( new_idx < 0 ) new_idx = 0;

		strpf( s1, "%d", new_idx );										
		m1 = s1;

		if( !m1.writefile( fnamelineindex ) )
			{
			printf( "NOTE: could not save line index %d to file: '%s'\n", line_idx, fnamelineindex.c_str() );
			}
		}
	
	printf( "getting line at index %d, from file: '%s'.\n", line_idx, fnamein0.c_str() );

	if( !get_line_at_index( fnamein0, line_idx, s1 ) )
		{
		printf( "ERROR: could not get hex address at line %d from file: '%s'.\n", line_idx, fnamein0.c_str() );
		exit_code = 1;
		goto do_exit;
		}

	
	if( s1.length() == 0 )
		{
		printf( "ERROR: line %d was empty, from file: '%s'.\n", line_idx, fnamein0.c_str() );
		exit_code = 1;
		goto do_exit;
		}
		
		
	
	printf( "line %d has str:\n'%s'\n", line_idx, s1.c_str() );
	

	say_lpc_str( s1, 0 );
	

	goto do_exit;
	}
//----





if( ( param_addr == 1 ) && ( param_rom0 == 0 ) )
	{
	printf( "ERROR: can't specify an addr without specifying a rom.\n" );
	exit_code = 1;
	goto do_exit;
	}

if( ( param_addr == 1 ) && ( ( param_str == 1 ) || ( param_strhex == 1) || ( param_strfile == 1) ) )
	{
	printf( "ERROR: can't specify an addr and a str.\n" );
	exit_code = 1;
	goto do_exit;
	}

if( ( param_addr == 0 ) && ( ( param_str == 0 ) && ( param_strhex == 0) && ( param_strfile == 0) ) )
	{
	printf( "ERROR: need either an addr or a str specified.\n" );
	exit_code = 1;
	goto do_exit;
	}

if( ( param_str == 1 ) && ( param_strhex == 1) )
	{
	printf( "ERROR: can't specify both a str and a strhex.\n" );
	exit_code = 1;
	goto do_exit;
	}

if( ( param_str == 1 ) && ( param_strfile == 1) )
	{
	printf( "ERROR: can't specify both a str and a strfile.\n" );
	exit_code = 1;
	goto do_exit;
	}

if( ( param_strhex == 1 ) && ( param_strfile == 1) )
	{
	printf( "ERROR: can't specify both a strhex and a strfile .\n" );
	exit_code = 1;
	goto do_exit;
	}




//printf("[2] '%s'\n", argv[2] );

//getchar();





if( param_str )
	{
	ss = str_lpc;
	}

if( param_strhex )
	{
	ss = strhex_lpc;
	}

if( param_strfile )
	{
	}

if( param_addr )
	{
	talk.say_tmc0580( vsm, addr );
	}



//ss = "45,AB,36,AE,D5,56,A7,3E,CA,D4,2A,EE,96,73,D5,55,57,5F,73,9C,6B,91,1E,27,FB,04,9F,34,A3,C6,CE,89,29,9A,A5,5F,EC,13,73,72,0D,CF,27,37,DE,7E,46,32,19,29,FA,FA,8C,20,B2,9A,7D,F3,9A,89,7B,8F,70,EF,36,13,F3,39,A5,DE,69,46,1A,3B,82,BB,F3,AC,73,CC,40,A2,43,44,4A,9F,76,3E";
//ss = "0C,58,AC,A5,C1,60,8A,EB,4C,86,D4,43,A3,61,B3,E9,D9,87,38,67,6A,1D,6D,2E,3E,C8,06,57,5D,6B,B2,90,8E,66,FA,92,76,60,33,C4,6C,25,ED,22,6E,AB,73,4A,BE,43,43,B4,A0,88,A6,87,25,EB,26,91,8B,0F,CF,C6,D4,2C,6F,5D,31,44,C4,EA,D9,59,73,88,55,5C,D0,CA,5B,02,73,B5,27,30,FA,B8,07,43,6B,EF,1A,6D,6D,9B,91,C3,35,43,BA,2C,A3,63,9A,37,A7,D9,B6,BB,E6,9C,74,77,CB,58,01,8C,51,11,C0,79,15,09,CC,D7,19,01,06,02,04,37,D3,BC,96,15,16,EA,B7,45,BE,DD,5D,AA,51,2B,7C,66,98,66,A7,D6,95,EE,09,55,7F,6D,88,3C,69,66,79,EA,E8,A3,4D,DD,4A,CB,AC,6F,5A,BA,B9,4E,DB,5E,BF,F4,42,20,8C,56,04,5C,33,C5,E0,01";


if( param_str )
	{
	say_lpc_str( ss, 1 );
	}
 
if( param_strhex )
	{
	say_lpc_str( ss, 0 );
	}


do_exit:
free( vsm );
free( bufsamp0 );
free( bufsamp1 );

//getch();

return exit_code;
}















