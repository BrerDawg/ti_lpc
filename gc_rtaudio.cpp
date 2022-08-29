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



//gc_rtaudio.cpp
//v1.01		03-jun-2019				//


#include "gc_rtaudio.h"





/*

//------------------------------------------------------------------------------------------------------------------------
//EXAMPLE CODE below - generate 2 tone outputs while mixing with incoming audio, see 'main()'
//has well worked with jack, also works with pulseaudio, pulseaudio seems to pop intermittently when an input is being used (mic)

mystr tim1;

struct st_osc_params_tag							//a pointer to this is stored in stuct passed to audio proc callback, see 'st_rtaud_arg_tag.usr_ptr' 
{
float freq0;
float gain0;

float freq1;	
float gain1;
} st_osc_params;


rtaud rta;




int dbg_cnt = 0;


double theta0 = 0;
double theta1 = 0;
double theta0_inc;
double theta1_inc;
#define twopi (M_PI * 2)



//-------------------- realtime audio proc -----------------------------
int cb_inout( void *bf_out, void *bf_in, int frames, double streamTime, RtAudioStreamStatus status, void *arg_in )
{
	
double dt = tim1.time_passed( tim1.ns_tim_start );
tim1.time_start( tim1.ns_tim_start );

if( !(dbg_cnt%40) )
	{
	printf( "cb_inout() - dt: %f, frames: %d\n", dt, frames );
	}
	

if ( status ) std::cout << "Stream over/underflow detected." << std::endl;



float *bfin = (float *) bf_in;											//depends on which 'audio_format' data type, refer: 'RtAudioFormat'
float *bfout = (float *) bf_out;


st_rtaud_arg_tag *arg = (st_rtaud_arg_tag*) arg_in;

st_osc_params_tag *usr = (st_osc_params_tag*) arg->usr_ptr;



theta0_inc = usr->freq0 * twopi / arg->srate;
theta1_inc = usr->freq1 * twopi / arg->srate;


//printf( "inout()- frames %d\n ",frames );
//printf( "inout()- arg->channels %d\n ",arg->ch_outs );
//return 0;

dbg_cnt++;
if( !(dbg_cnt%40) )
	{
//printf( "cb arg: %08x\n", (unsigned int)arg );
//	printf( "cb_inout() - arg.bufsiz: %d\n", arg->bufsiz );
	printf( "cb_inout() - arg.ch_ins: %d\n", arg->ch_ins );
	printf( "cb_inout() - arg.ch_outs: %d\n",arg->ch_outs );
	printf( "cb_inout() - arg.audio_format: 0x%08x\n\n",arg->audio_format );
	}


int idx_in = 0;
int idx_out = 0;

//below handles a max 2 channels of either/or input and output
for( int i = 0; i < frames; i++ )
	{
	float in0, in1, tone;
	
	in0 = in1 = 0;
	
	if ( arg->ch_ins != 0 ) in0 = 0.2 * bfin[ idx_in++ ];			//input samples?
	if ( arg->ch_ins == 2 ) in1 = 0.2 * bfin[ idx_in++ ];
	
	if ( arg->ch_outs != 0 )										//output?
		{
		tone = in0 + sin( theta0 );
		bfout[ idx_out++ ] = tone * usr->gain0;
		
		theta0 += theta0_inc;
		if( theta0 >= twopi ) theta0 -= twopi;
		}

	if ( arg->ch_outs == 2 )										//2nd output?
		{
		tone = in1 + sin( theta1 );									//mix
		bfout[ idx_out++ ] = tone * usr->gain1;
		
		theta1 += theta1_inc;
		if( theta1 >= twopi ) theta1 -= twopi;
		}
		
	}

//  unsigned int *bytes = (unsigned int *) data;
//  memcpy( outputBuffer, inputBuffer, *bytes );
 return 0;
}

//----------------------------------------------------------------------



int rtaudio_probe()
{
vector<RtAudio::DeviceInfo> vinfo;

rta.get_avail_devices( vinfo );

for( int i = 0; i < vinfo.size(); i++ )
	{
	printf("---- device number: %02d -----\n", i );
	printf("RtAudio::DeviceInfo.name: '%s'\n", vinfo[i].name.c_str() );
	printf("RtAudio::DeviceInfo.inputChannels: %d\n", vinfo[i].inputChannels );
	printf("RtAudio::DeviceInfo.outputChannels: %d\n", vinfo[i].outputChannels );
	printf("RtAudio::DeviceInfo.duplexChannels: %d\n", vinfo[i].duplexChannels );
	printf("RtAudio::DeviceInfo.isDefaultInput: %d\n", vinfo[i].isDefaultInput );
	printf("RtAudio::DeviceInfo.isDefaultOutput: %d\n", vinfo[i].isDefaultOutput );
	printf("RtAudio::DeviceInfo.preferredSampleRate: %d\n", vinfo[i].preferredSampleRate );
	printf("RtAudio::DeviceInfo.nativeFormats: 0x%04x\n", (int)vinfo[i].nativeFormats );
	for( int j = 0; j < vinfo[i].sampleRates.size(); j++ )
		{
		printf("supports samplerate: %d\n", vinfo[i].sampleRates[j] );
		}	

	printf("----------------------------\n\n" );
	}
}





int main( int argc, char *argv[] )
{

rta.verbose = 1;

rtaudio_probe();									//dump a list of devices to console


st_rtaud_arg_tag st_arg;							//this is used in audio proc callback to work out chan in/out counts
RtAudio::StreamOptions options;

options.streamName = "myrt";
options.numberOfBuffers = 2;						//for jack (at least) this is hard set by jack's settings and can't be changed via this parameter

options.flags = 0; 	//0 means interleaved, use oring options, refer 'RtAudioStreamFlags': RTAUDIO_NONINTERLEAVED, RTAUDIO_MINIMIZE_LATENCY, RTAUDIO_HOG_DEVICE, RTAUDIO_SCHEDULE_REALTIME, RTAUDIO_ALSA_USE_DEFAULT, RTAUDIO_JACK_DONT_CONNECT
					// !!! when using RTAUDIO_JACK_DONT_CONNECT, you can create any number of channels, you can't do this if auto connecting as 'openStream()' will fail if there is not enough channel mating ports  

options.priority = 0;								//only used with flag 'RTAUDIO_SCHEDULE_REALTIME'


uint16_t device_num_in = 0;							//use 0 for default device to be used
int channels_in = 2;								//if auto connecting (not RTAUDIO_JACK_DONT_CONNECT), there must be enough mathing ports or 'openStream()' will fail
int first_chan_in = 0;

uint16_t device_num_out = 0;						//use 0 for default device to be used
int channels_out = 2;								//if auto connecting (not RTAUDIO_JACK_DONT_CONNECT), there must be enough mathing ports or 'openStream()' will fail
int first_chan_out = 0;

int srate = 48000;
uint16_t frames = 2048;
unsigned int audio_format = RTAUDIO_FLOAT32;		//see below for supported format types, adj audio proc code to suit

//    RTAUDIO_SINT8: 8-bit signed integer.
//    RTAUDIO_SINT16: 16-bit signed integer.
//    RTAUDIO_SINT24: 24-bit signed integer.
//    RTAUDIO_SINT32: 32-bit signed integer.
//    RTAUDIO_FLOAT32: Normalized between plus/minus 1.0.
//    RTAUDIO_FLOAT64: Normalized between plus/minus 1.0.



st_osc_params.freq0 = 200;							//set up some audio proc callback user params
st_osc_params.gain0 = 0.1;

st_osc_params.freq1 = 600;
st_osc_params.gain1 = 0.1;
st_arg.usr_ptr = (void*)&st_osc_params;


//  --- un-comment to test each possibility ---
//if( !rta.start_stream_in( device_num_in, channels_in, first_chan_in, srate, frames, audio_format, &options, &st_arg, (void*)cb_inout ) )			//input only
//if( !rta.start_stream_out( device_num_out, channels_out, first_chan_out, srate, frames, audio_format, &options, &st_arg, (void*)cb_inout ) )		//output only
if( !rta.start_stream_duplex( device_num_in, device_num_out, channels_in, first_chan_in, channels_out, first_chan_out, srate, frames, audio_format, &options, &st_arg, (void*)cb_inout ) )		//input and output
	{
	printf("failed to open audio device!!!!\n" );	
	}
else{
//	printf("audio in device %d opened, srate is: %d, framecnt: %d\n", device_num_in, rta.get_srate(), rta.get_framecnt()  );							 //input only	
//	printf("audio out device %d opened, srate is: %d, framecnt: %d\n", device_num_out, rta.get_srate(), rta.get_framecnt()  );							 //output only	
	printf("audio devices opened in: %d, out: %d, srate is: %d, framecnt: %d\n", device_num_in, device_num_out, rta.get_srate(), rta.get_framecnt()  );	 //(duplex)	
	}

printf("\npress a key\n" );	
getchar();											//wait here and do nothing, audio proc will be generating audio samples
return 0;
}

//------------------------------------------------------------------------------------------------------------------------


*/













