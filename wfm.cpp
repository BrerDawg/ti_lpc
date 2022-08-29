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

//wfm.cpp
//v1.01	 27-may-2018		//

#include "wfm.h"

extern void update_gphs();




mgraph *gph0;
vector<float> gph0_vx;
vector<float> gph0_vamp0;
vector<float> gph0_vamp1;
vector<float> gph0_vamp2;
vector<float> gph0_vamp3;
vector<float> gph0_vamp4;

double gph0_posx = 0;
double gph0_posy0 = 0;
double gph0_scalex = 1;
double gph0_scaley0 = 1;

double gph0_posy1 = 0;
double gph0_scaley1 = 1;

double gph0_posy2 = 0;
double gph0_scaley2 = 1;

int gph0_sel_trc = 0;
int gph0_sel_samp_idx = 0;
int gph0_left_hov_idx = 0;
int gph0_keya = 0;
int gph0_keyx = 0;
int gph0_keyy = 0;
int gph0_keyc = 0;
int gph0_ctrl_key = 0;
int gph0_shift_key = 0;

bool gph0_left_button = 0;
double gph0_mx = 0;
double gph0_my = 0;

int sel_trc = 0;
double gph0_selx;
double gph0_sely;
double gph0_sel_lastx;
double gph0_sel_lasty;

int gph0_drag_idx1;
int gph0_selbox_idx1 = -1;
int gph0_selbox_idx2 = -1;

extern float lpc_srate;


//-----------------------------------------------------------------------------


int leftx = 0;
int samp_left = 0;



void dbg_dump()
{
	int count;
if ( gph0->get_sample_count_for_trc( 0, count ) )
	{
	int hov_idx;
	if( gph0->get_hover_idx( 0, hov_idx ) )
		{
//			samp_left = hov_idx - hov_idx / gph0_scalex;
//			gph0_posx = -gph0->w()*samp_left/count + 0;
//gph0_posx *= gph0_scalex;
//leftx = gph0_posx;

//leftx = -448;//gph0_posx;
//printf( "dbg_dump: %d, samp_left: %d, leftx: %d\n", hov_idx, samp_left, gph0_posx, samp_left, leftx );
		}
	}

}









/*
void xxxxxxzoom_by_scale( int hov_samp, float scale_in )
{
printf("zoom_by_scale() - hov_samp: %d\n", hov_samp );

int wid, hei;
gph0->get_vis_dimensions( wid, hei );


//if( gph0->get_hover_idx( 0, hov_idx ) )
//	{
//	gph0->set_selected_sample( 0, hov_idx, 0 );
//printf( "cb_gph0_mousemove hover idx: %d\n", hov_idx );
//	}

int left_idx;
if( gph0->get_left_edge_sample_idx( 0, left_idx ) )
	{
printf("grrrr left_idx: %d\n", left_idx );
	}

int right_idx;
if( gph0->get_right_edge_sample_idx( 0, right_idx ) )
	{
printf("grrrr right_idx: %d (delta: %d )\n", right_idx , right_idx-left_idx);
	}

//if( !zoom_in ) scale_in = 1.0/scale_in;
//int hov_samp = 31407;

float new_total_sample_wnd = ( right_idx - left_idx ) / scale_in;

float samples_left_of_mouse = ( hov_samp-left_idx ) / scale_in;
printf("grrrr samp_left_of_mouse: %f\n", samples_left_of_mouse );


float new_left_sample = hov_samp-0 - samples_left_of_mouse;

printf("grrrr new_left_sample: %f\n", new_left_sample );


int posx = -(float)new_left_sample / new_total_sample_wnd * wid;
printf("grrrr new posx: %d\n", posx );

//if( !zoom_in ) scale_in = 1.0/scale_in;

gph0_scalex *= scale_in;
gph0_posx = posx;
update_gph0( gph0_vamp0, gph0_vamp1, gph0_vamp2 );

printf( "gph0_scalex: %f\n", gph0_scalex );

}
*/



/*
//EXAMPLE CODE - mousewheel changes zoom scale focussed the currently selected sample



void cb_gph0_mousewheel( void*, int int0 )
{

int sel_sample;
if( !gph0->get_selected_idx( 0, sel_sample ) ) return;							//get the selected sample where zoom will expand/contract from, ignore zoom if no sel sample

bool zoom_in = 0;
if( int0 > 0.0 ) { gph0_scale_change = 1.5; zoom_in = 1; }
else gph0_scale_change = 1.5;


if( zoom_in ) gph0->zoom_h( 0, sel_sample, gph0_scale_change, 1 );		//expand
else gph0->zoom_h( 0, sel_sample, 1.0 / gph0_scale_change,  1 );			//contract

}
*/






float gph0_scale_change = 1.0;													//global/static var

int dbgcnt = 0;

