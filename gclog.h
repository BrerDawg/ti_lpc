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


//gclog.h

//	v1.01 16-12-11



#ifndef gclog_h
#define gclog_h



#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <string>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

using namespace std;


//#define _LARGE_FILES
#define _FILE_OFFSET_BITS 64			//large file handling



//windows code
#ifdef compile_for_windows
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <conio.h>
#include <shlobj.h>
#include <objbase.h>
#endif


//linux code
#ifndef compile_for_windows
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <langinfo.h>
#include <pthread.h>
#endif


#include "GCProfile.h"



class gclog
{

private:
void (*p_callback)( string s);
string fixed_str;
string log_fname;
string log_swap_fname;
bool use_mutex;

int log_lvl;



//linux code
#ifndef compile_for_windows
pthread_mutex_t mutex_pf;
pthread_mutex_t mutex_vlog;
#endif

//windows code
#ifdef compile_for_windows
HANDLE h_mutex_pf; 
HANDLE h_mutex_vlog; 
#endif



public:
bool disabled;
int file_log_lvl;
int callback_log_lvl;
int printf_log_lvl;
bool dbg_log;
bool show_datetime;
bool to_callback;
bool to_file;
bool to_printf;
unsigned long long int fsize_low_water_mark;
unsigned long long int fsize_high_water_mark;
vector<string> vlog;
bool log_culled;

private:
void outlog( string s, string log_fname );


public:
gclog( );
~gclog( );
bool create_mutexes( );
bool destroy_mutexes( );
bool lock_mutex_pf( );
bool unlock_mutex_pf( );
bool lock_mutex_vlog( );
bool unlock_mutex_vlog( );

void fixed_string( string s );
void set_fname( string fn );
void set_log_callback( void (*p_cb)( string ) );	//note: this callback is not thread safe and does not use mutexes
void clear_log_file( );
void destructive_stress_test_of_log_file( string fname, string swap_fname );

bool set_max_log_size( string swap_fname, unsigned long long int high_size, unsigned long long int low_size );

void pf( int loglvl, const char *fmt,... );
bool get_culled_state( bool new_state );
void get_time_now( struct tm &tt );
void make_time_str( struct tm tn, string &s );


int get_vlog_size( );
bool get_vlog_entry( int idx, string &s, bool erase );
int get_all_vlog_entries( vector<string> &vs, bool erase );

};




#endif


