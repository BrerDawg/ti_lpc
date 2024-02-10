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


//GCProfile.cpp

//---- v1.01 08-09-07		//faster operations
//---- v1.02 29-02-08		//fixed compiler warning messages
//---- v1.03 02-06-09		//added GetPrivateProfileParams,WritePrivateProfileParams
//---- v1.04 27-07-09		//reduced amount of file accesses by using string obj to hold file contents,see Index(),Save()
//---- v1.05 20-02-10 		//changed GetPrivateProfileString to GetPrivateProfileStr to avoid clash when compiling in Windows
							//changed WritePrivateProfileString to WritePrivateProfileStr to avoid clash when compiling in Windows
							//added pathname handling functions for obj my_str works with Linux or Windows

//---- v1.05 05-07-10		//fixed FindReplace() bug causing premature completion - search for 'changed 06-07-10'
//---- v1.06 25-07-10		//fixed WritePrivateProfileStr() bug causing exisiting csKey in follwing csSection to be used
							//-if a matching csKey was not found in specified csSection
//---- v1.07 25-07-10		//added mystr ExtractParamVal() and bool,int,float,double param extraction
//---- v1.08 08-08-10		//added mystr (char* s) (strings s) overloadining options, now: mystr m=str; , mystr m="hello"; work
//---- v1.09 19-08-10		//added ExtractElement(int nth,char delimiter,string &element);
//---- v1.10 07-09-10		//added Load2DimArrayEquStr(string sEqu[][2],int max_count,char equ,char delimiter);
//---- v1.11 20-12-10		//added FindNthSeqCharPosEnd(string &csRet,char c,int nth,int iStartPos)
//---- v1.12 22-01-11		//added LoadArrayInts( double array[] , int max_count , char delimiter )
//							//added LoadArrayDoubles( double array[] , int max_count , char delimiter )
//							//inceased #define cnMaxIniEntrySize 65536


//---- v1.13 15-02-11		//added LoadVectorDoubles( vector<double> &vdoub , char delimiter );
							//added LoadVectorInts( vector<double> &vint , char delimiter );
							//added LoadVectorFloats( vector<float> &vfloat , char delimiter );
//---- v1.14 19-02-11		//added padstr(string &sret,int pad,int lim);
							//edded prepadstr(string &sret,int pad,int lim);

//---- v1.15 14-03-11		//added mystr readfile, writefile and LoadVectorStrings
//---- v1.16 18-03-11		//added mystr padstrchar

//---- v1.17 08-04-11		//added mystr toupper() and tolower()
//---- v1.18 22-07-11		//added unicode conversion functions and "#define _FILE_OFFSET_BITS 64"
//---- v1.19 23-07-11		//fixed FindReplace() bug
//---- v1.20 23-07-11		//added cut_at_first_find(), cut_just_past_first_find()
//---- v1.21 23-12-11		//mystr delay_ms( int ms ) for linux/windows, see also 'delay_us()'
							//mystr time_start( unsigned long long int start ) 
							//mystr time_passed( unsigned long long int start ) for linux/windows
//---- v1.22 19-01-12		//fixed mystr::readfile(), now returns an int instead of bool, also calls mbc_fopen
//---- v1.23 26-03-13		//added mystr cut_just_past_first_find_and_keep_right(), cut_at_first_find_and_keep_right()
//---- v1.24 23-04-13		//fixed cut_just_past_first_find_and_keep_right() where cut str becomes empty
//---- v1.25 24-04-13		//added get_time_now(), make_date_str(), make_time_str()
//---- v1.26 01-06-13		//fixed mystr::gets() not return 1 on success, and lockup when trying to erase a line with just one char and that char is a CR
//---- v1.27 18-09-13		//added extract_param_vals()
//---- v1.28 13-10-13		//strpf() - increased storage from 1024 to 1MB, Note: if you make buffer too big it crashes
//							//			linux allowed 4MB, windows only 1MB
//---- v1.29 16-10-13		//added filesize()
//---- v1.30 22-10-13		//fixed extract_param_vals(), where find was used instead of compare
//---- v1.32 17-11-13		//added extract_param_vals_and_raw()
//---- v1.33 03-12-13		//added count_occurrence_char( char ch, int pos )
//---- v1.34 14-12-13		//make_filesize_str_ulli( string &sret, unsigned long long int bytes )
//---- v1.35 07-01-14		//for extract_param_vals(), extract_param_vals_raw() - added allow_unicode_chars flag
//---- v1.36 31-08-14		//fixed large filesize reporting for Windows int mystr::filesize()
//---- v1.36 31-08-14		//added mystr::appendfile()
//---- v1.36 31-08-14		//added mystr::make_filesize_str_ulli_discrete()
//---- v1.37 01-09-14		//fixed get_time_now( ) bug that was always returning Sunday
//---- v1.38 13-09-14		//added: make_single_folder(..), make_folders(..), check_folder_path_exists((..)
//---- v1.39 09-11-14		//added: time_str_to_tm(..), date_str_to_tm(..)
                            //added: add_slash_at_end_if_it_does_not_have_one(..)
                            //added: make_dir_file_list(..)
                            //added: check_only_contains_2chars(..)
                            //added: check_only_contains_3chars(..)
                            //added: check_only_contains_4chars(..)
//---- v1.40 15-nov-14		//added:  'struct stat' like fields to 'st_mystr_make_dir_file_list_tag', used in make_dir_file_list(..)
                            //added: strip_cr_or_lf(..)
                            //added: str_to_int(..), int_to_str(..)
                            //added: str_to_double(..), double_to_str(..)
                            //added: str_to_long_double(..), long_double_to_str(..)
//---- v1.41 16-nov-14		//fixed bug in tn.tm_mon range check in make_date_str( .. )
//---- v1.42 07-dec-14		//added: find_str_pos_in_list(..),   add_str_if_not_in_list(..)
//---- v1.43 11-dec-14		//added: length()
//---- v1.44 16-dec-14		//added: strip_leading_chars(..)
//---- v1.45 23-jan-15		//added: padstr_vector(..)
//---- v1.46 30-jan-15		//changed extract_param_vals_and_raw(..) so vraw captures both whitespaces and CRs, thus ignoring flags: 'skip_whitespace' and 'skip_cr' 
//---- v1.47 30-jan-15      //added strip_leading_chars_2(..), strip_any_chars2(..), strip_any_chars3(..), strip_any_chars4(..), strip_any_chars5(..), strip_any_chars6(..)
//---- v1.48 05-feb-15      //added count_occurrence_of_char_up_to_pos(..)
//---- v1.49 14-mar-15      //added StrToEscMostCommon1(..)
//---- v1.50 24-apr-15      //added ExtractParamValNotingUserQuotes(..)
//---- v1.51 02-mar-15      //added LoadVectorStringsNotingUserQuotes(..)
//---- v1.52 03-jun-15      //added strip_trailing_char(..), LoadVectorStrings_str_delimiter()
//---- v1.53 26-sep-16      //fixed LoadVectorInts( vector<double> &vint , char delimiter ), now its LoadVectorInts( vector<int> &vint , char delimiter )
//---- v1.54 10-mar-17      //added high precision WritePrivateProfileDOUBLE_precision(), set 'sprecision' to something like: "%.17g"
//---- v1.55 03-jul-17      //added make_engineering_str()
//---- v1.56 15-sep-17      //added get_pos_of_nth_occurrence(), cut_at_nth_occurrence(), cut_just_past_nth_occurrence(),
							//cut_just_passed_nth_occurrence_keep_right()

//---- v1.57 15-oct-17      //added warning comments: '!!! WILL STILL return full string if cut did not happen, this can be tested as it returns 0'
							//for xxx_keep_right() functions, like in: cut_just_past_first_find_and_keep_right()
//---- v1.58 20-jan-2018	//moded mystr::EscToStr(), had 'sscanf(szHex,"%x",&c);'   now: 'sscanf(szHex,"%x",&iChar);' , did this as later versions of gcc 
							//were causing memory corruptions and endless loop
//---- v1.59 20-feb-2018	//added: tail( schar delim, int max_delimits )
//---- v1.60 18-mar-2018	//added: cut_at_end_of_first_find_and_keep_right(...)
                         
//---- v1.61 17-apr-2018	//moded LoadVectorStrings() - to handle empty delimiter with no chars, like: abc,def,,ghi,jkl
//---- v1.62 27-apr-2018	//added: merge_path_and_stripped_fname(..)
							//added: operator+= for both char and string, e.g: m1 += "hello";
//-----v1.63 05-jul-2018	//added: make_engineering_str_exp()
//-----v1.64 27-jul-2018	//added: appendfile_str(), appendfile_sz()
//-----v1.65 08-jul-2018	//added: LoadArray_hex_uint8_t(), LoadArray_hex_int8_t(), 
//-----v1.66 01-dec-2018 	//added: delay_us( int usec )
//-----v1.67 16-dec-2018 	//added: WritePrivateProfile_uint64_t(), WritePrivateProfile_hex_uint64_t(), GetPrivateProfile_uint64_t(), GetPrivateProfile_hex_uint64_t()
//-----v1.68 28-dec-2018 	//added: ExtractParamVal_with_delimit(string param, string delim, string &equ),LoadArray_hex_uint8_t(), LoadArray_hex_int8_t(), LoadArray_hex_uint16_t(), LoadArray_hex_int16_t()
							//LoadArray_decimal_uint8_t(), LoadArray_decimal_int8_t(), LoadArray_decimal_uint16_t(), LoadArray_decimal_int16_t()
							//array_uint8_t_to_hex_str(), array_int8_t_to_decimal_str(), array_uint16_t_to_hex_str(), array_int16_t_to_decimal_str(), array_uint32_t_to_hex_str(), array_int32_t_to_decimal_str()

//-----v1.69 12-oct-2019 	//added: StrToEscMostCommon2() - handles '[' and ']'

//-----v1.70 16-oct-2019 	//fixed GetPrivateProfileStr() - was incorrectly specifying 'len'

//-----v1.71 03-dec-2019 	//fixed ExtractPath(), now returns 1 on success
//-----v1.72 04-jan-2020 	//fixed 'cut_just_past_first_find_and_keep_right()', was only cutting off first char of 'sfind' 
//-----v1.73 17-nov-2020 	//added missing 'return 1;' in 'ExtractFilename()', was causing crash only when optimization in use, e.g: '-O3', happened inside external function 'get_path_app_params()'
//-----v1.74 15-dec-2020 	//added: 'GCProfile::ClearAllProfileEntries()'
//-----v1.75 23-dec-2020 	//added: 'GCProfile::bneed_save' flag, now when just reading a profile there will be no save in 'GCProfile::~GCProfile()' destructor, this was 
							//causing data loss if you inadvertantly tried to open a non '.ini' formatted file, i.e: non 'ini' files were being corrupted when trying to read them

//-----v1.76 07-jan-2021 	//added: 'mystr::ExtractLastMostFolderName()'
//-----v1.77 25-jan-2021 	//added: 'StrToEscMostCommon3()' which converts 'Equal' symbols
//-----v1.78 29-jan-2021 	//moded LoadVectorStrings() - to handle first char being a delimiter, like: ,abc,def,,ghi,jkl
//-----v1.79 14-aug-2022 	//moded 'make_dir_file_list()' to include relative path, see :'st_mystr_make_dir_file_list_tag.path_relative'	!! this does not included a leading 'dir_sep'  '/' 
//-----v1.80 27-dec-2023 	//moded 'GCProfile::Save()' to fix 'fprintf()' write size limit, now uses 'fwrite()'
//-----v1.81 05-feb-2024 	//moded 'ExtractParamVal_with_delimit()' to return 0  if extracted param length is zero, e.g. this would happen when extracting 'events0=' when str contained this 'events0=,'

#include "GCProfile.h"
//note a line entry cannot be bigger that cnMaxIniEntrySize

mystr::mystr()
{
iPos=0;
iLen=0;
mystr_verbose = 1;
}


mystr::mystr(char *s)
{
csStr=s;
iPos=0;
iLen=strlen(s);	
mystr_verbose = 1;
}


mystr::mystr(string s)
{
csStr=s;
iPos=0;
iLen=s.length();	
mystr_verbose = 1;
}


void mystr::operator=(string csIn)
{
iPos=0;
csStr=csIn;
iLen=csStr.length();	
mystr_verbose = 1;
}

void mystr::operator=(char *ptr)
{
csStr=ptr;
iPos=0;
iLen=strlen(ptr);	
mystr_verbose = 1;
}



mystr& mystr::operator=(mystr &x)
{
return x;
}




/*
void mystr::operator+( string &x, string &y )
{
csStr = x + y;

iPos = 0;
iLen = csStr.length();	
}
*/

void mystr::operator+=( char c )
{
csStr += c;

iPos = 0;
iLen = csStr.length();	
}


void mystr::operator+=( string &x )
{
csStr += x;

iPos = 0;
iLen = csStr.length();	
}



//return mystr obj's string
const string mystr::str()
{
return csStr;
}



//return mystr obj's string
const char* mystr::szptr()
{
return csStr.c_str();
}








//v1.58 moded EscToStr(), had 'scanf(szHex,"%x",&c);'   now: 'scanf(szHex,"%x",&iChar);' , did this as later versions of gcc were causing memory corruptions and endless loop

//find and convert any occurrence of a 'C like' escaped backslash and hex nums, back to a acsii chars
//e.g. are below
// BackSlash= \5c
// Equal= \3d
// Comma= \2c
// CR=  \0d
// LF=  \0a
// BS=  \08
// ESC= \1b
// BEL= \07

//modifies mystr obj's string
//returns 1, of ok, else 0


bool mystr::EscToStr()
{
string s1;
char szHex[sizeof(int)],c;
int iChar;

szHex[2]=0;

//memset( szHex, 0x20, sizeof( szHex ) );		//clear

if(iLen==0) return 0;

for(int i=0;i<iLen;i++)				//process each char
	{

	c=csStr[i];

	if(c=='\\')						//if backslash read in next two hex chars
		{
		i++;
		if(i>=iLen) goto FinParam;
		szHex[0]=csStr[i];
		i++;
		if(i>=iLen) goto FinParam;
		szHex[1]=csStr[i];

		sscanf(szHex,"%x",&iChar);		//v1.58 - conv 2 hex char to an ascii char
		
		c = (char) iChar;				//v1.58
		}
	
	s1+=c;
	}

FinParam:
csStr=s1;										//modify this obj's string
iLen=csStr.length();
iPos=0;	
return 1;
}







//!! see also: StrToEscMostCommon1()
//find and convert char occurrence to a 'C like' escaped backslash followed by char's hex values
//YOU MUST also do a backslash '\' firstly, if you did it other than first, this function will get confused by the previous StrToEsc conversions that use a backslash

//e.g. are below
// BackSlash= \5c		--You MUST do this firstly, if you want backslashes
// Equal= \3d
// Comma= \2c
// CR=  \0d
// LF=  \0a
// BS=  \08
// ESC= \1b
// BEL= \07

//e.g:
//m1.StrToEsc( '\\' );				//convert multiline to 'c escaped', NOTE backslash MUST be done first
//m1.StrToEsc( '\n' );
//m1.StrToEsc( '\r' );
//m1.StrToEsc( '\t' );
//m1.StrToEsc( ',' );
//m1.StrToEsc( '/' );
//m1.StrToEsc( '*' );


//NOTE: you need to escape any backslashes firstly

//modifies mystr obj's string
//returns 1, of ok, else 0

bool mystr::StrToEsc(char escape)
{
char szNum[15];
string s1,sfind,sreplace;

if(iLen==0) return 0;

//FindReplace(s1,"\\","\\5c",0);		//replace all of any backslashes


sreplace="\\";					//make escape backslash and hex num e.g. \3d
sprintf(szNum,"%02x",escape);
szNum[2]=0;						//limit to max of 2 hex chars
sreplace+=szNum;

sfind=escape;
FindReplace(s1,sfind,sreplace,0);	//replace all occurences

csStr=s1;										//modify this obj's string
iLen=csStr.length();	
iPos=0;

return 1;
}















// SEE ---> mystr::StrToEscMostCommon2()

//find and convert any of the below defined char occurrence to a 'C like' escaped backslash followed by char's hex values

//e.g. are below
// BackSlash    '\\'    becomes \5c
// LF           '\n'    becomes \0a
// CR=          '\r'    becomes \0d
// Tab=         '\t'    becomes \09
// Comma=       ','     becomes \2c
// FwdSlash=    '/'     becomes \2f
// Asterisk=    '*'     becomes \2a


//modifies mystr obj's string
//returns 1, of ok, else 0

bool mystr::StrToEscMostCommon1()
{
char szNum[ 15 ];
string s1;

if( iLen == 0 ) return 0;


for( int i = 0; i < iLen; i++ )
    {
    char ch = csStr[ i ];
    
    switch( ch )
        {
        case '\\':
            s1 += "\\5c";
        break;

        case '\n':
            s1 += "\\0a";
        break;

        case '\r':
            s1 += "\\0d";
        break;

        case '\t':
            s1 += "\\09";
        break;

        case ',':
            s1 += "\\2c";
        break;

        case '/':
            s1 += "\\2f";
        break;

        case '*':
            s1 += "\\2a";
        break;

        default:
            s1 += ch;
        break;
        }
    }

csStr = s1;										//modify this obj's string
iLen = csStr.length();	
iPos = 0;

return 1;
}










//find and convert any of the below defined char occurrence to a 'C like' escaped backslash followed by char's hex values

//e.g. are below
// BackSlash    	'\\'    becomes \5c
// LF           	'\n'    becomes \0a
// CR=          	'\r'    becomes \0d
// Tab=         	'\t'    becomes \09
// Comma=       	','     becomes \2c
// FwdSlash=    	'/'     becomes \2f
// Asterisk=		'*'     becomes \2a
// Bracket Left=	'['     becomes \5b
// Bracket Right=	']'     becomes \5d


//modifies mystr obj's string
//returns 1, of ok, else 0

bool mystr::StrToEscMostCommon2()
{
char szNum[ 15 ];
string s1;

if( iLen == 0 ) return 0;


for( int i = 0; i < iLen; i++ )
    {
    char ch = csStr[ i ];
    
    switch( ch )
        {
        case '\\':
            s1 += "\\5c";
        break;

        case '\n':
            s1 += "\\0a";
        break;

        case '\r':
            s1 += "\\0d";
        break;

        case '\t':
            s1 += "\\09";
        break;

        case ',':
            s1 += "\\2c";
        break;

        case '/':
            s1 += "\\2f";
        break;

        case '*':
            s1 += "\\2a";
        break;

        case '[':
            s1 += "\\5b";
        break;

        case ']':
            s1 += "\\5d";
        break;

        default:
            s1 += ch;
        break;
        }
    }

csStr = s1;										//modify this obj's string
iLen = csStr.length();	
iPos = 0;

return 1;
}











//find and convert any of the below defined char occurrence to a 'C like' escaped backslash followed by char's hex values

//e.g. are below
// BackSlash    	'\\'    becomes \5c
// LF           	'\n'    becomes \0a
// CR=          	'\r'    becomes \0d
// Tab=         	'\t'    becomes \09
// Comma=       	','     becomes \2c
// FwdSlash=    	'/'     becomes \2f
// Asterisk=		'*'     becomes \2a
// Bracket Left=	'['     becomes \5b
// Bracket Right=	']'     becomes \5d
// Equal=			'='     becomes \3d


//modifies mystr obj's string
//returns 1, of ok, else 0

bool mystr::StrToEscMostCommon3()
{
char szNum[ 15 ];
string s1;

if( iLen == 0 ) return 0;


for( int i = 0; i < iLen; i++ )
    {
    char ch = csStr[ i ];
    
    switch( ch )
        {
        case '\\':
            s1 += "\\5c";
        break;

        case '\n':
            s1 += "\\0a";
        break;

        case '\r':
            s1 += "\\0d";
        break;

        case '\t':
            s1 += "\\09";
        break;

        case ',':
            s1 += "\\2c";
        break;

        case '/':
            s1 += "\\2f";
        break;

        case '*':
            s1 += "\\2a";
        break;

        case '[':
            s1 += "\\5b";
        break;

        case ']':
            s1 += "\\5d";
        break;
        
        case '=':
            s1 += "\\3d";
        break;

        default:
            s1 += ch;
        break;
        }
    }

csStr = s1;										//modify this obj's string
iLen = csStr.length();	
iPos = 0;

return 1;
}











//load supplied 8bit uint array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 128,127,3,9,8,7,1
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=128
// array[1]=127
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=1

//returns number of array items loaded, in this case 7
int mystr::LoadArray_decimal_uint8_t( uint8_t array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			int iv;
			sscanf( snum.c_str() , "%d", &iv  );	//conv num into int array
			array[count] = iv;

			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	int iv;
	sscanf( snum.c_str() , "%d", &iv  );	//conv num into int array
	array[count] = iv;
	count++;
	}

return count;
}







//load supplied 8bit int array with numbers extracted from mystr obj's string.
//a suitable string is similar to: -128,127,3,9,8,7,-1
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=-128
// array[1]=127
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=-1

//returns number of array items loaded, in this case 7
int mystr::LoadArray_decimal_int8_t( int8_t array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			int iv;
			sscanf( snum.c_str() , "%d", &iv  );	//conv num into int array
			array[count] = iv;

			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	int iv;
	sscanf( snum.c_str() , "%d", &iv  );	//conv num into int array
	array[count] = iv;
	count++;
	}

return count;
}








//load supplied 16bit uint array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1,12,3,9,8,7,2345
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=1
// array[1]=12
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=2345

//returns number of array items loaded, in this case 7

int mystr::LoadArray_decimal_uint16_t( uint16_t array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			int iv;
			sscanf( snum.c_str() , "%d", &iv  );	//conv num into int array
			array[count] = iv;
			
			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	int iv;
	sscanf( snum.c_str() , "%d", &iv  );	//conv num into int array
	array[count] = iv;
	count++;
	}

return count;
}











//load supplied 16bit int array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1,12,3,9,8,7,-2345
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=1
// array[1]=12
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=-2345

//returns number of array items loaded, in this case 7

int mystr::LoadArray_decimal_int16_t( int16_t array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			int iv;
			sscanf( snum.c_str() , "%d", &iv  );	//conv num into int array
			array[count] = iv;
			
			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	int iv;
	sscanf( snum.c_str() , "%d", &iv  );	//conv num into int array
	array[count] = iv;
	count++;
	}

return count;
}









//load supplied 8bit uint array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1,12,3,9,8,7,af
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=1
// array[1]=12
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=af

//returns number of array items loaded, in this case 7

int mystr::LoadArray_hex_uint8_t( uint8_t array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			int iv;
			sscanf( snum.c_str() , "%x", &iv  );	//conv num into int array
			array[count] = iv;

			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	int iv;
	sscanf( snum.c_str() , "%x", &iv  );	//conv num into int array
	array[count] = iv;
	count++;
	}

return count;
}











//load supplied 8bit int array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1,12,3,9,8,7,af
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=1
// array[1]=12
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=af

//returns number of array items loaded, in this case 7

int mystr::LoadArray_hex_int8_t( int8_t array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			int iv;
			sscanf( snum.c_str() , "%x", &iv  );	//conv num into int array
			array[count] = iv;
			
			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	int iv;
	sscanf( snum.c_str() , "%x", &iv  );	//conv num into int array
	array[count] = iv;
	count++;
	}

return count;
}










//load supplied 16bit uint array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1,12,3,9,8,7,23af
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=1
// array[1]=12
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=23af

//returns number of array items loaded, in this case 7

int mystr::LoadArray_hex_uint16_t( uint16_t array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			int iv;
			sscanf( snum.c_str() , "%x", &iv  );	//conv num into int array
			array[count] = iv;

			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	int iv;
	sscanf( snum.c_str() , "%x", &iv  );	//conv num into int array
	array[count] = iv;
	count++;
	}

return count;
}












//load supplied 16bit int array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1,12,3,9,8,7,23af
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=1
// array[1]=12
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=23af

//returns number of array items loaded, in this case 7

int mystr::LoadArray_hex_int16_t( int16_t array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			int iv;
			sscanf( snum.c_str() , "%x", &iv  );	//conv num into int array
			array[count] = iv;

			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	int iv;
	sscanf( snum.c_str() , "%x", &iv  );	//conv num into int array
	array[count] = iv;
	count++;
	}

return count;
}











//load supplied int array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1,1234,3,9,8,7,-23
//the user defined char delimiter: would be ','
//the results in 'array' would then be the numbers:
// array[0]=1
// array[1]=1234
// array[2]=3
// array[3]=9
// array[4]=8
// array[5]=7
// array[6]=-23

//returns number of array items loaded, in this case 7

int mystr::LoadArrayInts( int array[], int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			sscanf( snum.c_str() , "%d" , array + count  );	//conv num into int array
			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	sscanf( snum.c_str() , "%d" , array + count  );	//conv num into int array
	count++;
	}

return count;
}









//load supplied double array with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1.234e+10,5,0.1234,3.1415926,9,8,7 
//the user defined char delimiter: would be ','
//the results in 'array' would then be the double numbers:
// array[0]=1.234e+10
// array[1]=5
// array[2]=0.1234
// array[3]=3.1415926
// array[4]=9
// array[5]=8
// array[6]=7

//returns number of array items loaded, in this case 7

int mystr::LoadArrayDoubles( double array[] , int max_count , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;

if(iLen==0) return 0;
if(max_count<=0) return 0;

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			sscanf( snum.c_str() , "%lg" , array + count  );	//conv num into double array
			in_num = 0;
			snum = "";
			count++;
			if( count >= max_count ) break;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	sscanf( snum.c_str() , "%lg" , array + count  );	//conv num into int array
	count++;
	}

return count;
}









//load supplied int vector with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1,1234,3,9,8,7,-23
//the user defined char delimiter: would be ','
//the results in 'vint' would then be the int numbers:
// vint[0]=1
// vint[1]=1234
// vint[2]=3
// vint[3]=9
// vint[4]=8
// vint[5]=7
// vint[6]=-23

//returns number of vector items loaded, in this case 7

int mystr::LoadVectorInts( vector<int> &vint , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;
int n;

if(iLen==0) return 0;

vint.clear();

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			sscanf( snum.c_str() , "%d" , &n  );	//conv num into int array
			vint.push_back( n );
			in_num = 0;
			snum = "";
			count++;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	sscanf( snum.c_str() , "%d" , &n  );	//conv num into int array
	vint.push_back( n );
	count++;
	}

return count;
}











//load supplied double vector with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1.234e+10,5,0.1234,3.1415926,9,8,7 
//the user defined char delimiter: would be ','
//the results in 'vdoub' would then be the double numbers:
// vdoub[0]=1.234e+10
// vdoub[1]=5
// vdoub[2]=0.1234
// vdoub[3]=3.1415926
// vdoub[4]=9
// vdoub[5]=8
// vdoub[6]=7