void cb_gph0_mousewheel( void*, int int0 )
{
printf("gph0_scaley0: %f\n", gph0_scaley0);

if( gph0_keyx )
	{

//	if( int0 > 0.0 ) gph0->adj_posx_by( gph0_sel_trc, 10, 1 );
//	if( int0 < 0.0 ) gph0->adj_posx_by( gph0_sel_trc, -10, 1 );
	if( int0 > 0.0 ) gph0->adj_posx_all_traces_by( 50, 1 );
	if( int0 < 0.0 ) gph0->adj_posx_all_traces_by( -50, 1 );
return;
	}



if( gph0_keyy )
	{
	if( gph0_sel_trc == 0 )
		{
		if( int0 > 0.0 ) gph0_posy0 += 12.5;
		if( int0 < 0.0 ) gph0_posy0 += -12.5;

//		gph0->set_plot_offsy( gph0_sel_trc, gph0_posy0, 1 );
		gph0->set_posy( gph0_sel_trc, gph0_posy0, 1 );
		}

	if( gph0_sel_trc == 1 )
		{
		if( int0 > 0.0 ) gph0_posy1 += 12.5;
		if( int0 < 0.0 ) gph0_posy1 += -12.5;

		gph0->set_posy( gph0_sel_trc, gph0_posy1, 1 );
		}
	
	if( gph0_sel_trc == 2 )
		{
		if( int0 > 0.0 ) gph0_posy2 += 12.5;
		if( int0 < 0.0 ) gph0_posy2 += -12.5;

		gph0->set_posy( gph0_sel_trc, gph0_posy2, 1 );
		}
return;
	}


if( gph0_keya )
	{

	if( int0 > 0.0 ) gph0->adj_scaley_by( gph0_sel_trc, 1.2, 1 );
	if( int0 < 0.0 ) gph0->adj_scaley_by( gph0_sel_trc, 1.0/1.2, 1 );
	
	if( gph0_sel_trc == 0 )
		{
		gph0->get_scaley( gph0_sel_trc, gph0_scaley0 );
		}

	if( gph0_sel_trc == 1 )
		{
		gph0->get_scaley( gph0_sel_trc, gph0_scaley1 );
		}

	if( gph0_sel_trc == 2 )
		{
		gph0->get_scaley( gph0_sel_trc, gph0_scaley2 );
		}
	
//	if( int0 > 0.0 ) gph0->adj_posx_all_traces_by( 10, 1 );
//	if( int0 < 0.0 ) gph0->adj_posx_all_traces_by( -10, 1 );
return;
	}


bool zoom_in = 0;
if( int0 > 0.0 ) { gph0_scale_change = 1.5; zoom_in = 1; }
else gph0_scale_change = 1.5;


if( zoom_in ) gph0->zoom_h_all_traces( gph0_left_hov_idx, gph0_scale_change, 1 );
else gph0->zoom_h_all_traces( gph0_left_hov_idx, 1.0/gph0_scale_change, 1 );
//update_gph0_user_obj();							//need to update user obj's as zoom changes do not alter obj's

printf("gph0_scale_change: %f\n", gph0_scale_change );
//int sel;
//gph0->get_selected_idx( 0, sel );
//gph0->set_selected_sample( 0, sel, 0 );
return;

//if( 1 )
//	{
	int count;
//ffff( 31407, gph0_scalex );
//return;
	if ( gph0->get_sample_count_for_trc( 0, count ) )
		{

//if( zoom_in ) zoom_by_scale( gph0_hov_idx, scale_change );
//else zoom_by_scale( gph0_hov_idx, 1.0/scale_change );
//gph0->set_selected_sample( 0, gph0_sel_samp_idx, 0 );

		}
return;

//			gph0->set_selected_sample( 0, hov_idx, 0 );

//		printf( "mousewheel hover idx: %d, samp_left: %d, leftx: %d\n", hov_idx, samp_left, gph0_posx, samp_left, leftx );
//dbg_dump();
//			samp_left = hov_idx - hov_idx / gph0_scalex;
//			gph0_posx = -gph0->w()*(samp_left/count) + 0;

//leftx = -241;


//		printf( "mousewheel hover idx: %d, samp_left: %d, leftx: %d\n", hov_idx, samp_left, gph0_posx, samp_left, leftx );
//			}
//		}
//	}

//if( gph0_scalex < 1.0 )
//	{
//gph0_scalex = 1.0;
//	gph0_posx = 0;
//leftx = 0;
//	}

//gph0_scalex = 2.0;
//gph0_posx = 0;
//update_gph0( gph0_vy );


//if( gph0_scalex > 1.0 )
//	{
	
//	tr1.posx = gph0_posx;
//	}

}







void cb_gph0_keyup( void*, int int0 )
{

if( int0 == 'a' ) gph0_keya = 0;
if( int0 == 'x' ) gph0_keyx = 0;
if( int0 == 'y' ) gph0_keyy = 0;
if( int0 == 'c' ) gph0_keyc = 0;

if( ( int0 == FL_Control_L ) | ( int0 == FL_Control_R ) )
	{
	gph0_ctrl_key = 0;

	}

if( ( int0 == FL_Shift_L ) | ( int0 == FL_Shift_R ) )
	{
	gph0_shift_key = 0;
	}


}




void cb_gph0_keydown( void*, int int0 )
{
if( int0 == 'a' ) gph0_keya = 1;
if( int0 == 'x' ) gph0_keyx = 1;
if( int0 == 'y' ) gph0_keyy = 1;
if( int0 == 'c' ) gph0_keyc = 1;

if( int0 == 'h' ) cb_help(0,0);


if( gph0_keyc )
	{
	gph0_posy0 = 0;
	gph0_posy1 = 0;
	gph0_posy2 = 0;
	
	gph0->set_plot_offsy( 0, 0, 1 );
	gph0->set_plot_offsy( 1, 0, 1 );
	gph0->set_plot_offsy( 2, 0, 1 );
	
	gph0->set_posy( 0, 0.0, 1 );
	gph0->set_posy( 1, 0.0, 1 );
	gph0->set_posy( 2, 0.0, 1 );
	
	
	update_gphs();
	}


if( int0 == FL_Escape )
	{
	gph0_selbox_idx1 = -1;
	gph0_selbox_idx2 = -1;
	}
//gph0->center_on_sample( 0, 5000 );

int hov_idx;

if( ( int0 == FL_Control_L ) | ( int0 == FL_Control_R ) )
	{
	gph0_ctrl_key = 1;
	}

if( ( int0 == FL_Shift_L ) | ( int0 == FL_Shift_R ) )
	{
	gph0_shift_key = 1;
	}

//gph0->set_left_edge_sample_idx( 0, 41876/2 );

//gph0->center_on_sample( 0, 10000 );

/*
int sel_idx;

int idx;
double xx, yy;

graph2d_wnd *o = wnd_grph2d;

if( o->gph->get_selected_value_using_id( 0, idx, xx, yy ) )
    {

    if( int0 == FL_Down ) 
		{
		idx -= 10;
        if( idx < 0 ) idx = 0;

        o->gph->set_selected_sample( 0, idx, -1 );
		}


	if( int0 == FL_Up )
        {
        int count;

        o->gph->get_sample_count( 0, count );

        idx += 10;
        if( idx >= count ) idx = count - 1;

        o->gph->set_selected_sample( 0, idx, -1 );
        }



    if( int0 == FL_Left )
        {
        idx--;
        if( idx < 0 ) idx = 0;

        o->gph->set_selected_sample( 0, idx, -1 );
        }


    if( int0 == FL_Right )
        {
        int count;
        
        o->gph->get_sample_count( 0, count );
        idx++;

        if( idx >= count ) idx = count - 1;

        o->gph->set_selected_sample( 0, idx, -1 );
        }

//    o->sel_sample = idx;
    }

o->selected_sample = idx;

o->gph->get_selected_value( 0, xx, yy );

string s1;

strpf( s1, "%f", xx );
wnd_grph2d->fi_valx->value( s1.c_str() );

strpf( s1, "%f", yy );
wnd_grph2d->fi_valy->value( s1.c_str() );
*/
}






