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

//gcpipe.cpp

//	v1.01 25-01-12
//	v1.02 15-jul-2018		//minor mod in 'gcpipe.h' for 64 bit compile



#include "gcpipe.h"





gcpipe::gcpipe( )
{
dbg = 0;

p_callback = 0;

opened = 0;
szrd[ 0 ] = 0x0;
last_read_count = 0;

kill_rd_loop = 0;
data_avail = 0;

//-------------
//windows code
#ifdef compile_for_windows
#endif
//-------------

}








gcpipe::~gcpipe( )
{
string nm;
strpf( nm, "%s -", obj_name.c_str() );
if( dbg ) pf("%s gcpipe - destroying obj\n", nm.c_str() );


if( check_child_running( ) )
	{
	if( !close_pipe( 1000 ) )
		{
		}
	}


if ( dbg ) pf("%s gcpipe - destroyed obj\n", nm.c_str() );

}







void gcpipe::set_name( string s )
{
obj_name = s;
}





//note: if set, this callback is called with a debug string for display
//if its not set, a regular printf is called.
void gcpipe::set_dbg_callback( void (*p_cb)( string ) )
{
p_callback = p_cb;

}







//not thread safe
//e.g. pf( 5, "Tesing int=%05d, float=%f str=%s\n",z,(float)4.567,"George was Here" );
//will call printf, unless a callback was defined
void gcpipe::pf( const char *fmt,... )
{
string s, snew;
char szTmp[1024];
va_list ap;


va_start(ap, fmt);
vsnprintf( szTmp, sizeof( szTmp )-1, fmt, ap );

if( p_callback != 0 )
	{
	snew = szTmp;
	p_callback( snew.c_str() );
	}
else{
	printf( "%s", szTmp );
	}


va_end(ap);
}








//FILE* stream = popen ("sort", "w");


