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


//make SURE to ENABLE compiler optimizer, has halved resampler proc times: -O3 -mfpmath=sse -ffast-math -msse3

//gc_srateconv.cpp
//v1.01		06-sep-2018			//see rem'd out table based test srconv:  test_srconv_tbl()
//v1.02		07-nov-2020			//changed 'twopi' to a constant
//v1.03		07-dec-2020			//added: 'qdss_resample_float_complete()'
//v1.04		18-dec-2020			//added: 'float farrow_resample_float()', 'float farrow_resample_double()'





#include "gc_srateconv.h"



//extern double pi;
//extern double twopi;
#define twopi (2.0*M_PI)
#define ftwopi (2.0f*(float)M_PI)


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













float qdss_resample_float_complete( float x, float *buf, unsigned int bufsz, float fmax, float fsrate, int wnwdth )
{
//return buf[(int)x];

float r_g,r_w,r_a,r_snc,r_y;												//some local variables
int i, j;

if( wnwdth <= 0 ) return 0;
if( bufsz < ( wnwdth / 2 ) ) return 0;

r_g = 2.0f * fmax / fsrate;													//calc gain correction factor

r_y = 0.0f;
for ( i = -( wnwdth / 2 ); i <= ( wnwdth / 2 - 1 ); i++ ) 					//for 1 window width
	{
    j = (int) x + i;          												//calc input sample index

    r_w = 0.5f - 0.5f * cosf( ftwopi * ( 0.5f + ( j - x ) / wnwdth ) );		//make a von hann sample, will be used taper sinc's length

    r_a = ftwopi * ( j - x ) * fmax / fsrate;								//curr sinc location
    r_snc = 1.0f;
	if ( r_a != 0 ) r_snc = sinf( r_a ) / r_a;								//make a sinc (sin x/x) lpf sample

//printf("i: %d, r_w: %f, r_a: %f, r_snc: %f\n", i, r_w, r_a, r_snc );

    if ( ( j >= 0 ) & ( j < bufsz ) )										//src sample avail?
		{
		r_y = r_y + r_g * r_w * r_snc * buf[ j ];							//convolve/mac: first: apply von hann tapering to sinc, then both these to a src sample (adj gain as well)
		}
 	}
return r_y;                  												//new filtered intermediate sample

}



















//----------------------------------------------------------------------


//see 'farrow.cpp' c code in '....MyPrj/gc_music/farrow/' folder, it provides a graph and an indication of multiply/add operations, it also
//shows chebyshev impulse response and explains degree 0, 1 and 2 ( constant, linear interpolation and quadratic interpolation) and when they are used

//refer: https://www.dsprelated.com/showarticle/149.php
//refer: https://www.dsprelated.com/showcode/205.php
//refer: https://www.dsprelated.com/showarticle/22.php
//refer: https://www.dsprelated.com/showcode/203.php				//this has octave code, see  ..../aa_octave_code/farrow.m

/* ****************************************************
 * Farrow resampler (with optional bank switching)
 * M. Nentwig, 2011
 * Input stream is taken from stdin
 * Output stream goes to stdout
 * Main target was readability, is not optimized for efficiency.
 * ****************************************************/



#define order_2		//define this for '2nd' order polynomial 'cMatrix' ie. degree 0 and degree 1, which is a constant and linear interpolation respectively of the chebyshev filter impulse response),
					//stepping rate of exactly 1.0f, 2.0f, 3.0f etc only use bank 0, other rates that have a fractonal component use bank 0, 1, and 2
					//Also '#define order_2' uses slighly less multiply/add operations than '3rd' order polynomial.
					//the '3rd' order polynomial 'cMatrix' has degree 0, degree 1, degree 2(which is: constant, linear and quadratic interpolation of the chebyshev filter impulse response),

					//set 'gain_adj' to AVOID CLIPPING, a different value is needed when compiling for '2nd' order or '3rd' (try '1.0f/3.0f' or '1.0f/4.0f' respectively)