void cb_gph0_left_click( void *args )
{
//int trc = (int)args;


//printf("trc: %d\n", trc );
int sel_sample;



int trc = gph0->get_selected_trace_and_sample_indexes( sel_sample );
if( trc != -1 )
	{
//printf(">>>>>>>trc: %d\n", trc);
	gph0_sel_lastx = gph0_selx;
	gph0_sel_lasty = gph0_sely;

	gph0->deselect_all_samples( 0 );

	gph0->set_selected_sample( trc, sel_sample, -1 );
	gph0->get_selected_value( trc, gph0_selx, gph0_sely );

	printf("trc: %d, sel_sample: %d, x: %g, y: %g\n", trc, sel_sample, gph0_selx, gph0_sely );

	gph0_sel_trc = trc;
	gph0_sel_samp_idx = sel_sample;
	}

//double xx,yy;

//if( gph0->get_selected_value( 0, gph0_selx, gph0_sely ) )
//	{
//	}
}





void cb_gph0_mouseenter( void* args )
{
//printf("enter\n");
Fl::focus( (Fl_Window*)args );

mgraph *o = (mgraph* )args;

o->border_col.r = 0;
o->border_col.g = 0;
o->border_col.b = 0;
}

void cb_gph0_mouseleave( void* args )
{
mgraph *o = (mgraph* )args;

o->border_col.r = 150;
o->border_col.g = 150;
o->border_col.b = 150;
//printf("leave\n");
}



int drag_start_mousex;
double drag_start_posx;
bool gph0_drag_started;

void cb_gph0_left_click_anywhere( void* )
{
gph0_left_button = 1;

int my;
gph0->get_mouse_pixel_position_on_background( drag_start_mousex, my );					//get values for dragging
//double drag_start_posx;
//gph0->get_left_edge_sample_idx( 0, drag_left_sample_idx );
gph0->get_posx( 0, drag_start_posx );
gph0_drag_started = 0;

if( gph0_shift_key )
	{
	int left, right;
	gph0->get_mousex_sample_boundary_idx( 0, left, right );		//get right sample for starters, see mouse move cb

	if( gph0_selbox_idx1 == -1 ) 
		{
		gph0_selbox_idx1 = left;
		gph0_selbox_idx2 = left + 1;
		}
	else{
		if( right <= gph0_selbox_idx1 ) gph0_selbox_idx2 = right - 1;
		else  gph0_selbox_idx2 = right;
		}

//	gph0_selbox_idx1 = gph0_drag_idx1 - 1;				//this was the right sample on left button down, as dragging left, now make left sample
//	gph0_selbox_idx2 = right;


//	gph0_selbox_idx1
//	double dd;
//	gph0->get_mouse_position_relative_to_trc( 0, gph0_sel_x1, dd );				//get value for selection box
	}
else{
	gph0_selbox_idx1 = -1;
	gph0_selbox_idx2 = -1;
	}


/*
int mouse_right_idx;
if( gph0->get_mousex_sample_boundary_idx( 0, drag_start_sample_idx, mouse_right_idx ) )
	{

//	gph0->set_selected_sample( 0, gph0_hov_idx, 0 );
printf( "cb_gph0_left_click_anywhere() -  drag_sample_idx: %d\n", drag_start_sample_idx );
	}
*/

//return;

//int idx, sel_sample;
//double xx;
//double yy;

//int trc_id = 0;

//graph2d_wnd *o = wnd_grph2d;

//if( o->gph->get_selected_value_using_id( trc_id, idx, xx, yy ) )                            //use trace's id to find if that trace has selected sample
 //   {
//printf( "leftclick[%d]: xx: %f, yy: %f\n", trc_id, xx, yy );
//	int sel_sample;
 //   if( gph0->get_selected_idx( 0, sel_sample ) )
//		{
//gph0->deselect_all_samples( 0 );
//		gph0_sel_samp_idx = sel_sample;
//		}
 //   }
//else{
//    sel_sample = -1;
//    }

/*
o->gph->set_selected_sample( 0, o->selected_sample, -1 );

o->gph->get_selected_value( 0, xx, yy );

string s1;

strpf( s1, "%f", xx );
wnd_grph2d->fi_valx->value( s1.c_str() );

strpf( s1, "%f", yy );
wnd_grph2d->fi_valy->value( s1.c_str() );
*/
}



void cb_gph0_left_click_release( void* )
{
gph0_left_button = 0;

}







void cb_gph0_right_click_anywhere( void* )
{

}










