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


//pcaudio.cpp
//v1.01	 	25-may-2015			//	



#include "pcaudio.h"

extern bool my_verbose;

extern int audio_hardware_bits_max_integer;
extern int audio_hardware_bits_min_integer;

extern int lev_clip_flag;
extern int lev_clip_ch;
extern int lev_clip_polarity;
extern double lev_clip_val;

//linux code
#ifndef compile_for_windows
int audio_card_fd = -1;							//sound device file descriptor
unsigned char *ac_bufout = 0;					//data to line/spkr o/p (d/a)
unsigned char *ac_bufin = 0;					//data from mic/line i/p (a/d)
unsigned int ac_uc_bufsize = 0;
gcthrd *ac_thrd1 = 0;
gcthrd *ac_thrd2 = 0;
void audio_card_thrd_start();
void audio_card_thrd_stop();
//bool ac_need_more_data;							//set if we need more audio data
unsigned int max_fragments;						//holds max number of fragments (from empty linux sound driver) before write() calls will block
#endif


//windows code
#ifdef compile_for_windows
HWAVEOUT hwo = 0;
HWAVEIN hwi = 0;
bool bIssueMoreBuffers;
bool bStillBuffersToPlay;
bool bStillBuffersToRec;
bool issue_more_buf_rec;




DWORD dwBufferSize;

WAVEHDR wh1;					//one for each buffer
WAVEHDR wh2;
WAVEHDR wh3;
WAVEHDR wh4;
WAVEHDR wh33;

//char *RecordingWfex = 0;			//a WAVEFORMATEX dynamically allocated buffer pointer
//DWORD dwBiggestWfex;//largest possible size of WAVEFORMATEX,depends on compression chosen

bool bStarted=0;
bool started_wave_in = 0;


//function prototypes
void CALLBACK MCICallbackWaveOut( HWAVEOUT hw, UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2 );
void CALLBACK MCICallbackWaveIn( HWAVEIN hw, UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2 );

unsigned int ac_awaiting_in_buffer = 0;					//shows which buffer needs to be loaded in  audio_card_prepare_in()
unsigned int ac_awaiting_out_buffer = 0;				//shows which buffer needs to be loaded in  audio_card_prepare_out()

#endif




int ac_whtnz_iFilter = 0;
bool ac_whtnz_bFilterOn =0;

double ac_two_pi = M_PI * 2.0;

unsigned int ac_srate;
unsigned int ac_bits;
unsigned int ac_double_bufsize;
unsigned int ac_data_count_to_write = 0;		        //size of data to be written
double ac_sample_time;
double ac_tone_freq1;						            //use for test signal generation
double ac_tone_theta1;
double ac_tsg_gain1;
double ac_tone_inc1;
double ac_VU0 = def_VU0;				                //an audio level of 1.0 gives this value to sound card d/a
double ac_gain_out_ch1;				                    // 1.0 is unity
double ac_gain_out_ch2;				                    // 1.0 is unity


unsigned int ac_opened_in_channels = 0;
unsigned int ac_opened_out_channels = 0;

char *cBuff1 = 0;//[ cnBufsize ];
char *cBuff2 = 0;//[ cnBufsize ];
char *cBuff3 = 0;//[ cnBufsize ];
char *cBuff4 = 0;//[ cnBufsize ];
char *cBuff33 = 0;//[ cnBufsize ];


flts *audiocard_outbuf0 = 0;
flts *audiocard_outbuf1 = 0;
flts *audiocard_inbuf0 = 0;
flts *audiocard_inbuf1 = 0;


//these vars are to capture missing outbuf loads, if set, it means a buf failed to be loaded before it was needed to be sent to audio card d/a 
bool ac_missed_buf1 = 0;		//if set this indicates at least one ch1 buf failed to be loaded before it was needed to be sent to audio card d/a 
unsigned int ac_missed_buf_cnt1 = 0;					//this holds a count of how many missed buf events have occurred for ch1
bool ac_loaded_buf1 = 0;								//this is set by audio_card_prepare_out(..) for ch1, and reset by  


bool ac_missed_buf2 = 0;		//if set this indicates at least one ch2 buf failed to be loaded before it was needed to be sent to audio card d/a 
unsigned int ac_missed_buf_cnt2 = 0;					//this holds a count of how many missed buf events have occurred for ch1
bool ac_loaded_buf2 = 0;								//this is set by audio_card_prepare_out(..) for ch1, and reset by  


//for test signal injection - see audiocard_inject_test_sig(..)
int ac_test_sig_type = 0;//en_ac_test_sig_tone;// //en_ac_test_sig_whitenoise;
unsigned int  ac_test_sig_freq = 400;
unsigned int  ac_test_sig_level;


//int (*cb_user_process)( jack_nframes_t nframes, void *arg );			//windows callback to user for sample buffer loading
int (*cb_user_process_out)( unsigned int chn, unsigned int frames, void *arg );			//windows callback to user for sample buffer loading
int (*cb_user_process_in)( unsigned int chn, unsigned int frames, void *arg );			//windows callback to user for sample buffer loading
void *user_args;














//sets the o/p gain, 1.0 is unity
bool audio_card_set_out_gain( double ch1, double ch2 )
{
ac_gain_out_ch1 = ch1;
ac_gain_out_ch2 = ch2;
}