rtaud::rtaud()
{
device_num_in = -1;
device_num_out = -1;
verbose = 0;
samplerate = 0;
framecnt = 0;

adac.showWarnings( true );
}



rtaud::~rtaud()
{
if( ( device_num_in >= 0 ) || ( device_num_out >= 0 ) ) stop_stream();

}






int rtaud::get_avail_devices( vector< RtAudio::DeviceInfo> &vinfo )
{
int cnt = adac.getDeviceCount();

vinfo.clear();
if( cnt == 0 ) return 0;

for( int i = 0; i < cnt; i++ )
	{
	RtAudio::DeviceInfo o = adac.getDeviceInfo( i );
	vinfo.push_back( o );
	}

return vinfo.size();
}



int rtaud::get_srate()
{
if( ( device_num_in < 0 ) && ( device_num_out < 0 ) )
	{
	printf( "rtaud::get_srate() - failed, no audio device is opened\n" );
	samplerate = 0;
	return samplerate;
	}

samplerate = adac.getStreamSampleRate();
return samplerate;
}



int rtaud::get_framecnt()
{
if( ( device_num_in < 0 ) && ( device_num_out < 0 ) )
	{
	printf( "rtaud::get_framecnt() - failed, no audio device is opened\n" );
	return 0;
	}
return framecnt;
}






