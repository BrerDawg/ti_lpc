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


//synth_eng.cpp
//---- v1.02 		01/09/2013		//added compiler '-Dsim_jack' to avoid jack.h and libjack usage
//---- v1.03 		27-march-2018	//now using this to toggle jack inclusion, in globals.h: #define include_jack,
									//cleaned up callback mechanism, using 'st_jack_tag',
									//now handles more that 2 channels channels,
//---- v1.04		27-dec-2018		//added pulseaudio


#include "synth_eng.h"



#ifdef include_jack



//First start jackserver with: jackd -d alsa [-r 44100]
//Remember the Jack server's output ports are soundcard capture inputs, e.g. the Mic.
//The Mic is a source that outputs data from the server, we connect our client's input
//to this server's output.
//Likewise our client's output is fed into a Jack server's input, e.g. the
//soundcard's speakers are fed from Jack server inputs.



/*
// EXAMPLE CODE ------------------------



//NOTE: bufid is not used by jack, just for compatability with pcaudio.cpp
int cb_audio_process_in( unsigned int bufid, unsigned int frames, void *arg )
{
float f0, f1, f2;

st_jack_tag* sj = (st_jack_tag*)arg;

printf("in() - user arg: %d\n", (int) sj->user_arg );

for( int i = 0; i < frames; i++ )
	{
	f0 = joutbuf[ 0 ][ i ];
	f1 = joutbuf[ 1 ][ i ];
	f2 = joutbuf[ 2 ][ i ];
	}
}


//NOTE: bufid is not used by jack, just for compatability with pcaudio.cpp
int cb_audio_process_out( unsigned int bufid, unsigned int frames, void *arg )
{
st_jack_tag* sj = (st_jack_tag*)arg;

printf("out() - user arg: %d\n", (int) sj->user_arg );
for( int i = 0; i < frames; i++ )
	{
	joutbuf[ 0 ][ i ] = (float)( RAND_MAX / 2 - rand() ) / (float)( RAND_MAX / 2 ) * 0.5;
	joutbuf[ 1 ][ i ] = (float)( RAND_MAX / 2 - rand() ) / (float)( RAND_MAX / 2 ) * 0.3;
	joutbuf[ 2 ][ i ] = (float)( RAND_MAX / 2 - rand() ) / (float)( RAND_MAX / 2 ) * 0.1;
	}
}





//load up a struct with jack vars, this will be sent to callback
st_jack.cjack_obj = &myjack;
st_jack.inbuf[ 0 ] = &jinbuf[ 0 ];						//one for each chan
st_jack.inbuf[ 1 ] = &jinbuf[ 1 ];
st_jack.inbuf[ 2 ] = &jinbuf[ 2 ];
st_jack.outbuf[ 0 ] = &joutbuf[ 0 ];					//one for each chan
st_jack.outbuf[ 1 ] = &joutbuf[ 1 ];
st_jack.outbuf[ 2 ] = &joutbuf[ 2 ];

st_jack.cb_ptr_process_in = cb_audio_process_in;		//user audio sample process callback, MUST SET to zero if no callback to occur
st_jack.cb_ptr_process_out = cb_audio_process_out;		//user audio sample process callback
st_jack.user_arg = (void*)1234567;						//any (void*) can go here

if( !myjack.start_jack( "MyClient", 3, 3, framecnt, &st_jack ) )
	{
	}

---------------------------------------

*/







float jack_two_pi = 2.0 * M_PI;


//sample buffer processor callback, calls callback in: st_jack_tag

