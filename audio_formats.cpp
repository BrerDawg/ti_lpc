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

//audio_formats.cpp
//v1.01	---- 20-aug-2015                //rudimentary support for some popular audio file formats
//v1.02	---- 02-apr-2016				//change 'bool' to 'int' return val for load(), which shows up various failures
                                        //fixed memory leaks and occasional failure to close a file if memory alloc failed

//v1.03	---- 03-jun-2016				//changed 'int need' to 'unsigned int need', this sorted problems with load_sun() if 'data_block_size' in header is -1 
//v1.04	---- 14-sep-2016				//used nearbyint() when saving to allow correct rounding when converting doubles to integers, was only noticed
										//with 'baudline' plotter, it was showing some low level aliases, which were due to rounding errors, nearbyint() fixed this
										//--also added malloc storage methods to avoid limits of vector based var
										//--see cn_malloc_max_size 256 * 1024 * 1024 + 1  it puts a limit on how big realloc storage
										//--can grow (space for at least 33554432 doubles[assuming 8 bytes per double], @48KHz amounts to ~699 secs [11 mins:39 secs] )


//v1.05	---- 05-jun-2017				//added: fir_filter(), copypush_chx()
//v1.06	---- 15-jul-2018				//minor mod to: 'audio_formats.h' for 64 bit 
//v1.07	---- 05-sep-2018				//added fp methods , see rem'd out code EXAMPLE: 'test_audio_format_fp()'
//v1.08	---- 23-feb-2019				//in 'logpf()'	-- changed 'delete buf;'  to 'delete[] buf;'  -- discovered with valgrind,
										//also see load_malloc() v1.08, where pch0, pch1 a freed and then realloc

#include "audio_formats.h"



//be aware vector<double> vars are used here, and there is a limit to how big these can grow, so large audio files used to cause a hard to
//find runtime error, now have modifed with a 'std_vector_limit' to impose a limit on these vectors,
//intead try 'malloc storage methods' 



/* usage......

//---------------------------------------------------------------------------

//testing the malloc methods (alternative to vector based methods)
//reads a wav file, adj its levels and saves as a 8 bit .au,
//then creates an aiff file containing 2 tones
void test_malloc()
{
audio_formats af_wav, af_sun, af_aiff;
st_audio_formats_tag saf_wav, saf_sun, saf_aiff;


af_wav.verb = 1;
af_wav.log_to_printf = 1;

af_sun.verb = 1;
af_sun.log_to_printf = 1;

af_aiff.verb = 1;
af_aiff.log_to_printf = 1;

//wav
saf_wav.format = en_af_wav_pcm;
af_wav.load_malloc( "", "zjunk.wav", 32767, saf_wav );

for( int i = 0; i < af_wav.sizech0; i++ )
	{
	double dd;

	dd = af_wav.pch0[ i ];					//note use of malloc ptr within af obj
	af_sun.push_ch0( 0.5 * dd );			//note use of malloc based function call

	if( saf_wav.channels == 2 )
		{
		dd = af_wav.pch1[ i ];
		af_sun.push_ch1( 0.5 * dd );
		}
	}

//au
saf_sun.format = en_af_sun;
saf_sun.srate = saf_wav.srate;
saf_sun.encoding = 1;
saf_sun.channels = saf_wav.channels;

af_sun.save_malloc( "", "zjunk.au", 32767, saf_sun );

af_sun.clear_ch0();							//just an example of how to clear, not really req here
af_sun.clear_ch1();


//aiff
saf_aiff.format = en_af_aiff;
saf_aiff.srate = 48000;									//change srate
saf_aiff.channels = 2;


double t = 0;

double tinc = 1.0 / saf_aiff.srate;

for( int i = 0; i < 10 * saf_aiff.srate; i++ )					//make an aiff tone file
	{
	double dd;

	dd = sin( 2.0 * M_PI * 200.0 * t );					//tone ch1
	af_aiff.push_ch0( 0.99 * dd );

	dd = sin( 2.0 * M_PI * 400.0 * t );					//tone ch2
	af_aiff.push_ch1( 0.99 * dd );

	t += tinc;
	}

af_aiff.save_malloc( "", "ztone.aiff", 32767, saf_aiff );

}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//testing the vector based methods  (alternative to malloc methods)
void test_audio_formats()
{
audio_formats af;
st_audio_formats_tag saf;



af.verb = 1;                                //store progress in: string 'log'
af.log_to_printf = 1;                       //show to console aswell

saf.format = en_af_sun;

//af.load( "", "testing_left_right_stereo_48000_16_le.wav", 0, saf );
af.load( "", "testing_left_right_stereo_48000.au", 0, saf );

saf.encoding = 1;
saf.channels = 1;
af.save( "", "testing_left_right_stereo_48000_ulaw.au", 32767, saf );


//saf.format = en_af_wav_pcm;
//af.load( "", "testing_left_right_stereo_48000.wav", 0, saf );


//af.load( "", "madge.wav", 0, saf );

saf.format = en_af_wav_pcm;
saf.channels = 2;
af.save( "", "junk.wav", 32767, saf );

//saf.format = en_af_sun;
//saf.srate = 48000;
//saf.encoding = 1;
//saf.channels = 2;
//saf.is_big_endian = 0;
//af.save( "", "junk.au", 32767, saf );

//logpf("here\n" );
getchar();
}

//---------------------------------------------------------------------------
*/










audio_formats::audio_formats()
{
verb = 0;
log_to_printf = 0;


allocsz_ch0 = cn_malloc_granularity;				//set up for malloc operations
allocsz_ch1 = cn_malloc_granularity;

sizech0 = 0;
sizech1 = 0;

pch0 = (double*)malloc( sizeof(double) * allocsz_ch0 );
pch1 = (double*)malloc( sizeof(double) * allocsz_ch1 );


fp_audio_format = en_af_unknown;
fp_channels = 0;
fp_srate = 0;
fp_bytes_written = 0;
fp_offset = 0;
fp_encoding = 0;
fp_is_big_endian = 0;

fileptr = 0;
file_size = 0;
buf_size = 0;
ucbf = 0;
bf0 = 0;
bf1 = 0;
f_bf0 = 0;
f_bf1 = 0;


if( pch0 == 0 )
	{
	printf( "audio_formats::audio_formats() - unable to malloc memory for pch0 !!!!!!!\n" );
	logpf( "audio_formats::audio_formats() - unable to malloc memory for pch0 !!!!!!!\n" );
	allocsz_ch0 = 0;
	}


if( pch1 == 0 )
	{
	printf( "audio_formats::audio_formats() - unable to malloc memory for pch1 !!!!!!!\n" );
	logpf( "audio_formats::audio_formats() - unable to malloc memory for pch1 !!!!!!!\n" );
	allocsz_ch1 = 0;
	}

}








audio_formats::~audio_formats()
{
//printf ("audio_formats::~audio_formats() - freeing()\n" );

if( allocsz_ch0 != 0 ) free( pch0 );
if( allocsz_ch1 != 0 ) free( pch1 );

fp_write_close_free_mem();
}








//push an audio sample using malloc methods
bool audio_formats::push_ch0( double dd )
{

if( ( sizech0 * sizeof(double) ) >= allocsz_ch0 )								//at limit of allocation?
	{
	unsigned int sizenew = allocsz_ch0 + cn_malloc_granularity;
	
	if( sizenew >= cn_malloc_max_size )
		{
		printf( "audio_formats::push_ch0() - memory limit reached for pch0, limit: %u !!!!!!!\n", cn_malloc_max_size );
		logpf( "audio_formats::push_ch0() - memory limit reached for pch0, limit: %u !!!!!!!\n", cn_malloc_max_size );
		return 0;
		}

//asm("int3");						//useful to trigger debugger

	double *dp;
	dp = (double*)realloc( pch0, sizenew );

	if( dp == 0 )
		{
		printf( "audio_formats::push_ch0() - unable to realloc memory for pch0, size: %u !!!!!!!\n", sizenew );
		logpf( "audio_formats::push_ch0() - unable to realloc memory for pch0, size: %u !!!!!!!\n", sizenew );
		return 0;
		}

	pch0 = dp;							//new memory allocated, adj as req
	allocsz_ch0 = sizenew;
//printf("audio_formats::push_ch0() - sizech0: %u, allocsz_ch0: %u (%.3f MB)\n", sizech0, allocsz_ch0, (double)allocsz_ch0 / 1048576 );
	}

pch0[ sizech0++ ] = dd;					//store new sample
return 1;
}










//push an audio sample using malloc methods
bool audio_formats::push_ch1( double dd )
{

if( ( sizech1 * sizeof(double) ) >= allocsz_ch1 )								//at limit of allocation?
	{
	unsigned int sizenew = allocsz_ch1 + cn_malloc_granularity;
	
	if( sizenew >= cn_malloc_max_size )
		{
		printf( "audio_formats::push_ch1() - memory limit reached for pch1, limit: %u !!!!!!!\n", cn_malloc_max_size );
		logpf( "audio_formats::push_ch1() - memory limit reached for pch1, limit: %u !!!!!!!\n", cn_malloc_max_size );
		return 0;
		}

	double *dp;
	dp = (double*)realloc( pch1, sizenew );

	if( dp == 0 )
		{
		printf( "audio_formats::push_ch1() - unable to realloc memory for pch1, size: %u !!!!!!!\n", sizenew );
		logpf( "audio_formats::push_ch1() - unable to realloc memory for pch1, size: %u !!!!!!!\n", sizenew );
		return 0;
		}

	pch1 = dp;							//new memory allocated, adj as req
	allocsz_ch1 = sizenew;
//printf("audio_formats::push_ch1() - sizech1: %u, allocsz_ch0: %u (%.3f MB)\n", sizech1, allocsz_ch1, (double)allocsz_ch1 / 1048576 );
	}

pch1[ sizech1++ ] = dd;					//store new sample
return 1;
}










//clear audio samples using malloc methods
bool audio_formats::clear_ch0()
{

if( allocsz_ch0 != 0 ) free( pch0 );

sizech0 = 0;

allocsz_ch0 = cn_malloc_granularity;
pch0 = (double*)malloc( sizeof(double) * allocsz_ch0 );


if( pch0 == 0 )
	{
	printf( "audio_formats::clear_ch0() - unable to malloc memory for pch0 !!!!!!!\n" );
	logpf( "audio_formats::clear_ch0() - unable to malloc memory for pch0 !!!!!!!\n" );
	allocsz_ch0 = 0;
	return 0;
	}

return 1;
}









//clear audio samples using malloc methods
bool audio_formats::clear_ch1()
{

if( allocsz_ch1 != 0 ) free( pch1 );

sizech1 = 0;

allocsz_ch1 = cn_malloc_granularity;
pch1 = (double*)malloc( sizeof(double) * allocsz_ch1 );

if( pch1 == 0 )
	{
	printf( "audio_formats::clear_ch1() - unable to malloc memory for pch1 !!!!!!!\n" );
	logpf( "audio_formats::clear_ch1() - unable to malloc memory for pch1 !!!!!!!\n" );
	allocsz_ch1 = 0;
	return 0;
	}


return 1;
}











//for logging/error
void audio_formats::logpf( const char *fmt,... )
{
string s1;


int buf_size =  1 * 1024 * 1024;
char* buf = new char[ buf_size ];				//create a data buffer


if( buf == 0 )
	{
	printf( "audio_formats::logpf() - no memory allocated!!!!!!!!!!!!!!!!!!!!!!!\n" );
	return;
	}

va_list ap;

va_start( ap, fmt );
vsnprintf( buf, buf_size - 1, fmt, ap );

log += buf;
if ( log_to_printf ) printf( "%s", buf );

delete[] buf;
}









