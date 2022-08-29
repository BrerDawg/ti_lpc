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

//filter_code.cpp

//v1.01		01-sep-2018


#include "filter_code.h"




extern double pi;
extern double twopi;

extern void cslpf( const char *fmt,... );





//reads an iowa hills coeffients text file and get filter coeffs for one specified section,
//only parses the 2'nd Order Sections that have headings: 'Sect xx',
//specify required section starting at index 0,
//returns 1 on success, else 0
bool read_iowa_hills_coeffs( string fname, int section, double &a1, double &a2, double &b0, double &b1, double &b2 )
{
string s1, ssection;
mystr m1;


if( !m1.readfile( fname, 20000 ) )			//saftey margin
	{
	printf("read_iowa_hills_coeffs() - failed to read file: '%s'\n", fname.c_str() );
	return 0;
	}

printf("read_iowa_hills_coeffs() ---------------------------------\n" );

strpf( ssection, "Sect %d", section );

bool in_section = 0;
int section_line = 0;
int check_count = 0;

char c0, c1;
double dd;
int ii = 0;
for( ii = 0; ii < 20000; ii++ )				//saftey margin
	{
	if( !m1.gets( s1 ) ) break;
	if( s1.compare( ssection ) == 0 ) in_section = 1;

	if( in_section )
		{
		printf("got %03d: line: %03d, '%s'\n", ii, section_line, s1.c_str() );

		if( section_line == 2 ) { sscanf( s1.c_str(), "%c%c %lf", &c0, &c1, &dd );  a1 = dd; printf("check %c%c  %.17f\n", c0, c1, a1 ); check_count++; }
		if( section_line == 3 ) { sscanf( s1.c_str(), "%c%c %lf", &c0, &c1, &dd );  a2 = dd; printf("check %c%c  %.17f\n", c0, c1, a2 ); check_count++; }
		if( section_line == 4 ) { sscanf( s1.c_str(), "%c%c %lf", &c0, &c1, &dd );  b0 = dd; printf("check %c%c  %.17f\n", c0, c1, b0 ); check_count++; }
		if( section_line == 5 ) { sscanf( s1.c_str(), "%c%c %lf", &c0, &c1, &dd );  b1 = dd; printf("check %c%c  %.17f\n", c0, c1, b1 ); check_count++; }
		if( section_line == 6 ) { sscanf( s1.c_str(), "%c%c %lf", &c0, &c1, &dd );  b2 = dd; printf("check %c%c  %.17f\n", c0, c1, b2 ); check_count++; }
 
		section_line++;
		if( section_line >= 7 ) break;
		}
	}

//printf("read_iowa_hills_coeffs() - done - section_line %d, check_count: %d\n", section_line, check_count );
if( section_line != 7 )
	{
	printf("read_iowa_hills_coeffs() - 'section_line' expected to reach 7, only reached: %d, check coeffs read correctly\n", section_line );
	return 0;
	}

if( check_count != 5 )
	{
	printf("read_iowa_hills_coeffs() - 'check_count' expected to reach 5, only reached: %d, check coeffs read correctly\n", check_count );
	return 0;
	}

printf("read_iowa_hills_coeffs() ---------------------------------\n" );
return 1;
}



//note a0 is not used (it would be one), the other coeffs should be divided by the a0 you calc'd
// -- X --> add ------->------+-----> b0 ----> add --- Y ----> 
//           ^                |                 ^
//           |                v                 |
//           |                |                 |
//           ^               xn0                ^
//           |                |                 |
//           |                |                 |
//           |                v                 |
//          add <...-a1 <... xn0 ....> b1 ...> add
//           ^                |                 ^
//           |                v                 |
//           |                |                 |
//           |               xn1                |
//           ^                |                 ^
//           |                |                 |
//           |                v                 |
//           |<....-a2 <...  add ....> b2 ....> |



//2nd order direct form 2 iir filter

bool filter_iir_2nd_order( vector <st_cplex_tag> &viq, double a1, double a2, double b0,  double b1, double b2, double &d0r, double &d1r, double &d0i, double &d1i )
{

for ( int i = 0; i < viq.size(); i++ )					//for every sample
	{

    double sum_rev = viq[ i ].real - a1 * d0r - a2 * d1r;

    double sum_fwd = sum_rev * b0 + b1 * d0r + b2 * d1r;

    viq[ i ].real = sum_fwd;

    d1r = d0r;
    d0r = sum_rev;
	}


for ( int i = 0; i < viq.size(); i++ )					//for every sample
	{

    double sum_rev = viq[ i ].imag - a1 * d0i - a2 * d1i;

    double sum_fwd = sum_rev * b0 + b1 * d0i + b2 * d1i;

    viq[ i ].imag = sum_fwd;

    d1i = d0i;
    d0i = sum_rev;
	}
}