int cb_cjack_local_process( jack_nframes_t nframes, void *arg_in )
{
//printf("cb_myjack_process() - jack_was_here\n" );

st_jack_tag *sj = (st_jack_tag*) arg_in;

cjack *p = (cjack*) sj->cjack_obj;


//grab jack buffer ptrs
for( int i = 0; i < sj->ch_in ; i++ )
	{
	*(sj->inbuf[ i ]) = (sample_t*) jack_port_get_buffer( p->inport[ i ], nframes );
	}

for( int i = 0; i < sj->ch_out ; i++ )
	{
	*(sj->outbuf[ i ]) = (sample_t*) jack_port_get_buffer( p->outport[ i ], nframes );
	}

//if( p->chin > 0 ) *(sj->inbuf[ 0 ]) = (sample_t*) jack_port_get_buffer( p->inport[ 0 ], nframes );
//if( p->chin == 2 ) *(sj->inbuf[ 1 ]) = (sample_t*) jack_port_get_buffer( p->inport[ 1 ], nframes );
//if( p->chout > 0 ) *(sj->outbuf[ 0 ]) = (sample_t*) jack_port_get_buffer( p->outport[ 0 ], nframes );			//data to audio o/p left
//if( p->chout == 2 ) *(sj->outbuf[ 1 ]) = (sample_t*) jack_port_get_buffer( p->outport[ 1 ], nframes );		//data to audio o/p left







//need tone/noise overrides?
bool need_tone_in = 0;
bool need_tone_out = 0;
for ( int i = 0; i < sj->ch_in; i++ )
	{
	if( sj->tone_in[ i ].tone_wht != 0 )
		{
		need_tone_in = 1;
		break;
		}
	}


for ( int i = 0; i < sj->ch_out; i++ )
	{
	if( sj->tone_out[ i ].tone_wht != 0 )
		{
		need_tone_out = 1;
		break;
		}
	}






float ff;

//1st, insert tone/white noise on req input channels
if( need_tone_in )
	{
	for ( int i = 0; i < nframes; i++ )
		{
		for ( int j = 0; j < sj->ch_in; j++ )					//cycle in channels
			{
			if( sj->tone_in[ j ].tone_wht == 1 )								//tone?
				{
				ff = sj->tone_in[ j ].ampl * sin( sj->tone_in[ j ].theta );

				( *(sj->inbuf[ j ]) )[ i ] = ff * sj->tone_in[ j ].ampl;		//place sample

				sj->tone_in[ j ].theta += sj->tone_in[ j ].theta_inc;
				if( sj->tone_in[ j ].theta >= jack_two_pi ) sj->tone_in[ j ].theta -= jack_two_pi;
				}
			else{
				if( sj->tone_in[ j ].tone_wht == 2 )							//white noise?
					{
					ff = (float)( RAND_MAX / 2 - rand() ) / (float)( RAND_MAX / 2 );
					( *(sj->inbuf[ j ]) )[ i ] = ff * sj->tone_in[ j ].ampl;	//place sample
					}
				}
			}
		}
	}

//2nd, let user access input samples
if( sj->ch_in != 0 )
	{
	if( sj->cb_ptr_process_in != 0 ) sj->cb_ptr_process_in( 0, nframes, arg_in );			//call user audio callback for input
	}










//1st let user load output samples, then do tone override req
if( sj->ch_out != 0 )
	{
	if( sj->cb_ptr_process_out != 0 ) sj->cb_ptr_process_out( 0, nframes, arg_in );			//call user audio callback for output 
	}



//2nd, insert tone/white noise on req output channels
if( need_tone_out )
	{
	for ( int i = 0; i < nframes; i++ )
		{
		for ( int j = 0; j < sj->ch_out; j++ )					//cycle out channels
			{
			if( sj->tone_out[ j ].tone_wht == 1 )								//tone?
				{
				ff = sj->tone_out[ j ].ampl * sin( sj->tone_out[ j ].theta );
//				printf("j: %d\n", j);
				( *(sj->outbuf[ j ]) )[ i ] = ff;								//place sample

				sj->tone_out[ j ].theta += sj->tone_out[ j ].theta_inc;
				if( sj->tone_out[ j ].theta >= jack_two_pi ) sj->tone_out[ j ].theta -= jack_two_pi;
				}
			else{
				if( sj->tone_out[ j ].tone_wht == 2 )							//white noise?
					{
					ff =  (float)( RAND_MAX / 2 - rand() ) / (float)( RAND_MAX / 2 );
					( *(sj->outbuf[ j ]) )[ i ] = ff * sj->tone_out[ j ].ampl;	//place sample
					}
				}
			}
		}
	}


return 0;
}





