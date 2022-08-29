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

//gcpipe.h

//	v1.01 25-01-12
//	v1.02 15-jul-2018		//mods for 64 bit compile



#ifndef gcpipe_h
#define gcpipe_h



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





//linux code
#ifndef compile_for_windows
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <langinfo.h>
#include <pthread.h>
#endif

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

// Define OUTPUTBUFSIZE to be as big as the largest output you expect
// from the console app, plus one char. For example, if you
// expect the app to return 10 chars, define this to be 11.
#define OUTPUTBUFSIZE 4096

const TCHAR ErrorStr[] = "Error";
const TCHAR NoMem[] = "Out of memory";
const TCHAR NoPipeMsg[] = "Can't open pipe";
const TCHAR NoLaunchMsg[] = "Can't start console app";
const TCHAR NoOutput[] = "Can't read output of console app";

#endif



#include "GCProfile.h"

#define cn_gcpipe_out 1
#define cn_gcpipe_in 0




//extern pid_t child_pid;
//extern int pipe_in[ 2 ];					//This is the pipe with which we write to the child process
//extern int pipe_out[ 2 ];				//This is the pipe with which we read from the child process
//extern FILE* child_in;
//extern FILE* child_out;






class gcpipe
{
private:
bool opened;
pid_t child_pid;
string obj_name;
int last_read_count;
void (*p_callback)( string s);
string soflow;						//holds dos rd_pipe chars that occurred after a LF char
									//this is used to implement a fgets like call to rp_pipe for dos
									//the chars after a LF are kept at added to begining of data on
									//next rd_pipe call
public:
bool dbg;
bool kill_rd_loop;					//this is to stop the endless rd_pipe() loop, set by external code
bool data_avail;					//flags external code that data is ready for access
char szrd[ 8192 ];


//linux code
#ifndef compile_for_windows
//FILE *io[];
FILE **io;							//64 bit


int pipe_in[ 2 ];					//This is the pipe with which we write to the child process
int pipe_out[ 2 ];				//This is the pipe with which we read from the child process
FILE* child_in;
FILE* child_out;

#endif



//windows code
#ifdef compile_for_windows
STARTUPINFO sinfo;
PROCESS_INFORMATION pinfo;
HANDLE readfh, writefh;
char *cbuff;
#endif


private:
bool check_child_running( );
bool get_dos_error( DWORD dw, string &serr );


public:
void set_name( string s );
void set_dbg_callback( void (*p_cb)( string ) );
void pf( const char *fmt,... );
bool open_pipe( string fname, string args_in );
bool close_pipe( int timeout_ms );
bool wr_pipe( string s );
int rd_pipe( string &sout );
bool is_opened( );


public:
gcpipe( );
~gcpipe( );

};





#endif