//if 'scale_down_by_peak_int_val' is non zero then nomalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
int audio_formats::load( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
bool known = 0;
int ret = 0;

log = "";
if( af.format == en_af_raw_pcm )
    {
    ret = load_raw_pcm( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_raw_single_prec )
    {
    ret = load_raw_single_prec( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_raw_double_prec )
    {
    ret = load_raw_double_prec( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_wav_pcm )
    {
    ret = load_wav( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }

if( af.format == en_af_sun )
    {
    ret = load_sun( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }

if( af.format == en_af_aiff )
    {
    ret = load_aiff( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }

if( !known) if( verb ) logpf( "audio_formats::load() - unkown format was specified for filename: '%s'\n", fname.c_str() );
return ret;
}







//if 'scale_down_by_peak_int_val' is non zero then nomalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
int audio_formats::load_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
bool known = 0;
int ret = 0;

if( allocsz_ch0 != 0 ) free( pch0 );								//v1.08
if( allocsz_ch1 != 0 ) free( pch1 );								//v1.08

allocsz_ch0 = cn_malloc_granularity;								//v1.08, set up for malloc operations
allocsz_ch1 = cn_malloc_granularity;								//v1.08, set up for malloc operations

sizech0 = 0;														//v1.08
sizech1 = 0;														//v1.08

pch0 = (double*)malloc( sizeof(double) * allocsz_ch0 );				//v1.08
pch1 = (double*)malloc( sizeof(double) * allocsz_ch1 );				//v1.08


log = "";
if( af.format == en_af_raw_pcm )
    {
    ret = load_raw_pcm_malloc( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_raw_single_prec )
    {
    ret = load_raw_single_prec_malloc( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_raw_double_prec )
    {
    ret = load_raw_double_prec_malloc( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_wav_pcm )
    {
    ret = load_wav_malloc( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }

if( af.format == en_af_sun )
    {
    ret = load_sun_malloc( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }

if( af.format == en_af_aiff )
    {
    ret = load_aiff_malloc( path, fname, scale_down_by_peak_int_val, af );
    known = 1;
    }

if( !known) if( verb ) logpf( "audio_formats::load() - unkown format was specified for filename: '%s'\n", fname.c_str() );
return ret;
}











//samples must be between +/- 1.0, saved samples are scaled up by peak_int_val
bool audio_formats::save( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
bool known = 0;
bool ret = 0;

log = "";
if( af.format == en_af_raw_pcm )
    {
    ret = save_raw( path, fname, peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_wav_pcm )
    {
    ret = save_wav( path, fname, peak_int_val, af );
    known = 1;
    }

if( af.format == en_af_sun )
    {
    ret = save_sun( path, fname, peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_aiff )
    {
    ret = save_aiff( path, fname, peak_int_val, af );
    known = 1;
    }


if( !known) if( verb ) logpf( "audio_formats::save() - unkown format was specified for filename: '%s'\n", fname.c_str() );
return ret;
}









//samples must be between +/- 1.0, saved samples are scaled up by peak_int_val
bool audio_formats::save_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
bool known = 0;
bool ret = 0;

log = "";
if( af.format == en_af_raw_pcm )
    {
    ret = save_raw_malloc( path, fname, peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_wav_pcm )
    {
    ret = save_wav_malloc( path, fname, peak_int_val, af );
    known = 1;
    }

if( af.format == en_af_sun )
    {
    ret = save_sun_malloc( path, fname, peak_int_val, af );
    known = 1;
    }


if( af.format == en_af_aiff )
    {
    ret = save_aiff_malloc( path, fname, peak_int_val, af );
    known = 1;
    }


if( !known) if( verb ) logpf( "audio_formats::save() - unkown format was specified for filename: '%s'\n", fname.c_str() );
return ret;
}










//loads supplied flts buffer with audio samples, if 2 channels avail will interleave them,
//if sample_cnt_to_load = 2 and 'af' held 2 channels, then one sample from ch0 and ch1 would be put into 'fltsbuf'
bool audio_formats::sample_buf_load_flts( st_audio_formats_tag &af, flts* fltsbuf, unsigned int sample_cnt_to_load, unsigned int &actually_loaded )
{
double dd;

actually_loaded = 0;

if( sample_cnt_to_load == 0 ) return 1;

unsigned int cnt0 = af.vch0.size();
unsigned int cnt1 = af.vch1.size();

unsigned int cnt;

if ( cnt0 >= cnt1 ) cnt = cnt0;
else cnt = cnt1;

if( cnt == 0 ) return 1;


bool trk2 = 0;
int j = 0;
for( unsigned int i = 0; i < sample_cnt_to_load; i++ )
    {
    if( !trk2 )
        {
        if( j < cnt0 ) dd = af.vch0[ j ];
        else dd = 0;
        }
    else{
        if( j < cnt1 ) dd = af.vch1[ j ];
        else dd = 0;
        j++;
        }
    
    fltsbuf[ i ] = dd;
    
    if( af.channels == 2 )
        {
        trk2 = !trk2;
        }
    else{
        j++;
        }
    actually_loaded++;
    }
//printf( "sample_buf_fill_flts() - loaded: %u\n", cnt );

return 1;
}















//loads supplied flts buffer with audio samples using malloc double pointers: pch0, pch1, if 2 channels avail will interleave them,
//if sample_cnt_to_load = 2 and 'af' held 2 channels, then one sample from ch0 and ch1 would be put into 'fltsbuf'
bool audio_formats::sample_buf_load_flts_malloc( st_audio_formats_tag &af, flts* fltsbuf, unsigned int sample_cnt_to_load, unsigned int &actually_loaded )
{
double dd;

actually_loaded = 0;

if( sample_cnt_to_load == 0 ) return 1;

unsigned int cnt0 = sizech0;
unsigned int cnt1 = sizech1;

unsigned int cnt;

if ( cnt0 >= cnt1 ) cnt = cnt0;
else cnt = cnt1;

if( cnt == 0 ) return 1;


bool trk2 = 0;
int j = 0;
for( unsigned int i = 0; i < sample_cnt_to_load; i++ )
    {
    if( !trk2 )
        {
        if( j < cnt0 ) dd = pch0[ j ];
        else dd = 0;
        }
    else{
        if( j < cnt1 ) dd = pch1[ j ];
        else dd = 0;
        j++;
        }
    
    fltsbuf[ i ] = dd;
    
    if( af.channels == 2 )
        {
        trk2 = !trk2;
        }
    else{
        j++;
        }
    actually_loaded++;
    }
//printf( "sample_buf_fill_flts() - loaded: %u\n", cnt );

return 1;
}



















//load a AIFF formatted audio file (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 16 bit pcm only, 1 or 2 channel

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_aiff( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;
double min = 0;                                     //for normalising audio
double max = 0;
vector<double> vaud;
vector<double> vch0;
vector<double> vch1;

string fname1 = path;
fname1 += fname;


af.vch0.clear();
af.vch1.clear();



unsigned long long int fsz;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_aiff() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_aiff() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_aiff() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }






FILE *fp;

fp = m1.mbc_fopen( fname1, "rb" );		    	//open file


if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff() - failed to open file for reading: '%s'\n", fname1.c_str() );
	return -1;
	}

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_aiff() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }

unsigned int need = 54;

int read = fread( buf, 1 ,need, fp );             //read aiff audio file chunks etc...

if( read != need )
    {
    free( buf );
    fclose( fp );
    if( verb ) logpf( "audio_formats::load_aiff() - sun's au audio header not big enough for file: '%s', only read: %u\n", fname1.c_str(), read );
    return -4;
    }




if( strncmp( (char*)( buf + cn_riff_fmt_chunk_ckid ), "FORM", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff() - no 'FORM ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


unsigned int form_cksize;
form_cksize = buf[ cn_aiff_form_cksize ] << 24;
form_cksize += buf[ cn_aiff_form_cksize + 1 ] << 16;
form_cksize += buf[ cn_aiff_form_cksize + 2 ] << 8;
form_cksize += buf[ cn_aiff_form_cksize + 3 ];


if( strncmp( (char*)( buf + cn_aiff_aiff_ckid ), "AIFF", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff() - no 'AIFF ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


if( strncmp( (char*)( buf + cn_aiff_comm_ckid ), "COMM", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff() - no 'COMM ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


unsigned int comm_cksize;
comm_cksize = buf[ cn_aiff_comm_cksize ] << 24;
comm_cksize += buf[ cn_aiff_comm_cksize + 1 ] << 16;
comm_cksize += buf[ cn_aiff_comm_cksize + 2 ] << 8;
comm_cksize += buf[ cn_aiff_comm_cksize + 3 ];


unsigned int channels;
channels = buf[ cn_aiff_comm_channels ] << 8;
channels += buf[ cn_aiff_comm_channels + 1 ];


unsigned int sample_frames;
sample_frames = buf[ cn_aiff_comm_cksize ] << 24;
sample_frames += buf[ cn_aiff_comm_cksize + 1 ] << 16;
sample_frames + buf[ cn_aiff_comm_sample_frames + 2 ] << 8;
sample_frames += buf[ cn_aiff_comm_sample_frames + 3 ];


unsigned int bits_per_sample;
bits_per_sample = buf[ cn_aiff_comm_sample_size ] << 8;
bits_per_sample += buf[ cn_aiff_comm_sample_size + 1 ];




unsigned char buf_srate[ 10 ];

buf_srate[ 9 ] = buf[ cn_aiff_comm_sample_srate ];          //read in big endian 80 bit IEEE 754 extended precision floating point number (long double)
buf_srate[ 8 ] = buf[ cn_aiff_comm_sample_srate + 1 ];      //convert to little endian
buf_srate[ 7 ] = buf[ cn_aiff_comm_sample_srate + 2 ];
buf_srate[ 6 ] = buf[ cn_aiff_comm_sample_srate + 3 ];
buf_srate[ 5 ] = buf[ cn_aiff_comm_sample_srate + 4 ];
buf_srate[ 4 ] = buf[ cn_aiff_comm_sample_srate + 5 ];
buf_srate[ 3 ] = buf[ cn_aiff_comm_sample_srate + 6 ];
buf_srate[ 2 ] = buf[ cn_aiff_comm_sample_srate + 7 ];
buf_srate[ 1 ] = buf[ cn_aiff_comm_sample_srate + 8 ];
buf_srate[ 0 ] = buf[ cn_aiff_comm_sample_srate + 9 ];

long double ld = *(long double*)buf_srate;

unsigned int srate = ld;                                   //convert srate to int




if( strncmp( (char*)( buf + cn_aiff_comm_ssnd_ckid ), "SSND", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff() - no 'SSND ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}



unsigned int ssnd_cksize;
ssnd_cksize = buf[ cn_aiff_comm_ssnd_cksize ] << 24;
ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 1 ] << 16;
ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 2 ] << 8;
ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 3 ];



unsigned int offset;
offset = buf[ cn_aiff_comm_ssnd_offset ] << 24;
offset += buf[ cn_aiff_comm_ssnd_offset + 1 ] << 16;
offset += buf[ cn_aiff_comm_ssnd_offset + 2 ] << 8;
offset += buf[ cn_aiff_comm_ssnd_offset + 3 ];



unsigned int align_byte_cnt ;
align_byte_cnt = buf[ cn_aiff_comm_ssnd_block_size ] << 24;
align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 1 ] << 16;
align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 2 ] << 8;
align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 3 ];








/*
FILE *fp2;

s1 = "zzieee.bin";

fp2 = m1.mbc_fopen( s1, "wb" );		    	//open file
fwrite( buf_srate, 1, sizeof(buf_srate), fp2 );          //write block of audio data
fclose( fp2 );
*/

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_aiff() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    fclose( fp );
    return -6;
    }


if( bits_per_sample != 16 )
    {
    if( verb ) logpf( "audio_formats::load_aiff() - for filename: '%s', bets_per_sample is not supported: %u\n", fname1.c_str(), bits_per_sample );
    fclose( fp );
    return -5;
    }




af.path = path;
af.fname = fname1;
af.srate = srate;
af.offset = offset;
af.channels = channels;
af.is_big_endian = 1;


if( verb ) logpf( "audio_formats::load_aiff() - samplerate: %u, for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::load_aiff() - channels: %u found in file: '%s'\n", af.channels, fname1.c_str() );


//fclose( fp );
//return 1;

int skip = offset - 4 * 6;

if ( skip < 0 ) skip = 0;
for( int i = 0; i < skip; i++ )					    //step to first sample byte
    {
    int read = fread( buf, 1, 1, fp );
    }


unsigned int byte_cnt = ssnd_cksize - 8;            //data block size is less the unsigned long 'cn_aiff_comm_ssnd_offset and 'cn_aiff_comm_ssnd_block_size'
unsigned int cnt = 0;
unsigned int min_sample_idx;
unsigned int max_sample_idx;

//	if( verb ) logpf( "audio_formats::load_wav() - byte_cnt: %u\n", byte_cnt );

//logpf("byte_cnt: %u\n", byte_cnt );

double dd;
bool trk2 = 0;
char cc;
int iaud;
int i = 0;
while( 1 )
	{
    int read;

	need = byte_cnt - cnt;
	if( need > cn_audio_formats_buf_siz ) need = cn_audio_formats_buf_siz;
	
    read = fread( buf, 1, need, fp );          				//read block of audio data
	if ( read == 0 ) break;
	cnt += read;
	
	if( read != need )
		{
		if( verb ) logpf( "audio_formats::load_aiff() - tried to read: %u bytes, only got %u bytes for file: '%s'\n", need, read, fname1.c_str() );
		}


    for( int i = 0; i < read; )                 //process block
        {

        dd = (char)buf[ i ] * 256.0;        //preserve sign
        i++;
        if( i >= read )
            {
            if( verb ) logpf( "audio_formats::load_aiff() - odd number of bytes encountered, lost final sample\n" );
            break;
            }
        dd += buf[ i ];
        i++;



        if( channels == 1 )                     //1 trk?
            {
            vch0.push_back( dd );
            }
        else{
            if( trk2 == 0 ) vch0.push_back( dd );
            else vch1.push_back( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
        }
	}


if( verb )
	{
	logpf( "audio_formats::load_aiff() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_aiff() - sample: %06d has max: %g\n", max_sample_idx, max );
	}




double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_aiff() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_aiff() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < vch0.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    af.vch0.push_back( dd );
    }


for( int i = 0; i < vch1.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    af.vch1.push_back( dd );
    }

if( verb ) logpf( "audio_formats::load_aiff() - samples read (ch1: %u), (ch2: %u) from '%s'\n", af.vch0.size(), af.vch1.size(), fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_aiff() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_aiff() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}


















//load a AIFF formatted audio file using malloc double pointers: pch0, pch1 (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 16 bit pcm only, 1 or 2 channel

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_aiff_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;
double min = 0;                                     //for normalising audio
double max = 0;

string fname1 = path;
fname1 += fname;


clear_ch0();
clear_ch1();



unsigned long long int fsz;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }






FILE *fp;

fp = m1.mbc_fopen( fname1, "rb" );		    	//open file


if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff_malloc() - failed to open file for reading: '%s'\n", fname1.c_str() );
	return -1;
	}

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }

unsigned int need = 54;

int read = fread( buf, 1 ,need, fp );             //read aiff audio file chunks etc...

if( read != need )
    {
    free( buf );
    fclose( fp );
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - sun's au audio header not big enough for file: '%s', only read: %u\n", fname1.c_str(), read );
    return -4;
    }




if( strncmp( (char*)( buf + cn_riff_fmt_chunk_ckid ), "FORM", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff_malloc() - no 'FORM ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


unsigned int form_cksize;
form_cksize = buf[ cn_aiff_form_cksize ] << 24;
form_cksize += buf[ cn_aiff_form_cksize + 1 ] << 16;
form_cksize += buf[ cn_aiff_form_cksize + 2 ] << 8;
form_cksize += buf[ cn_aiff_form_cksize + 3 ];


if( strncmp( (char*)( buf + cn_aiff_aiff_ckid ), "AIFF", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff_malloc() - no 'AIFF ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


if( strncmp( (char*)( buf + cn_aiff_comm_ckid ), "COMM", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff_malloc() - no 'COMM ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


unsigned int comm_cksize;
comm_cksize = buf[ cn_aiff_comm_cksize ] << 24;
comm_cksize += buf[ cn_aiff_comm_cksize + 1 ] << 16;
comm_cksize += buf[ cn_aiff_comm_cksize + 2 ] << 8;
comm_cksize += buf[ cn_aiff_comm_cksize + 3 ];


unsigned int channels;
channels = buf[ cn_aiff_comm_channels ] << 8;
channels += buf[ cn_aiff_comm_channels + 1 ];


unsigned int sample_frames;
sample_frames = buf[ cn_aiff_comm_cksize ] << 24;
sample_frames += buf[ cn_aiff_comm_cksize + 1 ] << 16;
sample_frames + buf[ cn_aiff_comm_sample_frames + 2 ] << 8;
sample_frames += buf[ cn_aiff_comm_sample_frames + 3 ];


unsigned int bits_per_sample;
bits_per_sample = buf[ cn_aiff_comm_sample_size ] << 8;
bits_per_sample += buf[ cn_aiff_comm_sample_size + 1 ];




unsigned char buf_srate[ 10 ];

buf_srate[ 9 ] = buf[ cn_aiff_comm_sample_srate ];          //read in big endian 80 bit IEEE 754 extended precision floating point number (long double)
buf_srate[ 8 ] = buf[ cn_aiff_comm_sample_srate + 1 ];      //convert to little endian
buf_srate[ 7 ] = buf[ cn_aiff_comm_sample_srate + 2 ];
buf_srate[ 6 ] = buf[ cn_aiff_comm_sample_srate + 3 ];
buf_srate[ 5 ] = buf[ cn_aiff_comm_sample_srate + 4 ];
buf_srate[ 4 ] = buf[ cn_aiff_comm_sample_srate + 5 ];
buf_srate[ 3 ] = buf[ cn_aiff_comm_sample_srate + 6 ];
buf_srate[ 2 ] = buf[ cn_aiff_comm_sample_srate + 7 ];
buf_srate[ 1 ] = buf[ cn_aiff_comm_sample_srate + 8 ];
buf_srate[ 0 ] = buf[ cn_aiff_comm_sample_srate + 9 ];

long double ld = *(long double*)buf_srate;

unsigned int srate = ld;                                   //convert srate to int




if( strncmp( (char*)( buf + cn_aiff_comm_ssnd_ckid ), "SSND", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_aiff_malloc() - no 'SSND ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}



unsigned int ssnd_cksize;
ssnd_cksize = buf[ cn_aiff_comm_ssnd_cksize ] << 24;
ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 1 ] << 16;
ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 2 ] << 8;
ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 3 ];



unsigned int offset;
offset = buf[ cn_aiff_comm_ssnd_offset ] << 24;
offset += buf[ cn_aiff_comm_ssnd_offset + 1 ] << 16;
offset += buf[ cn_aiff_comm_ssnd_offset + 2 ] << 8;
offset += buf[ cn_aiff_comm_ssnd_offset + 3 ];



unsigned int align_byte_cnt ;
align_byte_cnt = buf[ cn_aiff_comm_ssnd_block_size ] << 24;
align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 1 ] << 16;
align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 2 ] << 8;
align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 3 ];








/*
FILE *fp2;

s1 = "zzieee.bin";

fp2 = m1.mbc_fopen( s1, "wb" );		    	//open file
fwrite( buf_srate, 1, sizeof(buf_srate), fp2 );          //write block of audio data
fclose( fp2 );
*/

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    fclose( fp );
    return -6;
    }


if( bits_per_sample != 16 )
    {
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - for filename: '%s', bets_per_sample is not supported: %u\n", fname1.c_str(), bits_per_sample );
    fclose( fp );
    return -5;
    }




af.path = path;
af.fname = fname1;
af.srate = srate;
af.offset = offset;
af.channels = channels;
af.is_big_endian = 1;


if( verb ) logpf( "audio_formats::load_aiff_malloc() - samplerate: %u, for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::load_aiff_malloc() - channels: %u found in file: '%s'\n", af.channels, fname1.c_str() );


//fclose( fp );
//return 1;

int skip = offset - 4 * 6;

if ( skip < 0 ) skip = 0;
for( int i = 0; i < skip; i++ )					    //step to first sample byte
    {
    int read = fread( buf, 1, 1, fp );
    }


unsigned int byte_cnt = ssnd_cksize - 8;            //data block size is less the unsigned long 'cn_aiff_comm_ssnd_offset and 'cn_aiff_comm_ssnd_block_size'
unsigned int cnt = 0;
unsigned int min_sample_idx;
unsigned int max_sample_idx;

//	if( verb ) logpf( "audio_formats::load_wav() - byte_cnt: %u\n", byte_cnt );

//logpf("byte_cnt: %u\n", byte_cnt );

double dd;
bool trk2 = 0;
char cc;
int iaud;
int i = 0;
while( 1 )
	{
    int read;

	need = byte_cnt - cnt;
	if( need > cn_audio_formats_buf_siz ) need = cn_audio_formats_buf_siz;
	
    read = fread( buf, 1, need, fp );          				//read block of audio data
	if ( read == 0 ) break;
	cnt += read;
	
	if( read != need )
		{
		if( verb ) logpf( "audio_formats::load_aiff_malloc() - tried to read: %u bytes, only got %u bytes for file: '%s'\n", need, read, fname1.c_str() );
		}


    for( int i = 0; i < read; )                 //process block
        {

        dd = (char)buf[ i ] * 256.0;        //preserve sign
        i++;
        if( i >= read )
            {
            if( verb ) logpf( "audio_formats::load_aiff_malloc() - odd number of bytes encountered, lost final sample\n" );
            break;
            }
        dd += buf[ i ];
        i++;



        if( channels == 1 )                     //1 trk?
            {
            push_ch0( dd );
            }
        else{
            if( trk2 == 0 ) push_ch0( dd );
            else push_ch1( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
        }
	}


if( verb )
	{
	logpf( "audio_formats::load_aiff_malloc() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_aiff_malloc() - sample: %06d has max: %g\n", max_sample_idx, max );
	}




double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < sizech0; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    pch0[ i ] = dd;
    }


for( int i = 0; i < sizech1; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    pch1[ i ] = dd;
    }

if( verb ) logpf( "audio_formats::load_aiff_malloc() - samples read (ch1: %u), (ch2: %u) from '%s'\n", sizech0, sizech1, fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_aiff_malloc() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}
















//searches file for matching string,
//reads from current file positin till spec string is found, file pos is moved to the found position, 
//also provides the position of match relative to start of file in 'found_at_pos'
//returns 1 on success, else 0 
bool audio_formats::find_in_file( FILE *fp, string str, unsigned int &found_at_pos )
{
found_at_pos = 0;

if( fp == 0 ) return 0;

unsigned int len = str.length();

if( len == 0 ) return 0;

unsigned char buf[ 8192 ];


long orig_file_pos = ftell( fp );
if( orig_file_pos == -1 )
	{
    if( verb ) logpf( "audio_formats::find_in_file() - can't find file's position using ftell()\n" );
	return 0;
	}

//asm("int3");						//useful to trigger debugger

long new_file_pos = orig_file_pos;		//make sure the file pos is not changed if there is no match
long file_offs = 0;
bool found_it = 0;
unsigned int pp = 0;
while( 1 )
	{
	
	unsigned int read = fread( buf, 1, 8192, fp );
	if( read == 0 ) break;
	
	unsigned int ptr = 0;
	for( int i = 0; i < read; i++ )
		{
		if( buf[ ptr + pp ] == str[ pp ] )
			{
			pp++;
			if( pp >= len )
				{
				found_at_pos = orig_file_pos + file_offs;
				new_file_pos = orig_file_pos + file_offs + pp;		//move past
				found_it = 1;
				goto done;
				}
			}
		else{
			pp = 0;
			ptr++;
			file_offs++;
			}
		}
	}

done:
logpf( "audio_formats::find_in_file() - found '%s' at filepos: %x\n", str.c_str(), found_at_pos );

fseek( fp, new_file_pos, SEEK_SET );                //move file ptr

if ( !found_it ) return 0;
return 1;
}









//load a wav audio file (only PCM 16 bit - 4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 16 bit pcm only, 1 or 2 channels

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_wav( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

string fname1 = path;
fname1 += fname;

vector<double> vch0;
vector<double> vch1;

af.vch0.clear();
af.vch1.clear();







unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_wav() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_wav() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_wav() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_wav() - failed to open file for reading: '%s'\n", fname1.c_str() );
	return -1;
	}


unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_wav() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }


unsigned int need = 12 + 24;							//size of 'hdr' + 'fmt chunk' and begining of 'data chunk', this will reveal various values which will determine
												//actual size of the sample data bytes
unsigned long read = fread( buf, 1, need, fp );          	//read block of audio data

if( read != need )
	{
	if( verb ) logpf( "audio_formats::load_wav() - failed to read header bytes: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}



if( strncmp( (char*)( buf + cn_riff_hdr_ckid ), "RIFF", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_wav() - no 'RIFF' found in file header: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


unsigned int tot_chunk_size = *(unsigned int*)( buf + cn_riff_hdr_cksize );

if( verb ) logpf( "audio_formats::load_wav() - tot_chunk_size size: %u\n", tot_chunk_size );



if( strncmp( (char*)( buf + cn_riff_hdr_waveid ), "WAVE", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_wav() - no 'WAVE' found in file header: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}



if( strncmp( (char*)( buf + 12 + cn_riff_fmt_chunk_ckid ), "fmt ", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_wav() - no 'fmt ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


unsigned int fmt_chunk_size;
unsigned int format_tag;
unsigned int wave_channels;
unsigned int samp_per_sec;
unsigned int avg_bytes_per_sec;
unsigned int block_align;
unsigned int bits_per_sample;
unsigned int cbsize;
unsigned int data_chunk_size;


fmt_chunk_size = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_cksize );



if( fmt_chunk_size != 16 )
    {
	if( verb ) logpf( "audio_formats::load_wav() - fmt_chunk_size is not equal to 16 (at location 16), ignoring this, file: '%s'\n", fname1.c_str() );
//    free( buf );
//	fclose( fp );
//	return 0;
    }


if( verb ) logpf( "audio_formats::load_wav() - fmt_chunk_size: %u\n", fmt_chunk_size );


format_tag = buf[ 12 + cn_riff_fmt_chunk_wformat_tag ];
format_tag += buf[ 12 + cn_riff_fmt_chunk_wformat_tag + 1 ] << 8;

if( verb ) logpf( "audio_formats::load_wav() - format_tag: %u\n", format_tag );


if( format_tag != 1 )                   //not a pcm format?
    {
    if( verb ) logpf( "audio_formats::load_wav() - unsupported format_tag: %u, file: '%s'\n", format_tag, fname1.c_str() );
    free( buf );
    fclose( fp );
    return -5;
    }

wave_channels = buf[ 12 + cn_riff_fmt_chunk_channels ];
wave_channels += buf[ 12 + cn_riff_fmt_chunk_channels + 1 ] << 8;

if( verb ) logpf( "audio_formats::load_wav() - wave_channels: %u\n", wave_channels );

if( ( wave_channels <= 0 ) || ( wave_channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_wav() - unsupported number of channels: %u\n", wave_channels );
    free( buf );
    fclose( fp );
    return -6;
    }



samp_per_sec = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_samp_per_sec );

if( verb ) logpf( "audio_formats::load_wav() - samples_per_sec: %u\n", samp_per_sec );



avg_bytes_per_sec = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_avg_bytes_per_sec );

if( verb ) logpf( "audio_formats::load_wav() - avg_bytes_per_sec: %u\n", avg_bytes_per_sec );


block_align = buf[ 12 + cn_riff_fmt_chunk_block_align ];
block_align += buf[ 12 + cn_riff_fmt_chunk_block_align + 1 ] << 8;

if( verb ) logpf( "audio_formats::load_wav() - block_align: %u\n", block_align );


bits_per_sample = buf[ 12 + cn_riff_fmt_chunk_bits_per_samp ];
bits_per_sample += buf[ 12 + cn_riff_fmt_chunk_bits_per_samp + 1 ] << 8;

if( verb ) logpf( "audio_formats::load_wav() - bits_per_sample: %u\n", bits_per_sample );



unsigned int data_chuck_pos;

if( !find_in_file( fp, "data", data_chuck_pos ) )
	{
	if( verb ) logpf( "audio_formats::load_wav() - no 'data' chunk was found in file: '%s'\n", fname1.c_str() );
	free( buf );
	fclose( fp );
	return -4;
	}
//getchar();

/*
while( 1 )
	{
	if( strncmp( (char*)( buf + 12 + 24 + cn_riff_data_chunk_ckid ), "data", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::load_wav() - no 'data' found in data chunk: '%s'\n", fname1.c_str() );
		free( buf );
		fclose( fp );
		return 0;
		}
	else{
		break;
		}
	}
*/

//data_chunk_size = *(unsigned int*)( buf + 12 + 24 + cn_riff_data_chunk_cksize );


read = fread( buf, 1, 4, fp );          				//read data chunk's size bytes
if( read != 4 )
    {
    if( verb ) logpf( "audio_formats::load_wav() - the file ended before data chunk's size bytes could be read: '%s'\n", fname1.c_str() );
    free( buf );
    fclose( fp );
    return -4;
    }

data_chunk_size = *(unsigned int*)( buf );


if( verb ) logpf( "audio_formats::load_wav() - data_chunk_size: %u\n", data_chunk_size );


unsigned int samples_per_channel = data_chunk_size / ( wave_channels * ( bits_per_sample / 8 ) );

if( verb ) logpf( "audio_formats::load_wav() - samples_per_channel: %u\n", samples_per_channel );

af.path = path;
af.fname = fname1;
af.encoding = 0;
af.offset = 0;
af.is_big_endian = 0;
af.channels = wave_channels;
af.srate = samp_per_sec;
af.bits_per_sample = bits_per_sample;



unsigned int byte_cnt = samples_per_channel * wave_channels * ( bits_per_sample / 8 );
unsigned int cnt = 0;
unsigned int min_sample_idx;
unsigned int max_sample_idx;


bool trk2 = 0;
char cc;
int iaud;
int i = 0;
while( 1 )
	{
    int read;


	need = byte_cnt - cnt;
	if( need > cn_audio_formats_buf_siz ) need = cn_audio_formats_buf_siz;
	
    read = fread( buf, 1, need, fp );          				//read block of audio data
	if ( read == 0 ) break;
	cnt += read;
	
	if( read != need )
		{
		if( verb ) logpf( "audio_formats::load_wav() - tried to read: %u bytes, only got %u bytes for file: '%s'\n", need, read, fname1.c_str() );
		}


    for( int i = 0; i < read; )                 //process block
        {
        i++;
        if( i >= read )
            {
            if( verb ) logpf( "audio_formats::load_wav() - odd number of bytes encountered, lost final sample\n" );
            break;
            }

        cc = buf[ i ];						    //preserve sign in msbyte (little endia)

        iaud = cc * 256;						//shift msbyte to its location preserving sign
        iaud |= buf[ i - 1 ];					//or in lsbyte
        dd = iaud;
        i++;

        if( wave_channels == 1 )                //1 trk?
            {
            vch0.push_back( dd );
            }
        else{
            if( trk2 == 0 ) vch0.push_back( dd );
            else vch1.push_back( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
        }
//logpf("read: %d\n", read );
	}


if( verb )
	{
	logpf( "audio_formats::load_wav() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_wav() - sample: %06d has max: %g\n", max_sample_idx, max );
	}





double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_wav() - will not normalise, down scaling by: %u\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_wav() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < vch0.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    af.vch0.push_back( dd );
    }


for( int i = 0; i < vch1.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    af.vch1.push_back( dd );
    }

if( verb ) logpf( "audio_formats::load_wav() - samples read (ch1: %u), (ch2: %u) from '%s'\n", af.vch0.size(), af.vch1.size(), fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_wav() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_wav() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}


















//load a wav audio file using malloc double pointers: pch0, pch1 (only PCM 16 bit - 4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 16 bit pcm only, 1 or 2 channels

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_wav_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

string fname1 = path;
fname1 += fname;


clear_ch0();
clear_ch1();





unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_wav_malloc() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_wav_malloc() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_wav_malloc() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_wav_malloc() - failed to open file for reading: '%s'\n", fname1.c_str() );
	return -1;
	}


unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_wav_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }


unsigned int need = 12 + 24;							//size of 'hdr' + 'fmt chunk' and begining of 'data chunk', this will reveal various values which will determine
												//actual size of the sample data bytes
unsigned long read = fread( buf, 1, need, fp );          	//read block of audio data

if( read != need )
	{
	if( verb ) logpf( "audio_formats::load_wav_malloc() - failed to read header bytes: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}



if( strncmp( (char*)( buf + cn_riff_hdr_ckid ), "RIFF", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_wav_malloc() - no 'RIFF' found in file header: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


unsigned int tot_chunk_size = *(unsigned int*)( buf + cn_riff_hdr_cksize );

if( verb ) logpf( "audio_formats::load_wav_malloc() - tot_chunk_size size: %u\n", tot_chunk_size );



if( strncmp( (char*)( buf + cn_riff_hdr_waveid ), "WAVE", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_wav_malloc() - no 'WAVE' found in file header: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}



if( strncmp( (char*)( buf + 12 + cn_riff_fmt_chunk_ckid ), "fmt ", 4 ) != 0 )
	{
	if( verb ) logpf( "audio_formats::load_wav_malloc() - no 'fmt ' found in chunk: '%s'\n", fname1.c_str() );
    free( buf );
	fclose( fp );
	return -4;
	}


unsigned int fmt_chunk_size;
unsigned int format_tag;
unsigned int wave_channels;
unsigned int samp_per_sec;
unsigned int avg_bytes_per_sec;
unsigned int block_align;
unsigned int bits_per_sample;
unsigned int cbsize;
unsigned int data_chunk_size;


fmt_chunk_size = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_cksize );



if( fmt_chunk_size != 16 )
    {
	if( verb ) logpf( "audio_formats::load_wav_malloc() - fmt_chunk_size is not equal to 16 (at location 16), ignoring this, file: '%s'\n", fname1.c_str() );
//    free( buf );
//	fclose( fp );
//	return 0;
    }


if( verb ) logpf( "audio_formats::load_wav_malloc() - fmt_chunk_size: %u\n", fmt_chunk_size );


format_tag = buf[ 12 + cn_riff_fmt_chunk_wformat_tag ];
format_tag += buf[ 12 + cn_riff_fmt_chunk_wformat_tag + 1 ] << 8;

if( verb ) logpf( "audio_formats::load_wav_malloc() - format_tag: %u\n", format_tag );


if( format_tag != 1 )                   //not a pcm format?
    {
    if( verb ) logpf( "audio_formats::load_wav_malloc() - unsupported format_tag: %u, file: '%s'\n", format_tag, fname1.c_str() );
    free( buf );
    fclose( fp );
    return -5;
    }

wave_channels = buf[ 12 + cn_riff_fmt_chunk_channels ];
wave_channels += buf[ 12 + cn_riff_fmt_chunk_channels + 1 ] << 8;

if( verb ) logpf( "audio_formats::load_wav_malloc() - wave_channels: %u\n", wave_channels );

if( ( wave_channels <= 0 ) || ( wave_channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_wav_malloc() - unsupported number of channels: %u\n", wave_channels );
    free( buf );
    fclose( fp );
    return -6;
    }



samp_per_sec = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_samp_per_sec );

if( verb ) logpf( "audio_formats::load_wav_malloc() - samples_per_sec: %u\n", samp_per_sec );



avg_bytes_per_sec = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_avg_bytes_per_sec );

if( verb ) logpf( "audio_formats::load_wav_malloc() - avg_bytes_per_sec: %u\n", avg_bytes_per_sec );


block_align = buf[ 12 + cn_riff_fmt_chunk_block_align ];
block_align += buf[ 12 + cn_riff_fmt_chunk_block_align + 1 ] << 8;

if( verb ) logpf( "audio_formats::load_wav_malloc() - block_align: %u\n", block_align );


bits_per_sample = buf[ 12 + cn_riff_fmt_chunk_bits_per_samp ];
bits_per_sample += buf[ 12 + cn_riff_fmt_chunk_bits_per_samp + 1 ] << 8;

if( verb ) logpf( "audio_formats::load_wav_malloc() - bits_per_sample: %u\n", bits_per_sample );



unsigned int data_chuck_pos;

if( !find_in_file( fp, "data", data_chuck_pos ) )
	{
	if( verb ) logpf( "audio_formats::load_wav_malloc() - no 'data' chunk was found in file: '%s'\n", fname1.c_str() );
	free( buf );
	fclose( fp );
	return -4;
	}
//getchar();

/*
while( 1 )
	{
	if( strncmp( (char*)( buf + 12 + 24 + cn_riff_data_chunk_ckid ), "data", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::load_wav() - no 'data' found in data chunk: '%s'\n", fname1.c_str() );
		free( buf );
		fclose( fp );
		return 0;
		}
	else{
		break;
		}
	}
*/

//data_chunk_size = *(unsigned int*)( buf + 12 + 24 + cn_riff_data_chunk_cksize );


read = fread( buf, 1, 4, fp );          				//read data chunk's size bytes
if( read != 4 )
    {
    if( verb ) logpf( "audio_formats::load_wav_malloc() - the file ended before data chunk's size bytes could be read: '%s'\n", fname1.c_str() );
    free( buf );
    fclose( fp );
    return -4;
    }

data_chunk_size = *(unsigned int*)( buf );


if( verb ) logpf( "audio_formats::load_wav_malloc() - data_chunk_size: %u\n", data_chunk_size );


unsigned int samples_per_channel = data_chunk_size / ( wave_channels * ( bits_per_sample / 8 ) );

if( verb ) logpf( "audio_formats::load_wav_malloc() - samples_per_channel: %u\n", samples_per_channel );

af.path = path;
af.fname = fname1;
af.encoding = 0;
af.offset = 0;
af.is_big_endian = 0;
af.channels = wave_channels;
af.srate = samp_per_sec;
af.bits_per_sample = bits_per_sample;



unsigned int byte_cnt = samples_per_channel * wave_channels * ( bits_per_sample / 8 );
unsigned int cnt = 0;
unsigned int min_sample_idx;
unsigned int max_sample_idx;


bool trk2 = 0;
char cc;
int iaud;
int i = 0;
while( 1 )
	{
    int read;


	need = byte_cnt - cnt;
	if( need > cn_audio_formats_buf_siz ) need = cn_audio_formats_buf_siz;
	
    read = fread( buf, 1, need, fp );          				//read block of audio data
	if ( read == 0 ) break;
	cnt += read;
	
	if( read != need )
		{
		if( verb ) logpf( "audio_formats::load_wav_malloc() - tried to read: %u bytes, only got %u bytes for file: '%s'\n", need, read, fname1.c_str() );
		}


    for( int i = 0; i < read; )                 //process block
        {
        i++;
        if( i >= read )
            {
            if( verb ) logpf( "audio_formats::load_wav_malloc() - odd number of bytes encountered, lost final sample\n" );
            break;
            }

        cc = buf[ i ];						    //preserve sign in msbyte (little endia)

        iaud = cc * 256;						//shift msbyte to its location preserving sign
        iaud |= buf[ i - 1 ];					//or in lsbyte
        dd = iaud;
        i++;

        if( wave_channels == 1 )                //1 trk?
            {
            push_ch0( dd );
            }
        else{
            if( trk2 == 0 ) push_ch0( dd );
            else push_ch1( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
        }
//logpf("read: %d\n", read );
	}


if( verb )
	{
	logpf( "audio_formats::load_wav_malloc() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_wav_malloc() - sample: %06d has max: %g\n", max_sample_idx, max );
	}





double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_wav_malloc() - will not normalise, down scaling by: %u\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_wav_malloc() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < sizech0; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    pch0[ i ] = dd;
    }


for( int i = 0; i < sizech1; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    pch1[ i ] = dd;
    }

if( verb ) logpf( "audio_formats::load_wav_malloc() - samples read (ch1: %u), (ch2: %u) from '%s'\n", sizech0, sizech1, fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_wav_malloc() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_wav_malloc() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}




















//save audio waveform in AIFF file format (limited to 4GB)
//supports pcm, 16 bit, big endian, 1 or 2 channel
//samples must be between +/- 1.0, saved samples are scaled up by peak_int_val
bool audio_formats::save_aiff( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;
double dd;


string fname1 = path;
fname1 += fname;

bool supported = 0;



unsigned int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::save_aiff() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return 0;
    }




{
unsigned long long int cnt0 = af.vch0.size();
unsigned long long int cnt1 = af.vch1.size();


if( cnt0 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_aiff() - ch1 is too big: %"LLU" for file filename: '%s'\n", cnt0, fname1.c_str() );
    return 0;
    }


if( cnt1 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_aiff() - ch2 is too big: %"LLU" for file filename: '%s'\n", cnt1, fname1.c_str() );
    return 0;
    }
}




{
unsigned int cnt0 = af.vch0.size();
unsigned int cnt1 = af.vch1.size();

if( ( cnt0 == 0 ) && ( cnt1 == 0 ) )
	{
	if( verb ) logpf( "audio_formats::save_aiff() - for filename: '%s', both channels are empty, no samples to write\n", fname1.c_str() );
	return 0;
	}


if( channels == 1 )                         //1 channel?
	{
	if( cnt0 == 0 )
		{
		if( verb ) logpf( "audio_formats::save_aiff() - for filename: '%s', ch1 is empty, no samples to write\n", fname1.c_str() );
		return 0;
		}
	}

if( channels == 2 )
	{
	if( cnt0 < cnt1 )
		{
		if( verb ) logpf( "audio_formats::save_aiff() - ch1 has less samples, will add zeroed samples to match ch2\n" );
		}

	if( cnt1 < cnt0 )
		{
		if( verb ) logpf( "audio_formats::save_aiff() - ch2 has less samples, will add zeroed samples to match ch1\n" );
		}

	if( cnt0 < cnt1 )									//make both channels the same size, by increasing ch0
		{
		for( int i = cnt0; i < cnt1; i++ ) af.vch0.push_back( 0 );	
		}

	if( cnt1 < cnt0 )									//make both channels the same size, by increasing ch1
		{
		for( int i = cnt1; i < cnt0; i++ ) af.vch1.push_back( 0 );	
		}
	}
}


if( verb ) logpf( "audio_formats::save_aiff() - denormalising using scale factor: %d\n", peak_int_val );


unsigned int cnt0 = af.vch0.size() * channels;


unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::save_aiff() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    return 0;
    }


unsigned int bytes_per_sample = 2;

unsigned int ui = cnt0 * bytes_per_sample + 46;				//samples * bytes_per_sample + hdr


//if( verb ) logpf( "audio_formats::save_aiff() - cnt0: %u\n", cnt0 );
//if( verb ) logpf( "audio_formats::save_aiff() - ui: %u\n", ui );

//save aiff header
strncpy( (char*)(buf + cn_aiff_form_ckid), "FORM", 4 );


buf[ cn_aiff_form_cksize ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_form_cksize + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_form_cksize + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_form_cksize + 3 ] = ui & 0xff;

strncpy( (char*)( buf + cn_aiff_aiff_ckid ), "AIFF", 4 );

strncpy( (char*)( buf + cn_aiff_comm_ckid ), "COMM", 4 );

ui = 18;
buf[ cn_aiff_comm_cksize ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_cksize + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_cksize + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_cksize + 3 ] = ui & 0xff;

buf[ cn_aiff_comm_channels ] = ( channels >> 8 ) & 0xff;
buf[ cn_aiff_comm_channels + 1 ] = channels & 0xff;


ui = cnt0 / channels;										//number of sample frames
buf[ cn_aiff_comm_sample_frames ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_sample_frames + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_sample_frames + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_sample_frames + 3 ] = ui & 0xff;


ui = 16;
buf[ cn_aiff_comm_sample_size ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_sample_size + 1 ] = ui & 0xff;



long double ld_srate = af.srate;				//sample rate is stored as a big edndian IEEE 754 extended precision floating point number (long double)
unsigned char *ld_ptr = (unsigned char *) &ld_srate;

buf[ cn_aiff_comm_sample_srate ] = ld_ptr[ 9 ];
buf[ cn_aiff_comm_sample_srate + 1 ] = ld_ptr[ 8 ];
buf[ cn_aiff_comm_sample_srate + 2 ] = ld_ptr[ 7 ];
buf[ cn_aiff_comm_sample_srate + 3 ] = ld_ptr[ 6 ];
buf[ cn_aiff_comm_sample_srate + 4 ] = ld_ptr[ 5 ];
buf[ cn_aiff_comm_sample_srate + 5 ] = ld_ptr[ 4 ];
buf[ cn_aiff_comm_sample_srate + 6 ] = ld_ptr[ 3 ];
buf[ cn_aiff_comm_sample_srate + 7 ] = ld_ptr[ 2 ];
buf[ cn_aiff_comm_sample_srate + 8 ] = ld_ptr[ 1 ];
buf[ cn_aiff_comm_sample_srate + 9 ] = ld_ptr[ 0 ];


strncpy( (char*)( buf + cn_aiff_comm_ssnd_ckid ), "SSND", 4 );



ui = cnt0 * bytes_per_sample + 8;				//samples * bytes_per_sample + some hdr
buf[ cn_aiff_comm_ssnd_cksize ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_ssnd_cksize + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_ssnd_cksize + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_ssnd_cksize + 3 ] = ui & 0xff;

ui = 0;
buf[ cn_aiff_comm_ssnd_offset ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_ssnd_offset + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_ssnd_offset + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_ssnd_offset + 3 ] = ui & 0xff;


ui = 0;
buf[ cn_aiff_comm_ssnd_block_size ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_ssnd_block_size + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_ssnd_block_size + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_ssnd_block_size + 3 ] = ui & 0xff;

FILE *fp = m1.mbc_fopen( fname1, "wb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::save_aiff() - failed to open file for writing: '%s'\n", fname1.c_str() );
    free( buf );
	return 0;
	}

int wrote;

wrote = fwrite( buf, 1, 54, fp );                //write aiff file header

if( wrote != 54 )
	{
	if( verb ) logpf( "audio_formats::save_aiff() - failed to write header to file: '%s', total bytes written: %u\n", fname1.c_str(), wrote );
    free( buf );
	fclose( fp );
	return 0;
	}

int iaud;

bool trk2 = 0;

unsigned int cnt = 0;
unsigned int j = 0;
unsigned int k = 0;

for( int i = 0; i < cnt0; i++ )
	{

	if( !trk2 ) dd = af.vch0[ k ];
	else  dd = af.vch1[ k ];


	iaud = nearbyint( dd * (double)peak_int_val );	//v1.04
	buf[ j ] = ( iaud >> 8  ) & 0xff;               //big endian pcm?
	j++;
	buf[ j ] =  iaud & 0xff;
	j++;


	if( j >= cn_audio_formats_buf_siz )
		{
		wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;

		if( wrote != j )
			{
			if( verb ) logpf( "audio_formats::save_aiff() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
			fclose( fp );
			return 0;
			}
		cnt += wrote;
		j = 0;
		}

	if( channels == 2 )
		{
        if( trk2 ) k++;
        trk2 = !trk2;
		}
    else{
        k++;
        }
//if(!(cnt%1000))printf("audio_formats::save_sun() - limit: %u, cnt: %u\n", cnt0, cnt );
	}

if( j != 0 )
	{
	wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;
	if( wrote != j )
		{
		if( verb ) logpf( "audio_formats::save_aiff() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
        free( buf );
		fclose( fp );
		return 0;
		}
	cnt += wrote;
	}


if( verb ) logpf( "audio_formats::save_aiff() - encoding is 16 bit pcm, big endian\n" );

if( verb ) logpf( "audio_formats::save_aiff() - wrote audio at sample rate of %u for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::save_aiff() - wrote %u samples in ch1 for file: '%s'\n", k, fname1.c_str() );
if( verb ) if( channels == 2 ) logpf( "audio_formats::save_aiff() - wrote %u samples in ch2 for file: '%s'\n", k, fname1.c_str() );

free( buf );
fclose( fp );

return 1;

}

















//save audio waveform in AIFF file format using malloc double pointers: pch0, pch1 (limited to 4GB)
//supports pcm, 16 bit, big endian, 1 or 2 channel
//samples must be between +/- 1.0, saved samples are scaled up by peak_int_val
bool audio_formats::save_aiff_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;
double dd;


string fname1 = path;
fname1 += fname;

bool supported = 0;



unsigned int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::save_aiff_malloc() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return 0;
    }




{
unsigned long long int cnt0 = sizech0;
unsigned long long int cnt1 = sizech1;


if( cnt0 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_aiff_malloc() - ch1 is too big: %"LLU" for file filename: '%s'\n", cnt0, fname1.c_str() );
    return 0;
    }


if( cnt1 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_aiff_malloc() - ch2 is too big: %"LLU" for file filename: '%s'\n", cnt1, fname1.c_str() );
    return 0;
    }
}



{
unsigned int cnt0 = sizech0;
unsigned int cnt1 = sizech1;

if( ( cnt0 == 0 ) && ( cnt1 == 0 ) )
	{
	if( verb ) logpf( "audio_formats::save_aiff_malloc() - for filename: '%s', both channels are empty, no samples to write\n", fname1.c_str() );
	return 0;
	}


if( channels == 1 )                         //1 channel?
	{
	if( cnt0 == 0 )
		{
		if( verb ) logpf( "audio_formats::save_aiff_malloc() - for filename: '%s', ch1 is empty, no samples to write\n", fname1.c_str() );
		return 0;
		}
	}

if( channels == 2 )
	{
	if( cnt0 < cnt1 )
		{
		if( verb ) logpf( "audio_formats::save_aiff_malloc() - ch1 has less samples, will add zeroed samples to match ch2\n" );
		}

	if( cnt1 < cnt0 )
		{
		if( verb ) logpf( "audio_formats::save_aiff_malloc() - ch2 has less samples, will add zeroed samples to match ch1\n" );
		}

	if( cnt0 < cnt1 )									//make both channels the same size, by increasing ch0
		{
		for( int i = cnt0; i < cnt1; i++ ) push_ch0( 0 );	
		}

	if( cnt1 < cnt0 )									//make both channels the same size, by increasing ch1
		{
		for( int i = cnt1; i < cnt0; i++ ) push_ch1( 0 );	
		}
	}
}


if( verb ) logpf( "audio_formats::save_aiff_malloc() - denormalising using scale factor: %d\n", peak_int_val );


unsigned int cnt0 = sizech0 * channels;


unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::save_aiff_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    return 0;
    }


unsigned int bytes_per_sample = 2;

unsigned int ui = cnt0 * bytes_per_sample + 46;				//samples * bytes_per_sample + hdr


//if( verb ) logpf( "audio_formats::save_aiff() - cnt0: %u\n", cnt0 );
//if( verb ) logpf( "audio_formats::save_aiff() - ui: %u\n", ui );

//save aiff header
strncpy( (char*)(buf + cn_aiff_form_ckid), "FORM", 4 );


buf[ cn_aiff_form_cksize ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_form_cksize + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_form_cksize + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_form_cksize + 3 ] = ui & 0xff;

strncpy( (char*)( buf + cn_aiff_aiff_ckid ), "AIFF", 4 );

strncpy( (char*)( buf + cn_aiff_comm_ckid ), "COMM", 4 );

ui = 18;
buf[ cn_aiff_comm_cksize ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_cksize + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_cksize + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_cksize + 3 ] = ui & 0xff;

buf[ cn_aiff_comm_channels ] = ( channels >> 8 ) & 0xff;
buf[ cn_aiff_comm_channels + 1 ] = channels & 0xff;


ui = cnt0 / channels;										//number of sample frames
buf[ cn_aiff_comm_sample_frames ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_sample_frames + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_sample_frames + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_sample_frames + 3 ] = ui & 0xff;


ui = 16;
buf[ cn_aiff_comm_sample_size ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_sample_size + 1 ] = ui & 0xff;



long double ld_srate = af.srate;				//sample rate is stored as a big edndian IEEE 754 extended precision floating point number (long double)
unsigned char *ld_ptr = (unsigned char *) &ld_srate;

buf[ cn_aiff_comm_sample_srate ] = ld_ptr[ 9 ];
buf[ cn_aiff_comm_sample_srate + 1 ] = ld_ptr[ 8 ];
buf[ cn_aiff_comm_sample_srate + 2 ] = ld_ptr[ 7 ];
buf[ cn_aiff_comm_sample_srate + 3 ] = ld_ptr[ 6 ];
buf[ cn_aiff_comm_sample_srate + 4 ] = ld_ptr[ 5 ];
buf[ cn_aiff_comm_sample_srate + 5 ] = ld_ptr[ 4 ];
buf[ cn_aiff_comm_sample_srate + 6 ] = ld_ptr[ 3 ];
buf[ cn_aiff_comm_sample_srate + 7 ] = ld_ptr[ 2 ];
buf[ cn_aiff_comm_sample_srate + 8 ] = ld_ptr[ 1 ];
buf[ cn_aiff_comm_sample_srate + 9 ] = ld_ptr[ 0 ];


strncpy( (char*)( buf + cn_aiff_comm_ssnd_ckid ), "SSND", 4 );



ui = cnt0 * bytes_per_sample + 8;				//samples * bytes_per_sample + some hdr
buf[ cn_aiff_comm_ssnd_cksize ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_ssnd_cksize + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_ssnd_cksize + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_ssnd_cksize + 3 ] = ui & 0xff;

ui = 0;
buf[ cn_aiff_comm_ssnd_offset ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_ssnd_offset + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_ssnd_offset + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_ssnd_offset + 3 ] = ui & 0xff;


ui = 0;
buf[ cn_aiff_comm_ssnd_block_size ] = ( ui >> 24 ) & 0xff;
buf[ cn_aiff_comm_ssnd_block_size + 1 ] = ( ui >> 16 ) & 0xff;
buf[ cn_aiff_comm_ssnd_block_size + 2 ] = ( ui >> 8 ) & 0xff;
buf[ cn_aiff_comm_ssnd_block_size + 3 ] = ui & 0xff;

FILE *fp = m1.mbc_fopen( fname1, "wb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::save_aiff_malloc() - failed to open file for writing: '%s'\n", fname1.c_str() );
    free( buf );
	return 0;
	}

int wrote;

wrote = fwrite( buf, 1, 54, fp );                //write aiff file header

if( wrote != 54 )
	{
	if( verb ) logpf( "audio_formats::save_aiff_malloc() - failed to write header to file: '%s', total bytes written: %u\n", fname1.c_str(), wrote );
    free( buf );
	fclose( fp );
	return 0;
	}

int iaud;

bool trk2 = 0;

unsigned int cnt = 0;
unsigned int j = 0;
unsigned int k = 0;

for( int i = 0; i < cnt0; i++ )
	{

	if( !trk2 ) dd = pch0[ k ];
	else  dd = pch1[ k ];


	iaud = nearbyint( dd * (double)peak_int_val );	//v1.04
	buf[ j ] = ( iaud >> 8  ) & 0xff;               //big endian pcm?
	j++;
	buf[ j ] =  iaud & 0xff;
	j++;


	if( j >= cn_audio_formats_buf_siz )
		{
		wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;

		if( wrote != j )
			{
			if( verb ) logpf( "audio_formats::save_aiff_malloc() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
			fclose( fp );
			return 0;
			}
		cnt += wrote;
		j = 0;
		}

	if( channels == 2 )
		{
        if( trk2 ) k++;
        trk2 = !trk2;
		}
    else{
        k++;
        }
//if(!(cnt%1000))printf("audio_formats::save_sun() - limit: %u, cnt: %u\n", cnt0, cnt );
	}

if( j != 0 )
	{
	wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;
	if( wrote != j )
		{
		if( verb ) logpf( "audio_formats::save_aiff_malloc() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
        free( buf );
		fclose( fp );
		return 0;
		}
	cnt += wrote;
	}


if( verb ) logpf( "audio_formats::save_aiff_malloc() - encoding is 16 bit pcm, big endian\n" );

if( verb ) logpf( "audio_formats::save_aiff_malloc() - wrote audio at sample rate of %u for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::save_aiff_malloc() - wrote %u samples in ch1 for file: '%s'\n", k, fname1.c_str() );
if( verb ) if( channels == 2 ) logpf( "audio_formats::save_aiff_malloc() - wrote %u samples in ch2 for file: '%s'\n", k, fname1.c_str() );

free( buf );
fclose( fp );

return 1;

}















//save a raw audio file (4GB limit)
//supports 16 bit pcm - litte or big endian, 1 or 2 channels
//samples must be between +/- 1.0, saved samples are scaled up by 'peak_int_val'
bool audio_formats::save_raw( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;

string fname1 = path;
fname1 += fname;

int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::save_raw() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return 0;
    }




unsigned long long int cnt0 = af.vch0.size();
unsigned long long int cnt1 = af.vch1.size();


if( cnt0 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_raw() - ch1 is too big: %"LLU" for file filename: '%s'\n", cnt0, fname1.c_str() );
    return 0;
    }


if( cnt1 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_raw() - ch2 is too big: %"LLU" for file filename: '%s'\n", cnt1, fname1.c_str() );
    return 0;
    }


if( ( cnt0 == 0 ) && ( cnt1 == 0 ) )
    {
    if( verb ) logpf( "audio_formats::save_raw() - for filename: '%s', both channels are empty, no samples to write\n", fname1.c_str() );
    return 0;
    }


if( channels == 1 )                         //1 channel?
	{
    if( cnt0 == 0 )
        {
        if( verb ) logpf( "audio_formats::save_raw() - for filename: '%s', ch1 is empty, no samples to write\n", fname1.c_str() );
        return 0;
        }
    }

if( channels == 2 )
	{
    if( cnt0 < cnt1 )
        {
        if( verb ) logpf( "audio_formats::save_raw() - ch1 has less samples, will add zeroed samples to match ch2\n" );
        }

    if( cnt1 < cnt0 )
        {
        if( verb ) logpf( "audio_formats::save_raw() - ch2 has less samples, will add zeroed samples to match ch1\n" );
        }

    if( cnt0 < cnt1 )									//make both channels the same size, by increasing ch0
        {
        for( int i = cnt0; i < cnt1; i++ ) af.vch0.push_back( 0 );	
        }

    if( cnt1 < cnt0 )									//make both channels the same size, by increasing ch1
        {
        for( int i = cnt1; i < cnt0; i++ ) af.vch1.push_back( 0 );	
        }
    }


if( verb ) logpf( "audio_formats::save_raw() - denormalising using scale factor: %d\n", peak_int_val );


cnt0 = af.vch0.size();



FILE *fp = m1.mbc_fopen( fname1, "wb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::save_raw() - failed to open file for writing: '%s'\n", fname1.c_str() );
	return 0;
	}

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::save_raw() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return 0;
    }

int iaud;
double dd;
int wrote;

bool trk2 = 0;

unsigned int cnt = 0;
int j = 0;
for( int i = 0; i < cnt0; i++ )
	{
	repeat_for_trk2:
	if( !trk2 ) dd = af.vch0[ i ];
	else  dd = af.vch1[ i ];

	iaud = nearbyint( dd * (double)peak_int_val );			//v1.04
	
	if ( !af.is_big_endian )                    	//little endian?
		{
		buf[ j ] = iaud & 0xff;
		j++;
		buf[ j ] = ( iaud >> 8  ) & 0xff;
		j++;
		}
	else{                    						//big endian?
		buf[ j ] = ( iaud >> 8  ) & 0xff;
		j++;
		buf[ j ] =  iaud & 0xff;
		j++;
		}

	if( j >= cn_audio_formats_buf_siz )
		{
		wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;

		if( wrote != j )
			{
			if( verb ) logpf( "audio_formats::save_raw() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
			fclose( fp );
			return 0;
			}
		cnt += wrote;
		j = 0;
		}

	if( channels == 2 )
		{
		trk2 = !trk2;
		if( trk2 ) goto repeat_for_trk2;
		}
	}

if( j != 0 )
	{
	wrote = fwrite( buf, 1, j, fp );          //write block of audio data
	if( wrote != j )
		{
		if( verb ) logpf( "audio_formats::save_raw() - failed to complete a write to file: '%s', total bytes written: %"LLU"\n", fname1.c_str(), cnt );
        free( buf );
		fclose( fp );
		return 0;
		}
//wrote = j;
	cnt += wrote;
	}


if( verb ) logpf( "audio_formats::save_raw() - wrote %b bytes to file: '%s'\n", cnt, fname1.c_str() );

free( buf );
fclose( fp );

return 1;
}	




















//save a raw audio file using malloc double pointers: pch0, pch1 (4GB limit)
//supports 16 bit pcm - litte or big endian, 1 or 2 channels
//samples must be between +/- 1.0, saved samples are scaled up by 'peak_int_val'
bool audio_formats::save_raw_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;

string fname1 = path;
fname1 += fname;

int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::save_raw_malloc() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return 0;
    }




unsigned long long int cnt0 = sizech0;
unsigned long long int cnt1 = sizech1;


if( cnt0 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_raw_malloc() - ch1 is too big: %"LLU" for file filename: '%s'\n", cnt0, fname1.c_str() );
    return 0;
    }


if( cnt1 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_raw_malloc() - ch2 is too big: %"LLU" for file filename: '%s'\n", cnt1, fname1.c_str() );
    return 0;
    }


if( ( cnt0 == 0 ) && ( cnt1 == 0 ) )
    {
    if( verb ) logpf( "audio_formats::save_raw_malloc() - for filename: '%s', both channels are empty, no samples to write\n", fname1.c_str() );
    return 0;
    }


if( channels == 1 )                         //1 channel?
	{
    if( cnt0 == 0 )
        {
        if( verb ) logpf( "audio_formats::save_raw_malloc() - for filename: '%s', ch1 is empty, no samples to write\n", fname1.c_str() );
        return 0;
        }
    }

if( channels == 2 )
	{
    if( cnt0 < cnt1 )
        {
        if( verb ) logpf( "audio_formats::save_raw_malloc() - ch1 has less samples, will add zeroed samples to match ch2\n" );
        }

    if( cnt1 < cnt0 )
        {
        if( verb ) logpf( "audio_formats::save_raw_malloc() - ch2 has less samples, will add zeroed samples to match ch1\n" );
        }

    if( cnt0 < cnt1 )									//make both channels the same size, by increasing ch0
        {
        for( int i = cnt0; i < cnt1; i++ ) push_ch0( 0 );	
        }

    if( cnt1 < cnt0 )									//make both channels the same size, by increasing ch1
        {
        for( int i = cnt1; i < cnt0; i++ ) push_ch1( 0 );	
        }
    }


if( verb ) logpf( "audio_formats::save_raw_malloc() - denormalising using scale factor: %d\n", peak_int_val );


cnt0 = sizech0;



FILE *fp = m1.mbc_fopen( fname1, "wb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::save_raw_malloc() - failed to open file for writing: '%s'\n", fname1.c_str() );
	return 0;
	}

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::save_raw_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return 0;
    }

int iaud;
double dd;
int wrote;

bool trk2 = 0;

unsigned int cnt = 0;
int j = 0;
for( int i = 0; i < cnt0; i++ )
	{
	repeat_for_trk2:
	if( !trk2 ) dd = pch0[ i ];
	else  dd = pch1[ i ];

	iaud = nearbyint( dd * (double)peak_int_val );			//v1.04
	
	if ( !af.is_big_endian )                    	//little endian?
		{
		buf[ j ] = iaud & 0xff;
		j++;
		buf[ j ] = ( iaud >> 8  ) & 0xff;
		j++;
		}
	else{                    						//big endian?
		buf[ j ] = ( iaud >> 8  ) & 0xff;
		j++;
		buf[ j ] =  iaud & 0xff;
		j++;
		}

	if( j >= cn_audio_formats_buf_siz )
		{
		wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;

		if( wrote != j )
			{
			if( verb ) logpf( "audio_formats::save_raw_malloc() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
			fclose( fp );
			return 0;
			}
		cnt += wrote;
		j = 0;
		}

	if( channels == 2 )
		{
		trk2 = !trk2;
		if( trk2 ) goto repeat_for_trk2;
		}
	}

if( j != 0 )
	{
	wrote = fwrite( buf, 1, j, fp );          //write block of audio data
	if( wrote != j )
		{
		if( verb ) logpf( "audio_formats::save_raw_malloc() - failed to complete a write to file: '%s', total bytes written: %"LLU"\n", fname1.c_str(), cnt );
        free( buf );
		fclose( fp );
		return 0;
		}
//wrote = j;
	cnt += wrote;
	}


if( verb ) logpf( "audio_formats::save_raw_malloc() - wrote %b bytes to file: '%s'\n", cnt, fname1.c_str() );

free( buf );
fclose( fp );

return 1;
}	




















//load a raw audio file (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 32 bit single precision - litte or big endian, 1 or 2 channels
//specify byte order with flag: 'st_audio_formats_tag.is_big_endian'

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_raw_single_prec( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

string fname1 = path;
fname1 += fname;

vector<double> vch0;
vector<double> vch1;

af.vch0.clear();
af.vch1.clear();

int channels = af.channels;

int min_sample_idx = 0;
int max_sample_idx = 0;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return -6;
    }


unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }
    


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - filename: '%s' is empty\n", fname1.c_str() );
    return -1;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_raw_single_prec() - failed to open file for reading: '%s'\n", fname1.c_str()  );
	return -1;
	}

//if( verb ) logpf( "%"LLU"\n", fsz);
//return 1;

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }


//    af.format = en_af_raw;
//    af.srate = 0;
af.encoding = 0;
af.offset = 0;
//	af.channels = channels;
//  af.is_big_endian;
af.path = path;
af.fname = fname1;



unsigned long long int cnt = 0;
bool trk2 = 0;
while( 1 )
    {
    int read;

    read = fread( buf, 1, cn_audio_formats_buf_siz, fp );          //read block of audio data

    if( read == 0 )
        {
//        fclose( fp );
        break;
        }

    cnt += read;
    unsigned char uc;
    unsigned int ui,ui2;
    float *ff;
    
    for( int i = 0; i < read; )                      //process block
        {
        ui = 0;
        for( int j = 0; j < 4; j++ )
            {
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_raw_single_prec() - odd number of bytes encountered, lost final sample\n" );
                break;
                }

            if ( !af.is_big_endian )                    //little endian?
                {
                ui = ui >> 8;					        //shift down
                 ui2 = buf[ i ];
                ui2 = ui2 << 24;                        //shift up to bits 31->24       
                ui |= ui2;
                }
            else{                                       //big endian?
                ui = ui << 8;					        //shift up
                ui2 = buf[ i ];
                ui |= ui2;
                }
            i++;
            }

        ff = (float*) &ui;                              //effectively convert to a float
        dd = *ff;
//printf("%04d: ui: %x, dd: %f\n", i, ui, dd ); 
//getchar();
        
        if( channels == 1 )                         //1 trk?
            {
            vch0.push_back( dd );
            }
        else{
            if( trk2 == 0 ) vch0.push_back( dd );
            else vch1.push_back( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
       }

    }

if( verb )
	{
	logpf( "audio_formats::load_raw_single_prec() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_raw_single_prec() - sample: %06d has max: %g\n", max_sample_idx, max );
	}






double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < vch0.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    af.vch0.push_back( dd );
    }


for( int i = 0; i < vch1.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    af.vch1.push_back( dd );
    }

if( verb ) logpf( "audio_formats::load_raw_single_prec() - samples read (ch1: %u), (ch2: %u) from '%s'\n", af.vch0.size(), af.vch1.size(), fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_raw_single_prec() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}



















//load a raw audio file using malloc double pointers: pch0, pch1 (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 32 bit single precision - litte or big endian, 1 or 2 channels
//specify byte order with flag: 'st_audio_formats_tag.is_big_endian'

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_raw_single_prec_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

string fname1 = path;
fname1 += fname;


clear_ch0();
clear_ch1();

int channels = af.channels;

int min_sample_idx = 0;
int max_sample_idx = 0;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return -6;
    }


unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }
    


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - filename: '%s' is empty\n", fname1.c_str() );
    return -1;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - failed to open file for reading: '%s'\n", fname1.c_str()  );
	return -1;
	}

//if( verb ) logpf( "%"LLU"\n", fsz);
//return 1;

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }


//    af.format = en_af_raw;
//    af.srate = 0;
af.encoding = 0;
af.offset = 0;
//	af.channels = channels;
//  af.is_big_endian;
af.path = path;
af.fname = fname1;



unsigned long long int cnt = 0;
bool trk2 = 0;
while( 1 )
    {
    int read;

    read = fread( buf, 1, cn_audio_formats_buf_siz, fp );          //read block of audio data

    if( read == 0 )
        {
//        fclose( fp );
        break;
        }

    cnt += read;
    unsigned char uc;
    unsigned int ui,ui2;
    float *ff;
    
    for( int i = 0; i < read; )                      //process block
        {
        ui = 0;
        for( int j = 0; j < 4; j++ )
            {
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - odd number of bytes encountered, lost final sample\n" );
                break;
                }

            if ( !af.is_big_endian )                    //little endian?
                {
                ui = ui >> 8;					        //shift down
                 ui2 = buf[ i ];
                ui2 = ui2 << 24;                        //shift up to bits 31->24       
                ui |= ui2;
                }
            else{                                       //big endian?
                ui = ui << 8;					        //shift up
                ui2 = buf[ i ];
                ui |= ui2;
                }
            i++;
            }

        ff = (float*) &ui;                              //effectively convert to a float
        dd = *ff;
//printf("%04d: ui: %x, dd: %f\n", i, ui, dd ); 
//getchar();
        
        if( channels == 1 )                         //1 trk?
            {
            push_ch0( dd );
            }
        else{
            if( trk2 == 0 ) push_ch0( dd );
            else push_ch1( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
       }

    }

if( verb )
	{
	logpf( "audio_formats::load_raw_single_prec_malloc() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_raw_single_prec_malloc() - sample: %06d has max: %g\n", max_sample_idx, max );
	}






double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < sizech0; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    pch0[ i ] = dd;
    }


for( int i = 0; i < sizech1; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    pch1[ i ] = dd;
    }

if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - samples read (ch1: %u), (ch2: %u) from '%s'\n", sizech0, sizech1, fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_raw_single_prec_malloc() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}

















//load a raw audio file (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 64 bit double precision - litte or big endian, 1 or 2 channels
//specify byte order with flag: 'st_audio_formats_tag.is_big_endian'

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_raw_double_prec( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

string fname1 = path;
fname1 += fname;

vector<double> vch0;
vector<double> vch1;

af.vch0.clear();
af.vch1.clear();


int channels = af.channels;

int min_sample_idx = 0;
int max_sample_idx = 0;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return -6;
    }


unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }
    


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - file size is too big: %"LLU" for file filename: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_raw_double_prec() - failed to open file for reading: '%s'\n", fname1.c_str()  );
	return -1;
	}

//if( verb ) logpf( "%"LLU"\n", fsz);
//return 1;


unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }



