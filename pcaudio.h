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


//pcaudio.h
//v1.01	 	25-may-2015			//	

#ifndef pcaudio_h
#define pcaudio_h

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
#include <sys/ioctl.h>
#include <linux/soundcard.h>
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

#include "mmsystem.h"
#include "mmreg.h"
#include "msacm.h"		//this needs mmreg.to be included first
#endif


#include "gcthrd.h"

#include "globals.h"
//#include "synth_eng.h"		//this was added just to simulate jack data types


using namespace std;




enum en_ac_test_sig
{
en_ac_test_sig_none,
en_ac_test_sig_tone	,
en_ac_test_sig_whitenoise,
};




//holds some stats on performance
struct st_audio_card_stats_tag
{
bool ac_missed_buf1;				//if set this indicates at least one ch1 buf failed to be loaded before it was needed to be sent to audio card d/a 
unsigned int ac_missed_buf_cnt1;						//this holds a count of how many missed buf events have occurred for ch1
bool ac_loaded_buf1;									//this is set by audio_card_prepare_out(..) for ch1, and reset by  


bool ac_missed_buf2;				//if set this indicates at least one ch2 buf failed to be loaded before it was needed to be sent to audio card d/a 
unsigned int ac_missed_buf_cnt2;						//this holds a count of how many missed buf events have occurred for ch1
bool ac_loaded_buf2;									//this is set by audio_card_prepare_out(..) for ch1, and reset by  
};






//bool audio_card_init( unsigned int srate, unsigned int channels, unsigned int bits, unsigned int sample_buf_size, int (*cb_user_process_in)( jack_nframes_t nframes, void *arg ), void* user_args_in );
bool audio_card_open_out( unsigned int srate, unsigned int channels, unsigned int bits, unsigned int sample_buf_size, int (*cb_user_process_out_i)( unsigned int chn, unsigned int frames, void *arg ), void* user_args_in );
bool audio_card_open_in( unsigned int srate, unsigned int channels, unsigned int bits, unsigned int sample_buf_size, int (*cb_user_process_in_i)( unsigned int chn, unsigned int frames, void *arg ), void* user_args_in );
bool audio_card_close( );
void audio_card_set_vu0( double vu0 );
bool audio_card_get_srate( unsigned int &sr );
flts *audio_card_get_outbuf( int ch );
flts *audio_card_get_inbuf( int ch );
bool audio_card_prepare_out( int ch );
bool audio_card_prepare_in( int ch );
void audio_card_inject_test_sig( int type,  unsigned int freq, unsigned int level );
void audio_card_make_test_signal_if_req(char *pBuf,int iSize);
bool audio_card_set_out_gain( double ch1, double ch2 );
//void audio_card_get_stats( st_audio_card_stats_tag &o );
//void audio_card_clear_stats( int ch );


//linux code
#ifndef compile_for_windows
unsigned int audio_card_data_size_for_non_blocking_write( unsigned int &fragments, unsigned int &fragmentsize );
#endif



#endif