#ifdef order_2
	/* **************************************************************
	 * example coefficients. 
	 * Each column [c0; c1; c2; ...] describes a polynomial for one tap coefficent in fractional time ft [0, 1]:
	 * tapCoeff = c0 + c1 * ft + c2 * ft ^ 2 + ...
	 * Each column corresponds to one tap. 
	 * The example filters is based on a 6th order Chebyshev Laplace-domain prototype.
	 * This version uses three sub-segments per tap (NBANKS = 3)
	 * **************************************************************/
	#define cn_farrow_gain_adj 1.0f/9.0f		//'order_2'

	const float cMatrix_float[] =
		{
		  2.87810386e-4, 4.70096244e-3, 7.93412570e-2, 4.39824536e-1, 1.31192924e+000, 2.67892232e+000, 4.16465421e+000, 5.16499621e+000, 5.15592605e+000, 3.99000369e+000, 2.00785470e+000, -7.42377060e-2, -1.52569354e+000, -1.94402804e+000, -1.40915797e+000, -3.86484652e-1, 5.44712939e-1, 9.77559688e-1, 8.32191447e-1, 3.22691788e-1, -2.13133045e-1, -5.08501962e-1, -4.82928807e-1, -2.36313854e-1, 4.76034568e-2, 2.16891966e-1, 2.20894063e-1, 1.08361553e-1, -2.63421832e-2, -1.06276015e-1, -1.07491548e-1, -5.53793711e-2, 4.86314061e-3, 3.94357182e-2, 4.06217506e-2, 2.17199064e-2, 1.60318761e-3, -8.40370106e-3, -8.10525279e-3, -3.62112499e-3, -4.13413072e-4, 2.33101911e-4, 
		  -3.26760325e-3, -6.46028234e-3, 1.46793247e-1, 5.90235537e-1, 1.18931309e+000, 1.57853546e+000, 1.40402774e+000, 5.76506323e-1, -6.33522788e-1, -1.74564700e+000, -2.24153717e+000, -1.91309453e+000, -9.55568978e-1, 1.58239169e-1, 9.36193787e-1, 1.10969783e+000, 7.33284446e-1, 1.06542194e-1, -4.15412084e-1, -6.06616434e-1, -4.54898908e-1, -1.20841199e-1, 1.82941623e-1, 3.12543429e-1, 2.49935829e-1, 8.05376898e-2, -7.83213666e-2, -1.47769751e-1, -1.18735248e-1, -3.70656555e-2, 3.72608374e-2, 6.71425397e-2, 5.17812605e-2, 1.55564930e-2, -1.40896327e-2, -2.35058137e-2, -1.59635057e-2, -3.44701792e-3, 4.14108065e-3, 4.56234829e-3, 1.59503132e-3, -3.17301882e-4,
		  5.64310141e-3, 7.74786707e-2, 2.11791763e-1, 2.84703201e-1, 1.85158633e-1, -8.41118142e-2, -3.98497442e-1, -5.86821615e-1, -5.40397941e-1, -2.47558080e-1, 1.50864737e-1, 4.59312895e-1, 5.41539400e-1, 3.84673917e-1, 9.39576331e-2, -1.74932542e-1, -3.01635463e-1, -2.56239225e-1, -9.87146864e-2, 6.82216764e-2, 1.59795852e-1, 1.48668245e-1, 6.62563431e-2, -2.71234898e-2, -8.07045577e-2, -7.76841351e-2, -3.55333136e-2, 1.23206602e-2, 3.88535040e-2, 3.64199073e-2, 1.54608563e-2, -6.59814558e-3, -1.72735099e-2, -1.46307777e-2, -5.04363288e-3, 3.31049461e-3, 6.01267607e-3, 3.83904192e-3, 3.92549958e-4, -1.36315264e-3, -9.76017430e-4, 7.46699178e-5
		};

	const double cMatrix_double[] =
		{
		  2.87810386e-4, 4.70096244e-3, 7.93412570e-2, 4.39824536e-1, 1.31192924e+000, 2.67892232e+000, 4.16465421e+000, 5.16499621e+000, 5.15592605e+000, 3.99000369e+000, 2.00785470e+000, -7.42377060e-2, -1.52569354e+000, -1.94402804e+000, -1.40915797e+000, -3.86484652e-1, 5.44712939e-1, 9.77559688e-1, 8.32191447e-1, 3.22691788e-1, -2.13133045e-1, -5.08501962e-1, -4.82928807e-1, -2.36313854e-1, 4.76034568e-2, 2.16891966e-1, 2.20894063e-1, 1.08361553e-1, -2.63421832e-2, -1.06276015e-1, -1.07491548e-1, -5.53793711e-2, 4.86314061e-3, 3.94357182e-2, 4.06217506e-2, 2.17199064e-2, 1.60318761e-3, -8.40370106e-3, -8.10525279e-3, -3.62112499e-3, -4.13413072e-4, 2.33101911e-4, 
		  -3.26760325e-3, -6.46028234e-3, 1.46793247e-1, 5.90235537e-1, 1.18931309e+000, 1.57853546e+000, 1.40402774e+000, 5.76506323e-1, -6.33522788e-1, -1.74564700e+000, -2.24153717e+000, -1.91309453e+000, -9.55568978e-1, 1.58239169e-1, 9.36193787e-1, 1.10969783e+000, 7.33284446e-1, 1.06542194e-1, -4.15412084e-1, -6.06616434e-1, -4.54898908e-1, -1.20841199e-1, 1.82941623e-1, 3.12543429e-1, 2.49935829e-1, 8.05376898e-2, -7.83213666e-2, -1.47769751e-1, -1.18735248e-1, -3.70656555e-2, 3.72608374e-2, 6.71425397e-2, 5.17812605e-2, 1.55564930e-2, -1.40896327e-2, -2.35058137e-2, -1.59635057e-2, -3.44701792e-3, 4.14108065e-3, 4.56234829e-3, 1.59503132e-3, -3.17301882e-4,
		  5.64310141e-3, 7.74786707e-2, 2.11791763e-1, 2.84703201e-1, 1.85158633e-1, -8.41118142e-2, -3.98497442e-1, -5.86821615e-1, -5.40397941e-1, -2.47558080e-1, 1.50864737e-1, 4.59312895e-1, 5.41539400e-1, 3.84673917e-1, 9.39576331e-2, -1.74932542e-1, -3.01635463e-1, -2.56239225e-1, -9.87146864e-2, 6.82216764e-2, 1.59795852e-1, 1.48668245e-1, 6.62563431e-2, -2.71234898e-2, -8.07045577e-2, -7.76841351e-2, -3.55333136e-2, 1.23206602e-2, 3.88535040e-2, 3.64199073e-2, 1.54608563e-2, -6.59814558e-3, -1.72735099e-2, -1.46307777e-2, -5.04363288e-3, 3.31049461e-3, 6.01267607e-3, 3.83904192e-3, 3.92549958e-4, -1.36315264e-3, -9.76017430e-4, 7.46699178e-5
		};

	#define NTAPS (14)
	#define NBANKS (3)
	#define ORDER (2)
#else
	/* Alternative example
	 * Similar impulse response as above
	 * A conventional Farrow design (no subdivisions => NBANKS = 1), order 3
	 */ 

	#define cn_farrow_gain_adj 1.0f/16.0f		//order 3

	const double cMatrix_float[] = 
		{
		  -8.57738278e-3, 7.82989032e-1, 7.19303539e+000, 6.90955718e+000, -2.62377450e+000, -6.85327127e-1, 1.44681608e+000, -8.79147907e-1, 7.82633997e-2, 1.91318985e-1, -1.88573400e-1, 6.91790782e-2, 3.07723786e-3, -6.74800912e-3,
		  2.32448021e-1, 2.52624309e+000, 7.67543936e+000, -8.83951796e+000, -5.49838636e+000, 6.07298348e+000, -2.16053205e+000, -7.59142947e-1, 1.41269409e+000, -8.17735712e-1, 1.98119464e-1, 9.15904145e-2, -9.18092030e-2, 2.74136108e-2,
		  -1.14183319e+000, 6.86126458e+000, -6.86015957e+000, -6.35135894e+000, 1.10745051e+001, -3.34847578e+000, -2.22405694e+000, 3.14374725e+000, -1.68249886e+000, 2.54083065e-1, 3.22275037e-1, -3.04794927e-1, 1.29393976e-1, -3.32026332e-2,
		  1.67363115e+000, -2.93090391e+000, -1.13549165e+000, 5.65274939e+000, -3.60291782e+000, -6.20715544e-1, 2.06619782e+000, -1.42159644e+000, 3.75075865e-1, 1.88433333e-1, -2.64135123e-1, 1.47117661e-1, -4.71871047e-2, 1.24921920e-2
		};

	const double cMatrix_double[] = 
		{
		  -8.57738278e-3, 7.82989032e-1, 7.19303539e+000, 6.90955718e+000, -2.62377450e+000, -6.85327127e-1, 1.44681608e+000, -8.79147907e-1, 7.82633997e-2, 1.91318985e-1, -1.88573400e-1, 6.91790782e-2, 3.07723786e-3, -6.74800912e-3,
		  2.32448021e-1, 2.52624309e+000, 7.67543936e+000, -8.83951796e+000, -5.49838636e+000, 6.07298348e+000, -2.16053205e+000, -7.59142947e-1, 1.41269409e+000, -8.17735712e-1, 1.98119464e-1, 9.15904145e-2, -9.18092030e-2, 2.74136108e-2,
		  -1.14183319e+000, 6.86126458e+000, -6.86015957e+000, -6.35135894e+000, 1.10745051e+001, -3.34847578e+000, -2.22405694e+000, 3.14374725e+000, -1.68249886e+000, 2.54083065e-1, 3.22275037e-1, -3.04794927e-1, 1.29393976e-1, -3.32026332e-2,
		  1.67363115e+000, -2.93090391e+000, -1.13549165e+000, 5.65274939e+000, -3.60291782e+000, -6.20715544e-1, 2.06619782e+000, -1.42159644e+000, 3.75075865e-1, 1.88433333e-1, -2.64135123e-1, 1.47117661e-1, -4.71871047e-2, 1.24921920e-2
		};
	#define NTAPS (14)
	#define NBANKS (1)
	#define ORDER (3)