//input and output
bool rtaud::start_stream_duplex( uint16_t start_in_device_num, uint16_t start_out_device_num, uint16_t channels_in, uint16_t first_chan_in, uint16_t channels_out, uint16_t first_chan_out, uint16_t srate, unsigned int frames, unsigned int audio_format, RtAudio::StreamOptions *options, st_rtaud_arg_tag *st_arg, void *cb )
{
if( verbose ) printf( "rtaud::start_stream_duplex()\n" );


if( device_num_in >= 0 )
	{
	printf( "rtaud::start_stream_duplex() - failed, 'device_num_in' %d stream already started\n", device_num_in );
	return 0;
	}

if( device_num_out >= 0 )
	{
	printf( "rtaud::start_stream_duplex() - failed, 'device_num_out' %d stream already started\n", device_num_out );
	return 0;
	}

if( channels_in == 0 )
	{
	printf( "rtaud::start_stream_duplex() - failed, 0 for 'channels_in' is invalid\n" );
	return 0;
	}

if( channels_out == 0 )
	{
	printf( "rtaud::start_stream_duplex() - failed, 0 for 'channels_out' is invalid\n" );
	return 0;
	}


if( cb == 0 )
	{
	printf( "rtaud::start_stream_duplex() - failed, a callback must be specified\n" );
	return 0;
	}

if( st_arg == 0 )
	{
	printf( "rtaud::start_stream_duplex() - failed, 'st_arg' is zero\n" );
	return 0;
	}

if ( adac.getDeviceCount() < 1 )
	{
	printf( "\nrtaud::start_stream_duplex() - no audio devices found\n" );
    return 0;
	}


if ( start_in_device_num >= adac.getDeviceCount() )
	{
	printf( "\nrtaud::start_stream_duplex() - 'start_in_device_num' %d not found\n", start_in_device_num );
    return 0;
	}

if ( start_out_device_num >= adac.getDeviceCount() )
	{
	printf( "\nrtaud::start_stream_duplex() - 'start_out_device_num' %d not found\n", start_out_device_num );
    return 0;
	}

device_num_in = start_in_device_num;
if ( start_in_device_num == 0 ) device_num_in = adac.getDefaultInputDevice();

device_num_out = start_out_device_num;
if ( start_in_device_num == 0 ) device_num_out = adac.getDefaultOutputDevice();


st_arg->device_num_in = device_num_in;
st_arg->device_num_out = device_num_out;
st_arg->srate = srate;
st_arg->ch_ins = channels_in;
st_arg->ch_outs = channels_out;
st_arg->audio_format = audio_format;


RtAudio::StreamParameters iparams, oparams;


try {
	iparams.deviceId = start_in_device_num;
	iparams.nChannels = channels_in;
	iparams.firstChannel = first_chan_in;

	oparams.deviceId = start_out_device_num;
	oparams.nChannels = channels_out;
	oparams.firstChannel = first_chan_out;

	unsigned int srate2 = srate;

	adac.openStream( &oparams, &iparams, audio_format, srate2, &frames, (RtAudioCallback)cb, (void *)st_arg, options );

	st_arg->srate = srate2;
	samplerate = srate2;

	st_arg->framecnt = frames;
	framecnt = frames;

	if( verbose) printf( "rtaud::start_stream_duplex() - opened stream: srate %d,  frame size: %d\n", st_arg->srate , st_arg->framecnt );
	}
catch ( RtAudioError& e ) 
	{
	string s1 = e.getMessage();
	
	printf( "rtaud::start_stream_duplex() - error while opening stream:-\n'%s'\n", s1.c_str() );
//    std::cout << '\n' << e.getMessage() << '\n' << std::endl;

	device_num_in = -1;
	device_num_out = -1;
    return 0;
	}
	 
st_arg->latency_in_bytes =  adac.getStreamLatency();
	
st_arg->srate = samplerate = adac.getStreamSampleRate();

if( verbose) printf( "rtaud::start_stream_duplex() - stream latency %d\n", st_arg->latency_in_bytes );



try {
	adac.startStream();
	}
catch( RtAudioError& e )
	{	
	string s1 = e.getMessage();
	
	printf( "rtaud::start_stream_duplex() - error while starting stream:-\n'%s'\n", s1.c_str() );
	if ( adac.isStreamOpen() ) adac.closeStream();
	return 0;
	}


return 1;
}