//returns number of vector items loaded, in this case 7

int mystr::LoadVectorDoubles( vector<double> &vdoub , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;
double d;

if(iLen==0) return 0;

vdoub.clear();

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			sscanf( snum.c_str() , "%lg" , &d  );	//conv num into double array
			vdoub.push_back( d );
			in_num = 0;
			snum = "";
			count++;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	sscanf( snum.c_str() , "%lg" , &d  );	//conv num into int array
	vdoub.push_back( d );
	count++;
	}

return count;
}






//load supplied float vector with numbers extracted from mystr obj's string.
//a suitable string is similar to: 1.234e+10,5,0.1234,3.1415926,9,8,7 
//the user defined char delimiter: would be ','
//the results in 'vfloat' would then be the double numbers:
// vfloat[0]=1.234e+10
// vfloat[1]=5
// vfloat[2]=0.1234
// vfloat[3]=3.1415926
// vfloat[4]=9
// vfloat[5]=8
// vfloat[6]=7

//returns number of vector items loaded, in this case 7

int mystr::LoadVectorFloats( vector<float> &vfloat , char delimiter )
{
string snum;
char c;
bool in_num = 0;
int count = 0;
float f;

if(iLen==0) return 0;

vfloat.clear();

snum="";
for( int i = 0; i < iLen; i++ )
	{
	c = csStr[i];				//get char
	if( c == delimiter)			//delimiter?
		{
		if( in_num )
			{
			sscanf( snum.c_str() , "%f" , &f  );	//conv num into double array
			vfloat.push_back( f );
			in_num = 0;
			snum = "";
			count++;
			}
		else{
			continue;
			}
		}
	else{
		in_num = 1;
		snum += c;				//build number
		}
	}

if( in_num )					//if there is no ending delimiter
	{
	sscanf( snum.c_str() , "%f" , &f  );	//conv num into int array
	vfloat.push_back( f );
	count++;
	}

return count;
}




//note see also: LoadVectorStringsNotingUserQuotes(..)

//load supplied string vector with strings extracted from mystr obj's string.
//a suitable string is similar to: hello,yes,no,nabble,bye,option,last 
//the user defined char delimiter: would be ','
//the results in 'vstr' would then be the strings:
// vstr[0]=hello
// vstr[1]=yes
// vstr[2]=no
// vstr[3]=nabble
// vstr[4]=bye
// vstr[5]=option
// vstr[6]=last

//returns number of vector items loaded, in this case 7

int mystr::LoadVectorStrings( vector<string> &vstr , char delimiter )  //<--- see also: LoadVectorStringsNotingUserQuotes(..)
{
string sstr;
char c,c0;
bool in_str = 0;
int count = 0;
float f;

if(iLen==0) return 0;

vstr.clear();

sstr="";

for( int i = 0; i < iLen; i++ )
	{
	c = csStr[ i ];						//get char

	if( i == 0 )						//v1.78, csStr[0] a delimiter?   like: ,abc,def,,ghi,jkl
		{
		if( c == delimiter)				//delimiter?
			{
			vstr.push_back( "" );		//store an empty string
			count++;
			continue;
			}
		}

//	if( i > 1 )							//v1.61, empty delimiter?
	if( i >= 1 )						//v1.78, empty delimiter?
		{
		c0 = csStr[ i - 1 ];			//peek behind
		if( ( c == delimiter ) && ( c0 == delimiter ) )		//just have delimiters and no chars?, like: abc,def,,ghi,jkl
			{
			vstr.push_back( "" );		//store an empty string
			count++;
			continue;
			}
		}


	if( c == delimiter)					//delimiter?
		{
		if( in_str )
			{
			vstr.push_back( sstr );
			in_str = 0;
			sstr="";
			count++;
			}
		else{
			continue;
			}
		}
	else{
		in_str = 1;
		sstr += c;						//build number
		}
	}

if( in_str )							//if there is no ending delimiter
	{
	vstr.push_back( sstr );
	count++;
	}



return count;
}





//convert supplied buffer to a 8 bit hex string with specified delimiting
//'array': ptr to buffer
//'max_count': is size of array to process
//'uppr_case': set this for upper case alphas
//'first_prefix': set this to have a prefix at begining of string, e.g:  'hex_str='
//'hex_prefix': set this to have a prefix at begining of each hex  of string, e.g:  '0x'		//this would produce: 0xFA, 0x1B
//'shex': will receive hex string
//returns 1 if a string was generated, else 0
bool mystr::array_uint8_t_to_hex_str( uint8_t array[], int max_count, bool uppr_case, string first_prefix, string hex_prefix, string delimit, string &shex )
{
if( max_count == 0 ) return 0;
shex = first_prefix;

string s1, st;
for( int i = 0; i < max_count; i++ )
	{
	if( uppr_case ) strpf( s1, "%s%02X%s", hex_prefix.c_str(), array[i], delimit.c_str() );
	else strpf( s1, "%s%02x%s", hex_prefix.c_str(), array[i], delimit.c_str() );
	shex += s1;
	}
return 1;
}






//convert supplied buffer to a 16 bit hex string with specified delimiting
//'array': ptr to buffer
//'max_count': is size of array to process
//'uppr_case': set this for upper case alphas
//'first_prefix': set this to have a prefix at begining of string, e.g:  'hex_str='
//'hex_prefix': set this to have a prefix at begining of each hex  of string, e.g:  '0x'		//this would produce: 0xFA, 0x1B
//'shex': will receive hex string
//returns 1 if a string was generated, else 0
bool mystr::array_uint16_t_to_hex_str( uint16_t array[], int max_count, bool uppr_case, string first_prefix, string hex_prefix, string delimit, string &shex )
{
if( max_count == 0 ) return 0;
shex = first_prefix;

string s1, st;
for( int i = 0; i < max_count; i++ )
	{
	if( uppr_case ) strpf( s1, "%s%04X%s", hex_prefix.c_str(), array[i], delimit.c_str() );
	else strpf( s1, "%s%04x%s", hex_prefix.c_str(), array[i], delimit.c_str() );
	shex += s1;
	}
return 1;
}








//convert supplied buffer to a 32 bit hex string with specified delimiting
//'array': ptr to buffer
//'max_count': is size of array to process
//'uppr_case': set this for upper case alphas
//'first_prefix': set this to have a prefix at begining of string, e.g:  'hex_str='
//'hex_prefix': set this to have a prefix at begining of each hex  of string, e.g:  '0x'		//this would produce: 0xFA, 0x1B
//'shex': will receive hex string
//returns 1 if a string was generated, else 0
bool mystr::array_uint32_t_to_hex_str( uint32_t array[], int max_count, bool uppr_case, string first_prefix, string hex_prefix, string delimit, string &shex )
{
if( max_count == 0 ) return 0;
shex = first_prefix;

string s1, st;
for( int i = 0; i < max_count; i++ )
	{
	if( uppr_case ) strpf( s1, "%s%08X%s", hex_prefix.c_str(), array[i], delimit.c_str() );
	else strpf( s1, "%s%08x%s", hex_prefix.c_str(), array[i], delimit.c_str() );
	shex += s1;
	}
return 1;
}








//convert supplied buffer to a 8 bit decimal string with specified delimiting
//'array': ptr to buffer
//'max_count': is size of array to process
//'uppr_case': set this for upper case alphas
//'first_prefix': set this to have a prefix at begining of string, e.g:  'decimal_str='
//'individual_prefix': set this to have a prefix at begining of each decimal of string, e.g:  '!!'		//this would produce: !!127, !!-128
//'sdecimal': will receive decimal string
//returns 1 if a string was generated, else 0
bool mystr::array_int8_t_to_decimal_str( int8_t array[], int max_count, string first_prefix, string individual_prefix, string delimit, string &sdecimal )
{
if( max_count == 0 ) return 0;
sdecimal = first_prefix;

string s1, st;
for( int i = 0; i < max_count; i++ )
	{
	strpf( s1, "%s%d%s", individual_prefix.c_str(), array[i], delimit.c_str() );
	sdecimal += s1;
	}
return 1;
}








//convert supplied buffer to a 16 bit decimal string with specified delimiting
//'array': ptr to buffer
//'max_count': is size of array to process
//'uppr_case': set this for upper case alphas
//'first_prefix': set this to have a prefix at begining of string, e.g:  'decimal_str='
//'individual_prefix': set this to have a prefix at begining of each decimal of string, e.g:  '!!'		//this would produce: !!127, !!-128
//'sdecimal': will receive decimal string
//returns 1 if a string was generated, else 0
bool mystr::array_int16_t_to_decimal_str( int16_t array[], int max_count, string first_prefix, string individual_prefix, string delimit, string &sdecimal )
{
if( max_count == 0 ) return 0;
sdecimal = first_prefix;

string s1, st;
for( int i = 0; i < max_count; i++ )
	{
	strpf( s1, "%s%d%s", individual_prefix.c_str(), array[i], delimit.c_str() );
	sdecimal += s1;
	}
return 1;
}






//convert supplied buffer to a 32 bit decimal string with specified delimiting
//'array': ptr to buffer
//'max_count': is size of array to process
//'uppr_case': set this for upper case alphas
//'first_prefix': set this to have a prefix at begining of string, e.g:  'decimal_str='
//'individual_prefix': set this to have a prefix at begining of each decimal of string, e.g:  '!!'		//this would produce: !!127, !!-128
//'sdecimal': will receive decimal string
//returns 1 if a string was generated, else 0
bool mystr::array_int32_t_to_decimal_str( int32_t array[], int max_count, string first_prefix, string individual_prefix, string delimit, string &sdecimal )
{
if( max_count == 0 ) return 0;
sdecimal = first_prefix;

string s1, st;
for( int i = 0; i < max_count; i++ )
	{
	strpf( s1, "%s%d%s", individual_prefix.c_str(), array[i], delimit.c_str() );
	sdecimal += s1;
	}
return 1;
}






//note: modifed version of LoadVectorStrings(..) to handle commas within an entry, via a 'ch_quote' mechanism,
//given an comma delimited parameter string list e.g: 'hello,more please,no,option,"with,comma","no comma", ""quoted"", ""leading quoted,trailing quoted"",""
//the user defined 'delimit' of ',' and 'ch_quote' of '"' and 'keep_single_quotes' = 0
//the results in 'vdoub' would then be the strings:
// vstr[0]=hello
// vstr[1]=more please
// vstr[2]=no
// vstr[3]=option
// vstr[4]=with,comma
// vstr[5]=no comma
// vstr[6]="quoted"
// vstr[7]="leading quote
// vstr[8]=trailing quote""
// vstr[9]="

//returns number of vector items loaded, in this case 9


//the user defined 'delimit' of ',' and 'ch_quote' of '"' and 'keep_single_quotes' = 1
//the results in 'vdoub' would then be the strings:
// vstr[0]=hello
// vstr[1]=more please
// vstr[2]=no
// vstr[3]=option
// vstr[4]="with,comma"
// vstr[5]="no comma"
// vstr[6]="quoted"
// vstr[7]="leading quote
// vstr[8]=trailing quote""
// vstr[9]="

//returns number of vector items loaded, in this case 9

int mystr::LoadVectorStringsNotingUserQuotes( vector<string> &vstr , char delimit, char ch_quote, bool keep_single_quotes )
{
int iPos=0, iEndPos;
int iLen;
int iLenParam;

iLen = csStr.length();
if( iLen == 0 ) return 0;

vstr.clear();

bool in_quote = 0;

string ss;

for( int i = 0; i < iLen; i++ )
	{
	char c1, c2;
	c1 = csStr[ i ];									//get two sequential chars
	if( i < ( iLen - 1 ) ) c2 = csStr[ i + 1 ];
	else c2 = 0;
	
	if( c1 == ch_quote  )								//quote?
		{
		if( c2 == ch_quote )
			{
			ss += c1;									//store only one of the two quote pairs
			i++;
//				printf("i: %d\n", i );
			}
		else{
			in_quote = !in_quote;						//single quote, toggle flag
			if( keep_single_quotes ) ss += c1;			//keep any quotes that are not paired
			if( !in_quote )
				{
				if( c2 == delimit )
					{
					vstr.push_back( ss );
					ss = "";
					i++;
					continue;
					}
				}
			} 
		}
	else{
		if( !in_quote )
			{
			if( c1 == delimit )
				{
				vstr.push_back( ss );
				ss = "";
				continue;
				}
			}
		ss += c1;										//store char as in quote
		}
	}

if( ss.length() )
	{
	vstr.push_back( ss );
	}

return vstr.size();
}











//load supplied string vector with strings extracted from mystr obj's string.
//a suitable string is similar to: hello_delimit_yes_delimit_no_delimit_nabble_delimit_bye_delimit_option_delimit_last_delimit_
//a suitable string is similar to: _delimit_hello_delimit_yes_delimit_no_delimit_nabble_delimit_bye_delimit_option_delimit_last 
//the user defined string delimiter: would be '_delimit_'
//the results in 'vstr' would then be the strings:
// vstr[0]=hello
// vstr[1]=yes
// vstr[2]=no
// vstr[3]=nabble
// vstr[4]=bye
// vstr[5]=option
// vstr[6]=last

//returns number of vector items loaded, in this case 7

int mystr::LoadVectorStrings_str_delimiter( vector<string> &vstr, string delim )
{
string s1;
mystr m1;

char c;
int count = 0;

if( iLen == 0 ) return 0;

vstr.clear();

int delim_len = delim.length();

if( ( delim_len == 0 ) || ( delim_len > iLen ) )
    {
    vstr.push_back( csStr );
    return 1;
    }



vector<int> vpos;                       //this holds start and end pos of each delimiter

vpos.push_back( 0 );                    //put in beginning of str

int ilast = 0;
while( 1 )                              //make a list of delimiter start positions
	{
    int pos = csStr.find( delim, ilast );
    
    if( pos != string::npos )               //found delimiter occurrence?
        {
        vpos.push_back( pos );
        ilast = pos + delim_len;            //set past delimiter
        vpos.push_back( ilast );
        if( ilast >= iLen ) break;
        }
    else{
        break;
        }
    }
//printf( "here1\n" );

vpos.push_back( iLen );                     //put in end of str

if( vpos.size() < 3 )                       //no delimeter?
    {
    vstr.push_back( csStr );
    return 1;
    }

//printf("vpos.size(): %02d\n", vpos.size() );

for( int i = 0; i < vpos.size(); i += 2 )      //go through string and cut it into a list, removeing delimter str entries
    {
    int p1 = vpos[ i ];
    int p2;
    
    if( i < ( vpos.size() - 1 ) )           //get next delimiter pos if any
        {
        p2 = vpos[ i + 1 ];
        }
    else{
        p2 = iLen - 1;
        }
//    printf("p1: %02d %02d\n", p1, p2 );
    
    m1 = csStr.substr( p1, p2 - p1 );       //cut between two delimiters, e.g: would end up with '_delimit_nabble'
    m1.cut_just_past_first_find_and_keep_right( s1, delim, 0 );
    if( s1.length() )
        {
        vstr.push_back( s1 );
        }
    }
return vstr.size();
}









//load supplied 2 dimensional string array with strings extracted from mystr obj's string.
//a suitable string is similar to: fruit=apple,cutlery=spoon,door=open,door=closed
//the user defined char equate: would be '=', 
//the user defined char delimiter: would be ','
//the result in sEqu is:
// sEqu[0][0]=fruit   	sEqu[0][1]=apple
// sEqu[1][0]=cutlery   sEqu[1][1]=spoon
// sEqu[2][0]=door   	sEqu[2][1]=open
// sEqu[3][0]=door   	sEqu[3][1]=closed

//returns number of array items loaded, in this case 4

int mystr::Load2DimArrayEquStr(string sEqu[][2],int max_count,char equate,char delimiter)
{
int pos,pequ,pdelim,count=0;
string s1,s2;
bool bfinish=0;
bool bfoundequ;

if(iLen==0) return 0;
if(max_count<=0) return 0;

pos=0;
while(1)
	{
	bfoundequ=1;						//assume and equate char will be found
	pequ=csStr.find(equate,pos);		//find next equate char
	if(pequ==string::npos)				//no more equates found?
		{
		pequ=iLen;
		bfoundequ=0;
		}

	pdelim=csStr.find(delimiter,pos);	//find next delimiter char
	if(pdelim==string::npos)			//no more delimiters found?
		{
		pdelim=iLen;
		bfinish=1;
		}

	if(pequ>pdelim)						//no equate char found before delimiter char?
		{
		pequ=pdelim;
		bfoundequ=0;
		}

	s1=csStr.substr(pos,pequ-pos);		//extract first string part

	pos=pequ;
	if(bfoundequ) pos++;				//skip over equate char

	if((pdelim-pequ)>=1) pequ++;		//don't extract delimiter char
	s2=csStr.substr(pos,pdelim-pequ);	//extract second string part

	sEqu[count][0]=s1;
	sEqu[count][1]=s2;

	count++;
	if(count>=max_count) break;
	if(bfinish) break;
	
	pos=pdelim+1;						//skip over delimiter char
	if(pos>=iLen) break;
	}

return count;
}





//assuming obj csStr is a pathname, extract path without filename
//specify the OS char that means subdir, e.g. linux='/' or dos='\'
//returns 1, if OK, 0 if failed.

bool mystr::ExtractPath(char subdir_char,string &path)
{
if(iLen==0) return 0;

int pos=csStr.rfind(subdir_char);		//find last subdir char

if(pos==string::npos) return 0;

path=csStr.substr(0,pos);

return 1;								//v1.71
}




//assuming obj csStr is a pathname, extract last most folder name without dir seperators or filename
//specify the OS char that means subdir, e.g. linux='/' or dos='\'
//e.g. if path in 'mystr' obj is /jpgs/ww2/airplanes/p51.jpg, this function will return 'airplanes'
//returns 1, if OK, 0 if failed.

bool mystr::ExtractLastMostFolderName( char subdir_char, string &folder_name )
{
folder_name = "";

if(iLen==0) return 0;

if(iLen<4) return 0;								//e.g: minimum len would be this: '/x/y'

int pos = iLen-1;


int posend=csStr.rfind( subdir_char, pos );			//find last subdir char
if(posend==string::npos) return 0;

posend--;
if( posend < 3 ) return 0;

int posstart=csStr.rfind( subdir_char, posend );	//find 2nd last subdir char

posstart++;
if( ( posstart + 1 ) >= posend ) return 0;

folder_name=csStr.substr( posstart, posend - posstart + 1 );

return 1;
}







//append a dir seperator if 'path' does not have (and 'path' is not empty),
//strip 'fname' of its path if any,
//set 'sret' = 'path' + 'fname',
//returns 1, on success, else 0
bool mystr::merge_path_and_stripped_fname( string dirsep, string path, string fname, string &sret )
{
string spath, s1;
mystr m1;

sret = "";
if( dirsep.length() == 0 ) return 0;

char cc = dirsep[ 0 ];

sret = path;
m1.add_slash_at_end_if_it_does_not_have_one( sret, dirsep );

m1 = fname;
m1.ExtractFilename( cc, fname );

sret += fname;
return 1;
}







//assuming obj csStr is a pathname, extract filename
//specify the OS char that means subdir, e.g. linux='/' or dos='\'
//returns 1, if OK, 0 if failed.

bool mystr::ExtractFilename(char subdir_char,string &filename)
{
if(iLen==0) return 0;

int pos=csStr.rfind(subdir_char);		//find last subdir char

if(pos==string::npos)
	{
	filename=csStr;
	return 1;
	}

pos++;									//skip over subdir char
if(pos>=iLen) return 0;

filename=csStr.substr(pos,iLen-pos);
return 1;																//v1.73
}








//if param = yes,no,1,0
bool mystr::ExtractParamBool(string param,bool &b)
{
//printf("mystr::ExtractParamBool() - here0\n" );
string equ;


if(param.length()==0) return 0;

if(ExtractParamVal(param,equ))
	{
	int len=equ.length();
//printf("mystr::ExtractParamBool() - here0.5\n" );
	for(int i=0;i<len;i++) equ[i]=equ[i]&0xdf;		//make upper case
	if(equ.compare("YES")==0) {b=1;return 1;}
	if(equ.compare("NO")==0) {b=0;return 1;}
	if(equ.compare("1")==0) {b=1;return 1;}
	if(equ.compare("0")==0) {b=0;return 1;}
	
//printf("mystr::ExtractParamBool() - here1\n" );
	return 1;
	}

//printf("mystr::ExtractParamBool() - here2\n" );

return 0;
}




bool mystr::ExtractParamInt(string param,int &n)
{
string equ;

if(param.length()==0) return 0;

if(ExtractParamVal(param,equ))
	{
	sscanf(equ.c_str(),"%d",&n);
	return 1;
	}

return 0;
}






bool mystr::ExtractParamFloat(string param,float &n)
{
string equ;

if(param.length()==0) return 0;

if(ExtractParamVal(param,equ))
	{
	sscanf(equ.c_str(),"%f",&n);
	return 1;
	}

return 0;
}





bool mystr::ExtractParamDouble(string param,double &n)
{
string equ;

if(param.length()==0) return 0;

if(ExtractParamVal(param,equ))
	{
	sscanf(equ.c_str(),"%lf",&n);
	return 1;
	}

return 0;
}



//given an user specified delimited string e.g. 'apple,orange,lemon,pear' (delimter=,) or
// or e.g. 'apple!orange!lemon!pear' (delimter=!) 
//find the 'nth' element and  retrieve.
// nth=0 would find 'apple' in 'element'
// nth=3 would find 'pear' in 'element'
//returns 1 if successful, else 0
bool mystr::ExtractElement(int nth,char delimiter,string &element)
{
int pos=0;
int len,start,end;
int j=0;

if (nth<0) return 0;

len=csStr.length();
if(len<=0) return 0;

end=len;
if(j==nth) goto found;					//first element?

for(;;)
	{
	pos=csStr.find(delimiter,pos);	
	if(pos==string::npos) return 0;
	pos++;
	if(pos>=len) return 0;
	j++;
	if(j==nth) goto found;
	}

found:
start=pos;
end=csStr.find(delimiter,pos);	
if(end==string::npos) end=len;


				
element=csStr.substr(start,end-start);

return 1;	
}



//given an comma delimited parameter string list e.g. 'mode=play,position=1234,status=ok'
//find param and extract its equate value i.e, param: 'mode='   equate: 'play' (note the equal sign was included in the param, otherwise it would end up in the equ value, like this: equate: '=play'
//there is no need for the equal sign to exist or seperate param from equate value
//i.e. 'type01,count01,autoyes'  param: 'count' would yield  equ: '01'
//returns 1 if successful, else 0;
int mystr::ExtractParamVal(string param,string &equ)		//<---------------- see also:  ExtractParamValNotingUserQuotes(..)
{
int iPos=0,iEndPos;
int iLen;
int iLenParam;

iLen=csStr.length();
if(iLen<3) return 0;

iLenParam=param.length();
if(iLenParam==0) return 0;

iPos=csStr.find(param,iPos);	
if(iPos==string::npos) return 0;
iPos+=param.length();							//skip over 'param='

iEndPos=csStr.find(",",iPos);					//find start of next param if any
if(iEndPos==string::npos)						//no ending comma?
	{
	iEndPos=iLen;	
	}
				
equ=csStr.substr(iPos,iEndPos-iPos);

return 1;	
}










//given a specified delimited parameter string list e.g. 'mode=play!!position=1234!!status=ok'
//here the delimiter is: "!!"
//find param and extract its equate value i.e, param: 'mode='   equate: 'play' (note the equal sign was included in the param, otherwise it would end up in the equ value, like this: equate: '=play'
//there is no need for the equal sign to exist or seperate param from equate value
//i.e. 'type01!!count01!!autoyes'  param: 'count' would yield  equ: '01'
//returns 1 if successful, else 0;

//another e.g, where the delimit is: "\n"
//chirp=00,2a,d4,32,b2,12,25,14,02,e1,c5,02,5f,5a,05,0f,26,fc,a5,a5,d6,dd,dc,fc,25,2b,22,21,0f,ff,f8,ee,ed,ef,f7,f6,fa,00,03,02,01,00,00,00,00,00,00,00,00,00
//energy=00,00,00 01,01,02,03,05,07,0a,0f,15,1e,2b,3d,56,00
//pitch=00
//k0=00
//k1=00
//k2=00

//if( m1.ExtractParamVal_with_delimit( "chirp=", "\n", s1 ) )
//	{
//	printf("chirp= '%s'\n", s1.c_str() );
//	}

//if( m1.ExtractParamVal_with_delimit( "energy=", "\n", s1 ) )
//	{
//	printf("energy= '%s'\n", s1.c_str() );
//	getchar();
//	}


int mystr::ExtractParamVal_with_delimit(string param, string delim, string &equ)		//<---------------- see also:  ExtractParamValNotingUserQuotes(..)
{
int iPos=0,iEndPos;
int iLen;
int iLenParam;

iLen=csStr.length();
if(iLen<3) return 0;

iLenParam=param.length();
if(iLenParam==0) return 0;

iPos=csStr.find(param,iPos);	
if(iPos==string::npos) return 0;
iPos+=param.length();							//skip over 'param='

iEndPos=csStr.find( delim, iPos);					//find start of next param if any
if(iEndPos==string::npos)						//no ending comma?
	{
	iEndPos=iLen;	
	}
				
equ=csStr.substr(iPos,iEndPos-iPos);

if( equ.length() == 0 ) return 0;										//v1.81

return 1;	
}







/*

for( int i = 0; i < iLen; i++ )
	{
	char c1, c2;
	c1 = csStr[ i ];										//get two sequential chars
	if( i < ( iLen - 1 ) ) c2 = csStr[ i + 1 ];
	else c2 = 0;
	
	if( in_equ )											//not in a param, i.e: a param match happened?
		{
		if( c1 == ch_quote  )								//quote?
			{
			if ( c2 == ch_quote )
				{
				equ += c1;									//store only one of the two quote pairs
				i++;
//				printf("i: %d\n", i );										//skip second quote
				}
			else{
				in_quote = !in_quote;						//single quote, toggle flag
				if( !in_quote )
					{
					if( c2 == delimit ) break;				//delimiter reached?, finish.
					}
				} 
			}
		else{
			if( !in_quote )
				{
				if( c1 == delimit ) break;					//delimiter reached?, finish.
				}
			equ += c1;										//store char as in quote
			}
		}
	else{
		sparam += c1;										//build up possible param
		if( sparam.find( param, 0 ) != string::npos )       //param match?
			{
			in_equ = 1;
			}
		}
	}
*/









//note: modifed version of ExtractParamVal(..) to handle commas in equate value, via a 'ch_quote' mechanism,
//given an comma delimited parameter string list e.g. 'mode=play,position=1234,status=ok'
//find param and extract its equate value i.e, param: 'mode='   equate: 'play' (note the equal sign was included in the param, otherwise it would end up in the equ value, like this: equate: '=play'