//open child process app and create stdin/stdout r/w pipes
//returns 1 if opened, else 0
bool gcpipe::open_pipe( string cmd, string args_in )
{
string nm;

strpf( nm, "%s -", obj_name.c_str() );
if( dbg ) pf("%s gcpipe - open_pipe(): trying to open pipe\n", nm.c_str() );


if( opened ) return 0;

szrd[ 0 ] = 0x0;
kill_rd_loop = 0;
data_avail = 0;
soflow = "";


//-------------
//linux code
#ifndef compile_for_windows


cmd += " ";
cmd += args_in;

//int compipe[2];
//int Parent2Child[ 2 ], Child2Parent[ 2 ];
//pid_t pid;

//if ( pipe( Parent2Child ) < 0 || pipe( Child2Parent ) < 0 ) return -1; // cant establish pipes


if ( pipe ( pipe_in ) || pipe ( pipe_out ) )
	{
	if ( dbg ) pf( "%s gcpipe - open_pipe(): can't create pipes\n", nm.c_str() );
	return 0;
    }


child_pid = fork();		//create a child process	


switch( child_pid )
	{
	case -1:
		if ( dbg ) pf( "%s gcpipe - open_pipe(): can't fork process\n", nm.c_str() );
		return 0; 					//cant fork

	case 0: 						//child fork?

//		dup2(compipe[0],0);			//replace stdin with the in side of the pipe
//		close(compipe[1]);			//close unused side of pipe (out side)
//		execlp("bash","bash","-c",cmd,NULL);

//		close( Parent2Child[ 1 ] ); dup2( Parent2Child[ 0 ],0 );
//		close( Child2Parent[ 0 ] ); dup2(Child2Parent[ 1 ],1 );



		close( 1 );
		dup( pipe_out[ 1 ] );	// dup uses the lowest numbered unused file descriptor as new descriptor. In our case this now is 1
		close( 0 );          	// dup uses the lowest numbered unused file descriptor as new descriptor. In our case this now is 0
		dup( pipe_in[ 0 ] );  

     
//		cmd += " ";
//		cmd += args_in;

		//!!!note you won't see this printf in console as its done in the child process
		if ( dbg ) pf( "%s gcpipe - open_pipe(): child process is trying (via execlp) this cmd: '%s'\n", nm.c_str(), cmd.c_str() );

		execlp( "bash", "bash", "-c", cmd.c_str(), NULL );

		if ( dbg ) pf( "%s gcpipe - open_pipe(): execlp probably failed\n", nm.c_str() );

		exit( 0 );		//should never get here

//		close ( pipe_out[ 0 ] );
//		close ( pipe_out[ 1 ] );
//		close ( pipe_in[ 0 ] );
//		close ( pipe_in[ 1 ] );
	
//		int result;
//		exit( 0 );
//        wait ( &result );             /* Wait for child to finish */
		return 0;


	default: 						//parent fork?
//		dup2(compipe[1],1);			//replace stdout with out side of the pipe
//		close(compipe[0]);			//close unused side of pipe (in side)
//		setvbuf(stdout,(char*)NULL,_IONBF,0);	/* Set non-buffered output on stdout */


//		close( Child2Parent[ 1 ] ); io[ 0 ] = fdopen( Child2Parent[ 0 ], "r" );
//		close( Parent2Child[ 0 ] ); io[ 1 ] = fdopen( Parent2Child[ 1 ], "w" );

		child_in = fdopen( pipe_out[ 0 ], "r" );
        child_out = fdopen ( pipe_in[ 1 ], "w" );
        close( pipe_out[ 1 ] );
        close( pipe_in[ 0 ] );

		if ( dbg ) pf( "%s gcpipe - open_pipe(): parent fork is creating child process (via execlp) using this cmd: '%s'\n", nm.c_str(), cmd.c_str() );


		int z = 0;
		z++;
		goto fin_open;
		
//		io[ 0 ] = fdopen( Child2Parent[ 0 ], "r" );
//		io[ 1 ] = fdopen( Parent2Child[ 1 ], "w" );


//		return 1;
	}


#endif
//--------------






//-------------
//windows code
#ifdef compile_for_windows

readfh = 0;
writefh = 0;

SECURITY_ATTRIBUTES sattr;
LPCSTR args;

args = args_in.c_str();

// Allocate a buffer to read the app's output
if ( !( cbuff = (char *)GlobalAlloc( GMEM_FIXED, OUTPUTBUFSIZE )))
	{
	if ( dbg ) pf( "%s gcpipe - Err1 %s, %s\n", nm.c_str(), &NoMem[ 0 ], &ErrorStr[ 0 ] );
	MessageBox( 0, &NoMem[ 0 ], &ErrorStr[ 0 ], MB_OK | MB_ICONEXCLAMATION);
	return 0;
	}

//printf("\nHere1\n");

// Initialize the STARTUPINFO struct
ZeroMemory( &sinfo, sizeof( STARTUPINFO ));
sinfo.cb = sizeof( STARTUPINFO );

sinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

// Uncomment this if you want to hide the other app's
// DOS window while it runs
//sinfo.wShowWindow = SW_HIDE;

sinfo.hStdInput = GetStdHandle( STD_INPUT_HANDLE );
sinfo.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
sinfo.hStdError = GetStdHandle( STD_ERROR_HANDLE );

// Initialize security attributes to allow the launched app to
// inherit the caller's STDOUT, STDIN, and STDERR
sattr.nLength = sizeof( SECURITY_ATTRIBUTES );
sattr.lpSecurityDescriptor = 0;
sattr.bInheritHandle = TRUE;

//printf("\nHere2\n");

// Get a pipe from which we read
// output from the launched app
if ( !CreatePipe( &readfh, &sinfo.hStdOutput, &sattr, 0 ) )
	{
	// Error opening the pipe
	if ( dbg ) pf( "%s gcpipe - Err2 %s, %s\n", nm.c_str(), &NoPipeMsg[ 0 ], &ErrorStr[ 0 ] );
	MessageBox( 0, &NoPipeMsg[ 0 ], &ErrorStr[0], MB_OK | MB_ICONEXCLAMATION);
	bad1:
	GlobalFree( cbuff );
	cbuff = 0;
	return 0;
	}

//printf("\nHere3\n");

// Get a pipe to which we write the args out to the launched app
if ( !CreatePipe( &sinfo.hStdInput, &writefh, &sattr, 0 ))
	{
	// Error opening the pipe

	if ( dbg ) pf( "%s gcpipe - Err3 %s, %s\n", nm.c_str(), &NoPipeMsg[ 0 ], &ErrorStr[ 0 ] );
//	MessageBox( 0, &NoPipeMsg[ 0], &ErrorStr[ 0 ], MB_OK|MB_ICONEXCLAMATION);
	bad2:
	CloseHandle( readfh );
	readfh = 0;
	CloseHandle( sinfo.hStdOutput );
	goto bad1;
	}

	if( dbg ) pf( "%s gcpipe - trying (via CreateProcess) this cmd: %s\n", nm.c_str(), cmd.c_str() );
// Launch the app. We should return immediately (while the app is running)
if ( !CreateProcess( 0, (char*)cmd.c_str(), 0, 0, TRUE, 0, 0, 0, &sinfo, &pinfo ) )
	{
	if ( dbg ) pf( "%s gcpipe - Err4 %s, %s\n", nm.c_str(), &NoLaunchMsg[ 0 ], &ErrorStr[ 0 ] );
	MessageBox(0, &NoLaunchMsg[ 0 ], &ErrorStr[ 0 ], MB_OK | MB_ICONEXCLAMATION);
	CloseHandle( sinfo.hStdInput );
	CloseHandle( writefh );
	writefh = 0;
	goto bad2;
	}

//pf("\nHere4\n");

// Don't need the read access to these pipes
CloseHandle( sinfo.hStdInput );
CloseHandle( sinfo.hStdOutput );

// We haven't yet read app's output
sinfo.dwFlags = 0;							//use this a pointer to cbuff

//pf("\nHere5\n");

/*
// args output still needs to be done?
while ( args )
	{

	// More args to the app?
	if ( args )
		{
		if ( !(*args) )
			{
			// Ok, we're done writing out the args to the app.
			// We can close that pipe now, and clear 'args' so
			// that we stop writing out args
	//		CloseHandle(writefh);
	//		writefh = 0;
			args = 0;
			}
// Write out more args to the app
		else{
			if ( !WriteFile( writefh, args, lstrlen( args ), &pinfo.dwThreadId, 0 ))
				{
//				bad4: GlobalFree( cbuff );
//				cbuff = 0;
//				break;
				}

			args += pinfo.dwThreadId;		//use pinfo.dwThreadId to show how many bytes written
			}
		}
	}

pf("\nHere6\n");
*/

/*

// Capture more output of the app?
if ( readfh )
	{
	// Read in upto OUTPUTBUFSIZE bytes
	if (!ReadFile( readfh, cbuff + sinfo.dwFlags, OUTPUTBUFSIZE - sinfo.dwFlags, &pinfo.dwProcessId, 0 ) || !pinfo.dwProcessId )
		{
		// If we aborted for any reason other than that the
		// app has closed that pipe, it's an
		// error. Otherwise, the program has finished its
		// output apparently
		if (GetLastError() != ERROR_BROKEN_PIPE && pinfo.dwProcessId )
			{
			// An error reading the pipe
			pf( "%s gcpipe - Err5 %s, %s\n", nm.c_str(), &NoOutput[0], &ErrorStr[ 0 ] );
			MessageBox(0, &NoOutput[0], &ErrorStr[0], MB_OK|MB_ICONEXCLAMATION);
			goto bad4;
			}

		// Close the pipe
		CloseHandle(readfh);
		readfh = 0;
		}

	sinfo.dwFlags += pinfo.dwProcessId;			//pinfo.dwProcessId is used to show many bytes read
												//sinfo.dwFlags is pointer to cbuff
	}

// Nul-terminate it
if ( cbuff ) *( cbuff + sinfo.dwFlags ) = 0;


MessageBox(0, cbuff, "Success", MB_OK );
*/

#endif
//--------------



fin_open:

if ( dbg ) pf( "%s gcpipe - open_pipe(): have opened pipes ok\n", nm.c_str() );

if ( dbg ) pf( "%s gcpipe - open_pipe(): checking child actually running\n", nm.c_str() );




opened = 1;

//mystr m1;
//m1.delay_ms( 1000 );

if( check_child_running( ) )			//check if child process running.
	{
	if ( dbg ) pf( "%s gcpipe - open_pipe(): confirmed child is running ok\n", nm.c_str() );
	}


//m1.delay_ms( 1000 );

return 1;

}