#endif


/* Set here the ratio between output and input sample rate.
//It could be changed even during runtime!
#define INCR (1.0f / 6.28f / 3.0f)		//set the ratio (orig)
#define INCR ( 0.5f )				//set the ratio
#define INCR (0.9438743126816935f)	//semintone down
#define INCR (1.0594630943592953f)	//semintone up
#define INCR (0.9438743126816935f)	//
*/


// Coefficient lookup "table"
static double c(int ixTap, int ixBank, int ixPower)
	{
	return cMatrix_double[ixPower * (NTAPS * NBANKS) + ixTap * NBANKS + ixBank];
	}




//double farrow_min = 0;
//double farrow_max = 0;

//this uses same fractional 'x' mechanism as the various other 'qdss_resample()' functions,
//here 'x' is called 'sampleIndexOutput', just step this value at the required rate to get any pitch you need
//returns one interpolated sample position at 'sampleIndexOutput', runs much faster that tapered SinC based functions
//see '#define order_2' above for which 'CMatrix' to use,
float farrow_resample_float( float sampleIndexOutput, float *bf, unsigned int bfsize, bool &end_reached )
{

if( sampleIndexOutput >= bfsize ) 
	{
	end_reached = 1;
	return 0.0f;
	}

end_reached = 0;

float out;
int ptr_in;

//Split output sample location into integer and fractional part:
//Integer part corresponds to sample index in input stream
//fractional part [0, 1[ spans one tap (covering NBANKS segments)
 
    int sio_int = floorf(sampleIndexOutput);
    float sio_fracInTap = sampleIndexOutput - (float)sio_int;
 //   assert((sio_fracInTap >= 0) && (sio_fracInTap < 1));
    
//Further split the fractional part into 
//bank index
//fractional part within the bank
    int sio_intBank = floorf(sio_fracInTap * (float) NBANKS);
    float sio_fracInBank = sio_fracInTap * (float) NBANKS - (float)sio_intBank;
//    assert((sio_fracInBank >= 0) && (sio_fracInBank < 1));





//****************************************************
//load new samples into the delay line, until the last 
//processed input sample (sampleIndexInput) catches
//up with the integer part of the output stream position (sio_int)
//****************************************************

int sampleIndexInput;


	sampleIndexInput = (int) sampleIndexOutput - 1;

    out = 0;
	while(sampleIndexInput < sio_int)
		{
		//Advance the delay line one step
		  ++sampleIndexInput;

		ptr_in = sampleIndexInput + 1;


		if( ptr_in >= bfsize ) 
			{
			end_reached = 1;
			goto done;
	//		printf( "farrow() - samplerate conversion completed - saving audio....\n" );
			}
		} //while delay line behind output data


	//****************************************************
	//Calculate one output sample:
	//"out" sums up the contribution of each tap
	//***************************************************

    for ( int ixTap = 0; ixTap < NTAPS; ++ixTap)
		{

		//****************************************************
		//* Contribution of tap "ixTap" to output: 
		//* ***************************************************
		//* Evaluate polynomial in sio_fracInBank:
		//c(ixTap, sio_intBank, 0) is the constant coefficient 
		//c(ixTap, sio_intBank, 1) is the linear coefficient etc

//		double hornerSum = c(ixTap, sio_intBank, ORDER);
		float hornerSum = cMatrix_float[ ORDER * (NTAPS * NBANKS) + ixTap * (int)NBANKS + sio_intBank];
		for( int ixPower = ORDER-1; ixPower >= 0; --ixPower)
			{
			hornerSum *= sio_fracInBank;
//			double d0 =  c(ixTap, sio_intBank, ixPower);
			


//			float f1 = cMatrix_float[ ixPower * (NTAPS * NBANKS) + ixTap * (int)NBANKS + sio_intBank];

			hornerSum += cMatrix_float[ ixPower * (NTAPS * NBANKS) + ixTap * (int)NBANKS + sio_intBank];

//if( d0 != d1 )
	{
//printf( "farrow %f %f\n", d0, d1 );
	}
//			if( d0 < farrow_min ) farrow_min = d0;
//			if( d0 > farrow_max ) farrow_max = d0;

//static double c(int ixTap, int ixBank, int ixPower)
//	{
//	return cMatrix[ixPower * (NTAPS * NBANKS) + ixTap * NBANKS + ixBank];
//	}


			} //for ixPower

		//* ****************************************************
		//* Weigh the delay line sample of this tap with the 
		//* polynomial result and add to output
		//* ***************************************************

		int pp = ptr_in - ixTap;
		if( pp >= 0 )
			{
//printf( "hornerSum %f\n", hornerSum );
			out += hornerSum * bf[ pp ];
			}
		else{
			out += 0.0f;
			}
		} // for ixTap


done: // out of input data => break loops and continue here



return out * cn_farrow_gain_adj;
}








//see float version for usage
double farrow_resample_double( double sampleIndexOutput, double *bf, unsigned int bfsize, bool &end_reached )
{

if( sampleIndexOutput >= bfsize ) 
	{
	end_reached = 1;
	return 0.0f;
	}

double out;
int ptr_in;
end_reached = 0;

//Split output sample location into integer and fractional part:
//Integer part corresponds to sample index in input stream
//fractional part [0, 1[ spans one tap (covering NBANKS segments)
 
    int sio_int = floor(sampleIndexOutput);
    double sio_fracInTap = sampleIndexOutput - (double)sio_int;
//    assert((sio_fracInTap >= 0) && (sio_fracInTap < 1));
    
//Further split the fractional part into 
//bank index
//fractional part within the bank
    int sio_intBank = floor(sio_fracInTap * (double) NBANKS);
    double sio_fracInBank = sio_fracInTap * (double) NBANKS - (double)sio_intBank;
 //   assert((sio_fracInBank >= 0) && (sio_fracInBank < 1));





//****************************************************
//load new samples into the delay line, until the last 
//processed input sample (sampleIndexInput) catches
//up with the integer part of the output stream position (sio_int)
//****************************************************

int sampleIndexInput;


	sampleIndexInput = (int) sampleIndexOutput - 1;

	while(sampleIndexInput < sio_int)
		{
//loop0_cnt++;

		//Advance the delay line one step
		  ++sampleIndexInput;

		ptr_in = sampleIndexInput + 1;

//ptr_in++;


//		for (ix = NTAPS-1; ix > 0; --ix) delayLine[ix] = delayLine[ix-1];      

	  
		//Read one input sample


//delayLine[0] = af0.pch0[ptr] * gain_adj_in;
//	ptr++;

		if( ptr_in >= bfsize ) 
			{
			end_reached = 1;
			goto done;
	//		printf( "farrow() - samplerate conversion completed - saving audio....\n" );
			}


	//      int flag = scanf("%lf", &delayLine[0]);
		} //while delay line behind output data


//printf( "main() - \n" );

	//****************************************************
	//Calculate one output sample:
	//"out" sums up the contribution of each tap
	//***************************************************
    out = 0;

    for (int ixTap = 0; ixTap < NTAPS; ++ixTap)
		{
//loop1_cnt++;

	//****************************************************
	//* Contribution of tap "ixTap" to output: 
	//* ***************************************************
	//* Evaluate polynomial in sio_fracInBank:
	//c(ixTap, sio_intBank, 0) is the constant coefficient 
	//c(ixTap, sio_intBank, 1) is the linear coefficient etc

		double hornerSum = cMatrix_float[ ORDER * (NTAPS * NBANKS) + ixTap * (int)NBANKS + sio_intBank];
		for( int ixPower = ORDER-1; ixPower >= 0; --ixPower)
			{
			hornerSum *= sio_fracInBank;
			hornerSum += cMatrix_float[ ixPower * (NTAPS * NBANKS) + ixTap * (int)NBANKS + sio_intBank];
			} //for ixPower

		//* ****************************************************
		//* Weigh the delay line sample of this tap with the 
		//* polynomial result and add to output
		//* ***************************************************
//		out += hornerSum * delayLine[ixTap];

int pp = ptr_in - ixTap;
if( pp >= 0 )
	{
	out += hornerSum * bf[ pp ];
	}
else{
	out += 0;
	}
		} // for ixTap

//printf(" loop0_cnt %d %d\n", loop0_cnt, loop1_cnt);
  
	//****************************************************
    //* Generate output sample and advance the position of
    //* the next output sample in the input data stream 
    //* ***************************************************
//    printf("%1.12le\n", out);
    
//afout.push_ch0( out/3.0f );
//afout.push_ch1( out/3.0f );

//    sampleIndexOutput += INCR;


//sample_out_cnt++;
//
//if( sample_out_cnt == 103 )		//reset start somewhere within sample, not at begining
//	{
//	loop0_cnt = 0;
//	loop1_cnt = 0;	
//	}
//if( sample_out_cnt == 104 ) goto done;
//


//	} // loop until out of input data
  
 done: // out of input data => break loops and continue here



return out * cn_farrow_gain_adj;
}
//----------------------------------------------------------------------