void cb_jerror( const char *desc )
{
fprintf( stderr, "cb_jerror() - JACK error: %s\n", desc );
}








int cb_srate_change_callback( jack_nframes_t nframes, void *arg_in )
{
st_jack_tag *st_args = (st_jack_tag*) arg_in;

cjack *p = (cjack*) st_args->cjack_obj;

int sr = p->get_srate();
printf ("cb_srate_change_callback() - srate= %d\n", nframes );

return 1;
}










void cb_jack_shutdown ( void *arg_in )
{
st_jack_tag *st_args = (st_jack_tag*) arg_in;

cjack *p = (cjack*) st_args->cjack_obj;

fprintf( stderr, "jack_shutdown() was called\n" );
}






//tone_wht:  0 = off, 1 = tone, 2 = white noise
bool cjack::set_tone_in( unsigned int tone_wht, unsigned int channel, double freq, double ampl, st_jack_tag *sj )
{
if ( channel >= max_channels ) return 0;

sj->tone_in[ channel ].tone_wht = tone_wht;
sj->tone_in[ channel ].freq = freq;
sj->tone_in[ channel ].ampl = ampl;
sj->tone_in[ channel ].theta = 0;
sj->tone_in[ channel ].theta_inc = freq * jack_two_pi / jack_srate;

}




//tone_wht:  0 = off, 1 = tone, 2 = white noise
bool cjack::set_tone_out( unsigned int tone_wht, unsigned int channel, double freq, double ampl, st_jack_tag *sj )
{
if ( channel >= max_channels ) return 0;

sj->tone_out[ channel ].tone_wht = tone_wht;
sj->tone_out[ channel ].freq = freq;
sj->tone_out[ channel ].ampl = ampl;
sj->tone_out[ channel ].theta = 0;
sj->tone_out[ channel ].theta_inc = freq * jack_two_pi / jack_srate;
}





//#ifdef sim_jack

//bool cjack::set_srate( int in_srate )
//{
//if( in_srate < 8000 ) return 0;

//srate = in_srate;
//return 1;
//}
//#endif




int cjack::get_srate()
{
if( client == 0 )
	{
	fprintf( stderr, "get_srate() - no client\n" );
	return 0;
	}

jack_srate = jack_get_sample_rate( client );				//get current sample rate

return jack_srate;
}





//these are to help compile without jack
//#ifdef sim_jack

//typedef unsigned int jack_nframes_t
//typedef double sample_t;

//sample_t *jack_port_get_buffer( void, jack_nframes_t )
//{

//}

//#endif





cjack::cjack()
{
//srate = sample_rate;						//assume
cb_ptr_process = 0;
jack_two_pi = 2.0 * M_PI;


//#ifdef sim_jack
//thrd1 = 0;
//#endif


//started = 0;
calls = 0;
jack_srate = 0;




}








cjack::~cjack()
{
cb_ptr_process = 0;

//#ifdef sim_jack
//thrd_stop();
//#endif

}






//e.g: see example at top,   myjack.start_jack( "MyClient", 3, 3, 0, framecnt, &st_jack );