//calc spec filter coeffs suitable for a Direct Form 1 - IIR, 2nd Order

//see ...
//  http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt



//Direct Form 1 - IIR, 2nd Order
// -X-----+---> b0 ----> add --------------+----Y----> 
//        |               ^                |
//       xn0              |               yn0
//        |               |                |
//        |...> b1 ....> add <.... -a1 <...|
//        |               ^                |
//       xn1              |               yn1
//        |               |                |
//        |...> b2 ....> add <.... -a2 <...|



bool calc_filter_iir_2nd_order( en_filter_pass_type_tag filt_type, double fc1, double in_Q, double db_gain, double srate, vector<double> &vcoeff )
{

printf( "filter_iir_2nd_order() - type: %u, fc1: %f, q: %f, db_gain: %f\n", filt_type, fc1, in_Q, db_gain );

if( fc1 >= srate / 2.0 )
	{
	printf( "filter_iir_2nd_order() - fc1: %f violates Nyquist, limiting to: %f\n", fc1, srate / 2.0 - 1 );
	fc1 = srate / 2.0 - 1;
	}

double Fs = srate;
double f0 = fc1;
double w0 = twopi * f0 / Fs;

double Q = in_Q;
//double BW = in_bw / fc1;

double dBgain = db_gain;

double A = pow ( 10.0 , ( dBgain / 40.0 ) );                   // A = sqrt( 10.0 ^( dBgain / 20.0 ) ) (for peaking and shelving EQ filters only)

double gain = pow ( 10.0 , ( db_gain / 20.0 ) );

double alpha = sin( w0 ) / ( 2.0 * Q );                                                         //(case: Q)
//double alpha = sin( w0 ) * sinh( log( 2.0 ) / 2.0 * BW * w0 / sin( w0 ) );                    //(case: BW)
//double alpha = sin( w0 ) / 2.0 * sqrt( ( A + 1.0 / A ) * ( 1.0 / S - 1 ) + 2.0 );             //(case: S)

double a0, a1, a2;
double b0, b1, b2;

bool ok = 0;

printf( "filter_iir_2nd_order() - w0: %f, alpha: %f, A: %f\n", w0, alpha, A );

if( filt_type == fpt_lowpass )
    {
    b0 =  ( 1.0 - cos( w0 ) ) / 2.0;
    b1 =   1.0 - cos( w0 );
    b2 =  ( 1.0 - cos( w0 ) ) / 2.0;
    a0 =   1.0 + alpha;
    a1 =  -2.0 * cos( w0 );
    a2 =   1.0 - alpha;
    ok = 1;
    }


if( filt_type == fpt_highpass )
    {
    b0 =  ( 1.0 + cos( w0 ) ) / 2.0;
    b1 = - ( 1.0 + cos( w0 ) );
    b2 =  ( 1.0 + cos( w0 ) ) / 2.0;
    a0 =   1.0 + alpha;
    a1 =  -2.0 * cos( w0 );
    a2 =   1.0 - alpha;
    ok = 1;
    }


if( filt_type == fpt_bandpass )           //constant skirt gain, peak gain = Q
    {
    b0 =   sin(w0)/2.0;             //Q*alpha;
    b1 =   0.0;
    b2 =  -sin(w0)/2.0;             //-Q*alpha;
    a0 =   1.0 + alpha;
    a1 =  -2.0*cos(w0);
    a2 =   1.0 - alpha;
    ok = 1;
    }

if( filt_type == fpt_bandpass2 )           //constant 0 dB peak gain
    {
    b0 =   alpha;
    b1 =   0.0;
    b2 =  -alpha;
    a0 =   1.0 + alpha;
    a1 =  -2.0*cos(w0);
    a2 =   1.0 - alpha;
    ok = 1;
    }

if( filt_type == fpt_notch )
    {
    b0 =   1.0;
    b1 =  -2.0*cos(w0);
    b2 =   1.0;
    a0 =   1.0 + alpha;
    a1 =  -2.0*cos(w0);
    a2 =   1.0 - alpha;
    ok = 1;
    }

if( filt_type == fpt_apf )
    {
    b0 =   1.0 - alpha;
    b1 =  -2.0*cos(w0);
    b2 =   1.0 + alpha;
    a0 =   1.0 + alpha;
    a1 =  -2.0*cos(w0);
    a2 =   1 - alpha;
    ok = 1;
    }

if( filt_type == fpt_peakeq )
    {
    b0 =   1.0 + alpha*A;
    b1 =  -2.0*cos(w0);
    b2 =   1.0 - alpha*A;
    a0 =   1.0 + alpha/A;
    a1 =  -2.0*cos(w0);
    a2 =   1.0 - alpha/A;
    ok = 1;
    }

if( filt_type == fpt_lowshelf )
    {
    b0 =    A*( (A+1.0) - (A-1.0)*cos(w0) + 2.0*sqrt(A)*alpha );
    b1 =  2.0*A*( (A-1.0) - (A+1.0)*cos(w0)                   );
    b2 =    A*( (A+1.0) - (A-1.0)*cos(w0) - 2.0*sqrt(A)*alpha );
    a0 =        (A+1.0) + (A-1.0)*cos(w0) + 2.0*sqrt(A)*alpha;
    a1 =   -2.0*( (A-1.0) + (A+1.0)*cos(w0)                   );
    a2 =        (A+1.0) + (A-1.0)*cos(w0) - 2.0*sqrt(A)*alpha;
    ok = 1;
    }

if( filt_type == fpt_highshelf )
    {
    b0 =    A*( (A+1.0) + (A-1.0)*cos(w0) + 2.0*sqrt(A)*alpha );
    b1 = -2.0*A*( (A-1.0) + (A+1.0)*cos(w0)                   );
    b2 =    A*( (A+1.0) + (A-1.0)*cos(w0) - 2.0*sqrt(A)*alpha );
    a0 =        (A+1.0) - (A-1.0)*cos(w0) + 2.0*sqrt(A)*alpha;
    a1 =    2.0*( (A-1.0) - (A+1.0)*cos(w0)                  );
    a2 =        (A+1.0) - (A-1.0)*cos(w0) - 2.0*sqrt(A)*alpha;
    ok = 1;
    }

if( !ok )
    {
    printf( "filter_iir_2nd_order() - type: %u, is not supported\n", filt_type );
    return 0;
    }

vcoeff.clear();

vcoeff.push_back( a1 / a0 );
vcoeff.push_back( a2 / a0 );
vcoeff.push_back( b0 / a0 );
vcoeff.push_back( b1 / a0 );
vcoeff.push_back( b2 / a0 );


printf( "filter_iir_2nd_order():\n" );
printf( "a1: %f, a2: %f\n", a1 / a0, a2 / a0 );
printf( "b0: %f, b1: %f, b2: %f\n", b0 / a0, b1 / a0, b2 / a0 );

//conv_vcoeff_to_scoeff( vcoeff, scoeff );

return 1;
}


