void qdss_resample_float_1ch_buf( float x, float step_factor, float *bfin, unsigned int bufsz, float *bfout, float fmax, float fsrate, int wnwdth )
{
}








bool test_qdss_resampl()
{
audio_formats af_src, af_dest;
st_audio_formats_tag saf_src, saf_dest;

af_src.verb = 1;                                	//store progress in: string 'log'
af_src.log_to_printf = 1;                       	//show to console aswell

af_dest.verb = 1;                               	//store progress in: string 'log'
af_dest.log_to_printf = 1;                       	//show to console aswell

saf_src.format = en_af_sun;
saf_src.format = en_af_wav_pcm;

string s1 = "testing_stereo_48000.au";
s1 = "sweep48k16b.wav";

if( !af_src.load_malloc( "", s1, 32767, saf_src ) )
	{
    printf( "test_qdss_resampl() - af.load() call returned 0: '%s'\n", s1.c_str() );
    return 0;
	}

saf_dest.is_big_endian = 	saf_src.is_big_endian;
saf_dest.srate = 			saf_src.srate;
saf_dest.encoding = 		saf_src.encoding;
saf_dest.format = 			saf_src.format;
saf_dest.channels = 		saf_src.channels;
//saf_dest.vch0 = 			saf_src.vch0;
//saf_dest.vch1 = 			saf_src.vch1;

double factor = 0.5;			//the req'd change to sample rate, 2 is double the original samplerate, 0.5 is half

unsigned int cnt_src = af_src.sizech0;
unsigned int cnt_dest = cnt_src * factor;

double *asrc = new double[ cnt_src ];
double *adest = new double[ cnt_dest ];

//load src array
for( int i = 0; i < af_src.sizech0; i++ )
	{
	asrc[ i ] = af_src.pch0[ i ] * 0.5;
//	adest[ i ] = 0.1 * asrc[ i ];
//	saf_dest.vch0[ i ] = 0.5 * saf_src.vch0[ i ];
	}

double x = 0;

int i = 0;
int j = 0;
double inc = 1.0 / factor;				//stepping factor src ptr

for( ; ; )
	{
	double val;

	double new_srate = saf_src.srate * factor;				//new samplerate 
	unsigned int filter_window_width = 200;					//e.g: 32 is better that 16
	double lpf_cutoff = new_srate / 2.0 - 1000;		//new samplerate nyquist limit, subtract a margin which needs to be larger if 'filter_window_width' is small

//	adest[ j ] = val;

//break;
	val = adest[ j ] = qdss_resample( x, asrc, cnt_src, lpf_cutoff, saf_src.srate, filter_window_width );
	
	x += inc;								//step src ptr
	i = (int)x;
	if( i >= cnt_src ) break;

	j++;									//step dest ptr
	if( j >= cnt_dest ) break;

/*
	if( !( j & 0x0 ) ) adest[ j ] = qdss_resample( i + 0.0, asrc, cnt_src, lpf_cutoff, new_srate, filter_width );
	j++;
	if( j >= cnt_dest ) break;

	if( j & 0x1 ) 
		{
		val = qdss_resample( i + 0.5, asrc, cnt_src, lpf_cutoff, new_srate, filter_width );
		adest[ j ] = val;
		j++;
		if( j >= cnt_dest ) break;
		}
*/
	}

printf( "---- i: %d, j: %d\n", i, j );


//store dest array
for( int i = 0; i < cnt_dest; i++ )
	{
	af_dest.push_ch0( adest[ i ] );
//	af_dest.push_ch1( adest[ i ] );
	}

saf_dest.srate = saf_src.srate * factor;

s1 = "zz_testing.wav";
if( !af_dest.save_malloc( "", s1, 32767, saf_dest ) )
    {
    printf( "test_qdss_resampl() - af.save() call returned 0: '%s'\n", s1.c_str() );
    return 0;
    }


delete asrc;
delete adest;
}






















//below code shows how to test 'fir_filter()' with a one sample impulse source, you can easily
//revert back to the source audio file by commenting out the: 'make a one sample impulse' code

