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

//gcthrd.cpp
//v1.01		07-01-12


#include "gcthrd.h"





//=------------------------------------------------------------------------------
gcthrd::gcthrd( )
{
thrd_dbg = 0;

thrd.finished = 1;
thrd.kill = 0;

logr = 0;

p_callback = 0;
use_mutex = 0;

}





//create mutex obj - general purpose mutex
bool gcthrd::create_mutex( )
{

if( use_mutex ) return 0;

#ifndef compile_for_windows
//used to use this but it won't compile within a class: mutex_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_init( &mutex1, NULL );
#endif


//windows code
#ifdef compile_for_windows
h_mutex1 = CreateMutex( NULL, FALSE, NULL );  // make a win mutex obj, not signalled ( not locked)
#endif

use_mutex = 1;
return 1;
}










//destroy mutex obj - general purpose mutex 
bool gcthrd::destroy_mutex( )
{
if( !use_mutex ) return 0;

#ifndef compile_for_windows
pthread_mutex_destroy( &mutex1 );
#endif


//windows code
#ifdef compile_for_windows
CloseHandle( h_mutex1 );							//close mutex obj
#endif

use_mutex = 0;
return 1;
}












//lock a mutex obj - general purpose mutex 
bool gcthrd::lock_mutex( )
{

if( ! use_mutex ) return 0;

//linux code
#ifndef compile_for_windows
pthread_mutex_lock( &mutex1 );			//block other thread to gain mutually exclusive access to sglobal
#endif

//windows code
#ifdef compile_for_windows
WaitForSingleObject( h_mutex1, INFINITE );
#endif

return 1;
}









//unlock a mutex obj - general purpose mutex 
bool gcthrd::unlock_mutex( )
{
if( !use_mutex ) return 0;

//linux code
#ifndef compile_for_windows
pthread_mutex_unlock( &mutex1 );		//release mutex
#endif


//windows code
#ifdef compile_for_windows
ReleaseMutex( h_mutex1 );
#endif

return 1;
}










gcthrd::~gcthrd( )
{
mystr m1;
thrd.kill = 1;
string nm;
	
strpf( nm, "%s -", obj_name.c_str() );
if( thrd_dbg ) printf("%s ~gcthrd() - called\n", nm.c_str() );

for( int i = 0; i < 10; i ++ )
	{
	if( thrd.finished == 1 ) break;
	
	m1.delay_ms( 100 );
	}

destroy_mutex();
}





void gcthrd::set_name( string s )
{
obj_name = s;
}






void gcthrd::set_log_ptr( int priority_in, gclog *logr_in )
{
pr = priority_in;
logr = logr_in;
}







void gcthrd::set_thrd_callback( void (*p_cb)( void* ), void *args_in )
{
p_callback = p_cb;
args = args_in;
}







bool gcthrd::create_thread( )
{

if( !thrd.finished )
	{
	string nm;
	
	strpf( nm, "%s -", obj_name.c_str() );

	if( thrd_dbg ) printf("%s create_thread() - thread still running\n", nm.c_str() );
	return 0;
	}

thrd.kill = 0;
thrd.finished = 1;

//linux code
#ifndef compile_for_windows
pthread_create( &thread_id, NULL, &thread_linux, this );		//start thread
#endif


//windows code
//start thread
#ifdef compile_for_windows
CreateThread( NULL, 0, 		// default values
&thread_win, 				// function to use
this, 							// data to pass to the function
0, NULL); 					// Start thread, we don't need a thread ID
#endif

return 1;
}







bool gcthrd::destroy_thread( )
{
string nm;


strpf( nm, "%s -", obj_name.c_str() );

if( thrd_dbg ) printf("%s destroy_thread() has been started\n", nm.c_str() );

if( thrd.finished )
	{
	if( thrd_dbg ) printf("%s destroy_thread() - thread not running\n", nm.c_str() );
	if( logr ) logr->pf( pr, "%s destroy_thread() - thread not running\n", nm.c_str() );
	return 0;
	}


if( thrd_dbg ) printf("%s destroy_thread() has been finished ok\n", nm.c_str() );

p_callback = 0;
thrd.kill = 1;	
return 1;

}