bool gcpipe::close_pipe( int timeout_ms )
{
string nm;
mystr m1;
bool retval;

strpf( nm, "%s -", obj_name.c_str() );

if ( dbg ) pf("%s gcpipe - close_pipe( %d ): attempt to close pipes\n", nm.c_str(), timeout_ms );

if( !check_child_running( ) ) return 1;


//wr_pipe( "exit\n" );

//return 1;



//opened = 0;
//return 1;



//-------------------
//linux code
#ifndef compile_for_windows


int status;

m1.time_start( m1.ns_tim_start );
while( 1 )
	{

	double dt = m1.time_passed( m1.ns_tim_start );
	if( dt > ( (double)timeout_ms * 0.001 ) )			//passed timeoput period?
		{
		if ( dbg ) pf("%s gcpipe - close_pipe( %d ): timeout occurred\n", nm.c_str(), timeout_ms );

		close( (int)child_in );
		close( (int)child_out );
		goto bad_close;
		}


//	if ( dbg ) pf("%s gcpipe - close_pipe( %d ): waitpid %d start\n", nm.c_str(), timeout_ms, child_pid );
//	waitpid( child_pid, &status, WNOHANG );		//chec if child running
//	if ( dbg ) pf("%s gcpipe - close_pipe( %d ): waitpid %d stop\n", nm.c_str(), timeout_ms, child_pid );

	if( !check_child_running( ) )
		{
		close( (int)child_in );
		close( (int)child_out );

//		close( (int) io[ cn_gcpipe_out ] );
//		close( (int) io[ cn_gcpipe_in ] );

		if ( dbg ) pf("%s gcpipe - close_pipe( %d ): WIFEXITED showed child exited\n", nm.c_str(), timeout_ms );
		goto good_close; //confirm status returned shows the child terminated
		}

//	if( WIFEXITED( status ) )
//		{
//		close( (int)child_in );
//		close( (int)child_out );

//		if ( dbg ) pf("%s gcpipe - close_pipe( %d ): WIFEXITED showed child exited\n", nm.c_str(), timeout_ms );
//		goto good_close; //confirm status returned shows the child terminated
//		}
	
	m1.delay_ms( 100 );
	}	

#endif	
//-------------------




//-------------------
//windows code
#ifdef compile_for_windows

// Close input pipe
if ( writefh ) CloseHandle( writefh );
writefh = 0;
// Close output pipe
if ( readfh ) CloseHandle( readfh );
readfh = 0;

// Wait for the app to finish

m1.time_start( m1.ns_tim_start );
while( 1 )
	{
	double dt = m1.time_passed( m1.ns_tim_start );
	if( dt > ( (double)timeout_ms * 0.001 ) )			//passed timeoput period?
		{
		if ( dbg ) pf("%s gcpipe - close_pipe( %d ): timeout occurred\n", nm.c_str(), timeout_ms );
		goto bad_close;
		}

	if ( dbg ) pf("%s gcpipe - close_pipe(): WaitForSingleObject before\n", nm.c_str() );
//	if( WaitForSingleObject( pinfo.hProcess, 100 ) == WAIT_OBJECT_0 ) break;			//terminated ok?
	
	DWORD dw = WaitForSingleObject( pinfo.hProcess, 1 );
	if ( dbg ) pf("%s gcpipe - close_pipe(): WaitForSingleObject after: %ld\n", nm.c_str(), dw );

	if( dw == WAIT_OBJECT_0 )
		{
		if ( dbg ) pf("%s gcpipe - close_pipe(): WaitForSingleObject completed\n", nm.c_str() );
		goto good_close;										//process has signalled?
		}

	m1.delay_ms( 1 );
	}




#endif
//-------------------



bad_close:
retval = 0;
goto close;


good_close:
retval = 1;
goto close;



close:

//-------------------
//windows code
#ifdef compile_for_windows
CloseHandle( pinfo.hProcess );
CloseHandle( pinfo.hThread );

if( cbuff ) GlobalFree( cbuff );
cbuff = 0;
#endif
//-------------------


opened = 0;

return retval;
}