bool cjack::start_jack( string client_name,  unsigned int channels_in, unsigned int channels_out, bool auto_conn, unsigned int bufsize, st_jack_tag *sj )
{
string s1;

client = 0;


sj->cjack_obj = this;						//user probably did this, but make sure its there

const char **ports;
jack_status_t status;

usr_buf_size = bufsize;

buf_dur = 1;


pr0 = 0;
pw0 = 1;
pr1 = 0;
pw1 = 1;

if( channels_in > max_channels )
	{
	printf("start_jack() - too many 'in' channels specified, limit to %d, requested: %d\n", max_channels, channels_in );
	channels_in = max_channels;
	}
if( channels_out > max_channels )
	{
	printf("start_jack() - too many 'out' channels specified, limit to %d, requested: %d\n", max_channels, channels_out );
	channels_out = max_channels;
	}

sj->ch_in = channels_in;
sj->ch_out = channels_out;



void (cjack::*pt2Member)( const char *desc ) = NULL;




jack_set_error_function ( cb_jerror );					//error callback


if ( ( client = jack_client_open( client_name.c_str(), JackNullOption, &status, NULL) ) == 0)
	{
    fprintf( stderr, "cjack::start_jack() - jack server not running?\n" );
    return 0;
	}




jack_set_process_callback( client, cb_cjack_local_process, (void*)sj );				//sample buffer processing callback (local)
  
jack_set_sample_rate_callback( client, cb_srate_change_callback, (void*)sj );		//sample rate has changed callback

jack_on_shutdown( client, cb_jack_shutdown, (void*)sj );							//server has been shutdown callback
  


for( int i = 0; i < sj->ch_in; i++ )
	{
	strpf( s1, "in_%d", i );
	inport[ i ] = jack_port_register( client, s1.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0 );

	if ( inport[ i ] == NULL )
		{
		fprintf(stderr, "cjack::start_jack() - Unable to register Jack inport %d\n", i );
		exit( 1 );
		}
	}

for( int i = 0; i < sj->ch_out; i++ )
	{
	strpf( s1, "out_%d", i );
	outport[ i ] = jack_port_register( client, s1.c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );

	if ( outport[ i ] == NULL )
		{
		fprintf(stderr, "cjack::start_jack() - Unable to register Jack outport %d\n", i );
		exit( 1 );
		}
	}



jack_srate = jack_get_sample_rate( client );				//get current sample rate

jack_set_buffer_size( client, usr_buf_size );		

jack_buf_size = jack_get_buffer_size( client );


for ( int i = 0; i < max_channels; i++ )
	{
	sj->tone_in[ i ].tone_wht = 0;
	sj->tone_out[ i ].tone_wht = 0;
	}

//set_tone_in( 1, 0, 400, 0.2, sj );
//set_tone_out( 2, 2, 400, 0.2, sj );

 
//tell the JACK server that we are ready to roll
if ( jack_activate( client ) )
	{
    fprintf( stderr, "cjack::start_jack() - Cannot activate client\n" );
    return 1;
	}





if( auto_conn )
	{
	//get the available outports from Jack, note for output you use JackPortIsInput !!!
	ports = jack_get_ports( client, NULL, NULL, JackPortIsPhysical | JackPortIsInput );
	if ( ports == NULL )
		{
		fprintf(stderr, "cjack::start_jack() - Cannot find any physical playback ports\n");
		exit(1);
		}

	//connect our outport to a Jack inport
	if( sj->ch_out >= 1 )
		{
		if( ports[ 0 ] != NULL )
			{
			printf( "cjack::start_jack() - Found Jack outport: '%s'\n", ports[ 0 ]);
			if ( jack_connect( client, jack_port_name ( outport[ 0 ] ), ports[ 0 ] ) )
				{
				fprintf ( stderr, "cjack::start_jack() - **** Cannot connect this client's outport: '%s', to Jack's inport: '%s' ****\n\n", jack_port_name ( outport[ 0 ] ) ,ports[ 0 ] );
				}
			else{
				fprintf ( stderr, "cjack::start_jack() - Connected this client's outport: '%s', to Jack's inport: '%s'\n", jack_port_name ( outport[ 0 ] ) ,ports[ 0 ] );
				}
			}
		else{
			jack_failed();
			}
		}

	//connect our outport to a Jack inport
	if( sj->ch_out >= 2 )
		{
		if( ports[ 1 ] != NULL )
			{
			printf( "cjack::start_jack() - Found Jack outport: '%s'\n", ports[ 1 ]);
			if ( jack_connect( client, jack_port_name ( outport[ 1 ] ), ports[ 1 ] ) )
				{
				fprintf ( stderr, "cjack::start_jack() - **** Cannot connect this client's outport: '%s', to Jack's inport: '%s' ****\n\n", jack_port_name ( outport[ 1 ] ) ,ports[ 1 ] );
				}
			else{
				fprintf ( stderr, "cjack::start_jack() - Connected this client's outport: '%s', to Jack's inport: '%s'\n", jack_port_name ( outport[ 1 ] ) ,ports[ 1 ] );
				}
			}
		else{
			jack_failed();
			}
		}

	free( ports );
	 





	//get the available inports from Jack, note for input you use JackPortIsOutput !!!
	ports = jack_get_ports ( client, NULL, NULL, JackPortIsPhysical | JackPortIsOutput);
	if ( ports == NULL )
		{
		fprintf(stderr, "cjack::start_jack() - No physical capture ports\n");
		exit (1);
		}

	//connect our inport to a Jack outport
	if( sj->ch_in >= 1 )
		{
		if( ports[ 0 ] != NULL )
			{
			printf( "cjack::start_jack() - Found Jack inport: '%s'\n", ports[ 0 ]);
			if ( jack_connect( client, ports[ 0 ], jack_port_name ( inport[ 0 ] ) ) )
				{
				fprintf ( stderr, "cjack::start_jack() - **** Cannot connect this client's inport: '%s', to Jack's outport: '%s' ****\n\n", jack_port_name ( inport[ 0 ] ) ,ports[ 0 ] );
				}
			else{
				fprintf ( stderr, "cjack::start_jack() - Connected this client's inport: '%s', to Jack's outport: '%s'\n", jack_port_name ( inport[ 0 ] ) ,ports[ 0 ] );
				}
			}
		else{
			jack_failed();
			}
		}

	//connect our inport to a Jack outport
	if( sj->ch_in >= 2 )
		{
		if( ports[ 1 ] != NULL )
			{
			printf( "cjack::start_jack() - Found Jack inport: '%s'\n", ports[ 1 ]);
			if ( jack_connect( client, ports[ 1 ], jack_port_name ( inport[ 1 ] ) ) )
				{
				fprintf ( stderr, "cjack::start_jack() - **** Cannot connect this client's inport: '%s', to Jack's outport: '%s' ****\n\n", jack_port_name ( inport[ 1 ] ) ,ports[ 1 ] );
				}
			else{
				fprintf ( stderr, "cjack::start_jack() - Connected this client's inport: '%s', to Jack's outport: '%s'\n", jack_port_name ( inport[ 1 ] ) ,ports[ 1 ] );
				}
			}
		else{
			jack_failed();
			}
		}

	free( ports );
	}




printf( "cjack::start_jack() - Engine sample rate: %d\n", (int)jack_srate );
printf( "cjack::start_jack() - JackBufSize: %d\n", jack_buf_size );
//printf( "BufSize: %d\n", buf_size );
//getc( stdin );

//#endif

return 1;
}














