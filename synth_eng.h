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


//synth_eng.h
//---- v1.04

#ifndef synth_eng_h
#define synth_eng_h


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string>
#include <vector>

#include "globals.h"


using namespace std;

#define max_channels 64


class caudio;




struct st_audio_tag
{
caudio *caudio_obj;							//need this for tone to be injected in local audio callback, before user callback is called

float **inbuf1, **inbuf2, **outbuf1, **outbuf2;

float **inbuf[ max_channels ];
float **outbuf[ max_channels ];

unsigned int ch_in;									//holds channel count
unsigned int ch_out;

//st_jack_tone_tag tone_in[ max_channels ], tone_out[ max_channels ];	//inject tone into a chan

int (*cb_ptr_process_in)( unsigned int bufid, unsigned int nframes, void* st_arg );		//user audio callback addresses
int (*cb_ptr_process_out)( unsigned int bufid, unsigned int nframes, void* st_arg );

void *user_arg;								//sent to user audio callback
};







class caudio
{

bool start_audio( string client_name, unsigned int channels_in, unsigned int channels_out, bool auto_conn, unsigned int bufsize, st_audio_tag *staud );
bool stop_audio();

};






#ifdef include_jack



#include <jack/jack.h>

typedef jack_default_audio_sample_t sample_t;				//this is a float

class cjack;



struct st_jack_tone_tag
{
unsigned int tone_wht;										// 0 = off, 1 = tone, 2 = white
float freq;
float ampl;
float theta;
float theta_inc;
};




struct st_jack_tag
{
cjack *cjack_obj;							//need this for tone to be injected in local audio callback, before user callback is called

sample_t **inbuf1, **inbuf2, **outbuf1, **outbuf2;

sample_t **inbuf[ max_channels ];
sample_t **outbuf[ max_channels ];

unsigned int ch_in;									//holds channel count
unsigned int ch_out;

st_jack_tone_tag tone_in[ max_channels ], tone_out[ max_channels ];	//inject tone into a chan

int (*cb_ptr_process_in)( unsigned int bufid, unsigned int nframes, void* st_arg );		//user audio callback addresses
int (*cb_ptr_process_out)( unsigned int bufid, unsigned int nframes, void* st_arg );

void *user_arg;								//sent to user audio callback
};













class cjack
{
private:

public:
int buf_dur;														//in seconds
//int chin, chout;
jack_nframes_t jack_srate;											//current sample rate
unsigned long calls, usr_buf_size;


int (*cb_ptr_process)( jack_nframes_t nframes, void *arg );
jack_client_t *client;

jack_port_t *inport[ max_channels ];				//input port
jack_port_t *outport[ max_channels ];				//output port

jack_nframes_t jack_buf_size, buf_size;				//frame size

unsigned int pr0, pw0;								//buffer pointers
unsigned int pr1, pw1;								//buffer pointers

bool tone_on_in[ max_channels ];
float tone_freq_in[ max_channels ];				//inject tone freq into in channel
float tone_amp_in[ max_channels ];
float tone_theta_in[ max_channels ];
float tone_theta_inc_in[ max_channels ];


public:
bool set_tone_in( unsigned int tone_wht, unsigned int channel, double freq, double ampl, st_jack_tag *sj );
bool set_tone_out( unsigned int tone_wht, unsigned int channel, double freq, double ampl, st_jack_tag *sj );
int get_srate();



private:
void jack_failed();


public:
cjack();
~cjack();

bool start_jack( string client_name, unsigned int channels_in, unsigned int channels_out, bool auto_conn, unsigned int bufsize, st_jack_tag *sj );
bool stop_jack();

};

#endif




#ifdef include_pulseaudio

#include <pulse/pulseaudio.h>

#endif




#endif
