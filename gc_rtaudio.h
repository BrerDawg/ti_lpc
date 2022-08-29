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

//gc_rtaudio.h
//v1.01


#ifndef gc_rtaudio_h
#define gc_rtaudio_h


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

#include <iostream>
#include <cstdlib>
#include <cstring>

#include <rtaudio/RtAudio.h>
#include <rtmidi/RtMidi.h>




using namespace std;


struct st_rtaud_arg_tag							//this is used in audio proc callback to work out chan in/out counts
{
uint16_t device_num_out;
uint16_t device_num_in;
uint16_t srate;
uint16_t framecnt;
uint16_t ch_outs;
uint16_t ch_ins;
uint16_t latency_in_bytes;
unsigned int audio_format;						//refer: RtAudioFormat
void *usr_ptr;
};




class rtaud
{
private:

public:
bool verbose;
RtAudio adac;
int device_num_in;
int device_num_out;
int samplerate;
int framecnt;

rtaud();
~rtaud();

int get_avail_devices( vector< RtAudio::DeviceInfo> &vinfo );
int get_srate();
int get_framecnt();
bool start_stream_duplex( uint16_t start_in_device_num, uint16_t start_out_device_num, uint16_t channels_in, uint16_t first_chan_in, uint16_t channels_out, uint16_t first_chan_out, uint16_t srate, unsigned int frames, unsigned int audio_format, RtAudio::StreamOptions *options, st_rtaud_arg_tag *st_arg, void *cb );
bool start_stream_out( uint16_t start_device_num, uint16_t channels_out, uint16_t first_chan_out, uint16_t srate, unsigned int frames, unsigned int audio_format, RtAudio::StreamOptions *options, st_rtaud_arg_tag *st_arg, void *cb );
bool start_stream_in( uint16_t start_device_num, uint16_t channels_in, uint16_t first_chan_in, uint16_t srate, unsigned int frames, unsigned int audio_format, RtAudio::StreamOptions *options, st_rtaud_arg_tag *st_arg, void *cb );
bool stop_stream();

};


#endif