bool test_fir_filter()
{
audio_formats af_src, af_dest;
st_audio_formats_tag saf_src, saf_dest;

af_src.verb = 1;                                //store progress in: string 'log'
af_src.log_to_printf = 1;                       //show to console aswell

af_dest.verb = 1;                                //store progress in: string 'log'
af_dest.log_to_printf = 1;                       //show to console aswell

saf_src.format = en_af_sun;
saf_src.format = en_af_wav_pcm;

string s1 = "testing_stereo_48000.au";
s1 = "sweep48k16b.wav";

//load a source audio file, it will only be used if you comment out the: 'make a one sample impulse' code
if( !af_src.load_malloc( "", s1, 32767, saf_src ) )
	{
    printf( "test_fir_filter() - af.load() call returned 0: '%s'\n", s1.c_str() );
    return 0;
	}

saf_dest.is_big_endian = 	saf_src.is_big_endian;
saf_dest.srate = 			saf_src.srate;
saf_dest.encoding = 		saf_src.encoding;
saf_dest.format = 			saf_src.format;
saf_dest.channels = 		saf_src.channels;
//af_dest.copypush_ch0( af_src.pch0, af_src.sizech0 );		//push copy for unity tests
//af_dest.copypush_ch1( af_src.pch0, af_src.sizech0 );		//push copy
//saf_dest.channels = 		2;




//----- calc filter coeffs ---
en_filter_window_type_tag  wnd_type;
en_filter_pass_type_tag filt_type;

wnd_type = fwt_blackman_harris;
filt_type = fpt_bandpass;

unsigned int srate = saf_src.srate;
unsigned int taps = 100;					//coeff count

double fc1 = 5000;
double fc2 = 9000;

vector<double> vfir_coeff;

filter_fir_windowed( wnd_type, filt_type, taps, fc1, fc2, srate, vfir_coeff );
//------


//----- make a one sample impulse ----
//centered in middle of total samples
af_dest.clear_ch0();
for( int i = 0; i < 200; i++ )
	{
	if( i == 100 ) af_dest.push_ch0( 1.0 );
	else af_dest.push_ch0( 0.0 );
	}
//-----


//filter
af_dest.fir_filter( 1, vfir_coeff );

//save audio file
s1 = "zz_testing.wav";
if( !af_dest.save_malloc( "", s1, 32767, saf_dest ) )
    {
    printf( "test_fir_filter() - af.save() call returned 0: '%s'\n", s1.c_str() );
    return 0;
    }
}



















//fast_mgraph fgph_temp;


//note: when exactly on a sample, the calc'd sample is still generated via convolution although this is not strictly required
//50% speed reqs 01 in-betweens 		o0 x o1											0, 0.5, 1.0						
//40% speed reqs 04 in-betweens 		o0 x x x x o2	(over 2 samples)				0, 0.4, 0.8, 1.2, 1.6, 2.0 
//20% speed reqs 04 in-betweens 		o0 x x x x o1									0, 0.2, 0.4, 0.6, 0.8, 1.0
//10% speed reqs 09 in-betweens 		o0x x x x x x x x x o1							0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0
//05% speed reqs 19 in-betweens 		o0 x x x x x x x x x x x x x x x x x x x o1		0, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95, 1.0


//e.g: 0.5x, 48000->24000, tbl_cnt = 2, fractional_step = 0.5, frq_cutoff = 23000, e.g: wnd_sz = 64
//original sample---fraction = 0.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)-------+ 2 tables before falling exactly onto an original sample
//interpol sample	fraction = 0.5  idx 1, sr_cnv_tbl[1][64]----------------------------------------+
//original sample---fraction = 1.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//...
//...


//e.g: 0.8x, 48000->38400, tbl_cnt = 5, fractional_step = 0.8, frq_cutoff= 38000, e.g: wnd_sz = 64
//original sample---fraction = 0.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)-------+
//interpol sample	fraction = 0.8  idx 1, sr_cnv_tbl[1][64]										|
//interpol sample	fraction = 1.6  idx 2, sr_cnv_tbl[2][64]										| 5 tables before falling exactly onto an original sample
//interpol sample	fraction = 2.4  idx 3, sr_cnv_tbl[3][64]										|
//interpol sample	fraction = 3.2  idx 4, sr_cnv_tbl[4][64]----------------------------------------+
//original sample---fraction = 4.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//interpol sample	fraction = 4.8  idx 1, sr_cnv_tbl[1][64]
//interpol sample	fraction = 5.6  idx 2, sr_cnv_tbl[2][64]
//interpol sample	fraction = 6.4  idx 3, sr_cnv_tbl[3][64]
//interpol sample	fraction = 7.2  idx 4, sr_cnv_tbl[4][64]
//original sample---fraction = 8.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//...
//...


//e.g: 0.4x, 48000->19200, tbl_cnt = 5, fractional_step = 0.4, frq_cutoff= 18000, e.g: wnd_sz = 64
//original sample---fraction = 0.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)-------+
//interpol sample	fraction = 0.4  idx 1, sr_cnv_tbl[1][64]										|
//interpol sample	fraction = 0.8  idx 2, sr_cnv_tbl[2][64]										| 5 tables before falling exactly onto an original sample
//interpol sample	fraction = 1.2  idx 3, sr_cnv_tbl[3][64]										|
//interpol sample	fraction = 1.6  idx 4, sr_cnv_tbl[4][64]----------------------------------------+
//original sample---fraction = 2.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//interpol sample	fraction = 2.4  idx 1, sr_cnv_tbl[1][64]
//interpol sample	fraction = 2.8  idx 2, sr_cnv_tbl[2][64]
//interpol sample	fraction = 2.2  idx 3, sr_cnv_tbl[3][64]
//interpol sample	fraction = 2.6  idx 4, sr_cnv_tbl[4][64]
//original sample---fraction = 4.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//interpol sample	fraction = 4.4  idx 1, sr_cnv_tbl[1][64]
//interpol sample	fraction = 4.8  idx 2, sr_cnv_tbl[2][64]
//interpol sample	fraction = 4.2  idx 3, sr_cnv_tbl[3][64]
//interpol sample	fraction = 4.6  idx 4, sr_cnv_tbl[4][64]
//original sample---fraction = 5.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//...
//...


//e.g: 0.2x, 48000->9600, tbl_cnt = 5, fractional_step = 0.2, frq_cutoff= 8000, e.g: wnd_sz = 64
//original sample---fraction = 0.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//interpol sample	fraction = 0.2  idx 1, sr_cnv_tbl[1][64]
//interpol sample	fraction = 0.4  idx 2, sr_cnv_tbl[2][64]
//interpol sample	fraction = 0.6  idx 3, sr_cnv_tbl[3][64]
//interpol sample	fraction = 0.8  idx 4, sr_cnv_tbl[4][64]
//original sample---fraction = 1.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//interpol sample	fraction = 1.2  idx 1, sr_cnv_tbl[1][64]
//interpol sample	fraction = 1.4  idx 2, sr_cnv_tbl[2][64]
//interpol sample	fraction = 1.6  idx 3, sr_cnv_tbl[3][64]
//interpol sample	fraction = 1.8  idx 4, sr_cnv_tbl[4][64]
//original sample---fraction = 2.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//interpol sample	fraction = 2.2  idx 1, sr_cnv_tbl[1][64]
//interpol sample	fraction = 2.4  idx 2, sr_cnv_tbl[2][64]
//interpol sample	fraction = 2.6  idx 3, sr_cnv_tbl[3][64]
//interpol sample	fraction = 2.8  idx 4, sr_cnv_tbl[4][64]
//original sample---fraction = 3.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//...
//...