//there is no need for the equal sign to exist or seperate param from equate value,
//i.e. 'type01,count01,autoyes'  param: 'count' would yield  equ: '01'

//this version will also treat equ values (i.e: after the param) that are between the 'ch_quotes' as not containing comma delimiters, this
//allows commas to be in the equ value,
//use a paired 'ch_quotes' ("") if you need to specify a single 'ch_quote'

//text1=this is a test1 ok1, text2=this is a test2 ok2	<--- normal, no 'ch_quote'

//text1=this is one whole equ value, that text1 param equates with, text2=this is a test2 ok2  <-!!----- this will cause a problem as the comma will mark end of the equ value
//text1="this is one whole equ value, that text1 param equates with", text2=this is a test2 ok2   <----- the equ returns no pre/post 'ch_quotes' like this ---> this is one whole equ value, that text1 param equates with
//text1="this is one ""whole"" equ value, that text1 param equates with", text2=this is a test2 ok2  <-- the equ returns no pre/post 'ch_quotes' like this ---> this is one "whole" equ value, that text1 param equates with
//text1=this is one ""whole"" equ value, text2=this is a test2 ok2   -- this just has one quote either side of 'whole', like this ---> this is one "whole" equ value

//text1=this is a te"st1 ok1, text2=this is a test2 ok2 <-!!- don't have a single 'ch_quote', single quotes should have a closing somewhere later

//text1="this is a ""test1"", ok1", text2="this is a ""test2"", ok2"

//"te,xt1"=this is a test1 ok1, text2=this is a test2 ok2 <--- this will also work if param spec was 'te,xt1', so 'ch_quotes' will also work in the param as well
//"te""xt1"=this is a test1 ok1, text2=this is a test2 ok2 <--- this will also work if param spec was 'te"xt1', so 'ch_quotes' will also work in the param as well

//returns 1 if successful, else 0;
int mystr::ExtractParamValNotingUserQuotes( string param, char delimit, char ch_quote, string &equ )
{

int iPos=0, iEndPos;
int iLen;
int iLenParam;

iLen = csStr.length();
if( iLen < 3 ) return 0;

iLenParam = param.length();
if( iLenParam == 0 ) return 0;

bool in_quote = 0;
//bool in_doub_quote = 0;
bool in_equ = 0;

bool match = 0;

string sparam;
equ = "";

for( int i = 0; i < iLen; i++ )
	{
	char c1, c2;
	c1 = csStr[ i ];										//get two sequential chars
	if( i < ( iLen - 1 ) ) c2 = csStr[ i + 1 ];
	else c2 = 0;
	
	if( in_equ )											//not in a param, i.e: a param match happened?
		{
		if( c1 == ch_quote  )								//quote?
			{
			if( c2 == ch_quote )
				{
				equ += c1;									//store only one of the two quote pairs
				i++;
//				printf("i: %d\n", i );										//skip second quote
				}
			else{
				in_quote = !in_quote;						//single quote, toggle flag
				if( !in_quote )
					{
					if( c2 == delimit ) break;				//delimiter reached?, finish
					}
				} 
			}
		else{
			if( !in_quote )
				{
				if( c1 == delimit ) break;					//delimiter reached?, finish
				}
			equ += c1;										//store char as in quote
			}
		}
	else{
		if( c1 == ch_quote  )								//quote?
			{
			if( c2 == ch_quote )
				{
                sparam += c1;										//build up possible param
                if( sparam.find( param, 0 ) != string::npos )       //param match?
                    {
                    in_equ = 1;
                    }
				i++;
				}
			else{
				in_quote = !in_quote;						//single quote, toggle flag
				if( !in_quote )
					{
					if( c2 == delimit )                     //delimiter reached?, clear the built param
                        {
                        sparam = "";
                        }
					}
				} 
			}
		else{
			if( !in_quote )
				{
				if( c1 == delimit )                         //delimiter reached?, clear the built param
                    {
                    sparam = "";
                    }
				}
			sparam += c1;										//store char as in quote
            if( sparam.find( param, 0 ) != string::npos )       //param match?
                {
                in_equ = 1;
                }
			}
		}
	}
/*
iPos = csStr.find( param, iPos );	
if( iPos == string::npos ) return 0;
iPos += param.length();							//skip over 'param='

iEndPos = csStr.find( ",", iPos );				//find start of next param if any
if( iEndPos == string::npos )						//no ending comma?
	{
	iEndPos = iLen;	
	}

equ = csStr.substr( iPos, iEndPos - iPos );
*/

return 1;	
}


















/*


void test_extract_param_vals()
{
string s1;

s1 = 	"inT= 24 ,-45														//signed int\n";
s1 +=	"unsIgned_int= 1, -1, 2												//unsigned int \n";
s1 +=	"hEx =0,2A,B3,D,e,1f												//hex\n";
s1 +=	"long_long_int = 0,1,-9223372036854775808, 9223372036854775807		//long long int\n";
s1 +=	"unsigned_long_long_int = 0,1,2, 18446744073709551615				//unsigned long long int\n";
s1 +=	"float=160e3, 0.1, -1e-30, 1e38, 1e39														//float\n";
s1 +=	"double=-60e-3, 1e3, 0.1,-5, 1e-150, 1.7e308, 1.8e308				//double\n";
s1 +=	"long_double = 100.1, 10.2, 0.1e-100, 1.7e308, 1.8e1024				//long double\n";
s1 +=	"char=a,b,c,1,2,3,+,-,*, ,x,\t,y,\r,z								//char, is able to store or reject, whitespaces and '\r' \n";		
s1 +=	"/?string =This is commented out,1234,hello, ,x,\t,y,\r,z?/			//string, has been commented out '\r'\n";		
s1 +=	"/??string =This is commented out,1234,hello, ,x,\t,y,\r,z?/			//string, has been commented out '\r'\n";		
s1 +=	"string =I was here,1234,hello, ,x,\t,y,\r,z						//string, is able to store or reject, whitespaces and '\r'\n";		

int ibuf[ 100 ];
unsigned int uibuf[ 100 ];
long long int llibuf[ 100 ];
unsigned long long int ullibuf[ 100 ];
float fbuf[ 100 ];
double dbuf[ 100 ];
long double ldbuf[ 100 ];
char cbuf[ 100 ];
string strbuf[ 100 ];

int count = 0;
mystr m1 = s1;
int pos = 0;

//test int - type 0
if( m1.extract_param_vals( "int", 1, 1, 1, 1, 0, (void*)ibuf, 100, count, pos ) == 0 )
	{
	printf( "int - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "int - got val: %d\n", ibuf[ i ] );
	}
printf( "---- int - pos: %d ----\n", pos );



//test unsigned_int - type 1
pos = 0;
if( m1.extract_param_vals( "unsigned_int", 1, 1, 1, 1, 1, (void*)uibuf, 100, count, pos ) == 0 )
	{
	printf( "unsigned_int - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "unsigned_int - got val: %u\n", uibuf[ i ] );
	}
printf( "---- unsigned_int - pos: %d ----\n", pos );




//test unsigned_long_long_int - type 2
pos = 0;
if( m1.extract_param_vals( "hex", 1, 1, 1, 1, 2, (void*)uibuf, 100, count, pos ) == 0 )
	{
	printf( "hex - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "hex - got val: %x\n", uibuf[ i ] );
	}
printf( "---- hex - pos: %d ----\n", pos );


//test long_long_int - type 3
pos = 0;
if( m1.extract_param_vals( "long_long_int", 1, 1, 1, 1, 3, (void*)llibuf, 100, count, pos ) == 0 )
	{
	printf( "long_long_int - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "long_long_int - got val: %Ld\n", llibuf[ i ] );
	}
printf( "---- long_long_int - pos: %d ----\n", pos );



//test unsigned_long_long_int - type 4
pos = 0;
if( m1.extract_param_vals( "unsigned_long_long_int", 1, 1, 1, 1, 4, (void*)ullibuf, 100, count, pos ) == 0 )
	{
	printf( "unsigned_long_long_int - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "unsigned_long_long_int - got val: %"LLU"\n", ullibuf[ i ] );
	}
printf( "---- unsigned_long_long_int - pos: %d ----\n", pos );



//test float - type 5
pos = 0;
if( m1.extract_param_vals( "float", 1, 1, 1, 1, 5, (void*)fbuf, 100, count, pos ) == 0 )
	{
	printf( "float - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "float - got val: %g\n", fbuf[ i ] );
	}
printf( "---- float - pos: %d ----\n", pos );


//test double - type 6
pos = 0;
if( m1.extract_param_vals( "double", 1, 1, 1, 1, 6, (void*)dbuf, 100, count, pos ) == 0 )
	{
	printf( "double - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "double - got val: %lg\n", dbuf[ i ] );
	}
printf( "---- double - pos: %d ----\n", pos );


//test long double - type 7
pos = 0;
if( m1.extract_param_vals( "long_double", 1, 1, 1, 1, 7, (void*)ldbuf, 100, count, pos ) == 0 )
	{
	printf( "long_double - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "long_double - got val: %Lg\n", ldbuf[ i ] );
	}
printf( "---- long_double - pos: %d ----\n", pos );



//test char - type 8
pos = 0;
if( m1.extract_param_vals( "char", 1, 1, 1, 1, 8, (void*)cbuf, 100, count, pos ) == 0 )
	{
	printf( "char - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "char - got val: %c\n", cbuf[ i ] );
	}
printf( "---- char - pos: %d ----\n", pos );


//test string - type 9
pos = 0;
if( m1.extract_param_vals( "string", 1, 1, 1, 1, 9, (void*)strbuf, 100, count, pos ) == 0 )
	{
	printf( "string - failed\n" );
	return;
	}

for( int i = 0; i < count; i ++ )
	{
	printf( "string - got val: %s\n", strbuf[ i ].c_str() );
	}
printf( "---- string - pos: %d ----\n", pos );

}

*/






//!!see usage example above: -- test_extract_param_vals()
//given a multiline, comma delimited parameter string list e.g:
//mode = 0,1,2,3,4,5
//quiet=1
//users = fred,tom,ken				//white space before equal is ignored
//cutlery=spoon,knife,fork			//string vals can't return a comma, as it is the delimitter
//freq=10,100,1e3,1e4,				//c like comments can be set to be ignored
//delay=1e-6, 0.001, -5

//and given 'type' of var, and pointer 'valptr' to an array of that type, it will
//find a line with matching param_name and extract all its values and store in 'valptr'
//white spaces are ignored expcept when 'type' is a string (8) 
//there must be an equal sign to seperate param from equate values
//i.e. 'mode = 0,2,3,4,5' would result in 0,2,3,4,5 being stored in successive 'valptr' locations 
//vals are considered to have finished at next '\n'
//will stop attemting to extract if 'stop_at_str' is found on left of equal sign
//!! beware that unicode chars (neg chars) are not handled and could be whited out.

//'param_name' must be a string
//'ignore_case' will compare uppercase converted 'param' to prospective uppercase converted param on each line
//'ignore_case' --it will not convert any returned string values to uppercase though
//'skip_whitespace' will not store ' ' and '\t' in char and string types
//'skip_cr' will not store '\r' in char and string types
//if 'type' = 0 will extract ints
//if 'type' = 1 will extract unsigned int
//if 'type' = 2 will extract 32bit hex
//if 'type' = 3 will extract long long int
//if 'type' = 4 will extract unsigned long long int
//if 'type' = 5 will extract float
//if 'type' = 6 will extract double
//if 'type' = 7 will extract long double
//if 'type' = 8 will extract char
//if 'type' = 9 will extract std::string

//'maxcnt' - is max number of values to extract and store in 'valptr'
//'valcnt' - is number of values extracted and stored in 'valptr'
//'pos' is starting position in mystr, it will be modified to last mystr pos tested
//'observe_c_commenting' - if set C and CPP comments methods are observed: ie: // and /*  */

//returns 1 if an entry is extracted
//returns 0 if entry matches 'stop_at_str' (before equal sign), note: pos will not be changed
//returns -1 if no matching 'param_name'
//returns -2 if incorrect vars supplied
int mystr::extract_param_vals( string param_name, bool ignore_case, bool skip_whitespace, bool skip_cr, bool observe_c_commenting, bool allow_unicode_chars, int type, void *valptr, int maxcnt, int &valcnt, int &pos, string stop_at_str )
{
string stotal;
int iPos=0,iEndPos;
int len;
int iLenParam;
bool in_val = 0;
bool in_slash_cmnt = 0;				// '//' comment
bool in_splat_cmnt = 0;				// '/*..*/' comment
bool skip_line = 0;
bool last_check_val = 0;
bool bcheck_for_stop_str = 0;

if( pos < 0 )  return -2;

valcnt = 0;
if( maxcnt <= 0 ) return -2;

stotal = csStr;
stotal += "\n";						//ensure there is an ending LF

len = csStr.length();				//nothing in mystr string?
if( len < 3 ) return -1;			//not enough in string?

if( pos >= len ) return -2;

int plen = param_name.size();
if( plen == 0 ) return -1;


int stop_at_str_len = stop_at_str.length();
if( stop_at_str_len > 0 ) bcheck_for_stop_str = 1;		//flag we want to check if we are at a 'stop_str'

int *iptr;
unsigned int *uiptr;
long long int *lliptr;
unsigned long long int *ulliptr;
float *fptr;
double *dptr;
long double *ldptr;
char *chptr;
string *strptr;
string stext;						//holds fetched chars

switch ( type )
	{
	case 0:
		iptr = (int*)valptr;
	break;

	case 1:
		uiptr = (unsigned int*)valptr;
	break;

	case 2:
		uiptr = (unsigned int*)valptr;
	break;

	case 3:
		lliptr = (long long int*)valptr;
	break;

	case 4:
		ulliptr = (unsigned long long int*)valptr;
	break;

	case 5:
		fptr = (float*)valptr;
	break;

	case 6:
		dptr = (double*)valptr;
	break;

	case 7:
		ldptr = (long double*)valptr;
	break;

	case 8:
		chptr = (char*)valptr;
	break;

	case 9:
		strptr = (string*)valptr;
	break;

	default:
		printf("mystr::extract_param_vals() - 'type' out of range\n" );
	return -2;
	}

char c1, c2;


if( ignore_case )									//convert param name to uppercase
	{

	for( int i = 0; i < plen; i++ )
		{
		char c = toupper( param_name[ i ] );
		stext += c;
		}

	param_name = stext;


	if( bcheck_for_stop_str )						//also convert 'stop_str' to uppercase
		{
		stext = "";
		for( int i = 0; i < stop_at_str_len; i++ )
			{
			char c = toupper( stop_at_str[ i ] );
			stext += c;
			}
		}

	stop_at_str = stext;
	}

stext = "";

int i;
for( i = pos; i < len; i++ )										//cycle through string
	{
	char c1 = stotal[ i ];
	if( ( i + 1 ) < len ) c2 = stotal[ i + 1 ];						//get next char if any
	else c2 = 0;

	if( !allow_unicode_chars )
		{
		if( c1 < 0x0 ) c1 = ' ';									//white out neg or unicode chars
		if( c2 < 0x0 ) c2 = ' ';									//eg: minus may be 0xe2 0x88, if cut from a webpage
		}

	if( ( ignore_case ) && ( !in_val  ) )				//need uppercase and still in param_name?
		{
		if( ( c1 >= 'a' ) && ( c1 <= 'z'  ) ) c1 = c1 & 0xdf;		//make uppercase
		if( ( c2 >= 'a' ) && ( c2 <= 'z'  ) ) c2 = c2 & 0xdf;		//make uppercase
		}

	if( c1 == '\n' )
		{
		in_slash_cmnt = 0;									//clear single line comment flg
		if( in_val )
			{
			last_check_val = 1;		//flag this is final check_val to be done before exit
			goto check_val;			//this check_val call ensures a solitary value without following a comma is detected and picked up
			}
		stext = "";
		skip_line = 0;
 		continue;
		}

	if( skip_line ) continue;								//skip till new line

	if( observe_c_commenting )
		{
		//handle '/*...*/' comments
		if( in_splat_cmnt )
			{
			if( ( c1 == '*' ) && ( c2 == '/' ) )						//ending comment?
				{
				in_splat_cmnt = 0;
				continue;
				}
			continue;									//skip processing commented chars	
			}
		else{
			if( ( c1 == '/' ) && ( c2 == '*' ) )					//start of comment?
				{
				in_splat_cmnt = 1;
				continue;
				}	
			}

		if( in_slash_cmnt )
			{
			continue;
			}
		else{
			if( ( c1 == '/' ) && ( c2 == '/' ) )		//single line comment?
				{
				in_slash_cmnt = 1;
				continue;
				}
			}
		}


	if( !in_val )									//not found param string yet			
		{
		if( ( c1 == ' ' ) || ( c1 == '\t' ) || ( c1 == '\r' ) )
			{
			continue;								//skip processing white space chars	
			}

		if( c1 == '=' )								//param equate symbol
			{
			if( stext.size() != 0 )
				{
				if( bcheck_for_stop_str )					//need to check for 'stop_at_str'?
					{
//					if( stext.find( stop_at_str ) != string::npos )	//check if param matching 'stop_at_str'
					if( stext.compare( stop_at_str ) == 0 )	//check if param matching 'stop_at_str'
						{									//v1.30 changed from find to compare
						return 0;
						}
					}

				if( param_name.compare( stext ) == 0 )	//found a match?
					{
					in_val = 1;						//flag to start collecting a value
					stext = "";
					continue;
					}
				else{
					skip_line = 1;
					continue;
					}
				}
			} 

		if( skip_cr )
			{
			if( c1 == '\r' ) continue;
			} 
		stext += c1;					//build param_name text for match checking
		continue;
		}
	else{
		if( c1 == ',' )								//value boundary
			{
			check_val:
			//convert val as required and store
			if( 1 )//stext.size() != 0 )				
				{
				switch ( type )
					{
					case 0:
						sscanf( stext.c_str(),"%d", iptr + valcnt );	//int
					break;

					case 1:
						sscanf( stext.c_str(),"%u", uiptr + valcnt );	//unsigned int
					break;

					case 2:
						sscanf( stext.c_str(),"%x", uiptr + valcnt );	//32 bit hex
					break;

					case 3:
						sscanf( stext.c_str(),"%Ld", lliptr + valcnt );	//long long int
					break;

					case 4:
						sscanf( stext.c_str(),"%" LLU, ulliptr + valcnt ); //unsigned long long int
					break;

					case 5:
						sscanf( stext.c_str(),"%g", fptr + valcnt );	//float
					break;

					case 6:
						sscanf( stext.c_str(),"%lg", dptr + valcnt );	//double
					break;

					case 7:
						sscanf( stext.c_str(),"%Lg", ldptr + valcnt );	//long double
					break;

					case 8:
						sscanf( stext.c_str(),"%c", chptr + valcnt );
					break;

					case 9:
						strptr[ valcnt ] = stext;
					break;

					default:								//can't get here
					break;
					}
				valcnt++;
				stext = "";
				if ( valcnt >= maxcnt ) goto done;
				if ( last_check_val ) goto done;
				}
//			else{
//				goto done;
//				}
			}
		else{
			if( skip_cr )
				{
				if( c1 == '\r' ) continue;
				}

			if( skip_whitespace )
				{
				if( ( c1 == ' ' ) || ( c1 == '\t' ) ) continue;
				}

			stext += c1;									//build value text
			continue;
			}
		}

	}

//pos = i - 1;						//if here no match found and end of string reached
//return -2;

done:
pos =  i;

if( valcnt > 0 ) return 1;
return -1;	
}









//!!see usage example above: -- test_extract_param_vals()

//same as extract_param_vals(), but also collects the params as strings in vraw
//v1.46 now ignores 'skip_whitespace' and 'skip_cr' flags, so vraw will capture these chars (only affects vraw)
int mystr::extract_param_vals_and_raw( string param_name, bool ignore_case, bool skip_whitespace, bool skip_cr, bool observe_c_commenting, bool allow_unicode_chars, int type, void *valptr, int maxcnt, int &valcnt, int &pos, string stop_at_str, vector<string> &vraw )
{
string stotal;
int iPos=0,iEndPos;
int len;
int iLenParam;
bool in_val = 0;
bool in_slash_cmnt = 0;				// '//' comment
bool in_splat_cmnt = 0;				// '/*..*/' comment
bool skip_line = 0;
bool last_check_val = 0;
bool bcheck_for_stop_str = 0;

if( pos < 0 )  return -2;

valcnt = 0;
if( maxcnt <= 0 ) return -2;

vraw.clear();

stotal = csStr;
stotal += "\n";						//ensure there is an ending LF

len = csStr.length();				//nothing in mystr string?
if( len < 3 ) return -1;			//not enough in string?

if( pos >= len ) return -2;

int plen = param_name.size();
if( plen == 0 ) return -1;


int stop_at_str_len = stop_at_str.length();
if( stop_at_str_len > 0 ) bcheck_for_stop_str = 1;		//flag we want to check if we are at a 'stop_str'

int *iptr;
unsigned int *uiptr;
long long int *lliptr;
unsigned long long int *ulliptr;
float *fptr;
double *dptr;
long double *ldptr;
char *chptr;
string *strptr;
string stext;						//holds fetched chars
string sraw;                        //added in v1.46 

switch ( type )
	{
	case 0:
		iptr = (int*)valptr;
	break;

	case 1:
		uiptr = (unsigned int*)valptr;
	break;

	case 2:
		uiptr = (unsigned int*)valptr;
	break;

	case 3:
		lliptr = (long long int*)valptr;
	break;

	case 4:
		ulliptr = (unsigned long long int*)valptr;
	break;

	case 5:
		fptr = (float*)valptr;
	break;

	case 6:
		dptr = (double*)valptr;
	break;

	case 7:
		ldptr = (long double*)valptr;
	break;

	case 8:
		chptr = (char*)valptr;
	break;

	case 9:
		strptr = (string*)valptr;
	break;

	default:
		printf("mystr::extract_param_vals() - 'type' out of range\n" );
	return -2;
	}

char c1, c2;


if( ignore_case )									//convert param name to uppercase
	{

	for( int i = 0; i < plen; i++ )
		{
		char c = toupper( param_name[ i ] );
		stext += c;
        sraw += c;
		}

	param_name = stext;


	if( bcheck_for_stop_str )						//also convert 'stop_str' to uppercase
		{
		stext = "";
        sraw = "";
		for( int i = 0; i < stop_at_str_len; i++ )
			{
			char c = toupper( stop_at_str[ i ] );
			stext += c;
            sraw += c;
			}
		}

	stop_at_str = stext;
	}

stext = "";
sraw = "";

int i;
for( i = pos; i < len; i++ )										//cycle through string
	{
	char c1 = stotal[ i ];
	if( ( i + 1 ) < len ) c2 = stotal[ i + 1 ];						//get next char if any
	else c2 = 0;

	if( !allow_unicode_chars )
		{
		if( c1 < 0x0 ) c1 = ' ';									//white out neg or unicode chars
		if( c2 < 0x0 ) c2 = ' ';									//eg: minus may be 0xe2 0x88, if cut from a webpage
		}

	if( ( ignore_case ) && ( !in_val  ) )				//need uppercase and still in param_name?
		{
		if( ( c1 >= 'a' ) && ( c1 <= 'z'  ) ) c1 = c1 & 0xdf;		//make uppercase
		if( ( c2 >= 'a' ) && ( c2 <= 'z'  ) ) c2 = c2 & 0xdf;		//make uppercase
		}

	if( c1 == '\n' )
		{
		in_slash_cmnt = 0;									//clear single line comment flg
		if( in_val )
			{
			last_check_val = 1;		//flag this is final check_val to be done before exit
			goto check_val;			//this check_val call ensures a solitary value without following a comma is detected and picked up
			}
		stext = "";
        sraw = "";
		skip_line = 0;
 		continue;
		}

	if( skip_line ) continue;								//skip till new line

	if( observe_c_commenting )
		{
		//handle '/*...*/' comments
		if( in_splat_cmnt )
			{
			if( ( c1 == '*' ) && ( c2 == '/' ) )						//ending comment?
				{
				in_splat_cmnt = 0;
				continue;
				}
			continue;									//skip processing commented chars	
			}
		else{
			if( ( c1 == '/' ) && ( c2 == '*' ) )					//start of comment?
				{
				in_splat_cmnt = 1;
				continue;
				}	
			}

		if( in_slash_cmnt )
			{
			continue;
			}
		else{
			if( ( c1 == '/' ) && ( c2 == '/' ) )		//single line comment?
				{
				in_slash_cmnt = 1;
				continue;
				}
			}
		}


	if( !in_val )									//not found param string yet			
		{
		if( ( c1 == ' ' ) || ( c1 == '\t' ) || ( c1 == '\r' ) )
			{
			continue;								//skip processing white space chars	
			}

		if( c1 == '=' )								//param equate symbol
			{
			if( stext.size() != 0 )
				{
				if( bcheck_for_stop_str )					//need to check for 'stop_at_str'?
					{
//					if( stext.find( stop_at_str ) != string::npos )	//check if param matching 'stop_at_str'
					if( stext.compare( stop_at_str ) == 0 )	//check if param matching 'stop_at_str'
						{									//v1.30 changed from find to compare
						return 0;
						}
					}

				if( param_name.compare( stext ) == 0 )	//found a match?
					{
					in_val = 1;						//flag to start collecting a value
					stext = "";
                    sraw = "";
					continue;
					}
				else{
					skip_line = 1;
					continue;
					}
				}
			} 

		if( skip_cr )
			{
			if( c1 == '\r' )
                {
                sraw += c1;
                continue;
                }
			}
		stext += c1;					//build param_name text for match checking
        sraw += c1;
		continue;
		}
	else{
		if( c1 == ',' )								//value boundary
			{
			check_val:
			//convert val as required and store
			if ( 1 )// stext.size() != 0 )				
				{
				vraw.push_back( sraw );			//store raw value
				switch ( type )
					{
					case 0:
						sscanf( stext.c_str(),"%d", iptr + valcnt );	//int
					break;

					case 1:
						sscanf( stext.c_str(),"%u", uiptr + valcnt );	//unsigned int
					break;

					case 2:
						sscanf( stext.c_str(),"%x", uiptr + valcnt );	//32 bit hex
					break;

					case 3:
						sscanf( stext.c_str(),"%Ld", lliptr + valcnt );	//long long int
					break;

					case 4:
						sscanf( stext.c_str(),"%" LLU, ulliptr + valcnt ); //unsigned long long int
					break;

					case 5:
						sscanf( stext.c_str(),"%g", fptr + valcnt );	//float
					break;

					case 6:
						sscanf( stext.c_str(),"%lg", dptr + valcnt );	//double
					break;

					case 7:
						sscanf( stext.c_str(),"%Lg", ldptr + valcnt );	//long double
					break;

					case 8:
						sscanf( stext.c_str(),"%c", chptr + valcnt );
					break;

					case 9:
						strptr[ valcnt ] = stext;
					break;

					default:								//can't get here
					break;
					}
				valcnt++;
				stext = "";
                sraw = "";
				if ( valcnt >= maxcnt ) goto done;
				if ( last_check_val ) goto done;
				}
//			else{
//				goto done;
//				}
			}
		else{
			if( skip_cr )
				{
				if( c1 == '\r' )
                    {
                    sraw += c1;
                    continue;
                    }
				}

			if( skip_whitespace )
				{
				if( ( c1 == ' ' ) || ( c1 == '\t' ) )
                    {
                    sraw += c1;                             //added in v1.46
                    continue;
                    }
				}

			stext += c1;									//build value text
            sraw += c1;
			continue;
			}
		}

	}

//pos = i - 1;						//if here no match found and end of string reached
//return -2;

done:
pos =  i;

if( valcnt > 0 ) return 1;
return -1;	
}








