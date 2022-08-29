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

//GCCol.h
//v1.02     20-09-2014
//v1.03     07-feb-2017
//v1.04     03-may-2017

#ifndef GCCol_h
#define GCCol_h

#include <string>

#include <FL/Fl.H>
//#include <FL/Fl_Window.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Color_Chooser.H>



using namespace std;

#define cn_gccol_col_max 256

#define cn_gccol_xgap 0
#define cn_gccol_ygap 0
#define cn_gccol_yoffset 1


#define cn_gccol_col_rec_wid 64							//selection rectangle
#define cn_gccol_col_rect_hei 6

#define cn_gccol_col_box_wid 20								//summed colour display


struct gccol_tag
{
int x, y, wid, hei;
int val;
};


class GCCol : public Fl_Widget 
{
private:
bool left_button;
int capture_col;								//1=red , 2=grn, 3=blue
gccol_tag obj_r;
gccol_tag obj_g;
gccol_tag obj_b;
string svalue;
int curx, cury;
int mix_box_ll;
int mix_box_rr;
int mix_box_tt;
int mix_box_bb;

//void (*pCallback)( Fl_Widget *w, void* e );

public:
bool changed;                           //set if colour was changed
bool showing_col_dlg;


private:
int handle(int);
void draw();
bool show_user_col_dlg();

public:
GCCol(int x, int y, int w, int h, const char *label );	
~GCCol();
void set_col_via_str( string s, bool do_callbck );
void value( string s );

void get_col_via_str( string &s, char delim );
const char* value();

int get_r();
int get_g();
int get_b();
void set_r( int r, bool do_callbck );
void set_g( int g, bool do_callbck );
void set_b( int b, bool do_callbck );

//void set_event_callback( void (*pFunct)(Fl_Widget *w, void* e ) );
};


#endif