//calc spec filter coeffs using spec window type(e.g. blackman) and spec filter type(e.g: hpf, notch)
//refer http://www.mikroe.com/chapters/view/72/chapter-2-fir-filters/
bool filter_fir_windowed( en_filter_window_type_tag   wnd_type, en_filter_pass_type_tag   filt_type, unsigned int taps, double fc1, double fc2, double srate, vector<double> &vcoeff )
{
int ilow;

if( ( taps & 0x1 ) == 1 )
	{
	taps++;
	printf( "filter_fir_windowed() - to keep window impulse response symetrical, taps count will be slighlty increased\n" );
	}

if( taps >= cn_filter_tap_limit )
	{
	printf( "filter_fir_windowed() - tap count is too large: %u, limit is: %u\n", taps, cn_filter_tap_limit - 1 );
	return 0;
	}

printf( "filter_fir_windowed() - srate: %f, fc1: %f, fc2: %f\n", srate, fc1, fc2 );
//printf( "filter_fir_windowed() - wnd_type: %d, filt_type: %d\n", wnd_type, filt_type );


double fs = srate;

//    fs = 20000;
//    fc1 = 2500;

double *w = new double[ taps + 10 ];            //add some extra space, even though only one extra space is required (as N = taps + 1)
double *hd = new double[ taps + 10 ];
double *hcoeff = new double[ taps + 10 ];

//filt_type = fpt_highpass;


vcoeff.clear();

double twopi = 2.0 * M_PI;

double wc1, wc2;

wc1 = twopi * fc1 / fs;
wc2 = twopi * fc2 / fs;

int Nf = taps;
double N = Nf + 1;

printf( "filter_fir_windowed() - coeff count will be: %u\n", (unsigned int)N );


double M = (double)Nf / 2.0;

//cslpf( 0, "filter_fir_windowed() - wnd_type: %d, filt_type: %d, taps: %d, wc1: %g, wc2: %g\n", wnd_type, filt_type, taps, wc1, wc2 );



//!! fix this
if( wnd_type == fwt_kaiser )
    {
    wnd_type = fwt_undefined;
    printf( "filter_fir_windowed() - wnd=kaiser not yet supported\n" );
    }


for( double n = 0; n < N; n++ )
	{
	int i = (int)n;
	
    
	switch( wnd_type )
		{
		case fwt_rect:
			w[ i ] = 1.0;                              										//rect window impulse resp
		break;

		case fwt_bartlett:
            ilow = ( N - 1.0 ) / 2.0 ;                        //work out which formula to use (what side of the triangle peak we are in)
            
            if( i <= ilow ) w[ i ] = 2.0 * n / ( N - 1.0 ) ;                                //bartlett/triangle window impulse resp
            else w[ i ] =  2.0 - 2.0 * n / ( N - 1.0 ) ;
		break;

		case fwt_hann:
           w[ i ] = 0.5 * ( 1.0 - cos( ( twopi * n / ( N - 1.0 ) ) ) );                     //hann window impulse resp
		break;

		case fwt_bartlett_hanning:
            w[ i ] = 0.62 - 0.48 * fabs( n / ( N - 1.0 ) - 0.5 ) + 0.38 * cos( ( twopi * ( n / ( N - 1.0 ) - 0.5 ) ) );        //bartlett-hanning window impulse resp
		break;

		case fwt_hamming:
            w[ i ] = 0.54 - 0.46 * cos( twopi * n / ( N - 1.0 ) );            //hamming window impulse resp, note the text in  http://www.mikroe.com.. listed above is slightly incorrect, used wikipedia version
		break;

		case fwt_blackman:
            w[ i ] = 0.42 - 0.5 * cos( twopi * n / ( N - 1.0 ) ) + 0.08 * cos( 2.0 * twopi * n / ( N - 1.0 ) );        //blackman window impulse resp
		break;

		case fwt_blackman_harris:
            w[ i ] = 0.35875 - 0.48829 * cos( twopi * n / ( N - 1.0 ) ) + 0.14128 * cos( 2.0 * twopi * n / ( N - 1.0 ) ) - 0.01168 * cos( 3.0 * twopi * n / ( N - 1.0 ) );        //blackman-harris window impulse resp
		break;

		default:
			printf( "filter_fir_windowed() - unknown filter window type(wnd): %u\n", wnd_type );
			return 0;
		break;
        }


	switch( filt_type )
		{
		case fpt_lowpass:

			//ideal lowpass impulse response
			if( (int)n != (int)M ) hd[ i ] = ( ( sin( wc1 * ( n - M ) ) ) / (  M_PI * ( n - M ) ) );        
			else hd[ i ] = wc1 / M_PI;
		break;

		case fpt_highpass:

			//ideal highpass impulse response
			if( (int)n != (int)M ) hd[ i ] = -( ( sin( wc1 * ( n - M ) ) ) / (  M_PI * ( n - M ) ) );        
			else hd[ i ] = 1.0 - wc1 / M_PI;
		break;


		case fpt_bandpass:

			//ideal bandpass impulse response
			if( (int)n != (int)M ) hd[ i ] =   ( sin( wc2 * ( n - M ) )  /  (  M_PI * ( n - M ) ) )   -   ( sin( wc1 * ( n - M ) ) /  (  M_PI * ( n - M ) ) );        
			else hd[ i ] = ( wc2 - wc1 ) / M_PI;
		break;


		case fpt_notch:

			//ideal notch impulse response
			if( (int)n != (int)M ) hd[ i ] =   ( sin( wc1 * ( n - M ) )  /  (  M_PI * ( n - M ) ) )   -   ( sin( wc2 * ( n - M ) ) /  (  M_PI * ( n - M ) ) );        
			else hd[ i ] = 1.0 - ( wc1 - wc2 ) / M_PI;
		break;
		
		default:
			printf( "filter_fir_windowed() - unknown filter type: %u\n", filt_type );
			return 0;
		break;
		}

    hcoeff[ i ] = w[ i ] * hd[ i ];             //window * impulse response
//	printf( "filter_fir_windowed() - w[%d]: %g, hd[]: %g, h[]: %g\n", (int)n, w[ i ], hd[ i ], hcoeff[ i ] );

//printf( "filter_fir_windowed() - w[%d]: %g, hd[]: %g, h[]: %g\n", (int)n, w[ i ], hd[ i ], hcoeff[ i ] );
 
    vcoeff.push_back( hcoeff[ i ] );
	}



delete w;
delete hd;
delete hcoeff;


return 1;
}