int mystr::str_to_int( string ss )
{
int ii;

sscanf( ss.c_str(), "%d", &ii );
return ii;
}


//use a typical prinf format, like: "%d"
void mystr::int_to_str( string &ss, int ii, string prinf_format )
{
ss = "";

strpf( ss, prinf_format.c_str(), ii );
}









double mystr::str_to_double( string ss )
{
double dd;

sscanf( ss.c_str(), "%lf", &dd );
return dd;
}



//use a typical prinf format, like:  "f", "%g", "e"
//NOTE that a 'float' is promoted to a double automatically in c code, which if why: "%f, %g %e" work with both 'float' and 'double' types
void mystr::double_to_str( string &ss, double dd, string prinf_format )
{
ss = "";

strpf( ss, prinf_format.c_str(), dd );
}











//NOTE: does not work correctly with mingw/gcc, so have down cast to a double for windows
long double mystr::str_to_long_double( string ss )
{
long double dd;
double d;

//linux code
#ifndef compile_for_windows
	sscanf( ss.c_str(), "%Lf", &dd );
//d=9876;
//	dd = d;
#endif


//windows code
#ifdef compile_for_windows
	sscanf( ss.c_str(), "%lf", &d );		//for some reason mingw/gcc does not work with "%Lf", kept getting NAN
	dd = d;
#endif

return dd;
}



//use a typical prinf format, like: "%Lf", "Lg", "Le"
void mystr::long_double_to_str( string &ss, long double dd, string prinf_format )
{
ss = "";

strpf( ss, prinf_format.c_str(), dd );
}













//From iStartPos find and replace sequential occurrences of non visible (<21Hx) with string csReplace
//keep doing this till a visible char is found.
//returns 1 if a replace was performed, else 0 
//e.g. csReplace="", and iStartPos=0, then it would return a string that would have no leading non visible/white chars
bool mystr::ReplaceSeqNonVisible(string &csRet,string csReplace,int iStartPos)
{
int i;

if(iLen==0) return 0;

if(iStartPos>=iLen) return 0;

for(i=iStartPos;i<iLen;i++)
	{
	if(csStr[i]>' ') break;	//non white & visible?
	}

if(i==iStartPos) return 0;							//found non visible first pos?

//printf("\niPos=%d,iStartPos=%d",i,iStartPos);
csStr.erase(iStartPos,i-iStartPos);					//erase seq chars found
csStr.insert(iStartPos,csReplace);					//insert the 'replace' str

//string csTmp;
//csTmp=csStr.substr(0,100);
//printf("\n%s\n",csTmp.c_str());


csRet=csStr;
return 1;	
}	







//From iStartPos find and replace sequential occurrences of char 'c' with string csReplace
//keep doing this till a char not equal to 'c' is found.
//returns 1 if a replace was performed, else 0 
//e.g. if 'c' is a space, csReplace="", and iStartPos=0, then it would return a string that would have no leading spaces
bool mystr::ReplaceSeqCharOccurrence(string &csRet,char c,string csReplace,int iStartPos)
{
if (iLen==0) return 0;
if(iStartPos<0) return 0;
if(iStartPos>=iLen) return 0;

int iPos=csStr.find_first_not_of(c,iStartPos);

if(iPos<0) return 0;
if(iPos==0) return 0;

//printf("\niPos=%d,iStartPos=%d",iPos,iStartPos);
csStr.erase(iStartPos,iPos-iStartPos);				//erase seq chars found
csStr.insert(iStartPos,csReplace);					//insert the 'replace' str

csRet=csStr;
return 1;	
}	





//From iStartPos find next csFind and replace it with csReplace
//keep doing find/replace end is reached
//returns 1 if a replace happened, else 0
//puts modified result in str csRet
bool mystr::FindReplace(string &csRet,string csFind,string csReplace,int iStartPos)
{
int iRet=0,iLastPos;
int iFindLen=csFind.length();
int iReplaceLen=csReplace.length();

csRet = csStr;											//added 23-07-11

if (iLen==0) return 0;
if(iStartPos<0) return 0;
if(iStartPos>=iLen) return 0;
if(iFindLen==0) return 0;


iLastPos=iStartPos;
while(1)
	{
	int iPos=csStr.find(csFind,iLastPos);				//match found?
	if(iPos<0)
		{
//		csRet=csStr;
		break;
		}
	else iRet=1;										//an OK find has happened

	csStr.erase(iPos,iFindLen);							//erase the found str
	csStr.insert(iPos,csReplace);						//insert the 'replace' str
	iLastPos=iPos+iReplaceLen;
//	if(iLastPos>=iLen) break;							//changed 06-07-10
	}

csRet=csStr;
return iRet;
}













//From iStartPos find next 'ch' and replace it with csReplace
//keep doing find/replace end is reached
//returns 1 if a replace happened, else 0
//puts modified result in str csRet
bool mystr::FindReplace(string &csRet, char ch, string csReplace, int iStartPos)
{
int iRet=0,iLastPos;

string csFind;

csFind += ch;

int iFindLen=csFind.length();
int iReplaceLen=csReplace.length();

csRet = csStr;											//added 23-07-11

if (iLen==0) return 0;
if(iStartPos<0) return 0;
if(iStartPos>=iLen) return 0;
if(iFindLen==0) return 0;


iLastPos=iStartPos;
while(1)
	{
	int iPos=csStr.find(csFind,iLastPos);				//match found?
	if(iPos<0)
		{
//		csRet=csStr;
		break;
		}
	else iRet=1;										//an OK find has happened

	csStr.erase(iPos,iFindLen);							//erase the found str
	csStr.insert(iPos,csReplace);						//insert the 'replace' str
	iLastPos=iPos+iReplaceLen;
//	if(iLastPos>=iLen) break;							//changed 06-07-10
	}

csRet=csStr;
return iRet;
}












//from last pos known get next str up to LF, remove any LF/CR in ret str
//return str and 1 if found else 0
bool mystr::gets(string &csRet)
{
int iPos1,iTmp;

if(iLen==0) return 0;
if (iPos>=(iLen-1)) return 0;


iPos1=csStr.find("\n",iPos);		//find LF
csRet=csStr.substr(iPos,iPos1-iPos);


for(;;)								//clear any CRs
	{
	iTmp=csRet.find("\r",0);		//find a CR
	if(iTmp>=0) csRet.erase( iTmp, 1 ); //delete it -- v1.26 changed from csRet.erase(iTmp,iTmp), as this can cause lockup when line contains just one char, and that char is a CR
	else break; 
	}

for(;;)								//clear any LFs
	{
	iTmp=csRet.find("\n",0);		//find a LF
	if(iTmp>=0) csRet.erase(iTmp,iTmp); //delete it
	else break ;
	}


if(iPos1<0) iPos=iLen-1;			//no LF found, move to end
else iPos=iPos1+1;					//LF found move over it

if(iPos>=iLen) iPos=iLen-1;			//make sure not to move past end

if(iPos1<0) return 0;
return 1;							//added v1.26
}









//From iStartPos find end pos of nth single or sequential occurrence of char 'c',
//calcs the pos directly at end of the nth 'c' single or sequence.
//returns pos if nth new occurrence found, else -1.
//e.g. if 'c' is a space, and str contains below line, nth val would return pos as shown:

//nth=            1 2    3       4    5          6     7
//str=drwxr-xr-x  3 root root    4096 2005-08-19 00:04 Calc98CE
//    01234567890123456789012345678901234567890
//nth=1 rets 12,   nth=2 rets 14,   nth=3 rets 19   nth=4 rets 27,  nth=32
int mystr::FindNthSeqCharPosEnd(char c,int nth,int iStartPos)
{
int i,in_seq=0,nthcount=0;

if (iLen==0) return -1;
if(iStartPos<0) return -1;
if(iStartPos>=iLen) return -1;

if(nth<=0) return -1;


for(i=iStartPos;i<iLen;i++)
	{
	
	if(csStr[i]==c)			//matching 'c'?
		{
		if(in_seq==1)		//aleardy in a seq of 'c'
			{
			continue;		//carry on and ignore
			}
		else{
			in_seq=1;		//mark possible start of a seq
			nthcount++;		//keep track of newly found 'c'
			}
		
		}
	else{					//not matching 'c' if here
		in_seq=0;
		if(nthcount==nth) goto foundit;	//enough new occurrences of 'c' found?
		}
	}
	
return -1;
//printf("\niPos=%d,iStartPos=%d",iPos,iStartPos);

foundit:
return i;	
}	








//prepad out supplied string with spaces up to n, don't pass limit.
//will also shorten to pad length
//see also padstr_vector(..)
void mystr::prepadstr(string &sret,int pad,int lim)
{
string s1;

int len=iLen;

if (pad<0) return;
if (lim<0) lim=0;
if (pad>lim) pad=lim;



if( (len>pad) )
    {
    sret=csStr.substr(0,pad);      //supply smaller string than supplied
    return;
    }

if( (len>lim) )
    {
    sret=csStr.substr(0,lim);      //supply smaller string than supplied
    return;
    }

if(len<pad)
    {
    int delta=pad-len;
    for(int i=0;i<delta;i++)    	//pad out as req
        {
        s1+=" ";
        }
    s1+=csStr;                       //prefix padding
    sret=s1;
    }

}




//pad out supplied string with spaces up to n, don't pass limit.
//will also shorten to pad length
//see also padstr_vector(..)
void mystr::padstr(string &sret,int pad,int lim)
{
int len=iLen;

if (pad<0) return;
if (lim<0) lim=0;
if (pad>lim) pad=lim;

sret = csStr;


if( (len>pad) )
    {
    sret=csStr.substr(0,pad);      //supply smaller string than supplied
    return;
    }

if( (len>lim) )
    {
    sret=csStr.substr(0,lim);      //supply smaller string than supplied
    return;
    }

if(len<pad)
    {
    int delta=pad-len;
    for(int i=0;i<delta;i++)    //pad out as req
        {
        sret+=" ";
        }
    }

}









//pad out supplied string with spec. char up to n, don't pass limit.
//will also shorten to pad length

//see also padstr_vector(..)
void mystr::padstrchar(string &sret, char ch, int pad, int lim )
{
int len=iLen;

if (pad<0) return;
if (lim<0) lim=0;
if (pad>lim) pad=lim;

sret = csStr;

if( (len>pad) )
    {
    sret=csStr.substr(0,pad);      //supply smaller string than supplied
    return;
    }

if( (len>lim) )
    {
    sret=csStr.substr(0,lim);      //supply smaller string than supplied
    return;
    }

if(len<pad)
    {
    int delta=pad-len;
    for(int i=0;i<delta;i++)    //pad out as req
        {
        sret+=ch;
        }
    }

}















//refer below function - padstr_vector( string &sret, vector<st_mystr_padstr_vector> vprm, int tot_length, char c_end_pad )

//performs a padding out of strings held in supplied vector using their defined params - see struct 'st_mystr_padstr_vector'
//will limit final string if needed beyond 'tot_length'
//will pad final string if below the 'tot_length' size using 'c_end_pad'

//left justify example
//--------------------
//assuming tot_length = 110
//assuming c_end_pad = '+'
//if vector<st_mystr_padstr_vector>.str        = pear01234567890,  rice0123456    , what01234567890, hand01234567890, eyes01234567890
//if vector<st_mystr_padstr_vector>.c_justify  = ~              ,  !              , #              , ?              , ]
//if vector<st_mystr_padstr_vector>.i_justify  = 0              ,  0              , 0              , 0              , 0
//if vector<st_mystr_padstr_vector>.c_interpad = .              ,  .              , .              , .              , .
//if vector<st_mystr_padstr_vector>.i_column   = 5              ,  25             , 45             , 65             , 85
//if vector<st_mystr_padstr_vector>.i_column   = 17             ,  17             , 17             , 17             , 17

//left justify result
//-------------------
//0         1         2         3         4         5         6         7         8         9         10        11        12'
//01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901'
//.....pear01234567890~~...rice0123456!!!!!!...what01234567890##...hand01234567890??...eyes01234567890]]++++++++'





//right justify example
//--------------------
//assuming tot_length = 110
//assuming c_end_pad = '+'
//if vector<st_mystr_padstr_vector>.str        = pear01234567890,  rice0123456    , what01234567890, hand01234567890, eyes01234567890
//if vector<st_mystr_padstr_vector>.c_justify  = ~              ,  !              , #              , ?              , ]
//if vector<st_mystr_padstr_vector>.i_justify  = 1              ,  1              , 1              , 1              , 1
//if vector<st_mystr_padstr_vector>.c_interpad = .              ,  .              , .              , .              , .
//if vector<st_mystr_padstr_vector>.i_column   = 5              ,  25             , 45             , 65             , 85
//if vector<st_mystr_padstr_vector>.i_column   = 17             ,  17             , 17             , 17             , 17


//right justify result
//--------------------
//0         1         2         3         4         5         6         7         8         9         10        11        12'
//01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901'
//.....~~pear01234567890...!!!!!!rice0123456...##what01234567890...??hand01234567890...]]eyes01234567890++++++++'








//centre justify example
//----------------------
//assuming tot_length = 110
//assuming c_end_pad = '+'
//if vector<st_mystr_padstr_vector>.str        = pear01234567890,  rice0123456    , what01234567890, hand01234567890, eyes01234567890
//if vector<st_mystr_padstr_vector>.c_justify  = ~              ,  !              , #              , ?              , ]
//if vector<st_mystr_padstr_vector>.i_justify  = 2              ,  2              , 2              , 2              , 2
//if vector<st_mystr_padstr_vector>.c_interpad = .              ,  .              , .              , .              , .
//if vector<st_mystr_padstr_vector>.i_column   = 5              ,  25             , 45             , 65             , 85
//if vector<st_mystr_padstr_vector>.i_column   = 17             ,  17             , 17             , 17             , 17

//centre justify result
//----------------------
//0         1         2         3         4         5         6         7         8         9         10        11        12'
//01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901'
//.....~pear01234567890~...!!!rice0123456!!!...#what01234567890#...?hand01234567890?...]eyes01234567890]++++++++'



/* example coding for: padstr_vector( string &sret, vector<st_mystr_padstr_vector> vprm, int tot_length, char c_end_pad )
string s1;
mystr m1;

vector<st_mystr_padstr_vector> oo;

st_mystr_padstr_vector o;

o.str = "pear01234567890";
o.c_justify = '~';
o.i_justify = 0;
o.c_interpad = '.';
o.i_column = 5;
o.i_len = 17;
oo.push_back( o );

o.str = "rice0123456";
o.c_justify = '!';
o.i_justify = 0;
o.c_interpad = '.';
o.i_column = 25;
o.i_len = 17;
oo.push_back( o );

o.str = "what01234567890";
o.c_justify = '#';
o.i_justify = 0;
o.c_interpad = '.';
o.i_column = 45;
o.i_len = 17;
oo.push_back( o );

o.str = "hand01234567890";
o.c_justify = '?';
o.i_justify = 0;
o.c_interpad = '.';
o.i_column = 65;
o.i_len = 17;
oo.push_back( o );

o.str = "eyes01234567890";
o.c_justify = ']';
o.i_justify = 0;
o.c_interpad = '.';
o.i_column = 85;
o.i_len = 17;
oo.push_back( o );

m1.padstr_vector( s1, oo, 110, '+' );
printf("'0         1         2         3         4         5         6         7         8         9         10        11        12'\n" );
printf("'01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901'\n" );
printf("'%s'\n", s1.c_str() );




oo.clear();
o.str = "pear01234567890";
o.c_justify = '~';
o.i_justify = 1;
o.c_interpad = '.';
o.i_column = 5;
o.i_len = 17;
oo.push_back( o );

o.str = "rice0123456";
o.c_justify = '!';
o.i_justify = 1;
o.c_interpad = '.';
o.i_column = 25;
o.i_len = 17;
oo.push_back( o );

o.str = "what01234567890";
o.c_justify = '#';
o.i_justify = 1;
o.c_interpad = '.';
o.i_column = 45;
o.i_len = 17;
oo.push_back( o );

o.str = "hand01234567890";
o.c_justify = '?';
o.i_justify = 1;
o.c_interpad = '.';
o.i_column = 65;
o.i_len = 17;
oo.push_back( o );

o.str = "eyes01234567890";
o.c_justify = ']';
o.i_justify = 1;
o.c_interpad = '.';
o.i_column = 85;
o.i_len = 17;
oo.push_back( o );

m1.padstr_vector( s1, oo, 110, '+' );
printf("'0         1         2         3         4         5         6         7         8         9         10        11        12'\n" );
printf("'01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901'\n" );
printf("'%s'\n", s1.c_str() );






oo.clear();
o.str = "pear01234567890";
o.c_justify = '~';
o.i_justify = 2;
o.c_interpad = '.';
o.i_column = 5;
o.i_len = 17;
oo.push_back( o );

o.str = "rice0123456";
o.c_justify = '!';
o.i_justify = 2;
o.c_interpad = '.';
o.i_column = 25;
o.i_len = 17;
oo.push_back( o );

o.str = "what01234567890";
o.c_justify = '#';
o.i_justify = 2;
o.c_interpad = '.';
o.i_column = 45;
o.i_len = 17;
oo.push_back( o );

o.str = "hand01234567890";
o.c_justify = '?';
o.i_justify = 2;
o.c_interpad = '.';
o.i_column = 65;
o.i_len = 17;
oo.push_back( o );

o.str = "eyes01234567890";
o.c_justify = ']';
o.i_justify = 2;
o.c_interpad = '.';
o.i_column = 85;
o.i_len = 17;
oo.push_back( o );

m1.padstr_vector( s1, oo, 110, '+' );
printf("'0         1         2         3         4         5         6         7         8         9         10        11        12'\n" );
printf("'01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901'\n" );
printf("'%s'\n", s1.c_str() );

*/

//see above detailed description and example code for usage
void mystr::padstr_vector( string &sret, vector<st_mystr_padstr_vector> vprm, int tot_length, char c_end_pad )
{
string s1;

sret = "";

if( tot_length == 0 )
    {
    return;
    }


sret = "";

int prm_len = vprm.size();
if( prm_len == 0 )                                                  //no params, pad out to tot_length with spaces
    {
    for( int i = 0; i < tot_length; i++  ) sret += ' '; 
    return;
    }


int pos = 0;


//initial padding to get to first column
int clmn_pad_len = vprm[ 0 ].i_column - pos;                        //calc how much column padding is required
for( int j = 0; j < clmn_pad_len; j++ )
    {
    sret += vprm[ 0 ].c_interpad;                                   //pad to staring column firstly
    pos++;
    if( pos >= tot_length ) return;
    }


for( int i = 0; i < prm_len; i++  )
    {
    string ss = vprm[ i ].str;                                      //get string to be inserted into column
    int len = vprm[ i ].str.length();
    int i_len = vprm[ i ].i_len;
    
    if( len > i_len ) len = i_len;                                  //limit how much string can be inserted in need be
 

    int clmn_len;
    char c_just = vprm[ i ].c_justify;

    if( ( i + 1 ) >= prm_len )                                      //at last param?
        {
        clmn_len = tot_length - vprm[ i ].i_column;
        }
    else{
        clmn_len = vprm[ i + 1 ].i_column - vprm[ i ].i_column;     //work out how long this entry can be
        }

    s1 = "";

    int just_len = i_len - len;                                     //work out justify length as difference between column spacing and inserted string's length
    if( just_len < 0 ) just_len = 0;

    int left_just_len = just_len;
    int right_just_len = just_len;
    
//printf(" clmn_len = %d\n", clmn_len );
//printf(" i_len = %d\n", vprm[ i ].i_len );
//printf(" len = %d\n", len );
//printf(" just_len = %d\n", just_len );
//getchar();
//exit(0);

//insert right justify

    if( vprm[ i ].i_justify == 2 )                                          //centre justify?
        {
        left_just_len = just_len / 2;
        right_just_len = just_len - left_just_len;
        }

    if( right_just_len < 0 ) right_just_len - 0;

    if( ( vprm[ i ].i_justify == 1 ) || ( vprm[ i ].i_justify == 2 ) )      //right justify or centre justify?
        {
        for( int j = 0; j < right_just_len; j++ )
            {
            sret += vprm[ i ].c_justify;                                    //insert justify
            pos++;
            if( pos >= tot_length ) return;
            }
        }

//insert string into column
    for( int j = 0; j < len; j++ )
        {
        sret += vprm[ i ].str[ j ];
        pos++;
        if( pos >= tot_length ) return;
        }

    
//insert left justify
    if( ( vprm[ i ].i_justify == 0 ) || ( vprm[ i ].i_justify == 2 ) )      //left justify or centre justify?
        {
        for( int j = 0; j < left_just_len; j++ ) 
            {
            sret += vprm[ i ].c_justify;                                    //insert justify
            pos++;
            if( pos >= tot_length ) return;
            }
        }


//insert column padding to get to next column
    clmn_pad_len = vprm[ i + 1 ].i_column - pos;                            //calc how much column padding is required
    for( int j = 0; j < clmn_pad_len; j++ )
        {
        sret += vprm[ i ].c_interpad;                                       //pad to next column
        pos++;
        if( pos >= tot_length ) return;
        }
    }

//ensure string meets required length by further padding
int cnt = tot_length - sret.length();
//printf("tot_length: %d, tot_length: %d, diff: %d\n", tot_length, sret.length(), cnt );
//getchar();
//exit(0);
for( int j = 0; j < cnt; j++ )
    {
    sret += c_end_pad;                                                      //pad to end
    }


}












//read a file with some limits
//returns number of lines read, else 0
int mystr::readfile( string fname, int maxlines )
{
FILE *fp;
string s1;
char buf[16384], *ptr;
int lines = 0;

if ( maxlines <= 0) return 0;


fp = mbc_fopen( fname, "rb" );			//open file

if( fp == 0 ) return 0;



for( int i = 0; i < maxlines; i++ )      	//read file's lines with some sort of limit
    {
    ptr = fgets( buf , sizeof( buf ) , fp );
    if( ptr == 0 ) break;
	
    s1 += buf;

    lines++;
    }

fclose( fp );

csStr=s1;										//modify this obj's string
iLen=csStr.length();	
iPos=0;

return lines;

}











//write contents of string into a file using fname as path and filename
//returns 1 on success, else 0
bool mystr::writefile( string fname )
{
FILE *fp;
int ret;

fp = fopen( fname.c_str() , "wb" );			//open file

if( fp == 0 )
    {
 	return 0;
	}

ret = fprintf ( fp, "%s", csStr.c_str() );
fclose( fp );

if ( ret < iLen ) return 0;

return 1;
}







//appends to spec file, does this as a binary operation
//set 'create_file_if_not_present' to create file if it does not exist, otherwise will return failed
//will confirm byte counts written matches spec size and return 0 if mismatched
//returns 1 on success, else 0
bool mystr::appendfile( string fname, bool create_file_if_not_preset, unsigned char *ucbuf, unsigned long long int buf_size )
{
FILE *fp;

if( buf_size == 0 ) return 0;

if( create_file_if_not_preset ) fp = fopen( fname.c_str() , "a+b" );			//open file for appending, creates if necessary
else  fp = fopen( fname.c_str() , "r+b" );			                            //open exisitng file for appending, creates if necessary

if( fp == 0 ) return 0;

unsigned long long int  wrote = fwrite( ucbuf, 1, buf_size, fp );

fclose( fp );

if( wrote != buf_size ) return 0;
return 1;
}









//appends to spec file, does this as a binary operation
//set 'create_file_if_not_present' to create file if it does not exist, otherwise will return failed
//will confirm byte counts written matches spec size and return 0 if mismatched
//returns 1 on success, else 0
bool mystr::appendfile_str( string fname, bool create_file_if_not_preset, bool clear_first, string ss )
{
FILE *fp;

//if( ss.length() == 0 ) return 0;

string mode = "r+b";								//open existing file for appending, creates if necessary
if( create_file_if_not_preset ) mode = "a+b";		//open file for appending, creates if necessary
if( clear_first )  mode = "wb";						//creates an empty file before writing

	
fp = fopen( fname.c_str(),  mode.c_str() );

if( fp == 0 ) return 0;

unsigned long long int  wrote = fwrite( ss.c_str(), 1, ss.length(), fp );

fclose( fp );

if( wrote != ss.length() ) return 0;
return 1;
}








//similar to appendfile_str(), but does not use string objs
bool mystr::appendfile_sz( char *fname, bool create_file_if_not_preset, bool clear_first, char *sz )
{
FILE *fp;

//if( ss.length() == 0 ) return 0;

char modesz[20] = "r+b";												//open existing file for appending, creates if necessary
if( create_file_if_not_preset ) strcpy( modesz, "a+b" );		//open file for appending, creates if necessary
if( clear_first )  strcpy( modesz, "wb" );						//creates an empty file before writing

fp = fopen( fname,  modesz );

if( fp == 0 ) return 0;

int len = strlen( sz );
unsigned long long int  wrote = fwrite( sz, 1, len, fp );

fclose( fp );

if( wrote != len ) return 0;
return 1;
}








//checks folder path exists

//returns 1 path exists
//returns 0, path does not exist
//returns -1 if path is actually not a folder, maybe a file

//see also mystr::make_folders(..)
int mystr::check_folder_path_exists( string path, char subdir_char  )
{
int ret;
string s1;
 
//printf( "zzstat path0: '%s'\n",path.c_str() );	

int len = path.length();
if( len == 0 ) return 0;


if( len > 1 )
	{
	if( path[ len - 1 ] == subdir_char )		//ending slash?
		{
		s1 = path.substr( 0, len - 1 );			//clear ending slash
		}
	else{
		s1 = path;
		}
	}
else{
	s1 = path;
	}



struct stat statbuf;

//printf( "zzstat path1: '%s'\n", s1.c_str() );	

if ( stat( s1.c_str(), &statbuf ) == 0 )                  	//name exists?
	{
//printf( "zzstat: %d\n", statbuf.st_mode );	
	if ( !S_ISDIR( statbuf.st_mode ) )
        {
//printf( "ENOTDIR: %d\n", ENOTDIR );	
        return -1;		                                	//name exists, not a dir, maybe a file?
        }
    return 1;
	}
else{
//printf( "zzstat failed: %d\n", errno );	
	return 0;
	}
}