//e.g: 0.1x, 48000->4800, tbl_cnt = 10, fractional_step = 0.1, frq_cutoff= 3000, e.g: wnd_sz = 64
//original sample---fraction = 0.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)-------+
//interpol sample	fraction = 0.1  idx 1, sr_cnv_tbl[1][64]										|
//interpol sample	fraction = 0.2  idx 2, sr_cnv_tbl[2][64]										|
//interpol sample	fraction = 0.3  idx 3, sr_cnv_tbl[3][64]										|
//interpol sample	fraction = 0.4  idx 4, sr_cnv_tbl[4][64]										| 10 tables before falling exactly onto an original sample
//interpol sample	fraction = 0.5  idx 5, sr_cnv_tbl[5][64]										|
//interpol sample	fraction = 0.6  idx 6, sr_cnv_tbl[6][64]										|
//interpol sample	fraction = 0.7  idx 7, sr_cnv_tbl[7][64]										|
//interpol sample	fraction = 0.8  idx 8, sr_cnv_tbl[8][64]										|
//interpol sample	fraction = 0.9  idx 9, sr_cnv_tbl[9][64]----------------------------------------+
//original sample---fraction = 1.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//interpol sample	fraction = 1.1  idx 0, sr_cnv_tbl[1][64]
//interpol sample	fraction = 1.2  idx 1, sr_cnv_tbl[2][64]
//interpol sample	fraction = 1.3  idx 2, sr_cnv_tbl[3][64]
//interpol sample	fraction = 1.4  idx 3, sr_cnv_tbl[4][64]
//interpol sample	fraction = 1.5  idx 4, sr_cnv_tbl[5][64]
//interpol sample	fraction = 1.6  idx 5, sr_cnv_tbl[6][64]
//interpol sample	fraction = 1.7  idx 6, sr_cnv_tbl[7][64]
//interpol sample	fraction = 1.8  idx 7, sr_cnv_tbl[8][64]
//interpol sample	fraction = 1.9  idx 8, sr_cnv_tbl[9][64]
//original sample---fraction = 2.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//...
//...


//e.g: 0.05x, 48000->2400, tbl_cnt = 20, fractional_step = 0.05, frq_cutoff= 1000, e.g: wnd_sz = 64
//original sample---fraction = 0.00  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)------+
//interpol sample	fraction = 0.05  idx 1, sr_cnv_tbl[1][64]										|
//interpol sample	fraction = 0.10  idx 2, sr_cnv_tbl[2][64]										|
//interpol sample	fraction = 0.15  idx 3, sr_cnv_tbl[3][64]										|
//interpol sample	fraction = 0.20  idx 4, sr_cnv_tbl[4][64]										|
//interpol sample	fraction = 0.25  idx 5, sr_cnv_tbl[5][64]										|
//interpol sample	fraction = 0.30  idx 6, sr_cnv_tbl[6][64]										|
//interpol sample	fraction = 0.35  idx 7, sr_cnv_tbl[7][64]										|
//interpol sample	fraction = 0.40  idx 8, sr_cnv_tbl[8][64]										|
//interpol sample	fraction = 0.45  idx 9, sr_cnv_tbl[9][64]										|
//interpol sample	fraction = 0.50  idx 10, sr_cnv_tbl[10][64]										| 20 tables before falling exactly onto an original sample
//interpol sample	fraction = 0.55  idx 11, sr_cnv_tbl[11][64]										|
//interpol sample	fraction = 0.60  idx 12, sr_cnv_tbl[12][64]										|
//interpol sample	fraction = 0.65  idx 13, sr_cnv_tbl[13][64]										|
//interpol sample	fraction = 0.70  idx 14, sr_cnv_tbl[14][64]										|
//interpol sample	fraction = 0.75  idx 15, sr_cnv_tbl[15][64]										|
//interpol sample	fraction = 0.80  idx 16, sr_cnv_tbl[16][64]										|
//interpol sample	fraction = 0.85  idx 17, sr_cnv_tbl[17][64]										|
//interpol sample	fraction = 0.90  idx 18, sr_cnv_tbl[18][64]										|
//interpol sample	fraction = 0.95  idx 19, sr_cnv_tbl[19][64]-------------------------------------+
//original sample---fraction = 1.00  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//...
//...


//e.g: 1.5x, 48000->72000, tbl_cnt = 2, fractional_step = 1.5, frq_cutoff= 24000, e.g: wnd_sz = 64
//original sample---fraction = 0.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)---------+ 2 tables before falling exactly onto an original sample
//interpol sample	fraction = 1.5  idx 1, sr_cnv_tbl[1][64]------------------------------------------+
//original sample---fraction = 3.0  idx 0, sr_cnv_tbl[0][64] (effctively the original sample)
//...
//...


//this can be used to help in converting to a different samplerate, it uses precalculated sinc tables and is very fast compared to 'on the fly' calcs,
//see above for examples of stepping fraction ratios and the number of sinc wfms required for the table, the aim is to get back to a whole sample (an original sample)\
// !!! remember to gain correct o/p of convolution with 'farray' table by:  float gain_corr = 2.0 * frq_cutoff / src_srate;			//calc gain correction factor
//see EXAMPLE code 'srconv_tbl_test()'

//src_srate - samplerate of source signal
//fractional_step - stepping ratio, MUST be a factor of 'src_srate', 0.2 = 20% x src_srate, 1.5 = 150%,   0.2 = 48000Hx -> 9600Hx,  1.5 = 48000Hz -> 72000Hz, see guide above
//frq_cutoff - low pass filter cutoff, if going down in samplerate, set this to 'a little' less that half the new lower samplerate, if going up set this to half the new higher samplerate
//tbl_cnt - this is the number of sinc wfms required to cover all possible fractions including x.0, see guide above. for 48KHz, 0.2 would req 5 sinc tables
//wnd_sz - the hann tapered sinc wfm width to use for convolution, higher is better but slower, e.g: 32, does not need to be a factor of two
//farray - is where the sinc wfms will be stored, it MUST be the size of: 'tbl_cnt' * 'wnd_sz'
//returns 1 on success, else 0
bool make_srate_conv_tables( float src_srate, float fractional_step, float frq_cutoff, uint16_t tbl_cnt, uint16_t wnd_sz, float *farray )
{
printf("make_srate_conv_tables() - src_srate: %f, fractional_step: %f, frq_cutoff: %f, tbl_cnt: %d, wnd_sz: %d\n", src_srate, fractional_step, frq_cutoff, tbl_cnt, wnd_sz );


if( src_srate > 192000 ) return 0;
if( src_srate < 2000 ) return 0;

if( fractional_step <= 0.0 ) return 0;

if( tbl_cnt < 1 ) return 0;
if( wnd_sz < 3 ) return 0;


//vector<float> fgph_vx;
//vector<float> fgph_vy0;
//vector<float> fgph_vy1;

//float twopi = M_PI * 2.0;

float x = 0;

float half_wnd = wnd_sz / 2;

for( int i = 0; i < tbl_cnt; i++ )														//cycle number of tables
	{
	float ff = 1.0/src_srate;
	
//	printf("%03d x: %f\n", i, x );

//	float gain = 2.0 * frq_cutoff / src_srate;											//calc gain correction factor

	for( int j = 0; j < wnd_sz; j++ ) 													//cycle window width
		{
		float fract = x - int(x);
		float theta = twopi * (0.5 + (-half_wnd + j - fract) / wnd_sz);          		//calc hann pos, 0->twopi
		
		float taper = 0.5 - 0.5 * cos( theta );											//make a von hann sample, will be used taper sinC's length

//		printf("   %03d, j-x: %f\n", j, -half_wnd + j - fract );
//		printf("   %03d, theta: %f, hann: %g\n", j, theta, taper );


		theta = twopi * ( -half_wnd + j - fract  ) * frq_cutoff / src_srate;			//calc sinC pos 
		float sinc = 1;
		if ( theta != 0 ) sinc = sin( theta ) / theta;									//make a sinc lpf sample, (sinC is sinx/x)

//		printf("   %03d, pre-pi: %f\n", j,  ( -half_wnd + j - fract  ) );
//		printf("   %03d, theta: %f, sinc: %g\n", j,  theta, sinc );
//		printf("   %03d, theta: %f, taprsinc: %g\n", j,  theta, taper * sinc );

//		if( i == 0 ) fgph_vx.push_back( j );
//		if( i == 0 ) fgph_vy0.push_back( taper * sinc );
//		if( i == 1 ) fgph_vy1.push_back( taper * sinc );
		
		int ptr = i*wnd_sz + j;
		farray[ ptr ] = taper * sinc; 													//store
		}

	x += fractional_step;
//	if( i == 1 ) fgph_temp.plotxy( fgph_vx, fgph_vy0, fgph_vy1 );

//getchar();
//exit(0);
	}


return 1;
}