bool gcpipe::check_child_running( )
{
string nm;
int running = 1;

strpf( nm, "%s -", obj_name.c_str() );

if( !opened )
	{
	return 0;
	}

//----------------------
//linux code
#ifndef compile_for_windows
int status;

int wid = waitpid( child_pid, &status, WNOHANG );		//check if child existed
//note: if wid = 0, then 'status' is not altered and holds -
//previous value, so don't use it in this case

if ( dbg ) pf("%s gcpipe - check_child_running( ): waitpid returned: %d, status= %d\n", nm.c_str(), wid, status );
if( wid == -1 )
	{
	if ( dbg ) pf("%s gcpipe - check_child_running( ): waitpid %d returned error -1\n", nm.c_str(), child_pid );
	running = 0;
	}

if( wid == 0 ) return 1;	//if waitpid returned zero the child has not had a state change, assume still running

if( WIFEXITED(status) )
	{
	if ( dbg ) pf("%s gcpipe - check_child_running( ): waitpid %d is 'exited', status=%d\n", nm.c_str(), child_pid, WEXITSTATUS(status) );
	running = 0;
	}

if( WIFSIGNALED(status) )
	{
	if ( dbg ) pf("%s gcpipe - check_child_running( ): waitpid %d is 'signalled', status=%d\n", nm.c_str(), child_pid, WTERMSIG(status) );
	running = 0;
	}

if( WIFSTOPPED(status) )
	{
	if ( dbg ) pf("%s gcpipe - check_child_running( ): waitpid %d is 'stopped', status=%d\n", nm.c_str(), child_pid,  WSTOPSIG(status) );
	running = 0;
	}

if( WIFCONTINUED(status) )
	{
	if ( dbg ) pf("%s gcpipe - check_child_running( ): waitpid %d is 'continued'\n", nm.c_str(), child_pid );
	running = 0;
	}


//if( !WIFEXITED( status ) ) return 1;		//not exited?


if( !running ) goto not_running;

if ( dbg ) pf("%s gcpipe - check_child_running( ): waitpid showed child is running\n", nm.c_str() );

return 1;					//must be running
#endif
//----------------------



//----------------------
//windows code
#ifdef compile_for_windows

DWORD nStatus = WaitForSingleObject( pinfo.hProcess, 1 );

if( nStatus == WAIT_OBJECT_0 ) 	//check if child app process is ended?
	{
	if ( dbg ) pf("%s gcpipe - check_child_running() - obj is signalled, so probably not running\n", nm.c_str() );
	goto not_running;
	}


if( nStatus == WAIT_TIMEOUT ) 	//check if child app process is ended?
	{
	if( dbg ) pf("obj is not signalled, and timed out waiting for signal, so probably running\n\n");
	if ( dbg ) pf("%s gcpipe - check_child_running() - obj is not signalled, and timed out waiting for signal, so probably running\n", nm.c_str() );
	}

return 1;	
#endif
//----------------------


not_running:

//opened = 0;
return 0;
}