//makes a single folder from specified path, does not nest,
//if a file exists with conflicting name it returns ENOTDIR
//else it returns result from mkdir(..)

//linux_permissions (sys/stat.h): could be like:  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

//see also mystr::make_folders(..)
int mystr::make_single_folder( string path, char subdir_char, unsigned int linux_permissions )
{
int ret;
string s1;
 
//printf( "stat path0: '%s'\n",path.c_str() );	

s1 = path;

int len = path.length();
if( len > 1 )
	{
	if( path[ len - 1 ] == subdir_char )		//ending slash?
		{
		s1 = path.substr( 0, len - 1 );			//clear ending slash
		}
	}

struct stat statbuf;

//printf( "stat path1: '%s'\n",s1.c_str() );	

if ( stat( s1.c_str(), &statbuf ) == 0 )                  //name exists?
	{
//printf( "st_mode: %d\n", statbuf.st_mode );	
	if ( !S_ISDIR( statbuf.st_mode ) )
        {
//printf( "ENOTDIR: %d\n", ENOTDIR );	
        return ENOTDIR;		                                //name exists, not a dir, maybe a file?
        }
	}
else{
//printf( "stat failed: %d\n", errno );	
	}

//linux code
#ifndef compile_for_windows
	ret = mkdir( s1.c_str(), linux_permissions );		     //make folder
#endif


//windows code
#ifdef compile_for_windows
	ret = mkdir( s1.c_str() );							    //make folder
#endif

return ret;	
}











//for testing folder
/*
mystr m1;
//return;

//linux code
#ifndef compile_for_windows
int ret = m1.make_folders( "./", dir_seperator[ 0 ], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( "junk2/", dir_seperator[ 0 ], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( "./junk3", dir_seperator[ 0 ], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
printf( "make_folders returned val was: %d\n\n", ret );


ret = m1.make_folders( "./junk1/junk22///", dir_seperator[ 0 ], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( "./junk1/j///", dir_seperator[ 0 ], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
printf( "make_folders returned val was: %d\n\n", ret );
#endif

//windows code
#ifdef compile_for_windows
int ret = m1.make_folders( ".\\", dir_seperator[ 0 ], 0 );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( "junk2\\", dir_seperator[ 0 ], 0 );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( ".\\junk3", dir_seperator[ 0 ], 0 );
printf( "make_folders returned val was: %d\n\n", ret );


ret = m1.make_folders( ".\\junk1\\junk2\\", dir_seperator[ 0 ], 0 );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( ".\\junk1\\j\\", dir_seperator[ 0 ], 0 );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( ".\\a\\b\\c", dir_seperator[ 0 ], 0 );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( "d:\\afolder", dir_seperator[ 0 ], 0 );
printf( "make_folders returned val was: %d\n\n", ret );

ret = m1.make_folders( "\\\\tsnsw01\\Maintenance\\Television\\afolder", dir_seperator[ 0 ], 0 );	//unc
printf( "make_folders returned val was: %d\n\n", ret );
#endif



*/





//makes a single or nested folders from specified path, needs to have a dir_seperator before first dir see examples below
//try mystr::check_folder_path_exists() for easy checking of exisitng dir or conflicting filename

//returns 1 if path was created or existed
//returns 0, path was not created and does not exist
//returns -1 if path is actually not a folder, maybe a file

//linux_permissions (sys/stat.h): could be like:  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

//e.g: "./folder1/folder2"					//will create folders in current working directory
//e.g: "/mnt/sda3/folder/folder2"
//e.g: "c:\folder1\folder2\folder3\"
//e.g: "c:/folder1"
//e.g: ".\\a\\b\\c"
//e.g: "./a"
//e.g: ".\\a"
//e.g: "\\server01\share1\afolder"			//unc path
//!!this will fail "a\\b\\c"				//instead use ".\\a\\b\\c"
int mystr::make_folders( string path, char subdir_char, unsigned int linux_permissions )
{
int ret, len;
mystr m1;
string s1, s2, schar;
int success = 0;
bool unc = 0;

vector<string> vs;

//printf( "trying for: '%s'\n", path.c_str() );	

len = path.size();

if( len < 1 ) return 0;

if( len > 2 )
	{
	if( path[ 0 ] == '\\' )						//check for unc
		{
		if( path[ 1 ] == '\\' )
			{
			unc = 1;
			}
		}
	}

/*
if( path[ len - 1 ] == subdir_char )		//ending slash?
	{
	s1 = path.substr( 0, len - 1 );			//clear ending slash
	}
else{
	s1 = path;
	}
*/
s1 = path;
len = s1.size();

//if( len < 2 ) return 0;


m1 = s1;
int count = m1.LoadVectorStrings( vs,subdir_char );
//printf( "count: %d\n", count );	

if( count == 0 ) return 0;

for( int i = 0; i < count; i++ )
	{
	if( i != ( count - 1 ) ) vs[ i ] += subdir_char;		//if not the last item in list, add the slash back, as this was removed due to it being a delimiter
//	printf( "folder0 '%s'\n", vs[ i ].c_str() );	
	}

s1 = "";

if( unc ) s1 = "\\\\";										//re-add unc that was stripped off

s1 += vs[ 0 ];												//make starting path

if ( count == 1 )
	{
//	printf( "folder0 '%s'\n", s1.c_str() );	
    if( m1.make_single_folder( s1, subdir_char, linux_permissions ) == ENOTDIR ) goto done_bad;
        

	if( errno != 0 )
		{
        if( ( errno == EEXIST ) || ( errno == ENOENT ) )            //is the error only that dir exists or did not exist?
			{
			goto done_ok;
			}
        else{
			goto done_bad;
            }
		}
	}
else{

	for( int i = 1; i < count; i++ )
		{
		s1 += vs[ i ];							//add the slash back, as this was removed due to it being a delimiter

//		printf( "folder11 '%s'\n", s1.c_str() );	

		if( m1.make_single_folder( s1, subdir_char, linux_permissions ) == ENOTDIR ) goto done_bad;

		if( errno != 0 )                        //was there an error after mkdir call in make_single_folder()
			{
			if( ( errno == EEXIST ) || ( errno == ENOENT ) )            //is the error only that dir exists or did not exist?
				{
				continue;
				}
			goto done_bad;
			}
		}
	}

done_ok:
return check_folder_path_exists( path, subdir_char );


done_bad:
printf( "mystr::make_folders() - posix error: %d, path: '%s'\n", errno, s1.c_str() );	
return check_folder_path_exists( path, subdir_char );

}









//convert string to upper case
void mystr::to_upper( string &sret )
{
char c;

sret="";

if (iLen==0) return; 

for( int i=0; i < iLen; i++ )
	{
	c = toupper( csStr[i] );
	sret += c;
	}

}








//convert string to lower case
void mystr::to_lower( string &sret )
{
char c;

sret="";

if (iLen==0) return; 

for( int i=0; i < iLen; i++ )
	{
	c = tolower( csStr[i] );
	sret += c;
	}
}








//convert utf8 mcb string to wchar string array
//note: linux locale must be set to utf8 for this function to work correctly
//linux's wchar_t is 32 bits long
//windows' wchar_t is 16 bits long

int mystr::mbcstr_wcstr( wstring &wstr )
{
unsigned long count;
int wchar_bytesize;
wchar_t *wstrptr;
const char *ptr;

wstr = L"";

if ( iLen == 0 ) return 0; 

wchar_bytesize = sizeof( wchar_t );					//find how big a wchar_t is


ptr = csStr.c_str();


//linux code
#ifndef compile_for_windows

mbstate_t mb_st;						//holds the current state of this multi byte char conversion
memset( &mb_st, 0, sizeof( mb_st ) );

size_t ret = mbsrtowcs( 0, &ptr, 0, &mb_st );		//work out how big wchar_t needs to be

//printf("\nret = %d\n", ret );

if( ( ret == -1 ) || ( ret == 0 ) ) return 0;

ret++;												//add room for terminating zero
wstrptr = ( wchar_t* ) malloc( ret * wchar_bytesize );	//allocate multiple wchar_t

if( wstrptr == 0 )
	{
	printf("\nmbcstr_wcstr() failed to allocate memory for mbsrtowcs()\n");
	return 0;
	}

wstrptr[ 0 ] = 0x0;

memset( &mb_st, 0, sizeof( mb_st ) );
ret = mbsrtowcs( wstrptr, &ptr, ret, &mb_st );		//convert mbc to wchar_t

//printf( "\nwc = %x\n", wstrptr[0] );
#endif


//windows code
#ifdef compile_for_windows

int ret = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, ptr, -1, 0, 0 );

if( ( ret == -1 ) || ( ret == 0 ) ) return 0;

wstrptr = ( wchar_t* ) malloc( ret * wchar_bytesize );	//allocate multiple wchar_t

if( wstrptr == 0 )
	{
	printf("\nmbcstr_wcstr() failed to allocate memory for MultiByteToWideChar()\n");
	return 0;
	}

wstrptr[ 0 ] = 0x0;

ret = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, ptr, -1, wstrptr, ret );

//strpf( s1, "MultiByteToWideChar() returned: %d\n", ret );

//if( sizeof( wchar_t ) == 4 ) strpf( shex, "%08x", (unsigned long)wchr );
//if( sizeof( wchar_t ) == 2 ) strpf( shex, "%04x", (unsigned long)wchr );

#endif

wstr = wstrptr;
free( wstrptr );

return 1;

/*
#ifdef compile_for_windows
string s1;
wchar_t wcstr [ 2048 ];
int ret = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, csStr.c_str(), csStr.length(), (WCHAR*)&wcstr, sizeof( wcstr ) );
if( ( ret >= 0 ) && ( ret < ( sizeof( wcstr ) - 1 ) ) ) wcstr[ ret ] = 0x0;
wstr = wcstr;
#endif
*/

}









//convert wchar string array to a utf8 mcb string
//note: linux locale must be set to utf8 for this function to work correctly
//linux's wchar_t is 32 bits long
//windows' wchar_t is 16 bits long

int mystr::wcstr_mbcstr( wstring &wstr, string &sret )
{
int wchar_bytesize;
const wchar_t *wcsptr;
char *strptr;

sret="";

wchar_bytesize = sizeof( wchar_t );				//find how big a wchar_t is

wcsptr = wstr.c_str();

//linux code
#ifndef compile_for_windows

mbstate_t mb_st;							//holds the current state of this multi byte char conversion
memset( &mb_st, 0, sizeof( mb_st ) );


size_t ret = wcsrtombs( 0, &wcsptr, 0, &mb_st );		//work out how big string needs to be

//printf("\nret2=%d\n", ret );
if( ( ret == -1 ) || ( ret == 0 ) ) return 0;

ret++;												//add room for terminating zero
strptr = ( char* ) malloc( ret );		//allocate multiple bytes for string

if( strptr == 0 )
	{
	printf("\nwcstr_mbcstr() failed to allocate memory for wcsrtombs()\n");
	return 0;
	}

strptr[ 0 ] = 0;

ret = wcsrtombs( strptr, &wcsptr, ret, &mb_st );		//convert wchar_t string to mbc string

#endif






//windows code
#ifdef compile_for_windows
//wchar_t wcstr[2];

//wcstr[ 0 ] = (wchar_t)tmp1;						//build a wide char from hex bytes
//wcstr[ 1 ] = 0x0;

int ret = WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), -1, 0, 0, 0, 0 );

if( ( ret == -1 ) || ( ret == 0 ) ) return 0;
//printf("\nret3=%d\n", ret);

strptr = ( char* ) malloc( ret  );		//allocate multiple bytes for string

if( strptr == 0 )
	{
	printf("\nwcstr_mbcstr() failed to allocate memory for WideCharToMultiByte()\n");
	return 0;
	}

strptr[ 0 ] = 0;

ret = WideCharToMultiByte( CP_UTF8, 0, wstr.c_str(), -1, strptr, ret, 0, 0 );

#endif




sret = strptr;
free( strptr );



return 1;
}














//checks if 'mbcfname' filename exists, by opening and closing file
//returns 1 if exists, else 0
//windows version converts mbcfname to a wstring before calling _wfsopen 
bool mystr::mbc_check_file_exists( string &mbcfname )
{
FILE *fp;

if ( mbcfname.length() == 0 ) return 0;

//linux code
#ifndef compile_for_windows
//cslpf("src file exists: %s \n", fname.c_str() );

fp = fopen( mbcfname.c_str() ,"rb" );		 //open file
#endif



//windows code
#ifdef compile_for_windows
wstring ws1;
mystr m1 = mbcfname;

m1.mbcstr_wcstr( ws1 );	//convert utf8 string to windows wchar string array

fp = _wfsopen( ws1.c_str(), L"rb", _SH_DENYNO );
#endif


if( fp == 0 )
	{
	return 0;
	}


fclose( fp );
return 1;
}







//opens filename 'mbcfname', returns ptr to opened file, must later be closed
//returns 1 if exists, else 0
//windows version converts 'mbcfname' and 'mode' to wstring before calling _wfsopen
//remember to write Byte Order Mark BOM at head of text files: utf-8 BOM is: "\xef\xbb\xbf"
FILE* mystr::mbc_fopen( string &mbcfname, const char *mode )
{
FILE *fp;

//linux code
#ifndef compile_for_windows
//cslpf("src file exists: %s \n", fname.c_str() );

fp = fopen( mbcfname.c_str() , mode );		 //open file

//printf("\nopening = %s, %x\n", mbcfname.c_str(), (unsigned int)fp ); 
#endif



//windows code
#ifdef compile_for_windows
wstring ws1, ws2;
mystr m1 = mbcfname;
m1.mbcstr_wcstr( ws1 );	//convert utf8 string to windows wchar string array

mystr m2 = (char*)mode;
m2.mbcstr_wcstr( ws2 );	//convert utf8 string to windows wchar string array


fp = _wfsopen( ws1.c_str(), ws2.c_str(), _SH_DENYNO );
#endif

return fp;
}











//get file 'fname' file size 
//returns 1 if successful, else 0
//v1.36 fixed large filesize reporting for Windows
bool mystr::filesize( string fname, unsigned long long int &filesz )
{
FILE *fp;
bool retval = 1;

filesz = 0;


int status;
struct stat statbuf;

//linux code
#ifndef compile_for_windows
status = lstat( (char*)fname.c_str(), &statbuf );
if( status == -1 ) retval = 0;
filesz = (unsigned long long int) statbuf.st_size;
#endif


//windows code
#ifdef compile_for_windows


LARGE_INTEGER li = { 0 };
HANDLE hFile = CreateFile( fname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

if ( hFile != INVALID_HANDLE_VALUE)
    {

    BOOL bSuccess = GetFileSizeEx( hFile, &li );         //need this before windows.h for GetFilesizeEx() to compile:   #define _WIN32_WINNT 0x0501
    
    filesz |= li.HighPart;
    filesz = filesz << 32;
    filesz |= li.LowPart;
    CloseHandle( hFile );
    }
else{
    retval = 0;    
    }

#endif


//below code did not detect large files on windows
/*
//windows code
#ifdef compile_for_windows
status = stat( (char*)fname.c_str(), &statbuf );
#endif

if( status != -1 )
	{
	filesz = (unsigned long long int) statbuf.st_size;
	return 1;
	}
else{
	return 0;
	}
*/

return retval;
}









//find if supplied 'str' has prefixed unicode byte order mark (BOM)
//sets 'type' with encoding found
//'type' may be set to detected formats: "UTF-8", "UTF-16LE", "UTF-16BE", "UTF-32LE", "UTF-32BE"
//returns number of bytes in encoding on finding BOM, or -1 if no encoding found

//UTF-8     ef bb bf
//UTF-16LE  ff fe
//UTF-16BE  fe ff
//UTF-32LE  ff fe 00 00
//UTF-32BE  00 00 fe ff
//NOTE: BOMs with 0x00 in them would make a string appear to prematurely terminate
//therefore an 'unsigned char *array' is used
int mystr::find_unicode_encoding ( unsigned char *array, unsigned int len, string &type )
{
int ret = -1;
int  count;
//string shex;
//unsigned char hex[ 4 ];

type="";

if( len < 2 ) goto fin_unknown;

if( len == 2 ) count = 2;				//work out how many bytes avail or required
if( len == 3 ) count = 3;
if( len >= 4 ) count = 4;

//for( int i = 0; i < collect; i++ )		//collect enough bytes for compare
//	{
//	hex[ i ] += array[ i ];
//	}


if( count == 2 )								//2 bytes
	{
	if( array[ 0 ] == 0xff )
		{
		if( array[ 1 ] == 0xfe )
			{
			type="UTF-16LE";
			ret = 2;
			goto fin_found;
			}
		}

	if( array[ 0 ] == 0xfe )					//2 bytes
		{
		if( array[ 1 ] == 0xff )
			{
			type="UTF-16BE";
			ret = 2;
			goto fin_found;
			}
		}
	}



if( count == 3 )
	{
	if( array[ 0 ] == 0xef )					//3 bytes
		{
		if( array[ 1 ] == 0xbb )
			{
			if( array[ 2 ] == 0xbf )
				{
				type="UTF-8";
				ret = 3;
				goto fin_found;
				}
			}
		}
	}
	


if( count >= 4 )
	{

	if( array[ 0 ] == 0xff )					//4 bytes
		{
		if( array[ 1 ] == 0xfe )
			{
			if( array[ 2 ] == 0x00 )
				{
				if( array[ 3 ] == 0x00 )
					{
					type="UTF-32LE";
					ret = 4;
					goto fin_found;
					}
				}
			}
		}

	if( array[ 0 ] == 0x00 )					//4 bytes
		{
		if( array[ 1 ] == 0x00 )
			{
			if( array[ 2 ] == 0xfe )
				{
				if( array[ 3 ] == 0xff )
					{
					type="UTF-32BE";
					ret = 4;
					goto fin_found;
					}
				}
			}
		}



	if( array[ 0 ] == 0xef )					//3 bytes
		{
		if( array[ 1 ] == 0xbb )
			{
			if( array[ 2 ] == 0xbf )
				{
				type="UTF-8";
				ret = 3;
				goto fin_found;
				}
			}
		}


	if( array[ 0 ] == 0xff )					//2 bytes
		{
		if( array[ 1 ] == 0xfe )
			{
			type="UTF-16LE";
			ret = 2;
			goto fin_found;
			}
		}

	if( array[ 0 ] == 0xfe )					//2 bytes
		{
		if( array[ 1 ] == 0xff )
			{
			type="UTF-16BE";
			ret = 2;
			goto fin_found;
			}
		}

	}


fin_unknown:
ret = -1;

fin_found:

return ret;
}








//given a fp pointing to head of a file, detect and move past a byte order mark BOM.
//sets 'type' with encoding found,
//'type' may be set to detected formats: "UTF-8", "UTF-16LE", "UTF-16BE", "UTF-32LE", "UTF-32BE"
//returns number of bytes in encoding on finding BOM, or -1 if no encoding found

int mystr::detect_unicode_bom_at_head_of_file( FILE *fp, string &type )
{
type="";
unsigned char buf[ 4 ];

if( fp == 0 ) return -1;						//no fp?

int read = fread( buf, 1, sizeof( buf ), fp );	//read up to 4 bytes

//printf("read %d bytes\n", read);
if( read == 0 ) return -1;						//read zero bytes?

int bom_size = find_unicode_encoding ( buf, read, type );	//see if bytes contain possible bom

if( bom_size == -1 )
	{
	fseek( fp, 0, SEEK_SET );		//if here no bom found ,move fp to begining of file
	return -1;
	}

fseek( fp, bom_size, SEEK_SET );			//if here valid bom found move fp past it

return bom_size;
}









//find first occurrence (after 'pos_start') of 'sfind' and terminate string at occurrence
//will also cut occurrence off, returns 1 if a cut happended, else 0
int mystr::cut_at_first_find( string &sret, string sfind, int pos_start )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

if( pos_start >= len ) return 0;

int pos = sret.find( sfind , pos_start );		//find first occurrence
if( pos < 0 ) 
	{
	return 0;
	}

sret = sret.substr( 0 , pos );		//extract first string part

return 1;
}









//find first occurrence (after 'pos_start') of 'sfind' and terminate string at next char after occurrence
//will NOT cut occurrence off, returns 1 if a cut happended, else 0
int mystr::cut_just_past_first_find( string &sret, string sfind, int pos_start )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

if( pos_start >= len ) return 0;

int pos = sret.find( sfind , pos_start );		//find first occurrence
if( pos < 0 ) 
	{
	return 0;
	}

if ( pos >= ( len - 1 ) ) return 1;

sret = sret.substr( 0 , pos + 1 );		//extract first string part

return 1;
}










//find first occurrence (after 'pos_start') of 'sfind' remove chars up to this point, so right chars are kept
//will also cut occurrence off, returns 1 if a cut happended, else 0

// !!! WILL STILL return full string if cut did not happen, this can be tested as it returns 0
int mystr::cut_just_past_first_find_and_keep_right( string &sret, string sfind, int pos_start )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

int find_len = sfind.length();					//added this in v1.72 
if( find_len == 0 ) return 0;					//added this in v1.72 

if( pos_start >= len ) return 0;

int pos = sret.find( sfind , pos_start );		//find first occurrence

if( pos == string::npos ) return 0;				//nothing found?

if ( pos >= ( len - 1 ) )						//nothing past cut?
	{
	sret = "";									//added this in v1.24 
	return 1;
	}

if( find_len == len )							//added this in v1.72 
	{
	sret = "";
	return 1;
	}

sret = sret.substr( pos + find_len , len - pos - find_len  );			//moded this in v1.72 

return 1;

}









//find first occurrence (after 'pos_start') of 'sfind' remove chars up to this point, so right chars are kept
//will NOT cut occurrence off, returns 1 if a cut happended, else 0

// !!! WILL STILL return full string if cut did not happen, this can be tested as it returns 0
int mystr::cut_at_first_find_and_keep_right( string &sret, string sfind, int pos_start )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

if( pos_start >= len ) return 0;

int pos = sret.find( sfind , pos_start );		//find first occurrence

if( pos == string::npos ) return 0;				//nothing found?


sret = sret.substr( pos , len - pos  );			//extract second string part

return 1;

}






//find at end of first occurrence (after 'pos_start') of 'sfind' remove chars up to this point, so right chars are kept,
//the occurrence is fully removed
//returns 1 if a cut happended, else 0
//will still return 1 if 'sfind' is found but nothing is left after removing sfind, i.e. 'sret' is set to empty,

//e.g:  for "tempo: 120",  cut_at_end_of_first_find_and_keep_right( s1, "temp: ", 0 ), will return s1 set to: "120" 

// !!! WILL STILL return full string if cut did not happen, this can be tested as it returns 0
int mystr::cut_at_end_of_first_find_and_keep_right( string &sret, string sfind, int pos_start )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

int find_len = sfind.length();
if( find_len == 0 ) return 0;

if( pos_start >= len ) return 0;

int pos = sret.find( sfind , pos_start );								//find first occurrence

if( pos == string::npos ) return 0;										//nothing found?

sret = "";
if( ( pos + find_len-1 ) >= len ) return 1;								//nothing left to ret?

int cut_len = pos + find_len;

sret = csStr.substr( cut_len, len - cut_len );		//extract second string part, just past the sfind
return 1;
}





//strip any cr and lf (if specified), stores changed string in 'sret'
void mystr::strip_cr_or_lf( string &sret, bool b_strip_cr, bool b_strip_lf )
{
mystr m1;

if( iLen == 0 ) return;

sret = csStr;

if( b_strip_cr )
    {
    m1 = sret;
    m1.FindReplace( sret, "\r", "", 0 );
    }

if( b_strip_lf )
    {
    m1 = sret;
    m1.FindReplace( sret, "\n", "", 0 );
    }

}









//strip any spec chars, stores changed string in 'sret'
void mystr::strip_any_chars1( string &sret, char c1, int pos )
{
mystr m1;

if( iLen == 0 ) return;

sret = csStr;

m1 = sret;
m1.FindReplace( sret, c1, "", pos );
}







//strip any spec chars, stores changed string in 'sret'
void mystr::strip_any_chars2( string &sret, char c1, char c2, int pos )
{
mystr m1;

if( iLen == 0 ) return;

sret = csStr;

m1 = sret;
m1.FindReplace( sret, c1, "", pos );

m1 = sret;
m1.FindReplace( sret, c2, "", pos );

}







//strip any spec chars, stores changed string in 'sret'
void mystr::strip_any_chars3( string &sret, char c1, char c2, char c3, int pos )
{
mystr m1;

if( iLen == 0 ) return;

sret = csStr;

m1 = sret;
m1.FindReplace( sret, c1, "", pos );

m1 = sret;
m1.FindReplace( sret, c2, "", pos );

m1 = sret;
m1.FindReplace( sret, c3, "", pos );
}








//strip any spec chars, stores changed string in 'sret'
void mystr::strip_any_chars4( string &sret, char c1, char c2, char c3, char c4, int pos )
{
mystr m1;

if( iLen == 0 ) return;

sret = csStr;

m1 = sret;
m1.FindReplace( sret, c1, "", pos );

m1 = sret;
m1.FindReplace( sret, c2, "", pos );

m1 = sret;
m1.FindReplace( sret, c3, "", pos );

m1 = sret;
m1.FindReplace( sret, c4, "", pos );
}









//strip any spec chars, stores changed string in 'sret'
void mystr::strip_any_chars5( string &sret, char c1, char c2, char c3, char c4, char c5, int pos )
{
mystr m1;

if( iLen == 0 ) return;

sret = csStr;

m1 = sret;
m1.FindReplace( sret, c1, "", pos );

m1 = sret;
m1.FindReplace( sret, c2, "", pos );

m1 = sret;
m1.FindReplace( sret, c3, "", pos );

m1 = sret;
m1.FindReplace( sret, c4, "", pos );

m1 = sret;
m1.FindReplace( sret, c5, "", pos );
}











//strip any spec chars, stores changed string in 'sret'
void mystr::strip_any_chars6( string &sret, char c1, char c2, char c3, char c4, char c5, char c6, int pos )
{
mystr m1;

if( iLen == 0 ) return;

sret = csStr;

m1 = sret;
m1.FindReplace( sret, c1, "", pos );

m1 = sret;
m1.FindReplace( sret, c2, "", pos );

m1 = sret;
m1.FindReplace( sret, c3, "", pos );

m1 = sret;
m1.FindReplace( sret, c4, "", pos );

m1 = sret;
m1.FindReplace( sret, c5, "", pos );

m1 = sret;
m1.FindReplace( sret, c6, "", pos );
}








