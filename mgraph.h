/*
Copyright (C) 2022 BrerDawg

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
hint
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


//mgraph.h
//v1.08 mgraph	 	21-dec-2017
//v1.05 surface3d 	06-jan-2018			//see also below  'to avoid windows error dlg: fl_line_style(): Could not create GDI pen object'
//v1.17 mgraph and fast_graph
//v1.18 mgraph
//v1.19 fast_graph

#ifndef mgraph_h
#define mgraph_h




#define _FILE_OFFSET_BITS 64			//large file handling, must be before all #include...
//#define _LARGE_FILES

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <locale.h>
#include <string>
#include <vector>
#include <wchar.h>
#include <algorithm>
#include <math.h>
#include <complex>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_File_Input.H>
#include <FL/Fl_Input_Choice.H>

//#include "globals.h"
//#include "pref.h"
#include "GCProfile.h"
//#include "GCLed.h"
//#include "gclog.h"
//#include "gcthrd.h"
//#include "gcpipe.h"
//#include "matheval.h"
//#include "wrksheet.h"

//linux code
#ifndef compile_for_windows

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
//#include <X11/Xaw/Form.h>									//64bit
//#include <X11/Xaw/Command.h>								//64bit

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


using namespace std;



#define cn_surfplot_deg2rad M_PI / 180.0
#define cn_surfplot_rad2deg ( 180.0 / M_PI )


struct mg_colref
{
int r, g, b;
};



struct mg_col_tag
{
int r;
int g;
int b;
};



struct clip_coord_tag
{
int x, y;
};


//-----------------------
//surfaceplot

//v1.03     	31-mar-2015         //adding surfaceplot_line_obj_tag
//v1.04         02-nov-2016

enum en_surfaceplot_line_style
{
en_spls_solid = FL_SOLID,
en_spls_dash = FL_DASH,
en_spls_dot = FL_DOT,
en_spls_dashdot = FL_DASHDOT,
en_spls_dashdotdot = FL_DASHDOTDOT,
en_spls_cap_flat = FL_CAP_FLAT,
en_spls_cap_round = FL_CAP_ROUND,
en_spls_cap_square = FL_CAP_SQUARE,
en_spls_cap_join_miter = FL_JOIN_MITER,
en_spls_cap_join_round = FL_JOIN_ROUND,
en_spls_cap_join_bevel = FL_JOIN_BEVEL,

};




//must be powers of 2, as anding used to test if active
enum en_plot_options_tag
{
en_po_none = 0,
en_po_dont_plot = 1,
en_po_show_centre_dot = 2,
en_po_show_normals = 4,
en_po_show_select_1 = 8,
en_po_show_select_2 = 16,
en_po_show_select_3 = 32,

};








enum en_which_corner_tag
{
en_cw_south_west,                   //x1,y1,z1
en_cw_south_east,                   //x2,y2,z2
en_cw_north_east,                   //x3,y3,z3
en_cw_north_west,                   //x4,y4,z4

};





















//holds one user line obj to plot
struct surfaceplot_line_obj_tag
{
int x1, y1, x2, y2;							//mathematical line representation, infinitely thin
double line_wid;
en_surfaceplot_line_style line_style;

int r, g, b;
//double gradient;
//double yintercept;

};








struct st_surfaceplot_val_tag		//hold a entry in the surface grid, where x,y are also specified, used when text objs are displayed
{
double x;
double y;
double val;							//height (on z-axis)
};




//holds one user text obj to plot
struct surfaceplot_text_obj_tag
{
double x, y;						//location of text relative to x, y val passed in: build_surface2(..)
double val;							//height (on z-axis)
string stext;
bool show_text_background;						//set this to give text an outline or shadow look

mg_col_tag foregnd;								//colour of text on top
mg_col_tag backgnd;								//colour of below text (outline or shadow look)
int fontsize;
int font;

};






// !!!!!! to avoid Windows ERROR dlg: fl_line_style(): Could not create GDI pen object !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// !!!!!! MAKE SURE YOU SET legal values for all below:

// !!!!!! grid_line_wid,   grid_line_style,   line_wid,   line_wid,
// !!!!!! FOR     st_surfplot_tag    surfaceplot_line_obj_tag    objects



struct st_surfplot_tag
{
double x1, y1, z1;          //holds translated, rotated 3d coords
double x2, y2, z2;
double x3, y3, z3;
double x4, y4, z4;


double nx, ny, nz;          //holds normals

double centrex, centrey, centrez;          //holds centre coord of obj

int ix1,iy1;                //holds pespective projected 2d coords
int ix2,iy2;
int ix3,iy3;
int ix4,iy4;

int inx, iny, inz;
int icx, icy;          //holds pespective projected 2d coords of centre of obj

mg_col_tag colline;
mg_col_tag colfill;
mg_col_tag col_select_1;
mg_col_tag col_select_2;
mg_col_tag col_select_3;

bool show_text_background;							//set this to give text an outline or shadow look
mg_col_tag foregnd;									//colour of text on top
mg_col_tag backgnd;									//colour of below text (outline or shadow look)

int font;											//set to -1 for no change
int fontsize;										//set to -1 for no change

int grid_line_wid;                                  //grid line width
en_surfaceplot_line_style grid_line_style;          //grid line style, see: en_surfaceplot_line_style


double face_angle;		//holds angle of obj's normal to observer, 0 degrees means obj facing the observer, +90 obj facing upward, -90 means obj facing downward

//col_tag col_hightlight_x;
//col_tag col_hightlight_y;

//the highlighted east side rect is rectangle left of rect that has west side highlighted
//this is done so no matter what z-order the rectangles end up drawn with, the
//highlighted line will always be drawn highlighted
int hightlight_x_line;	// 1 = west, 2 = east sides of this rectangle is hightlighted ( left hand side )
int hightlight_y_line;	// 1 = south, 2 = north side of this rectangle is hightlighted ( nearest side of viewer )

bool valid;

unsigned int options;                   //refer en_plot_options_tag
int id;                                 //holds unique id for ref


int line_wid;                           //user line obj
en_surfaceplot_line_style line_style;   //user line obj

//double gradient;                        //line equations vars
//double yintercept;

string stext;
};






//holds one user line obj that has been broken down to surface rect line segments
struct surfaceplot_lineseg_obj_tag
{
vector<st_surfplot_tag> vlineobj;                   //used to hold user line segmented objs
vector<st_surfplot_tag> vzlineobj;			        //holds 2d surface rects in z order for plotting

};





class surface3d : public Fl_Double_Window
{
private:
bool left_button;
bool middle_button;
bool right_button;

bool ctrl_key;
bool shift_key;


double cap_rotz;
double cap_rotx;
double cap_roty;
int cap_offx;
int cap_offy;

int mousex, mousey;
int lb_press_x, lb_press_y;
int mb_press_x, mb_press_y;
int rb_press_x, rb_press_y;

int midx, midy;
int dimx, dimy;
int surfx, surfy;
double rotx;
double roty;
double rotz;
double rho;
double srotx;							//holds trig rotation values for translate
double crotx;
double sroty;
double croty;
double srotz;
double crotz;

mg_col_tag col_ramp[ 10 ];


bool show_lines, show_filled;

int rect_count;
double valmin;
double valmax;

int selected_id;

double normals_length;					//holds size of normals that will be plotted


double dminx, dminy;
double dmaxx, dmaxy;

vector<double> vampl;							    //holds actual amplitude at a grid points
vector<st_surfplot_tag> vsp;				        //holds 3d surface rectangles
vector<st_surfplot_tag> vzorder;			        //holds 2d surface rects in z order for plotting

vector<st_surfplot_tag> vdotmark;                   //used to mark objs in some way
vector<st_surfplot_tag> vzdotmark;			        //holds 2d surface rects in z order for plotting

//vector<st_surfplot_tag> vlineobj;                   //used to hold user line segmented objs
//vector<st_surfplot_tag> vzlineobj;			        //holds 2d surface rects in z order for plotting

vector<surfaceplot_line_obj_tag> vlo;               //holds user line objs

vector<surfaceplot_lineseg_obj_tag> vlineseg;       //used to user line segmented objs

vector<surfaceplot_text_obj_tag> vtext;				//holds test objs



public:
int text_font;
int text_size;

mg_col_tag col_bkgd;
mg_col_tag col_line;
mg_col_tag col_hightlight_x;
mg_col_tag col_hightlight_y;
mg_col_tag col_fill;
bool force_col_fill;
bool bdotmark;

int grid_line_wid;                                  //grid line width, !!!!!!!!!   MUST be legal see: 'to avoid windows error dlg: fl_line_style(): Could not create GDI pen object'
en_surfaceplot_line_style grid_line_style;          //grid line style, see: en_surfaceplot_line_style

double scale, scalex, scaley;
int offx, offy;
double ampl_gain;
double rotate_x;
double rotate_y;
double rotate_z;
int highlight_every_x;
int highlight_every_y;
double std_wheel_step;
double ctrl_wheel_step;
double shift_wheel_step;
double wheel_gain;
bool show_xyz_gain_text;

public:
surface3d( int x, int y, int w, int h, const char *label = 0 );

bool build_surface( vector<double> &o, int dim_x, int dim_y, bool fill, bool lines, vector<surfaceplot_line_obj_tag> &lo, vector<surfaceplot_text_obj_tag> &vtxt );
bool build_surface2( vector<st_surfaceplot_val_tag> &o, int dim_x, int dim_y, bool fill, bool lines, vector<surfaceplot_line_obj_tag> &lo, vector<surfaceplot_text_obj_tag> &vtxt );

private:
void draw();
int handle( int e );
void make_grid();
int find_rect_clicked( int xx, int yy );

void plot_obj( vector<st_surfplot_tag > &o, int ptr, bool fill, bool line, bool user_line_obj, bool user_text_obj );


void plot( bool line, bool fill );

//void plot_3d_rect2( int ptr, bool line, bool fill, col_tag colfill, col_tag colline );

//void plot_3d_rect( double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4, bool line, bool fill, col_tag colfill, col_tag colline );

//void plot_3d( double x1, double y1, double z1, double x2, double y2, double z2, col_tag col );
//void translate( double &x1, double &y1, double &z1 );
void set_translate_vals( double rotate_x, double rotate_y, double rotate_z );
void trans_rotate_persp_obj( vector<st_surfplot_tag> &vv, vector<st_surfplot_tag> &vt, int ptr );

void trans_rotate_project( vector<st_surfplot_tag> &vv, vector<st_surfplot_tag> &vt, int ptr );
void trans_rotate( vector<st_surfplot_tag> &vv, vector<st_surfplot_tag> &vt, int ptr );
void perpective_proj( vector<st_surfplot_tag> &vt, int ptr );

double sinc( double x );
void matrix_formula_gen();
void dissolve_col( double fader, mg_col_tag &col_a, mg_col_tag &col_b, mg_col_tag &col_fader );
void dissolve_ramp( double level, mg_col_tag &col );
//double scale_line( double &aa, double &bb, double scale );
void find_centre( st_surfplot_tag o, double &centrex, double &centrey, double &centrez );
void get_centre( st_surfplot_tag o, double &centrex, double &centrey, double &centrez );
void scale_surface( st_surfplot_tag &o, double scalex, double scaley );
void calc_centre( st_surfplot_tag &o );
void translate_obj( st_surfplot_tag &o, double dx, double dy, double dz );
void translate_obj2( st_surfplot_tag &o, double dx, double dy, double dz );
void translate_line_to(  double &xx, double &yy, double &zz, double nx, double ny, double nz );
bool point_in_polygon( vector<int> &vx, vector<int> &vy, int xx, int yy );

void make_point_from_corner( st_surfplot_tag &vt1, double proportion, en_which_corner_tag which );
void make_point( double x1, double y1, double z1, double x2, double y2, double z2, double delta, double &xx, double &yy, double &zz );
//void make_centre( st_surfplot_tag &o, double &cx, double &cy, double &cz );
void calc_normals( st_surfplot_tag &o );
void rotate_point( double x1, double y1, double z1, double &x2, double &y2, double &z2, double thetax, double thetay, double thetaz );


};

//-----------------------













//-----------------------
//mgraph

//v1.07     30-may-2017

enum en_mgraph_line_style
{
en_mls_solid = FL_SOLID,
en_mls_dash = FL_DASH,
en_mls_dot = FL_DOT,
en_mls_dashdot = FL_DASHDOT,
en_mls_dashdotdot = FL_DASHDOTDOT,
en_mls_cap_flat = FL_CAP_FLAT,
en_mls_cap_round = FL_CAP_ROUND,
en_mls_cap_square = FL_CAP_SQUARE,
en_mls_cap_join_miter = FL_JOIN_MITER,
en_mls_cap_join_round = FL_JOIN_ROUND,
en_mls_cap_join_bevel = FL_JOIN_BEVEL,

};







struct pnt_tag
{
double x, y;
int r, g, b;
bool sel;									//used for group selection
};





struct trace_tag
{
string label;
vector<pnt_tag>pnt;
int id;                                     //this is useful to find the trace you want, if you don't know what order traces were push_back'd

mg_col_tag col;
double minx, maxx, miny, maxy;

en_mgraph_line_style line_style;
int line_thick;
bool vis;
bool lineplot;					            //connect sample points with lines

int border_left;
int border_right;
int border_top;
int border_bottom;

int clip_left;								//clip rect for this trace
int clip_right;
int clip_top;
int clip_bottom;

int use_pos_y;				//if >=0 will use this value as the trace id to get a specific y position, set to -1 for no offset

							//NOTE: if 'use_scale_y' is set to a trace id, this parameter should also be set as so to maintain correct coord relationship
int use_scale_y;			//if >=0 will use this value as the trace id to get a specific y scale, specify objs coords with similar range to trace coords,
							//set to -1 for no scaling, where values represent integer pixels 

double xunits_perpxl;               //-1 means auto scaling,  -2 means the use of graticle unit/division, so 1.0 amounts to 1 div

double yunits_perpxl;               //-1 means auto scaling
									//-2 means the use of graticle unit/division, so 1.0 amounts to 1 div
									//-3 means scale using which ever is the larger absolute wise: maxy or miny absolute wise: i.e: the larger of either of these: fabs(ymax) or fabs(ymin)

bool b_limit_auto_scale_min_for_y;          //set this to force auto y scaling min to 'limit_auto_scale_min_for_y'
double limit_auto_scale_min_for_y;          //this value limits the allowable display scale reduction when very large sample transitions occur

bool b_limit_auto_scale_max_for_y;          //set this to force auto y scaling max to 'limit_auto_scale_max_for_y'
double limit_auto_scale_max_for_y;          //this value limits the allowable display scale magnification when only very small sample transitions occur

double scalex;								//this allow individual trace scaling, 1.0 = no scaling
double scaley;								//this allow individual trace scaling, 1.0 = no scaling


double posx;
double posy;

int plot_offsx;								//pixel offsets just before plotting, not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
int plot_offsy;

int selected_idx;
int selected_pixel_rect_left;
int selected_pixel_rect_wid;
int selected_pixel_rect_top;
int selected_pixel_rect_hei;

int hover_sample_idx;						//sample idx the mouse is hovering over, if any

int mousex_low_sample_idx;					//lower sample idx mousex is nearest to (unlike 'hover_sample_idx',this does not req mouse to actually be within sample's rect, just the near it x-wise) 
int mousex_high_sample_idx;					//higher sample idx mousex is nearest to - see: 'get_mousex_sample_boundary_idx()'

bool show_as_spectrum;
double spectrum_baseline_y;

mg_col_tag single_sel_col;

mg_col_tag group_sel_trace_col;				//used if 'pnt[xx].sel' is set
mg_col_tag group_sel_rect_col;				//used if 'pnt[xx].sel' is set

bool sample_rect_hints_double_click;		//set this if a left double click is to show all sample point rectangles, set COLOUR with 'sample_rect_hints_col', (rect are shown regardless of congestion settings in: 'sample_rect_hints_distancex','sample_rect_hints_distancey'

//----- helps stop over hinting if sample point rectangle distances are too close -----
bool sample_rect_hints;						//if set the individual sample points are shown with a rect (if they don't cause congestion see below)
mg_col_tag sample_rect_hints_col;
vector<int>vrect_hints_px;					//used JUST in draw(), [NOT for user], to build/display a list of sample point bounding rect hints
vector<int>vrect_hints_py;
vector<bool>vrect_hints_sel;				//used JUST in draw(), [NOT for user], for: 'pnt[xx].sel'

//set these two vars zero to disable congestion detection
int sample_rect_hints_distancex;			//helps stop over hinting if x plot distances are too close, e.g: if set to 15, then when rects are less than 15 pixels apart (caused by zooming out) the hinting is turned off.
int sample_rect_hints_distancey;			//same as above, NOTE: if the graph is displaying y amplitudes, and you zooming into to a point on of the trace where only flat horizotal line is visible then
											//enabling 'show_sample_rect_hints_distancey' will ERRONEOUSLY clear the rectangles even though they are far apart x-wise
	
//------------------------------------------------------------------------------------


void (*left_click_cb_p_callback)( void *args );                 //see add_trace() to init this to zero
void *left_click_cb_args;

void (*left_double_click_cb_p_callback)( void *args );          //see add_trace() to init this to zero
void *left_double_click_cb_args;


void (*middle_click_cb_p_callback)( void *args );                //see add_trace() to init this to zero
void *middle_click_cb_args;


void (*right_click_cb_p_callback)( void *args );                //see add_trace() to init this to zero
void *right_click_cb_args;

void (*right_double_click_cb_p_callback)( void *args );         //see add_trace() to init this to zero
void *right_double_click_cb_args;


void (*keydown_cb_p_callback)( void *args, int int0 );          //see add_trace() to init this to zero
void *keydown_cb_args;

void (*keyup_cb_p_callback)( void *args, int int0 );          	//see add_trace() to init this to zero
void *keyup_cb_args;

void (*mousewheel_cb_p_callback)( void *args, int int0 );       //see add_trace() to init this to zero
void *mousewheel_cb_args;


//mouse callback values
//int0 holds mouse xpos as value between (left) 0 -->??(right)
//int1 holds mouse xpos as value between (top) 0 -->??(botm)
//dble0 holds mouse xpos as value between (left)-1.0 --> 0.0 --> +1.0 (right)
//dble1 holds mouse ypos as value between (botm)-1.0 --> 0.0 --> +1.0 (top)

void (*mousemove_cb_p_callback)( void *args, int int0, int int1, double dble0, double dble1  );       //see add_trace() to init this to zero
void *mousemove_cb_args;

};








struct st_mgraph_text_tag
{
int x;
int y;
string text;
int r;
int g;
int b;
int font;
int size;
};




//boolean to get the desired justification
enum en_mgraph_text_justify
{
en_tj_none=0,								//text is left justified so its right of the coord, and vertically positioned so bottom of text is on the coord 
en_tj_horiz_center=1,						//text is horizontally centred to coord
en_tj_horiz_right=2,						//text is left of the coord
en_tj_vert_center=4,						//text is vertically centred to coord
en_tj_vert_top=8,							//text is shifted downward so top of text is at coord
};




//mgraph draw object types  -- see: 'st_mgraph_draw_obj_tag'
enum en_mgraph_draw_obj_type
{
en_dobt_point,											//x1,y1
en_dobt_line,											//x1,y1,x2,y2
en_dobt_rect,											//x1,y1: first extreme,  x2,y2: second extreme (does not use a wid or hei)
en_dobt_rectf,											//x1,y1: first extreme,  x2,y2: second extreme (does not use a wid or hei)
en_dobt_arc,											//unfilled, define a bounding rect with: x1, y1, x2, y2, and start stop angles: arc1, arc2
en_dobt_pie,											//filled, define a bounding rect with: x1, y1, x2, y2, and start stop angles: arc1, arc2
en_dobt_text,											//just: x1,y1
en_dobt_polypoints,										//see: fl_begin_points(), just: x1,y1
en_dobt_polyline,										//see: fl_begin_line()
en_dobt_polygon,										//unfilled polygon, no need to close polygon with a last point matching the first
en_dobt_polygonf,										//filled polygon - see: fl_begin_polygon(), no need to close polygon with a last point matching the first, 3 coords make a triangle, go clockwise


en_dobt_polypoints_wnd_coords,							//point coords are absolute window integer pixel coords (not relative to x1, y1, where 0 is top left, down is positive), assumes: 'use_pos_x = use_scale_x = use_pos_y = use_scale_y = -1;'
en_dobt_polyline_wnd_coords,							//" "
en_dobt_polygon_wnd_coords,								//" "
en_dobt_polygonf_wnd_coords,							//" "
};



//holds one draw obj for mgraph
struct st_mgraph_draw_obj_tag
{
en_mgraph_draw_obj_type type;

bool visible;

int clip_left;											//clip rect for this obj
int clip_right;
int clip_top;
int clip_bottom;

int use_pos_x;				//if >=0 will use this value as the trace id to get a specific x position,set to -1 for no offset
							//NOTE: if 'use_scale_x' is set to a trace id, this parameter should also be set as so to maintain correct coord relationship 

int use_pos_y;				//if >=0 will use this value as the trace id to get a specific y position, set to -1 for no offset
							//NOTE: if 'use_scale_y' is set to a trace id, this parameter should also be set as so to maintain correct coord relationship

int use_scale_x;			//if >=0 will use this value as the trace id to get a specific x scale, specify objs coords with similar range to trace coords,
							//set to -1 for no scaling, where values represent integer pixels 

int use_scale_y;			//if >=0 will use this value as the trace id to get a specific y scale, specify objs coords with similar range to trace coords,
							//set to -1 for no scaling, where values represent integer pixels 

double x1;
double y1;
double x2;							//rectangles don't use width or height, specify just two coords x1,y1,  x2,y2 extremes
double y2;							//for arcs and pies, x2=wid, y2=hei

vector<double> vpolyx;				//for poly objs (fl_vertex types)
vector<double> vpolyy;

double arc1, arc2;
string stext;
en_mgraph_text_justify justify;
int r;
int g;
int b;
int font;
int font_size;
en_mgraph_line_style line_style;
int line_thick;
int draw_ordering;					//0 = before grid,  1 = after graticle but before traces,   2 = after traces, and in order of being 'pushed'
};















class mgraph : public Fl_Box
{
private:

int woffx;
int woffy;
int sizex;
int sizey;
int mousex, mousey;
bool double_click_left;
bool inside_control;

int idx_maxx;
int idx_maxy;


bool show_axisx;
bool show_axisy;


//double val_y_min, val_y_max, val_x_min, val_x_max;
int ref_id;                             //holds id of trace to use as reference, should not be negative, see: int z = vdrwobj[ i ].use_scale_x;


double ref_scalex;						//used to reference other traces wrt to 1st trace
double ref_d_midx;						//used to reference other traces wrt to 1st trace


int at_draw_ordering;					//keep track of where drawing stages are at, for user's draw obj preferred ordering, 0 = before grid,  1 = after grid before traces,   2 = after traces

vector<double> vscalex;              	//keep each trace's params, also used for: vdrwobj
vector<double> vscaley;
vector<double> vtrace_scale_x;
vector<double> vtrace_scale_y;
vector<double> vtrace_positionx;
vector<double> vtrace_positiony;
vector<double> vtrace_dmidx;
vector<double> vtrace_dmidy;
vector<int> vtrace_offx;
vector<int> vtrace_offy;
vector<int> vtrace_wid;
vector<int> vtrace_hei;
vector<int> vtrace_midx;
vector<int> vtrace_midy;
vector<int> vtrace_id;
vector<int> vplot_offsx;
vector<int> vplot_offsy;
vector<int> vtrace_minx;												//v1.16


void (*left_click_anywhere_cb_p_callback)( void *args );            //non trace specific
void *left_click_anywhere_cb_args;

void (*left_click_release_cb_p_callback)( void *args );
void *left_click_release_cb_args;

void (*left_double_click_anywhere_cb_p_callback)( void *args );
void *left_double_click_anywhere_cb_args;

void (*right_click_anywhere_cb_p_callback)( void *args );
void *right_click_anywhere_cb_args;

void (*right_double_click_anywhere_cb_p_callback)( void *args );
void *right_double_click_anywhere_cb_args;

void (*right_click_release_cb_p_callback)( void *args );
void *right_click_release_cb_args;

void (*mouseenter_cb_p_callback)( void *args );
void *mouseenter_cb_args;

void (*mouseleave_cb_p_callback)( void *args );
void *mouseleave_cb_args;



public:
bool left_button;
bool right_button;
bool middle_button;
int b_invert_wheel;
int mousewheel;

vector<trace_tag>trce;
int rect_size;
int graph_id;							//place a unique id value here
mg_col_tag border_col;
mg_col_tag background;
//int selected_sample;
//int selected_pixel_rect_left;
//int selected_pixel_rect_right;
//int selected_pixel_rect_top;
//int selected_pixel_rect_bot;

int bkgd_border_left;					//use to size background rect
int bkgd_border_top;
int bkgd_border_right;
int bkgd_border_bottom;

int graticule_border_left;				//use to size graticule rect
int graticule_border_top;
int graticule_border_right;
int graticule_border_bottom;
int graticle_count_y;
int graticle_count_x;
int grat_pixels_x;
int grat_pixels_y;
bool cro_graticle;
int htime_lissajous;                    //0 = htime, 
                                        //1 = channel 1-> H , channel 2-> V 
                                        //2 = channel 2-> H , channel 1-> V 
                                        //3 = channel 3-> H , channel 4-> V 
                                        //4 = channel 4-> H , channel 3-> V 


mg_colref col_cursor_along_y;        //cursors along y-axis (i.e. measuring time)
mg_colref col_cursor_along_x;        //cursors along x-axis (i.e. measuring amplitude)

int cursor_sel;                    //0 and 1 are vert, 2 and 3 are horiz

bool show_cursors_along_y;
bool show_cursors_along_x;
bool show_vert_units_div;
bool show_horiz_units_div;

double cursor_along_y0;         //cursors along y-axis (i.e. measuring time)
double cursor_along_y1;

double cursor_along_x0;        //cursors along x-axis (i.e. measuring amplitude)
double cursor_along_x1;

st_mgraph_text_tag text_cursor_along_y;             //holds text
st_mgraph_text_tag text_cursor_along_x;

st_mgraph_text_tag text_vert_units_div;
st_mgraph_text_tag text_horiz_units_div;
vector<st_mgraph_draw_obj_tag> vdrwobj;             //holds additional obj for drawing

int last_clicked_trace;                	//this is not the trace_id, but index of trace in: vector<trace_tag>trce
int hover_trace_idx;					//trace idx the mouse is hovering over, if any


bool use_line_clip;                     //set this to activate:  line_clip(..)
//int clip_minx;                          //put some limits on line plotting attempts, see: mgraph::line_clip(..)
//int clip_maxx;
//int clip_miny;
//int clip_maxy;

int clip_minx;							//includes widget x,y offset, downward is positive
int clip_maxx;
int clip_miny;							//NOTE: 'clip_miny' here is in an decreasing direction up the screen
int clip_maxy;

int clip_vportleft;						//upward is positive, for: 'line_clip()', 'clip_poly()' algorithm
int clip_vportright;
int clip_vporttop;						//NOTE: 'clip_vporttop' here is in an increasing direction up the screen
int clip_vportbot;



int last_mousex;				//holds pos vals covering the graticule (the border gaps being removed - also sent to: mousemove_cb_p_callback )
int last_mousey;
double last_nomalised_mousex;	//holds pos vals covering the graticule (the border gaps being removed - also sent to: mousemove_cb_p_callback )
								//(left)-1.0 --> 0.0 --> +1.0 (right)
double last_nomalised_mousey;	//(botm)-1.0 --> 0.0 --> +1.0 (top)



public:
mgraph( int x, int y, int w, int h, const char *label = 0 );
~mgraph();
void clear_traces();
void clear_trace( int idx );
void add_trace( trace_tag &tr );
void render( int ref_id_in );
bool get_sample_count_for_trc( int trc, int &count );
bool get_sample_count_using_id( int trc_id, int &count );
bool get_selected_value( int trc, double &xx, double &yy );
bool get_selected_value_using_id( int trc_id, int &idx, double &xx, double &yy );
void get_trace_min_max( int trace, double &xmin, double &xmax, double &ymin, double &ymax );
void set_trace_visibility( int idx, bool visible );
bool get_selected_idx( int trc, int &sel_idx );
void set_selected_sample( int trc, int sel_sample_idx, int issue_which_callback );

bool get_hover_idx( int trc, int &hov_idx );
bool get_hover_value_using_id( int trc_id, int &idx, double &xx, double &yy );

bool get_left_edge_sample_idx( int trc, int &left_idx );
bool get_right_edge_sample_idx( int trc, int &right_idx );


void get_trace_maxx( int trc, int &idx, double &x );
void get_trace_maxy( int trc, int &idx, double &x );

void set_left_click_anywhere_cb( void (*p_cb)( void* ), void *args_in );            //not trace specific
void set_left_double_click_anywhere_cb( void (*p_cb)( void* ), void *args_in );

void set_right_click_anywhere_cb( void (*p_cb)( void* ), void *args_in );
void set_right_double_click_anywhere_cb( void (*p_cb)( void* ), void *args_in );
void set_mouseenter_cb( void (*p_cb)( void* ), void *args_in );
void set_mouseleave_cb( void (*p_cb)( void* ), void *args_in );


bool set_left_click_cb( int trc, void (*p_cb)( void* ), void *args_in );
bool set_left_double_click_cb( int trc, void (*p_cb)( void* ), void *args_in );
void set_left_click_release_cb( void (*p_cb)( void* ), void *args_in );

bool set_middle_click_cb( int trc, void (*p_cb)( void* ), void *args_in );

bool set_right_click_cb( int trc, void (*p_cb)( void* ), void *args_in );
bool set_right_double_click_cb( int trc, void (*p_cb)( void* ), void *args_in );
void set_right_click_release_cb( void (*p_cb)( void* ), void *args_in );

bool set_keydown_cb( int trc, void (*p_cb)( void*, int int0 ), void *args_in );
bool set_keyup_cb( int trc, void (*p_cb)( void*, int int0 ), void *args_in );

bool set_mousewheel_cb( int trc, void (*p_cb)( void*, int int0 ), void *args_in );
bool set_mousemove_cb( int trc, void (*p_cb)( void*, int int0, int int1, double dble0, double dble1 ), void *args_in );


bool calc_draw_coords( int trc, double inx, double iny, int &xx, int &yy );
bool calc_draw_coords_internal( int trc, double inx, double iny, int &xx, int &yy );
void draw_text();
int find_trace_idx_from_trace_id( int id );
int find_trace_id_from_trace_idx( int trc );

void get_background_dimensions( int &wid, int &hei );
bool get_trace_vis_dimensions( int trc, int &wid, int &hei );
bool zoom_h( int trc, int zoom_at_sample_idx, float scale_in, bool force_extent_when_contracted_beyond );
void zoom_h_all_traces( int zoom_idx, float scale_change, bool force_extent_when_contracted_beyond );

void deselect_sample( int trc, int issue_which_callback );
void deselect_all_samples( int issue_which_callback );

bool get_posx( int trc, double &val );
bool get_posy( int trc, double &val );
bool get_scalex( int trc, double &val );
bool get_scaley( int trc, double &val );

bool adj_posx_by( int trc, double delta, bool need_redraw );
bool adj_posx_all_traces_by( double delta, bool need_redraw );
bool adj_posy_by( int trc, double delta, bool need_redraw );
bool adj_posy_all_traces_by( double delta, bool need_redraw );
bool adj_scalex_by( int trc, double delta, bool need_redraw );
bool adj_scalex_all_traces_by( double delta, bool need_redraw );
bool adj_scaley_by( int trc, double delta, bool need_redraw );
bool adj_scaley_all_traces_by( double delta, bool need_redraw );
bool set_posx( int trc, double val, bool need_redraw );
bool set_posx_all_traces( double val, bool need_redraw );
bool set_posy( int trc, double val, bool need_redraw );
bool set_posy_all_traces( double val, bool need_redraw );
bool set_scalex( int trc, double val, bool need_redraw );
bool set_scalex_all_traces( double val, bool need_redraw );
bool set_scaley( int trc, double val, bool need_redraw );
bool set_scaley_all_traces( double val, bool need_redraw );
bool set_plot_offsx( int trc, double val, bool need_redraw );
bool adj_plot_offsx_by( int trc, double val, bool need_redraw );
bool set_plot_offsy( int trc, double val, bool need_redraw );
bool adj_plot_offsy_by( int trc, double val, bool need_redraw );

bool get_mousex_sample_boundary_idx( int trc, int &low_idx, int &high_idx );
bool get_mousex_sample_boundary_idx_using_id( int trc_id, int &low_idx, int &high_idx );
bool center_on_sample( int trc, int sample_idx );

void get_mouse_pixel_position_on_background( int &mx, int &my );
bool get_mouse_pixel_position_on_trace( int trc, int &mx, int &my );

bool get_mouse_position_relative_to_trc( int trc, double &mx, double &my );			// SEE also !!!! get_mouse_position_relative_to_trc_new()
bool get_mouse_position_relative_to_trc_new( int trc, double &mx, double &my );

bool set_left_edge_sample_idx( int trc, int left_idx );
bool calc_draw_coords_for_trc_sample( int trc, int samp_idx, int &ix, int &iy );
int get_selected_trace_and_sample_indexes( int &samp_idx );
bool get_trace_offsxy( int trc, int &offx, int &offy );
void get_background_offsxy( int &offx, int &offy );
int get_hover_trace_idx();
bool get_posx_relative_to_trace( int trc, double &val );
bool get_posy_relative_to_trace( int trc, double &val );
void get_graticle_dimensions( int &wid, int &hei );
void get_graticle_offsxy( int &offx, int &offy );
bool get_sample_value( int trc, int idx, double &xx, double &yy );
bool get_plot_values( int trc, double &x1, double &y1, double &x2, double &y2, double &x3, double &y3, double &x4, double &y4 );
bool get_trace_to_plot_factors( int trc, double &trace_scalex, double &d_midx, double &scalex, double &trace_midx, int &trace_offx, double &positionx, int &plot_offsx,   double &trace_scaley, double &d_midy, double &scaley, double &trace_midy, int &trace_offy, double &positiony, int &plot_offsy );
void resize( int xx, int yy, int ww, int hh );
void get_trace_midxy( int idx, double &midx, double &midy );
bool get_plot_offsy( int trc, int &val );


private:
void draw();
int handle( int );
bool get_mouse_vals_internal( int trc, int mx, int my, double &x, double &y, bool get_as_hover );
int get_trace_idx_from_id( int id );
void clip_draw_line( int xx, int yy, int xxx, int yyy );
int rect_clip( int xx, int yy, int dx, int dy );
void draw_user_objs( int at_draw_ordering );

int inside_window( int x, int y, int edge );
int calc_condition( int x1, int y1, int x2, int y2, int edge );
void solve_intersection( int edge, int x1, int y1, int x2, int y2, int &x, int &y );
void add_point( int &ptr, vector<clip_coord_tag> &vo, int xx, int yy );
void check_sutherland_hodgmen();
bool clip_poly( vector<clip_coord_tag> vcrd_in, vector<clip_coord_tag> &vcrd_clip );
int compute_outcode( int xx, int yy );
bool line_clip( int &x1_in, int &y1_in, int &x2_in, int &y2_in );
void clip_draw_point( int xx, int yy );


};

//-----------------------









//-------------------------------------------------------------------------------
//SEE EXAMPLE CODE: 'test_fast_mgraph()'

#define cn_fast_mgraph_cnt_max 32
#define cn_fast_mgraph_trc_cnt_max 4


//simple to use graphing class
class fast_mgraph : public Fl_Double_Window
{
private:										//private var
void user_col( const char* szcol, mg_col_tag &col );
Fl_Menu_Bar *mnu;
int fnt_type;
int fnt_size;
int reduction;
int graph0_hei;									//for menu display shrink/restore
int sig_dig;
int wnd_cur_wid;
int wnd_cur_hei;



public:
int id0;																//just for user use
int id1;
mgraph *gph[cn_fast_mgraph_cnt_max];
mg_col_tag col_obj_axis[cn_fast_mgraph_cnt_max];

mg_col_tag col_obj_text[cn_fast_mgraph_cnt_max];

mg_col_tag col_obj_text_multi_1[cn_fast_mgraph_cnt_max];
mg_col_tag col_obj_text_multi_2[cn_fast_mgraph_cnt_max];
mg_col_tag col_obj_text_multi_3[cn_fast_mgraph_cnt_max];
mg_col_tag col_obj_text_multi_4[cn_fast_mgraph_cnt_max];

mg_col_tag col_hover_text_multi_1[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_hover_text_multi_2[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_hover_text_multi_3[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_hover_text_multi_4[ cn_fast_mgraph_cnt_max ];
	
mg_col_tag col_axis_multi_1[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_axis_multi_2[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_axis_multi_3[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_axis_multi_4[ cn_fast_mgraph_cnt_max ];


mg_col_tag col_trc_multi_1[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_trc_multi_2[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_trc_multi_3[ cn_fast_mgraph_cnt_max ];
mg_col_tag col_trc_multi_4[ cn_fast_mgraph_cnt_max ];

string trc_label_multi_1[ cn_fast_mgraph_cnt_max ];
string trc_label_multi_2[ cn_fast_mgraph_cnt_max ];
string trc_label_multi_3[ cn_fast_mgraph_cnt_max ];
string trc_label_multi_4[ cn_fast_mgraph_cnt_max ];

int gap;
int gph_cnt;
int x_axis_offs;
int left_hov_idx;
float scale_change;
bool keya[ cn_fast_mgraph_cnt_max ];
bool keyy[ cn_fast_mgraph_cnt_max ];
bool keym[ cn_fast_mgraph_cnt_max ];
bool keyshift[ cn_fast_mgraph_cnt_max ];
int sel_idx1[ cn_fast_mgraph_cnt_max ];
int sel_idx2[ cn_fast_mgraph_cnt_max ];
vector<double> vtrc_valx[ cn_fast_mgraph_cnt_max ];
vector<double> vtrc_valy1[ cn_fast_mgraph_cnt_max ];		//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
vector<double> vtrc_valy2[ cn_fast_mgraph_cnt_max ];
vector<double> vtrc_valy3[ cn_fast_mgraph_cnt_max ];
vector<double> vtrc_valy4[ cn_fast_mgraph_cnt_max ];
string trc_label[ cn_fast_mgraph_cnt_max ];					//if user wants to label a trace using the trace's colour key,
mg_col_tag color_trc[ cn_fast_mgraph_cnt_max ];				//also used for trc_label
mg_col_tag col_hover_text[cn_fast_mgraph_cnt_max];				//used for hover text colour

double scale_x;
double scale_trc_y[ cn_fast_mgraph_cnt_max ][cn_fast_mgraph_trc_cnt_max];
double max_defl_y[ cn_fast_mgraph_cnt_max ];					//value that should be shown as maximum y deflection, user adj via 'm' key and mousewheel
double shift_trc_y[ cn_fast_mgraph_cnt_max ][cn_fast_mgraph_trc_cnt_max];
float scale_change_h;										//global/static var

double pos_x;

double xunits_perpxl[ cn_fast_mgraph_cnt_max ];				//mgraph
double yunits_perpxl[ cn_fast_mgraph_cnt_max ];				//-1 means auto scaling,  set this to override auto scaling of the first trace (other traces are referenced to first trace)

int drag_start_mousex;
double drag_start_posx;
bool drag_started;
int multi_trace;								//if non zero more than one trace is shown on a single graph

bool use_logx, use_logy;									//v1.13, for log10 plots
bool disp_vals_undo_logx, disp_vals_undo_logy;				//v1.13, useful to keep a log trace's displayed values linear (e.g. for freq values)
double disp_vals_multiplier_x, disp_vals_multiplier_y;		//v1.13, useful for log plots, e.g: 20*log10, set to 20

bool sample_rect_hints_double_click;
bool sample_rect_hints;
mg_col_tag sample_rect_hints_col;
int sample_rect_hints_distancex;							//helps stop congestion
int sample_rect_hints_distancey;

bool center_sel_sample_on_zoom_change;

vector<st_mgraph_draw_obj_tag> vextra_drwobj[cn_fast_mgraph_cnt_max];     //holds additional obj for drawing

int xtick_marks[cn_fast_mgraph_cnt_max];								//for axis, number of ticks to show
int ytick_marks[cn_fast_mgraph_cnt_max];

bool b_x_axis_values_derived[cn_fast_mgraph_cnt_max];					//set this to derive 'x' axis display values using a 'leftmost' value plus a 'inc' value (only works with plots that have no 'x' vector supplied)
double x_axis_values_derived_left_value[cn_fast_mgraph_cnt_max];		//first 'x' plot point value (left most plot point)
double x_axis_values_derived_inc_value[cn_fast_mgraph_cnt_max];			//the inc value to be added to 'x_axis_values_derived_left_value' for each successive plot of 'y' vector

bool b_x_axis_vector_supplied[cn_fast_mgraph_cnt_max];

void (*left_click_cb_p )( void *mgrap, void *args );
void *left_click_cb_args;

void (*middle_click_cb_p )( void *mgrap, void *args );
void *middle_click_cb_args;

void (*right_click_cb_p )( void *mgrap, void *args );
void *right_click_cb_args;

int last_sel_trc;
int last_sel0_idx[ cn_fast_mgraph_cnt_max ], last_sel1_idx[ cn_fast_mgraph_cnt_max ];		//actually only ever use index: [0] for delta calcs
double last_sel0_x[ cn_fast_mgraph_cnt_max ], last_sel1_x[ cn_fast_mgraph_cnt_max ];
double last_sel0_y[ cn_fast_mgraph_cnt_max ], last_sel1_y[ cn_fast_mgraph_cnt_max ];


private:


public:
fast_mgraph( int gph_cnt = 1, int xx = 100, int yy = 100, int wid = 900, int hei = 500, const char *label = "FastMGraph" );
~fast_mgraph();
void set_selected_sample( int gph_idx, int trc, int sel_sample_idx );
void set_selected_sample( int trc, int sel_sample_idx );

void set_col_bkgd( int gph_idx, const char *col_bkgd );
void set_col_bkgd_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );

void set_col_axis1( int gph_idx, const char *col_axis );
void set_col_axis2( int gph_idx, const char *col_axis );
void set_col_axis3( int gph_idx, const char *col_axis );
void set_col_axis4( int gph_idx, const char *col_axis );

void set_col_axis1_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_axis2_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_axis3_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_axis4_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );


void set_col_hover_text1( int gph_idx, const char *col_axis );
void set_col_hover_text2( int gph_idx, const char *col_axis );
void set_col_hover_text3( int gph_idx, const char *col_axis );
void set_col_hover_text4( int gph_idx, const char *col_axis );

void set_col_hover_text1_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_hover_text2_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_hover_text3_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_hover_text4_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );



void set_obj_col_text1( int gph_idx, const char *col_text );
void set_obj_col_text2( int gph_idx, const char *col_text );
void set_obj_col_text3( int gph_idx, const char *col_text );
void set_obj_col_text4( int gph_idx, const char *col_text );

void set_obj_col_text1_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_obj_col_text2_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_obj_col_text3_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_obj_col_text4_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );

void set_col_trc1( int gph_idx, const char *col_trc1 );
void set_col_trc2( int gph_idx, const char *col_trc2 );
void set_col_trc3( int gph_idx, const char *col_trc3 );
void set_col_trc4( int gph_idx, const char *col_trc4 );
void set_col_trc1_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_trc2_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_trc3_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );
void set_col_trc4_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb );

void set_trc_label1( int grph_idx, string ss );
void set_trc_label2( int grph_idx, string ss );
void set_trc_label3( int grph_idx, string ss );
void set_trc_label4( int grph_idx, string ss );


void font_type( int ii );
void font_size( int ii );
void scale_y( int gph_idx, double trc1, double trc2=1.0, double trc3=1.0, double trc4=1.0 );
void shift_y( int gph_idx, double trc1, double trc2=0.0, double trc3=0.0, double trc4=0.0 );
void set_sig_dig( int ii );
void set_xunits_perpxl( double trc1=-1, double trc2=-1, double trc3=-1, double trc4=-1 );
void set_yunits_perpxl( double trc1=-1, double trc2=-1, double trc3=-1, double trc4=-1 );
void set_ymax_deflection_value( int gph_idx, double y_max_delf );


//multi graph, one trace per graph
void plot_vfloat( int gph_idx, vector<float> &vy1 );
void plotxy_vfloat( int gph_idx, vector<float> &vx, vector<float> &vy1 );

void plot_vdouble( int gph_idx, vector<double> &vy1 );
void plotxy_vdouble( int gph_idx, vector<double> &vx, vector<double> &vy1 );

//one graph, multi trace
void plot_vfloat_1( vector<float> &vy1 );
void plot_vfloat_2( vector<float> &vy1, vector<float> &vy2 );
void plot_vfloat_3( vector<float> &vy1, vector<float> &vy2, vector<float> &vy3 );
void plot_vfloat_4( vector<float> &vy1, vector<float> &vy2, vector<float> &vy3, vector<float> &vy4 );

void plot_vdouble_1( vector<double> &vy1 );
void plot_vdouble_2( vector<double> &vy1, vector<double> &vy2 );
void plot_vdouble_3( vector<double> &vy1, vector<double> &vy2, vector<double> &vy3 );
void plot_vdouble_4( vector<double> &vy1, vector<double> &vy2, vector<double> &vy3, vector<double> &vy4 );

void plotxy_vfloat_1( vector<float> &vx, vector<float> &vy1 );
void plotxy_vfloat_2( vector<float> &vx, vector<float> &vy1, vector<float> &vy2 );
void plotxy_vfloat_3( vector<float> &vx, vector<float> &vy1, vector<float> &vy2, vector<float> &vy3 );
void plotxy_vfloat_4( vector<float> &vx, vector<float> &vy1, vector<float> &vy2, vector<float> &vy3, vector<float> &vy4 );

void plotxy_vdouble_1( vector<double> &vx, vector<double> &vy1 );
void plotxy_vdouble_2( vector<double> &vx, vector<double> &vy1, vector<double> &vy2 );
void plotxy_vdouble_3( vector<double> &vx, vector<double> &vy1, vector<double> &vy2, vector<double> &vy3 );
void plotxy_vdouble_4( vector<double> &vx, vector<double> &vy1, vector<double> &vy2, vector<double> &vy3, vector<double> &vy4 );

void plot_grph_internal( int grph_idx );



/*
//one graph, 2 traces
void plot( vector<double> vf1, vector<double> vf2, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plot( vector<float> vf1, vector<float> vf2, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plot( double *d1, int cnt1, double *d2, int cnt2, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plot( float *f1, int cnt1, float *f2, int cnt2, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plot( int *i1, int cnt1, int *i2, int cnt2, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );


//one graph, 3 traces
void plot( vector<double> vf1, vector<double> vf2, vector<double> vf3, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plot( vector<float> vf1, vector<float> vf2, vector<float> vf3, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plot( double *d1, int cnt1, double *d2, int cnt2, double *d3, int cnt3, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plot( float *f1, int cnt1, float *f2, int cnt2, float *f3, int cnt3, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plot( int *i1, int cnt1, int *i2, int cnt2, int *i3, int cnt3, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );


//one graph, 4 traces
void plot( vector<double> vf1, vector<double> vf2, vector<double> vf3, vector<double> vf4, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plot( vector<float> vf1, vector<float> vf2, vector<float> vf3, vector<float> vf4, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plot( double *d1, int cnt1, double *d2, int cnt2, double *d3, int cnt3, double *d4, int cnt4, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plot( float *f1, int cnt1, float *f2, int cnt2, float *f3, int cnt3, float *f4, int cnt4, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plot( int *i1, int cnt1, int *i2, int cnt2, int *i3, int cnt3, int *i4, int cnt4, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );


//one graph, 1 trace, x, y required
void plotxy( vector<double> vx, vector<double> vy1, const char *col_trc1="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( vector<float> vfx, vector<float> vfy1,const char *col_trc1="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( vector<int> vix, vector<int> viy1, const char *col_trc1="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( float *arrx, float *arry1, int cnt, const char *col_trc1="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( double *arrx, double *arry1, int cnt, const char *col_trc1="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( int *arrx, int *arry1, int cnt, const char *col_trc1="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );


//one graph, 2 traces, first trace x, y is supplied, 2nd trace only y is supplied and must be same length as first trace
void plotxy( vector<double> vx, vector<double> vy1, vector<double> vy2, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plotxy( vector<float> vfx, vector<float> vfy1, vector<float> vfy2, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plotxy( vector<int> vix, vector<int> viy1, vector<int> viy2, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plotxy( float *arrx, float *arry1, float *arry2, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plotxy( double *arrx, double *arry1, double *arry2, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );
void plotxy( int *arrx, int *arry1, int *arry2, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="" );


//one graph, 3 traces, first trace x, y is supplied, 2nd/3rd traces only y is supplied and must be same length as first trace
void plotxy( vector<double> vx, vector<double> vy1, vector<double> vy2, vector<double> vy3, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plotxy( vector<float> vfx, vector<float> vfy1, vector<float> vfy2, vector<float> vfy3, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plotxy( vector<int> vix, vector<int> viy1, vector<int> viy2, vector<int> viy3, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plotxy( float *arrx, float *arry1, float *arry2, float *arry3, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plotxy( double *arrx, double *arry1, double *arry2, double *arry3, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );
void plotxy( int *arrx, int *arry1, int *arry2, int *arry3, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="" );


//one graph, 4 traces, first trace x, y is supplied, 2nd/3rd/4th traces only y is supplied and must be same length as first trace
void plotxy( vector<double> vx, vector<double> vy1, vector<double> vy2, vector<double> vy3, vector<double> vy4, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plotxy( vector<float> vfx, vector<float> vfy1, vector<float> vfy2, vector<float> fy3, vector<float> vfy4, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plotxy( vector<int> vix, vector<int> viy1, vector<int> viy2, vector<int> viy3, vector<int> viy4, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plotxy( float *arrx, float *arry1, float *arry2, float *arry3, float *arry4, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plotxy( double *arrx, double *arry1, double *arry2, double *arry3, double *arry4, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );
void plotxy( int *arrx, int *arry1, int *arry2, int *arry3, int *arry4, int cnt, const char *col_trc1="brwn", const char *col_trc2="drkg", const char *col_trc3="drkcy", const char *col_trc4="drkr", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="", const char *trc_label2="", const char *trc_label3="", const char *trc_label4="" );


//multiple graphs, one trace
void plot( int gph_idx, vector<double> vdbl, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plot( int gph_idx, vector<float> vf, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plot( int gph_idx, vector<int> vint, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plot( int gph_idx, float *arr, int cnt, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plot( int gph_idx, double *arr, int cnt, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plot( int gph_idx, int *arr, int cnt, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );

//multiple graphs, x, y supplied and must be same length 
void plotxy( int gph_idx, vector<float> vx, vector<float> vy, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( int gph_idx, vector<double> vx, vector<double> vy, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( int gph_idx, vector<int> x, vector<int> vy, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( int gph_idx, float *arrx, float *arry, int cnt, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( int gph_idx, double *arrx, double *arry, int cnt, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
void plotxy( int gph_idx, int *arrx, int *arry, int cnt, const char *col_trc="brwn", const char *col_bkgd="ofw", const char *col_axis="drkb", const char *col_text="drkgry", const char *trc_label1="" );
*/


void update_fg_user_obj( int graph_idx );

void back_fill( int req_cnt, vector<int> &viv );
void back_fill( int req_cnt, vector<float> &vfv );
void back_fill( int req_cnt, vector<double> &vv );

bool set_left_click_cb( void (*p_cb)( void* ), void *args_in );
bool set_middle_click_cb( void (*p_cb)( void* ), void *args_in );
bool set_right_click_cb( void (*p_cb)( void* ), void *args_in );

private:
int handle( int e );

};

//-------------------------------------------------------------------------------


#endif