void cb_gph0_mousemove( void*, int int0, int int1, double dble0, double dble1 )
{

int left, right;
if( gph0->get_mousex_sample_boundary_idx( 0, left, right ) )
	{
	gph0_left_hov_idx = left;												//get sample the mouse is nearest to, used when zooming (see mousewheel cb)
//	printf("left: %d, %d\n", left, right);
	}


//int hov_trc = gph0->get_hover_trace_idx();

//if( hov_trc != -1 )
//	{
//	printf("------------hov_trc %d\n", hov_trc);
//	}

//int mouse_left_idx;
//int mouse_right_idx;


if( gph0_left_button )												//dragging x-axis?
	{
	int mx, my;
	gph0->get_mouse_pixel_position_on_background( mx, my );
	int mouse_delta = mx - drag_start_mousex;


	if( fabs(mouse_delta) > 5) gph0_drag_started = 1;								//allow some dead band

	if(gph0_drag_started)
		{
		gph0_drag_started = 1;
		int wid, hei;
		gph0->get_trace_vis_dimensions( 0, wid, hei );

//		int left, right;
//		gph0->get_left_edge_sample_idx( 0, left );
//		gph0->get_right_edge_sample_idx( 0, right );

//		int samp_vis = right - left;
//		float samp_pixel_ratio = (float)samp_vis / wid;						//how many samples visible

//		int moveby_samp =  samp_pixel_ratio * mouse_delta;					//how many samples the mouse dragging has transversed

		int count;

		if( !gph0->get_sample_count_for_trc( 0, count ) );

		//if( count == 0 ) return 0;
		double scalex;
		gph0->get_scalex(0, scalex );

		float samples_vis = (count - 1) / scalex;
		float samp_pixel_ratio = (float)samples_vis / wid;						//how many samples visible

		float slip = (float)mouse_delta / wid;
		float slip2 = samples_vis * slip;

		float posx_shift = (float)slip2 / count * wid * scalex;

		gph0->set_posx( 0, drag_start_posx + posx_shift, 1 );					//drag traces
		gph0->set_posx( 1, drag_start_posx + posx_shift, 1 );
		gph0->set_posx( 2, drag_start_posx + posx_shift, 1 );
		}
	}

/*
if( gph0_left_button & gph0_shift_key )											//dragging selection?
	{
	int left, right;
	gph0->get_mousex_sample_boundary_idx( 0, left, right );

	bool going_left = 0;
	if( left < gph0_drag_idx1 ) going_left = 1;

	if( going_left )
		{
		gph0_selbox_idx1 = gph0_drag_idx1 - 1;				//this was the right sample on left button down, as dragging left, now make left sample
		gph0_selbox_idx2 = right;
		}
	else{
		gph0_selbox_idx1 = left;
		gph0_selbox_idx2 = gph0_drag_idx1;					//this was the right sample on left button down, leave it as is
		}
//	printf("going_left: %d\n", going_left);
	}
*/


if (gph0->get_mouse_position_relative_to_trc( 0, gph0_mx, gph0_my ) )		//get values for for graph text display
	{
	}

update_gph0_user_obj();
gph0->redraw();

return;
}








void load_settings_gph0( string ini )
{
string s1;
mystr m1;

GCProfile p( ini );
/*
trace_tag tr1;
pnt_tag pnt1;

pnt1.x = 0;
pnt1.y = 0;

tr1.pnt.push_back( pnt1 );
gph0->add_trace( tr1 );
gph0->add_trace( tr1 );
gph0->add_trace( tr1 );
*/

double dd;
dd=p.GetPrivateProfileDOUBLE( "Settings", "gph0_posx", 0 );
gph0_posx = dd;

dd=p.GetPrivateProfileDOUBLE( "Settings", "gph0_posy0", 0 );
gph0_posy0 = dd;

dd=p.GetPrivateProfileDOUBLE( "Settings", "gph0_scalex", 1.0 );
gph0_scalex = dd;

dd=p.GetPrivateProfileDOUBLE( "Settings", "gph0_scaley0", 1.0);
gph0_scaley0 = dd;


dd=p.GetPrivateProfileDOUBLE( "Settings", "gph0_posy1", 0 );
gph0_posy1 = dd;

dd=p.GetPrivateProfileDOUBLE( "Settings", "gph0_scaley1", 1.0);
gph0_scaley1 = dd;


dd=p.GetPrivateProfileDOUBLE( "Settings", "gph0_posy2", 0 );
gph0_posy2 = dd;

dd=p.GetPrivateProfileDOUBLE( "Settings", "gph0_scaley2", 1.0);
gph0_scaley2 = dd;

dd=p.GetPrivateProfileLONG( "Settings", "gph0_selbox_idx1", -1);
gph0_selbox_idx1 = dd;

dd=p.GetPrivateProfileLONG( "Settings", "gph0_selbox_idx2", -1);
gph0_selbox_idx2 = dd;

}





void save_settings_gph0( string ini )
{
string s1;
mystr m1;

GCProfile p( ini );

double dd;
gph0->get_posx( 0, dd );
p.WritePrivateProfileDOUBLE("Settings","gph0_posx", dd );

gph0->get_scalex( 0, dd );
p.WritePrivateProfileDOUBLE("Settings","gph0_scalex", dd );

gph0->get_posy( 0, dd );
p.WritePrivateProfileDOUBLE("Settings","gph0_posy0", dd );

gph0->get_scaley( 0, dd );
p.WritePrivateProfileDOUBLE("Settings","gph0_scaley0", dd );


gph0->get_posy( 1, dd );
p.WritePrivateProfileDOUBLE("Settings","gph0_posy1", dd );

gph0->get_scaley( 1, dd );
p.WritePrivateProfileDOUBLE("Settings","gph0_scaley1", dd );

gph0->get_posy( 2, dd );
p.WritePrivateProfileDOUBLE("Settings","gph0_posy2", dd );

gph0->get_scaley( 2, dd );
p.WritePrivateProfileDOUBLE("Settings","gph0_scaley2", dd );

p.WritePrivateProfileLONG( "Settings", "gph0_selbox_idx1", gph0_selbox_idx1);
p.WritePrivateProfileLONG( "Settings", "gph0_selbox_idx2", gph0_selbox_idx2);

}