//output only, no input
bool rtaud::start_stream_out( uint16_t start_device_num, uint16_t channels_out, uint16_t first_chan, uint16_t srate, unsigned int frames, unsigned int audio_format, RtAudio::StreamOptions *options, st_rtaud_arg_tag *st_arg, void *cb )
{
if( verbose ) printf( "rtaud::start_stream_out()\n" );


if( device_num_out >= 0 )
	{
	printf( "rtaud::start_stream_out() - failed, 'device_num_out' %d stream already started\n", device_num_out );
	return 0;
	}

if( channels_out == 0 )
	{
	printf( "rtaud::start_stream_out() - failed, 0 for 'channels_out' is invalid\n" );
	return 0;
	}


if( cb == 0 )
	{
	printf( "rtaud::start_stream_out() - failed, a callback must be specified\n" );
	return 0;
	}

if( st_arg == 0 )
	{
	printf( "rtaud::start_stream_out() - failed, 'st_arg' is zero\n" );
	return 0;
	}

if ( adac.getDeviceCount() < 1 )
	{
	printf( "\nrtaud::start_stream_out() - no audio devices found\n" );
    return 0;
	}


if ( start_device_num >= adac.getDeviceCount() )
	{
	printf( "\nrtaud::start_stream_out() - device %d not found\n", start_device_num );
    return 0;
	}

device_num_out = start_device_num;
if ( start_device_num == 0 ) device_num_out = adac.getDefaultOutputDevice();;


st_arg->device_num_out = device_num_out;
st_arg->srate = srate;
st_arg->ch_outs = channels_out;
st_arg->ch_ins = 0;
st_arg->audio_format = audio_format;


RtAudio::StreamParameters params;


try {
	params.deviceId = st_arg->device_num_out ;
	params.nChannels = channels_out;
	params.firstChannel = first_chan;

	unsigned int srate2 = srate;


	adac.openStream( &params, 0, audio_format, srate2, &frames, (RtAudioCallback)cb, (void *)st_arg, options );

	st_arg->srate = srate2;
	samplerate = srate2;

	st_arg->framecnt = frames;
	framecnt = frames;

	if( verbose) printf( "rtaud::start_stream_out() - opened stream: srate %d,  frame size: %d\n", st_arg->srate , st_arg->framecnt );
	}
catch ( RtAudioError& e ) 
	{
	string s1 = e.getMessage();
	
	printf( "rtaud::start_stream_out() - error while opening stream:-\n'%s'\n", s1.c_str() );
//    std::cout << '\n' << e.getMessage() << '\n' << std::endl;

	device_num_out = -1;
    return 0;
	}
	 
st_arg->latency_in_bytes =  adac.getStreamLatency();
	
st_arg->srate = samplerate = adac.getStreamSampleRate();
	
if( verbose) printf( "rtaud::start_stream_out() - stream latency %d\n", st_arg->latency_in_bytes );



try {
	adac.startStream();
	}
catch( RtAudioError& e )
	{	
	string s1 = e.getMessage();
	
	printf( "rtaud::start_stream_out() - error while starting stream:-\n'%s'\n", s1.c_str() );
	if ( adac.isStreamOpen() ) adac.closeStream();
	return 0;
	}


return 1;
}











