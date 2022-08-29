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


//gclog.cpp

//	v1.01 16-12-11



#include "gclog.h"





gclog::gclog( )
{
disabled = 0;
dbg_log = 1;
use_mutex = 0;
log_culled = 0;

log_swap_fname = "";

show_datetime = 1;
to_callback = 0;
to_file = 1;
to_printf = 1;

file_log_lvl = 5;				//set default mid range log level, 1 is highest 'priority'
callback_log_lvl = 5;
printf_log_lvl = 5;


p_callback = 0;


//fsize_low_water_mark = 1024 * 1024;
//fsize_low_water_mark *= 10;								//set a min 10MB filesize limit

//fsize_high_water_mark = 1024 * 1024 * 1024;
//fsize_high_water_mark *= 11;							//set a max 11MB filesize limit

}







gclog::~gclog( )
{

destroy_mutexes();

}











//create mutex objs
bool gclog::create_mutexes( )
{

if( use_mutex ) return 0;

#ifndef compile_for_windows
//used to use this but it won't complile within a class: mutex_log = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_init( &mutex_pf, NULL );
pthread_mutex_init( &mutex_vlog, NULL );
#endif


//windows code
#ifdef compile_for_windows
h_mutex_pf = CreateMutex( NULL, FALSE, NULL );  // make a win mutex obj, not signalled ( not locked)
h_mutex_vlog = CreateMutex( NULL, FALSE, NULL );  // make a win mutex obj, not signalled ( not locked)
#endif

use_mutex = 1;
return 1;
}








//destroy mutex objs
bool gclog::destroy_mutexes( )
{
if( !use_mutex ) return 0;

#ifndef compile_for_windows
pthread_mutex_destroy( &mutex_pf );
pthread_mutex_destroy( &mutex_vlog );
#endif


//windows code
#ifdef compile_for_windows
CloseHandle( h_mutex_pf );							//close mutex obj
CloseHandle( h_mutex_vlog );							//close mutex obj
#endif

use_mutex = 0;
return 1;
}








//lock a mutex obj
bool gclog::lock_mutex_pf( )
{


if( ! use_mutex ) return 0;

//linux code
#ifndef compile_for_windows
pthread_mutex_lock( &mutex_pf );			//block other thread to gain mutually exclusive access to sglobal
#endif

//windows code
#ifdef compile_for_windows
WaitForSingleObject( h_mutex_pf, INFINITE );
#endif

return 1;
}








//lock a mutex obj
bool gclog::lock_mutex_vlog( )
{


if( ! use_mutex ) return 0;

//linux code
#ifndef compile_for_windows
pthread_mutex_lock( &mutex_vlog );			//block other thread to gain mutually exclusive access to sglobal
#endif

//windows code
#ifdef compile_for_windows
WaitForSingleObject( h_mutex_vlog, INFINITE );
#endif

return 1;
}









//lock a mutex obj
bool gclog::unlock_mutex_pf( )
{
if( !use_mutex ) return 0;

//linux code
#ifndef compile_for_windows
pthread_mutex_unlock( &mutex_pf );		//release mutex
#endif


//windows code
#ifdef compile_for_windows
ReleaseMutex( h_mutex_pf );
#endif

return 1;
}













//lock a mutex obj
bool gclog::unlock_mutex_vlog( )
{
if( !use_mutex ) return 0;

//linux code
#ifndef compile_for_windows
pthread_mutex_unlock( &mutex_vlog );		//release mutex
#endif


//windows code
#ifdef compile_for_windows
ReleaseMutex( h_mutex_vlog );
#endif

return 1;
}









void gclog::fixed_string( string s )
{
fixed_str = s;
}




void gclog::set_fname( string fn )
{
log_fname = fn;
}





//note: this callback is not thread safe and does not usea mutex
void gclog::set_log_callback( void (*p_cb)( string ) )
{
p_callback = p_cb;

}





void gclog::clear_log_file( )
{
mystr m1;

if( disabled ) return;

if( log_fname.length() != 0 )
	{
	FILE *fp = m1.mbc_fopen( log_fname, "wb" );				 //create a file
	if( fp == 0 )
		{
		if( dbg_log ) fprintf( stdout, "Can't create log file: %s\n", log_fname.c_str() );
		return;
		}
	fclose( fp );
	}
}










//allow max log size to be specified and name of swap file to allow ripple of log entries
bool gclog::set_max_log_size( string swap_fname, unsigned long long int high_size, unsigned long long int low_size )
{
mystr m1;

log_swap_fname = "";

if(low_size >= high_size ) return 0;


//check swap file can be created
FILE *fp = m1.mbc_fopen( swap_fname, "wb" );				 //create swap file
if( fp == 0 )
	{
	return 0;
	}

fclose( fp );

remove( swap_fname.c_str() );

fsize_low_water_mark = low_size;
fsize_high_water_mark = high_size;

log_swap_fname = swap_fname;

return 1;
}