//build a filter from a string loaded with coeffs, allocates memory so it must be deleted
// coeff should be seperated by cr\lf
bool create_filter_from_coeffs( st_fir &fir, vector<double> &vcoeff )
{

if( fir.created ) delete_filter( fir );

if( vcoeff.size() == 0 ) return 0;

fir.coeff_ptr = new double [ vcoeff.size() ];				//alloc space
fir.prev = new double [ vcoeff.size() ];



for( int i = 0; i < vcoeff.size(); i++ )				//load coeffs
	{
	fir.coeff_ptr[ i ] = vcoeff[ i ];
	}

fir.coeff_cnt = vcoeff.size();
fir.created = 1;
//fir.bypass = 0;

fir_init( fir );									//clear fir buf

return 1;
}







//delete filter and free any memory allocated
void delete_filter( st_fir &fir )
{
if ( fir.created == 0 ) return;

fir.created = 0;

delete fir.coeff_ptr;
delete fir.prev;

}














void fir_init( st_fir &fir )
{
if( fir.created == 0 ) return;

fir.prev_idx = 0;

for( int i = 0; i < fir.coeff_cnt; i++ ) fir.prev[ i ] = 0;
}









void fir_in( st_fir &fir, double in )
{
if( fir.created == 0 ) return;

fir.prev[ fir.prev_idx++ ] = in;
if( fir.prev_idx == fir.coeff_cnt ) fir.prev_idx = 0;

}