//sample_buf_size should be set to number of samples to process between callbacks e.g: like 4096
//bool audio_card_init( unsigned int srate, unsigned int channels, unsigned int bits, unsigned int sample_buf_size, int (*cb_user_process_in)( jack_nframes_t nframes, void *arg ), void* user_args_in )
bool audio_card_open_out( unsigned int srate, unsigned int channels, unsigned int bits, unsigned int sample_buf_size, int (*cb_user_process_out_i)( unsigned int chn, unsigned int frames, void *arg ), void* user_args_in )
{
ac_gain_out_ch1 = 1.0;
ac_gain_out_ch2 = 1.0;


if( cb_user_process_out_i == 0 )
	{
	if ( my_verbose ) printf( "audio_card_open_out() - no callback specified\n" );
	return 0;
	}
cb_user_process_out = cb_user_process_out_i;			//set callback function
user_args = user_args_in;


ac_two_pi = M_PI * 2.0;
ac_srate = srate;
ac_opened_out_channels = channels;
ac_bits = bits;
ac_sample_time = 1.0 / (double)ac_srate;
ac_VU0 = def_VU0;


//linux code
#ifndef compile_for_windows
int arg;						// argument for ioctl calls

if( audio_card_fd != -1 )
	{
	if ( my_verbose ) printf( "audio_card_open_out() - already opened\n" );
	return 0;
	}


string s1 = "/dev/dsp";

audio_card_fd = open( s1.c_str() , O_RDWR );
if( audio_card_fd < 0 )
	{
	if ( my_verbose ) printf("audio_card_open_out() - failed to open device: %s\n", s1.c_str() );
	return 0;
	}



arg = ac_bits;
int status;

status = ioctl( audio_card_fd, SOUND_PCM_WRITE_BITS, &arg );
if ( status == -1 )
	{
	if ( my_verbose ) printf( "audio_card_open_out() - SOUND_PCM_WRITE_BITS ioctl failed: %d\n", arg );
	return 0;
	}
if ( arg != ac_bits )
	{
	if ( my_verbose ) printf("audio_card_open_out() - Unable to set sample size: %d\n", arg );
	return 0;
	}


arg = ac_opened_out_channels;
status = ioctl( audio_card_fd, SOUND_PCM_WRITE_CHANNELS, &arg );
if ( status == -1 )
	{
	if ( my_verbose ) printf( "audio_card_open_out() - SOUND_PCM_WRITE_CHANNELS ioctl failed: %d\n", arg );
	return 0;
	}

if ( arg != ac_opened_out_channels )
	{
	if ( my_verbose ) printf("audio_card_open_out() - Unable to set channel count: %d\n", arg );
	return 0;
	}


arg = srate;
status = ioctl( audio_card_fd, SOUND_PCM_WRITE_RATE, &arg );
if ( status == -1 )
	{
	if ( my_verbose ) printf( "audio_card_open_out() - SOUND_PCM_WRITE_WRITE ioctl failed: %d\n", arg );
	return 0;
	}

if ( arg != srate )
	{
	if ( my_verbose ) printf("audio_card_open_out() - Unable to set samplerate: %d\n", arg );
	return 0;
	}


unsigned int fragmentsize;

audio_card_data_size_for_non_blocking_write( max_fragments, fragmentsize );
if ( my_verbose ) printf("audio_card_open_out() - fragment size: %d, max fragments: %d\n", fragmentsize, max_fragments ); 


ac_uc_bufsize = sample_buf_size * ac_opened_out_channels * ( ac_bits / 8 ); // e.g: 4096 x 2 channels * ( 16 bits / 8 )  

ac_double_bufsize = sample_buf_size;


//sound card output buffers (to line/spk o/p)
ac_bufout = new unsigned char[ ac_uc_bufsize ];
for ( int i = 0; i < ac_uc_bufsize; i++ )					//clear char buf
	{
	ac_bufout[ i ] = 0;
	}


audiocard_outbuf0 = new flts[ ac_double_bufsize ];
for ( int i = 0; i < ac_double_bufsize; i++ )				//clear flts buf
	{
	audiocard_outbuf0[ i ] = 0;
	}


if( ac_opened_out_channels >= 2 )
	{
	audiocard_outbuf1 = new flts[ ac_double_bufsize ];
	for ( int i = 0; i < ac_double_bufsize; i++ )			//clear flts buf
		{
		audiocard_outbuf1[ i ] = 0;
		}
	}



goto ok;

#endif



//windows code
#ifdef compile_for_windows

if( hwo )
	{
	if ( my_verbose ) printf( "audio_card_open_out() - already opened\n" );
	return 0;
	}


if( ac_opened_out_channels > 2 ) ac_opened_out_channels = 2;


MMRESULT mmres;
WAVEFORMATEX wfex;


//srate = 44100;
//channels = 2;
//bits = 16;

wfex.wFormatTag = WAVE_FORMAT_PCM;
wfex.nChannels = ac_opened_out_channels;
wfex.nSamplesPerSec = ac_srate;
wfex.wBitsPerSample = ac_bits;
wfex.nBlockAlign = wfex.nChannels * wfex.wBitsPerSample / 8;		//e.g: 2 * 16 / 8 = 4
wfex.nAvgBytesPerSec = wfex.nSamplesPerSec * wfex.nBlockAlign;		//e.g: 48000 * 4 = 192000
wfex.cbSize = 0;


dwBufferSize = sample_buf_size * ac_opened_out_channels * ( ac_bits / 8 ); // e.g: 4096 x 2 channels * ( 16 bits / 8 )  



ac_double_bufsize = sample_buf_size;

cBuff1 = new char[ dwBufferSize ];					//for audio o/p to line/spkr
cBuff2 = new char[ dwBufferSize ];
cBuff3 = new char[ dwBufferSize ];
cBuff33 = new char[ dwBufferSize ];

for ( int i = 0; i < dwBufferSize; i++ )					//clear char buffers
	{
	cBuff1[ i ] = 0;
	cBuff2[ i ] = 0;
	cBuff3[ i ] = 0;
	cBuff33[ i ] = 0;
	}

audiocard_outbuf0 = new flts[ ac_double_bufsize ];
for ( int i = 0; i < ac_double_bufsize; i++ )				//clear flts buf
	{
	audiocard_outbuf0[ i ] = 0;
	}


if( ac_opened_out_channels >= 2 )
	{
	audiocard_outbuf1 = new flts[ ac_double_bufsize ];
	for ( int i = 0; i < ac_double_bufsize; i++ )			//clear flts buf
		{
		audiocard_outbuf1[ i ] = 0;
		}
	}






//printf( "audio_card_open() - cBuff1 size %d\n", dwBufferSize );
//printf( "audio_card_open() - audiocard_outbuf0 size %d\n", ac_double_bufsize );

//Sleep( 5000 );


//open wave out device
if( !hwo )
	{
	mmres = waveOutOpen( &hwo, WAVE_MAPPER, &wfex,(DWORD)MCICallbackWaveOut, 0, CALLBACK_FUNCTION | WAVE_ALLOWSYNC );
	if( mmres != MMSYSERR_NOERROR )
		{
		printf( "audio_card_open_out() - waveOutOpen() error - mmres = %d\n", mmres );
		goto fail;
		}

	}

printf("hwo: %x\n", hwo );



audio_card_make_test_signal_if_req(cBuff1, dwBufferSize);					//will just fill buffer with zero if no test signal selected


wh1.lpData=(LPSTR) cBuff1;
wh1.dwBufferLength=dwBufferSize;
wh1.dwFlags=0;
wh1.dwLoops=0;
wh1.dwUser=0;


//advise windows of buffer1 details
mmres=waveOutPrepareHeader(hwo,&wh1,sizeof(WAVEHDR) );
if(mmres!=MMSYSERR_NOERROR)
	{

	waveOutClose(hwo);

	goto fail;
	}



audio_card_make_test_signal_if_req(cBuff2, dwBufferSize);					//will just fill buffer with zero if no test signal selected



wh2.lpData=(LPSTR) cBuff2;
wh2.dwBufferLength=dwBufferSize;
wh2.dwFlags=0;
wh2.dwLoops=0;
wh2.dwUser=0;


//advise windows of buffer2 details
mmres=waveOutPrepareHeader(hwo,&wh2,sizeof(WAVEHDR) );
if(mmres!=MMSYSERR_NOERROR)
	{
	waveOutClose(hwo);
	goto fail;
	}



audio_card_make_test_signal_if_req(cBuff33, dwBufferSize);					//will just fill buffer with zero if no test signal selected



wh33.lpData=(LPSTR) cBuff33;
wh33.dwBufferLength=dwBufferSize;
wh33.dwFlags=0;
wh33.dwLoops=0;
wh33.dwUser=0;


//advise windows of buffer3 details
mmres=waveOutPrepareHeader(hwo,&wh33,sizeof(WAVEHDR) );
if(mmres!=MMSYSERR_NOERROR)
	{
	waveOutClose(hwo);
	goto fail;
	}





/*

MakeWhiteNoise(cBuff3, dwBufferSize);					//will just fill buffer with zero if no test signal selected

wh3.lpData=(LPSTR) cBuff3;
wh3.dwBufferLength=dwBufferSize;
wh3.dwFlags=0;
wh3.dwLoops=0;
wh3.dwUser=0;


//advise windows of buffer3 details
mmres=waveOutPrepareHeader(hwo,&wh3,sizeof(WAVEHDR) );
if(mmres!=MMSYSERR_NOERROR)
	{
	waveOutClose(hwo);
	goto fail;
	}
*/



bIssueMoreBuffers=1;
bStillBuffersToPlay=1;


bStarted = 1;



//add buffer 1 (wav playing buffer)
mmres=waveOutWrite(hwo,&wh1,sizeof(WAVEHDR)); 
if(mmres!=MMSYSERR_NOERROR)
	{
	printf( "audio_card_open_out() - waveOutWrite( wh1 ) error - mmres = %d\n", mmres );
	goto fail;
	}






//add buffer 2 (next wav playing buffer)
mmres=waveOutWrite(hwo,&wh2,sizeof(WAVEHDR)); 
if(mmres!=MMSYSERR_NOERROR)
	{
	printf( "audio_card_open_out() - waveOutWrite( wh2 ) error - mmres = %d\n", mmres );
	goto fail;
	}


//add buffer 3 (next wav playing buffer)
mmres=waveOutWrite(hwo,&wh33,sizeof(WAVEHDR)); 
if(mmres!=MMSYSERR_NOERROR)
	{
	printf( "audio_card_open_out() - waveOutWrite( wh3 ) error - mmres = %d\n", mmres );
	goto fail;
	}







goto ok;



fail:

if ( my_verbose ) printf( "audio_card_open_out() - failed\n" );


bIssueMoreBuffers = 0;
if( hwo ) waveOutClose( hwo );

hwo = 0;

return 0;
#endif



ok:

ac_missed_buf1 = 0;
ac_missed_buf2 = 0;
ac_missed_buf_cnt1 = 0;
ac_missed_buf_cnt2 = 0;
ac_loaded_buf1 = 0;
ac_loaded_buf2 = 0;


if ( my_verbose )
	{
	printf( "audio_card_open_out() - opened ok\n" );
	printf( "audio_card_open_out() - srate: %d, channels: %d, bits: %d\n", ac_srate, ac_opened_out_channels, ac_bits );
	printf( "audio_card_open_out() - framesize: %d\n", sample_buf_size );
	}

//linux code
#ifndef compile_for_windows
audio_card_thrd_start();
#endif


return 1;

}