bool cjack::stop_jack()
{

if( client == 0 )
	{
	fprintf( stderr, "stop_jack() - no client\n" );
	return 0;
	}

//#ifdef sim_jack
//thrd_stop();
//free_allocated_memory( outport0 );
//free_allocated_memory( outport1 );
//free_allocated_memory( inport0 );
//free_allocated_memory( inport1 );

//#endif


//#ifndef sim_jack

fprintf( stderr, "stop_jack()" );

jack_client_close( client );

client = 0;
//#endif

return 1;
}






/*

bool cjack::set_cb_process( int (*cb_ptr_process_in)( jack_nframes_t nframes, void *arg ) )
{

//tell the JACK server that we are ready to roll
if ( jack_deactivate( client ) )
	{
    fprintf( stderr, "Cannot deactivate client\n" );
    return 1;
	}

if ( cb_ptr_process_in == 0 )
	{
	fprintf( stderr, "set_cb_process() - function ptr was zero, ignoring\n" );
	return 0;
	}

cb_ptr_process = cb_ptr_process_in;

if (jack_set_process_callback( client, cb_ptr_process, this ) != 0 )		//sample buffer processing callback
	{
	fprintf( stderr, "set_cb_process() - function ptr was not changed\n" );
	}
return 1;
}

*/




/*
//only used when jack is being simulated
void cjack::free_allocated_memory( sample_t* buf )
{

if( buf )
	{
	free( buf );
	fprintf( stderr, "free_allocate_memory() - freeing memory buf\n" );
	buf = 0;
	}

//if( buf1 )
//	{
//	free( buf1 );
//	fprintf( stderr, "free_allocate_memory() - freeing memory buf1\n" );
//	buf1 = 0;
//	}

}

*/