//    af.format = en_af_raw;
//    af.srate = 0;
af.encoding = 0;
af.offset = 0;
//	af.channels = channels;
//  af.is_big_endian;
af.path = path;
af.fname = fname1;



unsigned long long int cnt = 0;
bool trk2 = 0;
while( 1 )
    {
    int read;

    read = fread( buf, 1, cn_audio_formats_buf_siz, fp );       //read block of audio data

    if( read == 0 )
        {
//        fclose( fp );
        break;
        }

    cnt += read;
    unsigned char uc;
    unsigned long long int ulli, ulli2;
    double *dptr;
    for( int i = 0; i < read; )                         //process block
        {
        ulli = 0;
        for( int j = 0; j < 8; j++ )
            {
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_raw_double_prec() - odd number of bytes encountered, lost final sample\n" );
                break;
                }

            if ( !af.is_big_endian )                    //little endian?
                {
                ulli = ulli >> 8;					    //shift down
                ulli2 = buf[ i ];
                ulli2 = ulli2 << 56;                    //shift up to bits 56->48       
                ulli |= ulli2;
                }
            else{                                       //big endian?
                ulli = ulli << 8;					    //shift up
                ulli2 = buf[ i ];
                ulli |= ulli2;
                }
            i++;
            }

        dptr = (double*) &ulli;                         //effectively convert to a double
        dd = *dptr;
//printf("%04d: ui: %x, dd: %f\n", i, ui, dd ); 
//getchar();
        
        if( channels == 1 )                             //1 trk?
            {
            vch0.push_back( dd );
            }
        else{
            if( trk2 == 0 ) vch0.push_back( dd );
            else vch1.push_back( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
       }

    }

if( verb )
	{
	logpf( "audio_formats::load_raw_double_prec() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_raw_double_prec() - sample: %06d has max: %g\n", max_sample_idx, max );
	}






double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                       //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < vch0.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    af.vch0.push_back( dd );
    }


for( int i = 0; i < vch1.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    af.vch1.push_back( dd );
    }

if( verb ) logpf( "audio_formats::load_raw_double_prec() - samples read (ch1: %u), (ch2: %u) from '%s'\n", af.vch0.size(), af.vch1.size(), fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_raw_double_prec() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}














//load a raw audio file using malloc double pointers: pch0, pch1 (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 64 bit double precision - litte or big endian, 1 or 2 channels
//specify byte order with flag: 'st_audio_formats_tag.is_big_endian'

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_raw_double_prec_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

string fname1 = path;
fname1 += fname;

clear_ch0();
clear_ch1();


int channels = af.channels;

int min_sample_idx = 0;
int max_sample_idx = 0;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return -6;
    }


unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }
    


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - file size is too big: %"LLU" for file filename: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - failed to open file for reading: '%s'\n", fname1.c_str()  );
	return -1;
	}