//wait till thread has completed, returns 0 if timeout occurred
bool gcthrd::wait_till_thread_destroyed( int timeout_ms )
{
mystr m1;
string nm;

strpf( nm, "%s -", obj_name.c_str() );

if( thrd_dbg ) printf("%s wait_till_thread_destroyed() has been started\n", nm.c_str() );


m1.time_start( m1.ns_tim_start );
while( 1 )
	{
	if( thrd.finished ) goto fin_ok;					//finished within timeout period?

	double dt = m1.time_passed( m1.ns_tim_start );
	if( dt > ( (double)timeout_ms * 0.001 ) )			//passed timeoput period?
		{

		if( thrd_dbg ) printf("%s wait_till_thread_destroyed() - timeout\n", nm.c_str() );
		if( logr ) logr->pf( pr, "%s wait_till_thread_destroyed() - timeout\n", nm.c_str() );

		return 0;
		}

	}

fin_ok:
if( thrd_dbg ) printf("%s wait_till_thread_destroyed() has been finished ok\n", nm.c_str() );
return 1;
}














//linux code
#ifndef compile_for_windows

//only use mutex locked logpf_thrd() calls
//---------------------- thread -------------------------
void* gcthrd::thread_linux( void* arg )
{
//mystr m1;

//string nm;
//int count = 0;


gcthrd* o = (gcthrd*) arg;

o->thrd.finished = 0;

//strpf( nm, "%s -", o->name.c_str() );

//if( o->thrd_dbg ) printf( "%s thrd started\n",nm.c_str()  );
//if( o->logr ) o->logr->pf( o->pr, "%s thrd started\n", nm.c_str(), count );

//m1.time_start( m1.ns_tim_start );

if( o->p_callback) o->p_callback( o->args );

o->thrd.finished = 1;

/*
while( 1 )
	{
	if( o->thrd.kill ) goto finthread;


	timespec ts, tsret;			//don't hog processor in this while()
	ts.tv_sec = 0;
	ts.tv_nsec = 1;				//4294967269 
	nanosleep( &ts, &tsret );
	
	double dt = m1.time_passed( m1.ns_tim_start );
	if( dt > 0.25 )
		{
		m1.time_start( m1.ns_tim_start );
		if( o->thrd_dbg ) printf( "%s thrd: hello %05d\n", nm.c_str(), count );

		if( o->logr ) o->logr->pf( o->pr, "%s thrd: hello %05d\n", nm.c_str(), count );
		}
	count++;
	}

finthread:
o->thrd.finished = 1;
o->thrd.kill = 0;

if( o->thrd_dbg ) printf( "%s thrd: finished\n", nm.c_str() );

if( o->logr ) o->logr->pf( o->pr, "%s thrd: finished\n", nm.c_str() );
*/

return (void*) 1;	
}
//-------------------------------------------------------

#endif






//windows code
#ifdef compile_for_windows
//---------------------- thread -------------------------
DWORD WINAPI gcthrd::thread_win( void* arg )
{
//mystr m1;

//string nm;
//int count = 0;


gcthrd* o = (gcthrd*) arg;

o->thrd.finished = 0;

//strpf( nm, "%s -", o->name.c_str() );

//if( o->thrd_dbg ) printf( "%s thrd started\n",nm.c_str()  );
//if( o->logr ) o->logr->pf( o->pr, "%s thrd started\n", nm.c_str(), count );

//m1.time_start( m1.ns_tim_start );

if( o->p_callback) o->p_callback( o->args );

/*
while( 1 )
	{
	if( o->thrd.kill ) goto finthread;


	timespec ts, tsret;			//don't hog processor in this while()
	ts.tv_sec = 0;
	ts.tv_nsec = 1;				//4294967269 
	nanosleep( &ts, &tsret );
	
	double dt = m1.time_passed( m1.ns_tim_start );
	if( dt > 0.25 )
		{
		m1.time_start( m1.ns_tim_start );
		if( o->thrd_dbg ) printf( "%s thrd: hello %05d\n", nm.c_str(), count );

		if( o->logr ) o->logr->pf( o->pr, "%s thrd: hello %05d\n", nm.c_str(), count );
		}
	count++;
	}

finthread:
o->thrd.finished = 1;
o->thrd.kill = 0;

if( o->thrd_dbg ) printf( "%s thrd: finished\n", nm.c_str() );

if( o->logr ) o->logr->pf( o->pr, "%s thrd: finished\n", nm.c_str() );
*/


return 1;

}
#endif
//-------------------------------------------------------


