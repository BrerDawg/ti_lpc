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

//GCLed.cpp
//---- v1.01    10-09-07        
//---- v1.02    20-09-14        //modified to be a subclass of Fl_Widget (used to be Fl_Window), this was done so it could placed in a pack or scroll widget, as in pref.cpp
                                //modified callback for use in pref.cpp, changed 'int (*pCallback)( Fl_Widget *w, int e )', to: void (*pCallback)( Fl_Widget *w, void* e );
                                //added outline when control has focus
                                //added set_col_from_str(..)
//---- v1.03    2017-oct-30     //modified/nullyfied 'if ( e & FL_LEAVE )' code, it no longer sets 'dont_pass_on = 1', so handle() no
								//longer returns 1 for FL_LEAVE messages, without this the parent app was blocked from receiving menu keybrd shortcuts like:  "FL_CTRL + 'c'"
//---- v1.04    2018-oct-30     //added 'id' 'id2' to help identify this obj when multiple instances are in use
//---- v1.05    2020-mar-18     //moded all non space keydowns to pass onto parent
//---- v1.06    2020-sep-19     //chaged 'if ( e & FL_ENTER )' to 'if ( e == FL_ENTER )' as this was stopping a parent 'FL_Scroll' scrolling via mousewheel when mouse inside 'GCLed'
//---- v1.07    2020-oct-04     //added 'set_mouse_button_changed_callback()' and public 'left_button' 'middle_button' 'right_button' state flags
//---- v1.08    2023-jan-07     //added 'set_col_from_rgb()' and 'adj_brightness_offset()' and 'adj_brightness_of_col_index()'
//---- v1.09    2023-oct-26     //added 'set_mouse_event_callback()' and 'last_event'

#include "GCLed.h"


using namespace std;



/*
//EXAMPLE CODE callback using 'set_mouse_button_changed_callback()'
//shown is a 4 state 4 coloured led

void cb_led_chrd_rec(Fl_Widget* w, void*v)
{
GCLed *wdj = (GCLed*)w;
int which = (intptr_t)v;


int z = wdj->GetColIndex();
int z2 = z;

bool gone_into_play = 0;
bool gone_into_record = 0;
bool gone_into_record_stop = 0;

if( wdj->left_button )
	{
	if( z == 0 ) { z2 = 1; gone_into_play = 1; }
	if( z == 1 ) z2 = 0; 
	if( z == 2 ) z2 = 0; 
	if( z == 3 ) { z2 = 0; gone_into_record_stop = 1; } 
	}

if( wdj->middle_button )
	{
	if( z == 1 ) z2 = 0; 
	if( z == 2 ) z2 = 0; 
	if( z == 3 ) { z2 = 0; gone_into_record_stop = 1; }
	}

if( wdj->right_button )
	{
	if( z == 0 ) z2 = 2;
	if( z == 1 ) z2 = 2;
	if( z == 2 ) z2 = 3; gone_into_record = 1;
	if( z == 3 ) { z2 = 0; gone_into_record_stop = 1; }
	}

if( gone_into_play ) 
	{
	do_something( 0 );
	}

if( gone_into_record ) 
	{
	do_something( 1 );
	}

if( gone_into_record_stop ) 
	{
	do_something( 2 );
	}


wdj->ChangeCol( z2 );													//change led colour
}



*/





GCLed::GCLed( int xx, int yy, int ww, int hh, const char *label ) : Fl_Widget( xx, yy, ww, hh, label )
{
has_focus = 0;

iIndex=0;
idelta_r = 0;
idelta_g = 0;
idelta_b = 0;

led_style = cn_gcled_style_square;

p_mouse_button_changed_cb = 0;
p_mouse_event_cb = 0;

id = 0;
id2 = 0;
left_button = middle_button = right_button = 0;

for(int i=0;i<cnMaxColIndex;i++)
	{
	iR[i]=iG[i]=iB[i]=0;
	}
}





GCLed::~GCLed()
{
	
}
 
 
 
 