//if( verb ) logpf( "%"LLU"\n", fsz);
//return 1;


unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }



//    af.format = en_af_raw;
//    af.srate = 0;
af.encoding = 0;
af.offset = 0;
//	af.channels = channels;
//  af.is_big_endian;
af.path = path;
af.fname = fname1;



unsigned long long int cnt = 0;
bool trk2 = 0;
while( 1 )
    {
    int read;

    read = fread( buf, 1, cn_audio_formats_buf_siz, fp );       //read block of audio data

    if( read == 0 )
        {
//        fclose( fp );
        break;
        }

    cnt += read;
    unsigned char uc;
    unsigned long long int ulli, ulli2;
    double *dptr;
    for( int i = 0; i < read; )                         //process block
        {
        ulli = 0;
        for( int j = 0; j < 8; j++ )
            {
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - odd number of bytes encountered, lost final sample\n" );
                break;
                }

            if ( !af.is_big_endian )                    //little endian?
                {
                ulli = ulli >> 8;					    //shift down
                ulli2 = buf[ i ];
                ulli2 = ulli2 << 56;                    //shift up to bits 56->48       
                ulli |= ulli2;
                }
            else{                                       //big endian?
                ulli = ulli << 8;					    //shift up
                ulli2 = buf[ i ];
                ulli |= ulli2;
                }
            i++;
            }

        dptr = (double*) &ulli;                         //effectively convert to a double
        dd = *dptr;
//printf("%04d: ui: %x, dd: %f\n", i, ui, dd ); 
//getchar();
        
        if( channels == 1 )                             //1 trk?
            {
            push_ch0( dd );
            }
        else{
            if( trk2 == 0 ) push_ch0( dd );
            else push_ch1( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
       }

    }

if( verb )
	{
	logpf( "audio_formats::load_raw_double_prec_malloc() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_raw_double_prec_malloc() - sample: %06d has max: %g\n", max_sample_idx, max );
	}






double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                       //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < sizech0; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    pch0[ i ] = dd;
    }


for( int i = 0; i < sizech1; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    pch1[ i ] = dd;
    }

if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - samples read (ch1: %u), (ch2: %u) from '%s'\n", sizech0, sizech1, fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_raw_double_prec_malloc() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}















//load a raw audio file (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 16 bit pcm - litte or big endian, 1 or 2 channels
//specify byte order with flag: 'st_audio_formats_tag.is_big_endian'

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_raw_pcm( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

string fname1 = path;
fname1 += fname;

vector<double> vch0;
vector<double> vch1;

af.vch0.clear();
af.vch1.clear();


int channels = af.channels;

int min_sample_idx = 0;
int max_sample_idx = 0;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return -6;
    }


unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }
    


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }





if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_raw_pcm() - failed to open file for reading: '%s'\n", fname1.c_str()  );
	return -1;
	}

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }

//if( verb ) logpf( "%"LLU"\n", fsz);
//return 1;



//    af.format = en_af_raw;
//    af.srate = 0;
af.encoding = 0;
af.offset = 0;
//	af.channels = channels;
//  af.is_big_endian;
af.path = path;
af.fname = fname1;



unsigned long long int cnt = 0;
bool trk2 = 0;
while( 1 )
    {
    int read;

    read = fread( buf, 1, cn_audio_formats_buf_siz, fp );          //read block of audio data

    if( read == 0 )
        {
//        fclose( fp );
        break;
        }

    cnt += read;
    char cc;
    int iaud;

    for( int i = 0; i < read; )                      //process block
        {

        if ( !af.is_big_endian )                    //little endian?
            {
            i++;
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_raw_pcm() - odd number of bytes encountered, lost final sample\n" );
                break;
                }
            cc = buf[ i ];						    //preserve sign in msbyte

            iaud = cc * 256;						//shift msbyte to its location preserving sign
            iaud |= buf[ i - 1 ];					//or in lsbyte
            dd = iaud;
            i++;
//            if( i == 0x9c2 )
//                {
//                logpf("%x, %x, %x\n, %f", buf[ 0x9c1 ], buf[ 0x9c0 ], iaud, dd );						    //preserve sign in msbyte
//                return 1;
//                
//                }

            }
        else{                                       //big endian?
            cc = buf[ i ];							//preserve sign in msbyte
            iaud = cc * 256;						//shift msbyte to its location preserving sign
        
            i++;
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_raw_pcm() - odd number of bytes encountered, lost final sample\n" );
                break;
                }
            iaud |= buf[ i ];					//or in lsbyte
            dd = iaud;
            i++;
            }
        
        if( channels == 1 )                         //1 trk?
            {
            vch0.push_back( dd );
            }
        else{
            if( trk2 == 0 ) vch0.push_back( dd );
            else vch1.push_back( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
       }

    }

if( verb )
	{
	logpf( "audio_formats::load_raw_pcm() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_raw_pcm() - sample: %06d has max: %g\n", max_sample_idx, max );
	}






double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_raw_pcm() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_raw_pcm() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < vch0.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    af.vch0.push_back( dd );
    }


for( int i = 0; i < vch1.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    af.vch1.push_back( dd );
    }

if( verb ) logpf( "audio_formats::load_raw_pcm() - samples read (ch1: %u), (ch2: %u) from '%s'\n", af.vch0.size(), af.vch1.size(), fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_raw_pcm() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}













//load a raw audio file (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 16 bit pcm - litte or big endian, 1 or 2 channels
//specify byte order with flag: 'st_audio_formats_tag.is_big_endian'

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::load_raw_pcm_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

string fname1 = path;
fname1 += fname;


clear_ch0();
clear_ch1();


int channels = af.channels;

int min_sample_idx = 0;
int max_sample_idx = 0;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return -6;
    }


unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }
    


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }





if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - failed to open file for reading: '%s'\n", fname1.c_str()  );
	return -1;
	}

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }

//if( verb ) logpf( "%"LLU"\n", fsz);
//return 1;



//    af.format = en_af_raw;
//    af.srate = 0;
af.encoding = 0;
af.offset = 0;
//	af.channels = channels;
//  af.is_big_endian;
af.path = path;
af.fname = fname1;



unsigned long long int cnt = 0;
bool trk2 = 0;
while( 1 )
    {
    int read;

    read = fread( buf, 1, cn_audio_formats_buf_siz, fp );          //read block of audio data

    if( read == 0 )
        {
//        fclose( fp );
        break;
        }

    cnt += read;
    char cc;
    int iaud;

    for( int i = 0; i < read; )                      //process block
        {

        if ( !af.is_big_endian )                    //little endian?
            {
            i++;
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - odd number of bytes encountered, lost final sample\n" );
                break;
                }
            cc = buf[ i ];						    //preserve sign in msbyte

            iaud = cc * 256;						//shift msbyte to its location preserving sign
            iaud |= buf[ i - 1 ];					//or in lsbyte
            dd = iaud;
            i++;
//            if( i == 0x9c2 )
//                {
//                logpf("%x, %x, %x\n, %f", buf[ 0x9c1 ], buf[ 0x9c0 ], iaud, dd );						    //preserve sign in msbyte
//                return 1;
//                
//                }

            }
        else{                                       //big endian?
            cc = buf[ i ];							//preserve sign in msbyte
            iaud = cc * 256;						//shift msbyte to its location preserving sign
        
            i++;
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - odd number of bytes encountered, lost final sample\n" );
                break;
                }
            iaud |= buf[ i ];					//or in lsbyte
            dd = iaud;
            i++;
            }
        
        if( channels == 1 )                         //1 trk?
            {
            push_ch0( dd );
            }
        else{
            if( trk2 == 0 ) push_ch0( dd );
            else push_ch1( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
       }

    }

if( verb )
	{
	logpf( "audio_formats::load_raw_pcm_malloc() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_raw_pcm_malloc() - sample: %06d has max: %g\n", max_sample_idx, max );
	}






double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;


for( int i = 0; i < sizech0; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

    pch0[ i ] = dd;
    }


for( int i = 0; i < sizech1; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    pch1[ i ] = dd;
    }

if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - samples read (ch1: %u), (ch2: %u) from '%s'\n", sizech0, sizech1, fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_raw_pcm_malloc() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}


















/*
** This routine converts from linear to ulaw
**
** Craig Reese: IDA/Supercomputing Research Center
** Joe Campbell: Department of Defense
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) "A New Digital Technique for Implementation of Any
**     Continuous PCM Companding Law," Villeret, Michel,
**     et al. 1973 IEEE Int. Conf. on Communications, Vol 1,
**     1973, pg. 11.12-11.17
** 3) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: Signed 16 bit linear sample
** Output: 8 bit ulaw sample
*/

#define ZEROTRAP    /* turn on the trap as per the MIL-STD */
#define BIAS 0x84   /* define the add-in bias for 16 bit samples */
#define CLIP 32635

unsigned char audio_formats::linear2ulaw( int sample )
{

  static int exp_lut[256] = {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
                             4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                             7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};
  int sign, exponent, mantissa;
  unsigned char ulawbyte;

  /* Get the sample into sign-magnitude. */
  sign = (sample >> 8) & 0x80;		/* set aside the sign */
  if (sign != 0) sample = -sample;		/* get magnitude */
  if (sample > CLIP) sample = CLIP;		/* clip the magnitude */

  /* Convert from 16 bit linear to ulaw. */
  sample = sample + BIAS;
  exponent = exp_lut[(sample >> 7) & 0xFF];
  mantissa = (sample >> (exponent + 3)) & 0x0F;
  ulawbyte = ~(sign | (exponent << 4) | mantissa);
#ifdef ZEROTRAP
  if (ulawbyte == 0) ulawbyte = 0x02;	/* optional CCITT trap */
#endif

  return(ulawbyte);
}

/*
** This routine converts from ulaw to 16 bit linear.
**
** Craig Reese: IDA/Supercomputing Research Center
** 29 September 1989
**
** References:
** 1) CCITT Recommendation G.711  (very difficult to follow)
** 2) MIL-STD-188-113,"Interoperability and Performance Standards
**     for Analog-to_Digital Conversion Techniques,"
**     17 February 1987
**
** Input: 8 bit ulaw sample
** Output: signed 16 bit linear sample
*/

int audio_formats::ulaw2linear( unsigned char ulawbyte )
{
  static int exp_lut[8] = {0,132,396,924,1980,4092,8316,16764};
  int sign, exponent, mantissa, sample;

  ulawbyte = ~ulawbyte;
  sign = (ulawbyte & 0x80);
  exponent = (ulawbyte >> 4) & 0x07;
  mantissa = ulawbyte & 0x0F;
  sample = exp_lut[exponent] + (mantissa << (exponent + 3));
  if (sign != 0) sample = -sample;

  return(sample);
}








//load a sun au formatted audio file (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 8 bit u-law - encoding 1, 1 or 2 channel
//supports 16 bit pcm - encoding 3, 1 or 2 channel

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated

int audio_formats::load_sun( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;
vector<double> vaud;
vector<double> vch0;
vector<double> vch1;

string fname1 = path;
fname1 += fname;


af.vch0.clear();
af.vch1.clear();

bool verb2 = 0;

if( verb2 ) printf("load_sun() - step 1\n" );


unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_sun() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_sun() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_sun() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }



FILE *fp;

fp = fopen( fname1.c_str(),"rb" );		    	//open file


if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_sun() - failed to open file for reading: '%s'\n", fname1.c_str() );
	return -1;
	}

if( verb2 ) printf("load_sun() - step 2\n" );


unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_sun() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }
    

unsigned int need = 24;                                      //this is minimum, there is an optional 'annotation field' that can be straight after the 24th byte

int read = fread( buf, 1 ,need, fp );             //read 6 x 32bit to get sun's au audio file header

if( verb2 ) printf("load_sun() - step 3\n" );


if( read != need )
    {
    free( buf );
    fclose( fp );
    if( verb ) logpf( "audio_formats::load_sun() - sun's au audio header not big enough for file: '%s', only read: %u\n", fname1.c_str(), read );
    return -4;
    }

//work out offset
unsigned int offset = buf[ 7 ];                     //0x1
offset += buf[ 6 ] * 256;                           //0x100
offset += buf[ 5 ] * 65536;                         //0x10000
offset += buf[ 4 ] * 16777216;                      //0x1000000
//    cslpf( 0, "offset= %d\n", offset );

//number of bytes of audio data
//note -- this can be set to 0xffffffff, if the data size is unknown
unsigned int data_block_size = buf[ 0xb ];          //0x1
data_block_size += buf[ 0xa ] * 256;                //0x100
data_block_size += buf[ 9 ] * 65536;                //0x10000
data_block_size += buf[ 8 ] * 16777216;             //0x1000000
//    cslpf( 0, "data_block_size= %d\n", data_block_size );


//format of audio
unsigned int encoding = buf[ 0xf ];                 //0x1
encoding += buf[ 0xe ] * 256;                       //0x100
encoding += buf[ 0xd ] * 65536;                     //0x10000
encoding += buf[ 0xc ] * 16777216;                  //0x1000000
//    cslpf( 0, "encoding= %d\n", encoding );

//samplerate of audio
unsigned int srate = buf[ 0x13 ];                   //0x1
srate += buf[ 0x12 ] * 256;                         //0x100
srate += buf[ 0x11 ] * 65536;                       //0x10000
srate += buf[ 0x10 ] * 16777216;                    //0x1000000
//    cslpf( 0, "srate= %d\n", srate );

//channels of audio
unsigned int channels = buf[ 0x17 ];                   //0x1
channels += buf[ 0x16 ] * 256;                         //0x100
channels += buf[ 0x15 ] * 65536;                       //0x10000
channels += buf[ 0x14 ] * 16777216;                    //0x1000000
//    cslpf( 0, "channels= %d\n", channels );

if( verb2 ) printf("load_sun() - step 4\n" );


bool supported = 0;
if( encoding == 1 )                                     //8 bit u-law
    {
    supported = 1;
    af.bits_per_sample = 8;
    }
    
if( encoding == 3 )                                     //16 bit pcm
    {
    supported = 1;
    af.bits_per_sample = 16;
    }



if( !supported )
    {
    if( verb ) logpf( "audio_formats::load_sun() - un-supported encoding: %u, for file: '%s', %u\n", encoding, fname1.c_str() );
    free( buf );
    fclose( fp );
    return -5;
    }


if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_sun() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    free( buf );
    fclose( fp );
    return -6;
    }


af.path = path;
af.fname = fname1;
af.srate = srate;
af.encoding = encoding;
af.offset = offset;
af.channels = channels;
af.is_big_endian = 1;


if( verb ) logpf( "audio_formats::load_sun() - samplerate: %u, for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::load_sun() - channels: %u found in file: '%s'\n", af.channels, fname1.c_str() );

if( verb ) if( af.encoding == 1 ) logpf( "audio_formats::load_sun() - encoding is 8 bit u-law\n" );
if( verb ) if( af.encoding == 3 ) logpf( "audio_formats::load_sun() - encoding is 16 bit pcm, big endian\n" );

if( verb2 ) printf("load_sun() - step 5\n" );


if( verb2 ) printf("load_sun() - encoding: %d\n", encoding  );
if( verb2 ) printf("load_sun() - channels: %d\n", channels  );
if( verb2 ) printf("load_sun() - data_block_size: %u\n", data_block_size  );


int skip = offset - 4 * 6;

if ( skip < 0 ) skip = 0;
for( int i = 0; i < skip; i++ )					    //step to first sample byte, this will skip optional 'annotation field' if it exists
    {
    int read = fread( buf, 1, 1, fp );
    }


unsigned int byte_cnt = data_block_size;
unsigned int cnt = 0;
unsigned int min_sample_idx;
unsigned int max_sample_idx;


//logpf("byte_cnt: %u\n", byte_cnt );

double dd;
bool trk2 = 0;
char cc;
int iaud;
int i = 0;
while( 1 )
	{
    int read;

	need = byte_cnt - cnt;
	if( need > cn_audio_formats_buf_siz ) need = cn_audio_formats_buf_siz;
	
    read = fread( buf, 1, need, fp );          				//read block of audio data
if( verb2 ) printf("load_sun() - read %d\n", read );
	if ( read == 0 ) break;
	cnt += read;

	
	if( read != need )
		{
		if( verb ) logpf( "audio_formats::load_sun() - tried to read: %u bytes, only got %u bytes for file: '%s'\n", need, read, fname1.c_str() );
		}


    for( int i = 0; i < read; )                 //process block
        {
        if( encoding == 1 )                     //u-law?
            {    
            cc = buf[ i ];					    //preserve sign in msbyte (little endia)
            dd = ulaw2linear( cc );
            i++;
            }


        if( encoding == 3 )                     //16 bit pcm, big endian
            {
            dd = (char)buf[ i ] * 256.0;        //preserve sign
            i++;
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_sun() - odd number of bytes encountered, lost final sample\n" );
                break;
                }
            dd += buf[ i ];
            i++;
            }



        if( channels == 1 )                     //1 trk?
            {
            vch0.push_back( dd );
            }
        else{
            if( trk2 == 0 ) vch0.push_back( dd );
            else vch1.push_back( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
        }
	}

//if( verb ) printf("load_sun() - done1\n", read );

if( verb2 ) printf("load_sun() - read ok: %u\n", cnt );

if( verb2 ) printf("load_sun() - af.vch0 size: %u, limit size: %u\n", vch0.size(), af.vch0.max_size() );
if( verb2 ) printf("load_sun() - af.vch1 size: %u, limit size: %u\n", vch1.size(), af.vch1.max_size() );

if( verb )
	{
	logpf( "audio_formats::load_sun() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_sun() - sample: %06d has max: %g\n", max_sample_idx, max );
	}




double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_sun() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_sun() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;

//if( verb ) printf("load_sun() - done4\n", read );

if( verb2 ) printf("load_sun() - normalising ch1 - samples to do: %u\n", vch0.size() );

//af.vch0.reserve( vch0.size() );
//af.vch1.clear();


unsigned int std_vector_limit;					//16777216 (16MB) limit on Windos see vector::max_size()

std_vector_limit = 12 * 1024 * 1024;			//safty margin for Windos, otherwise app has a Runtime crash and no debugger stack is available
if( verb2 ) printf("load_sun() - imposed a reduced std_vector_limit: %u\n", std_vector_limit );



for( unsigned int i = 0; i < vch0.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;

/*
	if( verb2 )
		{
//		if( i > 16770000 )
//			{
//			printf("load_sun() - sample %u, capacity: %u,   %u\n", i, af.vch0.capacity(), std_vector_limit );
//			}
	
		if( !(i % 10000 ) )
			{
			printf("load_sun() - sample %u\n", i );
			}

		}
*/

	if( i >= std_vector_limit )		//vector has a 
		{
		printf("load_sun() - af.vch0 sample cnt: %u, std_vector_limit: %u, dropping %u samples\n", af.vch0.size(), std_vector_limit, vch0.size() - std_vector_limit );
		break;
		}

    af.vch0.push_back( dd );
    }



if( verb2 ) printf("load_sun() - normalising ch2 - samples to do: %u\n", vch1.size() );

//if(  vch0.size() >= af.vch0.max_size() )
//	{
//	printf("load_sun() - af.vch0 size: %u, is passed limit: %u\n", vch0.size(), af.vch0.max_size() );
//	return 0;
//	}

//af.vch1.reserve( vch1.size() );
//af.vch1.clear();


for( unsigned int i = 0; i < vch1.size(); i++ )                      //scale wfm to max +/- 1.0
    {
    dd = vch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

	if( i > std_vector_limit )
		{
		printf("load_sun() - af.vch1 sample cnt: %u, std_vector_limit: %u, dropping %u samples\n", af.vch1.size(), std_vector_limit, vch1.size() - std_vector_limit );
		break;
		}

    af.vch1.push_back( dd );
    }

if( verb ) logpf( "audio_formats::load_sun() - samples read (ch1: %u), (ch2: %u) from '%s'\n", af.vch0.size(), af.vch1.size(), fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_sun() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_sun() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }

if( verb2 ) printf("load_sun() - closing file\n" );


//if( verb ) printf("load_sun() - done5\n", read );

free( buf );
fclose( fp );
return 1;
}













//load a sun au formatted audio file using malloc double pointers: pch0, pch1 (4GB limit), normalises audio between +/- 1.0 (nomalises only if peak_int_val == 0)
//supports 8 bit u-law - encoding 1, 1 or 2 channel
//supports 16 bit pcm - encoding 3, 1 or 2 channel

//if 'scale_down_by_peak_int_val' is non zero normalising is not performed, instead file audio sample levels are divided by 'scale_down_by_peak_int_val'
//returns 1 on success
//returns -1 file not openable or not found
//returns -2 file too big
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated

int audio_formats::load_sun_malloc( string path, string fname, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
string s1;
double min = 0;                                     //for normalising audio
double max = 0;

string fname1 = path;
fname1 += fname;


clear_ch0();
clear_ch1();



unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::load_sun_malloc() - failed to open filename: '%s'\n", fname1.c_str() );
    return -1;
    }


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::load_sun_malloc() - file size is too big: %"LLU" for file: '%s'\n", fsz, fname1.c_str() );
    return -2;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::load_sun_malloc() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }



FILE *fp;

fp = fopen( fname1.c_str(),"rb" );		    	//open file


if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::load_sun_malloc() - failed to open file for reading: '%s'\n", fname1.c_str() );
	return -1;
	}



unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::load_sun_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return -7;
    }
    

unsigned int need = 24;                                      //this is minimum, there is an optional 'annotation field' that can be straight after the 24th byte

int read = fread( buf, 1 ,need, fp );             //read 6 x 32bit to get sun's au audio file header



if( read != need )
    {
    free( buf );
    fclose( fp );
    if( verb ) logpf( "audio_formats::load_sun_malloc() - sun's au audio header not big enough for file: '%s', only read: %u\n", fname1.c_str(), read );
    return -4;
    }

//work out offset
unsigned int offset = buf[ 7 ];                     //0x1
offset += buf[ 6 ] * 256;                           //0x100
offset += buf[ 5 ] * 65536;                         //0x10000
offset += buf[ 4 ] * 16777216;                      //0x1000000
//    cslpf( 0, "offset= %d\n", offset );

//number of bytes of audio data
//note -- this can be set to 0xffffffff, if the data size is unknown
unsigned int data_block_size = buf[ 0xb ];          //0x1
data_block_size += buf[ 0xa ] * 256;                //0x100
data_block_size += buf[ 9 ] * 65536;                //0x10000
data_block_size += buf[ 8 ] * 16777216;             //0x1000000
//    cslpf( 0, "data_block_size= %d\n", data_block_size );


//format of audio
unsigned int encoding = buf[ 0xf ];                 //0x1
encoding += buf[ 0xe ] * 256;                       //0x100
encoding += buf[ 0xd ] * 65536;                     //0x10000
encoding += buf[ 0xc ] * 16777216;                  //0x1000000
//    cslpf( 0, "encoding= %d\n", encoding );

//samplerate of audio
unsigned int srate = buf[ 0x13 ];                   //0x1
srate += buf[ 0x12 ] * 256;                         //0x100
srate += buf[ 0x11 ] * 65536;                       //0x10000
srate += buf[ 0x10 ] * 16777216;                    //0x1000000
//    cslpf( 0, "srate= %d\n", srate );

//channels of audio
unsigned int channels = buf[ 0x17 ];                   //0x1
channels += buf[ 0x16 ] * 256;                         //0x100
channels += buf[ 0x15 ] * 65536;                       //0x10000
channels += buf[ 0x14 ] * 16777216;                    //0x1000000
//    cslpf( 0, "channels= %d\n", channels );



bool supported = 0;
if( encoding == 1 )                                     //8 bit u-law
    {
    supported = 1;
    af.bits_per_sample = 8;
    }
    
if( encoding == 3 )                                     //16 bit pcm
    {
    supported = 1;
    af.bits_per_sample = 16;
    }



if( !supported )
    {
    if( verb ) logpf( "audio_formats::load_sun_malloc() - un-supported encoding: %u, for file: '%s', %u\n", encoding, fname1.c_str() );
    free( buf );
    fclose( fp );
    return -5;
    }


if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::load_sun_malloc() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    free( buf );
    fclose( fp );
    return -6;
    }


af.path = path;
af.fname = fname1;
af.srate = srate;
af.encoding = encoding;
af.offset = offset;
af.channels = channels;
af.is_big_endian = 1;


if( verb ) logpf( "audio_formats::load_sun_malloc() - samplerate: %u, for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::load_sun_malloc() - channels: %u found in file: '%s'\n", af.channels, fname1.c_str() );

if( verb ) if( af.encoding == 1 ) logpf( "audio_formats::load_sun_malloc() - encoding is 8 bit u-law\n" );
if( verb ) if( af.encoding == 3 ) logpf( "audio_formats::load_sun_malloc() - encoding is 16 bit pcm, big endian\n" );


int skip = offset - 4 * 6;

if ( skip < 0 ) skip = 0;
for( int i = 0; i < skip; i++ )					    //step to first sample byte, this will skip optional 'annotation field' if it exists
    {
    int read = fread( buf, 1, 1, fp );
    }


unsigned int byte_cnt = data_block_size;
unsigned int cnt = 0;
unsigned int min_sample_idx;
unsigned int max_sample_idx;


//logpf("byte_cnt: %u\n", byte_cnt );

double dd;
bool trk2 = 0;
char cc;
int iaud;
int i = 0;
while( 1 )
	{
    int read;

	need = byte_cnt - cnt;
	if( need > cn_audio_formats_buf_siz ) need = cn_audio_formats_buf_siz;
	
    read = fread( buf, 1, need, fp );          				//read block of audio data
	if ( read == 0 ) break;
	cnt += read;

	
	if( read != need )
		{
		if( verb ) logpf( "audio_formats::load_sun_malloc() - tried to read: %u bytes, only got %u bytes for file: '%s'\n", need, read, fname1.c_str() );
		}


    for( int i = 0; i < read; )                 //process block
        {
        if( encoding == 1 )                     //u-law?
            {    
            cc = buf[ i ];					    //preserve sign in msbyte (little endia)
            dd = ulaw2linear( cc );
            i++;
            }


        if( encoding == 3 )                     //16 bit pcm, big endian
            {
            dd = (char)buf[ i ] * 256.0;        //preserve sign
            i++;
            if( i >= read )
                {
                if( verb ) logpf( "audio_formats::load_sun_malloc() - odd number of bytes encountered, lost final sample\n" );
                break;
                }
            dd += buf[ i ];
            i++;
            }



        if( channels == 1 )                     //1 trk?
            {
            push_ch0( dd );
            }
        else{
            if( trk2 == 0 ) push_ch0( dd );
            else push_ch1( dd );
            trk2 = !trk2;
            }

		if( dd < min ) { min = dd; min_sample_idx = i; }
		if( dd > max ) { max = dd; max_sample_idx = i; }
        }
	}

//if( verb ) printf("load_sun() - done1\n", read );


if( verb )
	{
	logpf( "audio_formats::load_sun_malloc() - sample: %06d has min: %g\n", min_sample_idx, min );
	logpf( "audio_formats::load_sun_malloc() - sample: %06d has max: %g\n", max_sample_idx, max );
	}




double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }

if( scale_down_by_peak_int_val != 0 )                                     //no normalising?, use a fixed scale spec by user
    {
    scale = 1.0 / scale_down_by_peak_int_val;
    if( verb ) logpf( "audio_formats::load_sun_malloc() - will not normalise, down scaling by: %d\n", scale_down_by_peak_int_val );
    }
else{
    if( verb ) logpf( "audio_formats::load_sun_malloc() - will normalise, down scaling by: %f\n", scale );
    }

double min2 = 0;
double max2 = 0;
double min3 = 0;
double max3 = 0;






for( unsigned int i = 0; i < sizech0; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch0[ i ] * scale;


    if( dd < min2 ) min2 = dd;
    if( dd > max2 ) max2 = dd;


    pch0[ i ] = dd;
    }


for( unsigned int i = 0; i < sizech1; i++ )                      //scale wfm to max +/- 1.0
    {
    dd = pch1[ i ] * scale;


    if( dd < min3 ) min3 = dd;
    if( dd > max3 ) max3 = dd;

    pch1[ i ] = dd;
    }

if( verb ) logpf( "audio_formats::load_sun_malloc() - samples read (ch1: %u), (ch2: %u) from '%s'\n", sizech0, sizech1, fname1.c_str() );