//destructive test to see if log is culling older entries and still staying within
//bounding 'watermarks' using a swap file
void gclog::destructive_stress_test_of_log_file( string fname, string swap_fname )
{

clear_log_file();

mystr m1;

log_fname = fname;

//preload a log file with 100000 bytes
FILE *fp = m1.mbc_fopen( log_fname, "wb" );		//open log file
int count = 0;

if( fp == 0 )
	{
	printf( "\nFailed to create log file: '%s'\n", log_fname.c_str() );
	return;
	}

for ( int i = 0; i < 10; i++ )
	{
	for ( int j = 0; j < 1000; j++ )
		{
		fprintf( fp, "%05d 789\n", count );
		count++;
		}
	}
fclose( fp );



//set some file limit watermarks
if( set_max_log_size( swap_fname, 100000, 99900 ) )
	{
	printf("\nFailed to create log swap file\n");
	}

//push past high watermark, to cause culling of older entries at begining of file
pf( 5, "Hello\n" );


// keep testing log culling further times
for ( int i = 0; i < 100000; i++ )
	{
	pf( 5, "Hello %d\n", i );
	}
	

}








//out to either or both console and log file and callback, cull old 
//log entries at head of log file if req, using a swap file
void gclog::outlog( string s, string fname )
{
string s1, stime, snew;
mystr m1 = s;

if( disabled ) return;


struct tm tn;
get_time_now( tn );
make_time_str( tn, stime );




m1.FindReplace( s1, "\r", "",0);		//remove any occurrances of a CR

if( show_datetime ) snew = stime + " ";
snew += fixed_str;

snew += s1;


//store in internal vector also
lock_mutex_vlog( );
vlog.push_back( snew );
unlock_mutex_vlog( );


if( to_printf )
	{
	if ( ( log_lvl <= printf_log_lvl ) && ( printf_log_lvl > 0 ) ) printf( "%s", snew.c_str() );

	}


if( to_callback )
	{
	if ( p_callback != 0 )
		{
		if ( ( log_lvl <= callback_log_lvl ) && ( callback_log_lvl > 0 ) )  p_callback( snew.c_str() );
		}
	}


if( to_file )
	{
	bool pause = 0;

	//get log file size
	int status;
	struct stat statbuf;
	statbuf.st_size = 0ULL;
	unsigned long long int byt_tot;
	status = stat( (char*)fname.c_str(), &statbuf );
	if( status != -1 )
		{
		byt_tot = (unsigned long long int) statbuf.st_size;
//		printf( "Fsize=%I64u", byt_tot );

		//passed the large size marker
		if ( log_swap_fname.length() != 0 )								//swap file specified
			{
			if( byt_tot >= fsize_high_water_mark )
				{

				FILE *fpswap = m1.mbc_fopen( log_swap_fname, "wb" );		 //create a swap file

				if( fpswap != 0 )
					{
					FILE *fp = m1.mbc_fopen( fname, "rb" );		//open log file

					if( fp != 0 )
						{
						unsigned long long int shorten = byt_tot - fsize_low_water_mark;
						fseek( fp, shorten , SEEK_SET );
						
						int sz = 1024 * 1024;
						
						unsigned char *buf = new unsigned char[ sz ];
						int read = 0;
						while( 1 )
							{
							read = fread( buf, 1, sz, fp );
							if( read == 0 ) break;
							
							fwrite(  buf, 1, read, fpswap );
							}

						fclose( fp );
						delete buf;
						}
					fclose( fpswap );
					
					if( pause ) m1.delay_ms( 2000 );
					
					if( remove( fname.c_str() ) != 0  )
						{
						if( dbg_log ) fprintf( stdout, "Failed to remove log file\n" );
						m1.delay_ms( 2000 );
						}

					if( pause ) m1.delay_ms( 2000 );

					//needed the loop for windows as rename() sometimes failed when triggering log culling code at
					//very high frequency during stress testing using 
					int loop_count = 0;
					loop_rename_req_for_win_sometimes:

					if( rename( log_swap_fname.c_str(), fname.c_str() ) != 0 )
						{
//						char sztmp[ 1024 ];
						string serr = strerror( errno );
//						string serro = strerror( errno, sztmp, sizeof( sztmp ) );


						if( loop_count < 100 )
							{
							goto loop_rename_req_for_win_sometimes;
							loop_count++;
							}
						else{
							fprintf( stdout, "Failed to rename log file - error is: %s\n" ,serr.c_str() );
							}
						}
					log_culled = 1;
					if( dbg_log ) fprintf( stdout, "Culling older entries at begining of log file\n" );
					if( pause ) m1.delay_ms( 2000 );
					}
				else{
					if( dbg_log ) fprintf( stdout, "Can't create log swap file: '%s'\n", log_swap_fname.c_str() );
					}

				}
				
			}
		}

	if ( ( log_lvl <= file_log_lvl ) && ( file_log_lvl > 0 ) ) 
		{
		//windows code
		#ifdef compile_for_windows
		m1 = snew;
		m1.FindReplace( snew, "\n", "\r\n",0);		//make a LF a CRLF
		#endif


		//FILE *fp = fopen( log_fname.c_str() ,"a+b" );		 //creat/open a file for appending

		FILE *fp = m1.mbc_fopen( fname, "a+b" );				 //create a file

		if( fp == 0 )
			{
			if( dbg_log ) fprintf( stdout, "Can't create log file: %s\n", fname.c_str() );
			return;
			}

		fprintf( fp, "%s", snew.c_str() );
		fclose( fp );
		}

	}





}










