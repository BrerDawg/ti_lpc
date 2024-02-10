/*
Copyright (C) 2024 BrerDawg

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

//GCProfile.h
//---- v1.81


#ifndef GCProfile_h
#define GCProfile_h

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501         //need this for GetFilesizeEx
#endif

#define _FILE_OFFSET_BITS 64	    //large file handling, must be before all #include...

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>
#include <ctype.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#define __STDC_FORMAT_MACROS			//needed this with windows/msys/mingw for: PRIu64
#include <inttypes.h>



//---------------- 
//linux code
#ifndef compile_for_windows
#define _FILE_OFFSET_BITS 64			//large file handling
//#define _LARGE_FILES
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>		//MakeIniPathFilename(..) needs this

#endif
//----------------

//#define compile_for_windows	0	//!!!!!!!!!!! uncomment for a dos/windows compile

//#define compile_for_windows		//!!!!!!!!!!! uncomment for a dos/windows compile

//---------------- 
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
//---------------- 


//linux code
#ifndef compile_for_windows
#define LLU "llu"
#endif


//windows code
#ifdef compile_for_windows
#define LLU "I64u"
#endif


using namespace std;

#define cnMaxPath 1024
#define cnMaxIniEntrySize 65536
#define cnMaxVarHistory 512

typedef unsigned long DWORD;

//function prototypes
void strpf(string &str,const char *fmt,...);






//NOTE: tried defining 'struct stat' in this struct, but it crashes when push_back() is called via a vector of this struct
struct st_mystr_make_dir_file_list_tag
{
string path;
string path_relative;						//this does not included a leading 'dir_sep'  '/'   v1.79	 

string fname;

unsigned long long int fsize;               //filesize
bool file;                                  //set if file
bool link;                                  //set if link

mode_t st_mode;                             //these are from 'struct stat', can't use 'struct stat' in this struct as it causes a crash when push_back() is called via a vector of this struct       
//nlink_t st_nlink;
//uid_t st_uid;
//gid_t st_gid;
dev_t st_rdev;
unsigned long long int st_size;
unsigned long long int st_size64;           //for 64 bit filesize values
time_t mtime;
time_t atime;
time_t ctime;
unsigned long long int depth;               //dir depth of this entry
};





struct st_mystr_padstr_vector               //see padstr_vector(..)
{
string str;                         //string to be inserted into a column
char c_justify;                     //char to use for justify padding
int i_justify;                      //0 = left, 1 = right,  2= centre
char c_interpad;                    //char to use for padding between columns
int i_column;                       //position of this column
int i_len;                          //max length of entry in this column
};






class mystr
{
private:
string csStr;
int iPos;
int iLen;
bool mystr_verbose;

public:
mystr();
mystr(char *s);
mystr(string s);
void operator=(string csIn);
void operator=(char *ptr);
mystr& operator=(mystr &x);
//void operator+( string &x, string &y );
void operator+=( char c );
void operator+=( string &x );

const string str();
const char* szptr();
unsigned long long int ns_tim_start;					//a ns timer start val,used in time_start(), time_time_passed
double perf_period;										//used in time_start(), time_time_passed

bool gets(string &csRet);
bool ExtractElement(int nth,char delimiter,string &element);
bool FindReplace(string &csRet,string csFind,string csReplace,int iStartPos);
bool FindReplace(string &csRet, char ch, string csReplace, int iStartPos);
bool ReplaceSeqCharOccurrence(string &csRet,char c,string csReplace,int iStartPos);		
bool ReplaceSeqNonVisible(string &csRet,string csReplace,int iStartPos);		

bool ExtractFilename(char subdir_char,string &filename);
bool ExtractPath(char subdir_char,string &path);
bool ExtractLastMostFolderName( char subdir_char, string &folder_name );

int ExtractParamVal(string param,string &equ);
int ExtractParamValNotingUserQuotes( string param, char delimit, char ch_quote, string &equ );
bool ExtractParamBool(string param,bool &b);
bool ExtractParamInt(string param,int &n);
bool ExtractParamFloat(string param,float &n);
bool ExtractParamDouble(string param,double &n);
int Load2DimArrayEquStr(string sEqu[][2],int max_count,char equ,char delimiter);
bool StrToEsc(char escape);
bool StrToEscMostCommon1();
bool StrToEscMostCommon2();
bool StrToEscMostCommon3();
bool EscToStr();

int FindNthSeqCharPosEnd(char c,int nth,int iStartPos);
int LoadArrayInts( int array[] , int max_count , char delimiter );
int LoadArrayDoubles( double array[] , int max_count , char delimiter );
int LoadVectorDoubles( vector<double> &vdoub , char delimiter );
int LoadVectorInts( vector<int> &vint , char delimiter );
int LoadVectorFloats( vector<float> &vfloat , char delimiter );
int LoadVectorStrings( vector<string> &vstr , char delimiter );
int LoadVectorStringsNotingUserQuotes( vector<string> &vstr , char delimit, char ch_quote, bool keep_single_quotes );
void padstr(string &sret,int pad,int lim);
void prepadstr(string &sret,int pad,int lim);
void padstrchar(string &sret, char ch, int pad, int lim );
int readfile( string fname, int maxlines );
bool writefile( string fname );
void to_upper( string &sret );
void to_lower( string &sret );
int mbcstr_wcstr( wstring &wstr );
int wcstr_mbcstr( wstring &wstr, string &sret );
bool mbc_check_file_exists( string &mbcfname );
FILE* mbc_fopen( string &mbcfname, const char *mode );
bool filesize( string fname, unsigned long long int &filesz );
int find_unicode_encoding ( unsigned char *array, unsigned int len, string &type );
int detect_unicode_bom_at_head_of_file( FILE *fp, string &type );
int cut_at_first_find( string &sret, string sfind, int pos_start );
int cut_just_past_first_find( string &sret, string sfind, int pos_start );
int cut_just_past_first_find_and_keep_right( string &sret, string sfind, int pos_start );
int cut_at_first_find_and_keep_right( string &sret, string sfind, int pos_start );
void delay_ms( int ms );
void delay_us( int usec );
void time_start( unsigned long long int &start );
double time_passed( unsigned long long int start );
void get_time_now( struct tm &tt );
bool make_date_str( struct tm tn, string &dow, string &dom, string &mon_num, string &mon_name, string &year, string &year_short );
void make_time_str( struct tm tn, string &hrs, string &mins, string &secs, string &time );
int extract_param_vals( string param_name, bool ignore_case, bool skip_whitespace, bool skip_cr, bool observe_c_commenting, bool allow_unicode_chars, int type, void *valptr, int maxcnt, int &valcnt, int &pos, string stop_at_str );
int extract_param_vals_and_raw( string param_name, bool ignore_case, bool skip_whitespace, bool skip_cr, bool observe_c_commenting, bool allow_unicode_chars, int type, void *valptr, int maxcnt, int &valcnt, int &pos, string stop_at_str, vector<string> &vraw );
int count_occurrence_char( char ch, int pos );
void make_filesize_str_ulli( string &sret, unsigned long long int bytes );
void make_filesize_str_ulli_discrete( string &snum, string &sunits, int fractional_digits, unsigned long long int bytes );
void make_engineering_str( string &snum, string &sunits, string &scombined, int fractional_digits, double dvalue, string sappend_num, string sappend_units );
bool appendfile( string fname, bool create_file_if_not_preset, unsigned char *buf, unsigned long long int buf_size );
int check_folder_path_exists( string path, char subdir_char );
int make_single_folder( string path, char subdir_char, unsigned int linux_permissions );
int make_folders( string path, char subdir_char, unsigned int linux_permissions );
bool time_str_to_tm( char delim, bool isdst, struct tm &tt );
bool date_str_to_tm( int format, struct tm &tt );
void add_slash_at_end_if_it_does_not_have_one( string &path, string dir_sep );
bool make_dir_file_list( bool which, string absolute_path, string dir, string dir_sep, unsigned long long int max_entries, bool recursive, vector<st_mystr_make_dir_file_list_tag> &vlist, unsigned long long int &cur_count, unsigned long long int &depth, unsigned long long int &bytes_tot );
int check_only_contains_2chars( char ch0, char ch1, int pos );
int check_only_contains_3chars( char ch0, char ch1, char ch2, int pos );
int check_only_contains_4chars( char ch0, char ch1, char ch2, char ch3, int pos );
void strip_cr_or_lf( string &sret, bool b_strip_cr, bool b_strip_lf );
int str_to_int( string ss );
void int_to_str( string &ss, int ii, string prinf_format );
double str_to_double( string ss );
void double_to_str( string &ss, double dd, string prinf_format );
long double str_to_long_double( string ss );
void long_double_to_str( string &ss, long double dd, string prinf_format );
int find_str_pos_in_list( vector<string> &vstr, string &ss );
int add_str_if_not_in_list( vector<string> &vstr, string &ss, int &pos );
int length();
bool strip_leading_chars( string &sret, char ch, int pos );
bool strip_leading_chars2( string &sret, char c1, char c2, int pos );
void padstr_vector( string &sret, vector<st_mystr_padstr_vector> vpad, int tot_length, char c_end_pad  );
void strip_any_chars1( string &sret, char c1, int pos );
void strip_any_chars2( string &sret, char c1, char c2, int pos );
void strip_any_chars3( string &sret, char c1, char c2, char c3, int pos );
void strip_any_chars4( string &sret, char c1, char c2, char c3, char c4, int pos );
void strip_any_chars5( string &sret, char c1, char c2, char c3, char c4, char c5, int pos );
void strip_any_chars6( string &sret, char c1, char c2, char c3, char c4, char c5, char c6, int pos );
int count_occurrence_of_char_up_to_pos( char ch, int start_pos, int end_pos );
bool strip_trailing_char( string &sret, char ch );
int LoadVectorStrings_str_delimiter( vector<string> &vstr, string delimiter );
int get_pos_of_nth_occurrence( char ch, int nth, int start_pos );
int cut_at_nth_occurrence(  string &sret, char ch, int nth, int start_pos );
int cut_just_past_nth_occurrence(  string &sret, char ch, int nth, int start_pos );
int cut_at_nth_occurrence_keep_right(  string &sret, char ch, int nth, int start_pos );
int cut_just_passed_nth_occurrence_keep_right(  string &sret, char ch, int nth, int start_pos );
bool tail( char delim, int max_delimits );
int cut_at_end_of_first_find_and_keep_right( string &sret, string sfind, int pos_start );
bool merge_path_and_stripped_fname( string dirsep, string path, string fname, string &sret );
void make_engineering_str_exp( string &snum, string &sunits, string &scombined, int fractional_digits, double dvalue, string sappend_num, string sappend_units, bool inc_plus );
bool appendfile_str( string fname, bool create_file_if_not_preset, bool clear_first, string ss );
bool appendfile_sz( char *fname, bool create_file_if_not_preset, bool clear_first, char *sz );
int LoadArray_decimal_uint8_t( uint8_t array[], int max_count , char delimiter );
int LoadArray_decimal_int8_t( int8_t array[], int max_count , char delimiter );
int LoadArray_decimal_int16_t( int16_t array[], int max_count , char delimiter );
int LoadArray_decimal_uint16_t( uint16_t array[], int max_count , char delimiter );
int LoadArray_hex_uint8_t( uint8_t array[], int max_count , char delimiter );
int LoadArray_hex_int8_t( int8_t array[], int max_count , char delimiter );
int LoadArray_hex_uint16_t( uint16_t array[], int max_count , char delimiter );
int LoadArray_hex_int16_t( int16_t array[], int max_count , char delimiter );
int ExtractParamVal_with_delimit(string param, string delim, string &equ);
bool array_uint8_t_to_hex_str( uint8_t array[], int max_count, bool uppr_case, string first_prefix, string individual_prefix, string delimit, string &shex );
bool array_int8_t_to_decimal_str( int8_t array[], int max_count, string first_prefix, string individual_prefix, string delimit, string &sdecimal );
bool array_uint16_t_to_hex_str( uint16_t array[], int max_count, bool uppr_case, string first_prefix, string hex_prefix, string delimit, string &shex );
bool array_int16_t_to_decimal_str( int16_t array[], int max_count, string first_prefix, string individual_prefix, string delimit, string &sdecimal );
bool array_uint32_t_to_hex_str( uint32_t array[], int max_count, bool uppr_case, string first_prefix, string hex_prefix, string delimit, string &shex );
bool array_int32_t_to_decimal_str( int32_t array[], int max_count, string first_prefix, string individual_prefix, string delimit, string &sdecimal );

};





class GCProfile
{
private:
string csFilename;
string csAll;

public:
bool bFastMode;
bool bneed_save;

public:
GCProfile(string csFilenameIn);
~GCProfile();

int Index();
int Exists();
int GetPrivateProfileStr(string csSection,string csKey,string csDefault,string *csString);
//int GetPrivateProfileStringF(string csSection,string csKey,string csDefault,string *csString);

bool WritePrivateProfileStr(string csSection,string csKey,string csString);
// int WritePrivateProfileString(string csSection,string csKey,string csString);
long GetPrivateProfileLONG(string csSection,string csKey,long lDefault);
float GetPrivateProfileFLOAT(string csSection,string csKey,float fDefault);
double GetPrivateProfileDOUBLE(string csSection,string csKey,double dDefault);
int GetPrivateProfileParams(string csSection,string csKey,string csDefault,string csParams[],int iParamCount);

uint64_t GetPrivateProfile_uint64_t(string csSection,string csKey, uint64_t ui_default );
uint64_t GetPrivateProfile_hex_uint64_t(string csSection,string csKey, uint64_t ui_default );


int WritePrivateProfileLONG(string csSection,string csKey,long lNum);
int WritePrivateProfileFLOAT(string csSection,string csKey,float fNum);
int WritePrivateProfileDOUBLE(string csSection,string csKey,double dNum);
int WritePrivateProfileParams(string csSection,string csKey,string csParams[],int iParamCount);

int WritePrivateProfileDOUBLE_precision(string csSection,string csKey, double dNum, string sprecision );

int WritePrivateProfile_uint64_t( string csSection, string csKey, uint64_t ui );
int WritePrivateProfile_hex_uint64_t( string csSection, string csKey, uint64_t ui );

bool Save();
int ClearAllProfileEntries();

private:
//int GetPrivateProfileSectionLineNum(string csSection);	
};

#endif