//sample_buf_size should be set to number of samples to process between callbacks e.g: like 4096
//bool audio_card_init( unsigned int srate, unsigned int channels, unsigned int bits, unsigned int sample_buf_size, int (*cb_user_process_in)( jack_nframes_t nframes, void *arg ), void* user_args_in )
bool audio_card_open_in( unsigned int srate, unsigned int channels, unsigned int bits, unsigned int sample_buf_size, int (*cb_user_process_in_i)( unsigned int chn, unsigned int frames, void *arg ), void* user_args_in )
{

if( cb_user_process_in_i == 0 )
	{
	if ( my_verbose ) printf( "audio_card_open_in() - no callback specified\n" );
	return 0;
	}

cb_user_process_in = cb_user_process_in_i;			//set callback function
user_args = user_args_in;


ac_two_pi = M_PI * 2.0;
ac_srate = srate;
ac_opened_in_channels = channels;
ac_bits = bits;
ac_sample_time = 1.0 / (double)ac_srate;
ac_VU0 = def_VU0;


//linux code
#ifndef compile_for_windows
int arg;						// argument for ioctl calls


if( audio_card_fd == -1 )
	{
	if ( my_verbose ) printf( "audio_card_open_in() - audio card not opened\n" );
	return 0;
	}



arg = srate;
int status = ioctl( audio_card_fd, SOUND_PCM_WRITE_RATE, &arg );
if ( status == -1 )
	{
	if ( my_verbose ) printf( "audio_card_open_in() - SOUND_PCM_WRITE_WRITE ioctl failed: %d\n", arg );
	return 0;
	}

if ( arg != srate )
	{
	if ( my_verbose ) printf("audio_card_open_in() - Unable to set samplerate: %d\n", arg );
	return 0;
	}


ac_uc_bufsize = sample_buf_size * ac_opened_out_channels * ( ac_bits / 8 ); // e.g: 4096 x 2 channels * ( 16 bits / 8 )  

ac_double_bufsize = sample_buf_size;




//sound card input buffers (from mic/line i/p)
ac_bufin = new unsigned char[ ac_uc_bufsize ];
for ( int i = 0; i < ac_uc_bufsize; i++ )					//clear char buf
	{
	ac_bufin[ i ] = 0;
	}



audiocard_inbuf0 = new flts[ ac_double_bufsize ];
for ( int i = 0; i < ac_double_bufsize; i++ )				//clear flts buf
	{
	audiocard_inbuf0[ i ] = 0;
	}


if( ac_opened_out_channels >= 2 )
	{
	audiocard_inbuf1 = new flts[ ac_double_bufsize ];
	for ( int i = 0; i < ac_double_bufsize; i++ )			//clear flts buf
		{
		audiocard_inbuf1[ i ] = 0;
		}
	}


goto ok;

#endif



//windows code
#ifdef compile_for_windows

if( hwi )
	{
	if ( my_verbose ) printf( "audio_card_open_in() - already opened\n" );
	return 0;
	}


if( ac_opened_out_channels > 2 ) ac_opened_out_channels = 2;


MMRESULT mmres;

//srate = 44100;
//channels = 2;
//bits = 16;

WAVEFORMATEX wfex_in;

wfex_in.wFormatTag = WAVE_FORMAT_PCM;
wfex_in.nChannels = ac_opened_in_channels;
wfex_in.nSamplesPerSec = ac_srate;
wfex_in.wBitsPerSample = ac_bits;
wfex_in.nBlockAlign = wfex_in.nChannels * wfex_in.wBitsPerSample / 8;			//e.g: 2 * 16 / 8 = 4
wfex_in.nAvgBytesPerSec = wfex_in.nSamplesPerSec * wfex_in.nBlockAlign;		//e.g: 48000 * 4 = 192000
wfex_in.cbSize = 0;


dwBufferSize = sample_buf_size * ac_opened_in_channels * ( ac_bits / 8 ); // e.g: 4096 x 2 channels * ( 16 bits / 8 )  



ac_double_bufsize = sample_buf_size;

cBuff3 = new char[ dwBufferSize ];					//for audio i/p from mic/line i/p
cBuff4 = new char[ dwBufferSize ];

for ( int i = 0; i < dwBufferSize; i++ )					//clear char buffers
	{
	cBuff3[ i ] = 0;
	cBuff4[ i ] = 0;
	}




audiocard_inbuf0 = new flts[ ac_double_bufsize ];
for ( int i = 0; i < ac_double_bufsize; i++ )				//clear flts buf
	{
	audiocard_inbuf0[ i ] = 0;
	}


if( ac_opened_in_channels >= 2 )
	{
	audiocard_inbuf1 = new flts[ ac_double_bufsize ];
	for ( int i = 0; i < ac_double_bufsize; i++ )			//clear flts buf
		{
		audiocard_inbuf1[ i ] = 0;
		}
	}





//printf( "audio_card_open() - cBuff1 size %d\n", dwBufferSize );
//printf( "audio_card_open() - audiocard_outbuf0 size %d\n", ac_double_bufsize );

//Sleep( 5000 );



//open wave input device

//find out how big the WAVEFORMATEX can be
//acmMetrics( 0, ACM_METRIC_MAX_SIZE_FORMAT, &dwBiggestWfex ); 
//allocate space for it
//RecordingWfex = new char[ dwBiggestWfex ];
//WAVEFORMATEX *wfex_in = (WAVEFORMATEX *) RecordingWfex;


issue_more_buf_rec = 1;


mmres = waveInOpen( &hwi, WAVE_MAPPER, &wfex_in, (DWORD)MCICallbackWaveIn, 0, CALLBACK_FUNCTION );  
if( mmres != MMSYSERR_NOERROR )
	{
	printf( "audio_card_open() - waveInOpen() error - mmres = %d\n", mmres );
	goto fail;
	}



wh3.lpData=(LPSTR) cBuff3;
wh3.dwBufferLength=dwBufferSize;
wh3.dwFlags=0;
wh3.dwLoops=0;
wh3.dwBytesRecorded=0;


wh4.lpData=(LPSTR) cBuff4;
wh4.dwBufferLength=dwBufferSize;
wh4.dwFlags=0;
wh4.dwLoops=0;
wh4.dwBytesRecorded=0;







//prepare buffer1
mmres=waveInPrepareHeader(hwi,&wh3,sizeof(WAVEHDR) );
if(mmres!=MMSYSERR_NOERROR)
	{
//	if(hwi) waveInClose(hwi);
//	hwi = 0;

	printf( "audio_card_open_in() - waveInPrepareHeader( wh3 ) error - mmres = %d\n", mmres );
	goto fail;
	goto fail;
	}


//prepare buffer2
mmres=waveInPrepareHeader( hwi, &wh4,sizeof(WAVEHDR) );
if(mmres!=MMSYSERR_NOERROR)
	{
	waveInUnprepareHeader( hwi, &wh3, sizeof(WAVEHDR) );
//	waveInClose(hwi);
//	hwi = 0;

	printf( "audio_card_open_in() - waveInPrepareHeader( wh4 ) error - mmres = %d\n", mmres );
	goto fail;
	}






//add where buffer is
mmres = waveInAddBuffer( hwi, &wh3, sizeof(WAVEHDR) ); 
if( mmres != MMSYSERR_NOERROR )
	{
	
	printf( "audio_card_open_in() - waveInAddBuffer( wh3 ) error - mmres = %d\n", mmres );
	goto fail;

//	waveInUnprepareHeader(hwi,&wh,sizeof(WAVEHDR) );
//	waveInUnprepareHeader(hwi,&wh2,sizeof(WAVEHDR) );
//	waveInUnprepareHeader(hwi,&wh3,sizeof(WAVEHDR) );

	}




//add where buffer is
mmres = waveInAddBuffer( hwi, &wh4, sizeof(WAVEHDR) ); 
if( mmres != MMSYSERR_NOERROR )
	{
	
	printf( "audio_card_open_in() - waveInAddBuffer( wh4 ) error - mmres = %d\n", mmres );
	goto fail;

//	waveInUnprepareHeader(hwi,&wh,sizeof(WAVEHDR) );
//	waveInUnprepareHeader(hwi,&wh2,sizeof(WAVEHDR) );
//	waveInUnprepareHeader(hwi,&wh3,sizeof(WAVEHDR) );

	}




started_wave_in = 1;



//start recording
mmres = waveInStart( hwi );
if( mmres != MMSYSERR_NOERROR )
	{
	printf( "audio_card_open_in() - error, mmres = %d\n", mmres );
	goto fail;
	}




goto ok;



fail:

if ( my_verbose ) printf( "audio_card_open_in() - failed\n" );

//if( RecordingWfex ) delete RecordingWfex;

issue_more_buf_rec = 0;
if( hwi ) waveInClose( hwi );

hwi = 0;

return 0;
#endif



ok:


if ( my_verbose )
	{
	printf( "audio_card_open_in() - opened ok\n" );
	printf( "audio_card_open_in() - srate: %d, channels: %d, bits: %d\n", ac_srate, ac_opened_out_channels, ac_bits );
	printf( "audio_card_open_in() - framesize: %d\n", sample_buf_size );
	}

//linux code
//#ifndef compile_for_windows
//audio_card_thrd_start();
//#endif


return 1;

}




















