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

//GCCol.cpp
//color picker widget

//v1.01     03-11-11
//v1.02     20-09-2014              //added flag 'changed' which may be read or reset by public code, GCCol sets this when colour is changed by user actions
                                    //also changed callback from 'int (*pCallback)( Fl_Widget *w, int e )', to: void (*pCallback)( Fl_Widget *w, void* e );

//v1.03     07-feb-2017				//added FL_MOUSEWHEEL control of cols, extremes val limits now reached when dragging out past control's borders
//v1.04     03-may-2017				//added 'show_user_col_dlg()', runs when user clicks in the RHS mix col box


#include "GCCol.h"
using namespace std;



GCCol::GCCol( int x, int y, int w, int h, const char *label = 0 ) : Fl_Widget( x, y, w, h, label )
{
//type( 0 );
box( FL_UP_BOX );
//color( FL_BACKGROUND_COLOR );
//selection_color( FL_BACKGROUND_COLOR );


//Fl_Tooltip::enable();

align( FL_ALIGN_LEFT );

//pCallback = 0;

left_button = 0;
capture_col = 0;

obj_r.val = 240;										//inital colour
obj_g.val = 200;
obj_b.val = 180;

changed = 0;
showing_col_dlg = 0;

}






GCCol::~GCCol()
{
	
}
 
 


void GCCol::value( string s )
{
sscanf( s.c_str(), "%d,%d,%d", &obj_r.val, &obj_g.val, &obj_b.val );

if( obj_r.val < 0 ) obj_r.val = 0;
if( obj_r.val > cn_gccol_col_max - 1 ) obj_r.val = cn_gccol_col_max - 1;

if( obj_g.val < 0 ) obj_g.val = 0;
if( obj_g.val > cn_gccol_col_max - 1 ) obj_g.val = cn_gccol_col_max - 1;

if( obj_b.val < 0 ) obj_b.val = 0;
if( obj_b.val > cn_gccol_col_max - 1 ) obj_b.val = cn_gccol_col_max - 1;

redraw();
}






void GCCol::set_col_via_str( string s, bool do_callbck )
{
sscanf( s.c_str(), "%d,%d,%d", &obj_r.val, &obj_g.val, &obj_b.val );

if( obj_r.val < 0 ) obj_r.val = 0;
if( obj_r.val > cn_gccol_col_max - 1 ) obj_r.val = cn_gccol_col_max - 1;

if( obj_g.val < 0 ) obj_g.val = 0;
if( obj_g.val > cn_gccol_col_max - 1 ) obj_g.val = cn_gccol_col_max - 1;

if( obj_b.val < 0 ) obj_b.val = 0;
if( obj_b.val > cn_gccol_col_max - 1 ) obj_b.val = cn_gccol_col_max - 1;

if( do_callbck ) do_callback();

redraw();
}









const char* GCCol::value()
{
string s1;

get_col_via_str( s1, ',' );

svalue = s1;

return svalue.c_str();

}








void GCCol::get_col_via_str( string &s, char delim )
{
char sz[ 255 ];

sprintf( sz, "%d%c%d%c%d", obj_r.val, delim, obj_g.val, delim,  obj_b.val );

s = sz;
}







int GCCol::get_r()
{
return obj_r.val;
}




int GCCol::get_g()
{
return obj_g.val;
}



int GCCol::get_b()
{
return obj_b.val;
}






void GCCol::set_r( int r, bool do_callbck )
{
obj_r.val = r;

if( r < 0 ) obj_r.val = 0;
if( r > cn_gccol_col_max - 1 ) obj_r.val = cn_gccol_col_max - 1;

if( do_callbck ) do_callback();

redraw();

if( do_callbck ) do_callback();
}






void GCCol::set_g( int g, bool do_callbck )
{
obj_g.val = g;

if( g < 0 ) obj_g.val = 0;
if( g > cn_gccol_col_max - 1 ) obj_g.val = cn_gccol_col_max - 1;


redraw();

if( do_callbck ) do_callback();
}






void GCCol::set_b( int b, bool do_callbck )
{
obj_b.val = b;

if( b < 0 ) obj_b.val = 0;
if( b > cn_gccol_col_max - 1 ) obj_b.val = cn_gccol_col_max - 1;

redraw();

if( do_callbck ) do_callback();
}