if( scale_down_by_peak_int_val == 0 )
    {
    if( verb ) logpf( "audio_formats::load_sun_malloc() - ch1 wfm (normalised) min: %f, max: %f\n", min2, max2 );
    if( verb ) logpf( "audio_formats::load_sun_malloc() - ch2 wfm (normalised) min: %f, max: %f\n", min3, max3 );
    }


free( buf );
fclose( fp );
return 1;
}


















//save a wav audio file (only PCM 16 bit, limited to 4GB)
//supports 16 bit pcm, 1 or 2 channels
//samples must be between +/- 1.0, saved samples are scaled up by peak_int_val
bool audio_formats::save_wav( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;
double dd;


string fname1 = path;
fname1 += fname;


int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::save_wav() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return 0;
    }




{
unsigned long long int cnt0 = af.vch0.size();
unsigned long long int cnt1 = af.vch1.size();


if( cnt0 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_wav() - ch1 is too big: %"LLU" for file filename: '%s'\n", cnt0, fname1.c_str() );
    return 0;
    }


if( cnt1 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_wav() - ch2 is too big: %"LLU" for file filename: '%s'\n", cnt1, fname1.c_str() );
    return 0;
    }
}








unsigned int cnt0 = af.vch0.size();
unsigned int cnt1 = af.vch1.size();

if( ( cnt0 == 0 ) && ( cnt1 == 0 ) )
    {
    if( verb ) logpf( "audio_formats::save_wav() - for filename: '%s', both channels are empty, no samples to write\n", fname1.c_str() );
    return 0;
    }


if( channels == 1 )                         //1 channel?
	{
    if( cnt0 == 0 )
        {
        if( verb ) logpf( "audio_formats::save_wav() - for filename: '%s', ch1 is empty, no samples to write\n", fname1.c_str() );
        return 0;
        }
    }

if( channels == 2 )
	{
    if( cnt0 < cnt1 )
        {
        if( verb ) logpf( "audio_formats::save_wav() - ch1 has less samples, will add zeroed samples to match ch2\n" );
        }

    if( cnt1 < cnt0 )
        {
        if( verb ) logpf( "audio_formats::save_wav() - ch2 has less samples, will add zeroed samples to match ch1\n" );
        }

    if( cnt0 < cnt1 )									//make both channels the same size, by increasing ch0
        {
        for( int i = cnt0; i < cnt1; i++ ) af.vch0.push_back( 0 );	
        }

    if( cnt1 < cnt0 )									//make both channels the same size, by increasing ch1
        {
        for( int i = cnt1; i < cnt0; i++ ) af.vch1.push_back( 0 );	
        }
    }


if( verb ) logpf( "audio_formats::save_wav() - denormalising using scale factor: %d\n", peak_int_val );


cnt0 = af.vch0.size() * channels;


FILE *fp = m1.mbc_fopen( fname1, "wb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::save_wav() - failed to open file for writing: '%s'\n", fname1.c_str() );
	return 0;
	}



unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::save_wav() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return 0;
    }



//save wav header
strncpy( (char*)(buf + cn_riff_hdr_ckid), "RIFF", 4 );


unsigned int data_chunk_cksize = cnt0 * 2;						//size of all the audio sample data in bytes
unsigned int tot_chunk_size = data_chunk_cksize + 36;			//this number is 4 less than the total wav filesize


*(unsigned int*)( buf + cn_riff_hdr_cksize ) = tot_chunk_size;


strncpy( (char*)( buf + cn_riff_hdr_waveid ), "WAVE", 4 );




strncpy( (char*)( buf + 12 + cn_riff_fmt_chunk_ckid ), "fmt ", 4 );            //store 'fmt '


*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_cksize ) = 16;                 //always make the 'fmt ' this size



*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_wformat_tag ) = 1;
*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_wformat_tag + 1 ) = 0;



*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_channels ) = channels;
*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_channels + 1 ) = 0;



*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_samp_per_sec ) = af.srate;



unsigned int avg_bytes_per_sec = af.srate * channels * 2;
*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_avg_bytes_per_sec ) = avg_bytes_per_sec;




*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_block_align ) = (unsigned char)( channels * 2 );		//1 channel = 2, 2 channel = 4
*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_block_align + 1 ) = 0;


*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_bits_per_samp ) = 16;
*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_bits_per_samp + 1 ) = 0;




strncpy( (char*)( buf + 36 + cn_riff_data_chunk_ckid ), "data", 4 );



*(unsigned int*)( buf + 36 + cn_riff_data_chunk_cksize ) = data_chunk_cksize;						//actual size of all sample data in bytes				




int wrote;
wrote = fwrite( buf, 1, 44, fp );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
if( wrote != 44 )
    {
	if( verb ) logpf( "audio_formats::save_wav() - failed to write 44 bytes of wav header for file: '%s'\n", fname1.c_str() );
    free( buf );
    fclose( fp);
	return 0;
    }


int iaud;

bool trk2 = 0;

unsigned int cnt = 0;
unsigned int j = 0;
unsigned int k = 0;

for( int i = 0; i < cnt0; i++ )
	{

	if( !trk2 ) dd = af.vch0[ k ];
	else  dd = af.vch1[ k ];

	iaud = nearbyint( dd * (double)peak_int_val );	//v1.04
    
    buf[ j ] = iaud & 0xff;                    	    //little endian
    j++;
    buf[ j ] = ( iaud >> 8  ) & 0xff;
    j++;

	if( j >= cn_audio_formats_buf_siz )
		{
		wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;

		if( wrote != j )
			{
			if( verb ) logpf( "audio_formats::save_wav() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
			fclose( fp );
			return 0;
			}
		cnt += wrote;
		j = 0;
		}

	if( channels == 2 )
		{
        if( trk2 ) k++;
        trk2 = !trk2;
		}
    else{
        k++;
        }
	}

if( j != 0 )
	{
	wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;
	if( wrote != j )
		{
		if( verb ) logpf( "audio_formats::save_wav() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
        free( buf );
		fclose( fp );
		return 0;
		}
	cnt += wrote;
	}


if( verb ) logpf( "audio_formats::save_wav() - wrote audio at sample rate of %u for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::save_wav() - wrote %u samples in ch1 for file: '%s'\n", k, fname1.c_str() );
if( verb ) if( channels == 2 ) logpf( "audio_formats::save_wav() - wrote %u samples in ch2 for file: '%s'\n", k, fname1.c_str() );

free( buf );
fclose( fp );

return 1;
}	















//save a wav audio file using malloc double pointers: pch0, pch1 (only PCM 16 bit, limited to 4GB)
//supports 16 bit pcm, 1 or 2 channels
//samples must be between +/- 1.0, saved samples are scaled up by peak_int_val
bool audio_formats::save_wav_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;
double dd;


string fname1 = path;
fname1 += fname;


int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::save_malloc_wav() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return 0;
    }




{
unsigned long long int cnt0 = sizech0;
unsigned long long int cnt1 = sizech1;


if( cnt0 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_malloc_wav() - ch1 is too big: %"LLU" for file filename: '%s'\n", cnt0, fname1.c_str() );
    return 0;
    }


if( cnt1 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_malloc_wav() - ch2 is too big: %"LLU" for file filename: '%s'\n", cnt1, fname1.c_str() );
    return 0;
    }
}








unsigned int cnt0 = sizech0;
unsigned int cnt1 = sizech1;

if( ( cnt0 == 0 ) && ( cnt1 == 0 ) )
    {
    if( verb ) logpf( "audio_formats::save_malloc_wav() - for filename: '%s', both channels are empty, no samples to write\n", fname1.c_str() );
    return 0;
    }


if( channels == 1 )                         //1 channel?
	{
    if( cnt0 == 0 )
        {
        if( verb ) logpf( "audio_formats::save_malloc_wav() - for filename: '%s', ch1 is empty, no samples to write\n", fname1.c_str() );
        return 0;
        }
    }

if( channels == 2 )
	{
    if( cnt0 < cnt1 )
        {
        if( verb ) logpf( "audio_formats::save_malloc_wav() - ch1 has less samples, will add zeroed samples to match ch2\n" );
        }

    if( cnt1 < cnt0 )
        {
        if( verb ) logpf( "audio_formats::save_malloc_wav() - ch2 has less samples, will add zeroed samples to match ch1\n" );
        }

    if( cnt0 < cnt1 )									//make both channels the same size, by increasing ch0
        {
        for( int i = cnt0; i < cnt1; i++ ) push_ch0( 0 );	
        }

    if( cnt1 < cnt0 )									//make both channels the same size, by increasing ch1
        {
        for( int i = cnt1; i < cnt0; i++ ) push_ch1( 0 );	
        }
    }


if( verb ) logpf( "audio_formats::save_malloc_wav() - denormalising using scale factor: %d\n", peak_int_val );


cnt0 = sizech0 * channels;


FILE *fp = m1.mbc_fopen( fname1, "wb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::save_malloc_wav() - failed to open file for writing: '%s'\n", fname1.c_str() );
	return 0;
	}



unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::save_malloc_wav() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return 0;
    }



//save wav header
strncpy( (char*)(buf + cn_riff_hdr_ckid), "RIFF", 4 );


unsigned int data_chunk_cksize = cnt0 * 2;						//size of all the audio sample data in bytes
unsigned int tot_chunk_size = data_chunk_cksize + 36;			//this number is 4 less than the total wav filesize


*(unsigned int*)( buf + cn_riff_hdr_cksize ) = tot_chunk_size;


strncpy( (char*)( buf + cn_riff_hdr_waveid ), "WAVE", 4 );




strncpy( (char*)( buf + 12 + cn_riff_fmt_chunk_ckid ), "fmt ", 4 );            //store 'fmt '


*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_cksize ) = 16;                 //always make the 'fmt ' this size



*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_wformat_tag ) = 1;
*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_wformat_tag + 1 ) = 0;



*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_channels ) = channels;
*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_channels + 1 ) = 0;



*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_samp_per_sec ) = af.srate;



unsigned int avg_bytes_per_sec = af.srate * channels * 2;
*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_avg_bytes_per_sec ) = avg_bytes_per_sec;




*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_block_align ) = (unsigned char)( channels * 2 );		//1 channel = 2, 2 channel = 4
*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_block_align + 1 ) = 0;


*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_bits_per_samp ) = 16;
*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_bits_per_samp + 1 ) = 0;




strncpy( (char*)( buf + 36 + cn_riff_data_chunk_ckid ), "data", 4 );



*(unsigned int*)( buf + 36 + cn_riff_data_chunk_cksize ) = data_chunk_cksize;						//actual size of all sample data in bytes				




int wrote;
wrote = fwrite( buf, 1, 44, fp );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
if( wrote != 44 )
    {
	if( verb ) logpf( "audio_formats::save_malloc_wav() - failed to write 44 bytes of wav header for file: '%s'\n", fname1.c_str() );
    free( buf );
    fclose( fp);
	return 0;
    }


int iaud;

bool trk2 = 0;

unsigned int cnt = 0;
unsigned int j = 0;
unsigned int k = 0;

for( int i = 0; i < cnt0; i++ )
	{

	if( !trk2 ) dd = pch0[ k ];
	else  dd = pch1[ k ];

	iaud = nearbyint( dd * (double)peak_int_val );	//v1.04
    
    buf[ j ] = iaud & 0xff;                    	    //little endian
    j++;
    buf[ j ] = ( iaud >> 8  ) & 0xff;
    j++;

	if( j >= cn_audio_formats_buf_siz )
		{
		wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;

		if( wrote != j )
			{
			if( verb ) logpf( "audio_formats::save_malloc_wav() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
			fclose( fp );
			return 0;
			}
		cnt += wrote;
		j = 0;
		}

	if( channels == 2 )
		{
        if( trk2 ) k++;
        trk2 = !trk2;
		}
    else{
        k++;
        }
	}

if( j != 0 )
	{
	wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;
	if( wrote != j )
		{
		if( verb ) logpf( "audio_formats::save_malloc_wav() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
        free( buf );
		fclose( fp );
		return 0;
		}
	cnt += wrote;
	}


if( verb ) logpf( "audio_formats::save_malloc_wav() - wrote audio at sample rate of %u for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::save_malloc_wav() - wrote %u samples in ch1 for file: '%s'\n", k, fname1.c_str() );
if( verb ) if( channels == 2 ) logpf( "audio_formats::save_malloc_wav() - wrote %u samples in ch2 for file: '%s'\n", k, fname1.c_str() );

free( buf );
fclose( fp );

return 1;
}	













//save audio waveform in Sun's .au file format (limited to 4GB)
//supports encoding 1:  u-law, 8 bit, 1 or 2 channel
//supports encoding 3:  pcm, 16 bit, little endian or big endian, 1 or 2 channel
//samples must be between +/- 1.0, saved samples are scaled up by peak_int_val

//note Sox warns if offset is 24, so have used min offset of 28 which gives an 'annotation field' of 4 bytes
bool audio_formats::save_sun( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;
double dd;


string fname1 = path;
fname1 += fname;

bool supported = 0;


if( af.encoding == 1 ) supported = 1;
if( af.encoding == 3 ) supported = 1;

if( !supported )
    {
    if( verb ) logpf( "audio_formats::save_sun() - unsupported encoding: %u\n", af.encoding );
    return 0;
    }

int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::save_sun() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return 0;
    }


//the offset space is not actually used in this code, just set it to a 4 bytes 'annotaion field' if it is supplied with dubious value (possibly because it was not set by user)
if( af.offset < 28 )
    {
    if( verb ) logpf( "audio_formats::save_sun() - for filename: '%s', data offset specified is too small for header: %u, using offs of: 28\n", fname1.c_str(), af.offset );
    af.offset = 28;
    }


if( af.offset > ( 1024 * 16 ) )
    {
    if( verb ) logpf( "audio_formats::save_sun() - for filename: '%s', data offset specified is too big for header: %u, using offs of: 28\n", fname1.c_str(), af.offset );
    af.offset = 28;
    }



{
unsigned long long int cnt0 = af.vch0.size();
unsigned long long int cnt1 = af.vch1.size();


if( cnt0 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_sun() - ch1 is too big: %"LLU" for file filename: '%s'\n", cnt0, fname1.c_str() );
    return 0;
    }


if( cnt1 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_sun() - ch2 is too big: %"LLU" for file filename: '%s'\n", cnt1, fname1.c_str() );
    return 0;
    }
}





unsigned int cnt0 = af.vch0.size();
unsigned int cnt1 = af.vch1.size();

if( ( cnt0 == 0 ) && ( cnt1 == 0 ) )
	{
	if( verb ) logpf( "audio_formats::save_sun() - for filename: '%s', both channels are empty, no samples to write\n", fname1.c_str() );
	return 0;
	}


if( channels == 1 )                         //1 channel?
	{
	if( cnt0 == 0 )
		{
		if( verb ) logpf( "audio_formats::save_sun() - for filename: '%s', ch1 is empty, no samples to write\n", fname1.c_str() );
		return 0;
		}
	}

if( channels == 2 )
	{
	if( cnt0 < cnt1 )
		{
		if( verb ) logpf( "audio_formats::save_sun() - ch1 has less samples, will add zeroed samples to match ch2\n" );
		}

	if( cnt1 < cnt0 )
		{
		if( verb ) logpf( "audio_formats::save_sun() - ch2 has less samples, will add zeroed samples to match ch1\n" );
		}

	if( cnt0 < cnt1 )									//make both channels the same size, by increasing ch0
		{
		for( int i = cnt0; i < cnt1; i++ ) af.vch0.push_back( 0 );	
		}

	if( cnt1 < cnt0 )									//make both channels the same size, by increasing ch1
		{
		for( int i = cnt1; i < cnt0; i++ ) af.vch1.push_back( 0 );	
		}
	}




if( verb ) logpf( "audio_formats::save_sun() - denormalising using scale factor: %d\n", peak_int_val );


cnt0 = af.vch0.size() * channels;




//if ( vsamp.size() == 0 ) return 0;

FILE *fp = m1.mbc_fopen( fname1, "wb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::save_sun() - failed to open file for writing: '%s'\n", fname1.c_str() );
	return 0;
	}

unsigned int wrote;

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::save_sun() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return 0;
    }


//unsigned char buf[ 4 * 7 ];                   //7 x 32bit big enough to hold sun's au audio file header

buf[ 0 ] = 0x2e;							    //sun's au file format - magic num
buf[ 1 ] = 0x73;
buf[ 2 ] = 0x6e;
buf[ 3 ] = 0x64;

buf[ 4 ] = ( af.offset >> 24 ) & 0xff;
buf[ 5 ] = ( af.offset >> 16 ) & 0xff;
buf[ 6 ] = ( af.offset >> 8 ) & 0xff;
buf[ 7 ] = ( af.offset ) & 0xff;



unsigned int samples = af.vch0.size() * channels;
unsigned int data_size = samples;

if( af.encoding == 3 ) data_size *= 2;

buf[ 8 ] = ( data_size >> 24 ) & 0xff;
buf[ 9 ] = ( data_size >> 16 ) & 0xff;
buf[ 10 ] = ( data_size >> 8 ) & 0xff;
buf[ 11 ] = ( data_size ) & 0xff;


buf[ 12 ] = ( af.encoding >> 24 ) & 0xff;
buf[ 13 ] = ( af.encoding >> 16 ) & 0xff;
buf[ 14 ] = ( af.encoding >> 8 ) & 0xff;
buf[ 15 ] = ( af.encoding ) & 0xff;

buf[ 16 ] = ( af.srate >> 24 ) & 0xff;
buf[ 17 ] = ( af.srate >> 16 ) & 0xff;
buf[ 18 ] = ( af.srate >> 8 ) & 0xff;
buf[ 19 ] = ( af.srate ) & 0xff;

buf[ 20 ] = ( channels >> 24 ) & 0xff;
buf[ 21 ] = ( channels >> 16 ) & 0xff;
buf[ 22 ] = ( channels >> 8 ) & 0xff;
buf[ 23 ] = ( channels ) & 0xff;

wrote = fwrite( buf, 1, 7 * 4, fp );                //write 7 x 32bit, sun's au audio file header, bytes 24-28 are the 'annotaion field', which is not set to anything here

if( wrote != ( 7 * 4 ) )
    {
    if( verb ) logpf( "audio_formats::save_sun() - failed to write header: '%s', total bytes written: %u\n", fname1.c_str(), wrote );
    free( buf );
    fclose( fp );
    return 0;
    }


int skip = af.offset - 7 * 4;
if ( skip < 0 ) skip = 0;
//unsigned char bf_null[ 1 ];

buf[ 0 ] = 0;
for( unsigned int i = 0; i < skip; i++ )						//step to first sample byte, if offset was > 28 write zeros till offset is written
    {
    wrote = fwrite( buf, 1, 1, fp );
    }








int iaud;

bool trk2 = 0;

unsigned int cnt = 0;
unsigned int j = 0;
unsigned int k = 0;

for( unsigned int i = 0; i < cnt0; i++ )
	{

	if( !trk2 ) dd = af.vch0[ k ];
	else  dd = af.vch1[ k ];


    if( af.encoding == 1 )                              //u-law?
        {
        unsigned char cc = linear2ulaw( nearbyint( dd * (double)peak_int_val ) );		//v1.04
        buf[ j ] =  cc;
        j++;
        }

    if( af.encoding == 3 )                              //16 bit pcm?
        {

        iaud = nearbyint( dd * (double)peak_int_val );	//v1.04
        buf[ j ] = ( iaud >> 8  ) & 0xff;               //big endian pcm?
        j++;
        buf[ j ] =  iaud & 0xff;
        j++;
        }


    if( j >= cn_audio_formats_buf_siz )
        {
        wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;

        if( wrote != j )
            {
            if( verb ) logpf( "audio_formats::save_sun() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
            fclose( fp );
            return 0;
            }
        cnt += wrote;
        j = 0;
        }

    if( channels == 2 )
        {
        if( trk2 ) k++;
        trk2 = !trk2;
        }
    else{
        k++;
        }
    //if(!(cnt%1000))printf("audio_formats::save_sun() - limit: %u, cnt: %u\n", cnt0, cnt );
    }

    if( j != 0 )
        {
        wrote = fwrite( buf, 1, j, fp );          //write block of audio data
    //wrote = j;
        if( wrote != j )
            {
            if( verb ) logpf( "audio_formats::save_sun() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
            fclose( fp );
            return 0;
            }
        cnt += wrote;
        }


if( verb ) if( af.encoding == 1 ) logpf( "audio_formats::save_sun() - encoding is 8 bit u-law\n" );
if( verb ) if( af.encoding == 3 ) logpf( "audio_formats::save_sun() - encoding is 16 bit pcm, big endian\n" );

if( verb ) logpf( "audio_formats::save_sun() - wrote audio at sample rate of %u for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::save_sun() - wrote %u samples in ch1 for file: '%s'\n", k, fname1.c_str() );
if( verb ) if( channels == 2 ) logpf( "audio_formats::save_sun() - wrote %u samples in ch2 for file: '%s'\n", k, fname1.c_str() );

free( buf );
fclose( fp );

return 1;

}




















//save audio waveform in Sun's .au file format using malloc double pointers: pch0, pch1 (limited to 4GB)
//supports encoding 1:  u-law, 8 bit, 1 or 2 channel
//supports encoding 3:  pcm, 16 bit, little endian or big endian, 1 or 2 channel
//samples must be between +/- 1.0, saved samples are scaled up by peak_int_val

//note Sox warns if offset is 24, so have used min offset of 28 which gives an 'annotation field' of 4 bytes
bool audio_formats::save_sun_malloc( string path, string fname, int peak_int_val, st_audio_formats_tag &af )
{
string s1;
mystr m1;
double dd;


string fname1 = path;
fname1 += fname;

bool supported = 0;


if( af.encoding == 1 ) supported = 1;
if( af.encoding == 3 ) supported = 1;

if( !supported )
    {
    if( verb ) logpf( "audio_formats::save_sun_malloc() - unsupported encoding: %u\n", af.encoding );
    return 0;
    }

int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::save_sun_malloc() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
    return 0;
    }


//the offset space is not actually used in this code, just set it to a 4 bytes 'annotaion field' if it is supplied with dubious value (possibly because it was not set by user)
if( af.offset < 28 )
    {
    if( verb ) logpf( "audio_formats::save_sun_malloc() - for filename: '%s', data offset specified is too small for header: %u, using offs of: 28\n", fname1.c_str(), af.offset );
    af.offset = 28;
    }


if( af.offset > ( 1024 * 16 ) )
    {
    if( verb ) logpf( "audio_formats::save_sun_malloc() - for filename: '%s', data offset specified is too big for header: %u, using offs of: 28\n", fname1.c_str(), af.offset );
    af.offset = 28;
    }



{
unsigned long long int cnt0 = sizech0;
unsigned long long int cnt1 = sizech1;


if( cnt0 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_sun_malloc() - ch1 is too big: %"LLU" for file filename: '%s'\n", cnt0, fname1.c_str() );
    return 0;
    }


if( cnt1 > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::save_sun_malloc() - ch2 is too big: %"LLU" for file filename: '%s'\n", cnt1, fname1.c_str() );
    return 0;
    }
}





unsigned int cnt0 = sizech0;
unsigned int cnt1 = sizech1;

if( ( cnt0 == 0 ) && ( cnt1 == 0 ) )
	{
	if( verb ) logpf( "audio_formats::save_sun_malloc() - for filename: '%s', both channels are empty, no samples to write\n", fname1.c_str() );
	return 0;
	}


if( channels == 1 )                         //1 channel?
	{
	if( cnt0 == 0 )
		{
		if( verb ) logpf( "audio_formats::save_sun_malloc() - for filename: '%s', ch1 is empty, no samples to write\n", fname1.c_str() );
		return 0;
		}
	}

if( channels == 2 )
	{
	if( cnt0 < cnt1 )
		{
		if( verb ) logpf( "audio_formats::save_sun_malloc() - ch1 has less samples, will add zeroed samples to match ch2\n" );
		}

	if( cnt1 < cnt0 )
		{
		if( verb ) logpf( "audio_formats::save_sun_malloc() - ch2 has less samples, will add zeroed samples to match ch1\n" );
		}

	if( cnt0 < cnt1 )									//make both channels the same size, by increasing ch0
		{
		for( int i = cnt0; i < cnt1; i++ ) push_ch0( 0 );	
		}

	if( cnt1 < cnt0 )									//make both channels the same size, by increasing ch1
		{
		for( int i = cnt1; i < cnt0; i++ ) push_ch1( 0 );	
		}
	}




if( verb ) logpf( "audio_formats::save_sun_malloc() - denormalising using scale factor: %d\n", peak_int_val );


cnt0 = sizech0 * channels;



FILE *fp = m1.mbc_fopen( fname1, "wb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::save_sun_malloc() - failed to open file for writing: '%s'\n", fname1.c_str() );
	return 0;
	}

unsigned int wrote;

unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::save_sun_malloc() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
    fclose( fp );
    return 0;
    }


//unsigned char buf[ 4 * 7 ];                   //7 x 32bit big enough to hold sun's au audio file header

buf[ 0 ] = 0x2e;							    //sun's au file format - magic num
buf[ 1 ] = 0x73;
buf[ 2 ] = 0x6e;
buf[ 3 ] = 0x64;

buf[ 4 ] = ( af.offset >> 24 ) & 0xff;
buf[ 5 ] = ( af.offset >> 16 ) & 0xff;
buf[ 6 ] = ( af.offset >> 8 ) & 0xff;
buf[ 7 ] = ( af.offset ) & 0xff;



unsigned int samples = sizech0 * channels;
unsigned int data_size = samples;

if( af.encoding == 3 ) data_size *= 2;

buf[ 8 ] = ( data_size >> 24 ) & 0xff;
buf[ 9 ] = ( data_size >> 16 ) & 0xff;
buf[ 10 ] = ( data_size >> 8 ) & 0xff;
buf[ 11 ] = ( data_size ) & 0xff;


buf[ 12 ] = ( af.encoding >> 24 ) & 0xff;
buf[ 13 ] = ( af.encoding >> 16 ) & 0xff;
buf[ 14 ] = ( af.encoding >> 8 ) & 0xff;
buf[ 15 ] = ( af.encoding ) & 0xff;

buf[ 16 ] = ( af.srate >> 24 ) & 0xff;
buf[ 17 ] = ( af.srate >> 16 ) & 0xff;
buf[ 18 ] = ( af.srate >> 8 ) & 0xff;
buf[ 19 ] = ( af.srate ) & 0xff;

buf[ 20 ] = ( channels >> 24 ) & 0xff;
buf[ 21 ] = ( channels >> 16 ) & 0xff;
buf[ 22 ] = ( channels >> 8 ) & 0xff;
buf[ 23 ] = ( channels ) & 0xff;

wrote = fwrite( buf, 1, 7 * 4, fp );                //write 7 x 32bit, sun's au audio file header, bytes 24-28 are the 'annotaion field', which is not set to anything here

if( wrote != ( 7 * 4 ) )
    {
    if( verb ) logpf( "audio_formats::save_sun_malloc() - failed to write header: '%s', total bytes written: %u\n", fname1.c_str(), wrote );
    free( buf );
    fclose( fp );
    return 0;
    }


int skip = af.offset - 7 * 4;
if ( skip < 0 ) skip = 0;
//unsigned char bf_null[ 1 ];

buf[ 0 ] = 0;
for( unsigned int i = 0; i < skip; i++ )						//step to first sample byte, if offset was > 28 write zeros till offset is written
    {
    wrote = fwrite( buf, 1, 1, fp );
    }








int iaud;

bool trk2 = 0;

unsigned int cnt = 0;
unsigned int j = 0;
unsigned int k = 0;