bool audio_card_close( )
{
mystr m1;

//linux code
#ifndef compile_for_windows

audio_card_thrd_stop();

if ( audio_card_fd != 0 )
	{
	close( audio_card_fd );
	audio_card_fd = -1;
	}
else{
	if ( my_verbose ) printf( "audio_card_close() - not opened\n" );
	}

if ( ac_bufout != 0 )
	{
	delete ac_bufout;
	ac_bufout = 0;
	}
#endif



//windows code
#ifdef compile_for_windows



bIssueMoreBuffers = 0;
issue_more_buf_rec = 0;


//Sleep( 1000 );					//this is to ensure all playing buffers a completed

m1.time_start( m1.ns_tim_start );

while ( 1 )							//wait till audio has stopped playing
	{
	if( ( !bStarted ) && ( !started_wave_in ) ) break;
	double d = m1.time_passed( m1.ns_tim_start );
	if( d > 2 )
		{
		if ( my_verbose ) printf( "audio_card_close() - timed out\n" );
		break;
		}
	}

if( hwi ) waveInStop( hwi );
if( hwi ) waveInReset( hwi );


if( hwo == 0 )
	{
	if ( my_verbose ) printf( "audio_card_close() - hwo = 0, failed\n" );
	}
else{
	waveOutClose( hwo );
	}

if( hwi == 0 )
	{
	if ( my_verbose ) printf( "audio_card_close() - hwi = 0, failed\n" );
	}
else{
	waveInClose( hwi );
	}

hwo = 0;
hwi = 0;
bStarted = 0;
started_wave_in = 0;

if ( cBuff1 ) delete cBuff1;
if ( cBuff2 ) delete cBuff2;
if ( cBuff3 ) delete cBuff3;
if ( cBuff4 ) delete cBuff4;
if ( cBuff33 ) delete cBuff33;

cBuff1 = 0;
cBuff2 = 0;
cBuff3 = 0;
cBuff4 = 0;
cBuff33 = 0;

#endif

ac_opened_in_channels = 0;
ac_opened_out_channels = 0;


cb_user_process_out = 0;				//clear callback function ptr
cb_user_process_in = 0;					//clear callback function ptr

if ( audiocard_outbuf0 ) delete audiocard_outbuf0;
if ( audiocard_outbuf1 ) delete audiocard_outbuf1;
if ( audiocard_inbuf0 ) delete audiocard_inbuf0;
if ( audiocard_inbuf1 ) delete audiocard_inbuf1;
audiocard_outbuf0 = 0;
audiocard_outbuf1 = 0;
audiocard_inbuf0 = 0;
audiocard_inbuf1 = 0;


return 1;
}









void audio_card_set_vu0( double vu0 )
{
ac_VU0 = vu0;
ac_test_sig_level = ac_VU0;

if ( my_verbose ) printf( "audio_card_set_vu0() - set to: %.2lf\n", ac_VU0 );
}








/*
//pass on stats
void audio_card_get_stats( st_audio_card_stats_tag &o )
{
o.ac_loaded_buf1 = ac_loaded_buf1;
o.ac_missed_buf1 = ac_missed_buf1;
o.ac_missed_buf_cnt1 = ac_missed_buf_cnt1;

o.ac_loaded_buf2 = ac_loaded_buf2;
o.ac_missed_buf2 = ac_missed_buf2;
o.ac_missed_buf_cnt2 = ac_missed_buf_cnt2;

}



//clear stats on spec channel
void audio_card_clear_stats( int ch )
{

if( ch == 0 )
	{
	ac_missed_buf1 = 0;
	ac_missed_buf_cnt1 = 0;
	}

if( ch == 1 )
	{
	ac_missed_buf2 = 0;
	ac_missed_buf_cnt2 = 0;
	}
}
*/















bool audio_card_get_srate( unsigned int &sr )
{

//linux code
#ifndef compile_for_windows
if( audio_card_fd == -1 )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - card not opened\n" );
	return 0;
	}
#endif


//windows code
#ifdef compile_for_windows
if( !hwo )
	{
	if ( my_verbose ) printf( "audio_card_get_srate() - card not opened\n" );
	return 0;
	}

#endif

sr = ac_srate;
return 1;
}









//get a pointer to spec o/p buffer (data to line/spkr o/p)
flts* audio_card_get_outbuf( int ch )
{



if( ch >= ac_opened_out_channels )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - channel %d not opened\n", ch );
	return 0;
	}



//windows code
#ifdef compile_for_windows

if( hwo == 0 )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - audio card not opened\n" );
	return 0;
	}
#endif



//linux code
#ifndef compile_for_windows

if( audio_card_fd == -1 )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - audio card not opened\n" );
	return 0;
	}
#endif



if ( ch == 0 ) return audiocard_outbuf0;
if ( ch == 1 ) return audiocard_outbuf1;

//if ( channel == 0 ) return audiocard_outbuf0;
//if ( channel == 1 ) return audiocard_outbuf1;

