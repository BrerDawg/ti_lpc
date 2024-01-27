//ti_lpc_cmd.h

//v1.01


#ifndef ti_lpc_cmd_h
#define ti_lpc_cmd_h

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




//linux code
#ifndef compile_for_windows

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
//#include <X11/Xaw/Form.h>
//#include <X11/Xaw/Command.h>

#define _FILE_OFFSET_BITS 64			//large file handling
//#define _LARGE_FILES
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>		//MakeIniPathFilename(..) needs this
#endif


//windows code
#ifdef compile_for_windows
#include <windows.h>
#include <process.h>
#include <winnls.h>
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <conio.h>

#define WC_ERR_INVALID_CHARS 0x0080		//missing from gcc's winnls.h
#endif



#include "audio_formats.h"



using namespace std;



#define pi ((float)M_PI)
#define twopi (2*(float)M_PI)


#define cns_build_date build_date







#endif