//linux/windows ms delay, see also 'delay_us()'
void mystr::delay_ms( int ms )
{

//linux code
#ifndef compile_for_windows
int secs = 0;
if ( ms >= 1000 )
	{
	secs = (int)(double) ms / (double)1000.0;

	ms = secs * 1000 - ms;
	}

timespec ts, tsret;
ts.tv_sec = secs;
ts.tv_nsec = (double)ms * (double)1e6;		//4294967269 
nanosleep( &ts, &tsret );

#endif


//windows code
#ifdef compile_for_windows
Sleep( ms );
#endif

}







//if 'usec' > 1000 will call 'delay_ms()' 
void mystr::delay_us( int usec )
{
if( usec <= 0 ) return;
	
if( usec > 1000 ) 
	{
	delay_ms( usec / 1000 );
	return;
	}

//linux code
#ifndef compile_for_windows
	timespec ts, tsret;
	ts.tv_sec = 0;
	ts.tv_nsec = (double)usec * 1000;		//4294967269 
	nanosleep( &ts, &tsret );
#endif


//windows code
#ifdef compile_for_windows

	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * (__int64)usec);

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
#endif

}






//start a timing timer with internal ns resolution
//the supplied var 'start' will hold an abitrary count in ns used by call time_passed()
void mystr::time_start( unsigned long long int &start )
{

//linux code
#ifndef compile_for_windows 

timespec ts_now;

clock_gettime( CLOCK_MONOTONIC, &ts_now );		//initial time read
start = ts_now.tv_nsec;							//get ns count
start += (double)ts_now.tv_sec * 1e9;					//make secs count a ns figure

#endif




//windows code
#ifdef compile_for_windows

LARGE_INTEGER li;
unsigned long long int big;

QueryPerformanceFrequency( &li );		//get performance counter's freq


big = 0;
big |= li.HighPart;
big = big << 32;
big |= li.LowPart;

perf_period = (double)1.0 / (double)big;		//derive performance counter's period

//cslpf("Freq = %I64u\n", big );
//cslpf("Period = %g\n", perf_period );

QueryPerformanceCounter( &li );

start = 0;
start |= li.HighPart;
start = start << 32;
start |= li.LowPart;

start = start * perf_period * 1e9;					//make performance counter a ns figure
#endif

}







//calc secs that have passed since call to time_start()
//calcs are maintained in ns internally and in supplied var 'start'
//returns time that has passed in seconds
double mystr::time_passed( unsigned long long int start )
{
unsigned long long int now, time_tot;

//linux code
#ifndef compile_for_windows 

timespec ts_now;

clock_gettime( CLOCK_MONOTONIC, &ts_now );		//read current time
now = ts_now.tv_nsec;							//get ns count
now += (double)ts_now.tv_sec * 1e9;				//make secs count a ns figure

time_tot = now - start;

#endif





//windows code
#ifdef compile_for_windows

LARGE_INTEGER li;

QueryPerformanceCounter( &li );

now = 0;
now |= li.HighPart;
now = now << 32;
now |= li.LowPart;

now = now * perf_period * 1e9;			//conv performance counter figure into ns

time_tot = now - start;
#endif


return (double)time_tot * 1e-9;			//conv internal ns delta to secs
}








//get local time into a tm struct
void mystr::get_time_now( struct tm &tt )
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
tt.tm_wday = st.wDayOfWeek;			//v1.37 fixed this, used to set to zero
tt.tm_mon = st.wMonth - 1;			//make it suite month value, ie Jan=0
tt.tm_year = st.wYear - 1900;       //make it suite year value for: struct tm
tt.tm_isdst = 0;

#endif

}






//make date string from supplied struct
bool mystr::make_date_str( struct tm tn, string &dow, string &dom, string &mon_num, string &mon_name, string &year, string &year_short )
{

if( ( tn.tm_wday < 0 ) || ( tn.tm_wday > 6 ) ) return 0;
if( ( tn.tm_mon < 0 ) || ( tn.tm_mon > 11 ) ) return 0;     //fixed v1.41

char day[ ][ 4 ]=
{
"Sun",
"Mon",
"Tue",
"Wed",
"Thu",
"Fri",
"Sat"
};

char month[ ][ 4 ]=
{
"Jan",
"Feb",
"Mar",
"Apr",
"May",
"Jun",
"Jul",
"Aug",
"Sep",
"Oct",
"Nov",
"Dec",
};

int year_offset = 1900;
int month_offset = 1;


strpf( dow, "%s", day[ tn.tm_wday ] ); 
strpf( dom, "%02d", tn.tm_mday ); 
strpf( mon_num, "%02d", tn.tm_mon + month_offset ); 

strpf( mon_name, "%s", month[ tn.tm_mon ] ); 
strpf( year, "%04d", tn.tm_year + year_offset ); 
year_short = year[ 2 ];
year_short += year[ 3 ];

//printf( "make_date_str: %s  %s  %s  %s  %s  %s\n", dow.c_str(), dom.c_str(),  mon_num.c_str(), mon_name.c_str(), year.c_str(), year_short.c_str()  );

return 1;
}






//make time string from supplied struct
void mystr::make_time_str( struct tm tn, string &hrs, string &mins, string &secs, string &time )
{

strpf( hrs, "%02d", tn.tm_hour ); 
strpf( mins, "%02d", tn.tm_min ); 
strpf( secs, "%02d", tn.tm_sec ); 

strpf( time, "%02d:%02d:%02d", tn.tm_hour, tn.tm_min, tn.tm_sec ); 

}














//extract time from supplied string and convert to integer values stored in time vars of 'struct tm', should be used with  'date_str_to_tm(..)'
//allowable format: hhdmmdss (d is char delimiter of choice [any non-zero will do] ), e.g: hh:mm:ss (must not have leading spaces)
//allowable format: hhmmss (d is zero)  (must not have leading spaces)
//returns 1 on success, else 0
bool mystr::time_str_to_tm( char delim, bool isdst, struct tm &tt )
{


char sz[ 3 ];

sz[ 2 ] = 0;

if( delim == 0 )
    {
    if( iLen < 6 ) return 0;

    sz[ 0 ] = csStr[ 0 ];                 //hhmmss format
    sz[ 1 ] = csStr[ 1 ];

    sscanf( sz, "%d", &tt.tm_hour );

    sz[ 0 ] = csStr[ 2 ];
    sz[ 1 ] = csStr[ 3 ];

    sscanf( sz, "%d", &tt.tm_min );

    sz[ 0 ] = csStr[ 4 ];
    sz[ 1 ] = csStr[ 5 ];

    sscanf( sz, "%d", &tt.tm_hour );
    }
else{
//printf( "time_str_to_tm %d '%s'\n", iLen, csStr.c_str() );

    if( iLen < 8 ) return 0;

    sz[ 0 ] = csStr[ 0 ];                 //hhdmmdss format
    sz[ 1 ] = csStr[ 1 ];

    sscanf( sz, "%d", &tt.tm_hour );

    sz[ 0 ] = csStr[ 3 ];
    sz[ 1 ] = csStr[ 4 ];

    sscanf( sz, "%d", &tt.tm_min );

    sz[ 0 ] = csStr[ 6 ];
    sz[ 1 ] = csStr[ 7 ];

    sscanf( sz, "%d", &tt.tm_sec );
    }


tt.tm_isdst = isdst;


return 1;
}







//extract date from supplied string and convert to integer values stored in time vars of 'struct tm', should be used with  'time_str_to_tm(..)'
//note: must not have leading spaces
//allowable format 0: ddmmyyyy
//allowable format 1: dd-mm-yyyy        //any ascii char can be used as delimiter as its not tested
//allowable format 10: mmddyyyy
//allowable format 11: mm-dd-yyyy       //any ascii char can be used as delimiter as its not tested
//allowable format 20: yyyymmdd
//allowable format 21: yyyy-mm-dd       //any ascii char can be used as delimiter as its not tested 

//returns 1 on success, else 0
bool mystr::date_str_to_tm( int format, struct tm &tt )
{
char sz[ 5 ];

sz[ 2 ] = 0;
sz[ 4 ] = 0;

tt.tm_wday = 0;                                //set this to something, it probably won't be correct

switch( format )
    {
    case  0:
        if( iLen < 8 ) return 0;
        sz[ 0 ] = csStr[ 0 ];                 //ddmmyyyy format
        sz[ 1 ] = csStr[ 1 ];
        sscanf( sz, "%d", &tt.tm_mday );

        sz[ 0 ] = csStr[ 2 ];
        sz[ 1 ] = csStr[ 3 ];
        sscanf( sz, "%d", &tt.tm_mon );
		tt.tm_mon -= 1;
    
        sz[ 0 ] = csStr[ 4 ];
        sz[ 1 ] = csStr[ 5 ];
        sz[ 2 ] = csStr[ 6 ];
        sz[ 3 ] = csStr[ 7 ];
        sscanf( sz, "%d", &tt.tm_year );
        tt.tm_year -= 1900;
    break;

    case  1:
        if( iLen < 10 ) return 0;
        sz[ 0 ] = csStr[ 0 ];                 //dd-mm-yyyy format
        sz[ 1 ] = csStr[ 1 ];
        sscanf( sz, "%d", &tt.tm_mday );

        sz[ 0 ] = csStr[ 3 ];
        sz[ 1 ] = csStr[ 4 ];
        sscanf( sz, "%d", &tt.tm_mon );
		tt.tm_mon -= 1;

        sz[ 0 ] = csStr[ 6 ];
        sz[ 1 ] = csStr[ 7 ];
        sz[ 2 ] = csStr[ 8 ];
        sz[ 3 ] = csStr[ 9 ];
        sscanf( sz, "%d", &tt.tm_year );
        tt.tm_year -= 1900;
    break;

    case  10:
        if( iLen < 8 ) return 0;
        sz[ 0 ] = csStr[ 0 ];                 //mmddyyyy format
        sz[ 1 ] = csStr[ 1 ];
        sscanf( sz, "%d", &tt.tm_mon );
		tt.tm_mon -= 1;

        sz[ 0 ] = csStr[ 2 ];
        sz[ 1 ] = csStr[ 3 ];
        sscanf( sz, "%d", &tt.tm_mday );
    
        sz[ 0 ] = csStr[ 4 ];
        sz[ 1 ] = csStr[ 5 ];
        sz[ 2 ] = csStr[ 6 ];
        sz[ 3 ] = csStr[ 7 ];
        sscanf( sz, "%d", &tt.tm_year );
        tt.tm_year -= 1900;
    break;

    case  11:
        if( iLen < 10 ) return 0;
        sz[ 0 ] = csStr[ 0 ];                 //mm-dd-yyyy format
        sz[ 1 ] = csStr[ 1 ];
        sscanf( sz, "%d", &tt.tm_mon );
		tt.tm_mon -= 1;

        sz[ 0 ] = csStr[ 3 ];
        sz[ 1 ] = csStr[ 4 ];
        sscanf( sz, "%d", &tt.tm_mday );
    
        sz[ 0 ] = csStr[ 6 ];
        sz[ 1 ] = csStr[ 7 ];
        sz[ 2 ] = csStr[ 8 ];
        sz[ 3 ] = csStr[ 9 ];
        sscanf( sz, "%d", &tt.tm_year );
        tt.tm_year -= 1900;
    break;


    case  20:
        if( iLen < 8 ) return 0;

        sz[ 0 ] = csStr[ 4 ];                 //yyyymmdd format
        sz[ 1 ] = csStr[ 5 ];
        sscanf( sz, "%d", &tt.tm_mon );
		tt.tm_mon -= 1;
    
        sz[ 0 ] = csStr[ 6 ];
        sz[ 1 ] = csStr[ 7 ];
        sscanf( sz, "%d", &tt.tm_mday );

        sz[ 0 ] = csStr[ 0 ];
        sz[ 1 ] = csStr[ 1 ];
        sz[ 2 ] = csStr[ 2 ];
        sz[ 3 ] = csStr[ 3 ];
        sscanf( sz, "%d", &tt.tm_year );
        tt.tm_year -= 1900;
    break;

    case  21:
        if( iLen < 10 ) return 0;
        sz[ 0 ] = csStr[ 5 ];                 //yyyy-mm-dd format
        sz[ 1 ] = csStr[ 6 ];
        sscanf( sz, "%d", &tt.tm_mon );
		tt.tm_mon -= 1;

        sz[ 0 ] = csStr[ 8 ];
        sz[ 1 ] = csStr[ 9 ];
        sscanf( sz, "%d", &tt.tm_mday );
    
        sz[ 0 ] = csStr[ 0 ];
        sz[ 1 ] = csStr[ 1 ];
        sz[ 2 ] = csStr[ 2 ];
        sz[ 3 ] = csStr[ 3 ];
        sscanf( sz, "%d", &tt.tm_year );
        tt.tm_year -= 1900;
    break;
    
    
    default:
    return 0;
    }



return 1;
}
















//will count number of occurrences of spec char, starts from 'pos'
int mystr::count_occurrence_char( char ch, int pos )
{
int len = iLen;

if( len == 0 ) return 0;

if ( pos >= len ) return 0;

int count = 0;
for (int i = pos; i < len; i++ )
	{
	if( csStr[ i ] == ch ) count++;
	}

return count;
}








//cut head of string (in need be) using spec delimiter, will keep str at a spec maximum delimit occurrence, keeps right (tail),
//useful to tail a log of lines to 'max_delimits'
//returns 1 if shortened, else 0
bool mystr::tail( char delim, int max_delimits )
{
if( max_delimits <= 0 ) return 0;

int lines = count_occurrence_char( delim, 0 );

if( lines <= max_delimits ) return 0;

int cut_cnt = lines - max_delimits;

printf("lines: %d, max_delimits: %d, cut_cnt: %d\n", lines, max_delimits, cut_cnt );

string s2 = csStr;

string s1;
if( !cut_just_passed_nth_occurrence_keep_right( s1, delim, cut_cnt, 0 ) )
	{
	csStr = s2;
	return 0;
	}

csStr = s1;

iLen = csStr.length();
return 1;
}










//will count number of occurrences of spec char, from 'start_pos' up to 'end_pos'
//returns number of occurrences
int mystr::count_occurrence_of_char_up_to_pos( char ch, int start_pos, int end_pos )
{
int len = iLen;

if( len == 0 ) return 0;

if( start_pos > end_pos ) return 0;

if( start_pos >= len ) return 0;

if( end_pos > len ) end_pos = len;

int cnt = 0;
for (int i = start_pos; i < end_pos; i++ )
	{
    if( csStr[ i ] == ch ) cnt++;
    }

return cnt;
}






//finds position of nth occrence of 'ch',
//nth must greater than zero,
//returns -1 if not found, else nth's pos
int mystr::get_pos_of_nth_occurrence( char ch, int nth, int start_pos )
{
int len = iLen;


if( len == 0 ) return -1;

if( start_pos >= len ) return -1;

if( nth <= 0 )  return -1;

int cnt = 0;
for (int i = start_pos; i < len; i++ )
	{
    if( csStr[ i ] == ch ) cnt++;
	if( cnt == nth ) return i;
    }

return -1;
}





//cuts at nth occurrence of 'ch',
//nth must greater than zero,
//will also cut off nth occurrence, returns 1 if a cut happended, else 0
int mystr::cut_at_nth_occurrence(  string &sret, char ch, int nth, int start_pos )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

if( start_pos >= len ) return 0;

if( nth <= 0 )  return 0;

int cnt = 0;
for (int i = start_pos; i < len; i++ )
	{
    if( csStr[ i ] == ch ) cnt++;
	if( cnt == nth )
		{
		sret = sret.substr( 0 , i  );		//extract first string part
		return 1;
		}	
    }


return 0;
}



//cuts at nth occurrence of 'ch',
//nth must greater than zero,
//will NOT cut off nth occurrence, returns 1 if a cut happended, else 0
int mystr::cut_just_past_nth_occurrence( string &sret, char ch, int nth, int start_pos )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

if( start_pos >= len ) return 0;

if( nth <= 0 )  return 0;

int cnt = 0;
for (int i = start_pos; i < len; i++ )
	{
    if( csStr[ i ] == ch ) cnt++;
	if( cnt == nth )
		{
		sret = sret.substr( 0 , i + 1 );		//extract first string part
		return 1;
		}	
    }


return 0;
}








//cuts at nth occurrence of 'ch', keeping right chars
//nth must greater than zero,
//will NOT cut off nth occurrence, returns 1 if a cut happended, else 0

// !!! WILL STILL return full string if cut did not happen, this can be tested as it returns 0
int mystr::cut_at_nth_occurrence_keep_right( string &sret, char ch, int nth, int start_pos )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

if( start_pos >= len ) return 0;

if( nth <= 0 )  return 0;

int cnt = 0;
for (int i = start_pos; i < len; i++ )
	{
    if( csStr[ i ] == ch ) cnt++;
	if( cnt == nth )
		{
		sret = sret.substr( i , len - i  );			//extract second string part
		return 1;
		}	
    }

return 0;
}







//cuts at nth occurrence of 'ch', keeping right chars
//nth must greater than zero,
//will also cut off nth occurrence, returns 1 if a cut happended, else 0

// !!! WILL STILL return full string if cut did not happen, this can be tested as it returns 0
int mystr::cut_just_passed_nth_occurrence_keep_right( string &sret, char ch, int nth, int start_pos )
{
int len = iLen;

sret = csStr;

if( len == 0 ) return 0;

if( start_pos >= len ) return 0;

if( nth <= 0 )  return 0;

int cnt = 0;
for (int i = start_pos; i < len; i++ )
	{
    if( csStr[ i ] == ch ) cnt++;
	if( cnt == nth )
		{
		if ( i >= ( len - 1 ) )						//nothing past cut?
			{
			sret = "";									//added this in v1.24 
			return 1;
			}
		sret = sret.substr( i + 1, len - i - 1  );			//extract second string part
		return 1;
		}	
    }

return 0;
}







//will test csStr only contains 2 supplied chars, starts from 'pos'
//returns 1 if only contains spec chars, else returns 0

//usefull for checking if a string only contains white spaces e.g: CR, LF ( 0x0d, 0x0a )
int mystr::check_only_contains_2chars( char ch0, char ch1, int pos )
{
int len = iLen;
char ch;

if( len == 0 ) return 1;

if ( pos >= len ) return 1;

for (int i = pos; i < len; i++ )
	{
	ch = csStr[ i ];
	if( ( ch == ch0 ) || ( ch == ch1 ) ) continue;
	return 0;							//if here did not meet 2 char req
	}

return 1;
}








//will test csStr only contains 3 supplied chars, starts from 'pos'
//returns 1 if only contains spec chars, else returns 0

//usefull for checking if a string only contains white spaces e.g: Space, CR, LF ( 0x20, 0x0d, 0x0a )
int mystr::check_only_contains_3chars( char ch0, char ch1, char ch2, int pos )
{
int len = iLen;
char ch;

if( len == 0 ) return 1;

if ( pos >= len ) return 1;

for (int i = pos; i < len; i++ )
	{
	ch = csStr[ i ];
	if( ( ch == ch0 ) || ( ch == ch1 ) || (  ch == ch2 ) ) continue;
	return 0;							//if here did not meet 3 char req
	}

return 1;
}









//will test csStr only contains 4 supplied chars, starts from 'pos'
//returns 1 if only contains spec chars, else returns 0

//usefull for checking if a string only contains white spaces e.g: Space, Tab, CR, LF ( 0x20, 0x09, 0x0d, 0x0a )
int mystr::check_only_contains_4chars( char ch0, char ch1, char ch2, char ch3, int pos )
{
int len = iLen;
char ch;

if( len == 0 ) return 1;

if ( pos >= len ) return 1;

for (int i = pos; i < len; i++ )
	{
	ch = csStr[ i ];
	if( ( ch == ch0 ) || ( ch == ch1 ) || (  ch == ch2 ) || (  ch == ch3 ) ) continue;
	return 0;							//if here did not meet 4 char req
	}

return 1;
}






//format bytes into readable str - handles TB, PB and EB
void mystr::make_filesize_str_ulli( string &sret, unsigned long long int bytes )
{
string s;
double dbytes = bytes;

//format filesize str

if( bytes < 1024 )
	{
	strpf( s,"%d  B ",(int)dbytes );
	}

if( ( bytes >= 1024 ) && ( bytes < 1048576 ) )
	{
	strpf( s,"%.2f KB ", (double)dbytes / (double)1024.0 );
	}

if( ( bytes >= 1048576 ) && ( bytes < (double)1073741824 ) )
	{
	strpf( s,"%.2f MB ", (double)dbytes / (double)1048576 );
	}

if( ( bytes >= 1073741824) && ( bytes < 1099511627776ULL ) )
	{
	strpf( s,"%.2f GB ", (double)dbytes / (double)1073741824ULL );
	}

if( ( bytes >= 1099511627776ULL ) && ( bytes < 1125899906842624ULL ) )
	{
	strpf( s,"%.2f TB ", (double)dbytes / (double)1099511627776ULL );
	}

if( ( bytes >= 1125899906842624ULL ) && ( bytes < 1152921504606846976ULL ) )
	{
	strpf( s,"%.2f PB ", (double)dbytes / (double)1125899906842624ULL );
	}

if( bytes >= 1152921504606846976ULL )
	{
	strpf( s,"%.2f EB ", (double)dbytes / (double)1152921504606846976ULL );
	}


sret = s;
}











//format bytes into readable strings - handles TB, PB and EB
//supplies the numbers and units in separate without padding
//the number of digits after the decimal point can be spec, with limits
void mystr::make_filesize_str_ulli_discrete( string &snum, string &sunits, int fractional_digits, unsigned long long int bytes )
{
string s;
double dbytes = bytes;

//format filesize str

if( fractional_digits < 0 ) fractional_digits = 2;
if( fractional_digits > 5 ) fractional_digits = 5;

if( bytes < 1024 )
	{
    strpf( snum,"%d",(int)dbytes );
	strpf( sunits,"B" );
	}

if( ( bytes >= 1024 ) && ( bytes < 1048576 ) )
	{
    if( fractional_digits == 0 ) strpf( snum,"%.0f", (double)dbytes / (double)1024.0 );
    if( fractional_digits == 1 ) strpf( snum,"%.1f", (double)dbytes / (double)1024.0 );
    if( fractional_digits == 2 ) strpf( snum,"%.2f", (double)dbytes / (double)1024.0 );
    if( fractional_digits == 3 ) strpf( snum,"%.3f", (double)dbytes / (double)1024.0 );
    if( fractional_digits == 4 ) strpf( snum,"%.4f", (double)dbytes / (double)1024.0 );
    if( fractional_digits == 5 ) strpf( snum,"%.5f", (double)dbytes / (double)1024.0 );
	strpf( sunits,"KB" );
	}

if( ( bytes >= 1048576 ) && ( bytes < (double)1073741824 ) )
	{
	if( fractional_digits == 0 ) strpf( snum,"%.0f", (double)dbytes / (double)1048576 );
	if( fractional_digits == 1 ) strpf( snum,"%.1f", (double)dbytes / (double)1048576 );
	if( fractional_digits == 2 ) strpf( snum,"%.2f", (double)dbytes / (double)1048576 );
	if( fractional_digits == 3 ) strpf( snum,"%.3f", (double)dbytes / (double)1048576 );
	if( fractional_digits == 4 ) strpf( snum,"%.4f", (double)dbytes / (double)1048576 );
	if( fractional_digits == 5 ) strpf( snum,"%.5f", (double)dbytes / (double)1048576 );
	strpf( sunits,"MB" );
	}

if( ( bytes >= 1073741824) && ( bytes < 1099511627776ULL ) )
	{
	strpf( snum,"%.2f", (double)dbytes / (double)1073741824ULL );
	strpf( sunits,"GB" );
	}

if( ( bytes >= 1099511627776ULL ) && ( bytes < 1125899906842624ULL ) )
	{
	strpf( snum,"%.2f", (double)dbytes / (double)1099511627776ULL );
	strpf( sunits,"TB" );
	}

if( ( bytes >= 1125899906842624ULL ) && ( bytes < 1152921504606846976ULL ) )
	{
	strpf( snum,"%.2f", (double)dbytes / (double)1125899906842624ULL );
	strpf( sunits,"PB" );
	}

if( bytes >= 1152921504606846976ULL )
	{
	strpf( snum,"%.2f", (double)dbytes / (double)1152921504606846976ULL );
	strpf( sunits,"EB" );
	}

}











//mystr m1;

//double dvalue = 1.234567890123456789e6;
//int fractional_digits = 3;

//string snum, sunits, scombined;

//m1.make_engineering_str( snum, sunits, scombined, fractional_digits, dvalue, " ", "Hz" );

//printf(" dvalue: %f,  snum: '%s', units: '%s' scombined: '%s'\n", dvalue, snum.c_str(), sunits.c_str(), scombined.c_str() );


//dvalue = 1.234567890123456789e-12;
//fractional_digits = 3;

//m1.make_engineering_str( snum, sunits, scombined, fractional_digits, dvalue, " ", "S" );

//printf(" dvalue: %f,  snum: '%s', units: '%s' scombined: '%s'\n", dvalue, snum.c_str(), sunits.c_str(), scombined.c_str() );

//see above example code
//format 'dvalue' into enigeering notation strings
//specify 'fractional_digits' to give an number of digits after decimal point,
//supplies a 'scombined' which is: snum + sappend_num + sappend_units,
//specify 'sappend_num' to append a string to 'scombined' e.g: " "
//specify 'sappend_units' to append a string to number e.g: "Hz"

//e.g: with fractional_digits = 3; 
//e.g. 1.23456789e18 		= 1.234 E
//e.g. 1.23456789e15 		= 1.234 P
//e.g. 1.23456789e12 		= 1.234 T
//e.g. 1.23456789e9 		= 1.234 G
//e.g. 1.23456789e6 		= 1.234 M
//e.g. 1.23456789e3 		= 1.234 K
//e.g. 1.23456789 			= 1.234
//e.g. 0.123456789 			= 0.123 
//e.g. 0.0123456789 		= 12.345 m
//e.g. 0.00123456789 		= 1.234 m
//e.g. 1.23456789e-6  		= 1.234 u
//e.g. 1.23456789e-9  		= 1.234 n
//e.g. 1.23456789e-12  		= 1.234 p
//e.g. 1.23456789e-15  		= 1.234 f  ...as in femto
//e.g. 1.23456789e-18  		= 1.234 a  ...as in atto