double fir_out( st_fir &fir )
{
if( fir.created == 0 ) return 0;

int idx = fir.prev_idx;

if ( fir.bypass )							//bypass, just o/p last fir_in value
	{
	return fir.prev[ idx ];
	}

double sum = 0;


for( int i = 0; i < fir.coeff_cnt; i++ )
	{
	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;

	sum += fir.prev[ idx ] * fir.coeff_ptr[ i ];
	};

return sum;
}



/*

//some in-line code to speed things up, coeff count must be a factor of 4
double fir_out_inline_4( st_fir &fir )
{
if( fir.created == 0 ) return 0;

int idx = fir.prev_idx;

if ( fir.bypass )							//bypass, just o/p last fir_in value
	{
	return fir.prev[ idx ];
	}

double sum = 0;


//for( int i = 0; i < fir.coeff_cnt; i++ )
//	{
//	if( idx != 0 ) idx--;
//	else idx = fir.coeff_cnt - 1;

//	sum += fir.prev[ idx ] * fir.coeff_ptr[ i ];
//	};


//int cnt = fir.coeff_cnt / 4;

for( int i = 0; i < fir.coeff_cnt; i++ )
	{
	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i ];
	};


return sum;
}
*/








/*
//some in-line code to speed things up, coeff count must be a factor of 8
double fir_out_inline_8( st_fir &fir )
{
if( fir.created == 0 ) return 0;

int idx = fir.prev_idx;

if ( fir.bypass )							//bypass, just o/p last fir_in value
	{
	return fir.prev[ idx ];
	}

double sum = 0;


//for( int i = 0; i < fir.coeff_cnt; i++ )
//	{
//	if( idx != 0 ) idx--;
//	else idx = fir.coeff_cnt - 1;

//	sum += fir.prev[ idx ] * fir.coeff_ptr[ i ];

//	};


//int cnt = fir.coeff_cnt / 8;

for( int i = 0; i < fir.coeff_cnt; )
	{
	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];


	if( idx != 0 ) idx--;
	else idx = fir.coeff_cnt - 1;
	sum += fir.prev[ idx ] * fir.coeff_ptr[ i++ ];

	}


return sum;
}
*/






void iir_init( st_iir &iir )
{
if( iir.created == 0 ) return;

iir.prev_idx = 0;

for( int i = 0; i < iir.coeff_cnt; i++ ) iir.prev[ i ] = 0;
}





//delete filter and free any memory allocated
void iir_delete_filter( st_iir &iir )
{
if ( iir.created == 0 ) return;

iir.created = 0;

delete iir.coeff_ptr;
delete iir.prev;
}