return 0;
}















//get a pointer to spec i/p buffer (data from mic/line i/p)
flts* audio_card_get_inbuf( int ch )
{

if( ch >= ac_opened_in_channels )
	{
	if ( my_verbose ) printf( "audio_card_get_inbuf() - channel %d not opened\n", ch );
	return 0;
	}



//windows code
#ifdef compile_for_windows

if( hwi == 0 )
	{
//	if ( my_verbose ) printf( "audio_card_get_inbuf() - audio card not opened\n" );
	return 0;
	}
#endif



//linux code
#ifndef compile_for_windows

if( audio_card_fd == -1 )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - audio card not opened\n" );
	return 0;
	}
#endif



if ( ch == 0 ) return audiocard_inbuf0;
if ( ch == 1 ) return audiocard_inbuf1;


return 0;
}


















//process a preloaded buffer and converts samples
//to integers for audio card sound driver and its d/a
bool audio_card_prepare_out( int ch )
{
flts *ptr = 0;
unsigned char *cptr = 0;

//return 0;

if( ch >= ac_opened_out_channels )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - specified channel is out of range\n" );
	return 0;
	}


ptr = 0;
if( ch == 0 ) ptr = audiocard_outbuf0;
if( ch == 1 ) ptr = audiocard_outbuf1;

if( ptr == 0 )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - ptr was zero\n" );
	return 0;
	}


//linux code
#ifndef compile_for_windows

//if( !ac_need_more_data )
//	{
//	return 1;
//	}

if( audio_card_fd == -1 )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - card not opened\n" );
	return 0;
	}

//ptr = audiocard_outbuf0;
cptr = (unsigned char* ) ac_bufout;

#endif




//windows code
#ifdef compile_for_windows

if( hwo == 0 )
	{
	if ( my_verbose ) printf( "audio_card_get_chan_outbuf() - card not opened\n" );
	return 0;
	}



if ( ac_awaiting_out_buffer == 0 )			//which buffer is required, specified in MCICallbackWaveOut
	{
	cptr = (unsigned char* ) cBuff1;
	}

if ( ac_awaiting_out_buffer == 1 )			//which buffer is required, specified in MCICallbackWaveOut
	{
	cptr = (unsigned char* ) cBuff2;
	}

if ( ac_awaiting_out_buffer == 2 )			//which buffer is required, specified in MCICallbackWaveOut
	{
	cptr = (unsigned char* ) cBuff33;
	}


#endif




//if( channel >= ac_opened_out_channels ) return 0;


if(  ac_test_sig_type != en_ac_test_sig_none )	return 1;			//if test signal, don't modify buffers

//return 1;


//linux code
//#ifndef compile_for_windows

int j = 0;
//cycle all float/double samples and convert to integer then store



if( ch == 0 )
	{
//int tmp =  ac_VU0 * ac_gain_out_ch1 * ptr[ 0 ];				//scale audio
//printf("aud-tmp: %d\n", tmp );

	for( int i = 0; i < ac_double_bufsize; i++ )
		{
		int iv1 =  ac_VU0 * ac_gain_out_ch1 * ptr[ i ];			//scale audio

		//capture any level clipping
		if ( iv1 > audio_hardware_bits_max_integer ) 
			{
			lev_clip_flag = 10;									//capture clipping
			lev_clip_ch = 0;
			lev_clip_polarity = 1;
			lev_clip_val = ptr[ i ];
			iv1 = audio_hardware_bits_max_integer;				//limit
			}
			
		if ( iv1 < audio_hardware_bits_min_integer ) 
			{
			lev_clip_flag = 10;									//capture clipping
			lev_clip_ch = 0;
			lev_clip_polarity = -1;
			lev_clip_val = ptr[ i ];
			iv1 = audio_hardware_bits_min_integer;				//limit
			}

		cptr[ j ] = iv1 & 0xff;			//little endianess
//ac_buf[ j ] = 0;
		j++;

		cptr[ j ] = iv1 >> 8;
//ac_buf[ j ] = 0;
		j++;

//		ac_buf[ j ] = iv1 & 0xff;				//little endianess
//ac_buf[ j ] = 0;
		j++;

//		ac_buf[ j ] = iv1 >> 8;
//ac_buf[ j ] = 0;
		j++;
		}
	ac_loaded_buf1 = 1;							//flag buf has been loaded
	}


if( ch == 1 )
	{
	for( int i = 0; i < ac_double_bufsize; i++ )
		{
		int iv1 =  ac_VU0 * ac_gain_out_ch2 * ptr[ i ];			//scale audio

		//capture any level clipping
		if ( iv1 > audio_hardware_bits_max_integer ) 
			{
			lev_clip_flag = 7;									//capture clipping
			lev_clip_ch = 1;
			lev_clip_polarity = 1;
			lev_clip_val = ptr[ i ];
			iv1 = audio_hardware_bits_max_integer;				//limit
			}
			
		if ( iv1 < audio_hardware_bits_min_integer ) 
			{
			lev_clip_flag = 7;									//capture clipping
			lev_clip_ch = 1;
			lev_clip_polarity = -1;
			lev_clip_val = ptr[ i ];
			iv1 = audio_hardware_bits_min_integer;				//limit
			}

//		ac_buf[ j ] = iv1 & 0xff;				//little endianess
		j++;

//		ac_buf[ j ] = iv1 >> 8;
		j++;

		cptr[ j ] = iv1 & 0xff;				//little endianess
		j++;

		cptr[ j ] = iv1 >> 8;
		j++;
		}
	ac_loaded_buf2 = 1;							//flag buf has been loaded
	}



ac_data_count_to_write = j;						//rem how much dat to write

}