void update_gph0_user_obj()
{
string s1;

int offx, offy;
int wid, hei;
gph0->get_background_offsxy( offx, offy );
gph0->get_background_dimensions( wid, hei );




st_mgraph_draw_obj_tag mdo;


mdo.type = (en_mgraph_draw_obj_type)en_dobt_text;


mdo.visible = 1;
mdo.draw_ordering = 2;				//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

mdo.use_pos_x = -1;                 //if not set to -1: then use this trace id to get a specific x position
mdo.use_scale_x = -1;               //if not set to -1: then use this trace id to get a specific x scale

mdo.use_pos_y = -1;                 //if not set to -1: then use this trace id to get a specific y position
mdo.use_scale_y = -1;               //if not set to -1: then use this trace id to get a specific y scale

mdo.clip_left = 0;					//samke as background
mdo.clip_right = 0;
mdo.clip_top = 0;
mdo.clip_bottom = 0;


mdo.x1 = 4;
mdo.y1 = 10;
mdo.x2;
mdo.y2;
//mdo.wid = 10;
//mdo.hei = 10;

//mouse pos
strpf( s1, "press h, x:%d y:%.2f", (int)gph0_mx, gph0_my );
mdo.arc1 = 0;
mdo.arc2 = 0;
mdo.stext = s1;
mdo.r = 229;
mdo.g = 142;
mdo.b = 0;
mdo.justify = en_tj_none;
mdo.font = 4;
mdo.font_size = 11;
mdo.line_style = (en_mgraph_line_style)FL_SOLID;
mdo.line_thick = 1;

gph0->vdrwobj.clear();
gph0->vdrwobj.push_back( mdo );



//sel sample
strpf( s1, "x:%d y:%.2f", (int)gph0_selx, gph0_sely );
mdo.x1 = 240;
mdo.y1 = 10;
mdo.arc1 = 0;
mdo.arc2 = 0;
mdo.stext = s1;

mdo.r = 255;
mdo.g = 255;
mdo.b = 0;

if( gph0_sel_trc == 1 )
	{
	mdo.r = 0;
	mdo.g = 255;
	mdo.b = 0;
	}

if( gph0_sel_trc == 2 )
	{
	mdo.r = 0;
	mdo.g = 255;
	mdo.b = 255;
	}

gph0->vdrwobj.push_back( mdo );



//deltas
double dt = ( gph0_selx - gph0_sel_lastx ) / lpc_srate;
double freq = fabs( 1.0 / dt );

mystr m1;
string snum, sunits, scombined, sappend_num, sappend_units;
sappend_units = "S";
sappend_num = "";
m1.make_engineering_str( snum, sunits, scombined, 3, dt, sappend_num, sappend_units );

strpf( s1, "dx:%d  dy:%.2f  dt:%s  freq:%.2f Hz", (int)(gph0_selx - gph0_sel_lastx), gph0_sely - gph0_sel_lasty, scombined.c_str(), freq );
mdo.x1 = 420;
mdo.y1 = 10;
mdo.arc1 = 0;
mdo.arc2 = 0;
mdo.stext = s1;

mdo.r = 229;
mdo.g = 142;
mdo.b = 0;

gph0->vdrwobj.push_back( mdo );


//selection rect
mdo.type  = en_dobt_rectf;
mdo.draw_ordering = 0;										//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

int ix, iy;


mdo.use_pos_x = 0;                 //if not set to -1: then use this trace id to get a specific x position
mdo.use_scale_x = 0;               //if not set to -1: then use this trace id to get a specific x scale

mdo.x1 = gph0_selbox_idx1;
mdo.y1 = 0;

mdo.x2 = gph0_selbox_idx2;
mdo.y2 = hei;

mdo.r = 100;
mdo.g = 100;
mdo.b = 64;

if( gph0_selbox_idx1 != -1 ) gph0->vdrwobj.push_back( mdo );		//only show if valid



/*
//mdo.use_pos_x = -1;                 //if not set to -1: then use this trace id to get a specific x position
//mdo.use_scale_x = -1;               //if not set to -1: then use this trace id to get a specific x scale

mdo.vpolyx.clear();
mdo.vpolyx.clear();


mdo.type = en_dobt_polygonf;
mdo.draw_ordering = 1;										//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

mdo.vpolyx.push_back( 250 );
mdo.vpolyy.push_back( -50 );

mdo.vpolyx.push_back( 500 );
mdo.vpolyy.push_back( -25 );

mdo.vpolyx.push_back( 750 );
mdo.vpolyy.push_back( 375 );

mdo.r = 100;
mdo.g = 80;
mdo.b = 64;

gph0->vdrwobj.push_back( mdo );



mdo.vpolyx.clear();
mdo.vpolyy.clear();


mdo.type = en_dobt_polypoints;
mdo.draw_ordering = 1;										//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

mdo.vpolyx.push_back( 1250 );
mdo.vpolyy.push_back( 25 );

mdo.vpolyx.push_back( 1400 );
mdo.vpolyy.push_back( 25 );

mdo.vpolyx.push_back( 1650 );
mdo.vpolyy.push_back( 200 );

mdo.r = 100;
mdo.g = 255;
mdo.b = 64;

gph0->vdrwobj.push_back( mdo );


mdo.vpolyy.clear();


mdo.use_pos_x = 0;                 //if not set to -1: then use this trace id to get a specific x position
mdo.use_scale_x = 0;               //if not set to -1: then use this trace id to get a specific x scale

mdo.use_pos_y = -1;                 //if not set to -1: then use this trace id to get a specific y position
mdo.use_scale_y = -1;               //if not set to -1: then use this trace id to get a specific y scale

mdo.type = en_dobt_pie;
mdo.draw_ordering = 1;										//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces


mdo.x1 = 1000;
mdo.y1 = 0;
mdo.x2 = 5000;
mdo.y2 = 100;
mdo.arc1 = 0;
mdo.arc1 = 360;

gph0->vdrwobj.push_back( mdo );
*/
}









