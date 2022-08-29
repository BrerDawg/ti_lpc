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

//gcthrd.h
//v1.01		07-01-12

#ifndef gcthrd_h
#define gcthrd_h



#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <string>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


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
#endif






#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>


#include "GCProfile.h"
#include "gclog.h"





struct wkfe_thread_tag
{
bool kill;
bool finished;
//int pause;
};







class gcthrd
{
private:

void (*p_callback)( void *args );
void *args;
bool use_mutex;


//linux code
#ifndef compile_for_windows
pthread_t thread_id;
pthread_mutex_t mutex1;
static void* thread_linux( void* arg );
#endif

//windows code
#ifdef compile_for_windows
DWORD WINAPI wkfe_thread_win(void* lpData);
HANDLE h_mutex1; 
PROCESS_INFORMATION processInformation;
static DWORD WINAPI thread_win( void* arg );
#endif


public:
bool thrd_dbg;
wkfe_thread_tag thrd;
gclog *logr;
int pr;
string obj_name;


public:
gcthrd(  );
~gcthrd(  );
void set_name( string s );
void set_thrd_callback( void (*p_cb)( void* ), void *args_in );

void set_log_ptr( int priority_in, gclog *logr_in );
bool create_thread( );
bool destroy_thread( );
bool wait_till_thread_destroyed( int timeout_ms );
double time_passed( unsigned long long int start );

bool create_mutex( );
bool destroy_mutex( );
bool lock_mutex( );
bool unlock_mutex( );


};

#endif