//converts audio card a/d integer samples to flts fater first scaling with ac_VU0
bool audio_card_prepare_in( int ch )
{
flts *ptr = 0;
unsigned char *cptr = 0;

//return 0;

//printf( "audio_card_prepare_in() - here\n" );

if( ch >= ac_opened_in_channels )
	{
//	if ( my_verbose ) printf( "audio_card_prepare_in() - specified channel is out of range\n" );
	return 0;
	}


ptr = 0;
if( ch == 0 ) ptr = audiocard_inbuf0;
if( ch == 1 ) ptr = audiocard_inbuf1;

if( ptr == 0 )
	{
	if ( my_verbose ) printf( "audio_card_prepare_in() - ptr was zero\n" );
	return 0;
	}


//linux code
#ifndef compile_for_windows


if( audio_card_fd == -1 )
	{
	if ( my_verbose ) printf( "audio_card_prepare_in() - card not opened\n" );
	return 0;
	}

cptr = (unsigned char* ) ac_bufin;

#endif




//windows code
#ifdef compile_for_windows

if( hwi == 0 )
	{
	if ( my_verbose ) printf( "audio_card_prepare_in() - card not opened\n" );
	return 0;
	}



if ( ac_awaiting_in_buffer == 0 )			//which buffer is required, specified in MCICallbackWaveIn
	{
	cptr = (unsigned char* ) cBuff3;
	}

if ( ac_awaiting_in_buffer == 1 )			//which buffer is required, specified in MCICallbackWaveIn
	{
	cptr = (unsigned char* ) cBuff4;
	}


#endif




//if(  ac_test_sig_type != en_ac_test_sig_none )	return 1;			//if test signal, don't modify buffers

//return 1;


//linux code
//#ifndef compile_for_windows

//cycle all integer samples and store as float/double (flts) samples
int j = 0;

if( ch == 0 )
	{
	for( int i = 0; i < ac_double_bufsize; i++ )
		{
		int low, high;

		low = cptr[ j ];					//little endianess					
		j++;

		high = cptr[ j ];					
		j++;

		high = high << 8;
		high |= low;

		if( high > 32767 ) high = high - 65536;	//make 2 byte signed word an signed integer
	
		ptr[ i ] = (double)high / ac_VU0;		//scale audio

		j++;

		j++;
		}
//printf( "audio_card_prepare_in - %f\n", ptr[ 0 ]  );
//printf( "audio_card_prepare_in - %d\n", ac_bufin[ 0 ], ac_bufin[ 1 ]  );
	}


if( ch == 1 )
	{
	for( int i = 0; i < ac_double_bufsize; i++ )
		{
		int low, high;

		j++;

		j++;

		low = cptr[ j ];					//little endianess					
		j++;

		high = cptr[ j ];				
		j++;

		high = high << 8;
		high |= low;

		if( high > 32767 ) high = high - 65536;	//make 2 byte signed word a signed integer

		ptr[ i ] = (double)high / ac_VU0;		//scale audio
		}
	}

//ac_data_count_to_read = j;						//rem how much dat to write
//ac_need_more_data = 0;						//flag we now have data

//printf("writing %d\n", j );

//			ac_need_more_data = 1;						//flag we need more data

//#endif



/*
//windows code
#ifdef compile_for_windows

//cycle all integer samples and store as float/double (flts) samples
int j = 0;

if( ch == 0 )
	{
//printf( "ac_double_bufsize= %d\n", ac_double_bufsize );
	for( int i = 0; i < ac_double_bufsize; i++ )
		{
		int low, high;

		low = cptr[ j ];						//little endianess					
		j++;

		high = (int)cptr[ j ] * 256;			//make msbyte preserving sign					
		j++;
	
		high += low;
		ptr[ i ] = (double)high / ac_VU0 ;		//scale audio

		j++;

		j++;
		}
	}


if( ch == 1 )
	{
	for( int i = 0; i < ac_double_bufsize; i++ )
		{
		int low, high;

		j++;

		j++;

		low = cptr[ j ];						//little endianess					
		j++;

		high = (int)cptr[ j ] * 256;			//make msbyte preserving sign					
		j++;

		high += low;
		ptr[ i ] = (double)high / ac_VU0 ;		//scale audio
		}
	}

//printf( "j = %d\n", j );

#endif

*/
}























//0 = none
//1 = whitenoise
//2 = tone
//level between 0 - 0x7fff
void audio_card_inject_test_sig( int type,  unsigned int freq, unsigned int level )
{
	
if( level > 0x7fff ) level = 0x7fff;

ac_test_sig_type = type;
ac_test_sig_freq = freq;
ac_test_sig_level = level;

}











//windows code
#ifdef compile_for_windows

//MM callback handler for wav output
void CALLBACK MCICallbackWaveOut(HWAVEOUT hw, UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
//printf( "MCICallbackWaveOut\n" );




if( cb_user_process_out == 0 )
	{
	printf("MCICallbackWaveOut() - no callback set\n" );
	return;
	}

LPWAVEHDR lpwh;

switch(uMsg)
	{ 
		case MM_WOM_OPEN:
		break;


		case WOM_DONE:	//clean up buffer freed, that this message points to
			lpwh=(LPWAVEHDR) dwParam1;


			waveOutUnprepareHeader(hw,(LPWAVEHDR) dwParam1,sizeof(WAVEHDR) );

//printf("Here1\n" );
			bStillBuffersToPlay=1;

			//have all buffers become free?
			if( wh1.dwFlags == WHDR_DONE )
				{
				if(wh2.dwFlags==WHDR_DONE)
					{
					if(wh33.dwFlags==WHDR_DONE)
						{
						bStillBuffersToPlay=0;
						bStarted=0;
						}
//					waveOutReset(hw);		//stop output
//					waveOutClose(hw);		//this kills buffers

					}
				}




			//keep supplying buffers after they done and have be unprepared
			if( (bIssueMoreBuffers) && (wh1.dwFlags==WHDR_DONE) )
					{
//printf("MCICallbackWaveOut wh1\n" );

					ac_awaiting_out_buffer = 0;					//identify which char buf for audio_card_prepare_out()
					if( cb_user_process_out != 0 ) cb_user_process_out( 0, ac_double_bufsize, user_args );		//call user code to generate/load samples
					audio_card_make_test_signal_if_req(cBuff1,dwBufferSize);	//overwrite with test signal?
					wh1.lpData=(LPSTR) cBuff1;
					wh1.dwBufferLength=dwBufferSize;
					wh1.dwFlags=0;
					waveOutPrepareHeader(hw ,&wh1,sizeof(WAVEHDR) );
					waveOutWrite(hw,&wh1,sizeof(WAVEHDR)); 
					}


			if( (bIssueMoreBuffers) && (wh2.dwFlags==WHDR_DONE) )
					{
//printf("MCICallbackWaveOut wh2\n" );
					ac_awaiting_out_buffer = 1;					//identify which char buf for audio_card_prepare_out()
					if( cb_user_process_out != 0 ) cb_user_process_out( 1, ac_double_bufsize, user_args );		//call user code to generate/load samples
					audio_card_make_test_signal_if_req(cBuff2,dwBufferSize);	//overwrite with test signal?
					wh2.lpData=(LPSTR) cBuff2;
					wh2.dwBufferLength=dwBufferSize;
					wh2.dwFlags=0;
					waveOutPrepareHeader(hw ,&wh2,sizeof(WAVEHDR) );
					waveOutWrite(hw,&wh2,sizeof(WAVEHDR)); 
					}


			if( (bIssueMoreBuffers) && (wh33.dwFlags==WHDR_DONE) )
					{
//printf("MCICallbackWaveOut wh33\n" );
					ac_awaiting_out_buffer = 2;					//identify which char buf for audio_card_prepare_out()
					if( cb_user_process_out != 0 ) cb_user_process_out( 2, ac_double_bufsize, user_args );		//call user code to generate/load samples
					audio_card_make_test_signal_if_req(cBuff33,dwBufferSize);	//overwrite with test signal?
					wh33.lpData=(LPSTR) cBuff33;
					wh33.dwBufferLength=dwBufferSize;
					wh33.dwFlags=0;
					waveOutPrepareHeader(hw ,&wh33,sizeof(WAVEHDR) );
					waveOutWrite(hw,&wh33,sizeof(WAVEHDR)); 
					}
		
		break;


		case MM_WOM_CLOSE:	//clean up buffers
			waveOutUnprepareHeader(hwo,&wh1,sizeof(WAVEHDR) );
			waveOutUnprepareHeader(hwo,&wh2,sizeof(WAVEHDR) );
			waveOutUnprepareHeader(hwo,&wh33,sizeof(WAVEHDR) );
//			waveOutUnprepareHeader(hwo,&wh3,sizeof(WAVEHDR) );

//			waveOutReset(hwo);		//???? locks up
//			waveOutClose(hwo);


//			hwo=0;
			//delete buffers
//			if(cBuff1) delete [] cBuff1;
//			if(cBuff2) delete [] cBuff2;
//			if(cBuff3) delete [] cBuff3;
//			cBuff1=0;
//			cBuff2=0;
//			cBuff3=0;
			
		break;

	}

if( !ac_loaded_buf1 )				//buf not loaded in time?
	{
	ac_missed_buf1 = 1;			//latch a missed buf
	ac_missed_buf_cnt1++;
	}

if( !ac_loaded_buf2 )				//buf not loaded in time?
	{
	ac_missed_buf2 = 1;			//latch a missed buf
	ac_missed_buf_cnt2++;
	}

ac_loaded_buf1 = 0;				//flag buf processed
ac_loaded_buf2 = 0;

}