void update_gph0( vector<float> &v0, vector<float> &v1, vector<float> &v2 )
{
double dd;

//v0.clear();
//v1.clear();
//v2.clear();


mg_col_tag col;
trace_tag tr1;

//gph0_posx = ;
//gph0_posy0;
//gph0_scaley0;
//gph0_scaley0;
//tr1.posx = gph0_posx;

gph0->get_scalex( 0, gph0_scalex );						//get current state, before refreshing traces
gph0->get_posx( 0, gph0_posx );
//gph0->get_scaley( 0, gph0_scaley0 );
//gph0->get_scaley( 1, gph0_scaley1 );
//gph0->get_scaley( 2, gph0_scaley2 );


update_gph0_user_obj();


gph0->clear_traces();

//------------------

col.r = 255;
col.g = 255;
col.b = 0;

//led_trace_col_wfm1->SetColIndex( 0, col.r, col.g, col.b );
//led_trace_col_wfm1->ChangeCol( 0 );


printf("gph0_scaley0: %f\n", gph0_scaley0);

tr1.id = 0;                      //identify which trace this is, helps when traces are push_back'd in unknown order
tr1.vis = 1;
tr1.col = col;
tr1.line_thick = 1;
tr1.lineplot = 1;
tr1.line_style = (en_mgraph_line_style) en_mls_solid;
tr1.border_left = 0;
tr1.border_right = 0;
tr1.border_top = 0;
tr1.border_bottom = 0;

tr1.show_as_spectrum = 0;                           //not a spectra plot
tr1.spectrum_baseline_y = 0;

tr1.b_limit_auto_scale_min_for_y = 0;
tr1.b_limit_auto_scale_max_for_y = 0;

int graticle_count_y = gph0->graticle_count_y;					//this should be an even number
int graticle_count_x = gph0->graticle_count_x;					//this should be an even number
int grat_pixels_x = gph0->grat_pixels_x;						//pxls per graticule
int grat_pixels_y = gph0->grat_pixels_y;

tr1.xunits_perpxl = -1;//1.0/grat_pixels_x;
tr1.yunits_perpxl = 0.01;//0.1 / grat_pixels_y;


tr1.posx = gph0_posx;
tr1.posy = gph0_posy0;

tr1.plot_offsx = 0; 								//not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
tr1.plot_offsy = 0;

tr1.use_pos_y = -1;				//if not -1, use this trace's id as a reference for this val
tr1.use_scale_y = -1;			//if not -1, use this trace's id as a reference for this val


tr1.scalex = gph0_scalex;
tr1.scaley = gph0_scaley0;

tr1.single_sel_col.r = 255;
tr1.single_sel_col.g = 0;
tr1.single_sel_col.b = 255;

tr1.group_sel_trace_col.r = 230;
tr1.group_sel_trace_col.g = 230;
tr1.group_sel_trace_col.b = 230;

tr1.group_sel_rect_col.r = 255;
tr1.group_sel_rect_col.g = 153;
tr1.group_sel_rect_col.b = 0;

tr1.sample_rect_hints_double_click = 1;

tr1.sample_rect_hints = 1;
tr1.sample_rect_hints_col.r = 150;
tr1.sample_rect_hints_col.g = 120;
tr1.sample_rect_hints_col.b = 150;
tr1.sample_rect_hints_distancex = 12;						//helps stop over hinting on x-axis
tr1.sample_rect_hints_distancey = 0;						//disable over hinting test/clearing for y-axis 

tr1.clip_left = tr1.border_left;
tr1.clip_right = tr1.border_right;
tr1.clip_top = tr1.border_top;
tr1.clip_bottom = tr1.border_bottom;


pnt_tag pnt1;

double minx, miny, maxx, maxy;

minx = miny = maxx = maxy = 0;

//tr1.posx = graticle_count_x / 2.0 * grat_pixels_x;			//offset to centre plot
for( int i = 0; i < v0.size(); i++ )
	{
//printf("y: %f\n", vy[ i ] );
	pnt1.x = i;
	pnt1.y = v0[ i ];
	pnt1.sel = 0;
if( ( i >= 5000 ) & ( i <= 20000) ) pnt1.sel = 1;

	tr1.pnt.push_back( pnt1 );

	if( i == 0 )
		{
		minx = maxx = pnt1.x;
		miny = maxy = pnt1.y;
		}

	if( pnt1.x < minx ) minx = pnt1.x;
	if( pnt1.y < miny ) miny = pnt1.y;

	if( pnt1.x > maxx ) maxx = pnt1.x;
	if( pnt1.y > maxy ) maxy = pnt1.y;
	}


gph0->add_trace( tr1 );
//int px = graticle_count_x * grat_pixels_x;

//double midx = ( maxx - minx ) / 2.0;
//double midy = ( maxy - miny ) / 2.0;

//if( ( minx < 0 ) & ( maxx > 0 ) ) midx = ( maxx + minx ) / 2.0;
//printf("midx: %f\n", midx );
//printf("minx: %f, maxx: %f\n", minx, maxx );

//printf("midy: %f\n", midy );
//printf("miny: %f, maxy: %f\n", miny, maxy );

//tr1.posx = 0;//-midx;
//tr1.posy = midy;

//tr1.vgroup_sel.clear();

//for(int i = 0; i < 5000; i++ )
//	{
//	tr1.vgroup_sel.push_back( i );
//	}

/*
gph0->clear_traces();

tr1.pnt.clear();
for( int i = 0; i < 10; i++ )
	{
pnt1.x = i;
pnt1.y = -0.1;
pnt1.sel = 0;
if( i == 3 ) {pnt1.sel = 1; pnt1.y = 0.1;}
tr1.pnt.push_back( pnt1 );
	}


gph0->add_trace( tr1 );
goto skip;
*/
//------------------










//------------------
col.r = 0;
col.g = 255;
col.b = 0;

//led_trace_col_wfm1->SetColIndex( 0, col.r, col.g, col.b );
//led_trace_col_wfm1->ChangeCol( 0 );



tr1.id = 1;                      //identify which trace this is, helps when traces are push_back'd in unknown order
tr1.vis = 1;
tr1.col = col;
tr1.line_thick = 1;
tr1.lineplot = 1;
tr1.line_style = (en_mgraph_line_style) en_mls_solid;
tr1.border_left = 0;
tr1.border_right = 0;
tr1.border_top = 0;
tr1.border_bottom = 0;

tr1.show_as_spectrum = 0;                           //not a spectra plot
tr1.spectrum_baseline_y = 0;

tr1.b_limit_auto_scale_min_for_y = 0;
tr1.b_limit_auto_scale_max_for_y = 0;

tr1.xunits_perpxl = -1;//1.0/grat_pixels_x;
tr1.yunits_perpxl = 0.001;//1.0/grat_pixels_y;

tr1.posx = gph0_posx;
tr1.posy = gph0_posy1;

tr1.plot_offsx = 0;                 				//not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
tr1.plot_offsy = 0;

tr1.use_pos_y = -1;				//if not -1, use this trace's id as a reference for this val
tr1.use_scale_y = -1;			//if not -1, use this trace's id as a reference for this val

tr1.scalex = gph0_scalex;
tr1.scaley = gph0_scaley1;

tr1.single_sel_col.r = 255;
tr1.single_sel_col.g = 0;
tr1.single_sel_col.b = 255;

tr1.sample_rect_hints_double_click = 1;

tr1.sample_rect_hints = 1;
tr1.sample_rect_hints_col.r = 150;
tr1.sample_rect_hints_col.g = 120;
tr1.sample_rect_hints_col.b = 150;
tr1.sample_rect_hints_distancex = 12;						//helps stop over hinting on x-axis
tr1.sample_rect_hints_distancey = 0;						//disable over hinting test for y-axis 

tr1.clip_left = tr1.border_left;
tr1.clip_right = tr1.border_right;
tr1.clip_top = tr1.border_top;
tr1.clip_bottom = tr1.border_bottom;

tr1.pnt.clear();


minx = miny = maxx = maxy = 0;

//tr1.posx = graticle_count_x / 2.0 * grat_pixels_x;			//offset to centre plot
for( int i = 0; i < v1.size(); i++ )
	{
//printf("y: %f\n", vy[ i ] );
	pnt1.x = i;
	pnt1.y = v1[ i ];
	pnt1.sel = 0;
	tr1.pnt.push_back( pnt1 );

	if( i == 0 )
		{
		minx = maxx = pnt1.x;
		miny = maxy = pnt1.y;
		}

	if( pnt1.x < minx ) minx = pnt1.x;
	if( pnt1.y < miny ) miny = pnt1.y;

	if( pnt1.x > maxx ) maxx = pnt1.x;
	if( pnt1.y > maxy ) maxy = pnt1.y;
	}

//px = graticle_count_x * grat_pixels_x;

//midx = ( maxx - minx ) / 2.0;
//midy = ( maxy - miny ) / 2.0;

//if( ( minx < 0 ) & ( maxx > 0 ) ) midx = ( maxx + minx ) / 2.0;
//printf("midx: %f\n", midx );
//printf("minx: %f, maxx: %f\n", minx, maxx );

//printf("midy: %f\n", midy );
//printf("miny: %f, maxy: %f\n", miny, maxy );

//tr1.posx = 0;//-midx;
//tr1.posy = midy;


gph0->add_trace( tr1 );
//----------------








//------------------
col.r = 0;
col.g = 255;
col.b = 255;

//led_trace_col_wfm1->SetColIndex( 0, col.r, col.g, col.b );
//led_trace_col_wfm1->ChangeCol( 0 );



tr1.id = 2;                      //identify which trace this is, helps when traces are push_back'd in unknown order
tr1.vis = 1;
tr1.col = col;
tr1.line_thick = 1;
tr1.lineplot = 1;
tr1.line_style = (en_mgraph_line_style) en_mls_solid;
tr1.border_left = 0;
tr1.border_right = 0;
tr1.border_top = 0;
tr1.border_bottom = 0;

tr1.show_as_spectrum = 0;                           //not a spectra plot
tr1.spectrum_baseline_y = 0;

tr1.b_limit_auto_scale_min_for_y = 0;
tr1.b_limit_auto_scale_max_for_y = 0;

tr1.xunits_perpxl = -1;//1.0/grat_pixels_x;
tr1.yunits_perpxl = 0.02;//0.005;//1.0/grat_pixels_y;

tr1.posx = gph0_posx;
tr1.posy = gph0_posy2;

tr1.plot_offsx = 0;                 				//not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
tr1.plot_offsy = 0;

tr1.use_pos_y = -1;				//if not -1, use this trace's id as a reference for this val
tr1.use_scale_y = -1;			//if not -1, use this trace's id as a reference for this val

tr1.scalex = gph0_scalex;
tr1.scaley = gph0_scaley2;

tr1.single_sel_col.r = 255;
tr1.single_sel_col.g = 0;
tr1.single_sel_col.b = 255;


tr1.group_sel_trace_col.r = 80;
tr1.group_sel_trace_col.g = 200;
tr1.group_sel_trace_col.b = 255;


tr1.group_sel_rect_col.r = 255;
tr1.group_sel_rect_col.g = 153;
tr1.group_sel_rect_col.b = 0;


tr1.sample_rect_hints_double_click = 0;

tr1.sample_rect_hints = 1;
tr1.sample_rect_hints_col.r = 150;
tr1.sample_rect_hints_col.g = 120;
tr1.sample_rect_hints_col.b = 150;
tr1.sample_rect_hints_distancex = 12;						//helps stop over hinting on x-axis
tr1.sample_rect_hints_distancey = 0;						//disable over hinting test for y-axis 

tr1.clip_left = tr1.border_left;
tr1.clip_right = tr1.border_right;
tr1.clip_top = tr1.border_top;
tr1.clip_bottom = tr1.border_bottom;

tr1.pnt.clear();


minx = miny = maxx = maxy = 0;

//tr1.posx = graticle_count_x / 2.0 * grat_pixels_x;			//offset to centre plot
for( int i = 0; i < v2.size(); i++ )
	{
//printf("y: %f\n", vy[ i ] );
	pnt1.x = i;
	pnt1.y = v2[ i ];
	pnt1.sel = 0;
	tr1.pnt.push_back( pnt1 );

	if( i == 0 )
		{
		minx = maxx = pnt1.x;
		miny = maxy = pnt1.y;
		}

	if( pnt1.x < minx ) minx = pnt1.x;
	if( pnt1.y < miny ) miny = pnt1.y;

	if( pnt1.x > maxx ) maxx = pnt1.x;
	if( pnt1.y > maxy ) maxy = pnt1.y;
	}

skip:
//px = graticle_count_x * grat_pixels_x;

//midx = ( maxx - minx ) / 2.0;
//midy = ( maxy - miny ) / 2.0;

//if( ( minx < 0 ) & ( maxx > 0 ) ) midx = ( maxx + minx ) / 2.0;
//printf("midx: %f\n", midx );
//printf("minx: %f, maxx: %f\n", minx, maxx );

//printf("midy: %f\n", midy );
//printf("miny: %f, maxy: %f\n", miny, maxy );

//tr1.posx = 0;//-midx;
//tr1.posy = midy;


gph0->add_trace( tr1 );

//----------------









//non trace specific cb
gph0->set_left_click_anywhere_cb( cb_gph0_left_click_anywhere, 0 );
gph0->set_right_click_anywhere_cb( cb_gph0_right_click_anywhere, 0 );
gph0->set_left_click_release_cb( cb_gph0_left_click_release, (void*)0 );
gph0->set_mouseenter_cb( cb_gph0_mouseenter, (void*)gph0 );
gph0->set_mouseleave_cb( cb_gph0_mouseleave, (void*)gph0 );


//non specific cb
gph0->set_left_click_cb( 0, cb_gph0_left_click, (void*)0 );
gph0->set_left_click_cb( 1, cb_gph0_left_click, (void*)1 );
gph0->set_left_click_cb( 2, cb_gph0_left_click, (void*)2 );

//gph->set_right_click_cb( trace_idx,  cb_spect_graph_right_click, this );
gph0->set_keydown_cb( 0, cb_gph0_keydown, 0 );
gph0->set_keyup_cb( 0, cb_gph0_keyup, 0 );
gph0->set_mousewheel_cb( 0, cb_gph0_mousewheel, 0 );
gph0->set_mousemove_cb( 0, cb_gph0_mousemove, (void*) 0 );
//gph0->set_mousemove_cb( 1, cb_gph0_mousemove, (void*)1 );
//gph0->set_mousemove_cb( 2, cb_gph0_mousemove, (void*)2 );


//gph->set_selected_sample( trace_idx, sel_sample, 0 );              //show selected sample if any


col.r = 64;
col.g = 64;
col.b = 64;
gph0->background = col;

int selected_sample = gph0_sel_samp_idx;
if( selected_sample < 0 ) selected_sample = 0;
gph0->set_selected_sample( 0, selected_sample, 0 );					//trigger: left_click_anywhere_cb_p_callback

//gph0->redraw();

gph0->render( -1 );
}