//returns one interpolated sample, uses preloaded collection of sinc wfms in a table for speed, samples are 'double' types, sinc table is a 'float' type
//pos - is a location in sample buffer to start convolution at, it will be transverse by 'sinc_sz' counts
//tbl_idx - is an index to one of the table's sinc wfms, this represents where between original samples the interpolated sample is to be, index of zero would effectively be the original sample
//sinc_tbl - holds a number of windowed sinc wfms, generated by a function like: 'make_srate_conv_tables()'
//sinc_sz - size of one sinc wfm
//buf - sample buffer
//bufsz - sample buffer size
float srconv_tbl_interp_fir_double( uint64_t pos, int tbl_idx, float *sinc_tbl, int sinc_sz, double *buf, unsigned int bufsz )
{

float mac = 0;
int ptr = pos;
int pcoeff = tbl_idx * sinc_sz;
for( uint64_t i = 0; i < sinc_sz; i++ )								//convolve
	{
	if( ptr >= bufsz ) break;
	
	mac += buf[ ptr++ ] * sinc_tbl[ pcoeff++ ];
	}
return mac;
}





//returns one interpolated sample, uses preloaded collection of sinc wfms in a table for speed, samples are 'float' types, sinc table is a 'float' type
//pos - is a location in sample buffer to start convolution at, it will be transverse by 'sinc_sz' counts
//tbl_idx - is an index to one of the table's sinc wfms, this represents where between original samples the interpolated sample is to be, index of zero would effectively be the original sample
//sinc_tbl - holds a number of windowed sinc wfms, generated by a function like: 'make_srate_conv_tables()'
//sinc_sz - size of one sinc wfm
//buf - sample buffer
//bufsz - sample buffer size
float srconv_tbl_interp_fir_float( uint64_t pos, int tbl_idx, float *sinc_tbl, int sinc_sz, float *buf, unsigned int bufsz )
{

float mac = 0;
int ptr = pos;
int pcoeff = tbl_idx * sinc_sz;
for( uint64_t i = 0; i < sinc_sz; i++ )								//convolve
	{
	if( ptr >= bufsz ) break;
	
	mac += buf[ ptr++ ] * sinc_tbl[ pcoeff++ ];
	}
return mac;
}