//get local time into a tm struct
void gclog::get_time_now( struct tm &tt )
{

//linux code
#ifndef compile_for_windows
time_t epoch_secs;
time( &epoch_secs );				//returns GMT time not local time (in epoch secs)

tt = *localtime( &epoch_secs );
#endif


//windows code
#ifdef compile_for_windows
SYSTEMTIME  st;
GetLocalTime( &st );

tt.tm_hour = st.wHour;
tt.tm_min = st.wMinute;
tt.tm_sec = st.wSecond;
tt.tm_mday = st.wDay;
tt.tm_wday = 0;
tt.tm_mon = st.wMonth - 1;			//make it suite month value, ie Jan=0
tt.tm_year = st.wYear - 1900;       //make it suite year value for: struct tm
tt.tm_isdst = 0;

#endif

}











//e.g. pf( 5, "Tesing int=%05d, float=%f str=%s\n",z,(float)4.567,"I was Here" );
//the loglvl should be between 1 and 10 (or higher), 1 is highest 'priority'
void gclog::pf( int loglvl, const char *fmt,... )
{
string s;
char szTmp[1024];
va_list ap;

if( disabled ) return;


log_lvl = loglvl;

va_start(ap, fmt);
vsnprintf(szTmp,sizeof(szTmp)-1,fmt, ap);

lock_mutex_pf();

	outlog( szTmp, log_fname );

unlock_mutex_pf();

va_end(ap);
}








//get state of culling flag, then set new state, uses mutex to ensure thread safe access
bool gclog::get_culled_state( bool new_state )
{
bool retval;

lock_mutex_pf();
	retval = log_culled;
	log_culled = new_state;
unlock_mutex_pf();

return retval;
}








//allows size of vector vlog to be obtained 
//this will use mutex and block so only one thread has access to vlog
int gclog::get_vlog_size( )
{
int size;

lock_mutex_vlog( );
	size = vlog.size();
unlock_mutex_vlog( );

return size;

}









//allow access and modification of spec single vector vlog
//this will use mutex and block, so only one thread has access to vlog
//returns 1 if entry retrieved, else 0
bool gclog::get_vlog_entry( int idx, string &s, bool erase )
{
bool retval = 0;

s = "";

lock_mutex_vlog( );
	if( idx >= 0 )
		{
		if( idx < vlog.size() )
			{
			s = vlog[ idx ];
			retval = 1;

			if( erase )
				{
				vlog.erase( vlog.begin() + idx );		//erase spec entry
				}
			}
		}
unlock_mutex_vlog( );

return retval;
}






//allow access and modification of all vector vlog entries
//this will use mutex and block so only one thread has access to vlog
//return number of entries retrieved, else 0
int gclog::get_all_vlog_entries( vector<string> &vs, bool erase )
{
int retval = 0;

vs.clear();

lock_mutex_vlog( );

	retval = vlog.size();

	if( retval > 0 )
		{
		vs = vlog;

		if( erase ) vlog.clear();		//erase all entries
		}

unlock_mutex_vlog( );

return retval;
}








//make time string from supplied struct suitable for logging
void gclog::make_time_str( struct tm tn, string &s )
{

int year_offset = 1900;

/*
//linux code
#ifndef compile_for_windows
#endif

    //windows code
#ifdef compile_for_windows
#endif
*/

strpf( s, "%04d-%02d-%02d %02d:%02d:%02d",tn.tm_year + year_offset, tn.tm_mon, tn.tm_mday, tn.tm_hour, tn.tm_min, tn.tm_sec ); 
}









/*
//out to console and log file
void gclog::out_csl_log( string s, string log_fname )
{
string s1, stime, snew;
mystr m1 = s;



struct tm tn;
get_time_now( tn );
make_time_str( tn, stime );


if ( te_csl != 0 )
	{
    int len=tb_csl->length();
	te_csl->insert_position( len );

	m1.FindReplace( s1, "\r", "",0);		//remove any occurrances of a CR

	snew = stime + " ";
	snew +=s1;
	
	te_csl->insert( snew.c_str());
	te_csl->show_insert_position();
	}


//windows code
#ifdef compile_for_windows
m1 = snew;
m1.FindReplace( snew, "\n", "\r\n",0);		//make a LF a CRLF
#endif


//FILE *fp = fopen( log_fname.c_str() ,"a+b" );		 //creat/open a file for appending

FILE *fp = m1.mbc_fopen( log_fname, "a+b" );				 //create a file

if( fp == 0 )
	{
	cslpf("Can't create file: %s\n", log_fname.c_str() );
	return;
	}

fprintf( fp, "%s", snew.c_str() );
fclose( fp );

}


*/