/*
//only used when jack is being simulated sim_jack
sample_t* cjack::allocate_memory( unsigned int bufsiz )
{
sample_t* buf = 0;

if ( bufsiz == 0 ) return 0;


//unsigned long buf_size = buf_dur * srate;

//unsigned long buf_size_bytes = buf_size * sizeof( sample_t );


buf = (sample_t *) malloc( bufsiz * sizeof( sample_t ) );
	
if( buf == 0)
	{
	fprintf( stderr, "allocate_memory() - no buf allocated\n" );
	return 0;
	}
fprintf( stderr, "allocate_memory() - buf allocated %u bytes\n", bufsiz );

return buf;


}
*/





/*
void cjack::clear_buffer( sample_t* buf )
{

if( buf ) memset ( buf, 0, buf_size );

}

*/










void cjack::jack_failed()
{
//#ifndef sim_jack

fprintf( stderr, "jack_failed()\n");

jack_client_close( client );

//free_allocated_memory();

exit( 0 );
//#endif

}






/*
//#ifdef sim_jack


//a gcthrd thread callback
void jackthrd_cb( void* args )
{
mystr m1;

string nm;
int count = 0;

//printf("callback1_1\n");

cjack *o = (cjack*) args;

strpf( nm, "%s -", o->thrd1->obj_name.c_str() );

//if( o->logr ) o->thrd1->logr->pf( o->pr, "%s thrd started\n", nm.c_str(), count );
//if( o->logr ) o->thrd1->logr->pf( o->pr, "%s cb() - callback\n",  nm.c_str() );


m1.time_start( m1.ns_tim_start );

while( 1 )
	{
	if( o->thrd1->thrd.kill ) goto finthread;


//	timespec ts, tsret;			//don't hog processor in this while()
//	ts.tv_sec = 0;
//	ts.tv_nsec = 1;				//4294967269 
//	nanosleep( &ts, &tsret );
	
	double dt = m1.time_passed( m1.ns_tim_start );
	double frame_time = 1.0 / (double)o->srate * o->usr_buf_size;
	if( dt >= frame_time )
		{
		m1.time_start( m1.ns_tim_start );
//		if( o->thrd1->thrd_dbg ) printf( "%s cb() -  hello %05d\n", nm.c_str(), count );

		//call user's callback, defined in start_jack()

//linux code
#ifndef compile_for_windows  // windows' MCICallbackWaveOut() will calls this

//#ifdef sim_jack
printf( "jackthrd_cb\n" );
//		if( o->cb_ptr_process ) o->cb_ptr_process( (unsigned int) o->chout, (unsigned int) o->usr_buf_size, args );
//#endif

#endif

//		if( o->logr ) o->logr->pf( o->pr, "%s cb() - hello %05d\n", nm.c_str(), count );
		}
	
	m1.delay_ms( 1 );			//don't hog processor in this while()
	count++;
	}

finthread:
o->thrd1->thrd.finished = 1;
o->thrd1->thrd.kill = 0;

if( o->thrd1->thrd_dbg ) printf( "%s thrd finished\n", nm.c_str() );

//if( o->thrd1->logr ) o->thrd1->logr->pf( o->pr, "%s thrd finished\n", nm.c_str() );
}

*/










/*
void cjack::thrd_start()
{
mystr m1;


if ( my_verbose ) printf( "cjack::thrd_start() - calledzzz\n" );

if( thrd1 == 0 ) 
	{
	thrd1 = new gcthrd();

//	thrd1->set_log_ptr( 5, logr );			//set thrd to use a prev created log obj
	thrd1->thrd_dbg = 1;
	thrd1->set_name( "jackthrd1" );
	thrd1->set_thrd_callback( jackthrd_cb, (void*)this );		//set thread callback
	}

thrd1->create_thread( );

}
*/



/*
void cjack::thrd_stop()
{

if( thrd1 != 0 )
	{
	thrd1->destroy_thread( );
	if( thrd1->wait_till_thread_destroyed( 1000 ) )
		{
		delete thrd1;
		thrd1 = 0;
		}
	}
}
*/
//#endif



#endif












#ifdef include_pulseaudio

#endif