for( unsigned int i = 0; i < cnt0; i++ )
	{

	if( !trk2 ) dd = pch0[ k ];
	else  dd = pch1[ k ];


    if( af.encoding == 1 )                              //u-law?
        {
        unsigned char cc = linear2ulaw( nearbyint( dd * (double)peak_int_val ) );		//v1.04
        buf[ j ] =  cc;
        j++;
        }

    if( af.encoding == 3 )                              //16 bit pcm?
        {

        iaud = nearbyint( dd * (double)peak_int_val );	//v1.04
        buf[ j ] = ( iaud >> 8  ) & 0xff;               //big endian pcm?
        j++;
        buf[ j ] =  iaud & 0xff;
        j++;
        }


    if( j >= cn_audio_formats_buf_siz )
        {
        wrote = fwrite( buf, 1, j, fp );          //write block of audio data
//wrote = j;

        if( wrote != j )
            {
            if( verb ) logpf( "audio_formats::save_sun_malloc() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
            fclose( fp );
            return 0;
            }
        cnt += wrote;
        j = 0;
        }

    if( channels == 2 )
        {
        if( trk2 ) k++;
        trk2 = !trk2;
        }
    else{
        k++;
        }
    //if(!(cnt%1000))printf("audio_formats::save_sun() - limit: %u, cnt: %u\n", cnt0, cnt );
    }

    if( j != 0 )
        {
        wrote = fwrite( buf, 1, j, fp );          //write block of audio data
    //wrote = j;
        if( wrote != j )
            {
            if( verb ) logpf( "audio_formats::save_sun_malloc() - failed to complete a write to file: '%s', total bytes written: %u\n", fname1.c_str(), cnt );
            free( buf );
            fclose( fp );
            return 0;
            }
        cnt += wrote;
        }


if( verb ) if( af.encoding == 1 ) logpf( "audio_formats::save_sun_malloc() - encoding is 8 bit u-law\n" );
if( verb ) if( af.encoding == 3 ) logpf( "audio_formats::save_sun_malloc() - encoding is 16 bit pcm, big endian\n" );

if( verb ) logpf( "audio_formats::save_sun_malloc() - wrote audio at sample rate of %u for file: '%s'\n", af.srate, fname1.c_str() );
if( verb ) logpf( "audio_formats::save_sun_malloc() - wrote %u samples in ch1 for file: '%s'\n", k, fname1.c_str() );
if( verb ) if( channels == 2 ) logpf( "audio_formats::save_sun_malloc() - wrote %u samples in ch2 for file: '%s'\n", k, fname1.c_str() );

free( buf );
fclose( fp );

return 1;

}



















//scale all audio channel so max/min levels don't exceed  '+/- max_val'
//the relative level difference between channels is maintained
bool audio_formats::normalise( double max_val, st_audio_formats_tag &af )
{
double min = 0;
double max = 0;


for( unsigned int i = 0; i < af.vch0.size(); i++ )
    {
    double dd = af.vch0[ i ];
    
    if( dd < min ) min = dd;
    if( dd > max ) max = dd;
    }

for( unsigned int i = 0; i < af.vch1.size(); i++ )
    {
    double dd = af.vch1[ i ];
    
    if( dd < min ) min = dd;
    if( dd > max ) max = dd;
    }


double scale = 1.0 / fabs( max );                           //work out what scale value will keep peak excursion to +/- 1.0
if( fabs( min ) > fabs( max ) )
    {
    scale = 1.0 / fabs( min );
    }


for( unsigned int i = 0; i < af.vch0.size(); i++ )
    {
    af.vch0[ i ] = af.vch0[ i ] * scale * max_val;          //scale to requested max value
    }

for( unsigned int i = 0; i < af.vch1.size(); i++ )
    {
    af.vch1[ i ] = af.vch1[ i ] * scale * max_val;
    }

return 1;
}










//make tone(s)
bool audio_formats::make_tone( double amp0, double freq0, double phase0, double amp1, double freq1, double phase1, double dur, st_audio_formats_tag &af )
{

double srate;
srate = af.srate;

double timepersample = 1.0 / srate;


double two_pi = 2.0 * M_PI;
double f1_theta0 = 0;
double f1_freq0 = freq0;
double f1_inc0 = f1_freq0 * two_pi * timepersample;

double f1_phase_shift0 = phase0;


double f1_theta1 = 0;
double f1_freq1 = freq1;
double f1_inc1 = f1_freq1 * two_pi * timepersample;

double f1_phase_shift1 = phase1;


if ( ( srate < 1000.0 ) || ( srate > 384000.0 ) )
    {
	if( verb ) logpf( "audio_formats::make_tone() - srate out of range: '%f'\n", srate );
    return 0;
    }



if ( dur < 0.0 )
    {
	if( verb ) logpf( "audio_formats::make_tone() - did nothing, duration was secs: '%f'\n", dur );
    return 0;
    }



if ( dur > 600.0 )
    {
	if( verb ) logpf( "audio_formats::make_tone() - limiting to 600 secs, was: '%f'\n", dur );
    dur = 600.0;
    }


int channels = af.channels;

if( ( channels <= 0 ) || ( channels > 2 ) )
    {
    if( verb ) logpf( "audio_formats::make_tone() - channel count specified is not supported: %u\n", channels );
    return 0;
    }


af.vch0.clear();
af.vch1.clear();


unsigned int write_cnt = srate * dur;		                        //calc number of samples


unsigned int wrcnt = 0;


while( 1 )
	{

    double v0, v1;

    v0 = amp0 * sin( f1_theta0 + f1_phase_shift0 );		        //tone1
    af.vch0.push_back( v0 );

    f1_theta0 += f1_inc0;
    if( f1_theta0 >= two_pi ) f1_theta0 -= two_pi;
    
    if( channels == 2 )
        {
        v1 = amp1 * sin( f1_theta1 + f1_phase_shift1 );		    //tone2
        af.vch1.push_back( v1 );

        f1_theta1 += f1_inc1;
        if( f1_theta1 >= two_pi ) f1_theta1 -= two_pi;
        }
        

	wrcnt++;
	if( wrcnt >=  write_cnt ) break;
	}

return 1;
}







//try to work out what audio file format this spec fname has
//returns 1 if it think it knows, else 0
bool audio_formats::find_file_format( string path, string fname, en_audio_formats &fmt )
{

string fname1 = path;
fname1 += fname;

if( verb ) logpf( "audio_formats::find_file_format() - '%s'\n", fname1.c_str() );

fmt = en_af_unknown;

unsigned long long int fsz;
mystr m1;
if( !m1.filesize( fname1, fsz ) )			//get file size
    {
    if( verb ) logpf( "audio_formats::find_file_format() - failed to open filename: '%s'\n", fname1.c_str() );
    return 0;
    }


if( fsz > 0xffffffff )
    {
    if( verb ) logpf( "audio_formats::find_file_format() - file size is too big: %"LLU" for file filename: '%s'\n", fsz, fname1.c_str() );
    return 0;
    }


if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::find_file_format() - filename: '%s' is empty\n", fname1.c_str() );
    return 0;
    }

FILE *fp = m1.mbc_fopen( fname1, "rb" );

if( fp == 0 )
	{
	if( verb ) logpf( "audio_formats::find_file_format() - failed to open file for reading: '%s'\n", fname1.c_str() );
	return 0;
	}


unsigned char *buf = (unsigned char*) malloc( cn_audio_formats_buf_siz ); //   [ cn_audio_formats_buf_siz ];

if( buf == 0 )
    {
    if( verb ) logpf( "audio_formats::find_file_format() - unable to alloc memory, was requesting %d bytes\n", cn_audio_formats_buf_siz );
	fclose( fp );
    return 0;
    }


unsigned int need = 4;							//
												//actual size of the sample data bytes
int read = fread( buf, 1, need, fp );          	//read block of audio data

if( read != need )
	{
	if( verb ) logpf( "audio_formats::load_wav() - failed to read header bytes: '%s'\n", fname1.c_str() );
    free ( buf );
	fclose( fp );
	return 0;
	}


if( strncmp( (char*)( buf + 0 ), ".snd", 4 ) == 0 )						//sun's .au format?
	{
	fmt = en_af_sun;
	goto done;
	}


if( strncmp( (char*)( buf + cn_riff_hdr_ckid ), "FORM", 4 ) == 0 )		//apple's aiff format?
	{
	fmt = en_af_aiff;
	goto done;
	}


if( strncmp( (char*)( buf + cn_riff_hdr_ckid ), "RIFF", 4 ) == 0 )		//msoftd's wav format?
	{
	fmt = en_af_wav_pcm;
	goto done;
	}


done:

free ( buf );
fclose( fp );
if( fmt == en_af_unknown ) return 0;
return 1;
}








//clear and push copy of channel
//pch is pointer to src audio
//count it number of samples to copy
bool audio_formats::copypush_ch0( double *pch, unsigned int count )
{
if( verb ) logpf( "audio_formats::copypush_ch0(): count: %u\n", count );

clear_ch0();

if( count == 0 ) return 1;

for( unsigned int i = 0; i < count; i++ )
	{
	push_ch0( pch[ i ] );
	}

return 1;
}








//clear and push copy of channel
//pch is pointer to src audio
//count it number of samples to copy
bool audio_formats::copypush_ch1( double *pch, unsigned int count )
{
if( verb ) logpf( "audio_formats::copypush_ch1(): count: %u\n", count );

clear_ch1();

if( count == 0 ) return 1;

for( unsigned int i = 0; i < count; i++ )
	{
	push_ch1( pch[ i ] );
	}

return 1;
}







//sample rate converter, not yet completed
bool audio_formats::srate_change( st_audio_formats_tag &afout )
{
}
























//fir convolution using supplied filter coeff vector  (only for malloc storage methods)
//only operates on: 'malloc storage methods'
//set 'which_chan'to:  0x1: ch0,    0x2: ch1,   0x3: ch0 and ch1
//returns 1 on success, else 0

//this code effectively works on the samples in-place, it does this by firsty delaying samples via buffers,
//therefore: when writing a completed calc'd sample, that sample's location is no longer within
//the convolution window (which the same size as the coeff count)
bool audio_formats::fir_filter( int which_chan, vector<double> &vcoeff )
{
if( verb ) logpf( "audio_formats::fir_filter() - which_chan: %d, vcoeff.size(): %u\n", which_chan, vcoeff.size() );


int fircnt = vcoeff.size();

//put some limit on fir coeffs
if( fircnt > ( 2 * 1024 * 1024 ) )
	{
	if( verb ) logpf( "audio_formats::fir_filter() - too many coeffs in vcoeff: %u, limit is: %u\n", (int)fircnt, vcoeff.size() );
	printf( "audio_formats::fir_filter() - too many coeffs in vcoeff: %u, limit is: %u\n", fircnt, vcoeff.size() );
	return 0;
	}


if( fircnt == 0 ) return 0;
if( which_chan == 0 ) return 0;

which_chan &= 0x3;

if( which_chan == 1 ) if( sizech0 == 0 ) return 1;				//empty?
if( which_chan == 2 ) if( sizech1 == 0 ) return 1;

if( which_chan == 3 ) if( !(sizech0 | sizech1 ) ) return 1;		//empty?


long long int  cnt = 0;

if( which_chan == 1 ) cnt = sizech0;
if( which_chan == 2 ) cnt = sizech1;
if( which_chan == 3 )
	{
	if( sizech0 >= sizech1 ) cnt = sizech0;
	if( sizech1 > sizech0 ) cnt = sizech1;
	}


if( fircnt > cnt )
	{
	if( verb ) logpf( "audio_formats::fir_filter() - not enough samples vs coeffs, samples: %"LLU", vcoeffs: %u\n", cnt, vcoeff.size() );
	printf( "audio_formats::fir_filter() - not enough samples vs coeffs, samples: %"LLU", vcoeffs: %u\n", cnt, vcoeff.size() );
	return 0;
	}

int mid = fircnt / 2;

double *inp0 = new double[ fircnt ];		//make delayed buffers that will be filled with as many i/p samples as there are coeffs
double *inp1 = new double[ fircnt ];

int wr_idx;
int rd_idx;

//load input buffers
for( wr_idx = 0; wr_idx < fircnt; wr_idx++ )
	{
	if( which_chan & 1 )
		{
		if( wr_idx < sizech0 )
			{
			inp0[ wr_idx ] = pch0[ wr_idx ];
			}
		else{
			inp0[ wr_idx ] = 0;
			}
		}

	if( which_chan & 2 )
		{
		if( wr_idx < sizech1 )
			{
			inp1[ wr_idx ] = pch1[ wr_idx ];
			}
		else{
			inp1[ wr_idx ] = 0;
			}
		}
	}

wr_idx = 0;

long long int k = fircnt;

//convolve
for( long long int i = 0; i < cnt; i++ )
	{
	double sum0, sum1;
	
//	bool store0;
//	bool store1;
	sum0 = 0;
	sum1 = 0;
//	store0 = 0;
//	store1 = 0;
	rd_idx = wr_idx - 1;										//keep rd ptr one behind wr ptr
	for( long long int j = 0; j < fircnt; j++ )
		{

		if( rd_idx < 0 ) rd_idx += fircnt;						//wrap rd ptr
		if( rd_idx >= fircnt ) rd_idx -= fircnt;


		if( which_chan & 1 )
			{
			sum0 += inp0[ rd_idx ] * vcoeff[ j ];             	//convolve delayed input samples with filter impulse resp 
			}

		if( which_chan & 2 )
			{
			sum1 += inp1[ rd_idx ] * vcoeff[ j ];
			}

		rd_idx--;
		}
	
	if( which_chan & 1 )
		{
		if( i < sizech0 )
			{
			pch0[ i ] = sum0;								//store convoled sample
			}

		if( k < sizech0 )
			{
			inp0[ wr_idx ] = pch0[ k ];						//keep loading i/p delay buffer
			}
		else{
			inp0[ wr_idx ] = 0;
			}
		}


	if( which_chan & 2 )
		{
		if( i < sizech1 )
			{
			pch1[ i ] = sum1;								//store convoled sample
			}

		if( k < sizech1 )
			{
			inp1[ wr_idx ] = pch1[ k ];						//keep loading i/p delay buffer
			}
		else{
			inp1[ wr_idx ] = 0;
			}
		}

	wr_idx++;
	if( wr_idx >= fircnt ) wr_idx = 0;

	k++;
	}

delete inp0;
delete inp1;

return 1;
}















//called internally at obj destruction, fills in header, completes any file writes by closing file, then frees mem
void audio_formats::fp_write_close_free_mem()
{
unsigned char buf[256];

//save sun header
if( ( fp_audio_format == en_af_sun ) & (fileptr != 0 ) )
	{
	fseek( fileptr, 0, SEEK_SET );                					//move to head of file
	
	buf[ 0 ] = 0x2e;							    				//sun's au file format - magic num
	buf[ 1 ] = 0x73;
	buf[ 2 ] = 0x6e;
	buf[ 3 ] = 0x64;

	buf[ 4 ] = ( fp_offset >> 24 ) & 0xff;
	buf[ 5 ] = ( fp_offset >> 16 ) & 0xff;
	buf[ 6 ] = ( fp_offset >> 8 ) & 0xff;
	buf[ 7 ] = ( fp_offset ) & 0xff;

//	int bytes_per_sample = 2;
//	if( fp_encoding == 1 ) bytes_per_sample = 1;
	
//	unsigned int samples = fp_bytes_written / bytes_per_sample;
	unsigned int data_size = fp_bytes_written;

	buf[ 8 ] = ( data_size >> 24 ) & 0xff;
	buf[ 9 ] = ( data_size >> 16 ) & 0xff;
	buf[ 10 ] = ( data_size >> 8 ) & 0xff;
	buf[ 11 ] = ( data_size ) & 0xff;


	buf[ 12 ] = ( fp_encoding >> 24 ) & 0xff;
	buf[ 13 ] = ( fp_encoding >> 16 ) & 0xff;
	buf[ 14 ] = ( fp_encoding >> 8 ) & 0xff;
	buf[ 15 ] = ( fp_encoding ) & 0xff;

	buf[ 16 ] = ( fp_srate >> 24 ) & 0xff;
	buf[ 17 ] = ( fp_srate >> 16 ) & 0xff;
	buf[ 18 ] = ( fp_srate >> 8 ) & 0xff;
	buf[ 19 ] = ( fp_srate ) & 0xff;

	buf[ 20 ] = ( fp_channels >> 24 ) & 0xff;
	buf[ 21 ] = ( fp_channels >> 16 ) & 0xff;
	buf[ 22 ] = ( fp_channels >> 8 ) & 0xff;
	buf[ 23 ] = ( fp_channels ) & 0xff;

	int wrote = fwrite( buf, 1, 7 * 4, fileptr );                //write 7 x 32bit, sun's au audio file header, bytes 24-28 are the 'annotaion field', which is not set to anything here

	if( wrote != ( 7 * 4 ) )
		{
		if( verb ) logpf( "audio_formats::fp_write_close_free_mem() - failed to write sun header: '%s', total bytes written: %u\n", pathname.c_str(), wrote );
		}
	}









//save aiff header
if( ( fp_audio_format == en_af_aiff ) & (fileptr != 0 ) )
	{
	fseek( fileptr, 0, SEEK_SET );                					//move to head of file
	
	unsigned int cnt0 = fp_bytes_written / 2;						//num of samples for both channels (bytes to 16 bit samples)

	unsigned int bytes_per_sample = 2;

	unsigned int ui = cnt0 * bytes_per_sample + 46;						//samples * bytes_per_sample + hdr

	strncpy( (char*)(buf + cn_aiff_form_ckid), "FORM", 4 );


	buf[ cn_aiff_form_cksize ] = ( ui >> 24 ) & 0xff;
	buf[ cn_aiff_form_cksize + 1 ] = ( ui >> 16 ) & 0xff;
	buf[ cn_aiff_form_cksize + 2 ] = ( ui >> 8 ) & 0xff;
	buf[ cn_aiff_form_cksize + 3 ] = ui & 0xff;

	strncpy( (char*)( buf + cn_aiff_aiff_ckid ), "AIFF", 4 );

	strncpy( (char*)( buf + cn_aiff_comm_ckid ), "COMM", 4 );

	ui = 18;
	buf[ cn_aiff_comm_cksize ] = ( ui >> 24 ) & 0xff;
	buf[ cn_aiff_comm_cksize + 1 ] = ( ui >> 16 ) & 0xff;
	buf[ cn_aiff_comm_cksize + 2 ] = ( ui >> 8 ) & 0xff;
	buf[ cn_aiff_comm_cksize + 3 ] = ui & 0xff;

	buf[ cn_aiff_comm_channels ] = ( fp_channels >> 8 ) & 0xff;
	buf[ cn_aiff_comm_channels + 1 ] = fp_channels & 0xff;


	ui = cnt0 / fp_channels;										//number of sample frames
	buf[ cn_aiff_comm_sample_frames ] = ( ui >> 24 ) & 0xff;
	buf[ cn_aiff_comm_sample_frames + 1 ] = ( ui >> 16 ) & 0xff;
	buf[ cn_aiff_comm_sample_frames + 2 ] = ( ui >> 8 ) & 0xff;
	buf[ cn_aiff_comm_sample_frames + 3 ] = ui & 0xff;


	ui = 16;
	buf[ cn_aiff_comm_sample_size ] = ( ui >> 8 ) & 0xff;
	buf[ cn_aiff_comm_sample_size + 1 ] = ui & 0xff;



	long double ld_srate = fp_srate;				//sample rate is stored as a big edndian IEEE 754 extended precision floating point number (long double)
	unsigned char *ld_ptr = (unsigned char *) &ld_srate;

	buf[ cn_aiff_comm_sample_srate ] = ld_ptr[ 9 ];
	buf[ cn_aiff_comm_sample_srate + 1 ] = ld_ptr[ 8 ];
	buf[ cn_aiff_comm_sample_srate + 2 ] = ld_ptr[ 7 ];
	buf[ cn_aiff_comm_sample_srate + 3 ] = ld_ptr[ 6 ];
	buf[ cn_aiff_comm_sample_srate + 4 ] = ld_ptr[ 5 ];
	buf[ cn_aiff_comm_sample_srate + 5 ] = ld_ptr[ 4 ];
	buf[ cn_aiff_comm_sample_srate + 6 ] = ld_ptr[ 3 ];
	buf[ cn_aiff_comm_sample_srate + 7 ] = ld_ptr[ 2 ];
	buf[ cn_aiff_comm_sample_srate + 8 ] = ld_ptr[ 1 ];
	buf[ cn_aiff_comm_sample_srate + 9 ] = ld_ptr[ 0 ];


	strncpy( (char*)( buf + cn_aiff_comm_ssnd_ckid ), "SSND", 4 );



	ui = cnt0 * bytes_per_sample + 8;				//samples * bytes_per_sample + some hdr
	buf[ cn_aiff_comm_ssnd_cksize ] = ( ui >> 24 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_cksize + 1 ] = ( ui >> 16 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_cksize + 2 ] = ( ui >> 8 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_cksize + 3 ] = ui & 0xff;

	ui = 0;
	buf[ cn_aiff_comm_ssnd_offset ] = ( ui >> 24 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_offset + 1 ] = ( ui >> 16 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_offset + 2 ] = ( ui >> 8 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_offset + 3 ] = ui & 0xff;


	ui = 0;
	buf[ cn_aiff_comm_ssnd_block_size ] = ( ui >> 24 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_block_size + 1 ] = ( ui >> 16 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_block_size + 2 ] = ( ui >> 8 ) & 0xff;
	buf[ cn_aiff_comm_ssnd_block_size + 3 ] = ui & 0xff;

	int wrote;
	wrote = fwrite( buf, 1, 54, fileptr );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
	if( wrote != 54 )
		{
		if( verb ) logpf( "audio_formats::fp_write_close_free_mem() - failed to write 54 bytes of aiff header for file: '%s', total bytes written: %u\n", pathname.c_str(), wrote );
		}
	}




//save wav header
if( ( fp_audio_format == en_af_wav_pcm ) & (fileptr != 0 ) )
	{
	fseek( fileptr, 0, SEEK_SET );                							//move to head of file

	strncpy( (char*)(buf + cn_riff_hdr_ckid), "RIFF", 4 );

	unsigned int data_chunk_cksize = fp_bytes_written;						//size of all the audio sample data in bytes
	unsigned int tot_chunk_size = data_chunk_cksize + 36;			//this number is 4 less than the total wav filesize

	*(unsigned int*)( buf + cn_riff_hdr_cksize ) = tot_chunk_size;

	strncpy( (char*)( buf + cn_riff_hdr_waveid ), "WAVE", 4 );


	strncpy( (char*)( buf + 12 + cn_riff_fmt_chunk_ckid ), "fmt ", 4 );            //store 'fmt '

	*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_cksize ) = 16;                 //always make the 'fmt ' this size


	*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_wformat_tag ) = 1;
	*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_wformat_tag + 1 ) = 0;

	*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_channels ) = fp_channels;
	*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_channels + 1 ) = 0;


	*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_samp_per_sec ) = fp_srate;


	unsigned int avg_bytes_per_sec = fp_srate * fp_channels * 2;
	*(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_avg_bytes_per_sec ) = avg_bytes_per_sec;


	*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_block_align ) = (unsigned char)( fp_channels * 2 );		//1 channel = 2, 2 channel = 4
	*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_block_align + 1 ) = 0;


	*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_bits_per_samp ) = 16;
	*(unsigned char*)( buf + 12 + cn_riff_fmt_chunk_bits_per_samp + 1 ) = 0;


	strncpy( (char*)( buf + 36 + cn_riff_data_chunk_ckid ), "data", 4 );

	*(unsigned int*)( buf + 36 + cn_riff_data_chunk_cksize ) = data_chunk_cksize;						//actual size of all sample data in bytes				

	int wrote;
	wrote = fwrite( buf, 1, 44, fileptr );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
	if( wrote != 44 )
		{
		if( verb ) logpf( "audio_formats::fp_write_close_free_mem() - failed to write 44 bytes of wav header for file: '%s', total bytes written: %u\n", pathname.c_str(), wrote );
		}
	}


if( fileptr ) fclose( fileptr );
fileptr = 0;


if( bf0 != 0 ) free( bf0 );
bf0 = 0;

if( bf1 != 0 ) free( bf1 );
bf1 = 0;


if( f_bf0 != 0 ) free( f_bf0 );
f_bf0 = 0;

if( f_bf1 != 0 ) free( f_bf1 );
f_bf1 = 0;


if( ucbf != 0 ) free( ucbf );
ucbf = 0;

pathname = "";
fp_audio_format = en_af_unknown;
fp_channels = 0;
fp_srate = 0;
fp_bytes_written = 0;
fp_offset = 0;
fp_encoding = 0;
fp_is_big_endian = 0;

}















//----------------------------------------------------------------------

//open an audio file for later reading, double precision mem block based, 'fread()' calls occur in: 'af.fp_read_double()'
// !!! make SURE this obj is destroyed to allow file closure and mem freeing,
//only supports: 'en_af_sun', 'en_af_wav_pcm', 'en_af_aiff', i.e: NO raw support
//auto discovers file format and sets:  'saf.format',  'saf.srate',  'saf.channels',  'saf.encode'(only for sun: 1=ulaw, 3=16bit)
//creates internal malloc sample buffers of 'bfsz',  these are accessed by: af.bf0[], af.bf1[] 
//audio sample levels are divided by 'scale_down_by_peak_int_val' to get down to to -/+ 1.0 range
//DOES not normalise audio,

//returns 1 on success
//returns -1 file not openable or not found
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::fp_open_double( string path, string fname, unsigned int bfsz, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
mystr m1;
string fname1 = path;
fname1 += fname;

fp_write_close_free_mem();


log = "";

unsigned long long int fsz;

if( !m1.filesize( fname1, fsz ) )
	{
    if( verb ) logpf( "audio_formats::fp_open_double() - couldn't get file size for file: '%s'\n", fname1.c_str() );
	fp_write_close_free_mem();
    return -1;
	}

if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::fp_open_double() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }



if( !find_file_format( path, fname, af.format ) )
	{
	if( verb ) logpf( "audio_formats::fp_open_double() - unknown audio file format: '%s'\n", fname.c_str() );
	return -5;
	}


fileptr = m1.mbc_fopen( fname1, "rb" );

if( fileptr == 0 ) 
	{
	if( verb ) logpf( "audio_formats::fp_open_double() - couldn't open a file pointer to filename: '%s'\n", fname.c_str() );
	fp_write_close_free_mem();
	return -1;
	}




unsigned char buf[256];

//read appropriate header for this audio file format


if( af.format == en_af_sun )
	{
	//read au header details

	unsigned int need = 24;                                      //this is minimum, there is an optional 'annotation field' that can be straight after the 24th byte

	int read = fread( buf, 1 ,need, fileptr );             //read 6 x 32bit to get sun's au audio file header


	if( read != need )
		{
		fp_write_close_free_mem();
		if( verb ) logpf( "audio_formats::fp_open_double() - sun's au audio header not big enough for file: '%s', only read: %u\n", fname1.c_str(), read );
		return -4;
		}

	//work out offset
	unsigned int offset = buf[ 7 ];                     //0x1
	offset += buf[ 6 ] * 256;                           //0x100
	offset += buf[ 5 ] * 65536;                         //0x10000
	offset += buf[ 4 ] * 16777216;                      //0x1000000
	//    cslpf( 0, "offset= %d\n", offset );

	//number of bytes of audio data
	//note -- this can be set to 0xffffffff, if the data size is unknown
	unsigned int data_block_size = buf[ 0xb ];          //0x1
	data_block_size += buf[ 0xa ] * 256;                //0x100
	data_block_size += buf[ 9 ] * 65536;                //0x10000
	data_block_size += buf[ 8 ] * 16777216;             //0x1000000
	//    cslpf( 0, "data_block_size= %d\n", data_block_size );


	//format of audio
	unsigned int encoding = buf[ 0xf ];                 //0x1
	encoding += buf[ 0xe ] * 256;                       //0x100
	encoding += buf[ 0xd ] * 65536;                     //0x10000
	encoding += buf[ 0xc ] * 16777216;                  //0x1000000
	//    cslpf( 0, "encoding= %d\n", encoding );

	//samplerate of audio
	unsigned int srate = buf[ 0x13 ];                   //0x1
	srate += buf[ 0x12 ] * 256;                         //0x100
	srate += buf[ 0x11 ] * 65536;                       //0x10000
	srate += buf[ 0x10 ] * 16777216;                    //0x1000000
	//    cslpf( 0, "srate= %d\n", srate );

	//channels of audio
	unsigned int channels = buf[ 0x17 ];                   //0x1
	channels += buf[ 0x16 ] * 256;                         //0x100
	channels += buf[ 0x15 ] * 65536;                       //0x10000
	channels += buf[ 0x14 ] * 16777216;                    //0x1000000
	//    cslpf( 0, "channels= %d\n", channels );


	bool supported = 0;
	if( encoding == 1 )                                     //8 bit u-law
		{
		supported = 1;
		af.bits_per_sample = 8;
		}
		
	if( encoding == 3 )                                     //16 bit pcm
		{
		supported = 1;
		af.bits_per_sample = 16;
		}



	if( !supported )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - un-supported encoding: %u, for file: '%s', %u\n", encoding, fname1.c_str() );
		fp_write_close_free_mem();
		return -5;
		}


	if( ( channels <= 0 ) || ( channels > 2 ) )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
		fp_write_close_free_mem();
		return -6;
		}


	af.path = path;
	af.fname = fname1;
	af.offset = offset;
	af.encoding = encoding;
	af.is_big_endian = 1;
	af.srate = srate;
	af.channels = channels;
	}