//see also: 'make_engineering_str_exp()'
void mystr::make_engineering_str( string &snum, string &sunits, string &scombined, int fractional_digits, double dvalue, string sappend_num, string sappend_units )
{
string s1, sprec;

if( fractional_digits < 0 ) fractional_digits = 2;
if( fractional_digits > 17 ) fractional_digits = 17;

double has_sign = 1;
if( dvalue < 0.0 )
	{
	has_sign = -1;
	dvalue = -dvalue;
	}

sprec = "\%.0f";
if( fractional_digits == 1 ) sprec = "\%.1f";
if( fractional_digits == 2 ) sprec = "\%.2f";
if( fractional_digits == 3 ) sprec = "\%.3f";
if( fractional_digits == 4 ) sprec = "\%.4f";
if( fractional_digits == 5 ) sprec = "\%.5f";
if( fractional_digits == 6 ) sprec = "\%.6f";
if( fractional_digits == 7 ) sprec = "\%.7f";
if( fractional_digits == 8 ) sprec = "\%.8f";
if( fractional_digits == 9 ) sprec = "\%.9f";
if( fractional_digits == 10 ) sprec = "\%.10f";
if( fractional_digits == 11 ) sprec = "\%.11f";
if( fractional_digits == 12 ) sprec = "\%.12f";
if( fractional_digits == 13 ) sprec = "\%.13f";
if( fractional_digits == 14 ) sprec = "\%.14f";
if( fractional_digits == 15 ) sprec = "\%.15f";
if( fractional_digits == 16 ) sprec = "\%.16f";
if( fractional_digits == 17 ) sprec = "\%.17f";


if( dvalue == 0.0 )
	{
	goto done;
	}

if( dvalue < 1e-1 )
	{

	if( ( dvalue >= 1e-3 ) && ( dvalue < 1e-1 ) )
		{
		dvalue /= 1e-3;
		strpf( sunits, "m" );
		goto done;
		}

	if( ( dvalue >= 1e-6 ) && ( dvalue < 1e-3 ) )
		{
		dvalue /= 1e-6;
		strpf( sunits, "u" );
		goto done;
		}

	if( ( dvalue >= 1e-9 ) && ( dvalue < 1e-6 ) )
		{
		dvalue /= 1e-9;
		strpf( sunits, "n" );
		goto done;
		}

	if( ( dvalue >= 1e-12 ) && ( dvalue < 1e-9  ) )
		{
		dvalue /= 1e-12;
		strpf( sunits, "p" );
		goto done;
		}

	if( ( dvalue >= 1e-15 ) && ( dvalue < 1e-12 ) )
		{
		dvalue /= 1e-15;
		strpf( sunits, "f" );			//femto
		goto done;
		}

	if( dvalue < 1e-15 )
		{
		dvalue /= 1e-18;
		strpf( sunits, "a" );			//atto
		goto done;
		}
	}
else{
	if( dvalue < 1e3 )
		{
		sunits = "";
		goto done;
		}

	if( ( dvalue >= 1e3 ) && ( dvalue < 1e6 ) )
		{
		dvalue /= 1e3;
		strpf( sunits, "K" );
		goto done;
		}

	if( ( dvalue >= 1e6 ) && ( dvalue < 1e9 ) )
		{
		dvalue /= 1e6;
		strpf( sunits, "M" );
		goto done;
		}

	if( ( dvalue >= 1e9) && ( dvalue < 1e12 ) )
		{
		dvalue /= 1e9;
		strpf( sunits, "G" );
		goto done;
		}

	if( ( dvalue >= 1e12 ) && ( dvalue < 1e15  ) )
		{
		dvalue /= 1e12;
		strpf( sunits, "T" );
		}

	if( ( dvalue >= 1e15 ) && ( dvalue < 1e18 ) )
		{
		dvalue /= 1e15;
		strpf( sunits, "P" );
		goto done;
		}

	if( dvalue >= 1e18 )
		{
		dvalue /= 1e18;
		strpf( sunits, "E" );
		goto done;
		}

	}

done:
strpf( snum, sprec.c_str(), dvalue * has_sign );
strpf( scombined, "%s%s%s%s", snum.c_str(), sappend_num.c_str(), sunits.c_str(), sappend_units.c_str() );
}










//mystr m1;

//double dvalue = 1.234567890123456789e6;
//int fractional_digits = 3;

//string snum, sunits, scombined;
//bool inc_plus = 1;
//m1.make_engineering_str_exp( snum, sunits, scombined, fractional_digits, dvalue, " ", "Hz", inc_plus );

//printf(" dvalue: %f,  snum: '%s', units: '%s' scombined: '%s'\n", dvalue, snum.c_str(), sunits.c_str(), scombined.c_str() );


//dvalue = 1.234567890123456789e-12;
//fractional_digits = 3;

//m1.make_engineering_str_exp( snum, sunits, scombined, fractional_digits, dvalue, " ", "S", inc_plus );

//printf(" dvalue: %f,  snum: '%s', units: '%s' scombined: '%s'\n", dvalue, snum.c_str(), sunits.c_str(), scombined.c_str() );

//see above example code
//format 'dvalue' into enigeering notation strings
//specify 'fractional_digits' to give an number of digits after decimal point,
//supplies a 'scombined' which is: snum + sappend_num + sappend_units,
//specify 'sappend_num' to append a string to 'scombined' e.g: " "
//specify 'sappend_units' to append a string to number e.g: "Hz"
//specify 'inc_plus' for '+' to be included after 'e' as in 1.234 e+6

//e.g: with fractional_digits = 3; 
//e.g. 1.23456789e18 		= 1.234 e18 	or e+18
//e.g. 1.23456789e15 		= 1.234 e15		or e+15
//e.g. 1.23456789e12 		= 1.234 e12 	or e+12
//e.g. 1.23456789e9 		= 1.234 e9 		or e+9
//e.g. 1.23456789e6 		= 1.234 e6 		or e+6
//e.g. 1.23456789e3 		= 1.234 e3 		or e+3
//e.g. 1.23456789 			= 1.234
//e.g. 0.123456789 			= 0.123 
//e.g. 0.0123456789 		= 12.345 e-3
//e.g. 0.00123456789 		= 1.234 e-6
//e.g. 1.23456789e-6  		= 1.234 e-9
//e.g. 1.23456789e-9  		= 1.234 e-9
//e.g. 1.23456789e-12  		= 1.234 e-12
//e.g. 1.23456789e-15  		= 1.234 e-15  ...as in femto
//e.g. 1.23456789e-18  		= 1.234 e-18  ...as in atto

//see also: 'make_engineering_str()'
void mystr::make_engineering_str_exp( string &snum, string &sunits, string &scombined, int fractional_digits, double dvalue, string sappend_num, string sappend_units, bool inc_plus )
{
string s1, sprec;

if( fractional_digits < 0 ) fractional_digits = 2;
if( fractional_digits > 17 ) fractional_digits = 17;

double has_sign = 1;
if( dvalue < 0.0 )
	{
	has_sign = -1;
	dvalue = -dvalue;
	}

sprec = "\%.0f";
if( fractional_digits == 1 ) sprec = "\%.1f";
if( fractional_digits == 2 ) sprec = "\%.2f";
if( fractional_digits == 3 ) sprec = "\%.3f";
if( fractional_digits == 4 ) sprec = "\%.4f";
if( fractional_digits == 5 ) sprec = "\%.5f";
if( fractional_digits == 6 ) sprec = "\%.6f";
if( fractional_digits == 7 ) sprec = "\%.7f";
if( fractional_digits == 8 ) sprec = "\%.8f";
if( fractional_digits == 9 ) sprec = "\%.9f";
if( fractional_digits == 10 ) sprec = "\%.10f";
if( fractional_digits == 11 ) sprec = "\%.11f";
if( fractional_digits == 12 ) sprec = "\%.12f";
if( fractional_digits == 13 ) sprec = "\%.13f";
if( fractional_digits == 14 ) sprec = "\%.14f";
if( fractional_digits == 15 ) sprec = "\%.15f";
if( fractional_digits == 16 ) sprec = "\%.16f";
if( fractional_digits == 17 ) sprec = "\%.17f";


if( dvalue == 0.0 )
	{
	goto done;
	}


if( dvalue < 1e-1 )
	{

	if( ( dvalue >= 1e-3 ) && ( dvalue < 1e-1 ) )
		{
		dvalue /= 1e-3;
		strpf( sunits, "e-3" );
		goto done;
		}

	if( ( dvalue >= 1e-6 ) && ( dvalue < 1e-3 ) )
		{
		dvalue /= 1e-6;
		strpf( sunits, "e-6" );
		goto done;
		}

	if( ( dvalue >= 1e-9 ) && ( dvalue < 1e-6 ) )
		{
		dvalue /= 1e-9;
		strpf( sunits, "e-9" );
		goto done;
		}

	if( ( dvalue >= 1e-12 ) && ( dvalue < 1e-9  ) )
		{
		dvalue /= 1e-12;
		strpf( sunits, "e-12" );
		goto done;
		}

	if( ( dvalue >= 1e-15 ) && ( dvalue < 1e-12 ) )
		{
		dvalue /= 1e-15;
		strpf( sunits, "e-15" );			//femto
		goto done;
		}

	if( dvalue < 1e-15 )
		{
		dvalue /= 1e-18;
		strpf( sunits, "e-18" );			//atto
		goto done;
		}
	}
else{
	string splus;
	if( inc_plus ) splus = "+";
	if( dvalue < 1e3 )
		{
		sunits = "";
		goto done;
		}

	if( ( dvalue >= 1e3 ) && ( dvalue < 1e6 ) )
		{
		dvalue /= 1e3;
		strpf( sunits, "e%s3", splus.c_str() );
		goto done;
		}

	if( ( dvalue >= 1e6 ) && ( dvalue < 1e9 ) )
		{
		dvalue /= 1e6;
		strpf( sunits, "e%s6", splus.c_str() );
		goto done;
		}

	if( ( dvalue >= 1e9) && ( dvalue < 1e12 ) )
		{
		dvalue /= 1e9;
		strpf( sunits, "e%s9", splus.c_str() );
		goto done;
		}

	if( ( dvalue >= 1e12 ) && ( dvalue < 1e15  ) )
		{
		dvalue /= 1e12;
		strpf( sunits, "e%s12", splus.c_str() );
		}

	if( ( dvalue >= 1e15 ) && ( dvalue < 1e18 ) )
		{
		dvalue /= 1e15;
		strpf( sunits, "e%s15", splus.c_str() );
		goto done;
		}

	if( dvalue >= 1e18 )
		{
		dvalue /= 1e18;
		strpf( sunits, "e%s18", splus.c_str() );
		goto done;
		}

	}

done:
strpf( snum, sprec.c_str(), dvalue * has_sign );
strpf( scombined, "%s%s%s%s", snum.c_str(), sappend_num.c_str(), sunits.c_str(), sappend_units.c_str() );
}











//adds supplied dir seperator to end of string if it does not have one
void mystr::add_slash_at_end_if_it_does_not_have_one( string &path, string dir_sep )
{
int len = path.length();

if( len < 1 ) return;

if( dir_sep.length() == 0 ) return;

if ( path[ len - 1 ] != dir_sep[ 0 ] ) path += dir_sep[ 0 ];

}







int grrr_cnt = 0;


//creates a dir or file list stored in 'vlist', note some vars MUST to be ZEROED before calling
//set 'which' to: 0 = dir only list,      1 = file/link only list
//set 'absolute_path' to a drive or dir
//set 'dir' to a dir to start creating list at
//set 'dir_sep' to either "/" or "\\" to suit o/s
//vector<st_mystr_make_dir_file_list_tag> list, must be cleared before calling as this code is re-entrant,
//set 'max_entries' to 0 if no limit to vlist length is required,
//set 'recursive' to step into sub directories
//set 'cur_count' to zero before calling as this code is re-entrant,
//set 'depth' to zero before calling as this code is re-entrant,
//set 'bytes_tot' to zero before calling as this code is re-entrant,
//returns 1 on success, else 0

//moded v1.79 to include 'st_mystr_make_dir_file_list_tag.path_relative'		//this does not included a leading 'dir_sep'  '/' 
//note: re-entrant code





bool mystr::make_dir_file_list( bool which, string absolute_path, string dir, string dir_sep, unsigned long long int max_entries, bool recursive, vector<st_mystr_make_dir_file_list_tag> &vlist, unsigned long long int &cur_count, unsigned long long int &depth, unsigned long long int &bytes_tot )
{

//----------------
//linux code
#ifndef compile_for_windows 
mystr m1;

bool store, skip;

struct stat statbuf;
unsigned long long int big;
string subdir, path, tmp_pathname;
string sfile_size, sfile_mod_date;
st_mystr_make_dir_file_list_tag o;



//ffd.dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY;

//tmpdir = dir;

m1.add_slash_at_end_if_it_does_not_have_one( dir, dir_sep );

path = absolute_path;
path += dir;



grrr_cnt++;

DIR *dp;
struct dirent *dirp;

//printf ( "make_dir_file_list() - trying opendir: '%s'  %d\n", path.c_str(), grrr_cnt );
dp  = opendir( (char*)path.c_str() );


if( dp == NULL )
	{
	if( mystr_verbose ) printf ( "make_dir_file_list() - error opening dir: '%s' \n", path.c_str() );
	return 0;
	}




while( 1 )
	{
    skip = 0;
	store = 0;

    errno = 0;          //clear this, if no more dir entries are found by readdir(), it does not change errno, it just returns null which means no error happened,
                        //so if errno is zero and readdir() returned null then it can be assumed no more entries are avail
	dirp = readdir( dp );
	if ( dirp == NULL )
		{
		if( errno != 0 )    //if readdir() returned null, and errno is non zero, then an error did occur
			{
            if( mystr_verbose )
                {
                printf( "---------\n" );
    			printf( "make_dir_file_list() - error from call to readdir()\n" );
                printf( "error at path: '%s'\n", path.c_str() );
                printf( "error= %d, '%s'\n", errno, strerror( errno ) );
                printf( "---------\n" );
                }
            break;
			}
		else{
			break;
			}
		}



	if( dirp->d_type == DT_DIR )						//directory found?
		{
		if ( strcmp( dirp->d_name, ".") == 0 ) skip = 1;
		if ( strcmp( dirp->d_name, "..") == 0 ) skip = 1;

		if ( ( !skip ) && ( recursive ) )
			{

//			subdir = dir + "/";
			subdir = dir;
			subdir += dirp->d_name;						//add new directory

            depth++;;
//printf("absolute_path: '%s'\n", absolute_path.c_str() );
//printf("================>relative dir: '%s'\n", subdir.c_str() );

			make_dir_file_list( which, absolute_path, subdir, dir_sep, max_entries, recursive, vlist, cur_count, depth, bytes_tot );		//re-enter this function
            depth--;
			}
			
		}

    o.depth = depth;

    if( which == 0 )                                                    	//dir req?
        {
        if( !skip  )
            {
            if( dirp->d_type == DT_DIR ) 		                            //dir found?
                {
                o.file = 0;
                o.link = 0;

                cur_count ++;
                store = 1;   
//printf("================>relative dir: '%s'\n", subdir.c_str() );
                }
            }
        }
    else{
        if( !skip  )
            {
            if( ( dirp->d_type == DT_REG  ) ||( dirp->d_type == DT_LNK )  )		//file/link found?
                {
                if( dirp->d_type == DT_REG ) 		                            //file found?
                    {
                    o.file = 1;
                    o.link = 0;
                    }
                else{
                    o.file = 0; 		                                         //link found
                    o.link = 1;
                    }

//printf("================>relative dir: '%s'\n", subdir.c_str() );

                tmp_pathname = path;
                m1.add_slash_at_end_if_it_does_not_have_one( path, dir_sep );

                tmp_pathname += dirp->d_name;

//printf("lstat( '%s'\n", tmp_pathname.c_str() );

                big = 0;
                int status;
                statbuf.st_size = -1ULL;
                status = lstat( (char*)tmp_pathname.c_str(), &statbuf );
                if( status != -1 )
                    {
                    big = (unsigned long long int) statbuf.st_size;
                    o.fsize = big;

                    o.st_mode = statbuf.st_mode;        //copy over 'struct stat' fields
//                    o.st_nlink = statbuf.st_nlink;    /left these out to be compatible with windows
//                    o.st_uid = statbuf.st_uid;
//                    o.st_gid = statbuf.st_gid;
                    o.st_rdev = statbuf.st_rdev;
                    o.st_size = statbuf.st_size;
                    o.st_size64 = big;
                    o.mtime = statbuf.st_mtime;
                    o.atime = statbuf.st_atime;
                    o.ctime = statbuf.st_ctime;
                    
//					o.statbuf = statbuf;
                    
                    bytes_tot += big;                   //keep a tally total filesize bytes
                    }
                else{
                    if( mystr_verbose )
                        {
                        printf( "---------\n" );
                        printf( "make_dir_file_list() - error from call to lstat(), fname: '%s'\n", tmp_pathname.c_str() );
                        printf( "error= %d, '%s'\n", errno, strerror( errno ) );
                        printf( "---------\n" );
                        }
                    }
                
                cur_count ++;
                store = 1;   
                }

            }
        }



	if ( store )
		{
        o.path = path;
        o.path_relative = dir;										//v1.79
        o.fname = dirp->d_name;
		vlist.push_back( o );
//printf("================>relative dir: '%s'\n", dir.c_str() );
		}

    if( max_entries != 0 )                                                   //limit specified?
        {
        if( cur_count >= max_entries ) break;                                //limit reached?
        }
	}

if( dp != 0 )
	{
	if ( closedir( dp ) == -1 )
		{
		if( mystr_verbose ) printf( "make_dir_file_list() - error after calling closedir()\n", path.c_str() );
		return 0;
		}
    }


return 1;

#endif 

//----------------




//----------------
//windows code
#ifdef compile_for_windows 
mystr m1;

bool store, skip;
WIN32_FIND_DATAW ffd;
HANDLE hFind;
unsigned long long int big;
string s1, subdir, path, sfname;
string sfile_size, sfile_mod_date;
wstring ws;
st_mystr_make_dir_file_list_tag o;
struct stat statbuf;

//ffd.dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY;

unsigned long long int file_cnt = 0;

//tmpdir = dir;

m1.add_slash_at_end_if_it_does_not_have_one( dir, dir_sep );

path = absolute_path;
path += dir;

s1 = path + "*.*";

m1 = s1;
m1.mbcstr_wcstr( ws );								//conv mbc str to wchar str

hFind = FindFirstFileW( ws.c_str() , &ffd );


if ( hFind == INVALID_HANDLE_VALUE )
	{
	if( mystr_verbose ) printf ( "make_dir_file_list() - error opening dir: '%s'\n", s1.c_str() );
	return 0;
	}




while( 1 )

	{
    skip = 0;
	store = 0;

	if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )     			//dir?
	   {
		if ( wcscmp( ffd.cFileName, L".") == 0 ) skip = 1;
		if ( wcscmp( ffd.cFileName, L"..") == 0 ) skip = 1;

		if ( ( !skip ) && ( recursive ) )
			{
            o.file = 0;
			o.link = 0;

			subdir = dir;

			ws = ffd.cFileName;
			m1.wcstr_mbcstr( ws, sfname );		//conv wchar str to mbc str
			subdir += sfname;

            depth++;;
			make_dir_file_list( which, absolute_path, subdir, dir_sep, max_entries, recursive,  vlist, cur_count, depth, bytes_tot );		//re-enter this function
            depth--;
			}
        }

    o.depth = depth;

    if( which == 0 )														//dir req?
        {
        if( !skip  )
            {
			if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )           //dir found?
				{
				o.file = 0;
				o.link = 0;

				cur_count ++;
				store = 1;   
				}
			}
		}
	else{
        if( !skip  )
			{
			if( ! ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )			//dir found?
				{
				if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT  )		//link found?
					{
					o.file = 0;
					o.link = 1;
					}
				else{
					o.file = 1; 		                                        //file found
					o.link = 0;
					}


				ws = ffd.cFileName;
				m1.wcstr_mbcstr( ws, sfname );		//conv wchar str to mbc str

                string tmp_pathname = path;
                m1.add_slash_at_end_if_it_does_not_have_one( path, dir_sep );

                tmp_pathname += sfname;


                int status;
                statbuf.st_size = -1;
                status = stat( (char*)tmp_pathname.c_str(), &statbuf );
                if( status != -1 )
                    {
                    big = (unsigned long long int) statbuf.st_size;
                    o.st_mode = statbuf.st_mode;        //copy over 'struct stat' fields
//                    o.st_nlink = statbuf.st_nlink;
//                    o.st_uid = statbuf.st_uid;
//                    o.st_gid = statbuf.st_gid;
                    o.st_rdev = statbuf.st_rdev;
                    o.st_size = statbuf.st_size;
                    o.st_size64 = big;
                    o.mtime = statbuf.st_mtime;
                    o.atime = statbuf.st_atime;
                    o.ctime = statbuf.st_ctime;
//                    o.fsize = big;
                    
//                  bytes_tot += big;                   //keep a tally total filesize bytes
                    }
                else{
                    if( mystr_verbose )
                        {
                        printf( "---------\n" );
                        printf( "make_dir_file_list() - error from call to lstat(), fname: '%s'\n", tmp_pathname.c_str() );
                        printf( "error= %d, '%s'\n", errno, strerror( errno ) );
                        printf( "---------\n" );
                        }
                    }





			big = 0;
			big |= ffd.nFileSizeHigh;						//make 2 dwords a 64 bit var
			big = big << 32;
			big |= ffd.nFileSizeLow;

			o.fsize = big;
            bytes_tot += big;                   //keep a tally total filesize bytes

	//		ws = ffd.cFileName;
	//		m1.wcstr_mbcstr( ws, sfname );          //conv wchar str to mbc str

				cur_count ++;
				store = 1;
				}
			}
		}


	if ( store )
		{

//		ws = ffd.cFileName;
//		m1.wcstr_mbcstr( ws, s1 );						//conv wchar str to mbc str
//		fprintf( fp, "%s%s%s%s%s\r\n",sfile_size.c_str(), sfile_mod_date.c_str(), path.c_str(), slash.c_str(), s1.c_str() );

//		ws = ffd.cFileName;
//		m1.wcstr_mbcstr( ws, s1 );						//conv wchar str to mbc str
//		fprintf( fp, "%s%s%s%s%s\r\n",sfile_size.c_str(), sfile_mod_date.c_str(), path.c_str(), slash.c_str(), s1.c_str() );


		ws = ffd.cFileName;
		m1.wcstr_mbcstr( ws, sfname );		//conv wchar str to mbc str

        o.path = path;
        o.fname = sfname;
		vlist.push_back( o );

		}



    if( max_entries != 0 )                                                   //limit specified?
        {
        if( cur_count >= max_entries ) break;                                //limit reached?
        }


	if ( FindNextFileW( hFind, &ffd ) == 0 )
		{
		break;
		}
	}

FindClose( hFind );

return 1;

#endif 
//----------------

}

















//searches supplied vector for matching str entry,
//returns pos if found, else -1
int mystr::find_str_pos_in_list( vector<string> &vstr, string &ss )
{

for( int i = 0 ; i < vstr.size(); i++ )
        {
        if( vstr[ i ].compare( ss ) == 0  ) return i;
        }

return -1;
}






//searches supplied vector for matching str entry, if not present it adds it,
//sets 'pos' to where in list str resides,
//returns 1 if had to add the str,  else 0 if already in list
 
int mystr::add_str_if_not_in_list( vector<string> &vstr, string &ss, int &pos )
{

pos = find_str_pos_in_list( vstr, ss );

if( pos == -1  )                    //not in list?
    {
    pos = vstr.size();
    vstr.push_back( ss );
    return 1;
    }

return pos;
}







int mystr::length()
{
iLen = csStr.length();

return iLen;
}











//strip any leading 'ch' chars, if any
//returns 1, on strip being performed, else 0
bool mystr::strip_leading_chars( string &sret, char ch, int pos )
{
mystr m1;

if( iLen == 0 ) return 0;
if( pos >= iLen ) return 0;


for( int i = pos; i < iLen; i++ )
	{
	if( csStr[ i ] != ch )				//non strip char found?
		{
		sret = csStr.substr( i, iLen - i );			//fetch right string after last strip char
		return 1;
		}
	}

sret = csStr;
return 0;
}










//strip any leading 'c1' or 'c2' chars, if any
//returns 1, on strip being performed, else 0

//e.g: strip_leading_chars2( s1, ' ', '\t', 0 );         //will strip any leading spaces and tabs, and place result into 's1'
bool mystr::strip_leading_chars2( string &sret, char c1, char c2, int pos )
{
mystr m1;

if( iLen == 0 ) return 0;
if( pos >= iLen ) return 0;


for( int i = pos; i < iLen; i++ )
	{
	if( ( csStr[ i ] != c1 ) && ( csStr[ i ] != c2 ) )				//non strip char found?
		{
		sret = csStr.substr( i, iLen - i );			//fetch right string after last strip char
		return 1;
		}
	}

sret = csStr;
return 0;
}













//strip any trailing 'ch' chars, if any
//returns 1, on strip being performed, else 0
bool mystr::strip_trailing_char( string &sret, char ch )
{
mystr m1;

sret = "";

if( iLen == 0 ) return 0;


for( int i = iLen - 1; i >= 0; i-- )
	{
	if( csStr[ i ] != ch )				            //non strip char found?
		{
		sret = csStr.substr( 0, i + 1 );			    //fetch left string after last strip char
		return 1;
		}
	}

return 0;
}



















GCProfile::GCProfile(string csFilenameIn)
{
bFastMode=1;
bneed_save = 0;

csFilename=csFilenameIn;

Index();
}





GCProfile::~GCProfile()
{
if( bneed_save ) Save();


}








//returns true if cleared and saved ok 
int GCProfile::ClearAllProfileEntries()
{
FILE *fp;


string csSave = "";

fp=fopen(csFilename.c_str(),"w");				//open file for writing
if(fp)
	{
	fprintf(fp,"%s",csSave.c_str());
	fclose(fp);

	csAll="\n";	//make sure first line is a blank so first [Section] has a LF before it, helps simplify code for searching for a Section

	return 1;
	}

return 0;
}








bool GCProfile::Save()
{
FILE *fp;

//csFilename+="Test";

string csSave=csAll.substr(1,-1);				//remove initally added "\n" by function Index()

fp=fopen(csFilename.c_str(),"w");				//open file for writing
if(fp)
	{
//	fprintf(fp,"%s",csSave.c_str());
	fwrite( csSave.c_str(), 1, csSave.length(), fp ); 					//v1.80
	fclose(fp);
	return 1;
	}

return 0;
}