bool GCCol::show_user_col_dlg()
{
showing_col_dlg = 1;

unsigned char r = (unsigned char)obj_r.val;
unsigned char g = (unsigned char)obj_g.val;
unsigned char b = (unsigned char)obj_b.val;

if( fl_color_chooser( "Set a Colour", r, g, b ) )
	{
	obj_r.val = r;
	obj_g.val = g;
	obj_b.val = b;

	showing_col_dlg = 0;
	return 1;
	}

showing_col_dlg = 0;
return 0;
}







void GCCol::draw()
{
mix_box_ll = x() + cn_gccol_xgap + cn_gccol_col_rec_wid + 2;
mix_box_rr = mix_box_ll + cn_gccol_col_rec_wid - 1;
mix_box_tt = y() + cn_gccol_yoffset;
mix_box_bb = mix_box_tt + h() - 1;


obj_r.x = x() + cn_gccol_xgap;
obj_r.y = y() + cn_gccol_yoffset;
obj_r.wid = cn_gccol_col_rec_wid;
obj_r.hei = cn_gccol_col_rect_hei;


obj_g.x = x() + cn_gccol_xgap;
obj_g.y = y() + cn_gccol_yoffset + cn_gccol_ygap + cn_gccol_col_rect_hei;
obj_g.wid = cn_gccol_col_rec_wid;
obj_g.hei = cn_gccol_col_rect_hei;


obj_b.x = x() + cn_gccol_xgap;
obj_b.y = y() + cn_gccol_yoffset + 2 * cn_gccol_ygap + 2 * cn_gccol_col_rect_hei;
obj_b.wid = cn_gccol_col_rec_wid;
obj_b.hei = cn_gccol_col_rect_hei;


fl_line_style( FL_SOLID );
fl_color( 0, 0, 0 );

fl_rect( x(), y() , w(), h() );			//overall control rectangle


//draw red selection rectangle
fl_rect( obj_r.x, obj_r.y , obj_r.wid, obj_r.hei );
for( int i = 0; i < cn_gccol_col_rec_wid; i++ )
	{
	fl_color( i * 4, 0, 0 );
//	fl_point( x() + obj_r.x + i , y() + obj_r.y );

	for( int j = 1; j < 5; j++ )
		{
		fl_point( obj_r.x + i , obj_r.y + j );
		}
	}

//draw red cursor pos
fl_color( 255, 255, 255 );

for( int i = 1; i < 5; i++ )
	{
	int xx = obj_r.x + obj_r.val / ( cn_gccol_col_max / cn_gccol_col_rec_wid );
	fl_color( 255, 255, 255 );
//	if( xx <= obj_r.x ) xx = obj_r.x + 1;
	if( xx > ( obj_r.x + obj_r.wid - 2 ) ) xx = obj_r.x + obj_r.wid - 2;
	fl_point( xx , obj_r.y + i );
	fl_point( xx + 1 , obj_r.y + i );
	}


//draw green selection rectangle
fl_color( 0, 0, 0 );
fl_rect( obj_g.x, obj_g.y , obj_g.wid, obj_g.hei );
for( int i = 0; i < cn_gccol_col_rec_wid; i++ )
	{
	fl_color( 0, i * 4, 0 );

	for( int j = 1; j < 5; j++ )
		{
		fl_point( obj_g.x + i , obj_g.y + j );
		}
	}

//draw green cursor pos
fl_color( 255, 255, 255 );

for( int i = 1; i < 5; i++ )
	{
	int xx = obj_g.x + obj_g.val / ( cn_gccol_col_max / cn_gccol_col_rec_wid );
//	if( obj_g.val > cn_gccol_col_max / 2 ) fl_color( 0, 0, 0 );
//	else  fl_color( 255, 255, 255 );
	
	fl_color( 255, 255, 255 );
	if( xx > ( obj_g.x + obj_g.wid - 2 ) ) xx = obj_g.x + obj_g.wid - 2;
	fl_point( xx , obj_g.y + i );
	fl_point( xx + 1 , obj_g.y + i );
	}




//draw blue selection rectangle
fl_color( 0, 0, 0 );
fl_rect( obj_b.x, obj_b.y , obj_b.wid, obj_b.hei );
for( int i = 0; i < cn_gccol_col_rec_wid; i++ )
	{
	fl_color( 0, 0, i * 4 );

	for( int j = 1; j < 5; j++ )
		{
		fl_point( obj_b.x + i , obj_b.y + j );
		}
	}

//draw blue cursor pos
fl_color( 255, 255, 255 );

for( int i = 1; i < 5; i++ )
	{
	int xx = obj_b.x + obj_b.val / ( cn_gccol_col_max / cn_gccol_col_rec_wid );
	fl_color( 255, 255, 255 );
	if( xx > ( obj_b.x + obj_b.wid - 2 ) ) xx = obj_b.x + obj_b.wid - 2;
	fl_point( xx , obj_b.y + i );
	fl_point( xx + 1 , obj_b.y + i );
	}



//draw summed resultant col
fl_color( 0, 0, 0 );
fl_rect( x() + cn_gccol_col_rec_wid , y() , cn_gccol_col_box_wid, h() );
fl_color( 255, 255, 255 );
fl_rect( x() + cn_gccol_col_rec_wid + 1 , y() + 1 , cn_gccol_col_box_wid - 2, h() - 2 );
fl_color( obj_r.val, obj_g.val, obj_b.val );
fl_rectf( x() + cn_gccol_col_rec_wid + 2 , y() + 2 , cn_gccol_col_box_wid - 4, h() - 4 );	//show summed col


//  draw_label();

//fl_rectf( x()+1, y() + 1, w() - 2, h() - 2 , iR[iIndex], iG[iIndex], iB[iIndex] );


}