//----------------------
//windows code
#ifdef compile_for_windows

bool gcpipe::get_dos_error( DWORD dw, string &serr )
{
LPVOID lpMsgBuf;
//LPVOID lpDisplayBuf;

FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dw,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf,
				0, 
				NULL
				);


// get the error message and exit the process

//lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 

//StringCchPrintf((LPTSTR)lpDisplayBuf,  LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf); 
serr =  (char*)lpMsgBuf;

//		MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

//		LocalFree(lpMsgBuf);
//		LocalFree(lpDisplayBuf);

}

#endif
//----------------------














bool gcpipe::wr_pipe( string ss )
{
string nm;
string s1;
	
strpf( nm, "%s -", obj_name.c_str() );

if( !check_child_running( ) ) return 0;


if ( dbg ) pf("%s gcpipe - wr_pipe(): %s\n", nm.c_str(), ss.c_str() );


//-------------------
//linux code
#ifndef compile_for_windows
//fprintf( io[ cn_gcpipe_out ], "%s", s.c_str() );
//fflush( io[ cn_gcpipe_out ] );

fprintf( child_out, "%s", ss.c_str() );
fflush( child_out );
#endif
//-------------------





//-------------------
//windows code
#ifdef compile_for_windows


// Input and/or output still needs to be done?

int len = ss.length();

char* ptr = (char*)ss.c_str();

DWORD just_written = 0;
DWORD tot_written = 0;

if( !writefh )
	{
	if ( dbg ) pf("%s gcpipe - wr_pipe(): null writefh found\n", nm.c_str() );
	return -1;
	}

while ( 1 )
	{

	// Write out str till done
	bool ret = WriteFile( writefh, ptr + tot_written, len - tot_written, &just_written, 0);
	

	if ( !ret )
		{
//			bad4: 
//			GlobalFree(cbuff);
//			cbuff = 0;
//			break;

		DWORD dw = GetLastError( );
		
		get_dos_error( dw, s1 );

//		if ( dbg ) pf("%s gcpipe - wr_pipe(): WriteFile failed\n", nm.c_str() );
		if ( dbg ) pf( "%s gcpipe - wr_pipe(): Err is: %s\n", nm.c_str(), s1.c_str() );
		return -1;
		}

	tot_written += just_written;
	if( tot_written >= len ) goto fin_wr;

	}


#endif
//-------------------

fin_wr:

return 1;
}