//build a filter from a vector loaded with coeffs, allocates memory so it must be deleted
bool create_iir_filter_from_coeffs( st_iir &iir, vector<double> &vcoeff )
{

if( iir.created ) iir_delete_filter( iir );

if( vcoeff.size() == 0 ) return 0;

iir.coeff_ptr = new double [ vcoeff.size() ];				//alloc space
iir.prev = new double [ vcoeff.size() ];



for( int i = 0; i < vcoeff.size(); i++ )				//load coeffs
	{
	iir.coeff_ptr[ i ] = vcoeff[ i ];
	}

iir.coeff_cnt = vcoeff.size();
iir.created = 1;
//fir.bypass = 0;

iir_init( iir );									//clear fir buf

return 1;
}











//build a filter from file, allocates memory so it must be deleted
bool create_filter_from_file( st_fir &fir, string fname )
{
mystr m1, m2;
string s1;
double dd;
vector<double> vd;

if( fir.created ) delete_filter( fir );


if( m1.readfile( fname, 10000 ) == 0 )
	{
	cslpf( "create_filter_from_file() - failed to open file: '%s'\n", fname.c_str()  );
	}

s1 = m1.str();
s1 += "\r\n";							//ensure ending newline
m1 = s1;

while( 1 )
	{
	if( m1.gets( s1 ) == 0 ) break;

	m2 = s1;
	m2.cut_at_first_find( s1, "//", 0 );		// cut off any comments

	m2 = s1;
	m2.to_lower( s1 );								//change to lowercase

	if( s1.find( "end" ) != string::npos ) break;	//end found on this line?

		
	int ret = sscanf( s1.c_str(), "%lf", &dd );

	if( ( ret != 0 ) && ( ret != EOF ) )		//conv to double
		{
		vd.push_back( dd );
		}
	}


if( vd.size() == 0 )
	{
	cslpf( "create_filter_from_file() - no coeff read: '%s'\n", fname.c_str()  );
	return 0;
	}


cslpf( "create_filter_from_file() - %d filter coeffs read from: '%s'\n", vd.size(), fname.c_str()  );

if( vd.size() >=  10000 )
	{
	cslpf( "create_filter_from_file() - too many filter coeffs (%d) read from: '%s'\n", vd.size(), fname.c_str()  );
	return 0;
	}

fir.coeff_ptr = new double [ vd.size() ];				//alloc space
fir.prev = new double [ vd.size() ];



for( int i = 0; i < vd.size(); i++ )				//loed coeffs
	{
	fir.coeff_ptr[ i ] = vd[ i ];

//	cslpf( "coeff %d: %g\n", i, vd[ i ] );
	}

fir.coeff_cnt = vd.size();
fir.created = 1;
//fir.bypass = 0;

fir_init( fir );									//clear fir buf

return 1;
}












//build a filter from a string loaded with coeffs, allocates memory so it must be deleted
// coeff should be seperated by cr\lf
bool create_filter_from_string( st_fir &fir, string scoeff )
{
mystr m1, m2;
string s1;
double dd;
vector<double> vd;

if( fir.created ) delete_filter( fir );



s1 = scoeff;
s1 += "\r\n";							//ensure ending newline
m1 = s1;

while( 1 )
	{
	if( m1.gets( s1 ) == 0 ) break;

	m2 = s1;
	m2.cut_at_first_find( s1, "//", 0 );		// cut off any comments

	m2 = s1;
	m2.to_lower( s1 );								//change to lowercase

	if( s1.find( "end" ) != string::npos ) break;	//end found on this line?

		
	int ret = sscanf( s1.c_str(), "%lf", &dd );

	if( ( ret != 0 ) && ( ret != EOF ) )		//conv to double
		{
		vd.push_back( dd );
		}
	}


if( vd.size() == 0 )
	{
	cslpf( "create_filter_from_string() - no coeff read\n"  );
	return 0;
	}


cslpf( "create_filter_from_string() - %d filter coeffs read\n", vd.size() );

if( vd.size() >=  10000 )
	{
	cslpf( "create_filter_from_string() - too many filter coeffs (%d) read \n", vd.size() );
	return 0;
	}

fir.coeff_ptr = new double [ vd.size() ];				//alloc space
fir.prev = new double [ vd.size() ];



for( int i = 0; i < vd.size(); i++ )				//load coeffs
	{
	fir.coeff_ptr[ i ] = vd[ i ];

//	cslpf( "coeff %d: %g\n", i, vd[ i ] );
	}

fir.coeff_cnt = vd.size();
fir.created = 1;
//fir.bypass = 0;

fir_init( fir );									//clear fir buf

return 1;
}