if( af.format == en_af_aiff )
	{
	//read aiff header details

	unsigned int need = 54;

	int read = fread( buf, 1 ,need, fileptr );             //read aiff audio file chunks etc...

	if( read != need )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - sun's au audio header not big enough for file: '%s', only read: %u\n", fname1.c_str(), read );
		fp_write_close_free_mem();
		return -4;
		}




	if( strncmp( (char*)( buf + cn_riff_fmt_chunk_ckid ), "FORM", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - no 'FORM ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	unsigned int form_cksize;
	form_cksize = buf[ cn_aiff_form_cksize ] << 24;
	form_cksize += buf[ cn_aiff_form_cksize + 1 ] << 16;
	form_cksize += buf[ cn_aiff_form_cksize + 2 ] << 8;
	form_cksize += buf[ cn_aiff_form_cksize + 3 ];


	if( strncmp( (char*)( buf + cn_aiff_aiff_ckid ), "AIFF", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - no 'AIFF ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	if( strncmp( (char*)( buf + cn_aiff_comm_ckid ), "COMM", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - no 'COMM ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	unsigned int comm_cksize;
	comm_cksize = buf[ cn_aiff_comm_cksize ] << 24;
	comm_cksize += buf[ cn_aiff_comm_cksize + 1 ] << 16;
	comm_cksize += buf[ cn_aiff_comm_cksize + 2 ] << 8;
	comm_cksize += buf[ cn_aiff_comm_cksize + 3 ];


	unsigned int channels;
	channels = buf[ cn_aiff_comm_channels ] << 8;
	channels += buf[ cn_aiff_comm_channels + 1 ];


	unsigned int sample_frames;
	sample_frames = buf[ cn_aiff_comm_cksize ] << 24;
	sample_frames += buf[ cn_aiff_comm_cksize + 1 ] << 16;
	sample_frames + buf[ cn_aiff_comm_sample_frames + 2 ] << 8;
	sample_frames += buf[ cn_aiff_comm_sample_frames + 3 ];


	unsigned int bits_per_sample;
	bits_per_sample = buf[ cn_aiff_comm_sample_size ] << 8;
	bits_per_sample += buf[ cn_aiff_comm_sample_size + 1 ];




	unsigned char buf_srate[ 10 ];

	buf_srate[ 9 ] = buf[ cn_aiff_comm_sample_srate ];          //read in big endian 80 bit IEEE 754 extended precision floating point number (long double)
	buf_srate[ 8 ] = buf[ cn_aiff_comm_sample_srate + 1 ];      //convert to little endian
	buf_srate[ 7 ] = buf[ cn_aiff_comm_sample_srate + 2 ];
	buf_srate[ 6 ] = buf[ cn_aiff_comm_sample_srate + 3 ];
	buf_srate[ 5 ] = buf[ cn_aiff_comm_sample_srate + 4 ];
	buf_srate[ 4 ] = buf[ cn_aiff_comm_sample_srate + 5 ];
	buf_srate[ 3 ] = buf[ cn_aiff_comm_sample_srate + 6 ];
	buf_srate[ 2 ] = buf[ cn_aiff_comm_sample_srate + 7 ];
	buf_srate[ 1 ] = buf[ cn_aiff_comm_sample_srate + 8 ];
	buf_srate[ 0 ] = buf[ cn_aiff_comm_sample_srate + 9 ];

	long double ld = *(long double*)buf_srate;

	unsigned int srate = ld;                                   //convert srate to int




	if( strncmp( (char*)( buf + cn_aiff_comm_ssnd_ckid ), "SSND", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - no 'SSND ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}



	unsigned int ssnd_cksize;
	ssnd_cksize = buf[ cn_aiff_comm_ssnd_cksize ] << 24;
	ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 1 ] << 16;
	ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 2 ] << 8;
	ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 3 ];



	unsigned int offset;
	offset = buf[ cn_aiff_comm_ssnd_offset ] << 24;
	offset += buf[ cn_aiff_comm_ssnd_offset + 1 ] << 16;
	offset += buf[ cn_aiff_comm_ssnd_offset + 2 ] << 8;
	offset += buf[ cn_aiff_comm_ssnd_offset + 3 ];



	unsigned int align_byte_cnt ;
	align_byte_cnt = buf[ cn_aiff_comm_ssnd_block_size ] << 24;
	align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 1 ] << 16;
	align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 2 ] << 8;
	align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 3 ];


	if( ( channels <= 0 ) || ( channels > 2 ) )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
		fp_write_close_free_mem();
		return -6;
		}


	if( bits_per_sample != 16 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - for filename: '%s', bets_per_sample is not supported: %u\n", fname1.c_str(), bits_per_sample );
		fp_write_close_free_mem();
		return -5;
		}


	af.path = path;
	af.fname = fname1;
	af.offset = offset;
	af.encoding = 0;
	af.is_big_endian = 1;
	af.srate = srate;
	af.channels = channels;
	af.bits_per_sample = bits_per_sample;
	}


if( af.format == en_af_wav_pcm )
	{
	unsigned int need = 12 + 24;							//size of 'hdr' + 'fmt chunk' and begining of 'data chunk', this will reveal various values which will determine
													//actual size of the sample data bytes
	unsigned long read = fread( buf, 1, need, fileptr );          	//read block of audio data

	if( read != need )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - failed to read header bytes: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}



	if( strncmp( (char*)( buf + cn_riff_hdr_ckid ), "RIFF", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - no 'RIFF' found in file header: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	unsigned int tot_chunk_size = *(unsigned int*)( buf + cn_riff_hdr_cksize );

	if( verb ) logpf( "audio_formats::fp_open_double() - tot_chunk_size size: %u\n", tot_chunk_size );



	if( strncmp( (char*)( buf + cn_riff_hdr_waveid ), "WAVE", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - no 'WAVE' found in file header: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}



	if( strncmp( (char*)( buf + 12 + cn_riff_fmt_chunk_ckid ), "fmt ", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - no 'fmt ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	unsigned int fmt_chunk_size;
	unsigned int format_tag;
	unsigned int wave_channels;
	unsigned int samp_per_sec;
	unsigned int avg_bytes_per_sec;
	unsigned int block_align;
	unsigned int bits_per_sample;
	unsigned int cbsize;
	unsigned int data_chunk_size;


	fmt_chunk_size = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_cksize );



	if( fmt_chunk_size != 16 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - fmt_chunk_size is not equal to 16 (at location 16), ignoring this, file: '%s'\n", fname1.c_str() );
	//    free( buf );
	//	fclose( fp );
	//	return 0;
		}


	if( verb ) logpf( "audio_formats::fp_open_double() - fmt_chunk_size: %u\n", fmt_chunk_size );


	format_tag = buf[ 12 + cn_riff_fmt_chunk_wformat_tag ];
	format_tag += buf[ 12 + cn_riff_fmt_chunk_wformat_tag + 1 ] << 8;

	if( verb ) logpf( "audio_formats::fp_open_double() - format_tag: %u\n", format_tag );


	if( format_tag != 1 )                   //not a pcm format?
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - unsupported format_tag: %u, file: '%s'\n", format_tag, fname1.c_str() );
		fp_write_close_free_mem();
		return -5;
		}

	wave_channels = buf[ 12 + cn_riff_fmt_chunk_channels ];
	wave_channels += buf[ 12 + cn_riff_fmt_chunk_channels + 1 ] << 8;

	if( verb ) logpf( "audio_formats::fp_open_double() - wave_channels: %u\n", wave_channels );

	if( ( wave_channels <= 0 ) || ( wave_channels > 2 ) )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - unsupported number of channels: %u\n", wave_channels );
		fp_write_close_free_mem();
		return -6;
		}



	samp_per_sec = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_samp_per_sec );

	if( verb ) logpf( "audio_formats::fp_open_double() - samples_per_sec: %u\n", samp_per_sec );



	avg_bytes_per_sec = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_avg_bytes_per_sec );

	if( verb ) logpf( "audio_formats::fp_open_double() - avg_bytes_per_sec: %u\n", avg_bytes_per_sec );


	block_align = buf[ 12 + cn_riff_fmt_chunk_block_align ];
	block_align += buf[ 12 + cn_riff_fmt_chunk_block_align + 1 ] << 8;

	if( verb ) logpf( "audio_formats::fp_open_double() - block_align: %u\n", block_align );


	bits_per_sample = buf[ 12 + cn_riff_fmt_chunk_bits_per_samp ];
	bits_per_sample += buf[ 12 + cn_riff_fmt_chunk_bits_per_samp + 1 ] << 8;

	if( verb ) logpf( "audio_formats::fp_open_double() - bits_per_sample: %u\n", bits_per_sample );



	unsigned int data_chuck_pos;

	if( !find_in_file( fileptr, "data", data_chuck_pos ) )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - no 'data' chunk was found in file: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}
	//getchar();


	//data_chunk_size = *(unsigned int*)( buf + 12 + 24 + cn_riff_data_chunk_cksize );


	read = fread( buf, 1, 4, fileptr );          				//read data chunk's size bytes
	if( read != 4 )
		{
		if( verb ) logpf( "audio_formats::fp_open_double() - the file ended before data chunk's size bytes could be read: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}

	data_chunk_size = *(unsigned int*)( buf );

	if( verb ) logpf( "audio_formats::fp_open_double() - data_chunk_size: %u\n", data_chunk_size );


	unsigned int samples_per_channel = data_chunk_size / ( wave_channels * ( bits_per_sample / 8 ) );

	if( verb ) logpf( "audio_formats::fp_open_double() - samples_per_channel: %u\n", samples_per_channel );

	af.path = path;
	af.fname = fname1;
	af.offset = 0;
	af.encoding = 0;
	af.is_big_endian = 0;
	af.srate = samp_per_sec;
	af.channels = wave_channels;
	af.bits_per_sample = bits_per_sample;
	}



//alloc mem
if( bfsz == 0 )
	{
	if( verb ) logpf( "audio_formats::fp_open_double() - buf_size spec was zero: '%s'\n", fname.c_str() );
	return -7;
	}

bf0 = (double*) malloc( bfsz * sizeof(double) );
if( bf0 == 0 ) 
	{
	logpf( "audio_formats::fp_open_double() - unable to alloc bf0 of size: %d, filename: '%s'\n", bfsz*sizeof(double), fname.c_str() );
	return -7;
	}

bf1 = (double*) malloc( bfsz * sizeof(double) );
if( bf1 == 0 ) 
	{
	logpf( "audio_formats::fp_open_double() - unable to alloc bf1 of size: %d, filename: '%s'\n", bfsz*sizeof(double), fname.c_str() );
	fp_write_close_free_mem();
	return -7;
	}

ucbf = (unsigned char*) malloc( bfsz * 4 );		//assume 2 channel (16 bit)
if( ucbf == 0 ) 
	{
	logpf( "audio_formats::fp_open_double() - unable to alloc ucbf of size: %d, filename: '%s'\n", bfsz * 4, fname.c_str() );
	fp_write_close_free_mem();
    return -2;
	}

buf_size = bfsz;
file_size = fsz;
pathname = fname1;

req_scale_down_by_peak_int_val = scale_down_by_peak_int_val;

return 1;
}








//reads 'count' samples from prev opened audio file, data stored in: 'af.bf0[]', 'af.bf1[]', 
//audio sample levels are divided by prev defined: 'scale_down_by_peak_int_val' to get down to to -/+ 1.0 range
//DOES not normalise audio,

//returns number of samples read, else a neg val specifying error
int audio_formats::fp_read_double( unsigned int count, st_audio_formats_tag &af )
{
if( fileptr == 0 ) 
	{
	if( verb ) logpf( "audio_formats::fp_read_double() - file not open: '%s'\n", pathname.c_str() );
	return -1;
	}

if( af.channels == 0 )
	{
	if( verb ) logpf( "audio_formats::fp_read_double() - channels found is zero: '%s'\n", pathname.c_str() );
	return -2;
	}

if( count == 0 ) return 0;


if( count >= buf_size ) 
	{
	if( verb ) logpf( "audio_formats::fp_read_double() - count: %u, is larger than alloc buf: %u: '%s'\n", count, buf_size, pathname.c_str() );
	return -3;
	}

string s1;
double min = 0;                                     //for normalising audio
double max = 0;
double dd;

bool trk2 = 0;
char cc;
int iaud;

int read;

bool is_ulaw = 0;
if( ( af.format == en_af_sun) & ( af.encoding == 1 ) ) is_ulaw = 1;

int need = count;

need *= af.channels;

if( need > buf_size ) need = buf_size;

int sample_byte_size = 2;
if( is_ulaw ) sample_byte_size = 1;


read = fread( ucbf, 1, need * sample_byte_size, fileptr );          	//read block of audio data, 2 bytes per sample


if( read != 0 )
	{
	int idx = 0;
	for( int i = 0; i < read; )                 //process block
		{
		if( !is_ulaw) i++;

		if( i >= read )
			{
			if( verb ) logpf( "audio_formats::fp_read_double() - odd number of bytes encountered, lost final sample\n" );
			break;
			}

		if( !af.is_big_endian )
			{

			cc = ucbf[ i ];						    //preserve sign in msbyte (little endian)

			iaud = cc * 256;						//shift msbyte to its location preserving sign
			iaud |= ucbf[ i - 1 ];					//or in lsbyte
			}
		else{
			if( is_ulaw )
				{    
				cc = ucbf[ i ];					   	 	//preserve sign in msbyte (little endian)
				iaud = ulaw2linear( cc );
				}
			else{
				iaud = (char)ucbf[ i - 1 ] * 256.0;      //preserve sign (big endian)
				iaud += ucbf[ i ];
				}
			}
			


		dd = (double)iaud / req_scale_down_by_peak_int_val;

		i++;

		if( af.channels == 1 )                //1 trk?
			{
			bf0[ idx++ ] = dd;
//if( idx < 5 ) printf("rd0: %f, iaud: %d\n", dd, iaud );
			}
		else{
			if( trk2 == 0 )
				{
				bf0[ idx ] = dd;
//if( idx < 5 ) printf("rd0: %f, iaud: %d\n", dd, iaud );
				}
			else{
				bf1[ idx++ ] = dd;
//if( idx < 6 ) printf("rd1: %f, iaud: %d\n", dd, iaud );
				}
			trk2 = !trk2;
			}

	//		if( dd < min ) { min = dd; min_sample_idx = i; }
	//		if( dd > max ) { max = dd; max_sample_idx = i; }
		}
	//logpf("read: %d\n", read );
	}

return (read / sample_byte_size / af.channels);
}




































//create an audio file for later saving, double precision mem block based, 'fwrite()' calls occur in: 'af.fp_write_double()'
// !!! make SURE this obj is destroyed to allow writing of the audio file header params, file closure and mem freeing,
//only supports: 'en_af_sun', 'en_af_wav_pcm', 'en_af_aiff', i.e: NO raw support
//NEEDS these set:  'saf.format',  'saf.srate',  'saf.channels',  'saf.encode'(only for sun: 1=ulaw, 3=16bit)
//creates internal malloc sample buffers of 'bfsz',  these are accessed by: af.bf0[], af.bf1[] 
//samples should be between +/- 1.0, saved samples are scaled up by 'peak_int_val'
//DOES not normalise audio,