void add_graphs( Fl_Window* o )
{
o->begin();

int menu_hei = 0;

int xx = fluid_gph1->x();
int yy = fluid_gph1->y();
int wid = fluid_gph1->w();
int hei = fluid_gph1->h();

fluid_gph1->hide();
//printf("%d, %d,%d,%d,\n", xx, yy, wid, hei );
//exit(0);

int grat_pixels_x = 25;						//pxls per graticule
int grat_pixels_y = 25;

int border = 10;

int gratcnt_x = (wid - 2 * border) / grat_pixels_x;
int gratcnt_y = (hei - 2 * border) / grat_pixels_y;

if(gratcnt_x&1) gratcnt_x--;				//make even
if(gratcnt_y&1) gratcnt_y--;				//make even

int graticle_count_y = gratcnt_y;			//this should be an even number
int graticle_count_x = gratcnt_x;			//this should be an even number


wid = graticle_count_x * grat_pixels_x; //calc size of wnd using graticule details
wid += 2 * border;

hei = graticle_count_y * grat_pixels_y;
hei += 2 * border;


gph0 = new mgraph( xx, menu_hei + yy, wid, hei );

gph0->htime_lissajous = 0;				//0 means no lissajous

gph0->bkgd_border_left = border;
gph0->bkgd_border_top = border;
gph0->bkgd_border_right = border;
gph0->bkgd_border_bottom = border;

gph0->graticule_border_left = 0;
gph0->graticule_border_top = 0;
gph0->graticule_border_right = 0;
gph0->graticule_border_bottom = 0;

gph0->graticle_count_x = graticle_count_x;
gph0->grat_pixels_x = grat_pixels_x;

gph0->graticle_count_y = graticle_count_y;
gph0->grat_pixels_y = grat_pixels_y;
gph0->cro_graticle = 1;

gph0->show_cursors_along_y = 0;
gph0->cursor_along_y0 = 10;         		//cursors along y-axis (i.e. measuring time)
gph0->cursor_along_y1 = 20;

gph0->show_cursors_along_x = 0;
gph0->cursor_along_x0 = 1;        	 		//cursors along x-axis (i.e. measuring amplitude)
gph0->cursor_along_x1 = -2;

o->end();
}


//-----------------------------------------------------------------------------