//MM callback handler for wav input
void CALLBACK MCICallbackWaveIn( HWAVEIN hw, UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2 )
{
LPWAVEHDR Temp=(LPWAVEHDR)dwParam1;

//printf("here1\n" );

if( cb_user_process_in == 0 )
	{
	printf("MCICallbackWaveIn() - no callback set\n" );
	return;
	}

	switch(uMsg)
	{
		case MM_WIM_OPEN:

		break;

		case MM_WIM_DATA:
			
//printf("here2\n" );
				

				//clean up buffer freed, that this message points to
			waveInUnprepareHeader( hw ,(LPWAVEHDR) dwParam1 , sizeof(WAVEHDR) );

			//have all buffers become free?
			if( wh3.dwFlags&WHDR_DONE )
				{
				if( wh4.dwFlags&WHDR_DONE )
					{
					started_wave_in = 0;
					}
				}


			//keep supplying buffers as they are freed
			if( issue_more_buf_rec && ( wh3.dwFlags & WHDR_DONE ) )
					{
					ac_awaiting_in_buffer = 0;					//identify which char buf for audio_card_prepare_in()
					if( cb_user_process_in != 0 ) cb_user_process_in( 0, ac_double_bufsize, user_args );		//call user code to accept samples from soundcard
					wh3.lpData = (LPSTR)cBuff3;
					wh3.dwBufferLength = dwBufferSize;
					wh3.dwFlags = 0;
					wh3.dwBytesRecorded = 0;
					waveInPrepareHeader( hw ,&wh3, sizeof(WAVEHDR) );
					waveInAddBuffer( hw, &wh3, sizeof(WAVEHDR) ); 
//printf("wh3\n" );

					}

			if( issue_more_buf_rec &&( wh4.dwFlags & WHDR_DONE ) )
					{
					ac_awaiting_in_buffer = 1;					//identify which char buf for audio_card_prepare_in()
					if( cb_user_process_in != 0 ) cb_user_process_in( 1, ac_double_bufsize, user_args );		//call user code to accept samples from soundcard
					wh4.lpData= (LPSTR)cBuff4;
					wh4.dwBufferLength = dwBufferSize;
					wh4.dwFlags = 0;
					wh4.dwBytesRecorded = 0;
					waveInPrepareHeader( hw ,&wh4, sizeof(WAVEHDR) );
					waveInAddBuffer( hw, &wh4, sizeof(WAVEHDR)); 
//printf("wh4\n" );
					}

		break;


		case MM_WIM_CLOSE:	//clean up buffers

			waveInUnprepareHeader( hw, &wh3, sizeof(WAVEHDR) );
			waveInUnprepareHeader( hw, &wh4, sizeof(WAVEHDR) );

		break;




	}
}




#endif









//inject supplied char buffer with tone or white noise in 16 bit little endian
//stereo audio format, left channel first, right second
//will do nothing to char buffer if 'ac_test_sig_type == en_ac_test_sig_none'
//iSize must be multiple of 4
void audio_card_make_test_signal_if_req(char *pBuf,int iSize)
{
int iSample;
static int iLastSample2;
static int iLastSample3;
static int iLastSample4;
static int iLastSample5;
static int iLastSample6;
static int iLastSample7;
static int iLastSample8;
static int iLastSample9;


if(  ac_test_sig_type == en_ac_test_sig_none )				//no test signal, then audio_card_prepare_channel() should have been called and already loaded cBuffx
	{
	}





if(  ac_test_sig_type == en_ac_test_sig_tone )				//tone test signal?
	{

	for( int i = 0; i < iSize; i += 4 )
		{
		ac_tone_freq1 = ac_test_sig_freq;

		ac_tone_inc1 = ac_tone_freq1 * ac_two_pi * ac_sample_time;

		ac_tsg_gain1 = ac_test_sig_level;
		
		double dv = (double)ac_tsg_gain1 * sin( ac_tone_theta1 );
		int ui = dv;

//	printf("iSize=%d, ui= %x\n", iSize, ui );

		pBuf[ i ] = ui & 0xff;
		pBuf[ i + 1 ] = ui >> 8;

		pBuf[ i + 2 ] = ui & 0xff;
		pBuf[ i + 3 ] = ui >> 8;

//		pBuf[ i + 0 ] = 0;
//		pBuf[ i + 1 ] = 0;
//		pBuf[ i + 2 ] = 0;
//		pBuf[ i + 3 ] = 0;

		//calc next sample's phase
		ac_tone_theta1 += ac_tone_inc1;
		if( ac_tone_theta1 >= ac_two_pi ) ac_tone_theta1 -= ac_two_pi;
		}

	ac_data_count_to_write = iSize;					//flag how much to write to sound card
	return;
	}




if(  ac_test_sig_type == en_ac_test_sig_whitenoise )		//whitenoise signal?
	{
	ac_tsg_gain1 = ac_test_sig_level;

	for(int i=0;i<iSize;i+=4)
		{
		//this gives val -1.0 -> 1.0
		double whnz = (double) rand() / (double)( RAND_MAX / 2 ) - 1.0;	//RAND_MAX is 2147483647 0x7fffffff

		iSample = whnz * ac_tsg_gain1;


		if( ac_whtnz_bFilterOn != 0 )			//do filtering?
			{

			if( ac_whtnz_iFilter ==1)
				{	
				iSample=iSample+iLastSample2;
				iSample=iSample/2;
				}

			if( ac_whtnz_iFilter ==2)
				{	
				iSample=iSample+iLastSample2+iLastSample3;
				iSample=iSample/3;
				}

			if( ac_whtnz_iFilter ==3)
				{	
				iSample=iSample+iLastSample2+iLastSample3+iLastSample4;
				iSample=iSample/4;
				}

			if( ac_whtnz_iFilter ==4)
				{	
				iSample=iSample+iLastSample2+iLastSample3+iLastSample4+iLastSample5;
				iSample=iSample/5;
				}

			if( ac_whtnz_iFilter ==5)
				{	
				iSample=iSample+iLastSample2+iLastSample3+iLastSample4+iLastSample5+iLastSample6;
				iSample=iSample/6;
				}

			if( ac_whtnz_iFilter ==6)
				{	
				iSample=iSample+iLastSample2+iLastSample3+iLastSample4+iLastSample5+iLastSample6+iLastSample7;
				iSample=iSample/7;
				}

			if( ac_whtnz_iFilter ==7)
				{	
				iSample=iSample+iLastSample2+iLastSample3+iLastSample4+iLastSample5+iLastSample6+iLastSample7+iLastSample8;
				iSample=iSample/8;
				}

			if( ac_whtnz_iFilter ==8)
				{	
				iSample=iSample+iLastSample2+iLastSample3+iLastSample4+iLastSample5+iLastSample6+iLastSample7+iLastSample8+iLastSample9;
				iSample=iSample/9;
				}
			}


		pBuf[i]=iSample&0xff;				//left
		pBuf[i+1]=(iSample&0xff00)>>8;

		pBuf[i+2]=iSample&0xff;				//right
		pBuf[i+3]=(iSample&0xff00)>>8;


		iLastSample9=iLastSample8;
		iLastSample8=iLastSample7;
		iLastSample7=iLastSample6;
		iLastSample6=iLastSample5;
		iLastSample5=iLastSample4;
		iLastSample4=iLastSample3;
		iLastSample3=iLastSample2;
		iLastSample2=iSample;

		}

	ac_data_count_to_write = iSize;					//flag how much to write to sound card
	}

}
















//linux code
#ifndef compile_for_windows