//read entire csFilename file into csAll str
int GCProfile::Index()
{
string csLine;
FILE *fp;
char szStr[cnMaxIniEntrySize];
int iLineNum=0;

fp=fopen(csFilename.c_str(),"r");

csAll="\n";	//make sure first line is a blank so first [Section] has a LF before it, helps simplify code for searching for a Section

if(fp)
	{
	while(1)
		{
		string csLine;
		strcpy(szStr,"");

		fgets(szStr,sizeof(szStr),fp);
		szStr[sizeof(szStr)-1]=0x0;			//ensure if str full its still terminated
		csLine=szStr;
		
		if (csLine.length()==0)						//blank line?
			{
			fclose(fp);

//string csTmp=csAll.substr(0,400);						//inset new key value
//printf("\nFoundIndex %s",csTmp.c_str());
//return 1;

			return 1;
			}

		mystr ms;
		ms=csLine;
		ms.ReplaceSeqNonVisible(csLine,"",0);		//delete leading white chars or non visible chars
		csAll+=csLine;
		}
	}
return 0;
}










//returns true if file exists
int GCProfile::Exists()
{
FILE *fp;

fp=fopen(csFilename.c_str(),"r");

if(fp!=0)
	{
	fclose(fp);
	return 1;
	}

return 0;
}









//looks for csSection and csKey if found obtains assigned string into csString (or csDefault if not found)
//returns line number if section/key found, or -1 if not
int GCProfile::GetPrivateProfileStr(string csSection,string csKey,string csDefault,string *csString)
{
string csTmp;

*csString=csDefault;									//assume returning csDefault

string csSectionFind="\n[";								//make string for '\n[Section]'
csSectionFind+=csSection;
csSectionFind+="]";

string csKeyFind="\n";
csKeyFind+=csKey;
csKeyFind+="=";											//make string for '\nkey='


int iPos=csAll.find(csSectionFind,0);		//Section key match on this line?
//printf("\nFound Get.. %s at %d\n",csSectionFind.c_str(),iPos);



if(iPos<0) return -1;						//not found?

int iPosEndSection=csAll.find("\n[",iPos+1);	//pos of next Section if any?


//csTmp=csAll.substr(iPos,iPosEndSection);				//v1.69 this is incorrect incorrectly spec 'len', v1.70 below is the fix -- make str from Section to next Section or end

csTmp=csAll.substr( iPos, iPosEndSection - iPos );		//v1.70 correctly specifying 'len', make str from Section to next Section or end
//printf("\nstripped %s\n",csTmp.c_str());
//printf("\nzzzzzz %d %d csTmp '%s'\n",iPos, iPosEndSection, csTmp.c_str());

iPos=csTmp.find(csKeyFind,0);					//pos of Key in this section, if any?
if(iPos>=0)
	{
//printf("\nFound NexSection.. at %d\n",iPosEndSection);
//printf("\nFound %s Get.. at %d\n",csKeyFind.c_str(),iPos);
	iPos=csTmp.find("=",iPos);					//pos of the Key's '='
	int iPosEnd=csTmp.find("\n",iPos);					//pos of LF if any
	csTmp=csTmp.substr(iPos+1,iPosEnd-iPos-1);		//fetch all after '='
	if(csTmp.length()==0) goto UseDefault;
	*csString=csTmp;
//printf("\n-Found value %s-\n",(*csString).c_str());
	return 1;
	}

UseDefault:
//printf("\nDefault value %s\n",(*csString).c_str());
return -1;


/*
FILE *fp;
char szStr[cnMaxIniEntrySize];
int iLineNum=0;

*csString=csDefault;									//assume returning csDefault

if(csSection.length()==0) return -1;
if(csKey.length()==0) return -1;



string csSectionFind="[";								//make string for '[section]'
csSectionFind+=csSection;
csSectionFind+="]";

string csKeyFind=csKey;
csKeyFind+="=";											//make string for 'key='


fp=fopen(csFilename.c_str(),"r");

if(fp)
	{
	while(1)
		{
		string csLine;
		
		strcpy(szStr,"");

		fgets(szStr,sizeof(szStr),fp);
		szStr[sizeof(szStr)-1]=0x0;			//ensure if str full its still terminated
		csLine=szStr;
//		printf("%s",csLine.c_str());
		if (csLine.length()==0)						//end of file?
			{
			fclose(fp);
			return -1;
			}
		iLineNum++;
		int iPos=csLine.find (csSectionFind,0);		//a section key match on this line?
		if(iPos<0) continue;						//if not continue
		if(iPos!=0) continue;						//if it is and not at start of line continue

			
//		printf("\r\nFound Section...");
		while(1)
			{
			strcpy(szStr,"");
			fgets(szStr,sizeof(szStr),fp);
			csLine=szStr;
			if (csLine.length()==0)
				{
				fclose(fp);
				return -1;
				}

			iLineNum++;
		
			if(csLine[0]=='[')						//is this line starting with a section key bracket?
				{
				fclose(fp);
				return -1;
				}
			int iEqualPos=csLine.find(csKeyFind,0);	//a key match on this line?
			if(iEqualPos<0) continue;					//if not continue
			if(iEqualPos!=0) continue;					//if it is and not at start of line continue

//csLine+="\r";
//csLine+="\n";
//csLine+="\n";
//csLine+="\r";

			while(1)									//clear any CRs LFs at end of line
				{
				int iLen;
				iLen=csLine.length();
				
				if(csLine[iLen-1]=='\r') {csLine=csLine.substr(0,iLen-1); continue;}	
				if(csLine[iLen-1]=='\n') {csLine=csLine.substr(0,iLen-1); continue;}
				break;	
				}
//			printf("\r\nFound Key...");
			iEqualPos=csLine.find ('=',0);				//get pos of equal sign?
//			printf("Equ=%d,Len=%d\r\n",iEqualPos,csLine.length());
//for(int i=0;i<csLine.length();i++)
//	{
//	printf("%02x ",csLine[i]);		
//	}
			if((iEqualPos+1)<(int)csLine.length())			//is there more after the equal sign?
				{
				*csString=csLine.substr(iEqualPos+1,csLine.npos);
				fclose(fp);
				return iLineNum;
				}
			else
				{
				fclose(fp);
				return iLineNum;						//key is equal to nothing
				}
			}

		}
	}

*/
return -1;												//returning csDefault
}







/*

//looks for csSection and csKey if found obtains assigned string into csString (or csDefault if not found)
//returns line number if section/key found, or -1 if not
int GCProfile::GetPrivateProfileString(string csSection,string csKey,string csDefault,string *csString)
{
	
if(bFastMode)
	{
	return GetPrivateProfileStringF(csSection,csKey,csDefault,csString);
	}

FILE *fp;
char szStr[cnMaxIniEntrySize];
int iLineNum=0;

*csString=csDefault;									//assume returning csDefault

if(csSection.length()==0) return -1;
if(csKey.length()==0) return -1;



string csSectionFind="[";								//make string for '[section]'
csSectionFind+=csSection;
csSectionFind+="]";

string csKeyFind=csKey;
csKeyFind+="=";											//make string for 'key='


fp=fopen(csFilename.c_str(),"r");

if(fp)
	{
	while(1)
		{
		string csLine;
		
		strcpy(szStr,"");

		fgets(szStr,sizeof(szStr),fp);
		szStr[sizeof(szStr)-1]=0x0;			//ensure if str full its still terminated
		csLine=szStr;
//		printf("%s",csLine.c_str());
		if (csLine.length()==0)						//end of file?
			{
			fclose(fp);
			return -1;
			}
		iLineNum++;
		int iPos=csLine.find (csSectionFind,0);		//a section key match on this line?
		if(iPos<0) continue;						//if not continue
		if(iPos!=0) continue;						//if it is and not at start of line continue

			
//		printf("\r\nFound Section...");
		while(1)
			{
			strcpy(szStr,"");
			fgets(szStr,sizeof(szStr),fp);
			csLine=szStr;
			if (csLine.length()==0)
				{
				fclose(fp);
				return -1;
				}

			iLineNum++;
		
			if(csLine[0]=='[')						//is this line starting with a section key bracket?
				{
				fclose(fp);
				return -1;
				}
			int iEqualPos=csLine.find(csKeyFind,0);	//a key match on this line?
			if(iEqualPos<0) continue;					//if not continue
			if(iEqualPos!=0) continue;					//if it is and not at start of line continue

//csLine+="\r";
//csLine+="\n";
//csLine+="\n";
//csLine+="\r";

			while(1)									//clear any CRs LFs at end of line
				{
				int iLen;
				iLen=csLine.length();
				
				if(csLine[iLen-1]=='\r') {csLine=csLine.substr(0,iLen-1); continue;}	
				if(csLine[iLen-1]=='\n') {csLine=csLine.substr(0,iLen-1); continue;}
				break;	
				}
//			printf("\r\nFound Key...");
			iEqualPos=csLine.find ('=',0);				//get pos of equal sign?
//			printf("Equ=%d,Len=%d\r\n",iEqualPos,csLine.length());
//for(int i=0;i<csLine.length();i++)
//	{
//	printf("%02x ",csLine[i]);		
//	}
			if((iEqualPos+1)<(int)csLine.length())			//is there more after the equal sign?
				{
				*csString=csLine.substr(iEqualPos+1,csLine.npos);
				fclose(fp);
				return iLineNum;
				}
			else
				{
				fclose(fp);
				return iLineNum;						//key is equal to nothing
				}
			}

		}
	}


return -1;												//returning csDefault
}

*/





/*

//looks for matching csSection
//returns line number if csSection found, or -1 if not
int GCProfile::GetPrivateProfileSectionLineNum(string csSection)
{
FILE *fp;
char szStr[cnMaxIniEntrySize];
int iLineNum=0;


if(csSection.length()==0) return -1;



string csSectionFind="[";								//make string for '[section]'
csSectionFind+=csSection;
csSectionFind+="]";


fp=fopen(csFilename.c_str(),"r");

if(fp)
	{
	while(1)
		{
		string csLine;
		
		strcpy(szStr,"");

		fgets(szStr,sizeof(szStr),fp);
		csLine=szStr;
		iLineNum++;

		if (csLine.length()==0)						//end of file?
			{
			fclose(fp);
			return -1;
			}

		int iPos=csLine.find (csSectionFind,0);		//a section key match on this line?
		if(iPos<0) continue;						//if not continue
		if(iPos!=0) continue;						//if it is and not at start of line continue
		fclose(fp);
		return iLineNum;						//key is equal to nothing
		}
	}

return -1;
}			



*/








//looks for csSection and csKey and modifies the string that csKey equals to
//will create csSection and csKey if required
//returns 1 always
bool GCProfile::WritePrivateProfileStr(string csSection,string csKey,string csString)
{
string csTmp;
int iPosEndSection;
bool bEndSectionFound=0;

mystr ms;
ms=csString;
ms.FindReplace(csString,"\n","",0);		//delete all LFs

string csSectionFind="\n[";								//make string for '\n[Section]'
csSectionFind+=csSection;
csSectionFind+="]";

string csKeyFind="\n";
csKeyFind+=csKey;
csKeyFind+="=";											//make string for '\nkey='


int iPosSection=csAll.find(csSectionFind,0);		//Section match on this line?
//printf("\nFound Get.. %s at %d\n",csSectionFind.c_str(),iPosSection);

if(iPosSection>=0)										//found Section?
	{

	int iPos=csAll.find(csKeyFind,iPosSection);			//pos of Key in this section, if any?
	if(iPos>=0)											//a Key found somewhere in str?
		{
		int iPosEndSection=csAll.find("\n[",iPosSection+1);	//pos of next Section if any?
		if(iPosEndSection>=0)							//next section found?
			{
			if(iPos<iPosEndSection)						//found Key within this section?
				{
				iPos=csAll.find("=",iPos);				//pos of the Key's '='
				int iPosEnd=csAll.find("\n",iPos);		//pos of LF if any
				csAll.erase(iPos+1,iPosEnd-iPos-1);		//erase all after '=' to end of line
				csAll.insert(iPos+1,csString);			//insert new key value
				
				}
			else{										//key not found in this section?
				csTmp+=csKeyFind;
				csTmp+=csString;
				csAll.insert(iPosEndSection,csTmp);		//insert new key value
			
				}
			}
		else{											//no following section	
			iPos=csAll.find("=",iPos);					//pos of the Key's '='
			int iPosEnd=csAll.find("\n",iPos);			//pos of LF if any
			csAll.erase(iPos+1,iPosEnd-iPos-1);			//erase all after '=' to end of line
			csAll.insert(iPos+1,csString);				//insert new key value
			}
		}
	else{												//key not found		
		int iPosEndSection=csAll.find("\n[",iPosSection+1);		//pos of next Section if any?
		if(iPosEndSection>=0)							//next Section found?
			{
			csTmp+=csKeyFind;
			csTmp+=csString;
			csAll.insert(iPosEndSection,csTmp);			//insert new key value
			}
		else{											//next Section not found	
			iPosEndSection=csAll.length()-1;			//insert key at end of str
			csTmp+=csKeyFind;
			csTmp+=csString;
			csAll.insert(iPosEndSection,csTmp);			//insert new key value
		
			}		
		}	

/*
	if(iPosEndSection>=0)						
		{
		if(iPos<iPosEndSection)							//found key within this section?
			{
			
			}
		}
	if( (iPos>=0)&&(!bEndSectionFound) )				//key found within section?
		{
		iPos=csAll.find("=",iPos);						//pos of the Key's '='
		int iPosEnd=csAll.find("\n",iPos);				//pos of LF if any
		csAll.erase(iPos+1,iPosEnd-iPos-1);				//erase all after '=' to end of line
		csAll.insert(iPos+1,csString);					//inset new key value

		}
	else{												//Section found but no Key
		if(iPosEndSection<0) iPosEndSection=csAll.length()-1; 	//if no more Sections found point to end of str
//printf("\nFound NexSection.. at %d\n",iPosEndSection);
		csTmp+=csKeyFind;
		csTmp+=csString;
		csAll.insert(iPosEndSection,csTmp);				//insert new key value
		}
*/
	}
else{													//No Section or Key
	iPosEndSection=csAll.length()-1;					//go to end
//printf("\nFound NexSection2.. at %d\n",iPosEndSection);
	csTmp+=csSectionFind;								//make Section
	csTmp+=csKeyFind;									//make Key
	csTmp+=csString;
	csAll.insert(iPosEndSection,csTmp);					//insert new key value
	}

bneed_save = 1;

//csTmp=csAll.substr(0,400);						//inset new key value
//printf("\nFound %s",csTmp.c_str());
return 1;
}


/*

//looks for csSection and csKey and modifies the string that csKey equals to
//will create file,csSection and csKey if required
//returns true if done ok
int GCProfile::WritePrivateProfileString(string csSection,string csKey,string csString)
{
FILE *fp;
char szStr[cnMaxIniEntrySize];
string csLine;
string csTotal;
string csTmp;
int iPos=0;
int iLineNumSection;
int iLineNumKey;
int iLineNumFile=0;
int iSkipAddingOriginalcsKey=0;

//printf("\r\nWritePrivateProfileString...\r\n");


WritePrivateProfileStringF(csSection,csKey,csString);
return 1;

//find if entry already exists and its line number
iLineNumKey=GetPrivateProfileString(csSection,csKey,"",&csTmp);
iLineNumSection=GetPrivateProfileSectionLineNum(csSection);		//get csSection line pos in any


//printf("\r\n\r\ncsLineNumSection=%d\r\n",iLineNumSection);
//printf("\r\n\r\ncsKeyLineNum=%d\r\n\r\n",iLineNumKey);

//if(iLineNumKey==-1)			//no csSection or no csKey?		
//	{
//	iLineNumSectionKey=GetPrivateProfileSectionLineNum(csSection);
//	
//	}


fp=fopen(csFilename.c_str(),"r");

if(fp!=0)					//read file lines into a string
	{
	while(1)
		{
		strcpy(szStr,"");
		fgets(szStr,sizeof(szStr),fp);
		szStr[sizeof(szStr)-1]=0x0;			//ensure if str full its still terminated
		iLineNumFile++;
		csLine=szStr;

		if (csLine.length()==0)
			{
			break;
			}

		while(1)									//clear any CRs LFs at end of line
			{
			int iLen;
			iLen=csLine.length();
			
			if(csLine[iLen-1]=='\r') {csLine=csLine.substr(0,iLen-1); continue;}	
			if(csLine[iLen-1]=='\n') {csLine=csLine.substr(0,iLen-1); continue;}
			break;	
			}

		if((iLineNumKey!=-1)&&(iLineNumSection!=-1))			//found csKey and csSection		
			{
			if(iLineNumFile==iLineNumKey)						//is this the line of csKey
				{
				csTotal+=csKey;									//add csKey
				csTotal+="=";
				csTotal+=csString;								//add csString
				csTotal+="\n";
				iSkipAddingOriginalcsKey=1;
				}
			}

		if(iSkipAddingOriginalcsKey)
			{
			iSkipAddingOriginalcsKey=0;
			}
		else{
			csLine+="\n";								//add only LF
			csTotal+=csLine;
			}

		if((iLineNumKey==-1)&&(iLineNumSection!=-1))			//no csKey but found csSection		
			{
			if(iLineNumFile==iLineNumSection)				//is this the line of csSection
				{
				csTotal+=csKey;									//add csKey
				csTotal+="=";
				csTotal+=csString;								//add csString
				csTotal+="\n";
				}
			}


		}
	fclose(fp);



	if((iLineNumSection==-1)&&(iLineNumKey==-1))			//no csSection and no csKey		
		{
		csTotal+="[";	
		csTotal+=csSection;								//add csSection
		csTotal+="]\n";
		csTotal+=csKey;									//add csKey
		csTotal+="=";
		csTotal+=csString;								//add csString
		csTotal+="\n";
		}
	}



string csNewFilename;
csNewFilename=csFilename;
//csNewFilename+="_2";
fp=fopen(csNewFilename.c_str(),"w");				//open file for writing
if(fp)
	{
//printf("\r\n%s",csTotal.c_str());
	int iLen=csTotal.length();

	while(1)			//output the modified PrivateProfileString file
		{
		string csTmp;
		int iLFPos=csTotal.find ('\n',iPos);
//printf("\r\n LFPos=%d, Pos=%d",iLFPos,iPos);
		
		if(iLFPos==-1)
			{
			iLFPos=iLen;
			}

		csTmp=csTotal.substr(iPos,iLFPos-iPos+1);
//		printf("%s",csTmp.c_str());
		fprintf(fp,"%s",csTmp.c_str());
		
		iPos=iLFPos+1;


		if(iPos>=iLen)
			{
			break;
			}
		}
	fclose(fp);
	return 1;
	}



fclose(fp);
return 0;
}


*/




long GCProfile::GetPrivateProfileLONG(string csSection,string csKey,long lDefault)
{
char szDefault[15];
string csDefault;
string csRet;
long lRet;

sprintf(szDefault,"%ld",lDefault);

csDefault=szDefault;

GetPrivateProfileStr(csSection,csKey,csDefault,&csRet);

sscanf(csRet.c_str(),"%ld",&lRet);

return lRet;
}












float GCProfile::GetPrivateProfileFLOAT(string csSection,string csKey,float fDefault)
{
char szDefault[15];
string csDefault;
string csRet;
float fRet;

sprintf(szDefault,"%f",fDefault);

csDefault=szDefault;

GetPrivateProfileStr(csSection,csKey,csDefault,&csRet);

sscanf(csRet.c_str(),"%f",&fRet);

return fRet;
}











double GCProfile::GetPrivateProfileDOUBLE(string csSection,string csKey,double dDefault)
{
char szDefault[15];
string csDefault;
string csRet;
double dRet;

sprintf(szDefault,"%lg",dDefault);

csDefault=szDefault;

GetPrivateProfileStr(csSection,csKey,csDefault,&csRet);

sscanf(csRet.c_str(),"%lg",&dRet);

return dRet;
}









uint64_t GCProfile::GetPrivateProfile_uint64_t(string csSection,string csKey, uint64_t ui_default )
{
char szDefault[ 64 ];
string csDefault;
string csRet;
uint64_t ret;

sprintf( szDefault,"%" PRIu64 "", ui_default );

csDefault = szDefault;

GetPrivateProfileStr( csSection, csKey, csDefault, &csRet);

sscanf( csRet.c_str(),"%" PRIu64 "" ,&ret );

return ret;
}





uint64_t GCProfile::GetPrivateProfile_hex_uint64_t(string csSection,string csKey, uint64_t ui_default )
{
char szDefault[ 64 ];
string csDefault;
string csRet;
uint64_t ret;

sprintf( szDefault,"%" PRIx64 "", ui_default );

csDefault = szDefault;

GetPrivateProfileStr( csSection, csKey, csDefault, &csRet);

sscanf( csRet.c_str(),"%" PRIx64 "" ,&ret );

return ret;
}












//get a comma delimited line of parameters, convert chars that are represented by a backslash and hex number back to ascii
//non printable chars are also stored as backslash followed by char's hex values e.g:
// Comma= \2c
// CR=  \0d
// LF=  \0a
// BS=  \08
// ESC= \1b
// BEL= \07
// BackSlash= \5c
int GCProfile::GetPrivateProfileParams(string csSection,string csKey,string csDefault,string csParams[],int iParamCount)
{
char szTmp[3];
string csRead,csStr;
double dRet;
int iLen,iParamsRead=0;
int c,j=0;

if(iParamCount<=0) return 0;			//no param array supplied?

GetPrivateProfileStr(csSection,csKey,csDefault,&csRead); //read profile str


iLen=csRead.length();
if(iLen==0) return 0;					//if nothing here can't proceed, must be no csDefault supplied either

for(int i=0;i<iParamCount;i++)			//process each parameter
	{
	csStr="";

	for(;j<iLen;j++)				//read and conv profile str into params
		{
		c=csRead[j];
		if(c==',')
			{
			j++;						//skip over comma
			if(j>=iLen) goto FinParam;
			
			break;						//do next param, if any
			}

		if(c=='\\')						//if backslash read in next two hex chars
			{
			j++;
			if(j>=iLen) goto FinParam;
			szTmp[0]=csRead[j];
			j++;
			if(j>=iLen) goto FinParam;
			szTmp[1]=csRead[j];
			szTmp[2]=0;

			sscanf(szTmp,"%x",&c);		//conv 2 hex char to an ascii char
			}
		
		csStr+=c;						//store char to build param

		}
FinParam:
	csParams[i]=csStr;					//store completed param
//printf("\nGet %d = %s\n",i,csParams[i].c_str());

	}


return iParamsRead;
}






//write lNum to PrivateProfile using csSection,csKey
//returns 1 if OK, or 0 if fail
int GCProfile::WritePrivateProfileLONG(string csSection,string csKey,long lNum)
{
char szNum[15];
string csNum;
string csRet;

sprintf(szNum,"%ld",lNum);

csNum=szNum;

int iRet=WritePrivateProfileStr(csSection,csKey,csNum);


return iRet;
}














//write fNum to PrivateProfile using csSection,csKey
//returns 1 if OK, or 0 if fail
int GCProfile::WritePrivateProfileFLOAT(string csSection,string csKey,float fNum)
{
char szNum[15];
string csNum;
string csRet;

sprintf(szNum,"%f",fNum);

csNum=szNum;

int iRet=WritePrivateProfileStr(csSection,csKey,csNum);


return iRet;
}






//write dNum to PrivateProfile using csSection,csKey
//returns 1 if OK, or 0 if fail
int GCProfile::WritePrivateProfileDOUBLE(string csSection,string csKey,double dNum)
{
char szNum[15];
string csNum;
string csRet;

sprintf(szNum,"%lg",dNum);

csNum=szNum;

int iRet=WritePrivateProfileStr(csSection,csKey,csNum);

return iRet;
}





//write dNum to PrivateProfile using csSection,csKey
//you MUST specify sprintf formatted string to be used, e.g: sprecision = "%.17g"
//returns 1 if OK, or 0 if fail
int GCProfile::WritePrivateProfileDOUBLE_precision(string csSection,string csKey, double dNum, string sprecision )
{
char szNum[32];
string csNum;
string csRet;

sprintf(szNum, sprecision.c_str(), dNum );

csNum=szNum;

int iRet=WritePrivateProfileStr(csSection,csKey,csNum);

return iRet;
}





//write uint64_t to PrivateProfile using csSection, csKey
//returns 1 if OK, or 0 if fail
int GCProfile::WritePrivateProfile_uint64_t( string csSection, string csKey, uint64_t ui )
{
char szNum[ 64 ];
string csNum;
string csRet;

sprintf( szNum,"%" PRIu64 "", ui );

csNum = szNum;

int iRet = WritePrivateProfileStr(csSection,csKey,csNum);


return iRet;
}







//write uint64_t as hex to PrivateProfile using csSection, csKey
//returns 1 if OK, or 0 if fail
int GCProfile::WritePrivateProfile_hex_uint64_t( string csSection, string csKey, uint64_t ui )
{
char szNum[ 64 ];
string csNum;
string csRet;

sprintf( szNum,"%" PRIx64 "", ui );

csNum = szNum;

int iRet = WritePrivateProfileStr(csSection,csKey,csNum);


return iRet;
}






//write a comma delimited line of parameters, if parameter needs to hold a comma it uses '\2c'
//non printable chars are stored as backslash followed by char's hex values e.g:
// Comma= \2c
// CR=  \0d
// LF=  \0a
// BS=  \08
// ESC= \1b
// BEL= \07
// BackSlash= \5c
int GCProfile::WritePrivateProfileParams(string csSection,string csKey,string csParams[],int iParamCount)
{
char szNum[15];
string csStr,csTmp;
string csRet;
int c;
int iLen;

if(iParamCount<=0) goto finwrite;		//write empty param line

for(int i=0;i<iParamCount;i++)			//process each parameter
	{
	iLen=csParams[i].length();
	for(int j=0;j<iLen;j++)	
		{
		c=csParams[i][j];
		if( (c<0x20)||(c==',')||(c=='\\') )	//is char to be represented as a '\xx' char
			{
			csStr+='\\';
			sprintf(szNum,"%02x",c);
			szNum[2]=0;					//limit to max of 2 hex chars
			csStr+=szNum;
			}
		else csStr+=c;					//keep char as normal
		}
	csStr+=',';							//add delimiter
	}

iLen=csStr.length();
if(iLen!=0)								//anything to store?
	{
	csStr=csStr.substr(0,iLen-1);		//remove last char which should be ending comma
	}
//sprintf(szNum,"%lg",dNum);

//csNum=szNum;

finwrite:
int iRet=WritePrivateProfileStr(csSection,csKey,csStr);

return iRet;
}







//string class printf(...) like function
//e.g. strpf(csStr,"\nTesing int=%05d, float=%f str=%s\n",z,(float)4.567,"Garry was Here");

void strpf(string &str,const char *fmt,...)
{
char szTmp [ 1 * 1024 * 1024 ];
va_list ap;

va_start(ap, fmt);
vsnprintf(szTmp,sizeof(szTmp)-1,fmt, ap);
str=szTmp;

va_end(ap);
}