//returns 1 on success
//returns -1 file not openable for writing
//returns -2 channel count specified is not supported
//returns -3 memory not allocated
//returns -4 write to file failed
int audio_formats::fp_create_double( string path, string fname, unsigned int bfsz, int peak_int_val, st_audio_formats_tag &af )
{
mystr m1;
string fname1 = path;
fname1 += fname;

fp_write_close_free_mem();


log = "";

af.path = path;
af.fname = fname1;
af.offset = 0;
af.is_big_endian = 0;
af.bits_per_sample = 16;


if( ( af.channels == 0 ) | ( af.channels > 2 ) )
	{
	if( verb ) logpf( "audio_formats::fp_create_double() - unsupported channel count specified: %d, filename: '%s'\n", af.channels, fname.c_str() );
	fp_write_close_free_mem();
	return -1;
	}


//alloc mem
if( bfsz == 0 )
	{
	if( verb ) logpf( "audio_formats::fp_create_double() - buf_size spec was zero: '%s'\n", fname.c_str() );
	fp_write_close_free_mem();
	return -7;
	}

bf0 = (double*) malloc( bfsz * sizeof(double) );
if( bf0 == 0 ) 
	{
	logpf( "audio_formats::fp_create_double() - unable to alloc bf0 of size: %d, filename: '%s'\n", bfsz*sizeof(double), fname.c_str() );
	fp_write_close_free_mem();
	return -7;
	}

bf1 = (double*) malloc( bfsz * sizeof(double) );
if( bf1 == 0 ) 
	{
	logpf( "audio_formats::fp_create_double() - unable to alloc bf1 of size: %d, filename: '%s'\n", bfsz*sizeof(double), fname.c_str() );
	fp_write_close_free_mem();
	return -7;
	}

ucbf = (unsigned char*) malloc( bfsz * 4 );		//assume 2 channel (16 bit)
if( ucbf == 0 ) 
	{
	logpf( "audio_formats::fp_create_double() - unable to alloc ucbf of size: %d, filename: '%s'\n", bfsz * 4, fname.c_str() );
	fp_write_close_free_mem();
    return -2;
	}




fileptr = m1.mbc_fopen( fname1, "wb" );

if( fileptr == 0 ) 
	{
	if( verb ) logpf( "audio_formats::fp_create_double() - couldn't open a file pointer to filename: '%s'\n", fname.c_str() );
	fp_write_close_free_mem();
	return -1;
	}



//make a hole for header, it will be filled in when file is closed
unsigned char buf[256];

if( af.format == en_af_sun )
	{
	af.is_big_endian = 1;
	if( af.encoding != 1 ) af.encoding = 3;							//default to 16 bit when not ulaw
	
	af.offset = 28;
	int wrote;
	wrote = fwrite( buf, 1, 7 * 4, fileptr );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
	if( wrote != 7 * 4 )
		{
		if( verb ) logpf( "audio_formats::fp_create_double() - failed to write 28 bytes of sun header for file: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}
	}

if( af.format == en_af_aiff )
	{
	af.encoding = 0;
	af.is_big_endian = 1;

	int wrote;
	wrote = fwrite( buf, 1, 54, fileptr );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
	if( wrote != 54 )
		{
		if( verb ) logpf( "audio_formats::fp_create_double() - failed to write 54 bytes of aiff header for file: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}
	}


if( af.format == en_af_wav_pcm )
	{
	af.encoding = 0;

	int wrote;
	wrote = fwrite( buf, 1, 44, fileptr );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
	if( wrote != 44 )
		{
		if( verb ) logpf( "audio_formats::fp_create_double() - failed to write 44 bytes of wav header for file: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}
	}



buf_size = bfsz;
pathname = fname1;

req_peak_int_val = peak_int_val;

fp_audio_format = af.format;
fp_srate = af.srate;
fp_channels = af.channels;
fp_offset = af.offset;
fp_encoding = af.encoding;
fp_is_big_endian = af.is_big_endian;
return 1;
}









//writes 'count' samples from prev created audio file, data read from: 'af.bf0[]', 'af.bf1[]',
//samples should be between +/- 1.0, saved samples are scaled up by prev defined: 'peak_int_val'
//DOES not normalise audio,

//returns number of samples writtn, else a neg val spec error
//returns -1 file not open
//returns -2 channel count specified is not supported
//returns -3 count exceeds buf size
//returns -4 write to file failed
int audio_formats::fp_write_double( unsigned int count, st_audio_formats_tag &af )
{
if( fileptr == 0 ) 
	{
	if( verb ) logpf( "audio_formats::fp_write_double() - file not open: '%s'\n", pathname.c_str() );
	return -1;
	}

if( af.channels == 0 )
	{
	if( verb ) logpf( "audio_formats::fp_write_double() - channels found is zero: '%s'\n", pathname.c_str() );
	return -2;
	}

if( count == 0 ) return 0;


if( count >= buf_size ) 
	{
	if( verb ) logpf( "audio_formats::fp_write_double() - count: %u, is larger than alloc buf: %u: '%s'\n", count, buf_size, pathname.c_str() );
	return -3;
	}

int iaud;

//bool is_ulaw = 0;
//if( ( af.format == en_af_sun) & ( af.encoding == 1 ) ) is_ulaw = 1;


bool trk2 = 0;

if( af.channels == 2 ) trk2 = 1;

unsigned int j = 0;
unsigned int wrote = 0;

double dd;
unsigned char uc;

int bytes_per_sample = 2;

//8 bit
if( ( af.format == en_af_sun ) & ( af.encoding == 1 ) )							//ulaw
	{
	bytes_per_sample = 1;
	for( int i = 0; i < count; i++ )
		{
		dd = bf0[ i ]; 
        uc = linear2ulaw( nearbyint( dd * (double)req_peak_int_val ) );			//v1.04
		
		ucbf[ j++ ] = uc;
		
//		if( i < 5 ) printf("ulaw wr0: %f, cc: %d\n", dd, (unsigned int)uc );

		if( trk2 ) 
			{
			dd = bf1[ i ];
			uc = linear2ulaw( nearbyint( dd * (double)req_peak_int_val ) );		//v1.04
			ucbf[ j++ ] = uc;
//			if( i < 5 ) printf("ulaw wr0: %f, cc: %d\n", dd, (unsigned int)uc );
			}
		}
	}
else{
//16 bit
	if( !af.is_big_endian )
		{
		for( int i = 0; i < count; i++ )
			{
			dd = bf0[ i ];
			iaud = nearbyint( dd * (double)req_peak_int_val );
			
			
			ucbf[ j++ ] =  iaud & 0xff;
			ucbf[ j++ ] = ( iaud >> 8  ) & 0xff;               			//little endian

//			if( i < 5 ) printf("wr0: %f, iaud: %d\n", dd, iaud );

			if( trk2 ) 
				{
				dd = bf1[ i ];
				iaud = nearbyint( dd * (double)req_peak_int_val );
				ucbf[ j++ ] =  iaud & 0xff;
				ucbf[ j++ ] = ( iaud >> 8  ) & 0xff;               		//little endian
//				if( i < 5 ) printf("wr1: %f, iaud: %d\n", dd, iaud );
				}
			}

		}
	else{
//16 bit
		for( int i = 0; i < count; i++ )
			{
			dd = bf0[ i ];
			iaud = nearbyint( dd * (double)req_peak_int_val );
			ucbf[ j++ ] = ( iaud >> 8  ) & 0xff;               			//big endian
			ucbf[ j++ ] =  iaud & 0xff;

//			if( i < 5 ) printf("wr0: %f, iaud: %d\n", dd, iaud );

			if( trk2 ) 
				{
				dd = bf1[ i ];
				iaud = nearbyint( dd * (double)req_peak_int_val );
				ucbf[ j++ ] = ( iaud >> 8  ) & 0xff;               		//big endian
				ucbf[ j++ ] =  iaud & 0xff;

//				if( i < 5 ) printf("wr1: %f, iaud: %d\n", dd, iaud );
				}
			}
		}
	}

wrote = fwrite( ucbf, 1, j, fileptr );          //write block of audio data
if( wrote != j )
	{
	if( verb ) logpf( "audio_formats::fp_write_double() - failed to complete a write to file: '%s', total bytes written: %u\n", pathname.c_str(), wrote );
	return -4;
	}

fp_bytes_written += wrote;

return (wrote / bytes_per_sample / af.channels);
}

//----------------------------------------------------------------------
















//----------------------------------------------------------------------

//open an audio file for later reading, float precision mem block based, 'fread()' calls occur in: 'af.fp_read_float()'
// !!! make SURE this obj is destroyed to allow file closure and mem freeing,
//only supports: 'en_af_sun', 'en_af_wav_pcm', 'en_af_aiff', i.e: NO raw support
//auto discovers file format and sets:  'saf.format',  'saf.srate',  'saf.channels',  'saf.encode'(only for sun: 1=ulaw, 3=16bit)
//creates internal malloc sample buffers of 'bfsz',  these are accessed by: af.f_bf0[], af.f_bf1[] 
//audio sample levels are divided by 'scale_down_by_peak_int_val' to get down to to -/+ 1.0 range
//DOES not normalise audio,

//returns 1 on success
//returns -1 file not openable or not found
//returns -3 file is zero
//returns -4 file header not correct
//returns -5 un-supported encoding
//returns -6 channel count specified is not supported
//returns -7 memory not allocated
int audio_formats::fp_open_float( string path, string fname, unsigned int bfsz, int scale_down_by_peak_int_val, st_audio_formats_tag &af )
{
mystr m1;
string fname1 = path;
fname1 += fname;

fp_write_close_free_mem();


log = "";

unsigned long long int fsz;

if( !m1.filesize( fname1, fsz ) )
	{
    if( verb ) logpf( "audio_formats::fp_open_float() - couldn't get file size for file: '%s'\n", fname1.c_str() );
	fp_write_close_free_mem();
    return -1;
	}

if( fsz == 0 )
    {
    if( verb ) logpf( "audio_formats::fp_open_float() - filename: '%s' is empty\n", fname1.c_str() );
    return -3;
    }



if( !find_file_format( path, fname, af.format ) )
	{
	if( verb ) logpf( "audio_formats::fp_open_float() - unknown audio file format: '%s'\n", fname.c_str() );
	return -5;
	}


fileptr = m1.mbc_fopen( fname1, "rb" );

if( fileptr == 0 ) 
	{
	if( verb ) logpf( "audio_formats::fp_open_float() - couldn't open a file pointer to filename: '%s'\n", fname.c_str() );
	fp_write_close_free_mem();
	return -1;
	}




unsigned char buf[256];

//read appropriate header for this audio file format


if( af.format == en_af_sun )
	{
	//read au header details

	unsigned int need = 24;                                      //this is minimum, there is an optional 'annotation field' that can be straight after the 24th byte

	int read = fread( buf, 1 ,need, fileptr );             //read 6 x 32bit to get sun's au audio file header


	if( read != need )
		{
		fp_write_close_free_mem();
		if( verb ) logpf( "audio_formats::fp_open_float() - sun's au audio header not big enough for file: '%s', only read: %u\n", fname1.c_str(), read );
		return -4;
		}

	//work out offset
	unsigned int offset = buf[ 7 ];                     //0x1
	offset += buf[ 6 ] * 256;                           //0x100
	offset += buf[ 5 ] * 65536;                         //0x10000
	offset += buf[ 4 ] * 16777216;                      //0x1000000
	//    cslpf( 0, "offset= %d\n", offset );

	//number of bytes of audio data
	//note -- this can be set to 0xffffffff, if the data size is unknown
	unsigned int data_block_size = buf[ 0xb ];          //0x1
	data_block_size += buf[ 0xa ] * 256;                //0x100
	data_block_size += buf[ 9 ] * 65536;                //0x10000
	data_block_size += buf[ 8 ] * 16777216;             //0x1000000
	//    cslpf( 0, "data_block_size= %d\n", data_block_size );


	//format of audio
	unsigned int encoding = buf[ 0xf ];                 //0x1
	encoding += buf[ 0xe ] * 256;                       //0x100
	encoding += buf[ 0xd ] * 65536;                     //0x10000
	encoding += buf[ 0xc ] * 16777216;                  //0x1000000
	//    cslpf( 0, "encoding= %d\n", encoding );

	//samplerate of audio
	unsigned int srate = buf[ 0x13 ];                   //0x1
	srate += buf[ 0x12 ] * 256;                         //0x100
	srate += buf[ 0x11 ] * 65536;                       //0x10000
	srate += buf[ 0x10 ] * 16777216;                    //0x1000000
	//    cslpf( 0, "srate= %d\n", srate );

	//channels of audio
	unsigned int channels = buf[ 0x17 ];                   //0x1
	channels += buf[ 0x16 ] * 256;                         //0x100
	channels += buf[ 0x15 ] * 65536;                       //0x10000
	channels += buf[ 0x14 ] * 16777216;                    //0x1000000
	//    cslpf( 0, "channels= %d\n", channels );


	bool supported = 0;
	if( encoding == 1 )                                     //8 bit u-law
		{
		supported = 1;
		af.bits_per_sample = 8;
		}
		
	if( encoding == 3 )                                     //16 bit pcm
		{
		supported = 1;
		af.bits_per_sample = 16;
		}



	if( !supported )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - un-supported encoding: %u, for file: '%s', %u\n", encoding, fname1.c_str() );
		fp_write_close_free_mem();
		return -5;
		}


	if( ( channels <= 0 ) || ( channels > 2 ) )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
		fp_write_close_free_mem();
		return -6;
		}


	af.path = path;
	af.fname = fname1;
	af.offset = offset;
	af.encoding = encoding;
	af.is_big_endian = 1;
	af.srate = srate;
	af.channels = channels;
	}




if( af.format == en_af_aiff )
	{
	//read aiff header details

	unsigned int need = 54;

	int read = fread( buf, 1 ,need, fileptr );             //read aiff audio file chunks etc...

	if( read != need )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - sun's au audio header not big enough for file: '%s', only read: %u\n", fname1.c_str(), read );
		fp_write_close_free_mem();
		return -4;
		}




	if( strncmp( (char*)( buf + cn_riff_fmt_chunk_ckid ), "FORM", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - no 'FORM ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	unsigned int form_cksize;
	form_cksize = buf[ cn_aiff_form_cksize ] << 24;
	form_cksize += buf[ cn_aiff_form_cksize + 1 ] << 16;
	form_cksize += buf[ cn_aiff_form_cksize + 2 ] << 8;
	form_cksize += buf[ cn_aiff_form_cksize + 3 ];


	if( strncmp( (char*)( buf + cn_aiff_aiff_ckid ), "AIFF", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - no 'AIFF ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	if( strncmp( (char*)( buf + cn_aiff_comm_ckid ), "COMM", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - no 'COMM ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	unsigned int comm_cksize;
	comm_cksize = buf[ cn_aiff_comm_cksize ] << 24;
	comm_cksize += buf[ cn_aiff_comm_cksize + 1 ] << 16;
	comm_cksize += buf[ cn_aiff_comm_cksize + 2 ] << 8;
	comm_cksize += buf[ cn_aiff_comm_cksize + 3 ];


	unsigned int channels;
	channels = buf[ cn_aiff_comm_channels ] << 8;
	channels += buf[ cn_aiff_comm_channels + 1 ];


	unsigned int sample_frames;
	sample_frames = buf[ cn_aiff_comm_cksize ] << 24;
	sample_frames += buf[ cn_aiff_comm_cksize + 1 ] << 16;
	sample_frames + buf[ cn_aiff_comm_sample_frames + 2 ] << 8;
	sample_frames += buf[ cn_aiff_comm_sample_frames + 3 ];


	unsigned int bits_per_sample;
	bits_per_sample = buf[ cn_aiff_comm_sample_size ] << 8;
	bits_per_sample += buf[ cn_aiff_comm_sample_size + 1 ];




	unsigned char buf_srate[ 10 ];

	buf_srate[ 9 ] = buf[ cn_aiff_comm_sample_srate ];          //read in big endian 80 bit IEEE 754 extended precision floating point number (long double)
	buf_srate[ 8 ] = buf[ cn_aiff_comm_sample_srate + 1 ];      //convert to little endian
	buf_srate[ 7 ] = buf[ cn_aiff_comm_sample_srate + 2 ];
	buf_srate[ 6 ] = buf[ cn_aiff_comm_sample_srate + 3 ];
	buf_srate[ 5 ] = buf[ cn_aiff_comm_sample_srate + 4 ];
	buf_srate[ 4 ] = buf[ cn_aiff_comm_sample_srate + 5 ];
	buf_srate[ 3 ] = buf[ cn_aiff_comm_sample_srate + 6 ];
	buf_srate[ 2 ] = buf[ cn_aiff_comm_sample_srate + 7 ];
	buf_srate[ 1 ] = buf[ cn_aiff_comm_sample_srate + 8 ];
	buf_srate[ 0 ] = buf[ cn_aiff_comm_sample_srate + 9 ];

	long double ld = *(long double*)buf_srate;

	unsigned int srate = ld;                                   //convert srate to int




	if( strncmp( (char*)( buf + cn_aiff_comm_ssnd_ckid ), "SSND", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - no 'SSND ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}



	unsigned int ssnd_cksize;
	ssnd_cksize = buf[ cn_aiff_comm_ssnd_cksize ] << 24;
	ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 1 ] << 16;
	ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 2 ] << 8;
	ssnd_cksize += buf[ cn_aiff_comm_ssnd_cksize + 3 ];



	unsigned int offset;
	offset = buf[ cn_aiff_comm_ssnd_offset ] << 24;
	offset += buf[ cn_aiff_comm_ssnd_offset + 1 ] << 16;
	offset += buf[ cn_aiff_comm_ssnd_offset + 2 ] << 8;
	offset += buf[ cn_aiff_comm_ssnd_offset + 3 ];



	unsigned int align_byte_cnt ;
	align_byte_cnt = buf[ cn_aiff_comm_ssnd_block_size ] << 24;
	align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 1 ] << 16;
	align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 2 ] << 8;
	align_byte_cnt += buf[ cn_aiff_comm_ssnd_block_size + 3 ];


	if( ( channels <= 0 ) || ( channels > 2 ) )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - for filename: '%s', channel count specified is not supported: %u\n", fname1.c_str(), channels );
		fp_write_close_free_mem();
		return -6;
		}


	if( bits_per_sample != 16 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - for filename: '%s', bets_per_sample is not supported: %u\n", fname1.c_str(), bits_per_sample );
		fp_write_close_free_mem();
		return -5;
		}


	af.path = path;
	af.fname = fname1;
	af.offset = offset;
	af.encoding = 0;
	af.is_big_endian = 1;
	af.srate = srate;
	af.channels = channels;
	af.bits_per_sample = bits_per_sample;
	}


if( af.format == en_af_wav_pcm )
	{
	unsigned int need = 12 + 24;							//size of 'hdr' + 'fmt chunk' and begining of 'data chunk', this will reveal various values which will determine
													//actual size of the sample data bytes
	unsigned long read = fread( buf, 1, need, fileptr );          	//read block of audio data

	if( read != need )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - failed to read header bytes: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}



	if( strncmp( (char*)( buf + cn_riff_hdr_ckid ), "RIFF", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - no 'RIFF' found in file header: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	unsigned int tot_chunk_size = *(unsigned int*)( buf + cn_riff_hdr_cksize );

	if( verb ) logpf( "audio_formats::fp_open_float() - tot_chunk_size size: %u\n", tot_chunk_size );



	if( strncmp( (char*)( buf + cn_riff_hdr_waveid ), "WAVE", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - no 'WAVE' found in file header: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}



	if( strncmp( (char*)( buf + 12 + cn_riff_fmt_chunk_ckid ), "fmt ", 4 ) != 0 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - no 'fmt ' found in chunk: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}


	unsigned int fmt_chunk_size;
	unsigned int format_tag;
	unsigned int wave_channels;
	unsigned int samp_per_sec;
	unsigned int avg_bytes_per_sec;
	unsigned int block_align;
	unsigned int bits_per_sample;
	unsigned int cbsize;
	unsigned int data_chunk_size;


	fmt_chunk_size = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_cksize );



	if( fmt_chunk_size != 16 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - fmt_chunk_size is not equal to 16 (at location 16), ignoring this, file: '%s'\n", fname1.c_str() );
	//    free( buf );
	//	fclose( fp );
	//	return 0;
		}


	if( verb ) logpf( "audio_formats::fp_open_float() - fmt_chunk_size: %u\n", fmt_chunk_size );


	format_tag = buf[ 12 + cn_riff_fmt_chunk_wformat_tag ];
	format_tag += buf[ 12 + cn_riff_fmt_chunk_wformat_tag + 1 ] << 8;

	if( verb ) logpf( "audio_formats::fp_open_float() - format_tag: %u\n", format_tag );


	if( format_tag != 1 )                   //not a pcm format?
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - unsupported format_tag: %u, file: '%s'\n", format_tag, fname1.c_str() );
		fp_write_close_free_mem();
		return -5;
		}

	wave_channels = buf[ 12 + cn_riff_fmt_chunk_channels ];
	wave_channels += buf[ 12 + cn_riff_fmt_chunk_channels + 1 ] << 8;

	if( verb ) logpf( "audio_formats::fp_open_float() - wave_channels: %u\n", wave_channels );

	if( ( wave_channels <= 0 ) || ( wave_channels > 2 ) )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - unsupported number of channels: %u\n", wave_channels );
		fp_write_close_free_mem();
		return -6;
		}



	samp_per_sec = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_samp_per_sec );

	if( verb ) logpf( "audio_formats::fp_open_float() - samples_per_sec: %u\n", samp_per_sec );



	avg_bytes_per_sec = *(unsigned int*)( buf + 12 + cn_riff_fmt_chunk_avg_bytes_per_sec );

	if( verb ) logpf( "audio_formats::fp_open_float() - avg_bytes_per_sec: %u\n", avg_bytes_per_sec );


	block_align = buf[ 12 + cn_riff_fmt_chunk_block_align ];
	block_align += buf[ 12 + cn_riff_fmt_chunk_block_align + 1 ] << 8;

	if( verb ) logpf( "audio_formats::fp_open_float() - block_align: %u\n", block_align );


	bits_per_sample = buf[ 12 + cn_riff_fmt_chunk_bits_per_samp ];
	bits_per_sample += buf[ 12 + cn_riff_fmt_chunk_bits_per_samp + 1 ] << 8;

	if( verb ) logpf( "audio_formats::fp_open_float() - bits_per_sample: %u\n", bits_per_sample );



	unsigned int data_chuck_pos;

	if( !find_in_file( fileptr, "data", data_chuck_pos ) )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - no 'data' chunk was found in file: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}
	//getchar();


	//data_chunk_size = *(unsigned int*)( buf + 12 + 24 + cn_riff_data_chunk_cksize );


	read = fread( buf, 1, 4, fileptr );          				//read data chunk's size bytes
	if( read != 4 )
		{
		if( verb ) logpf( "audio_formats::fp_open_float() - the file ended before data chunk's size bytes could be read: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}

	data_chunk_size = *(unsigned int*)( buf );

	if( verb ) logpf( "audio_formats::fp_open_float() - data_chunk_size: %u\n", data_chunk_size );


	unsigned int samples_per_channel = data_chunk_size / ( wave_channels * ( bits_per_sample / 8 ) );

	if( verb ) logpf( "audio_formats::fp_open_float() - samples_per_channel: %u\n", samples_per_channel );

	af.path = path;
	af.fname = fname1;
	af.offset = 0;
	af.encoding = 0;
	af.is_big_endian = 0;
	af.srate = samp_per_sec;
	af.channels = wave_channels;
	af.bits_per_sample = bits_per_sample;
	}



//alloc mem
if( bfsz == 0 )
	{
	if( verb ) logpf( "audio_formats::fp_open_float() - buf_size spec was zero: '%s'\n", fname.c_str() );
	return -7;
	}

f_bf0 = (float*) malloc( bfsz * sizeof(float) );
if( f_bf0 == 0 ) 
	{
	logpf( "audio_formats::fp_open_float() - unable to alloc f_bf0 of size: %d, filename: '%s'\n", bfsz*sizeof(double), fname.c_str() );
	return -7;
	}

f_bf1 = (float*) malloc( bfsz * sizeof(float) );
if( f_bf1 == 0 ) 
	{
	logpf( "audio_formats::fp_open_float() - unable to alloc f_bf1 of size: %d, filename: '%s'\n", bfsz*sizeof(double), fname.c_str() );
	fp_write_close_free_mem();
	return -7;
	}

ucbf = (unsigned char*) malloc( bfsz * 4 );		//assume 2 channel (16 bit)
if( ucbf == 0 ) 
	{
	logpf( "audio_formats::fp_open_float() - unable to alloc ucbf of size: %d, filename: '%s'\n", bfsz * 4, fname.c_str() );
	fp_write_close_free_mem();
    return -2;
	}


buf_size = bfsz;
file_size = fsz;
pathname = fname1;

req_scale_down_by_peak_int_val = scale_down_by_peak_int_val;


return 1;
}








//reads 'count' samples from prev opened audio file, data stored in: 'af.f_bf0[]', 'af.f_bf1[]', 
//audio sample levels are divided by prev defined: 'scale_down_by_peak_int_val' to get down to to -/+ 1.0 range
//DOES not normalise audio,

//returns number of samples read, else a neg val specifying error
int audio_formats::fp_read_float( unsigned int count, st_audio_formats_tag &af )
{
if( fileptr == 0 ) 
	{
	if( verb ) logpf( "audio_formats::fp_read_float() - file not open: '%s'\n", pathname.c_str() );
	return -1;
	}

if( af.channels == 0 )
	{
	if( verb ) logpf( "audio_formats::fp_read_float() - channels found is zero: '%s'\n", pathname.c_str() );
	return -2;
	}

if( count == 0 ) return 0;


if( count >= buf_size ) 
	{
	if( verb ) logpf( "audio_formats::fp_read_float() - count: %u, is larger than alloc buf: %u: '%s'\n", count, buf_size, pathname.c_str() );
	return -3;
	}

string s1;
double min = 0;                                     //for normalising audio
double max = 0;
float ff;

bool trk2 = 0;
char cc;
int iaud;

int read;

bool is_ulaw = 0;
if( ( af.format == en_af_sun) & ( af.encoding == 1 ) ) is_ulaw = 1;

int need = count;

need *= af.channels;

if( need > buf_size ) need = buf_size;

int sample_byte_size = 2;
if( is_ulaw ) sample_byte_size = 1;


read = fread( ucbf, 1, need * sample_byte_size, fileptr );          	//read block of audio data, 2 bytes per sample


if( read != 0 )
	{
	int idx = 0;
	for( int i = 0; i < read; )                 //process block
		{
		if( !is_ulaw) i++;

		if( i >= read )
			{
			if( verb ) logpf( "audio_formats::fp_read_float() - odd number of bytes encountered, lost final sample\n" );
			break;
			}

		if( !af.is_big_endian )
			{

			cc = ucbf[ i ];						    //preserve sign in msbyte (little endian)

			iaud = cc * 256;						//shift msbyte to its location preserving sign
			iaud |= ucbf[ i - 1 ];					//or in lsbyte
			}
		else{
			if( is_ulaw )
				{    
				cc = ucbf[ i ];					   	 	//preserve sign in msbyte (little endian)
				iaud = ulaw2linear( cc );
				}
			else{
				iaud = (char)ucbf[ i - 1 ] * 256.0;      //preserve sign (big endian)
				iaud += ucbf[ i ];
				}
			}
			


		ff = (float)iaud / req_scale_down_by_peak_int_val;

		i++;

		if( af.channels == 1 )                //1 trk?
			{
			f_bf0[ idx++ ] = ff;
//if( idx < 5 ) printf("rd0: %f, iaud: %d\n", dd, iaud );
			}
		else{
			if( trk2 == 0 )
				{
				f_bf0[ idx ] = ff;
//if( idx < 5 ) printf("rd0: %f, iaud: %d\n", dd, iaud );
				}
			else{
				f_bf1[ idx++ ] = ff;
//if( idx < 6 ) printf("rd1: %f, iaud: %d\n", dd, iaud );
				}
			trk2 = !trk2;
			}

	//		if( dd < min ) { min = dd; min_sample_idx = i; }
	//		if( dd > max ) { max = dd; max_sample_idx = i; }
		}
	//logpf("read: %d\n", read );
	}

return (read / sample_byte_size / af.channels);
}




































//create an audio file for later saving, float precision mem block based, 'fwrite()' calls occur in: 'af.fp_write_float()'
// !!! make SURE this obj is destroyed to allow writing of the audio file header params, file closure and mem freeing,
//only supports: 'en_af_sun', 'en_af_wav_pcm', 'en_af_aiff', i.e: NO raw support
//NEEDS these set:  'saf.format',  'saf.srate',  'saf.channels',  'saf.encode'(only for sun: 1=ulaw, 3=16bit)
//creates internal malloc sample buffers of 'bfsz',  these are accessed by: af.f_bf0[], af.f_bf1[] 
//samples should be between +/- 1.0, saved samples are scaled up by 'peak_int_val'
//DOES not normalise audio,

//returns 1 on success
//returns -1 file not openable for writing
//returns -2 channel count specified is not supported
//returns -3 memory not allocated
//returns -4 write to file failed
int audio_formats::fp_create_float( string path, string fname, unsigned int bfsz, int peak_int_val, st_audio_formats_tag &af )
{
mystr m1;
string fname1 = path;
fname1 += fname;

fp_write_close_free_mem();


log = "";

af.path = path;
af.fname = fname1;
af.offset = 0;
af.is_big_endian = 0;
af.bits_per_sample = 16;


if( ( af.channels == 0 ) | ( af.channels > 2 ) )
	{
	if( verb ) logpf( "audio_formats::fp_create_float() - unsupported channel count specified: %d, filename: '%s'\n", af.channels, fname.c_str() );
	fp_write_close_free_mem();
	return -1;
	}


//alloc mem
if( bfsz == 0 )
	{
	if( verb ) logpf( "audio_formats::fp_create_float() - buf_size spec was zero: '%s'\n", fname.c_str() );
	fp_write_close_free_mem();
	return -7;
	}

f_bf0 = (float*) malloc( bfsz * sizeof(float) );
if( f_bf0 == 0 ) 
	{
	logpf( "audio_formats::fp_create_float() - unable to alloc bf0 of size: %d, filename: '%s'\n", bfsz*sizeof(double), fname.c_str() );
	fp_write_close_free_mem();
	return -7;
	}

f_bf1 = (float*) malloc( bfsz * sizeof(float) );
if( f_bf1 == 0 ) 
	{
	logpf( "audio_formats::fp_create_float() - unable to alloc bf1 of size: %d, filename: '%s'\n", bfsz*sizeof(double), fname.c_str() );
	fp_write_close_free_mem();
	return -7;
	}

ucbf = (unsigned char*) malloc( bfsz * 4 );		//assume 2 channel (16 bit)
if( ucbf == 0 ) 
	{
	logpf( "audio_formats::fp_create_float() - unable to alloc ucbf of size: %d, filename: '%s'\n", bfsz * 4, fname.c_str() );
	fp_write_close_free_mem();
    return -2;
	}




fileptr = m1.mbc_fopen( fname1, "wb" );

if( fileptr == 0 ) 
	{
	if( verb ) logpf( "audio_formats::fp_create_float() - couldn't open a file pointer to filename: '%s'\n", fname.c_str() );
	fp_write_close_free_mem();
	return -1;
	}



//make a hole for header, it will be filled in when file is closed
unsigned char buf[256];

if( af.format == en_af_sun )
	{
	af.is_big_endian = 1;
	if( af.encoding != 1 ) af.encoding = 3;							//default to 16 bit when not ulaw
	
	af.offset = 28;
	int wrote;
	wrote = fwrite( buf, 1, 7 * 4, fileptr );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
	if( wrote != 7 * 4 )
		{
		if( verb ) logpf( "audio_formats::fp_create_float() - failed to write 28 bytes of sun header for file: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}
	}

if( af.format == en_af_aiff )
	{
	af.encoding = 0;
	af.is_big_endian = 1;

	int wrote;
	wrote = fwrite( buf, 1, 54, fileptr );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
	if( wrote != 54 )
		{
		if( verb ) logpf( "audio_formats::fp_create_float() - failed to write 54 bytes of aiff header for file: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}
	}


if( af.format == en_af_wav_pcm )
	{
	af.encoding = 0;

	int wrote;
	wrote = fwrite( buf, 1, 44, fileptr );						//write header, 'fmt chunk' and start of 'data chuck', up to where audio data starts
	if( wrote != 44 )
		{
		if( verb ) logpf( "audio_formats::fp_create_float() - failed to write 44 bytes of wav header for file: '%s'\n", fname1.c_str() );
		fp_write_close_free_mem();
		return -4;
		}
	}



buf_size = bfsz;
pathname = fname1;

req_peak_int_val = peak_int_val;

fp_audio_format = af.format;
fp_srate = af.srate;
fp_channels = af.channels;
fp_offset = af.offset;
fp_encoding = af.encoding;
fp_is_big_endian = af.is_big_endian;
return 1;
}









//writes 'count' samples from prev created audio file, data read from: 'af.f_bf0[]', 'af.f_bf1[]',
//samples should be between +/- 1.0, saved samples are scaled up by prev defined: 'peak_int_val'
//DOES not normalise audio,


//returns number of samples writtn, else a neg val spec error
//returns -1 file not open
//returns -2 channel count specified is not supported
//returns -3 count exceeds buf size
//returns -4 write to file failed
int audio_formats::fp_write_float( unsigned int count, st_audio_formats_tag &af )
{
if( fileptr == 0 ) 
	{
	if( verb ) logpf( "audio_formats::fp_write_float() - file not open: '%s'\n", pathname.c_str() );
	return -1;
	}

if( af.channels == 0 )
	{
	if( verb ) logpf( "audio_formats::fp_write_float() - channels found is zero: '%s'\n", pathname.c_str() );
	return -2;
	}

if( count == 0 ) return 0;


if( count >= buf_size ) 
	{
	if( verb ) logpf( "audio_formats::fp_write_float() - count: %u, is larger than alloc buf: %u: '%s'\n", count, buf_size, pathname.c_str() );
	return -3;
	}

int iaud;

//bool is_ulaw = 0;
//if( ( af.format == en_af_sun) & ( af.encoding == 1 ) ) is_ulaw = 1;

int bytes_per_sample = 2;

bool trk2 = 0;

if( af.channels == 2 ) trk2 = 1;

unsigned int j = 0;
unsigned int wrote = 0;

float ff;
unsigned char uc;


//8 bit
if( ( af.format == en_af_sun ) & ( af.encoding == 1 ) )							//ulaw
	{
	bytes_per_sample = 1;
	for( int i = 0; i < count; i++ )
		{
		ff = f_bf0[ i ]; 
        uc = linear2ulaw( nearbyint( ff * (float)req_peak_int_val ) );			//v1.04
		
		ucbf[ j++ ] = uc;
		
//		if( i < 5 ) printf("ulaw wr0: %f, cc: %d\n", dd, (unsigned int)uc );

		if( trk2 ) 
			{
			ff = f_bf1[ i ];
			uc = linear2ulaw( nearbyint( ff * (float)req_peak_int_val ) );		//v1.04
			ucbf[ j++ ] = uc;
//			if( i < 5 ) printf("ulaw wr0: %f, cc: %d\n", dd, (unsigned int)uc );
			}
		}
	}
else{
//16 bit
	if( !af.is_big_endian )
		{
		for( int i = 0; i < count; i++ )
			{
			ff = f_bf0[ i ];
			iaud = nearbyint( ff * (float)req_peak_int_val );
			
			
			ucbf[ j++ ] =  iaud & 0xff;
			ucbf[ j++ ] = ( iaud >> 8  ) & 0xff;               			//little endian

//			if( i < 5 ) printf("wr0: %f, iaud: %d\n", dd, iaud );

			if( trk2 ) 
				{
				ff = f_bf1[ i ];
				iaud = nearbyint( ff * (float)req_peak_int_val );
				ucbf[ j++ ] =  iaud & 0xff;
				ucbf[ j++ ] = ( iaud >> 8  ) & 0xff;               		//little endian
//				if( i < 5 ) printf("wr1: %f, iaud: %d\n", dd, iaud );
				}
			}

		}
	else{
//16 bit
		for( int i = 0; i < count; i++ )
			{
			ff = f_bf0[ i ];
			iaud = nearbyint( ff * (float)req_peak_int_val );
			ucbf[ j++ ] = ( iaud >> 8  ) & 0xff;               			//big endian
			ucbf[ j++ ] =  iaud & 0xff;

//			if( i < 5 ) printf("wr0: %f, iaud: %d\n", dd, iaud );

			if( trk2 ) 
				{
				ff = f_bf1[ i ];
				iaud = nearbyint( ff * (float)req_peak_int_val );
				ucbf[ j++ ] = ( iaud >> 8  ) & 0xff;               		//big endian
				ucbf[ j++ ] =  iaud & 0xff;

//				if( i < 5 ) printf("wr1: %f, iaud: %d\n", dd, iaud );
				}
			}
		}
	}

wrote = fwrite( ucbf, 1, j, fileptr );          //write block of audio data
if( wrote != j )
	{
	if( verb ) logpf( "audio_formats::fp_write_float() - failed to complete a write to file: '%s', total bytes written: %u\n", pathname.c_str(), wrote );
	return -4;
	}

fp_bytes_written += wrote;

return (wrote / bytes_per_sample / af.channels);
}
//----------------------------------------------------------------------
















/*
//EXAMPLE CODE: test code using fp based methods, copies audio file using 'af.fp_xxxx' methods, see also fast_mgraph
fast_mgraph fgph_af_fp;

void test_audio_format_fp()
{
vector<float> fgph_vx;
vector<float> fgph_vy0;
vector<float> fgph_vy1;

audio_formats af_1;
st_audio_formats_tag saf_1;

audio_formats af_2;
st_audio_formats_tag saf_2;

af_1.verb = 1;
af_1.log_to_printf = 1;
//saf_1.encoding = 3;
//saf_1.offset = 0;
//saf_1.format = en_af_sun;
//saf_1.channels = 0;
//saf_1.srate = 0;

//string fname = "/home/gc/Desktop/mp3/Film Soundtracks/BasilPoledouris/BasilPoledouris/ConanTrimmed48K.wav";
string fname = "/home/gc/Desktop/mp3/Film Soundtracks/BasilPoledouris/BasilPoledouris/ConanTrimmed48K.aiff";
//string fname = "/home/gc/Desktop/mp3/Film Soundtracks/BasilPoledouris/BasilPoledouris/ConanTrimmed48K.au";
//string fname = "./testing_stereo_48000_ulaw.au";
//string fname = "./zzzzjunk_1ch_ulaw.au";
string fname2 = "./zzzzjunk.au";

int read;
int wrote;
int read_total = 0;
int write_total = 0;

if( af_1.fp_open_float( "", fname, 65536, 32767, saf_1 ) <= 0 )					//open src file, use float version rather than double version: fp_open_double()
	{
	printf("failed to open: '%s'\n", fname.c_str() );
	goto failed_done;
	}

printf("reading: '%s'\n", af_1.pathname.c_str() );

printf("saf_1.format: %d\n", saf_1.format);
printf("saf_1.channels: %d\n", saf_1.channels);
printf("saf_1.srate: %d\n", saf_1.srate);
printf("af_1.encoding: %d\n", saf_1.encoding);
printf("af_1.file_size: %d\n", (int)af_1.file_size);



af_2.verb = 1;																	//adj req dest format
af_2.log_to_printf = 1;
saf_2.encoding = 3;
saf_2.offset = 0;
saf_2.format = en_af_sun;
saf_2.channels = saf_1.channels;
saf_2.srate = saf_1.srate;


if( af_2.fp_create_float( "", fname2.c_str(), 65536, 32767, saf_2 ) <= 0 )		//create dest file, use float version rather than double version: fp_create_double()
	{
	printf("failed to create: '%s'\n", fname2.c_str() );
	goto failed_done;
	}



//copy loop
while(1)
{
read = af_1.fp_read_float( 50000, saf_1 );					//use float version rather than double version: fp_read_double()
if( read == 0 ) break;

if( read < 0 )
	{
	printf("failed to read: '%s'\n", fname.c_str() );
	goto failed_done;
	}
	

//printf("read: %d\n", read);
for( int i = 0; i < read; i++ )
	{
	if( ( read_total < 1 ) & ( i < 5000 ) )					//only graph small amount
		{
		fgph_vx.push_back( i );
		fgph_vy0.push_back( af_1.f_bf0[i] );
		fgph_vy1.push_back( af_1.f_bf1[i] );
		}
	
	af_2.f_bf0[i] = af_1.f_bf0[i];							//copy samples for write
	af_2.f_bf1[i] = af_1.f_bf1[i];
	}

read_total += read;


wrote = af_2.fp_write_float( read, saf_2 );					//use float version rather than double version: fp_write_double()
write_total += wrote;

if( wrote <= 0 )
	{
	printf("failed to write: '%s'\n", fname2.c_str() );
	goto failed_done;
	}
printf("read total: %d, write total: %d, wrote: %d, \n", read_total, write_total, wrote );

fgph_af_fp.plotxy( fgph_vx, fgph_vy0, fgph_vy1, "brwn", "drkg", "ofw", "drkb", "drkgry", "f_bf0[i]", "f_bf1[i]" );
}

failed_done:


printf("read total: %d, write total: %d\n", read_total, write_total );
printf("wrote to: '%s'\n", af_2.pathname.c_str() );

return;
}
*/