//input only, no output
bool rtaud::start_stream_in( uint16_t start_device_num, uint16_t channels_in, uint16_t first_chan, uint16_t srate, unsigned int frames, unsigned int audio_format, RtAudio::StreamOptions *options, st_rtaud_arg_tag *st_arg, void *cb )
{
if( verbose ) printf( "rtaud::start_stream_in()\n" );


if( device_num_in >= 0 )
	{
	printf( "rtaud::start_stream_in() - failed, 'device_num_in' %d stream already started\n", device_num_out );
	return 0;
	}

if( channels_in == 0 )
	{
	printf( "rtaud::start_stream_in() - failed, 0 for 'channels_in' is invalid\n" );
	return 0;
	}


if( cb == 0 )
	{
	printf( "rtaud::start_stream_in() - failed, a callback must be specified\n" );
	return 0;
	}

if( st_arg == 0 )
	{
	printf( "rtaud::start_stream_in() - failed, 'st_arg' is zero\n" );
	return 0;
	}

if ( adac.getDeviceCount() < 1 )
	{
	printf( "\nrtaud::start_stream_in() - no audio devices found\n" );
    return 0;
	}


if ( start_device_num >= adac.getDeviceCount() )
	{
	printf( "\nrtaud::start_stream_in() - device %d not found\n", start_device_num );
    return 0;
	}

device_num_in = start_device_num;
if ( start_device_num == 0 ) device_num_in = adac.getDefaultInputDevice();;


st_arg->device_num_in = device_num_in;
st_arg->srate = srate;
st_arg->ch_outs = 0;
st_arg->ch_ins = channels_in;
st_arg->audio_format = audio_format;


RtAudio::StreamParameters params;


try {
	params.deviceId = st_arg->device_num_in;
	params.nChannels = channels_in;
	params.firstChannel = first_chan;

	unsigned int srate2 = srate;


	adac.openStream( 0, &params, audio_format, srate2, &frames, (RtAudioCallback)cb, (void *)st_arg, options );

	st_arg->srate = srate2;
	samplerate = srate2;

	st_arg->framecnt = frames;
	framecnt = frames;

	if( verbose) printf( "rtaud::start_stream_in() - opened stream: srate %d,  frame size: %d\n", st_arg->srate , st_arg->framecnt );
	}
catch ( RtAudioError& e ) 
	{
	string s1 = e.getMessage();
	
	printf( "rtaud::start_stream_in() - error while opening stream:-\n'%s'\n", s1.c_str() );
//    std::cout << '\n' << e.getMessage() << '\n' << std::endl;

	device_num_in = -1;
    return 0;
	}
	 
st_arg->latency_in_bytes =  adac.getStreamLatency();
	
st_arg->srate = samplerate = adac.getStreamSampleRate();
	
if( verbose) printf( "rtaud::start_stream_in() - stream latency %d\n", st_arg->latency_in_bytes );



try {
	adac.startStream();
	}
catch( RtAudioError& e )
	{	
	string s1 = e.getMessage();
	
	printf( "rtaud::start_stream_in() - error while starting stream:-\n'%s'\n", s1.c_str() );
	if ( adac.isStreamOpen() ) adac.closeStream();
	return 0;
	}


return 1;
}










bool rtaud::stop_stream()
{
if( verbose ) printf( "rtaud::stop_stream()\n" );

if( ( device_num_in < 0 ) && ( device_num_out < 0 ) )
	{
	printf( "rtaud::stop_stream() - failed, already stopped\n" );
	return 0;
	}

adac.stopStream();
if ( adac.isStreamOpen() ) adac.closeStream();

device_num_in = -1;
device_num_out = -1;

return 0;
}