//returns number of bytes that can be written without write() blocking,
//also gets fragment size and current fragment count
//fragment count will vary dependant how full the audio buffer has 
//become by repeated write() calls
unsigned int audio_card_data_size_for_non_blocking_write( unsigned int &fragments, unsigned int &fragmentsize )
{

if( ioctl( audio_card_fd, SNDCTL_DSP_GETBLKSIZE, &fragmentsize ) ==-1 ) 
	{
	perror("SNDCTL_DSP_GETBLKSIZE");
	} 
else{
//	printf("Fragment size: %d\n", fragmentsize ); 
	}


audio_buf_info info; 

if( ioctl( audio_card_fd, SNDCTL_DSP_GETOSPACE,&info ) ==-1 )
	{
	}
else{
//	printf("Fragment count: %d\n", info.fragments ); 

	}


if( !ac_loaded_buf1 )				//buf not loaded in time?
	{
	ac_missed_buf1 = 1;			    //latch a missed buf
	ac_missed_buf_cnt1++;
	}

if( !ac_loaded_buf2 )				//buf not loaded in time?
	{
	ac_missed_buf2 = 1;			    //latch a missed buf
	ac_missed_buf_cnt2++;
	}

ac_loaded_buf1 = 0;				    //flag buf processed
ac_loaded_buf2 = 0;

fragments = info.fragments;
//printf("ac_data_count_to_write: %d\n",ac_data_count_to_write );

return info.fragments * fragmentsize;
}







//a gcthrd thread callback
//this writes samples to audio card's line o/p ( i.e: data to d/a)
void audio_card_out_thrd_cb( void* args )
{
mystr m1, m2;

string nm;
int count = 0;
unsigned int max_data;
double slip = 0;					//used to throttle this callback
unsigned int half_max_fragments = max_fragments / 2;
//printf("callback1_1\n");

gcthrd *o = (gcthrd*) args;

strpf( nm, "%s -", o->obj_name.c_str() );

//if( o->logr ) o->thrd1->logr->pf( o->pr, "%s thrd started\n", nm.c_str(), count );
//if( o->logr ) o->thrd1->logr->pf( o->pr, "%s cb() - callback\n",  nm.c_str() );

//ac_need_more_data = 1;					//flag we need data

m1.time_start( m1.ns_tim_start );

while( 1 )
	{
	if( o->thrd.kill ) goto finthread;

	
	double dt = m1.time_passed( m1.ns_tim_start );
	double frame_time = 1.0 / (double)ac_srate * ac_double_bufsize;
	if( dt >= ( frame_time + slip ) )		//is callback's time to be called?
		{
		m1.time_start( m1.ns_tim_start );
//		if( o->cb_ptr_process ) o->cb_ptr_process( (unsigned int) o->chout, (unsigned int) o->usr_buf_size, args );

//		if( ac_need_more_data )				//need more data?
			{
			if( cb_user_process_out ) cb_user_process_out( 0, ac_double_bufsize, args );
			audio_card_make_test_signal_if_req( (char*)ac_bufout, ac_double_bufsize * 4 );	//overwrite with test signal?

//			if( cb_user_process_in ) cb_user_process_in( 0, ac_double_bufsize, args );
			int status = write( audio_card_fd, ac_bufout, ac_data_count_to_write );			//write buf to line/spkr o/p (d/a)
			if ( status != ac_data_count_to_write ) if ( my_verbose ) printf("audio_card_thrd_cb() - wrong number of bytes written\n");
			}

//			{
//m2.time_start( m2.ns_tim_start );
//			int status = read( audio_card_fd, ac_bufin, ac_uc_bufsize );			 //read buf from mic/line i/p (a/d)
//			if ( status != ac_uc_bufsize ) if ( my_verbose ) printf("audio_card_thrd_cb() - wrote wrong number of bytes read\n");
//
//double drd = m2.time_passed( m2.ns_tim_start );
//printf( "drd = %g\n", drd );
//			}

		unsigned int fragments;
		unsigned int fragmentsize;
		max_data = audio_card_data_size_for_non_blocking_write( fragments, fragmentsize );

		double slip_secs = frame_time * 0.02;	//set slip time to 2% of frametime
	
		//keep loaded fragments at around half sound driver's max reported when driver was first opened
		if( fragments > half_max_fragments ) slip = -slip_secs;		//call callback faster?
		else slip = slip_secs;										//call callback slower?

		//below printf shows if code dithers between the two slip times and
		//you should see fragments at half the maximum allowable when audio card was opened

//		printf( "max_fragments: %d fragments: %d, slip: %lf\n", max_fragments, fragments, slip );
		continue;
		}

/*
	if( !ac_need_more_data )				//have data?
		{


		if ( max_data >= ac_data_count_to_write )
			{
			int status = write( audio_card_fd, ac_buf, ac_data_count_to_write );			//play buf
			if ( status != ac_data_count_to_write ) if ( my_verbose ) printf("audio_card_thrd_cb() - wrong number of bytes written\n");
			ac_need_more_data = 1;						//flag we need more data
			}
		}
	*/
	m1.delay_ms( 1 );			//don't hog processor in this while()
	count++;
	}

finthread:
o->thrd.finished = 1;
o->thrd.kill = 0;

if( o->thrd_dbg ) printf( "%s thrd finished\n", nm.c_str() );

}














//a gcthrd thread callback
//this read samples from audio card's mic/line i/p ( i.e: data from a/d)
void audio_card_in_thrd_cb( void* args )
{
mystr m1;

string nm;

gcthrd *o = (gcthrd*) args;

strpf( nm, "%s -", o->obj_name.c_str() );


while( 1 )
	{
	if( o->thrd.kill ) goto finthread;

//m1.time_start( m1.ns_tim_start );
	int status = read( audio_card_fd, ac_bufin, ac_uc_bufsize );			 //read buf from mic/line i/p (a/d)
	if( cb_user_process_in ) cb_user_process_in( 0, ac_double_bufsize, args );
	if ( status != ac_uc_bufsize ) if ( my_verbose ) printf("audio_card_thrd_cb() - wrong number of bytes read\n");

//double drd = m1.time_passed( m1.ns_tim_start );
//printf( "drd = %g\n", drd );
	m1.delay_ms( 1 );			//don't hog processor in this while()
	}

finthread:
o->thrd.finished = 1;
o->thrd.kill = 0;

if( o->thrd_dbg ) printf( "%s thrd finished\n", nm.c_str() );

}










void audio_card_thrd_start()
{
mystr m1;


if ( my_verbose ) printf( "audio_card_thrd_start() - called\n" );

if( ac_thrd1 == 0 ) 
	{
	ac_thrd1 = new gcthrd();

//	thrd1->set_log_ptr( 5, logr );			//set thrd to use a prev created log obj
	ac_thrd1->thrd_dbg = 1;
	ac_thrd1->set_name( "ac_thrd1" );
	ac_thrd1->set_thrd_callback( audio_card_out_thrd_cb, ac_thrd1 );		//set thread callback
	}

ac_thrd1->create_thread( );



if( ac_thrd2 == 0 ) 
	{
	ac_thrd2 = new gcthrd();

	ac_thrd2->thrd_dbg = 1;
	ac_thrd2->set_name( "ac_thrd2" );
	ac_thrd2->set_thrd_callback( audio_card_in_thrd_cb, ac_thrd2 );		//set thread callback
	}

ac_thrd2->create_thread( );

}







void audio_card_thrd_stop()
{

if( ac_thrd1 != 0 )
	{
	if ( my_verbose ) printf( "audio_card_thrd_stop() - ac_thrd1 to be killed\n" );
	ac_thrd1->destroy_thread( );
	if( ac_thrd1->wait_till_thread_destroyed( 1000 ) )
		{
		delete ac_thrd1;
		ac_thrd1 = 0;
		}
	}


if( ac_thrd2 != 0 )
	{
	if ( my_verbose ) printf( "audio_card_thrd_stop() - ac_thrd2 to be killed\n" );
	ac_thrd2->destroy_thread( );
	if( ac_thrd2->wait_till_thread_destroyed( 1000 ) )
		{
		delete ac_thrd2;
		ac_thrd2 = 0;
		}
	}
}

#endif