void GCLed::draw()
{
//Fl_Window::draw();


int rr = iR[ iIndex ] + idelta_r;
int gg = iG[ iIndex ] + idelta_g;
int bb = iB[ iIndex ] + idelta_b;

if( rr < 0 ) rr = 0;
if( rr > 255 ) rr = 255;

if( gg < 0 ) gg = 0;
if( gg > 255 ) gg = 255;

if( bb < 0 ) bb = 0;
if( bb > 255 ) bb = 255;




if( led_style == cn_gcled_style_square )
    {
    fl_line_style( FL_SOLID );
    fl_color( 0, 0, 0 );
    fl_rect( x() + 2, y() + 2, w() - 4, h() - 4 );                  //draw outline



	
    fl_rectf( x() + 3, y() + 3, w() - 6, h() - 6, rr, gg, bb );              //draw fill col

    fl_color( 0, 0, 0 );

    //draw focus
    if( !has_focus ) fl_color( parent()->color() );

    fl_line_style( FL_DOT );
    fl_rect( x(), y(), w(), h() );

    fl_line_style( FL_SOLID );
    }


if( led_style == cn_gcled_style_round )
    {
    fl_line_style( FL_SOLID );
    fl_color( 0, 0, 0 );
    fl_arc( x() + 2, y() + 2, w() - 4, h() - 4, 0, 360 );                   //draw outline

    fl_color( rr, gg, bb );
    fl_pie( x() + 3, y() + 3, w() - 6, h() - 6, 0, 360 );                   //draw fill col

    //draw focus
    fl_color( 0, 0, 0 );

    if( !has_focus ) fl_color( parent()->color() );

    fl_line_style( FL_DOT );
    fl_arc( x(), y(), w(), h(), 0, 360 );

    fl_line_style( FL_SOLID );
    }

fl_color( 0, 0, 0 );
}



int GCLed::handle( int e )
{

bool need_redraw = 0;
bool dont_pass_on = 0;

bool b_do_event_cb = 0;

last_event = e;

//added event's here to trigger 'p_mouse_event_cb()'					//v1.09
if ( ( e == FL_ENTER ) || ( e == FL_LEAVE ) || ( e == FL_FOCUS ) || ( e == FL_UNFOCUS ) || ( e == FL_PUSH ) || ( e == FL_RELEASE ) || ( e == FL_KEYDOWN ) || ( e == FL_KEYUP ) || ( e == FL_MOUSEWHEEL ) )
    {
	b_do_event_cb = 1;
	}


if( b_do_event_cb )
	{
	if( p_mouse_event_cb ) p_mouse_event_cb( cb_mouse_event_obj, cb_mouse_event_args );	//v1.09
	}


if ( e == FL_ENTER )		//v1.06, was 'e & FL_ENTER'
    {
    dont_pass_on = 1;		//need this for tooltip
    }


if ( e & FL_LEAVE )			//see v1.03 mods, this code does nothing now
    {
//    dont_pass_on = 1;		//need this for tooltip
    }


//return Fl_Widget::handle(e);

if ( e == FL_FOCUS )
    {
    has_focus = 1;		    //need this for focus rectangle to be drawn
    dont_pass_on = 1;
    need_redraw = 1;
    }


if ( e == FL_UNFOCUS )
    {
    has_focus = 0;		    //need this for focus rectangle to be drawn
    dont_pass_on = 1;
    need_redraw = 1;
    }




if ( e == FL_PUSH )
	{
	if( Fl::event_button() == 1 ) left_button = 1;
	if( Fl::event_button() == 2 ) middle_button = 1;
	if( Fl::event_button() == 3 ) right_button = 1;
 
    take_focus();
    has_focus = 1;

	if( p_mouse_button_changed_cb ) p_mouse_button_changed_cb( cb_mouse_button_changed_obj, cb_mouse_button_changed_args );
    do_callback();
    
    dont_pass_on = 1;
    need_redraw = 1;
	}


if ( e == FL_RELEASE )
	{
	if( Fl::event_button() == 1 ) left_button = 0;
	if( Fl::event_button() == 2 ) middle_button = 0;
	if( Fl::event_button() == 3 ) right_button = 0;
 
	if( p_mouse_button_changed_cb ) p_mouse_button_changed_cb( cb_mouse_button_changed_obj, cb_mouse_button_changed_args );
    dont_pass_on = 1;
    need_redraw = 1;
	}

if ( e == FL_KEYDOWN )
	{
    int key = Fl::event_key();
	
	if( key == ' ' ) 
		{
		do_callback();
		dont_pass_on = 1;												//v1.05
		}
    }




if( need_redraw )
	{
	redraw();
	}

	
if( dont_pass_on ) return 1;

return Fl_Widget::handle(e);
}