bool gcpipe::is_opened( )
{
if( !check_child_running( ) ) return 0;
return 1;
}






//NOTE: this will block in a loop and also in the fgets() call used within, use a thread to
//collect chars, set 'kill_rd_loop' to break loop, but fgets() will needs to receive some
//chars to unblock it firstly. The dos ReadFile() call is prechecked via GetNumberOfConsoleInputEvents
//for chars being available to avoid ReadFile causing a block if nothing in pipe, this
//differs from the fgets call in linux code which does block.

//you need to clear 'kill_rd_loop' and 'data_avail' to allow this function to start

//only returns if 'kill_rd_loop' is set and something was read.

//returns num of chars read, else -1 on no child app and no pipes being opended.

//note: dos code has been written to simulate an fgets call, it returns on a LF being struck, but
//maintains any characters after LF in overflow str (soflow), these are retrieved on next call
int gcpipe::rd_pipe( string &sout )
{
string nm;
mystr m1;
bool got_something = 0;
int len;
//int grr1;

strpf( nm, "%s -", obj_name.c_str() );

if( !check_child_running( ) ) return -1;

if ( dbg ) pf("%s gcpipe - rd_pipe()\n", nm.c_str() );

last_read_count = 0;

if( data_avail ) goto rd_fin;
if( kill_rd_loop ) goto rd_fin;


//-------------------
//linux code
#ifndef compile_for_windows

szrd[ 0 ] = 0x0;


while( 1 )
	{

	if( kill_rd_loop == 1 ) goto rd_fin;
	
	if( data_avail == 0 )
		{

//		fflush( io[ cnIn ] );
//			fgets( szrd, sizeof( szrd ) - 2, io[ cn_gcpipe_in ]);
		fgets( szrd, sizeof( szrd ) - 2, child_in );
		last_read_count = strlen( szrd );
		
		sout = szrd;
		data_avail = 1;								//flag that have got something, do this after sout is set to str
		
		goto rd_fin;
		}
	m1.delay_ms( 1 );
	}


#endif	
//-------------------




//-------------------
//windows code
#ifdef compile_for_windows


if ( !readfh ) return -1;

//last_read_count = 0;

//m1.delay_ms( 2000 );


sout = "";

len = soflow.length();

//build a str only up to LF using any overflow from last call
for( int i = 0; i < len; i++ )
	{
	char ch = soflow[ i ];
	sout += ch;
	if( ch == '\n' )							//LF found?
		{
		i++;
		if( i < len )							//still more in overflow
			{
			soflow = soflow.substr( i, len - i );	//keep only unused in soflow
			got_something = 1;
			goto rd_fin_dos;			//no need to use ReadFile to read any more as yet
			}
		else{
			soflow = "";							//clear soflow overflow
			got_something = 1;
			goto rd_fin_dos;			//no need to use ReadFile to read any more as yet
			}
		}
	}

soflow = "";										//clear soflow overflow

cbuff[ 0 ] = 0x0;
while( 1 )
	{

	DWORD tot_read = 0;
	DWORD just_read = 0;
	if( kill_rd_loop == 1 ) goto rd_fin_dos;						//break out of loop
	
	if( data_avail == 0 )
		{
		// Capture more output of the app?
		if ( readfh )
			{
			DWORD dwReady;
			
			GetNumberOfConsoleInputEvents( readfh , &dwReady );			//peek if console has something

			if( dwReady > 0 )
				{

				// Read in upto OUTPUTBUFSIZE bytes
				if ( !ReadFile( readfh, cbuff, OUTPUTBUFSIZE - 1, &just_read, 0 ) )
					{
					// If we aborted for any reason other than that the
					// app has closed that pipe, it's an
					// error. Otherwise, the program has finished its
					// output apparently
					if ( GetLastError() == ERROR_BROKEN_PIPE )
						{
						// An error reading the pipe
						if( dbg ) pf( "%s gcpipe - Err5 %s, %s\n", nm.c_str(), &NoOutput[0], &ErrorStr[ 0 ] );
						MessageBox(0, &NoOutput[0], &ErrorStr[0], MB_OK|MB_ICONEXCLAMATION);
						goto rd_fin_dos;
						}

					// Close the pipe
			//		CloseHandle( readfh );
			//		readfh = 0;
					}
				else{
					*( cbuff + just_read ) = 0;							//terminate it
					last_read_count = strlen( cbuff );
					
					//build a str only up to LF
					int i;
					for( i = 0; i < last_read_count; i++ )
						{
						char ch = cbuff[ i ];
						sout += ch;
						if( ch == '\n' )
							{
							got_something = 1;
							break;							//LF found?
							}
						}

					i++;
					if( i < last_read_count )	//still more after LF?
						{

						//build a overflow str
						for( int j = i; j < last_read_count; j++ )
							{
							soflow += cbuff[ j ];
							}
						}

					goto rd_fin_dos;
					}
				}
			}
		}
	}


rd_fin_dos:
last_read_count = sout.length();
if( got_something )
	{
	data_avail = 1;								//flag that have got something, do this after sout is set to str
	}

//if ( dbg ) printf( "%s gcpipe - rd_pipe read %d bytes\n", nm.c_str(), last_read_count );


//MessageBox( 0, cbuff, "Success", MB_OK );

#endif
//-------------------


rd_fin:

if ( dbg ) printf("%s gcpipe - rd_pipe() finished: %d bytes read\n", nm.c_str(), last_read_count );

return last_read_count;
}