int GCCol::handle(int e)
{
bool need_redraw = 0;
bool dont_pass_on = 0;
bool do_callbck = 0;

//return Fl_Widget::handle(e);

if ( e & FL_ENTER ) dont_pass_on = 1;		//need this for tooltip


if ( e & FL_LEAVE ) dont_pass_on = 1;		//need this for tooltip


//	return
//printf("Event=%d\n", e );

if ( e & FL_MOVE )
	{
	curx = Fl::event_x();
	cury = Fl::event_y();

	bool capture = 0;
	if( left_button )
		{
		//in red rect?
		if( capture_col == 1 )
			{
			obj_r.val = ( curx - obj_r.x ) * cn_gccol_col_max / cn_gccol_col_rec_wid;	//adj value
			if( obj_r.val < 0 ) obj_r.val = 0;
			if( obj_r.val > ( cn_gccol_col_max - 1 ) ) obj_r.val = cn_gccol_col_max - 1; 	//adj value
			do_callbck = 1;
			need_redraw = 1;

			capture = 1;
			}


		//in green rect?
		if( capture_col == 2 )
			{
			obj_g.val = ( curx - obj_g.x ) * cn_gccol_col_max / cn_gccol_col_rec_wid;	//adj value
			if( obj_g.val < 0 ) obj_g.val = 0;
			if( obj_g.val > ( cn_gccol_col_max - 1 ) ) obj_g.val = cn_gccol_col_max - 1; 	//adj value
			do_callbck = 1;
			need_redraw = 1;

			capture = 1;
			}

		//in blue rect?
		if( capture_col == 3 )
			{
			obj_b.val = ( curx - obj_b.x ) * cn_gccol_col_max / cn_gccol_col_rec_wid;	//adj value
			if( obj_b.val < 0 ) obj_b.val = 0;
			if( obj_b.val > ( cn_gccol_col_max - 1 ) ) obj_b.val = cn_gccol_col_max - 1; 	//adj value
			do_callbck = 1;
			need_redraw = 1;

			capture = 1;
			}


		if( ( !capture ) & ( !showing_col_dlg ) )
			{
			if( ( cury >= mix_box_tt  ) & ( cury <= mix_box_bb ) )
				{
				if( ( curx >= mix_box_ll  ) & ( curx <= mix_box_rr ) )
					{
//					printf("curx: %d, cury: %d\n", curx, cury );
					if( show_user_col_dlg() )
						{
						do_callbck = 1;
						changed = 1;
						}
					}
				}
			}
		}

	if( changed ) dont_pass_on = 1;
	need_redraw = 1;
	}


if ( e == FL_PUSH ) 
	{
//	exit(0);
//	if ( Fl::visible_focus() && handle( FL_FOCUS )) Fl::focus( this );
	if( Fl::event_button() == 1 )
		{
		curx = Fl::event_x();
		cury = Fl::event_y();
		left_button = 1;

		//in red rect?
		if( ( curx > ( obj_r.x ) ) && ( curx < ( obj_r.x + obj_r.wid ) )   )
			{
			if( ( cury > ( obj_r.y ) ) && ( cury < ( obj_r.y + obj_r.hei ) )   )
				{
				obj_r.val = ( curx - obj_r.x ) * cn_gccol_col_max / cn_gccol_col_rec_wid;	//adj value
				capture_col = 1;
				do_callbck = 1;
                changed = 1;
				}
			}


		//in green rect?
		if( ( curx > ( obj_g.x ) ) && ( curx < ( obj_g.x + obj_g.wid ) )   )
			{
			if( ( cury > ( obj_g.y ) ) && ( cury < ( obj_g.y + obj_g.hei ) )   )
				{
				obj_g.val = ( curx - obj_g.x ) * cn_gccol_col_max / cn_gccol_col_rec_wid;	//adj value
				capture_col = 2;
				do_callbck = 1;
                changed = 1;
				}
			}

		//in blue rect?
		if( ( curx > ( obj_b.x ) ) && ( curx < ( obj_b.x + obj_b.wid ) )   )
			{
			if( ( cury > ( obj_b.y ) ) && ( cury < ( obj_b.y + obj_b.hei ) )   )
				{
				obj_b.val = ( curx - obj_b.x ) * cn_gccol_col_max / cn_gccol_col_rec_wid;	//adj value
				capture_col = 3;
				do_callbck = 1;
                changed = 1;
				}
			}

		dont_pass_on = 1;
		need_redraw = 1;
		}
	}



if ( e == FL_RELEASE ) 
	{
	if( Fl::event_button() == 1 )
		{
		left_button = 0;
		capture_col = 0;
		dont_pass_on = 1;
		}
	}


	//mousewheel
if ( e == FL_MOUSEWHEEL )
	{
	int mousewheel = Fl::event_dy();

	if( ( curx > ( obj_r.x ) ) && ( curx < ( obj_r.x + obj_r.wid ) )   )
		{
		if( ( cury > ( obj_r.y ) ) && ( cury < ( obj_r.y + obj_r.hei ) )   )
			{
			obj_r.val -= mousewheel;
			if( obj_r.val < 0 ) obj_r.val = 0;
			if( obj_r.val > ( cn_gccol_col_max - 1 ) ) obj_r.val = cn_gccol_col_max - 1; 	//adj value
			do_callbck = 1;
			need_redraw = 1;
			}
		}


	if( ( curx > ( obj_g.x ) ) && ( curx < ( obj_g.x + obj_g.wid ) )   )
		{
		if( ( cury > ( obj_g.y ) ) && ( cury < ( obj_g.y + obj_r.hei ) )   )
			{
			obj_g.val -= mousewheel;
			if( obj_g.val < 0 ) obj_g.val = 0;
			if( obj_g.val > ( cn_gccol_col_max - 1 ) ) obj_g.val = cn_gccol_col_max - 1; 	//adj value
			do_callbck = 1;
			need_redraw = 1;
			}
		}


	if( ( curx > ( obj_b.x ) ) && ( curx < ( obj_b.x + obj_b.wid ) )   )
		{
		if( ( cury > ( obj_b.y ) ) && ( cury < ( obj_b.y + obj_r.hei ) )   )
			{
			obj_b.val -= mousewheel;
			if( obj_b.val < 0 ) obj_b.val = 0;
			if( obj_b.val > ( cn_gccol_col_max - 1 ) ) obj_b.val = cn_gccol_col_max - 1; 	//adj value
			do_callbck = 1;
			need_redraw = 1;
			}
		}
	}

if( need_redraw )
	{
	redraw();
	}

if( do_callbck )
	{
    do_callback();
    
//	if( pCallback ) (*pCallback)( this, (void*)e ); 			//do callback?
	}

if( !dont_pass_on )
	{
	return Fl_Widget::handle(e);
	}
	

return 1;

}











/*
void GCCol::set_event_callback(void (*pFunct)( Fl_Widget *w, void* e ) )
{
pCallback = pFunct;

}
*/