int GCLed::SetColIndex(int iIndex,int iRin,int iGin,int iBin)
{
if( iIndex >= cnMaxColIndex ) return 0;

iR[iIndex]=iRin;
iG[iIndex]=iGin;
iB[iIndex]=iBin;

return 1;	
}



//load colour indexes from a string like this: "255 0 0, 0 255 0, 0 0 255"
//note that each rgb colour set is seperated from next set by a comma, there MUST be a space between primary colour values within each set  
int GCLed::set_col_from_str( string rbg_list, char delimiter )
{
string s1;
mystr m1;
vector<string> vstr;
int colours[ 3 ];

m1 = rbg_list;

m1.LoadVectorStrings( vstr, delimiter );                //extract each colour set

for( int i = 0; i < vstr.size(); i++ )
    {
    if( i >= cnMaxColIndex ) break;                            //ensure we dont pass storage limit

    m1 = vstr[ i ];
//printf( "vstr[ i ]: %s\n", vstr[ i ].c_str() );

    int j = m1.LoadArrayInts( colours, 3 , ' ' );       //extract primary colour values
    if( j == 3 )
        {
        iR[ i ] = colours[ 0 ];                         //store colours
        iG[ i ] = colours[ 1 ];
        iB[ i ] = colours[ 2 ];
        }
    }


return 1;	
}






//load colour indexes from a string like this: "255 0 0, 0 255 0, 0 0 255"
//note that each rgb colour set is seperated from next set by a comma, there MUST be a space between primary colour values within each set  
void GCLed::set_index_col_rgb( unsigned int idx, uint8_t rr, uint8_t bb, uint8_t gg  )
{

if( idx >= cnMaxColIndex ) return;

if( rr < 0 ) rr = 0;
if( rr > 255 ) rr = 255;

if( gg < 0 ) gg = 0;
if( gg > 255 ) gg = 255;

if( bb < 0 ) bb = 0;
if( bb > 255 ) bb = 255;

 
iR[ idx ] = rr;
iG[ idx ] = gg;
iB[ idx ] = bb;

redraw();
}





//adj brightness by adding an offset to the current displayed 'SetColIndex()'
void GCLed::adj_brightness_offset( int idelta_r_in, int idelta_g_in, int idelta_b_in )
{

idelta_r = idelta_r_in;
idelta_g = idelta_g_in;
idelta_b = idelta_b_in;

redraw();
}








//adj brightness by adding an offset to the spec led colour index
void GCLed::adj_brightness_of_col_index( unsigned int idx, int idelta_r, int idelta_b, int idelta_g )
{
if( idx >= cnMaxColIndex ) return;


int rr = iR[ idx ] + idelta_r;
int gg = iG[ idx ] + idelta_g;
int bb = iB[ idx ] + idelta_b;

if( rr < 0 ) rr = 0;
if( rr > 255 ) rr = 255;

if( gg < 0 ) gg = 0;
if( gg > 255 ) gg = 255;

if( bb < 0 ) bb = 0;
if( bb > 255 ) bb = 255;

iR[ idx ] = rr;
iG[ idx ] = gg;
iB[ idx ] = bb;

redraw();
}







int GCLed::GetColIndex()
{
if(iIndex>=cnMaxColIndex) return -1;

return iIndex;
}




bool GCLed::ChangeCol( int iIn )
{
if(iIn>=cnMaxColIndex) return 0;

iIndex=iIn;
redraw();
return 1;
}





//e.g: use public var 'left_button' to determine state
void GCLed::set_mouse_button_changed_callback( void (*p_cb)( void*, void* ), void *obj_in, void *args_in )
{
p_mouse_button_changed_cb = p_cb;
cb_mouse_button_changed_obj = obj_in;
cb_mouse_button_changed_args = args_in;
}





//could be used to handle a mouse enter/leave event
void GCLed::set_mouse_event_callback( void (*p_cb)( void*, void* ), void *obj_in, void *args_in )
{
p_mouse_event_cb = p_cb;
cb_mouse_event_obj = obj_in;
cb_mouse_event_args = args_in;
}