/*

fast_mgraph fgph_sinc(2);
fast_mgraph fgph_srconv;


//below code demonstrates samplerate conversion using hann tapered/windowed sinc wfms held in a table (single precision), it runs reasonably fast, the sinc tables
//are generated by above: 'make_srate_conv_tables()', see also the examples of fractions and their req table sizes/counts in 'make_srate_conv_tables()',
//the below code generates a 400Hz test tone at 48KHz, then down converts it to 40Hz, it does this for two wnd sizes, first with a 64 tap sinc wnd,
//then with an 9 tap wnd, it save audio as 'zzsrconv1.au' and 'zzsrconv2.au', you can clearly hear interpolation distortion on the second file,
//also, the distortion harmonics are visible with 'baudline', on the 'fast_graph' display you can just see the distortion on the second smaller trace,
//the phase shifts are due to the different sinc sizes which req setting the starting point past the beginning of the 400Hz tone's first sample, half the sinc
//size has been chosen to provided clean conversion at the start of the converted tone. Note: the 9 tap converted signal is lower in level, even
//though a 'gain_corr' is applied, a comparision has been made with double precision 'qdss_resample()' (rem'd out), and quality is the same, interestingly 'qdss_resample()'
//suffers a loss of quality when the interger part of the passed sample is quite large (try a 10 sec duration, you can see in baudline the noise gets worse as numbers increase over time),
//this may be due to loss of precision in the sin/cos functions that it repeatedly calls,
//note: for small sinc wnd size, its better to use an odd number as this gives a slightly less interpolation distortion
//note: see also the use of 'srconv_tbl_interp_fir_float()', rem'd out below

//rem: check with baudline

void test_srconv_tbl()
{

audio_formats af1, af2;
st_audio_formats_tag saf1;


#define cn_sr_cnv_fractions_max 20
#define cn_sr_cnv_wnd_max 128

float sr_cnv_tbl_1[ cn_sr_cnv_fractions_max * cn_sr_cnv_wnd_max ];						//samplerate converter sinc coeff table
float sr_cnv_tbl_2[ cn_sr_cnv_fractions_max * cn_sr_cnv_wnd_max ];						//samplerate converter sinc coeff table


vector<float> fgph_vx;
vector<float> fgph_vy0;
vector<float> fgph_vy1;




//float buf_dummy[ 1024 ];
//qdss_resample_float_hack( 0, buf_dummy, 1024, 38400 / 2, 48000, 64 );

//make_srate_conv_tables( 48000, fractional_step, 9600 / 2.0, 5, 64, sr_cnv_tbl );

float dur = 0.1;
float src_srate = 48000;
float fractional_step = 0.1;							//0.1 for src_srate of 48000Hz amounts to srate conversion down to 4800Hz, 400Hz tone becomes 40Hz
float frq_cutoff = (src_srate * fractional_step) / 2.1;	//slightly more than nyquist
int tbl_cnt = 10;										//number of sinc wfms req, see above examples, 0.1 stepping reqs 10 sinc wfms in table
int sinc_sz1 = 64;										//use 2 different sinc wfm sizes (taps) to show quality change
int sinc_sz2 = 9;										//for small wnds, use an ODD number, this helps reduce interpolation distortion slightly

make_srate_conv_tables( src_srate, fractional_step, frq_cutoff, tbl_cnt, sinc_sz1, sr_cnv_tbl_1 );			//make multiple sinc wfms in table: 'sr_cnv_tbl_x'
make_srate_conv_tables( src_srate, fractional_step, frq_cutoff, tbl_cnt, sinc_sz2, sr_cnv_tbl_2 );



//show sinc wfm
for( int i = 0; i < sinc_sz1; i++ )
	{
	fgph_vx.push_back( i );
	int ptr = 0 + i;
	fgph_vy0.push_back( sr_cnv_tbl_1[ ptr ] );
	}
fgph_sinc.plotxy( 0, fgph_vx, fgph_vy0, "brwn", "ofw", "drkb", "drkgry", "sinc 64 tap" );

//void plotxy( vector<double> &vx, vector<double> &vy1, const char *col_trc1="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );

fgph_vx.clear();
fgph_vy0.clear();

//show sinc wfm
for( int i = 0; i < sinc_sz2; i++ )
	{
	fgph_vx.push_back( i );
	int ptr = 0 + i;
	fgph_vy0.push_back( sr_cnv_tbl_2[ ptr ] );
	}

fgph_sinc.plotxy( 1, fgph_vx, fgph_vy0, "brwn", "ofw", "drkb", "drkgry", "sinc 8 tap" );


fgph_vx.clear();
fgph_vy0.clear();



float theta1 = 0;
double theta1_inc = 400 * twopi / src_srate;

int samp_cnt = dur * src_srate;

float *fsamples = new float[ samp_cnt ];
double *dsamples = new double[ samp_cnt ];

for( int i = 0; i < samp_cnt; i++ )									//make test sine wfm
	{
	
//	fgph_vx.push_back( i );

	double dd = sin( theta1 );
	fsamples[ i ] = dd;
	dsamples[ i ] = dd;
//	fgph_vy0.push_back( ff );

//	fgph_vy1.push_back( ff * 0.9 );

	theta1 += theta1_inc;
	if( theta1 >= twopi ) theta1 -= twopi;
	}

//string fname = "/home/gc/Desktop/mp3/Film Soundtracks/BasilPoledouris/BasilPoledouris/ConanTrimmed48K.wav";

//if( !af1.find_file_format( "", fname, saf1.format ) )
//	{
//	printf( "failed to detct audio file format: '%s'\n", fname.c_str() );
//	exit(0);
//	}

//if( !af1.load_malloc( "", fname, 0, saf1 ) )
//	{
//	printf( "failed to load audio: '%s'\n", fname.c_str() );
//	exit(0);
//	}


//---- 64 tap ----

float gain_corr = 2.0 * frq_cutoff / src_srate;										//calc gain correction factor

int half_sinc_sz = sinc_sz1 / 2;

int srconv_modulo = 0;
float fpos = half_sinc_sz;
for( int i = half_sinc_sz; i < samp_cnt - half_sinc_sz; i++ )
//for( int i = half_sinc_sz; i < 48000*10 - half_sinc_sz; i++ )
	{
//	fgph_vx.push_back( i * 1.0 / src_srate );
//	fgph_vy0.push_back( farry[i] );
	
loop1:
	float mac = 0;
//	float mac2 = 0;
	for( int j = 0; j < sinc_sz1; j++ )													//convolve
		{
		int ptr = ((int)fpos) + j - half_sinc_sz;
		if( ptr >= samp_cnt ) goto done1;
		
		mac += fsamples[ ptr ] * sr_cnv_tbl_1[ srconv_modulo*sinc_sz1 + j ];			//sine test
//		mac += af1.pch0[ ((int)fpos) + j - half_sinc_sz ] * sr_cnv_tbl[ srconv_modulo*sinc_sz + j ];	
		}

//mac = srconv_tbl_interp_fir(fpos, srconv_modulo, sr_cnv_tbl_1, sinc_sz1, fsamples, samp_cnt );		//this does the same convolution just above and produces the same result


//extern double qdss_resample( double x, double *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth );
//	mac = qdss_resample( fpos, dsamples, samp_cnt, frq_cutoff, src_srate, sinc_sz1 );	//do an 'on the fly calc' for comparision
	af1.push_ch0( mac * gain_corr * 0.9 );

//	af1.push_ch0( farry[i - 32 ] * 0.95 );
	fgph_vx.push_back( fpos * 1.0 / (src_srate * fractional_step) );
//	af2.push_ch0( mac * gain_corr * 0.9 );
	fgph_vy0.push_back( mac * gain_corr );

//	printf( "i: %02d, srconv_modulo: %d, fpos: %f\n", i, srconv_modulo, fpos );
	fpos += fractional_step;
	srconv_modulo++;
	if( srconv_modulo >= tbl_cnt ) 
		{
		fpos = nearbyint( fpos );								//round up to nearest integer, this would be an orginal sample position
//		printf( "-->i: %02d, srconv_modulo: %d, fpos: %f\n", i, srconv_modulo, fpos );
//		getchar();
		srconv_modulo = 0;
		}
	else{
		goto loop1;
		}
	}
//---------
	
done1:


//---- 8 tap ----
half_sinc_sz = sinc_sz2 / 2;

srconv_modulo = 0;
fpos = half_sinc_sz;
for( int i = half_sinc_sz; i < samp_cnt - half_sinc_sz; i++ )
	{
	
loop2:
	float mac = 0;
	for( int j = 0; j < sinc_sz2; j++ )												//convolve
		{
		int ptr = ((int)fpos) + j - half_sinc_sz;
		if( ptr >= samp_cnt ) goto done2;
		
		mac += fsamples[ ptr ] * sr_cnv_tbl_2[ srconv_modulo*sinc_sz2 + j ];			//sine test
		}

//mac = srconv_tbl_interp_fir(fpos, srconv_modulo, sr_cnv_tbl_1, sinc_sz1, fsamples, samp_cnt );		//this does the same convolution just above and produces the same result

//extern double qdss_resample( double x, double *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth );
//	mac = qdss_resample( fpos, dsamples, samp_cnt, frq_cutoff, src_srate, sinc_sz2 );	//do an 'on the fly calc' for comparision

	af2.push_ch0( mac * gain_corr * 0.9 * 2.4 );			//with 8 tap wnd, the gain is quite out, bring it back up

	fgph_vy1.push_back( mac * gain_corr * 2.4 );			//with 8 tap wnd, the gain is quite out, bring it back up

	fpos += fractional_step;
	srconv_modulo++;
	if( srconv_modulo >= tbl_cnt ) 
		{
		fpos = nearbyint( fpos );								//round up to nearest integer, this would be an orginal sample position
		srconv_modulo = 0;
		}
	else{
		goto loop2;
		}
	}
//---------




done2:
fgph_srconv.plotxy( fgph_vx, fgph_vy0, fgph_vy1, "brwn", "drkg", "ofw", "drkb", "drkgry", "64 tap", "8 tap" );


saf1.encoding = 3;
saf1.offset = 0;
saf1.format = en_af_sun;
saf1.channels = 1;
saf1.srate = src_srate;
af1.save_malloc( "", "zzsrconv1.au", 32767, saf1 );
af2.save_malloc( "", "zzsrconv2.au", 32767, saf1 );

delete fsamples;

//printf("af2.sizech0: %d %d %d\n", af2.sizech0, fgph_vx.size(), fgph_vy0.size() );
//getchar();
//float ff = convolve_with_tble(  buf_dummy, 1024, sr_cnv_tbl[ 0 ], 64 );

//getchar();
//exit(0);
	
}
*/



