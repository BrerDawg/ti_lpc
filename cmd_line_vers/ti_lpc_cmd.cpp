//ti_lpc_cmd.h
//v1.01		27-jan-2024			//




float srate_lpc = 8000;
float srate_wav = 48000;



#include "ti_lpc_cmd.h"



//function prototypes
double qdss_resample( double x, double *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth );



//fast_mgraph gph0;


audio_formats af0;
audio_formats af1;
st_audio_formats_tag saf0;
st_audio_formats_tag saf1;






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




#define ftwopi twopi


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







int main(int argc, char **argv)
{

test_tone();
 

//getch();

return 1;
}















