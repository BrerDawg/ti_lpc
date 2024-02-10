/*
Copyright (C) 2022 BrerDawg

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

//GCLed.h
//---- v1.09

#ifndef gcled_h
#define gcled_h

#include <FL/Fl.H>
//#include <FL/Fl_Window.H>
//#include <FL/Fl_Box.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include "GCProfile.h"

using namespace std;

#define cnMaxColIndex 10

enum en_gcled_style             //see led_style
{
cn_gcled_style_square,
cn_gcled_style_round,
};



class GCLed : public Fl_Widget
{
private:
bool has_focus;

int iR[cnMaxColIndex];
int iG[cnMaxColIndex];
int iB[cnMaxColIndex];
int iIndex;

void (*p_mouse_button_changed_cb)( void* obj, void *args ); 
void *cb_mouse_button_changed_obj;
void *cb_mouse_button_changed_args;


void (*p_mouse_event_cb)( void* obj, void *args ); 
void *cb_mouse_event_obj;
void *cb_mouse_event_args;

public:
int led_style;                           //e.g: could be: cn_gcled_style_square, cn_gcled_style_round
int id;
int id2;
bool left_button;
bool middle_button;
bool right_button;
int idelta_r;						//for brightness offset of currently shown led col
int idelta_g;
int idelta_b;
int last_event;



private:
void draw();
int handle( int );

public:
GCLed( int x, int y, int w, int h, const char *label );	
~GCLed();
int SetColIndex(int iIndex,int iRin,int iGin,int iBin);
int GetColIndex();
int set_col_from_str( string rbg_list, char delimiter );

bool ChangeCol( int iIn );
void set_mouse_button_changed_callback( void (*p_cb)( void*, void* ), void *obj_in, void *args_in );
void set_index_col_rgb( unsigned int idx, uint8_t rr, uint8_t bb, uint8_t gg );
void adj_brightness_offset( int idelta_r_in, int idelta_g_in, int idelta_b_in );
void adj_brightness_of_col_index( unsigned int idx, int idelta_r, int idelta_b, int idelta_g );
void set_mouse_event_callback( void (*p_cb)( void*, void* ), void *obj_in, void *args_in );

};

#endif
