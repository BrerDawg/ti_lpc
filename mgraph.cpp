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

//mgraph.cpp
//v1.07 mgraph	 	30-may-2017
//v1.05 surface3d 	05-jan-2018
//v1.06     		23-jan-2018 		//added: 2 structs to help remove global dependancies: mg_colref, mg_col_tag
//v1.07     		03-feb-2018 		//added user draw obj ordering, and: en_dobt_polypoints, en_dobt_polyline, en_dobt_polygon
										//also now allows user draw obj to be specified in trace 'float' coords, when 'use_scale_x/use_scale_y' are set to a trace id
//v1.08     		01-jul-2018 		//moded: get_mouse_vals_internal(),  added:  get_hover_idx(),  get_hover_value_using_id(), get_left_edge_sample_idx(),  get_right_edge_sample_idx(), get_trace_vis_dimensions(), get_background_dimensions()
										//added: zoom_h(), zoom_h_all_traces(), center_on_sample(), set_keyup_cb(), deselect_sample(), deselect_all_samples(),
										//added: adj_posx_by(), adj_posx_all_traces_by(), adj_posy_by(), adj_posy_all_traces_by(), adj_scalex_by(), adj_scalex_all_traces_by(), adj_scaley_by(), adj_scaley_all_traces_by(), 
										//added: set_posx(), set_posx_all_traces(), set_posy(), set_posy_all_traces(), set_scalex(), set_scalex_all_traces(), set_scaley(), set_scaley_all_traces(), 
										//added: get_posx(), get_posy(), get_scalex(), get_scaley(), get_mousex_sample_boundary_idx(), get_mousex_sample_boundary_idx_using_id()
										//renamed get_sample_count_for_trc() [was: 'get_sample_count()'], added: get_sample_count_using_id()
										//added: get_mouse_pixel_position_on_background(), get_mouse_pixel_position_on_trace(), get_mouse_position_relative_to_trc(), set_left_edge_sample_idx()
										//moded: calc_draw_coords_internal(), added: calc_draw_coords_internal, calc_draw_coords_for_trc_sample()
										//added: rect_clip()
										//added: 'sample_rect_hints_double_click' for all rect to be displayed, set their col with 'sample_rect_hints_col' 
										//added: congested sample point rect detection/clearing code, see: 'sample_rect_hints', 'sample_rect_hints_distancex', 'sample_rect_hints_distancey', 'sample_rect_hints_col'
										//added: 'single_sel_col'
										//added: multiple sample selection ability using: 'pnt[xx].sel', see: 'group_sel_trace_col', 'group_sel_rect_col'
										//added: set_mouseenter_cb(), set_mouseleave_cb(), and 'border_col', so border col can be change when mouse enters/leaves control
										//added: en_dobt_polygonf, and proper line and polygon clipping algorithms for user draw objs, see: 'line_clip()', 'clip_poly()'
										//added: text justification for user draw objs, see: 'en_mgraph_text_justify' and 'st_mgraph_draw_obj_tag.justify'
										//added: get_selected_trace_and_sample_indexes()
										//added: 'graph_id' allows a unique public id to be assigned
										//added: get_trace_offsxy(), get_background_offsxy(), clip_draw_point(),resize()
										//further mods, e.g: trace clipping, see: 'get_trace_to_plot_factors()', see: trace_tag.plot_offsx/y
										//added: 'fast_mgraph' class, see at bottom, there's rem'd out example code for usage: 'test_fast_mgraph()'
										//added: 'cb_fast_mgraph_menu_sample()', see 'b_allow_updating'
							
//v1.09     		21-jul-2018			//minor mod to: 'mgraph.h' for 64 bit 
										//added: back_fill()


//v1.10     		31-aug-2018			//moded 'fast_graph': set default 'sig_dig' from 2 to 4, added call to 'update_fg_user_obj(ii)' when cursor keys move the sel sample,
										//improved sample delta display operations

//v1.11     		03-sep-2018			//moded 'fast_graph': fixed 'fast_graph' multi graph trace label not displaying
//v1.12     		08-oct-2018			//moded 'fast_graph': removed all 'pass-by reference' vectors (as in: 'vector<double> &vy1' ) as 'back_fill()' was modifying them 
										//casuing problems when vectors are shared between more tham 1 instance of fast_mgraph objs

//v1.13     		31-may-2019			//added: mgraph::get_trace_midxy( int idx, double &midx, double &midy );
										//added improved" 'get_mouse_position_relative_to_trc_new()'
										//for 'fast_mgraph', added (flog10 potting options, see: 'use_logx', 'disp_vals_multiplier_x', 'disp_vals_not_logy', etc
										//for 'fast_mgraph', added 'sample_rect_hints_distancex', etc

//v1.14     		07-nov-2019			//moded: 'center_on_sample()' for selectect sample to be correctly shown after centering 
//v1.15				20-mar-2020			//added: 'fast_mgraph::set_xunits_perpxl()'  and 'set_yunits_perpxl()' to change away from auto rescaling (-1)
										//added: fast_mgraph::set_left_click_cb()' and 'set_left_click_cb()' and 'set_left_click_cb()'
										
//v1.16				23-dec-2020			//multiple changes in 'mgraph' and 'fast_graph' to fix zoom bug when 'x' trace did not start at zero (i.e: 'trce[ trc ].minx'!= 0.0),
										//it was causing annoying x-axis offset, most changes commented with 'v1.16'
										//also added: 'en_dobt_polypoints_wnd_coords', 'en_dobt_polyline_wnd_coords', 'en_dobt_polygon_wnd_coords', 'en_dobt_polygonf_wnd_coords'

//v1.17				17-oct-2021			//added return value to 'mgraph::rect_clip()', was causing a crash when 'O3' optimization is used in LLEM prj with audio callback running

//v1.18				16-dec-2022			//added public var 'mousewheel' and 'b_invert_wheel'

//v1.19				07-jan-2023			//improved 'fast_mgraph' to allow persistent selection, zoom and y pos adj between repeated trace plots, added 'm' key for y-axis scaling also,
										//added 'set_ymax_deflection_value()',
										//removed and simplified many plot functions, added 'set_col_bkgd()', 'set_col_axis()' etc...to reduce call variable count,
										//now only takes vectors of floats or doubles, see e.g: 'plot_vfloat_2()'   'plotxy_vfloat_2()'    'plotxy_vdouble_2()'   etc,
										//added 'xtick_marks[]' and  'ytick_marks[]',
										//added 'b_x_axis_values_derived[]' to allow user to define 'x' axis labelling values, only works when no 'x' vector is supplied
										
//v1.20				16-apr-2023			//added 'vector<surfaceplot_text_obj_tag> vtxt' to 'test_surface3d()'
//v1.21				17-sep-2023			//added 'b_bring_window_to_front' to 'fast_mgraph', used in 'cb_fast_gph_mouseenter()', now 'bring to front' is by default off

//v1.22				14-oct-2023			//added 's' key option to select first sample on first trace if nothing is currently selected, see:  'keys[]'
										//fixed x/y scale ticks marks changing size when trace amplitude was change by user, see 'fscly_tickx'
										//added 'mgraph::get_mouse( int &mx, int &my )'
										//moded 'update_fg_user_obj()' to improve 'fast_mgraph' x/y tick display values and hover values when 'a' 'm' 'y' adj have been made, see 'fscly_tickx', 'fscly_tickx'
										//added 'sample_rect_showing[cn_mgraph_trace_max]' which is toggled by double clicking if 'trace_tag.sample_rect_hints' is set
										//added 'trace_tag.sample_rect_flicker' flag 'mgraph' trace struct, shows momentary flicking of sample rect hinting when mouse is moved, not that useful
										//added 'get_pixel_position_as_trc_values()'
										//improved sample selection operations, now only a right click anywhere deselects
										//added menu 'Sample' to 'fast_mgraph' to allow recording and recall of 4 previous plots, see 'b_prev_plot_recording'
										//added 'fit_plot()' to 'fast_mgraph'

#include "mgraph.h"





//--------------------------------------------------------------------------
//surface3d
//v1.02     30-sep-2014         //v1.02
//v1.03     31-mar-2015         //added crude user line obj support: surfaceplot_line_obj_tag
//v1.04     02-nov-2016         //added text obj support via: 'surfaceplot_text_obj_tag', text coords are relative to vals passed to build_surface2(..)
//								//build_surface2(..) takes values of: x, y, val (val = height), the x, y vals are to determine text location for given text coords

//v1.05     06-jan-2018         //nan will cause std::sort to crash, force eps as done below above in: build_surface2()  'fpclassify( ampl ) == FP_NAN'
								//added wheel_gain user adj, adjs sample amplitudes (z-axis), hold ctrl and shift keys down,
								//added text_font, int text_size
								//see also in the 'mgraph.h' warning:  'to avoid windows error dlg: fl_line_style(): Could not create GDI pen object'


//--------------------------------------------------------------------------
//consecutive x locations are dim_y locations apart, like below:
//e.g: ptr = x * dim_y + y;   o[ ptr ] = val;






/*
void test_surface3d()
{
int dimx, dimy;
dimx = 64;
dimy = 64;


//dimx = cn_pixels_to_plot * cn_filter_sub_samples * cn_reso;
//dimy = cn_pixels_to_plot * cn_filter_sub_samples * cn_reso;


vector<double>o;
o.resize( dimx * dimy );
int sub_pixels = 8;

plt3d->scale = 3000;
plt3d->ampl_gain = 1.0;
plt3d->highlight_every_x = sub_pixels;
plt3d->highlight_every_y = sub_pixels;
plt3d->offy = 60;



surfaceplot_line_obj_tag loo;
vector<surfaceplot_line_obj_tag> lo;
vector<surfaceplot_text_obj_tag> vtxt;									//v1.20

for( int yy = 0; yy < dimy; yy++ )              //zero grid val
    {
    for( int xx = 0; xx < dimx; xx++ )
        {
        int ptr = yy + dimy * xx;               //note xx is multiples of: yy * dimy
        o[ ptr ] = 0.0;
        }
    }


for( int yy = 32; yy < 32 + 8; yy++ )
    {
    for( int xx = 32; xx < 32 + 8; xx++ )
        {
        int ptr = yy + dimy * xx;
        o[ ptr ] = 0.1;
        }
    }


int ptr = 0 + dimy * 0;;                    //note x is multiples of: y * dimy
o[ ptr ] = 0.05;


ptr = 8 + dimy * 0;
o[ ptr ] = 0.05;

ptr = 0 + dimy * 16;
o[ ptr ] = 0.05;


ptr = 36 + dimy * 36;
o[ ptr ] = 0.5;


loo.x1 = -30.5;								//a line obj def
loo.y1 = -31.5;
loo.x2 = 31.0;
loo.y2 = 31.0;
loo.line_wid = 2;
loo.line_style = en_spls_solid;                 //user line obj line style, see: en_surfaceplot_line_style
loo.r = 64;
loo.g = 64;
loo.b = 64;

lo.push_back( loo );



loo.x1 = -5.5;								//a line obj def
loo.y1 = 31.5;
loo.x2 = 20.0;
loo.y2 = -31.0;
loo.line_wid = 3;
loo.line_style = en_spls_solid;                 //user line obj line style, see: en_surfaceplot_line_style
loo.r = 64;
loo.g = 64;
loo.b = 64;

lo.push_back( loo );



loo.x1 = -31.5;								//a line obj def
loo.y1 = 2.0;
loo.x2 = 31.5;
loo.y2 = 2.0;
loo.line_wid = 2;
loo.line_style = en_spls_solid;                 //user line obj line style, see: en_surfaceplot_line_style
loo.r = 128;
loo.g = 128;
loo.b = 128;

lo.push_back( loo );




loo.x1 = 2.0;								//a line obj def
loo.y1 = -31.5;
loo.x2 = 2.0;
loo.y2 = 31.5;
loo.line_wid = 2;
loo.line_style = en_spls_solid;                 //user line obj line style, see: en_surfaceplot_line_style
loo.r = 255;
loo.g = 0;
loo.b = 0;

lo.push_back( loo );



plt3d->grid_line_style = en_spls_solid;              //grid line style, see: en_surfaceplot_line_style
plt3d->grid_line_wid = 1;                            //grid line width

plt3d->build_surface( o, dimx, dimy, 1, 1, lo, vtxt );

}
*/












surface3d::surface3d( int x, int y, int w, int h, const char *label ) : Fl_Double_Window( x, y, w, h, label )
{
text_font = 4;
text_size = 11;

left_button = 0;
middle_button = 0;
right_button = 0;

ctrl_key = 0;
shift_key = 0;
std_wheel_step = 1;
ctrl_wheel_step = 1;
shift_wheel_step = 1;


show_xyz_gain_text = 1;

show_lines = 1;
show_filled = 1;

scale = 1000;
scalex = 1;
scaley = 1;

force_col_fill = 0;
bdotmark = 1;

offx = 0;
offy = -100;
dimx = 0;
dimy = 0;

grid_line_style = en_spls_solid;              //grid line style, see: en_surfaceplot_line_style
grid_line_wid = 1;                            //grid line width


selected_id = -1;

normals_length = 0.1;

rotate_x = 15;
rotate_y = 0;
rotate_z = -15;

lb_press_x = 0;
lb_press_y = 0;
mb_press_x = 0;
mb_press_y = 0;
rb_press_x = 0;
rb_press_y = 0;
mousex = 0;
mousey = 0;


rotx = 0;
roty = 0;
rotz = 0;
rho = 15;
ampl_gain = 1.0;
wheel_gain = 1.0;

col_bkgd.r = 255;
col_bkgd.g = 255;
col_bkgd.b = 255;

col_line.r = 0;
col_line.g = 0;
col_line.b = 0;

col_fill.r = 255;
col_fill.g = 255;
col_fill.b = 255;

highlight_every_x = 0;
highlight_every_y = 0;

col_hightlight_x.r = 0;
col_hightlight_x.g = 255;
col_hightlight_x.b = 0;

col_hightlight_y.r = 255;
col_hightlight_y.g = 0;
col_hightlight_y.b = 0;

//matrix_ffilter_widthormula_gen();


//set up colour ramp values for z axis contour colouring, see dissolve_ramp(..)
col_ramp[ 9 ].r = 255;					//red
col_ramp[ 9 ].g = 29;
col_ramp[ 9 ].b = 0;

col_ramp[ 8 ].r = 255;
col_ramp[ 8 ].g = 128;
col_ramp[ 8 ].b = 0;

col_ramp[ 7 ].r = 255;					//yellow
col_ramp[ 7 ].g = 255;
col_ramp[ 7 ].b = 0;

col_ramp[ 6 ].r = 128;
col_ramp[ 6 ].g = 255;
col_ramp[ 6 ].b = 0;

col_ramp[ 5 ].r = 0;					//green
col_ramp[ 5 ].g = 255;
col_ramp[ 5 ].b = 0;

col_ramp[ 4 ].r = 0;
col_ramp[ 4 ].g = 255;				
col_ramp[ 4 ].b = 128;

col_ramp[ 3 ].r = 0;					//cyan
col_ramp[ 3 ].g = 255;					
col_ramp[ 3 ].b = 255;

col_ramp[ 2 ].r = 0;
col_ramp[ 2 ].g = 128;
col_ramp[ 2 ].b = 255;

col_ramp[ 1 ].r = 0;
col_ramp[ 1 ].g = 0;
col_ramp[ 1 ].b = 255;					//blue

col_ramp[ 0 ].r = 142;
col_ramp[ 0 ].g = 0;
col_ramp[ 0 ].b = 255;

}


struct matrix_tag
{
string s11, s12, s13, s14;
string s21, s22, s23, s24;
string s31, s32, s33, s34;
string s41, s42, s43, s44;

};


void mult_matrix( matrix_tag &res, matrix_tag &a, matrix_tag &b )
{
res.s11 = a.s11 + b.s11;
res.s11 += " + ";
res.s11 += a.s12 + b.s21;
res.s11 += " + ";
res.s11 += a.s13 + b.s31;
res.s11 += " + ";
res.s11 += a.s14 + b.s41;

res.s21 = a.s11 + b.s12;
res.s21 += " + ";
res.s21 += a.s12 + b.s22;
res.s21 += " + ";
res.s21 += a.s13 + b.s32;
res.s21 += " + ";
res.s21 += a.s14 + b.s42;

res.s31 = a.s11 + b.s13;
res.s31 += " + ";
res.s31 += a.s12 + b.s23;
res.s31 += " + ";
res.s31 += a.s13 + b.s33;
res.s31 += " + ";
res.s31 += a.s14 + b.s43;

res.s41 = a.s11 + b.s14;
res.s41 += " + ";
res.s41 += a.s12 + b.s24;
res.s41 += " + ";
res.s41 += a.s13 + b.s34;
res.s41 += " + ";
res.s41 += a.s14 + b.s44;
//---------------------

res.s12 = a.s21 + b.s11;
res.s12 += " + ";
res.s12 += a.s22 + b.s21;
res.s12 += " + ";
res.s12 += a.s23 + b.s31;
res.s12 += " + ";
res.s12 += a.s24 + b.s41;

res.s22 = a.s21 + b.s12;
res.s22 += " + ";
res.s22 += a.s22 + b.s22;
res.s22 += " + ";
res.s22 += a.s23 + b.s32;
res.s22 += " + ";
res.s22 += a.s24 + b.s42;

res.s32 = a.s21 + b.s13;
res.s32 += " + ";
res.s32 += a.s22 + b.s23;
res.s32 += " + ";
res.s32 += a.s23 + b.s33;
res.s32 += " + ";
res.s32 += a.s24 + b.s43;

res.s42 = a.s21 + b.s14;
res.s42 += " + ";
res.s42 += a.s22 + b.s24;
res.s42 += " + ";
res.s42 += a.s23 + b.s34;
res.s42 += " + ";
res.s42 += a.s24 + b.s44;
//---------------------

res.s13 = a.s31 + b.s11;
res.s13 += " + ";
res.s13 += a.s32 + b.s21;
res.s13 += " + ";
res.s13 += a.s33 + b.s31;
res.s13 += " + ";
res.s13 += a.s34 + b.s41;

res.s23 = a.s31 + b.s12;
res.s23 += " + ";
res.s23 += a.s32 + b.s22;
res.s23 += " + ";
res.s23 += a.s33 + b.s32;
res.s23 += " + ";
res.s23 += a.s34 + b.s42;

res.s33 = a.s31 + b.s13;
res.s33 += " + ";
res.s33 += a.s32 + b.s23;
res.s33 += " + ";
res.s33 += a.s33 + b.s33;
res.s33 += " + ";
res.s33 += a.s34 + b.s43;

res.s43 = a.s31 + b.s14;
res.s43 += " + ";
res.s43 += a.s32 + b.s24;
res.s43 += " + ";
res.s43 += a.s33 + b.s34;
res.s43 += " + ";
res.s43 += a.s34 + b.s44;
//---------------------



res.s14 = a.s41 + b.s11;
res.s14 += " + ";
res.s14 += a.s42 + b.s21;
res.s14 += " + ";
res.s14 += a.s43 + b.s31;
res.s14 += " + ";
res.s14 += a.s44 + b.s41;

res.s24 = a.s41 + b.s12;
res.s24 += " + ";
res.s24 += a.s42 + b.s22;
res.s24 += " + ";
res.s24 += a.s43 + b.s32;
res.s24 += " + ";
res.s24 += a.s44 + b.s42;

res.s34 = a.s41 + b.s13;
res.s34 += " + ";
res.s34 += a.s42 + b.s23;
res.s34 += " + ";
res.s34 += a.s43 + b.s33;
res.s34 += " + ";
res.s34 += a.s44 + b.s43;

res.s44 = a.s41 + b.s14;
res.s44 += " + ";
res.s44 += a.s42 + b.s24;
res.s44 += " + ";
res.s44 += a.s43 + b.s34;
res.s44 += " + ";
res.s44 += a.s44 + b.s44;
//---------------------
}





//these formulae where derived with help of string manipulation function: matrix_formula_gen()
//they were generated by multiplying the 3 axis rotation matrices by each other
void surface3d::matrix_formula_gen()
{
matrix_tag a;
matrix_tag b;
matrix_tag c;
matrix_tag res;

//rotate around x-axis
a.s11 = "1.";
a.s12 = "0.";
a.s13 = "0.";
a.s14 = "0.";

a.s21 = "0.";
a.s22 = "cosx.";
a.s23 = "-sinx.";
a.s24 = "0.";

a.s31 = "0.";
a.s32 = "sinx.";
a.s33 = "cosx.";
a.s34 = "0.";

a.s41 = "0.";
a.s42 = "0.";
a.s43 = "0.";
a.s44 = "1.";

//rotate around y-axis
b.s11 = "cosy.";
b.s12 = "0.";
b.s13 = "siny.";
b.s14 = "0.";

b.s21 = "0.";
b.s22 = "1.";
b.s23 = "0.";
b.s24 = "0.";

b.s31 = "-siny.";
b.s32 = "0.";
b.s33 = "cosy.";
b.s34 = "0.";

b.s41 = "0.";
b.s42 = "0.";
b.s43 = "0.";
b.s44 = "1.";

//Rotation x-axis by y-axis (combined)
a.s11 = "cosy.";
a.s12 = "0.";
a.s13 = "siny.";
a.s14 = "0.";

a.s21 = "-sinx-siny.";
a.s22 = "cosx.";
a.s23 = "-sinxcosy.";
a.s24 = "0.";

a.s31 = "cosx-siny.";
a.s32 = "sinx.";
a.s33 = "cosxcosy.";
a.s34 = "0.";

a.s41 = "0.";
a.s42 = "0.";
a.s43 = "0.";
a.s44 = "1.";


//rotate around z-axis
b.s11 = "cosz.";
b.s12 = "-sinz.";
b.s13 = "0.";
b.s14 = "0.";

b.s21 = "sinz.";
b.s22 = "cosz.";
b.s23 = "0.";
b.s24 = "0.";

b.s31 = "0.";
b.s32 = "0.";
b.s33 = "1.";
b.s34 = "0.";

b.s41 = "0.";
b.s42 = "0.";
b.s43 = "0.";
b.s44 = "1.";


mult_matrix( res, a, b );


printf( "-------------------------------------------------------------\n");
printf( "matrix a11= %s\n", res.s11.c_str() );
printf( "matrix a21= %s\n", res.s21.c_str() );
printf( "matrix a31= %s\n", res.s31.c_str() );
printf( "matrix a41= %s\n", res.s41.c_str() );
printf( "\n");
printf( "matrix a12= %s\n", res.s12.c_str() );
printf( "matrix a22= %s\n", res.s22.c_str() );
printf( "matrix a32= %s\n", res.s32.c_str() );
printf( "matrix a42= %s\n", res.s42.c_str() );
printf( "\n");
printf( "matrix a12= %s\n", res.s13.c_str() );
printf( "matrix a22= %s\n", res.s23.c_str() );
printf( "matrix a32= %s\n", res.s33.c_str() );
printf( "matrix a42= %s\n", res.s43.c_str() );
printf( "\n");
printf( "matrix a12= %s\n", res.s14.c_str() );
printf( "matrix a22= %s\n", res.s24.c_str() );
printf( "matrix a32= %s\n", res.s34.c_str() );
printf( "matrix a42= %s\n", res.s44.c_str() );
printf( "-------------------------------------------------------------\n");
}









//translate, rotate, pespective divide spec vsp entry and place vzorder vector
void surface3d::trans_rotate_persp_obj( vector<st_surfplot_tag> &vv, vector<st_surfplot_tag> &vt, int ptr )
{
double xt, yt, zt, x1, y1 ,z1;
int xx, yy;

double scale_x = scalex * scale;
double scale_y = scaley * scale;


vt[ ptr ].valid = 0;			//assume cannot plot this


//these formulae where derived with help of string manipulation function: matrix_formula_gen()
//they were generated by mutiplying the 3 axis rotation matrices by each other
//----------------------
//rotate x-axis and y-axis
//xt = x1 * croty +                             z1 * sroty;
//yt = x1 * -srotx * -sroty  +  y1 * crotx  +   z1 * -srotx * croty;
//zt = x1 * crotx * -sroty   +  y1 * srotx  +   z1 * crotx * croty;
//----------------------




//------------------------------------------------
x1 = vv[ ptr ].x1;
y1 = vv[ ptr ].y1;
z1 = vv[ ptr ].z1;


//----------------------
//tanslate, rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;
//----------------------


if( zt == 0 ) zt = 1e-6;
zt = -zt + rho;
if( zt < 0 ) return;

vt[ ptr ].x1 = xt;                      //store rotated coords without perpective divide
vt[ ptr ].y1 = yt;
vt[ ptr ].z1 = zt;


xt = (float)scale_x * xt / zt;          //pespective divide/projection
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

vt[ ptr ].ix1 = xx;
vt[ ptr ].iy1 = yy;


//------------------------------------------------






//------------------------------------------------
x1 = vv[ ptr ].x2;
y1 = vv[ ptr ].y2;
z1 = vv[ ptr ].z2;


//----------------------
//translate, rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;
//----------------------


if( zt == 0 ) zt = 1e-6;
zt = -zt + rho;
if( zt < 0 ) return;

vt[ ptr ].x2 = xt;                      //store rotated coords without perpective divide
vt[ ptr ].y2 = yt;
vt[ ptr ].z2 = zt;


xt = (float)scale_x * xt / zt;          //pespective divide/projection
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

vt[ ptr ].ix2 = xx;
vt[ ptr ].iy2 = yy;
//------------------------------------------------






//------------------------------------------------
x1 = vv[ ptr ].x3;
y1 = vv[ ptr ].y3;
z1 = vv[ ptr ].z3;


//----------------------
//translate, rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;
//----------------------



if( zt == 0 ) zt = 1e-6;
zt = -zt + rho;
if( zt < 0 ) return;

vt[ ptr ].x3 = xt;                      //store rotated coords without perpective divide
vt[ ptr ].y3 = yt;
vt[ ptr ].z3 = zt;


xt = (float)scale_x * xt / zt;          //pespective divide/projection
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

vt[ ptr ].ix3 = xx;
vt[ ptr ].iy3 = yy;
//------------------------------------------------





//------------------------------------------------
x1 = vv[ ptr ].x4;
y1 = vv[ ptr ].y4;
z1 = vv[ ptr ].z4;


//----------------------
//translate, rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;
//----------------------

if( zt == 0 ) zt = 1e-6;
zt = -zt + rho;
if( zt < 0 ) return;

vt[ ptr ].x4 = xt;                      //store rotated coords without perpective divide
vt[ ptr ].y4 = yt;
vt[ ptr ].z4 = zt;


xt = (float)scale_x * xt / zt;          //pespective divide/projection
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

vt[ ptr ].ix4 = xx;
vt[ ptr ].iy4 = yy;
//------------------------------------------------



//vzorder[ ptr ].colfill = colfill;
//vzorder[ ptr ].colline = colline;

vt[ ptr ].valid = 1;				        //mark as plotable

vt[ ptr ].options = vv[ ptr ].options;
vt[ ptr ].col_select_1 = vv[ ptr ].col_select_1;
vt[ ptr ].col_select_2 = vv[ ptr ].col_select_2;
vt[ ptr ].col_select_3 = vv[ ptr ].col_select_3;




//double centrex, centrey, centrez;
//find_centre( vt[ ptr ], centrex, centrey, centrez );
//vt[ ptr ].centrex = centrex;
//vt[ ptr ].centrey = centrey;
//vt[ ptr ].centrez = centrez;


}








void surface3d::trans_rotate_project( vector<st_surfplot_tag> &vv, vector<st_surfplot_tag> &vt, int ptr )
{
//                trans_rotate_persp_obj( vv, vt, ptr  );
trans_rotate( vv, vt, ptr );
perpective_proj( vt, ptr );
}











//translate, rotate spec vsp entry
void surface3d::trans_rotate( vector<st_surfplot_tag> &vv, vector<st_surfplot_tag> &vt, int ptr )
{
double xt, yt, zt, x1, y1 ,z1;
int xx, yy;



//these formulae where derived with help of string manipulation function: matrix_formula_gen()
//they were generated by mutiplying the 3 axis rotation matrices by each other
//----------------------
//rotate x-axis and y-axis
//xt = x1 * croty +                             z1 * sroty;
//yt = x1 * -srotx * -sroty  +  y1 * crotx  +   z1 * -srotx * croty;
//zt = x1 * crotx * -sroty   +  y1 * srotx  +   z1 * crotx * croty;
//----------------------



//------------------------------------------------
x1 = vv[ ptr ].x1;
y1 = vv[ ptr ].y1;
z1 = vv[ ptr ].z1;

//----------------------
//translate, rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;
//----------------------

if( zt == 0 ) zt = 1e-6;                //offset z axis by rho
zt = -zt + rho;

vt[ ptr ].x1 = xt;                      //store rotated coords without perpective divide
vt[ ptr ].y1 = yt;
vt[ ptr ].z1 = zt;

//------------------------------------------------






//------------------------------------------------
x1 = vv[ ptr ].x2;
y1 = vv[ ptr ].y2;
z1 = vv[ ptr ].z2;


//----------------------
//translate, rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;
//----------------------

if( zt == 0 ) zt = 1e-6;                //offset z axis by rho
zt = -zt + rho;

vt[ ptr ].x2 = xt;                      //store rotated coords without perpective divide
vt[ ptr ].y2 = yt;
vt[ ptr ].z2 = zt;

//------------------------------------------------






//------------------------------------------------
x1 = vv[ ptr ].x3;
y1 = vv[ ptr ].y3;
z1 = vv[ ptr ].z3;

//----------------------
//translate, rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;
//----------------------

if( zt == 0 ) zt = 1e-6;                //offset z axis by rho
zt = -zt + rho;

vt[ ptr ].x3 = xt;                      //store rotated coords without perpective divide
vt[ ptr ].y3 = yt;
vt[ ptr ].z3 = zt;

//------------------------------------------------





//------------------------------------------------
x1 = vv[ ptr ].x4;
y1 = vv[ ptr ].y4;
z1 = vv[ ptr ].z4;


//----------------------
//translate, rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;
//----------------------

if( zt == 0 ) zt = 1e-6;                //offset z axis by rho
zt = -zt + rho;

vt[ ptr ].x4 = xt;                      //store rotated coords without perpective divide
vt[ ptr ].y4 = yt;
vt[ ptr ].z4 = zt;

//------------------------------------------------



//vzorder[ ptr ].colfill = colfill;
//vzorder[ ptr ].colline = colline;


vt[ ptr ].options = vv[ ptr ].options;
vt[ ptr ].col_select_1 = vv[ ptr ].col_select_1;
vt[ ptr ].col_select_2 = vv[ ptr ].col_select_2;
vt[ ptr ].col_select_3 = vv[ ptr ].col_select_3;




//double centrex, centrey, centrez;
//find_centre( vt[ ptr ], centrex, centrey, centrez );
//vt[ ptr ].centrex = centrex;
//vt[ ptr ].centrey = centrey;
//vt[ ptr ].centrez = centrez;

//scale_surface_translated( vt[ ptr ], 0.2, 0.2 );

//make_point_at( vt[ ptr ], 0.2, en_cw_south_west );

calc_normals( vt[ ptr ] );

if( vt[ ptr ].id == 1001 )
    {
//    printf( "id %04d: nx: %g, ny: %g, nz: %g\n", vt[ ptr ].id,  vt[ ptr ].nx, vt[ ptr ].ny, vt[ ptr ].nz );
    }

if( vt[ ptr ].id == 1082 )
    {
//    printf( "id %04d: z1: %g, z2: %g, z3: %g, z4: %g, nx: %g, ny: %g, nz: %g\n", vt[ ptr ].id, vt[ ptr ].z1, vt[ ptr ].z2, vt[ ptr ].z3, vt[ ptr ].z4, vt[ ptr ].nx, vt[ ptr ].ny, vt[ ptr ].nz );
    }

if( vt[ ptr ].id == 1151 )
    {
    
//    printf( "id %04d: z1: %g, z2: %g, z3: %g, z4: %g, nx: %g, ny: %g, nz: %g\n", vt[ ptr ].id, vt[ ptr ].z1, vt[ ptr ].z2, vt[ ptr ].z3, vt[ ptr ].z4, vt[ ptr ].nx, vt[ ptr ].ny, vt[ ptr ].nz  );
    }



//make_point_at( vt[ ptr ], 0.8, en_cw_south_west );
}












void surface3d::make_point( double x1, double y1, double z1, double x2, double y2, double z2, double delta, double &xx, double &yy, double &zz )
{
xx = x1 + delta * ( x2 - x1 );
yy = y1 + delta * ( y2 - y1 );
zz = z1 + delta * ( z2 - z1 );
}




//make a new point at a distance from specified corner, proportion should be bewteen 0.0->1.0, 
//if proportion 0.0 = point at spec corner ( like: x1, y1, z1 )
//if proportion 1.0 = point at next clock wise corner  ( like: x2, y2, z2 )

void surface3d::make_point_from_corner( st_surfplot_tag &vt1, double proportion, en_which_corner_tag which )
{
double x1, y1, z1, x2, y2, z2;
double xx, yy, zz;


if( which == en_cw_south_west )
    {
    x1 = vt1.x1;
    y1 = vt1.y1;
    z1 = vt1.z1;

    x2 = vt1.x2;
    y2 = vt1.y2;
    z2 = vt1.z2;

    make_point( x1, y1, z1, x2, y2, z2, proportion, xx, yy, zz );

    vt1.x1 = xx;
    vt1.y1 = yy;
    vt1.z1 = zz;
    }

if( which == en_cw_south_east )
    {
    x1 = vt1.x2;
    y1 = vt1.y2;
    z1 = vt1.z2;

    x2 = vt1.x3;
    y2 = vt1.y3;
    z2 = vt1.z3;

    make_point( x1, y1, z1, x2, y2, z2, proportion, xx, yy, zz );

    vt1.x2 = xx;
    vt1.y2 = yy;
    vt1.z2 = zz;
    }


if( which == en_cw_north_east )
    {
    x1 = vt1.x3;
    y1 = vt1.y3;
    z1 = vt1.z3;

    x2 = vt1.x4;
    y2 = vt1.y4;
    z2 = vt1.z4;

    make_point( x1, y1, z1, x2, y2, z2, proportion, xx, yy, zz );

    vt1.x3 = xx;
    vt1.y3 = yy;
    vt1.z3 = zz;
    }

if( which == en_cw_north_west )
    {
    x1 = vt1.x4;
    y1 = vt1.y4;
    z1 = vt1.z4;

    x2 = vt1.x1;
    y2 = vt1.y1;
    z2 = vt1.z1;

    make_point( x1, y1, z1, x2, y2, z2, proportion, xx, yy, zz );

    vt1.x4 = xx;
    vt1.y4 = yy;
    vt1.z4 = zz;
    }

}






//3d->2d scaling and perspective projection of spec translated 3d obj
void surface3d::perpective_proj( vector<st_surfplot_tag> &vt, int ptr )
{
double xt, yt, zt;
int xx, yy;

double scale_x = scalex * scale;
double scale_y = scaley * scale;

vt[ ptr ].valid = 0;			//assume cannot plot this


//-----------------------
xt = vt[ ptr ].x1;
yt = vt[ ptr ].y1;
zt = vt[ ptr ].z1;


if( zt < 0 ) return;


xt = (float)scale_x * xt / zt;          //pespective divide/projection
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

vt[ ptr ].ix1 = xx;
vt[ ptr ].iy1 = yy;
//-----------------------



//-----------------------
xt = vt[ ptr ].x2;
yt = vt[ ptr ].y2;
zt = vt[ ptr ].z2;


if( zt < 0 ) return;


xt = (float)scale_x * xt / zt;          //pespective divide/projection
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

vt[ ptr ].ix2 = xx;
vt[ ptr ].iy2 = yy;
//-----------------------


//-----------------------
xt = vt[ ptr ].x3;
yt = vt[ ptr ].y3;
zt = vt[ ptr ].z3;


if( zt < 0 ) return;


xt = (float)scale_x * xt / zt;          //pespective divide/projection
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

vt[ ptr ].ix3 = xx;
vt[ ptr ].iy3 = yy;
//-----------------------




//-----------------------
xt = vt[ ptr ].x4;
yt = vt[ ptr ].y4;
zt = vt[ ptr ].z4;


if( zt < 0 ) return;


xt = (float)scale_x * xt / zt;          //pespective divide/projection
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

vt[ ptr ].ix4 = xx;
vt[ ptr ].iy4 = yy;
//-----------------------







//-------- centre and normals projection --------

double mx, my, mz;
//make_point( vt[ ptr ].x1, vt[ ptr ].y1, vt[ ptr ].z1, vt[ ptr ].x3, vt[ ptr ].y3, vt[ ptr ].z3, 0.5, mx, my, mz );


get_centre( vt[ ptr ], mx, my, mz );

vt[ ptr ].centrex = mx;
vt[ ptr ].centrey = my;
vt[ ptr ].centrez = mz;

xt = vt[ ptr ].centrex;
yt = vt[ ptr ].centrey;
zt = vt[ ptr ].centrez;

if( vt[ ptr ].id == 1001 )
    {
//    printf( "id %04d: nx: %g, ny: %g, nz: %g\n", vt[ ptr ].id,  vt[ ptr ].nx, vt[ ptr ].ny, vt[ ptr ].nz );
    }



xt = (float)scale_x * xt / zt;          //pespective divide/projection of centre
yt = (float)scale_y * yt / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );


vt[ ptr ].icx = xx;						//store projected 2d centre coords
vt[ ptr ].icy = yy;

xt = vt[ ptr ].nx;                      //do normals as well
yt = vt[ ptr ].ny;
//zt = vt[ ptr ].nz;

//if( zt < 0 ) return;

xt = (float)scale_x * xt * normals_length / zt;          //pespective divide/projection using z from centre of obj
yt = (float)scale_y * yt * normals_length / zt;

xx = nearbyint( xt );
yy = nearbyint( yt );

//calc angle of face relative to observer
//0 degrees means obj facing the observer, +90 obj facing upward, -90 means obj facing downward
double angle;

double zz = vt[ ptr ].nz;
if( zz == 0.0 ) zz = 1e-6;
angle = atan2( -vt[ ptr ].ny , vt[ ptr ].nz );
angle *= 180 / M_PI;
vt[ ptr ].face_angle = angle;


if( vt[ ptr ].id == 1151 )
    {
//	printf( "angle: %g\n", angle );
    }



vt[ ptr ].inx = xx;
vt[ ptr ].iny = yy;

//-----------------------




vt[ ptr ].valid = 1;				        //mark as plotable

}
















//set trig values for rotation around x, y, z axis
void surface3d::set_translate_vals( double rx, double ry, double rz )
{

srotx = sin( cn_surfplot_deg2rad * ( rotx + rx - 90.0 ) );
crotx = cos( cn_surfplot_deg2rad * ( rotx + rx - 90.0 ) );

sroty = sin( cn_surfplot_deg2rad * ( roty + ry ) );
croty = cos( cn_surfplot_deg2rad * ( roty + ry ) );

srotz = sin( cn_surfplot_deg2rad * ( rotz + rz - 0 ) );
crotz = cos( cn_surfplot_deg2rad * ( rotz + rz - 0 ) );
}




/*


void surface3d::translate( double &x1, double &y1, double &z1 )
{
double xt, yt, zt;

//double rotate = M_PI * .05;
//double tilt = 0;

//bool negative = 0;
//if ( z1 < 0  ) negative = 1;




xt = x1;
yt = y1;
zt = z1;

//these formulae where derived with help of string manipulation function: matrix_formula_gen()
//they were generated by mutiplying the 3 axis rotation matrices by each other
//----------------------
//rotate x-axis and y-axis
//xt = x1 * croty +                             z1 * sroty;
//yt = x1 * -srotx * -sroty  +  y1 * crotx  +   z1 * -srotx * croty;
//zt = x1 * crotx * -sroty   +  y1 * srotx  +   z1 * crotx * croty;
//----------------------

//----------------------
//rotate x-axis and y-axis and z-axiz
xt = x1 * croty * crotz + y1 * croty * -srotz + z1 * sroty;
yt = x1 * -srotx * -sroty * crotz + x1 * crotx * srotz + y1 * -srotx * -sroty * -srotz + y1 * crotx * crotz + z1 * -srotx * croty;
zt = x1 * crotx * -sroty * crotz + x1 * srotx * srotz + y1 * crotx * -sroty * -srotz + y1 * srotx * crotz + z1 * crotx * croty;

//----------------------


//yt = y1 * crotx - z1 * srotx;		//rotate around x-axis
//zt = y1 * srotx + z1 * crotx;

//y1 = yt;
//z1 = zt;

//xt = x1 * croty + z1 * sroty;		//rotate around y-axis
//zt = -x1 * sroty + z1 * croty;

//x1 = xt;
//z1 = zt;

//xt = x1 * crotz - y1 * srotz;		//rotate around z-axis
//yt = x1 * srotz + y1 * crotz;

x1 = xt;
y1 = yt;
z1 = zt;

//if( negative ) z1 = -z1;

//if ( z1 < 0 ) z1 = -z1; 

}


*/






//return compare of z value between s1 and s2 
static int cmp_zaxis(const st_surfplot_tag &s1, const st_surfplot_tag &s2 )
{
double avg1 = s1.z1 + s1.z2 + s1.z3 + s1.z4;
double avg2 = s2.z1 + s2.z2 + s2.z3 + s2.z4;




if( avg1 <= avg2 ) return 0;

//if( s1.face_angle > s2.face_angle) return 0;

return 1;
}







//return compare of z value between s1 and s2 
static int cmp_zaxis3(const st_surfplot_tag &s1, const st_surfplot_tag &s2 )
{

if( s1.centrez <= s2.centrez ) return 0;

return 1;
}






//return compare of z value between s1 and s2 
static int cmp_zaxis2(const st_surfplot_tag &s1, const st_surfplot_tag &s2)
{


//printf("\nSort B->S S1: %20jd %s\n",(intmax_t)s1.size,s1.str.c_str());
//printf("\nSort S2: %20jd %s\n",(intmax_t)s2.size,s2.str.c_str());

double z1_max = s1.z1;
double z2_max = s2.z1;

if( s1.z2 > z1_max ) z1_max = s1.z2;	//find lowest z value
if( s1.z3 > z1_max ) z1_max = s1.z3;
if( s1.z4 > z1_max ) z1_max = s1.z4;


if( s2.z2 > z2_max ) z2_max = s2.z2;	//find lowest z value
if( s2.z3 > z2_max ) z2_max = s2.z3;
if( s2.z4 > z2_max ) z2_max = s2.z4;

if( z1_max == z2_max ) return 0;
if( z1_max < z2_max ) return 0;


double avg1 = s1.z1 + s1.z2;

//if( s1.z1 == s2.z1 ) return 0;
//if( s1.z1 < s2.z1 ) return 0;
return 1;
}




/*
//return compare of z value between s1 and s2 
static int cmp_zaxis(const int &s1, const int &s2)
{

//printf("\nSort B->S S1: %20jd %s\n",(intmax_t)s1.size,s1.str.c_str());
//printf("\nSort S2: %20jd %s\n",(intmax_t)s2.size,s2.str.c_str());

if( s1 == s2 ) return 0;
if( s1 < s2 ) return 0;
else return 1;
}

*/




/*

void surface3d::plot( bool line, bool fill )
{
double div1;
double div2;

scalex = 1000;
scaley = 1000;

double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

x1 = vsp[ ptr ].x1;
y1 = vsp[ ptr ].y1;
z1 = vsp[ ptr ].z1;
x2 = vsp[ ptr ].x2;
y2 = vsp[ ptr ].y2;
z2 = vsp[ ptr ].z2;
x3 = vsp[ ptr ].x3;
y3 = vsp[ ptr ].y3;
z3 = vsp[ ptr ].z3;
x4 = vsp[ ptr ].x4;
y4 = vsp[ ptr ].y4;
z4 = vsp[ ptr ].z4;

translate( x1, y1, z1 );
translate( x2, y2, z2 );
translate( x3, y3, z3 );
translate( x4, y4, z4 );


std::sort( vsp.begin(), vsp.end(), cmp_zaxis);



if( z1 == 0 ) z1 = 1e-6;
if( z2 == 0 ) z2 = 1e-6;
if( z3 == 0 ) z3 = 1e-6;
if( z4 == 0 ) z4 = 1e-6;

z1 = z1 + rho;
z2 = z2 + rho;
z3 = z3 + rho;
z4 = z4 + rho;

if( z1 < 0 ) return;
if( z2 < 0 ) return;
if( z3 < 0 ) return;
if( z4 < 0 ) return;

x1 = (float)scalex * x1 / z1;
y1 = (float)scaley * y1 / z1;
x2 = (float)scalex * x2 / z2;
y2 = (float)scaley * y2 / z2;
x3 = (float)scalex * x3 / z3;
y3 = (float)scaley * y3 / z3;
x4 = (float)scalex * x4 / z4;
y4 = (float)scaley * y4 / z4;


int xx1, yy1, xx2, yy2, xx3, yy3, xx4, yy4;

xx1 = nearbyint( x1 );
yy1 = nearbyint( y1 );
xx2 = nearbyint( x2 );
yy2 = nearbyint( y2 );
xx3 = nearbyint( x3 );
yy3 = nearbyint( y3 );
xx4 = nearbyint( x4 );
yy4 = nearbyint( y4 );


if( fill )
	{
	fl_color( colfill.r, colfill.g, colfill.b );
	fl_begin_complex_polygon();

	fl_vertex( midx + xx1, midy - yy1 );
	fl_vertex( midx + xx2, midy - yy2 );
	fl_vertex( midx + xx3, midy - yy3 );
	fl_vertex( midx + xx4, midy - yy4 );

	fl_end_complex_polygon();
	}


if( line )
	{
	fl_color( colline.r, colline.g, colline.b );

	fl_begin_loop();

	fl_vertex( midx + xx1, midy - yy1 );
	fl_vertex( midx + xx2, midy - yy2 );
	fl_vertex( midx + xx3, midy - yy3 );
	fl_vertex( midx + xx4, midy - yy4 );

	fl_end_loop();
	}
}



*/



void surface3d::plot_obj( vector<st_surfplot_tag> &o, int ptr, bool fill, bool line, bool user_line_obj, bool user_text_obj )
{
bool highlight_south = 0;
bool highlight_east = 0;
bool highlight_west = 0;
bool highlight_north = 0;



if ( ! o[ ptr ].valid ) return;

if( o[ ptr ].options & en_po_dont_plot ) return;


//if( ( o[ ptr ].face_angle >= 90.0 ) || ( o[ ptr ].face_angle <= -90.0 ) ) return;		//backface culling, not that usefull for this application

//return;

if( fill )
	{
	//select colour
    fl_color( o[ ptr ].colfill.r, o[ ptr ].colfill.g, o[ ptr ].colfill.b );

    if( o[ ptr ].options & en_po_show_select_1 )
        {
        fl_color( o[ ptr ].col_select_1.r, o[ ptr ].col_select_1.g, o[ ptr ].col_select_1.b );
        }

    if( o[ ptr ].options & en_po_show_select_2 )
        {
        fl_color( o[ ptr ].col_select_2.r, o[ ptr ].col_select_2.g, o[ ptr ].col_select_2.b );
       }

    if( o[ ptr ].options & en_po_show_select_3 )
        {
        fl_color( o[ ptr ].col_select_3.r, o[ ptr ].col_select_3.g, o[ ptr ].col_select_3.b );
       }


    fl_line_style ( o[ ptr ].grid_line_style, o[ ptr ].grid_line_wid );         	//for windows you must set line setting after the colour, see the manual


	fl_begin_complex_polygon();

	fl_vertex( midx + offx + o[ ptr ].ix1, midy - offy - o[ ptr ].iy1 );
	fl_vertex( midx + offx + o[ ptr ].ix2, midy - offy - o[ ptr ].iy2 );
	fl_vertex( midx + offx + o[ ptr ].ix3, midy - offy - o[ ptr ].iy3 );
	fl_vertex( midx + offx + o[ ptr ].ix4, midy - offy - o[ ptr ].iy4 );

	fl_end_complex_polygon();


    if( o[ ptr ].options & en_po_show_centre_dot )
        {
        fl_color( 0, 0, 0 );
		fl_line_style ( o[ ptr ].line_style, o[ ptr ].line_wid );         //for windows you must set line setting after the colour, see the manual
        int xx = midx + offx + o[ ptr ].icx;
        int yy = midy - offy - o[ ptr ].icy;

        fl_point( xx, yy );
        }


    if( o[ ptr ].options & en_po_show_normals )
        {
        fl_color( 0, 0, 0 );
		fl_line_style ( o[ ptr ].line_style, o[ ptr ].line_wid );         //for windows you must set line setting after the colour, see the manual
        int xx = midx + offx + o[ ptr ].icx;
        int yy = midy - offy - o[ ptr ].icy;

        int nx = o[ ptr ].inx;//100;//o[ ptr ].inx;
        int ny = o[ ptr ].iny;//;//o[ ptr ].iny;
        
        fl_line( xx, yy, xx - ( nx ) , yy + ( ny ) );
        }


	}


if( o[ ptr ].hightlight_x_line == 1 ) highlight_west = 1;
if( o[ ptr ].hightlight_x_line == 2 ) highlight_east = 1;

if( o[ ptr ].hightlight_y_line == 1 ) highlight_south = 1;
if( o[ ptr ].hightlight_y_line == 2 ) highlight_north = 1;



if( line )
	{
	fl_color( o[ ptr ].colline.r, o[ ptr ].colline.g, o[ ptr ].colline.b );
    fl_line_style ( o[ ptr ].line_style, o[ ptr ].line_wid );         				//for windows you must set line setting after the colour, see the manual
	if( !highlight_west  )			//need normal colour on west side?
		{
		fl_begin_line();
		fl_vertex( midx + offx + o[ ptr ].ix4, midy - offy - o[ ptr ].iy4 );
		fl_vertex( midx + offx + o[ ptr ].ix1, midy - offy - o[ ptr ].iy1 );
		fl_end_line();
		}

	if( !highlight_east  )			//need normal colour on east side?
		{
		fl_begin_line();
		fl_vertex( midx + offx + o[ ptr ].ix2, midy - offy - o[ ptr ].iy2 );
		fl_vertex( midx + offx + o[ ptr ].ix3, midy - offy - o[ ptr ].iy3 );
		fl_end_line();
		}

	if( !highlight_south  )			//need normal colour on done_south side?
		{
		fl_begin_line();
		fl_vertex( midx + offx + o[ ptr ].ix1, midy - offy - o[ ptr ].iy1 );
		fl_vertex( midx + offx + o[ ptr ].ix2, midy - offy - o[ ptr ].iy2 );
		fl_end_line();
		}

	if( !highlight_north  )			//need normal colour on done_south side?
		{
		fl_begin_line();
		fl_vertex( midx + offx + o[ ptr ].ix3, midy - offy - o[ ptr ].iy3 );
		fl_vertex( midx + offx + o[ ptr ].ix4, midy - offy - o[ ptr ].iy4 );
		fl_end_line();
		}




	//highlighted colour
	fl_color( col_hightlight_x.r, col_hightlight_x.g, col_hightlight_x.b );
    fl_line_style ( o[ ptr ].line_style, o[ ptr ].line_wid );         				//for windows you must set line setting after the colour, see the manual
	if ( highlight_west )	//need highlight on west side?
		{
		fl_begin_line();
		fl_vertex( midx + offx + o[ ptr ].ix4, midy - offy - o[ ptr ].iy4 );
		fl_vertex( midx + offx + o[ ptr ].ix1, midy - offy - o[ ptr ].iy1 );
		fl_end_line();
		}

	if ( highlight_east )	//need highlight on east side?
		{
		fl_begin_line();
		fl_vertex( midx + offx + o[ ptr ].ix2, midy - offy - o[ ptr ].iy2 );
		fl_vertex( midx + offx + o[ ptr ].ix3, midy - offy - o[ ptr ].iy3 );
		fl_end_line();
		}


	if ( highlight_south )	//need highlight on south side?
		{
		fl_begin_line();
		fl_vertex( midx + offx + o[ ptr ].ix1, midy - offy - o[ ptr ].iy1 );
		fl_vertex( midx + offx + o[ ptr ].ix2, midy - offy - o[ ptr ].iy2 );
		fl_end_line();
		}


	if ( highlight_north )	//need highlight on north side?
		{
		fl_begin_line();
		fl_vertex( midx + offx + o[ ptr ].ix3, midy - offy - o[ ptr ].iy3 );
		fl_vertex( midx + offx + o[ ptr ].ix4, midy - offy - o[ ptr ].iy4 );
		fl_end_line();
		}

	}





if( user_line_obj )
	{

    fl_color( o[ ptr ].colline.r, o[ ptr ].colline.g, o[ ptr ].colline.b );
    fl_line_style ( o[ ptr ].line_style, o[ ptr ].line_wid );         //for windows you must set line setting after the colour, see the manual

    fl_begin_line();
    fl_vertex( midx + offx + o[ ptr ].ix1, midy - offy - o[ ptr ].iy1 );
    fl_vertex( midx + offx + o[ ptr ].ix2, midy - offy - o[ ptr ].iy2 );
    fl_end_line();
	}



if( user_text_obj )
	{
	int ffont = fl_font();
	int fsize = fl_size();
	int fsize2 = fsize;

	if( o[ ptr ].fontsize != -1 )
		{
		fsize2 = o[ ptr ].fontsize;
		}

	fl_font( ffont, fsize2 );


	if( o[ ptr ].font != -1 )
		{
		fl_font( o[ ptr ].font, fsize2 );
		}

	if( o[ ptr ].show_text_background )
		{
		fl_color( o[ ptr ].backgnd.r, o[ ptr ].backgnd.g, o[ ptr ].backgnd.b );
		fl_line_style ( o[ ptr ].line_style, o[ ptr ].line_wid );         //for windows you must set line setting after the colour, see the manual
		fl_draw( o[ ptr ].stext.c_str(), midx + offx + o[ ptr ].ix1 + 1, midy - offy - o[ ptr ].iy1 + 1 );
		}

	fl_color( o[ ptr ].foregnd.r, o[ ptr ].foregnd.g, o[ ptr ].foregnd.b );
    fl_line_style ( o[ ptr ].line_style, o[ ptr ].line_wid );         //for windows you must set line setting after the colour, see the manual
	fl_draw( o[ ptr ].stext.c_str(), midx + offx + o[ ptr ].ix1, midy - offy - o[ ptr ].iy1 );

	fl_font( ffont, fsize );

	}

}






double surface3d::sinc( double x )
{

if (x != 0)
	{

	x *= M_PI;
	return ( sin(x)/x);
	}
return 1;
}








//produce a colour proportional to predefined col table held in col_ramp
//dissolve linearly between table colours
void surface3d::dissolve_ramp( double level, mg_col_tag &col )
{
int idx = 0, idx_low;

mg_col_tag col1, col2;




double band = 0;


if( level >= 0.9 )
	{
	band = 0.9;
	idx = 9;
	goto fin;
	}

if( level >= 0.8 )
	{
	band = 0.8;
	idx = 8;
	goto fin;
	}

if( level >= 0.7 )
	{
	band = 0.7;
	idx = 7;
	goto fin;
	}

if( level >= 0.6 )
	{
	band = 0.6;
	idx = 6;
	goto fin;
	}

if( level >= 0.5 )
	{
	band = 0.5;
	idx = 5;
	goto fin;
	}

if( level >= 0.4 )
	{
	band = 0.4;
	idx = 4;
	goto fin;
	}

if( level >= 0.3 )
	{
	band = 0.3;
	idx = 3;
	goto fin;
	}

if( level >= 0.2 )
	{
	band = 0.2;
	idx = 2;
	goto fin;
	}


if( level >= 0.1 )
	{
	band = 0.1;
	idx = 1;
	}



fin:

double new_level = ( level - band ) * 10.0;

idx_low = idx - 1;

if( idx_low < 0 ) idx_low = 0;

dissolve_col( new_level, col_ramp[ idx_low ], col_ramp[ idx ], col );
 
}







//dissolve linearly between 2 cols using fader value as control
//fader = 0, produces full col_a
//fader = 0.5 produces 50% mix between col_a and col_b
//fader = 0.99, produces full col_b
void surface3d::dissolve_col( double fader, mg_col_tag &col_a, mg_col_tag &col_b, mg_col_tag &col_fader )
{
col_fader.r = ( ( 1.0 - fader ) * col_a.r ) + fader * col_b.r;
col_fader.g = ( ( 1.0 - fader ) * col_a.g ) + fader * col_b.g;
col_fader.b = ( ( 1.0 - fader ) * col_a.b ) + fader * col_b.b;
}








//#define cn_max_gridx 50
//#define cn_max_gridy 50
//#define cn_max_gridz 50

//double grid[ cn_max_gridx ][ cn_max_gridy ];

/*
struct plot_grid_tag
{
double x1, y1, z1;
double x2, y2, z2;
double x3, y3, z3;
double x4, y4, z4;

int r;
int g;
int b;
};

plot_grid_tag plot_grid[ cn_max_gridx ][ cn_max_gridy ];
*/







//loads and scales amplitudes of surface using supplied vector, accesses vector using spec dimensions,
//the scaled values are stored to a new vector in a grid format

//consecutive x locations are dim_y locations apart, like below:
//e.g: ptr = x * dim_y + y;   o[ ptr ] = val;
bool surface3d::build_surface( vector<double> &o, int dim_x, int dim_y, bool fill, bool lines, vector<surfaceplot_line_obj_tag> &lo, vector<surfaceplot_text_obj_tag> &vtxt )
{

if( dim_x < 2 ) return 0;					//need at least 2 points per dimension
if( dim_y < 2 ) return 0;					//need at least 2 points per dimension

dimx = dim_x;
dimy = dim_y;

surfx = dimx - 1;			//number of actual surfaces (rectangles) is one less than point dimensions
surfy = dimy - 1;			//number of actual surfaces (rectangles) is one less than point dimensions

show_filled = fill;
show_lines = lines;

rect_count = surfx * surfy;					//total rectangle surface count


//setup vector obj dimensions
vampl.resize( dimx * dimy );			//holds actual amplitude at a grid points
vsp.resize( surfx * surfy );			//holds 3d surface rectangles
vzorder.resize( surfx * surfy );		//holds 2d surface rects in z order for plotting

vlineseg.clear();
vlo.clear();

//make line equations for any user objs
if( lo.size() != 0 )					//user objs?
    {
//    vlineobj.resize( surfx * surfy );			//holds 3d line obj
//    vzlineobj.resize( surfx * surfy );		    //holds 2d line obj in z order for plotting

    int cnt = lo.size();
    for( int i = 0; i < cnt; i++ )		//for each user obj
        {
        double dy = lo[ i ].y2 - lo[ i ].y1;    //calc line equation:  y = mx + b
        double dx = lo[ i ].x2 - lo[ i ].x1;
        
//        lo[ i ].gradient = dy / dx;
//        lo[ i ].yintercept = lo[ i ].y1 - lo[ i ].gradient * lo[ i ].x1;

//printf( "line %d: m: %f, b: %f\n", i, lo[ i ].gradient, lo[ i ].yintercept  );

        surfaceplot_lineseg_obj_tag ooo;
        
        ooo.vlineobj.resize( dimx * dimy );
        ooo.vzlineobj.resize( dimx * dimy );
        vlineseg.push_back( ooo );                  //just allocate space at this moment
        }
        
    vlo = lo;
    }




if( bdotmark)
    {
    vdotmark.resize( surfx * surfy );       //holds 3d surface rectangles
    vzdotmark.resize( surfx * surfy );		//holds 2d surface rects in z order for plotting
    }

vampl = o;

//work out max and min amplitudes, adj gain also
valmin = 0;
valmax = 0;

for( int yy = 0; yy < dimy; yy++ )
	{
	for( int xx = 0; xx < dimx; xx++ )
		{
		int ptr = xx * dimy + yy;
		
		double ampl = o[ ptr ] * ampl_gain * wheel_gain;
		
		vampl[ ptr ] = ampl;

		if( ampl < valmin ) valmin = ampl;
		if( ampl > valmax ) valmax = ampl;
		}
	}



vtext = vtxt;

for( int i = 0; i < vtext.size(); i++ )
	{
//	vtext[ i ].x *= 0.1;
//	vtext[ i ].y *= 0.1;
	}


redraw();
return 1;
}















//loads and scales amplitudes of surface using supplied vector, accesses vector using spec dimensions,
//the scaled values are stored to a new vector in a grid format

//consecutive x locations are dim_y locations apart, like below:
//e.g: ptr = x * dim_y + y;   o[ ptr ].val = val,  also allows o[ ptr ].x, o[ ptr ].y to be specified

//nan will cause std::sort to crash, force eps as done below above in: build_surface2()
bool surface3d::build_surface2( vector<st_surfaceplot_val_tag> &o, int dim_x, int dim_y, bool fill, bool lines, vector<surfaceplot_line_obj_tag> &lo, vector<surfaceplot_text_obj_tag> &vtxt )
{

if( dim_x < 2 ) return 0;					//need at least 2 points per dimension
if( dim_y < 2 ) return 0;					//need at least 2 points per dimension

dimx = dim_x;
dimy = dim_y;

surfx = dimx - 1;			//number of actual surfaces (rectangles) is one less than point dimensions
surfy = dimy - 1;			//number of actual surfaces (rectangles) is one less than point dimensions

show_filled = fill;
show_lines = lines;

rect_count = surfx * surfy;					//total rectangle surface count


//setup vector obj dimensions
vampl.resize( dimx * dimy );			//holds actual amplitude at a grid points
vsp.resize( surfx * surfy );			//holds 3d surface rectangles
vzorder.resize( surfx * surfy );		//holds 2d surface rects in z order for plotting

vlineseg.clear();
vlo.clear();

//make line equations for any user objs
if( lo.size() != 0 )					//user objs?
    {
//    vlineobj.resize( surfx * surfy );			//holds 3d line obj
//    vzlineobj.resize( surfx * surfy );		    //holds 2d line obj in z order for plotting

    int cnt = lo.size();
    for( int i = 0; i < cnt; i++ )		//for each user obj
        {
        double dy = lo[ i ].y2 - lo[ i ].y1;    //calc line equation:  y = mx + b
        double dx = lo[ i ].x2 - lo[ i ].x1;
        
//        lo[ i ].gradient = dy / dx;
//        lo[ i ].yintercept = lo[ i ].y1 - lo[ i ].gradient * lo[ i ].x1;

//printf( "line %d: m: %f, b: %f\n", i, lo[ i ].gradient, lo[ i ].yintercept  );

        surfaceplot_lineseg_obj_tag ooo;
        
        ooo.vlineobj.resize( dimx * dimy );
        ooo.vzlineobj.resize( dimx * dimy );
        vlineseg.push_back( ooo );                  //just allocate space at this moment
        }
        
    vlo = lo;
    }



if( bdotmark)
    {
    vdotmark.resize( surfx * surfy );       //holds 3d surface rectangles
 //   vzdotmark.resize( surfx * surfy );		//holds 2d surface rects in z order for plotting
    }

//vampl = o;

//work out max and min amplitudes, adj gain also
valmin = 0;
valmax = 0;

dminx = dminy = dmaxx = dmaxy = 0;

for( int yy = 0; yy < dimy; yy++ )
	{
	for( int xx = 0; xx < dimx; xx++ )
		{
		int ptr = xx * dimy + yy;
		
		double ampl = o[ ptr ].val * ampl_gain;
		
		if( fpclassify( ampl ) == FP_NAN )				//nan cause std::sort to crash, see sort call well below, [search for eps]
			{
			double eps = 2.2204e-16;
			printf( "build_surface2() - ampl: %f, fpclassify(ampl) == FP_NAN, using eps: %e\n", ampl, eps );
			ampl = eps; 
			}

		vampl[ ptr ] = ampl;

		if( ampl < valmin ) valmin = ampl;
		if( ampl > valmax ) valmax = ampl;

		if( o[ ptr ].x < dminx  ) dminx = o[ ptr ].x;			//make min/max
		if( o[ ptr ].y < dminy  ) dminy = o[ ptr ].y;

		if( o[ ptr ].x > dmaxx  ) dmaxx = o[ ptr ].x;
		if( o[ ptr ].y > dmaxy  ) dmaxy = o[ ptr ].y;
		}
	}

//printf("dminx: %f, dminy: %f, dmaxx: %f, dmaxy: %f, dimx: %d\n", dminx, dminy, dmaxx, dmaxy, dimx );

vtext = vtxt;

//for( int i = 0; i < vtext.size(); i++ )
//	{
//	vtext[ i ].x *= 1;
//	vtext[ i ].y *= 1;
//	}


redraw();
return 1;
}










/*
//scale a line about its centre
//returns offset from origin
double surface3d::scale_line( double &aa, double &bb, double scale )
{
double cc, dd;
double delta;
double offs_orig;


if( fabs( aa ) < fabs( bb ) )               //find number closer to origin
    {
    delta = aa - bb;                        //find length

    offs_orig = aa - delta / 2.0;           //offs centre of this length to origin
    }
else{
    delta = bb - aa;                        //find length
    offs_orig = bb + delta / 2.0;           //offs centre of this length to origin
    }


aa = ( aa - offs_orig ) * scale + offs_orig; //translate to origin, scale, translate back to original pos
bb = ( bb - offs_orig ) * scale + offs_orig; //translate to origin, scale, translate back to original pos

return offs_orig;
}

*/









void surface3d::translate_obj( st_surfplot_tag &o, double dx, double dy, double dz )
{
o.x1 += dx;
o.x2 += dx;
o.x3 += dx;
o.x4 += dx;

o.y1 += dy;
o.y2 += dy;
o.y3 += dy;
o.y4 += dy;

o.z1 += dz;
o.z2 += dz;
o.z3 += dz;
o.z4 += dz;
}







void surface3d::translate_obj2( st_surfplot_tag &o, double dx, double dy, double dz )
{
o.x1 += dx;
o.x2 += dx;
o.x3 += dx;
o.x4 += dx;

o.y1 += dy;
o.y2 += dy;
o.y3 += dy;
o.y4 += dy;

o.z1 += dz;
o.z2 += dz;
o.z3 += dz;
o.z4 += dz;
}









/*

void surface3d::find_centre( st_surfplot_tag o, double &centrex, double &centrey, double &centrez )
{

scale_surface( o, 0.0001, 0.0001 );

centrex = o.x1;
centrey = o.y1;
centrez = o.z1;

}
*/











//scale a surface about its centre
//works out gradient of 3d line
void surface3d::scale_surface( st_surfplot_tag &o, double scalex, double scaley )
{

double aa, bb, cc;
double delta, scale;
double offs_orig;

//o.y1 = 2;
//o.y4 = 6;
//o.z1 = 3;
//o.z4 = 7;

//printf( "before x1: %g, x2: %g, x3: %g, x4: %g \n", o.x1, o.x2, o.x3, o.x4 );
//printf( "before y1: %g, y2: %g, y3: %g, y4: %g \n", o.y1, o.y2, o.y3, o.y4 );
//printf( "before z1: %g, z2: %g, z3: %g, z4: %g \n", o.z1, o.z2, o.z3, o.z4 );

double rise;
double run;
double zz;
double b;
double dxa, dya, dza;
double dxb, dyb, dzb;
double new_length;
double shorten;

//formulate line equation for x


dza = o.z2 - o.z1;							//south
dxa = o.x2 - o.x1;
double mx1 = dza / dxa;


dzb = o.z4 - o.z3;							//north
dxb = o.x4 - o.x3;
double mx2 = dzb / dxb;

b = o.z1 - mx1 * o.x1;						//work out intercept



//printf( "mx1: %g, mx2: %g\n", mx1, mx2 );

//scalex = 1.0;

new_length = dxa - ( dxa * scalex );
shorten = new_length / 2.0;

o.x1 = o.x1 + shorten;
o.x2 = o.x2 - shorten;

//o.x1 += 0.035;

zz = mx1 * o.x1 + b;						//calc new coord
o.z1 = zz;
//printf( "b: %g\n", b );
//printf( "o.x1: %g, zz: %g\n", o.x1, zz );


//o.x2 -= 0.035;

zz = mx1 * o.x2 + b;
o.z2 = zz;
//printf( "o.x2: %g, zz: %g\n", o.x2, zz );



new_length = dxa - ( dxa * scalex );
shorten = new_length / 2.0;

b = o.z3 - mx2 * o.x3;						//work out intercept

o.x4 = o.x4 + shorten;
o.x3 = o.x3 - shorten;

zz = mx2 * o.x3 + b;						//calc new coord
o.z3 = zz;


zz = mx2 * o.x4 + b;						//calc new coord
o.z4 = zz;




dza = o.z4 - o.z1;							//west							
dya = o.y4 - o.y1;
double my1 = dza / dya;

dzb = o.z3 - o.z2;							//east
dyb = o.y3 - o.y2;
double my2 = dzb / dya;

b = o.z1 - my1 * o.y1;					    //work out intercept

//scaley = 0.5;

//printf( "dza: %g, dya: %g\n", dza, dya );


new_length = dya - ( dya * scaley );
shorten = new_length / 2.0;

//printf( "len: %g, len: %g\n", new_length, shorten );


o.y4 = o.y4 - shorten;
o.y1 = o.y1 + shorten;


zz = my1 * o.y4 + b;						    //calc new coord
o.z4 = zz;

zz = my1 * o.y1 + b;						    //calc new coord
o.z1 = zz;

//printf( "my1: %g, my2: %g\n", my1, my2 );

//printf( "b: %g\n", b );
//printf( "o.y1: %g, zz: %g\n", o.y1, zz );





b = o.z3 - my1 * o.y3;					    //work out intercept

new_length = dyb - ( dyb * scaley );
shorten = new_length / 2.0;

o.y3 = o.y3 - shorten;
o.y2 = o.y2 + shorten;

zz = my1 * o.y3 + b;						    //calc new coord
o.z3 = zz;

zz = my1 * o.y2 + b;						    //calc new coord
o.z2 = zz;


}












/*
//find centre of obj
void surface3d::make_centre( st_surfplot_tag &o, double &cx, double &cy, double &cz )
{
st_surfplot_tag o1;

o1 = o;
make_point_from_corner( o1, 0.5, en_cw_south_west );          //bring in the sw edge on x axis

make_point_from_corner( o1, 0.5, en_cw_south_east );          //bring in the se edge on y axis

cx = o.x1;
}
*/














void surface3d::calc_normals( st_surfplot_tag &o )
{
double vx1, vy1, vz1;
double vx2, vy2, vz2;
double cx, cy, cz;
double dist;
double nx, ny, nz;

vx1 = ( o.x2 - o.x1 );                      //vectors
vy1 = ( o.y2 - o.y1 );
vz1 = ( o.z2 - o.z1 );

vx2 = ( o.x3 - o.x2 );
vy2 = ( o.y3 - o.y2 );
vz2 = ( o.z3 - o.z2 );

cx = vy1 * vz2 - vz1 * vy2;                 //cross product to get normals
cy = vz1 * vx2 - vx1 * vz2;
cz = vx1 * vy2 - vy1 * vx2;

dist = sqrt( cx * cx + cy * cy + cz * cz );

nx = cx / dist;                             //normalise normals
ny = cy / dist;
nz = cz / dist;

o.nx = nx;
o.ny = ny;
o.nz = nz;

}













//get centre of obj
void surface3d::get_centre( st_surfplot_tag o, double &centrex, double &centrey, double &centrez )
{

st_surfplot_tag o1;

o1 = o;

calc_centre( o1 );


centrex = o1.centrex;
centrey = o1.centrey;
centrez = o1.centrez;

}






void surface3d::calc_centre( st_surfplot_tag &o )
{
double x1, y1, z1;
double x2, y2, z2;
double x3, y3, z3;
double x4, y4, z4;

double cx, cy, cz;

st_surfplot_tag o1;
o1 = o;

make_point_from_corner( o, 0.5, en_cw_south_west );          //bring in the sw edge on x axis
x1 = o.x1;
y1 = o.y1;
z1 = o.z1;


o = o1;
make_point_from_corner( o, 0.5, en_cw_south_east );          //bring in the se edge on y axis
x2 = o.x2;
y2 = o.y2;
z2 = o.z2;


//o.x1 = x1;
//o.y1 = y1;
//o.z1 = z1;


o = o1;
make_point_from_corner( o, 0.5, en_cw_north_east );          //bring in the sw edge on x axis
x3 = o.x3;
y3 = o.y3;
z3 = o.z3;



o = o1;
make_point_from_corner( o, 0.5, en_cw_north_west );          //bring in the sw edge on x axis
x4 = o.x4;
y4 = o.y4;
z4 = o.z4;

o = o1;

o.x1 = x1;
o.y1 = y1;
o.z1 = z1;

o.x2 = x2;
o.y2 = y2;
o.z2 = z2;

o.x3 = x3;
o.y3 = y3;
o.z3 = z3;

o.x4 = x4;
o.y4 = y4;
o.z4 = z4;



make_point( x4, y4, z4, x2, y2, z2, 0.5, cx, cy, cz );


o = o1;

o.centrex = cx;
o.centrey = cy;
o.centrez = cz;

o.x1 = cx;
o.y1 = cy;
o.z1 = cz;

o.x2 = cx;
o.y2 = cy;
o.z2 = cz;

//make_point_from_corner( o, 0.5, en_cw_north_east );          //bring in the sw edge on x axis

//make_point_from_corner( o, 0.5, en_cw_north_west );          //bring in the se edge on y axis

return;


//make_centre( o, cx, cy, cz );

o.centrex = cx;
o.centrey = cy;
o.centrez = cz;

translate_obj2( o, cx, cy, cz );


return;


//en_cw_south_west,                   //x1,y1,z1
//en_cw_south_east,                   //x2,y2,z2
//en_cw_north_east,                   //x3,y3,z3
//en_cw_north_west,                   //x4,y4,z4

//st_surfplot_tag o1;

o1 = o;
make_point_from_corner( o1, scalex, en_cw_south_west );          //bring in the sw edge on x axis

x1 = o1.x1;
y1 = o1.y1;
z1 = o1.z1;



o1 = o;
make_point_from_corner( o1, 1.0 - scalex, en_cw_south_west );      //bring in the se edge on x axis

x2 = o1.x1;
y2 = o1.y1;
z2 = o1.z1;



o1 = o;
make_point_from_corner( o1, scaley, en_cw_south_east );            //bring in the se edge on y axis

x2 = o1.x2;
y2 = o1.y2;
z2 = o1.z2;



o1 = o;
make_point_from_corner( o1, 1.0 - scaley, en_cw_south_east );      //bring in the ne edge on y axis


x3 = o1.x2;
y3 = o1.y2;
z3 = o1.z2;



o1 = o;
make_point_from_corner( o1, scalex, en_cw_north_east );      //bring in the ne edge on x axis


x3 = o1.x3;
y3 = o1.y3;
z3 = o1.z3;


o1 = o;
make_point_from_corner( o1, 1.0 - scalex, en_cw_north_east );            //bring in the nw edge on x axis

x4 = o1.x3;
y4 = o1.y3;
z4 = o1.z3;



o.x1 = x1;
o.y1 = y1;
o.z1 = z1;

o.x2 = x2;
o.y2 = y2;
o.z2 = z2;

o.x3 = x3;
o.y3 = y3;
o.z3 = z3;

o.x4 = x4;
o.y4 = y4;
o.z4 = z4;

/*
double aa, bb, cc;
double delta, scale;
double offs_orig;


double rise;
double run;
double zz;
double b;
double dxa, dya, dza;
double dxb, dyb, dzb;
double new_length;
double shorten;

//formulate line equation for x


dza = o.z2 - o.z1;							//south
dxa = o.x2 - o.x1;
double mx1 = dza / dxa;


dzb = o.z4 - o.z3;							//north
dxb = o.x4 - o.x3;
double mx2 = dzb / dxb;

b = o.z1 - mx1 * o.x1;						//work out intercept

new_length = dxa - ( dxa * scalex );
shorten = new_length / 2.0;

o.x1 = o.x1 + shorten;
o.x2 = o.x2 - shorten;


zz = mx1 * o.x1 + b;						//calc new coord
o.z1 = zz;

zz = mx1 * o.x2 + b;
o.z2 = zz;



new_length = dxa - ( dxa * scalex );
shorten = new_length / 2.0;

b = o.z3 - mx2 * o.x3;						//work out intercept

o.x4 = o.x4 + shorten;
o.x3 = o.x3 - shorten;

zz = mx2 * o.x3 + b;						//calc new coord
o.z3 = zz;


zz = mx2 * o.x4 + b;						//calc new coord
o.z4 = zz;




dza = o.z4 - o.z1;							//west							
dya = o.y4 - o.y1;
double my1 = dza / dya;

dzb = o.z3 - o.z2;							//east
dyb = o.y3 - o.y2;
double my2 = dzb / dya;

b = o.z1 - my1 * o.y1;					    //work out intercept



new_length = dya - ( dya * scaley );
shorten = new_length / 2.0;


o.y4 = o.y4 - shorten;
o.y1 = o.y1 + shorten;

zz = my1 * o.y4 + b;						    //calc new coord
o.z4 = zz;

zz = my1 * o.y1 + b;						    //calc new coord
o.z1 = zz;





b = o.z3 - my1 * o.y3;					    //work out intercept

new_length = dyb - ( dyb * scaley );
shorten = new_length / 2.0;

o.y3 = o.y3 - shorten;
o.y2 = o.y2 + shorten;

zz = my1 * o.y3 + b;						    //calc new coord
o.z3 = zz;

zz = my1 * o.y2 + b;						    //calc new coord
o.z2 = zz;

*/
}

















// Liang-Barsky function by Daniel White @ http://www.skytopia.com/project/articles/compsci/clipping.html
// This function inputs 8 numbers, and outputs 4 new numbers (plus a boolean value to say whether the clipped line is drawn at all).
//
bool LiangBarsky (double edgeLeft, double edgeRight, double edgeBottom, double edgeTop,   // Define the x/y clipping values for the border.
                  double x0src, double y0src, double x1src, double y1src,                 // Define the start and end points of the line.
                  double &x0clip, double &y0clip, double &x1clip, double &y1clip)         // The output values, so declare these outside.
{
    
    double t0 = 0.0;    double t1 = 1.0;
    double xdelta = x1src-x0src;
    double ydelta = y1src-y0src;
    double p,q,r;

    for(int edge=0; edge<4; edge++) {   // Traverse through left, right, bottom, top edges.
        if (edge==0) {  p = -xdelta;    q = -(edgeLeft-x0src);  }
        if (edge==1) {  p = xdelta;     q =  (edgeRight-x0src); }
        if (edge==2) {  p = -ydelta;    q = -(edgeBottom-y0src);}
        if (edge==3) {  p = ydelta;     q =  (edgeTop-y0src);   }   
        r = q/p;
        if(p==0 && q<0) return false;   // Don't draw line at all. (parallel line outside)

        if(p<0) {
            if(r>t1) return false;         // Don't draw line at all.
            else if(r>t0) t0=r;            // Line is clipped!
        } else if(p>0) {
            if(r<t0) return false;      // Don't draw line at all.
            else if(r<t1) t1=r;         // Line is clipped!
        }
    }

    x0clip = x0src + t0*xdelta;
    y0clip = y0src + t0*ydelta;
    x1clip = x0src + t1*xdelta;
    y1clip = y0src + t1*ydelta;

    return true;        // (clipped) line is drawn
}






















/*

surfplot_tag o1, o2, o11, o22;
o1.x1 = s1.x1;
o1.y1 = s1.y1;
o1.z1 = s1.z1;

o1.x2 = s1.x2;
o1.y2 = s1.y2;
o1.z2 = s1.z2;

o1.x3 = s1.x3;
o1.y3 = s1.y3;
o1.z3 = s1.z3;

o1.x4 = s1.x4;
o1.y4 = s1.y4;
o1.z4 = s1.z4;


o2.x1 = s2.x1;
o2.y1 = s2.y1;
o2.z1 = s2.z1;

o2.x2 = s2.x2;
o2.y2 = s2.y2;
o2.z2 = s2.z2;

o2.x3 = s2.x3;
o2.y3 = s2.y3;
o2.z3 = s2.z3;

o2.x4 = s2.x4;
o2.y4 = s2.y4;
o2.z4 = s2.z4;


double centrex, double centrey, double centrez;
 
find_centre( o1, centrex, centrey, centrez );
*/








//rotate a x2, y2, z2 point around x1, y1, z1 point, using spec thetas,
//will move to origin firstly,
//thetas need to be in radians
void surface3d::rotate_point( double x1, double y1, double z1, double &x2, double &y2, double &z2, double thetax, double thetay, double thetaz )
{

double ox, oy, oz;


x2 = x2 + -x1;                   //move so x1,y1,z1 are effectively at origin
y2 = y2 + -y1;
z2 = z2 + -z1;

double rx2, ry2, rz2;

if( thetax != 0.0 )             //rotate around x-xais 
    {
    ry2 = y2 * cos( thetax ) + z2 * sin( thetax );
    rz2 = z2 * cos( thetax ) - y2 * sin( thetax );

    y2 = ry2;
    z2 = rz2;
    }

if( thetay != 0.0 )             //rotate around y-xais 
    {
    rx2 = x2 * cos( thetay ) + z2 * sin( thetay );
    rz2 = z2 * cos( thetay ) - x2 * sin( thetay );

    x2 = rx2;
    z2 = rz2;
    }


if( thetaz != 0.0 )             //rotate around z-xais 
    {
    rx2 = x2 * cos( thetaz ) + y2 * sin( thetaz );
    ry2 = y2 * cos( thetaz ) - x2 * sin( thetaz );
    
    x2 = rx2;
    y2 = ry2;
    }
    

x2 = x2 + x1;                   //move so x1,y1,z1 are effectively restored
y2 = y2 + y1;
z2 = z2 + z1;
}












//build 3d surface grid from the scaled amplitude grid vector,
//the surface is then translated/rotated and projected for display,
//highlighting and grid bounding lines may be added,
//user line obj may be added,
//does translate, perspective projection, sorts by z-axis and calls plot function
void surface3d::make_grid()
{
double x;
double y;
double z;


if( dimx < 2 ) return;					//need at least 2 points per dimension
if( dimy < 2 ) return;					//need at least 2 points per dimension

double rectx = 0.1;						//default x and y incs
double recty = 0.1;
double rectz = 0.1;



double startx = -( surfx * rectx ) / 2.0;
double starty = -( surfy * recty ) / 2.0;




//printf("startx: %f, starty: %f\n", startx, starty ); 

//build 3d grid rectangles
int id = 0;
y = starty;
for( int yy = 0; yy < surfy; yy++ )			//for each surface rectangle
	{
	x = startx;

	for( int xx = 0; xx < surfx; xx++ )
		{
		double z1, z2, z3, z4;

		int ptr1 = xx * dimy + yy;	        //rem point matrix is bigger than rect matrix

//		z1 = grid[ xx ][ yy ];

		z1 = vampl[ ptr1 ] * wheel_gain;	//south west
		z2 = 0;								//south east
		z3 = 0;								//north east
		z3 = 0;								//north west

        ptr1 = ( xx + 1 ) * dimy + yy;
        z2 = vampl[ ptr1 ] * wheel_gain;

        ptr1 = ( xx + 1 ) * dimy + yy + 1;
        z3 = vampl[ ptr1 ] * wheel_gain;

        ptr1 = xx * dimy + yy + 1;
        z4 = vampl[ ptr1 ] * wheel_gain;

		

		st_surfplot_tag o;
		o.x1 = x;
		o.y1 = y;
		o.z1 = z1;
		o.x2 = x + rectx;
		o.y2 = y;
		o.z2 = z2;
		o.x3 = x + rectx;
		o.y3 = y + recty;
		o.z3 = z3;
		o.x4 = x;
		o.y4 = y + recty;
		o.z4 = z4;
        o.options = en_po_none;
        o.id = id;
        

        o.col_select_1.r = 255;
        o.col_select_1.g = 128;
        o.col_select_1.b = 128;

        o.col_select_2.r = 128;
        o.col_select_2.g = 128;
        o.col_select_2.b = 255;
        
        o.col_select_3.r = 255;
        o.col_select_3.g = 128;
        o.col_select_3.b = 255;


        o.grid_line_wid = grid_line_wid;
        o.grid_line_style = grid_line_style;        //grid line style, see: en_surfaceplot_line_style
        o.line_wid = grid_line_wid;      			//these need legal values also
        o.line_style = grid_line_style; 


		int ptr2 = xx * surfy + yy;		    		//rem point matrix is bigger than rect matrix
		vsp[ ptr2 ] = o;							//store coords of rectangle
		vzorder[ ptr2 ] = o;
		vzorder[ ptr2 ].hightlight_x_line = 0;		//set no highlighted lines, as yet
		vzorder[ ptr2 ].hightlight_y_line = 0;
		vzorder[ ptr2 ].id = id;


        int lcnt = vlo.size();
        if( lcnt != 0 )                             //any user line objs?
            {
            //each user line obj will be made up of a number of short line segments depending
            //on how many 3d grid rectangles the line occupies
            for( int i = 0; i < lcnt; i++ )	        //for each obj
                {
               
                st_surfplot_tag o;
                
                double linx1, liny1, linx2, liny2;


                o.x1 = x;
                o.y1 = y;
                o.z1 = z1;
                o.x2 = x + rectx;
                o.y2 = y;
                o.z2 = z2;
                o.x3 = x + rectx;
                o.y3 = y + recty;
                o.z3 = z3;
                o.x4 = x;
                o.y4 = y + recty;
                o.z4 = z4;
                o.options = en_po_dont_plot;	    //assume not line segment not visible
                o.id = id;



                double dx;
                double dy;
                double dz;
                double grad;
                double zintercept;

                bool east_higher = 0;               //flg which way z axis is changing
                bool east_lower = 0;
                bool north_higher = 0;
                bool north_lower = 0;

				
                double x1, y1, z1;

                x1 = o.x1;                          //rotation anchor point (origin) of surface rect that holds line segment
                y1 = o.y1;
                z1 = o.z1;


                if( o.z2 > o.z1 )                   //west to east height change, east higher?
                    {
                    east_higher = 1;
					dz = o.z2 - o.z1;
					dx = o.x2 - o.x1;

					grad = dz / dx;
					zintercept = o.z1 - grad * o.x1;
                    }


                if( o.z1 > o.z2 )                   //west to east height change, east lower?
                    {
                    east_lower = 1;
					dz = o.z1 - o.z2;
					dx = o.x1 - o.x2;
                    if( dx == 0.0 ) dx = 1e-6;
					grad = dz / dx;
					zintercept = o.z1 - grad * o.x1;
                    }


                if( o.z1 > o.z4 )                   //south to north height change, north lower?
                    {
                    north_lower = 1;
					dz = o.z4 - o.z1;
					dy = o.y4 - o.y1;

					grad = dz / dy;
					zintercept = o.z1 - grad * o.y1;
                    }

                if( o.z4 > o.z1 )                   //south to north height change, north higher?
                    {
                    north_higher = 1;

					dz = o.z4 - o.z1;
					dy = o.y4 - o.y1;

					grad = dz / dy;
					zintercept = o.z1 - grad * o.y1;
                    }

				double clip_left = o.x1;
				double clip_right = o.x2;
				double clip_bott = o.y1;
				double clip_top = o.y3;
				
				double clipx1, clipy1, clipx2, clipy2;
				
                linx1 = vlo[ i ].x1 * rectx;                //get line obj coords
                liny1 = vlo[ i ].y1 * recty;
                linx2 = vlo[ i ].x2 * rectx;
                liny2 = vlo[ i ].y2 * recty;


if( o.id == 2553 )
        {
		
//        printf( "lineid %d: o.z1: %f, o.z2: %f, o.z3: %f, o.z4: %f\n", o.id, o.z1, o.z2, o.z3, o.z4 );
//        printf( "lineid %d: o.x1: %f, o.y1: %f, o.x2: %f, o.y2: %f\n", o.id, o.x1, o.y1, o.x2, o.y2 );
//        printf( "lineid %d: o.x3: %f, o.y3: %f, o.x4: %f, o.y4: %f\n", o.id, o.x3, o.y3, o.x4, o.y4 );
        
//        printf( "lineeq: o.x1: %f, o.y1: %f, o.x2: %f, o.y2: %f\n", linx1, liny1, linx2, liny2 );
        }

				//clip line down to a line segment using current surface rect as the border limits
				if( LiangBarsky ( clip_left, clip_right, clip_bott, clip_top, linx1, liny1, linx2, liny2, clipx1, clipy1, clipx2, clipy2 ) )
					{
					o.x1 = clipx1;
					o.y1 = clipy1;
					o.x2 = clipx2;
					o.y2 = clipy2;



//rotate_point( x1, y1, z1, nx, ny, nz, 0, -t4hetay, 0 );

                    o.x1 = clipx1;
                    o.y1 = clipy1;
                    o.x2 = clipx2;
                    o.y2 = clipy2;

                    if( east_lower )
                        {
                        o.z1 = grad * clipx1 + zintercept;      //calc new heights of line segment
                        o.z2 = grad * clipx2 + zintercept;
                        }


                    if( east_higher )
                        {
                        o.z1 = grad * clipx1 + zintercept;      //calc new heights of line segment
                        o.z2 = grad * clipx2 + zintercept;
                        }

                    if( north_lower )
                        {
							
                        o.z1 = grad * clipy1 + zintercept;      //calc new heights of line segment
                        o.z2 = grad * clipy2 + zintercept;
                        }

                    if( north_higher )
                        {
							
                        o.z1 = grad * clipy1 + zintercept;      //calc new heights of line segment
                        o.z2 = grad * clipy2 + zintercept;
                        }


//rotate_point( x1, y1, z1, nx, ny, nz, 0, -thetay, 0 );




					o.options = en_po_none;									//show segment visible in this rect	
					}
                  
                                

                o.colline.r = vlo[ i ].r;
                o.colline.g = vlo[ i ].g;
                o.colline.b = vlo[ i ].b;

                 
                o.line_wid = vlo[ i ].line_wid;
                o.line_style = vlo[ i ].line_style;
//                vlineobj[ ptr2 ] = o;                                     //store new line segment for curr surface rect
                
                vlineseg[ i ].vlineobj[ ptr2 ] = o;                         //store new line segment for curr surface rect
                }
            }

        
        if( bdotmark )
            {
            
            st_surfplot_tag o;
            o.x1 = x;
            o.y1 = y;
            o.z1 = z1;
            o.x2 = x + rectx;
            o.y2 = y;
            o.z2 = z2;
            o.x3 = x + rectx;
            o.y3 = y + recty;
            o.z3 = z3;
            o.x4 = x;
            o.y4 = y + recty;
            o.z4 = z4;
            o.options = en_po_dont_plot;
            o.id = id;

            o.col_select_1.r = 255;
            o.col_select_1.g = 128;
            o.col_select_1.b = 128;

            o.col_select_2.r = 128;
            o.col_select_2.g = 128;
            o.col_select_2.b = 255;
            
            o.col_select_3.r = 255;
            o.col_select_3.g = 128;
            o.col_select_3.b = 255;

			o.line_wid = 1;
			o.line_style = (en_surfaceplot_line_style)FL_SOLID;

			o.grid_line_wid = 1;
			o.grid_line_style = (en_surfaceplot_line_style)FL_SOLID;

            if ( id == selected_id )
                {
//            if( yy == 14 ) 
//                {
//                if( xx == 35 )
//                    {
                    o.options = en_po_show_select_2 | en_po_show_normals;
                    

//o.x1 = 0.00;             //sw
//o.x2 = 2;             //se
//o.x3 -= 0.025;             //ne
//o.x4 += 0.025;             //nw

//o.y1 += 0.025;               //sw
//o.y2 += 0.025;               //se
//o.y3 -= 0.025;               //ne
//o.y4 -= 0.025;               //nw


//o.z1 = 1;               //sw
//o.z2 = 3;               //se
//o.z3 -= 0.05;               //ne
//o.z4 -= 0.05;               //nw

//o.x4 -= 0.05;             //nw

//printf( "o.x2: %g\n", o.x2 );

//o.x4 += 0.05;           //nw

//o.z1 -= 0.05;

//translate_obj( o, 0.0, -0.05, 0.0 );

            scale_surface( o, 0.5, 0.5 );

//                    }
                }


            if( yy == 14 ) 
                {
                if( xx == 35 )
                    {
                    o.options = en_po_show_select_3 | en_po_show_centre_dot | en_po_show_normals;
                    }
                }

            vdotmark[ ptr2 ] = o;
            }

		x += rectx;
        id++;
		}

	y += recty;
	}



if( bdotmark )
    {
	vzdotmark = vdotmark;									//this copies over line styles and widths
	}






//pi vals for different quadrants
//#define cn_pi_45 M_PI / 4.0
//#define cn_pi_90 M_PI / 2.0
//#define cn_pi_180 M_PI
//#define cn_pi_270 1.5 * M_PI

//#define cn_pi_360 2.0 * M_PI



//translate, rotate and pespective divide grid rectangles
int highlight_counter_x, highlight_counter_y;
bool do_highlight_x, do_highlight_y;				//set if line is to hightlight

highlight_counter_y = highlight_every_y;				//setup for possible line hightlight
do_highlight_y = 0;
for( int yy = 0; yy < surfy; yy++ )
	{
	highlight_counter_x = highlight_every_x; //setup for possible line hightlight
	do_highlight_x = 0;
	for( int xx = 0; xx < surfx; xx++ )
		{
		double x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

		mg_col_tag colrect;
		int ptr1 = xx * surfy + yy;	//rem point matrix is bigger than rect matrix

//		double v = grid[ xx ][ yy ];

		double val_z1;								//south west
		double val_z2;								//south east
		double val_z3;								//north east
		double val_z4;								//north west


		double val_rot_z1;					//south west - rotated by position on grid
		double val_rot_z2;					//south east
		double val_rot_z3;					//north east
		double val_rot_z4;					//north west
		
		double theta_x = M_PI * ( (double)xx / (double)surfx );
		double theta_y = M_PI * ( (double)yy / (double)surfy );
		
		double theta = theta_x;
//		val_rot_z1 = val_z1 * cos( theta_x ) + val_z2 * sin( theta_y );
//		val_rot_z2 = val_z1 * cos( theta_x );

//		printf("thetax: %f, thetay: %f\n", theta_x, theta_y );
		val_z1 = vsp[ ptr1 ].z1;// vampl[ ptr1 ];
//		ptr1 = ( xx + 1 ) * surfy + yy;
		val_z2 = vsp[ ptr1 ].z2;//vampl[ ptr1 ];

//		ptr1 = ( xx + 1 ) * surfy + yy + 1;
		val_z3 = vsp[ ptr1 ].z3;//vampl[ ptr1 ];

//		ptr1 = xx * surfy + yy + 1;
		val_z4 = vsp[ ptr1 ].z4;//vampl[ ptr1 ];

		bool quadx = 0;
		bool quady = 0;

		bool odd_x = 1; 
		bool odd_y = 1; 
//		if( ( surfx % 2) != 0 ) odd_x = 1;	//is there an odd number of rext in x dim

//		if( ( surfy % 2) != 0 ) odd_y = 1;	//is there an odd number of rext in y dim
		
//		double centre_x = 0.5;
//		double centre_y = 0.5;

//		if( odd_x ) centre_x = 0.5;
//		if( odd_y ) centre_y = 0.5;
		
		double half_x = surfx / 2.0;
		double half_y = surfy / 2.0;
	
//		if( (double)surfx > half_x ) quadx = 1;
//		if( (double)surfy > half_y ) quady = 1;
		
		double newx = -half_x + xx + 0.5;	//make a cartesian coord centred on current rect
		double newy = -half_y + yy + 0.5;

	
//		double v = vampl[ ptr1 ];				//scale amplitude to between 0 - 0.999
//		double v = val_z1;						//scale amplitude to between 0 - 0.999

		double v = ( val_z1 + val_z2 + val_z3 + val_z4 ) / 4.0 ;	//average out rect z values
		v = v - valmin;
		v = v / ( valmax - valmin );			//scale amplitude to between 0 - 0.999
	
//printf("y=%d  x=%d, vampl=%f\n", yy, xx, v );
		
				

//			fl_begin_complex_polygon();


/*
		x1 = vsp[ ptr ].x1;
		y1 = vsp[ ptr ].y1;
		z1 = vsp[ ptr ].z1;
		x2 = vsp[ ptr ].x2;
		y2 = vsp[ ptr ].y2;
		z2 = vsp[ ptr ].z2;
		x3 = vsp[ ptr ].x3;
		y3 = vsp[ ptr ].y3;
		z3 = vsp[ ptr ].z3;
		x4 = vsp[ ptr ].x4;
		y4 = vsp[ ptr ].y4;
		z4 = vsp[ ptr ].z4;
//			plot_3d_rect( x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, 1, 1, colfill, colline );
//			plot_3d_rect2( ptr, 1, 1, colfill, colline );

//			fl_end_complex_polygon();
*/

		int ptr2 = xx * surfy + yy;	//rem point matrix is bigger than rect matrix

		trans_rotate_project( vsp, vzorder, ptr2 );	//do rotate, perspective divide to make plot values



        int cnt = vlineseg.size();
        if( cnt != 0 )                                       //any user line objs?
            {
            for( int i = 0; i < cnt; i++ )
                {
                trans_rotate_project( vlineseg[ i ].vlineobj, vlineseg[ i ].vzlineobj, ptr2 );
//                trans_rotate_project( vlineobj, vzlineobj, ptr2 );

//                vzlineobj[ ptr2 ].colline = vlineobj[ ptr2 ].colline;             //copy over some settings
//                vzlineobj[ ptr2 ].line_wid = vlineobj[ ptr2 ].line_wid;
//                vzlineobj[ ptr2 ].line_style = vlineobj[ ptr2 ].line_style;
                
                vlineseg[ i ].vzlineobj[ ptr2 ].colline =  vlineseg[ i ].vlineobj[ ptr2 ].colline;             //copy over some settings
                vlineseg[ i ].vzlineobj[ ptr2 ].line_wid =  vlineseg[ i ].vlineobj[ ptr2 ].line_wid;
                vlineseg[ i ].vzlineobj[ ptr2 ].line_style =  vlineseg[ i ].vlineobj[ ptr2 ].line_style;
                }
            }




        
		if( bdotmark ) trans_rotate_project( vdotmark, vzdotmark, ptr2 );	//do rotate, perspective divide to make plot values


		dissolve_ramp( v, colrect );			        //use amplitude to modulate colour

		if( !force_col_fill ) vzorder[ ptr2 ].colfill = colrect;
		else vzorder[ ptr2 ].colfill = col_fill;

		vzorder[ ptr2 ].colline = col_line;             //copy over some settings
		vzorder[ ptr2 ].grid_line_wid = grid_line_wid;
		vzorder[ ptr2 ].grid_line_style = grid_line_style;


		//--- x line highlight ----
		if( do_highlight_x == 1 )						//just highlighted?
			{
			highlight_counter_x = highlight_every_x;	//renew
			do_highlight_x = 0;

			vzorder[ ptr2 ].hightlight_x_line = 1;		//set highlighted line, west side

			if( xx != 0 ) 
				{
				int ptr2 = ( xx - 1 ) * surfy + yy;
				vzorder[ ptr2 ].hightlight_x_line = 2;	//set highlighted line, east side
				}
			}

		if( highlight_counter_x > 0 )
			{
			highlight_counter_x--;
			if( highlight_counter_x == 0 ) do_highlight_x = 1;
			}
		//-------------------------



		//--- y line highlight ----
		if( do_highlight_y == 1 )						//highlighted?
			{
			vzorder[ ptr2 ].hightlight_y_line = 1;		//set highlighted line, south side

			if( yy != 0 ) 
				{
				int ptr2 = xx * surfy + yy - 1;
				vzorder[ ptr2 ].hightlight_y_line = 2;	//set highlighted line, north side
				}
			}
		}

	if( do_highlight_y == 1 )							//just highlighted?
		{
		highlight_counter_y = highlight_every_y;		//renew
		do_highlight_y = 0;
		}
	
	if( highlight_counter_y > 0 )
		{
		highlight_counter_y--;
		if( highlight_counter_y == 0 ) do_highlight_y = 1;
		}
		//-------------------------
	}

//printf("vzorder.size(): %d\n", vzorder.size() );
//printf("vzorder.begin(): %d\n", vzorder.begin() );
//printf("vzorder.end(): %d\n", vzorder.end() );
//return;

//sort by z-axis so farther away rectangles are plotted first
std::sort( vzorder.begin(), vzorder.end(), cmp_zaxis );					//nan will cause std::sort to crash, force eps as done well above in: build_surface2()



//draw grid rectangles/lines
//this plot will access the vector obj linearly as it has been sorted by z-axis
for( int i = 0; i < rect_count; i++ )
	{
	
	plot_obj( vzorder, i, show_filled, show_lines, 0, 0 );
//			printf("z1= %f\n", vzorder[ ptr_linear ].z1 );
	}


                
/*
if( vlineobj.size() )                       //any user line objs?
    {
    std::sort( vzlineobj.begin(), vzlineobj.end(), cmp_zaxis );

    for( int i = 0; i < rect_count; i++ )
        {
        plot_obj( vzlineobj, i, 0, 0, 1 );
        }
    }
*/



//draw user line obj segments
int cnt = vlineseg.size();
for( int j = 0; j < cnt; j++ )                     //any user line objs?
    {
    std::sort( vlineseg[ j ].vzlineobj.begin(), vlineseg[ j ].vzlineobj.end(), cmp_zaxis );

    for( int i = 0; i < rect_count; i++ )
        {
        plot_obj( vlineseg[ j ].vzlineobj, i, 0, 0, 1, 0 );
        }
    }




if( bdotmark)
    {
//draw grid rectangles/lines
//this plot will access the vector obj linearly as it has been sorted by z-axis

    //sort by z-axis so farther away rectangles are plotted first
    std::sort( vzdotmark.begin(), vzdotmark.end(), cmp_zaxis );



    for( int i = 0; i < rect_count; i++ )
        {
        
        plot_obj( vzdotmark, i, show_filled, show_lines, 0, 0 );
    //			printf("z1= %f\n", vzorder[ ptr_linear ].z1 );
        }
    }






//text obj code
vector<st_surfplot_tag> vto;			        				//holds 3d text coords
vector<st_surfplot_tag> vzto;			        				//holds translated/projected 2d text coords
st_surfplot_tag sp;

double finx = startx + rectx * surfx;							//these are internal grid coords see: rectx, recty
double finy = starty + recty * surfy;


//printf("startx: %f, finx: %f\n", startx, finx ); 
//printf("starty: %f, finy: %f\n", starty, finy ); 


double cntrx = dminx + ( dmaxx - dminx ) / 2.0;					//find a central point using user's grid x,y range values
double cntry = dminy + ( dmaxy - dminy ) / 2.0;

//printf("cntrx: %f, cntry: %f\n", cntrx, cntry ); 

double scalx = ( finx - startx ) / ( dmaxx - dminx );			//make a scaling factor using interal grid x,y vals and user supplied grid vals
double scaly = ( finy - starty ) / ( dmaxy - dminy );
sp.valid = 1;
sp.options = en_po_none;

sp.line_wid = 1;
sp.line_style = en_spls_solid;
sp.grid_line_wid = 1;
sp.grid_line_style = en_spls_solid;



for( int i = 0; i < vtext.size(); i++ )							//convert user text objs
	{
	sp.stext = vtext[ i ].stext;

	
	sp.x1 = ( vtext[ i ].x - cntrx ) * scalx;
	sp.y1 = ( vtext[ i ].y - cntry ) * scaly;
	sp.z1 = vtext[ i ].val;
	sp.show_text_background = vtext[ i ].show_text_background;

	sp.foregnd = vtext[ i ].foregnd;
	sp.backgnd = vtext[ i ].backgnd;
	sp.font = vtext[ i ].font;
	sp.fontsize = vtext[ i ].fontsize;

	vto.push_back( sp );
	}

vzto = vto;									//copies over attributes
for( int i = 0; i < vto.size(); i++ )
	{

	trans_rotate_project( vto, vzto, i );	//do rotate, perspective divide to make plot values

	plot_obj( vzto, i, 0, 0, 0, 1 );
	}
}










void surface3d::draw()
{
string s1;

int iF = fl_font();
int iS = fl_size();


fl_font( text_font, text_size );

midx = w() / 2;
midy = h() / 2;

fl_color( col_bkgd.r, col_bkgd.g, col_bkgd.b );
fl_rectf( 0, 0, w(), h() );

set_translate_vals( rotate_x, rotate_y, rotate_z );
make_grid();

//fl_line( midx + x + raster_offx, midy - y  + raster_offy, midx + oldx + raster_offx, midy - oldy  + raster_offy );
//fl_color( 255, 255, 0 );


//fl_rectf( woffx, woffy, w() - woffx, h() - woffy, 255, 255, 255 );


if( show_xyz_gain_text )
	{
	int txt_x = 5;

	int hei=14;
	int text_yoffs = h() - 6 * hei;


	//strpf( s1, "Gamma: %lf", gamma_display );
	//fl_draw(s1.c_str(), txt_x, text_yoffs + hei * 3);

	fl_color( 0, 0, 0 );

	strpf( s1, "rotx: %f", rotx );
	fl_draw(s1.c_str(), txt_x, text_yoffs +  hei * 2 );

	strpf( s1, "roty: %f", roty );
	fl_draw(s1.c_str(), txt_x, text_yoffs +  hei * 3 );

	strpf( s1, "rotz: %f", rotz );
	fl_draw(s1.c_str(), txt_x, text_yoffs +  hei * 4 );

	strpf( s1, "gain: %f", wheel_gain );
	fl_draw(s1.c_str(), txt_x, text_yoffs +  hei * 5 );

	}

fl_font( iF, iS );
}













//see:    http://alienryderflex.com/polygon/
//
//  vector<int> vx  holds horizontal coordinates of corners
//  vector<int> vy  holds vertical coordinates of corners (must have num of corners to vx)
//  int xx, yy      holds point to be tested
//
//
//  The function will return '1' if the point xx,yy is inside the polygon, or
//  '0' if it is not.  If the point is exactly on the edge of the polygon,
//  then the function may return '1' or '0'.
//
//  Note that division by zero is avoided because the division is protected
//  by the "if" clause which surrounds it.

bool surface3d::point_in_polygon( vector<int> &vx, vector<int> &vy, int xx, int yy )
{
int sides = vx.size();

if( sides == 0 ) return 0;
if( sides != vy.size() ) return 0;

int i, j = sides - 1;
bool odd_nodes = 0 ;

for ( i = 0; i < sides; i++ )
    {
    if ( ( vy[ i ] < yy && vy[ j ] >= yy || vy[ j ] < yy && vy[ i ] >= yy ) && ( vx[ i ] <= xx || vx[ j ] <= xx )) 
        {
        odd_nodes ^= ( vx[ i ] + ( yy - vy[ i ] ) / ( vy[ j ]- vy[ i ] ) * ( vx[ j ] - vx[ i ] ) < xx ); 
        }
    j = i; 
    }
        
return odd_nodes;
} 










int surface3d::find_rect_clicked( int xx, int yy )
{
vector<int> vx;
vector<int> vy;


//printf( "xx=: %d, yy: %d\n", xx, yy );

for( int i = 0; i < rect_count; i++ )
	{
    vx.clear();
    vy.clear();

    int x1 = midx + offx + vzorder[ i ].ix1;
    int y1 = midy - offy - vzorder[ i ].iy1;

    int x2 = midx + offx + vzorder[ i ].ix2;
    int y2 = midy - offy - vzorder[ i ].iy2;

    int x3 = midx + offx + vzorder[ i ].ix3;
    int y3 = midy - offy - vzorder[ i ].iy3;

    int x4 = midx + offx + vzorder[ i ].ix4;
    int y4 = midy - offy - vzorder[ i ].iy4;

	vx.push_back( x1 );
	vy.push_back( y1 );
 	vx.push_back( x2 );
	vy.push_back( y2 );
	vx.push_back( x3 );
	vy.push_back( y3 );
	vx.push_back( x4 );
	vy.push_back( y4 );
   
   if( point_in_polygon( vx, vy, xx, yy ) )
        {
        return vzorder[ i ].id;
        }
	}

return -1;
}







int surface3d::handle( int e )
{
bool dont_pass_on = 0;
bool need_redraw = 0;


if ( e == FL_KEYDOWN )
	{
	int key = Fl::event_key();
	
	if ( key == 'a' ) rotx -= 5.00;
	if ( key == 'z' ) rotx += 5.00;

	if ( key == 's' ) roty += 5.00;
	if ( key == 'x' ) roty -= 5.00;

	if ( key == 'd' ) rotz += 5.00;
	if ( key == 'c' ) rotz -= 5.00;

	if ( key == 'f' ) rho += 0.20;
	if ( key == 'v' ) rho -= 0.20;

	if( key == FL_Home )
		{
		rotx = 0;
		roty = 0;
		rotz = 0;
		
		offx = 0;
		offy = 0;
		}

	if( key == FL_Up )
		{
		offy += 10;
		}

	if( key == FL_Down )
		{
		offy -= 10;
		}

	if( key == FL_Left )
		{
		offx -= 10;
		}

	if( key == FL_Right )
		{
		offx += 10;
		}

	need_redraw = 1;
	dont_pass_on = 1;
	}



if ( e == FL_PUSH )
	{
	if( Fl::event_button() == 1 )
		{
		left_button = 1;

		cap_rotz = rotz;						//grab current angle
		cap_rotx = rotx;						//grab current angle
		cap_offx = offx;						//grab current angle
		cap_offy = offy;						//grab current angle
		
		lb_press_x = Fl::event_x();
		lb_press_y = Fl::event_y();
        
		}

	if( Fl::event_button() == 2 )
		{
		middle_button = 1;

		cap_offx = offx;						//grab current angle
		cap_offy = offy;						//grab current angle

		mb_press_x = Fl::event_x();
		mb_press_y = Fl::event_y();
		}

	if( Fl::event_button() == 3 )
		{
		right_button = 1;

		cap_rotx = rotx;						//grab current angle
		cap_roty = roty;						//grab current angle

		rb_press_x = Fl::event_x();
		rb_press_y = Fl::event_y();


        selected_id = find_rect_clicked( rb_press_x, rb_press_y );
        
        if( selected_id != -1 )
            {
            printf("clicked %d\n", selected_id );
            }
		}

	need_redraw = 1;
	dont_pass_on = 1;
//matrix_formula_gen();
	}


if( e & FL_MOVE )
	{
	mousex = Fl::event_x();
	mousey = Fl::event_y();

	if ( left_button )
		{
		int dx = mousex - lb_press_x;
		int dy = lb_press_y - mousey;


	if ( ctrl_key )
		{
        offx = cap_offx + dx / 1;
        offy = cap_offy + dy / 1;
        }
    else{
		rotz = cap_rotz + dx / 2;
		rotx = cap_rotx - dy / 2;
        }

		need_redraw = 1;
		}


	if ( middle_button )
		{
		int dx = mousex - mb_press_x;
		int dy = mb_press_y - mousey;

		offx = cap_offx + dx;
		offy = cap_offy + dy;

		need_redraw = 1;
		}


	if ( right_button )
		{
		int dx = mousex - rb_press_x;
		int dy = rb_press_y - mousey;

		rotx = cap_rotx - dy / 2;
		roty = cap_roty + dx / 2;

		need_redraw = 1;
		}

	dont_pass_on = 1;
	}



if ( e == FL_RELEASE )
	{

	if( Fl::event_button() == 1 )
		{
		left_button = 0;
		}

	if( Fl::event_button() == 2 )
		{
		middle_button = 0;
		}

	if( Fl::event_button() == 3 )
		{
		right_button = 0;
		}
		
	dont_pass_on = 1;
	need_redraw = 1;
	}




	//mousewheel
if ( e == FL_MOUSEWHEEL )
	{
	int mousewheel = Fl::event_dy();
    double dd = rho;

    double step = std_wheel_step;
    
    if( ctrl_key ) step = ctrl_wheel_step;
    if( shift_key ) step = shift_wheel_step;
 

    if( mousewheel > 0 ) dd -= step;
    if( mousewheel < 0 ) dd += step;

	if( ctrl_key & shift_key )
		{
		if( mousewheel > 0 ) wheel_gain += 0.1;
		if( mousewheel < 0 ) wheel_gain -= 0.1;
		}

    rho = dd;

	if(  Fl::event_dy() > 0 )
		{
//		rho -= 5;
		}
	else{
//		rho += 5;
		}

	dont_pass_on = 1;
	need_redraw = 1;
	}




if ( ( e == FL_KEYDOWN ) || ( e == FL_SHORTCUT ) )					//key pressed?
	{
	int key = Fl::event_key();
	
	if( ( key == FL_Control_L ) || (  key == FL_Control_R ) ) ctrl_key = 1;
	if( ( key == FL_Shift_L ) || (  key == FL_Shift_R ) ) shift_key = 1;
	
	need_redraw = 1;
    dont_pass_on = 0;
	}


if ( e == FL_KEYUP )												//key release?
	{
	int key = Fl::event_key();
	
	if( ( key == FL_Control_L ) || ( key == FL_Control_R ) ) ctrl_key = 0;
	if( ( key == FL_Shift_L ) || (  key == FL_Shift_R ) ) shift_key = 0;

	need_redraw = 1;
    dont_pass_on = 0;
	}


if( need_redraw ) redraw();
if( dont_pass_on ) return 1;
return Fl_Double_Window::handle(e);
}

//----------------------------------------------------------















//----------------------------------------------------------
//v1.01     22-jun-2014
//v1.02     21-sep-2014     //mousewheel callback, added h and v cursors, calc_draw_coords(..), get_selected_value_using_id(..), lissajous
//v1.03     18-nov-2014     //added: st_mgraph_draw_obj_tag
//v1.04     04-dec-2014     //added auto y scale limits, see limit_auto_scale_min_for_y, limit_auto_scale_max_for_y   
//v1.05     09-feb-2015     //added vtrace_id  
//v1.06     29-may-2015     //moved 'show_as_spectrum' and 'spectrum_baseline_y' to: struct 'trace_tag'  
//v1.07     30-may-2017     //added mousemove callback with normalised pos values: (left)-1.0 --> 0.0 --> +1.0 (right), (botm)-1.0 --> 0.0 --> +1.0 (top)
							//handy public var now avail, e.g: (int)last_mousex, (double)last_nomalised_mousex
//v1.08     21-dec-2017     //moded set_selected_sample() and added option 'issue_which_callback == 3' for left_click_anywhere_cb_p_callback() and
							//also added option 'issue_which_callback == 4' for right_click_anywhere_cb_p_callback()

mgraph::mgraph( int x, int y, int w, int h, const char *label ) : Fl_Box( x, y, w, h, label )
{
string s1;

graph_id = 0;

left_button = 0;
right_button = 0;
middle_button = 0;

b_invert_wheel = 0;
mousewheel = 0;

inside_control = 0;

rect_size = 10 / 2;

woffx = x;
woffy = y;
double_click_left = 0;

bkgd_border_left = 0;
bkgd_border_top = 0;
bkgd_border_right = 0;
bkgd_border_bottom = 0;


graticule_border_right = 0;
graticule_border_top = 0;
graticule_border_right = 0;
graticule_border_bottom = 0;

border_col.r = 0;
border_col.g = 0;
border_col.b = 0;

background.r = 64;
background.g = 64;
background.b = 64;


cro_graticle = 0;

idx_maxy = -1;
idx_maxx = -1;

//show_as_spectrum = 0;
//spectrum_baseline_y = 0;
htime_lissajous = 0;

//cursor params
cursor_sel = 0;

show_cursors_along_y = 0;           //cursors along y-axis (i.e. measuring time)
cursor_along_y0 = 10;
cursor_along_y1 = 100;

show_cursors_along_x = 0;           //cursors along x-axis (i.e. measuring amplitude)
cursor_along_x0 = 1;
cursor_along_x1 = -2;

show_vert_units_div = 0;
show_horiz_units_div = 0;

last_clicked_trace = -1;
hover_trace_idx = -1;

//cursor default col
col_cursor_along_y.r = 180;
col_cursor_along_y.g = 100;
col_cursor_along_y.b = 120;

col_cursor_along_x.r = 120;
col_cursor_along_x.g = 150;
col_cursor_along_x.b = 220;

//load default units/div text obj
text_vert_units_div.x = 10;
text_vert_units_div.y = 10;
strpf( s1, "V/div: ??" );
text_vert_units_div.text = s1;
text_vert_units_div.r = 255;
text_vert_units_div.g = 255;
text_vert_units_div.b = 0;
text_vert_units_div.font = 0;
text_vert_units_div.size = 9;

text_horiz_units_div.x = 120;
text_horiz_units_div.y = 10;
strpf( s1, "H/div: ??" );
text_horiz_units_div.text = s1;
text_horiz_units_div.r = 255;
text_horiz_units_div.g = 255;
text_horiz_units_div.b = 0;
text_horiz_units_div.font = 0;
text_horiz_units_div.size = 9;

//load default cursor text obj
text_cursor_along_x.x = 250;
text_cursor_along_x.y = 10;
strpf( s1, "cV: ??" );
text_cursor_along_x.text = s1;
text_cursor_along_x.r = 255;
text_cursor_along_x.g = 255;
text_cursor_along_x.b = 0;
text_cursor_along_x.font = 0;
text_cursor_along_x.size = 9;

text_cursor_along_y.x = 350;
text_cursor_along_y.y = 10;
strpf( s1, "vH: ??" );
text_cursor_along_y.text = s1;
text_cursor_along_y.r = 255;
text_cursor_along_y.g = 255;
text_cursor_along_y.b = 0;
text_cursor_along_y.font = 0;
text_cursor_along_y.size = 9;


left_click_anywhere_cb_p_callback = 0;
left_double_click_anywhere_cb_p_callback = 0;
left_click_release_cb_p_callback = 0;

right_click_anywhere_cb_p_callback = 0;
right_double_click_anywhere_cb_p_callback = 0;
right_click_release_cb_p_callback = 0;

mouseenter_cb_p_callback = 0;
mouseleave_cb_p_callback = 0;

use_line_clip = 1;              //set this to activate:  line_clip(..)              
clip_minx = -5000;              //puts some limits on line plotting attempts, see: line_clip(..)
clip_maxx = 5000;

clip_miny = -5000;
clip_maxy = 5000;


//clip_wdg_minx;				// will included widget x,y offset and borders [ set in draw() ], used for sample point rectangle hints
//clip_wdg_maxx;
//clip_wdg_miny;	
//clip_wdg_maxy;

mousemove_cnt = 4;				//set this to a value that won't show sample rect hinting on first draw
drw_cnt = 0;	


for( int i = 0; i < cn_mgraph_trace_max; i++ )
	{
	sample_rect_showing[i] = 0;
	}

}



mgraph::~mgraph()
{
}



void mgraph::clear_traces()
{
trce.clear();
//redraw();
}








void mgraph::clear_trace( int idx )
{
if( idx < 0 ) return;
if( idx >= trce.size() ) return;

trce.erase( trce.begin() + idx  );

//redraw();
}






//double trace_max = 0;

void mgraph::add_trace( trace_tag &tr )
{

if( tr.pnt.size() == 0 ) return;

if ( trce.size() >= cn_mgraph_trace_max ) return;

tr.minx = tr.pnt[ 0 ].x;
tr.maxx = tr.pnt[ 0 ].x;

tr.miny = tr.pnt[ 0 ].y;
tr.maxy = tr.pnt[ 0 ].y;

idx_maxx = -1;
idx_maxy = -1;

for( int j = 0; j < tr.pnt.size(); j++ )		//work out mins,maxs
	{
	if( tr.pnt[ j ].x <  tr.minx )  tr.minx = tr.pnt[ j ].x;
	if( tr.pnt[ j ].x > tr.maxx )  { tr.maxx = tr.pnt[ j ].x; idx_maxx = j; }

	if( tr.pnt[ j ].y <  tr.miny ) tr.miny = tr.pnt[ j ].y;
	if( tr.pnt[ j ].y > tr.maxy ) { tr.maxy = tr.pnt[ j ].y; idx_maxy = j; }
	}

tr.left_click_cb_p_callback = 0;
tr.left_double_click_cb_p_callback = 0;

tr.middle_click_cb_p_callback = 0;

tr.right_click_cb_p_callback = 0;
tr.right_double_click_cb_p_callback = 0;


tr.keydown_cb_p_callback = 0;
tr.keyup_cb_p_callback = 0;
tr.mousewheel_cb_p_callback = 0;
tr.mousemove_cb_p_callback = 0;


tr.selected_idx = -1;
tr.hover_sample_idx = -1;

tr.mousex_low_sample_idx = -1;
tr.mousex_high_sample_idx = -1;


trce.push_back( tr );
}









//finds and returns trace's index, if matching id is found
//else returns -1;

//note: the index a trace has is dependant on when it was push_back'd and can vary, the id though is fixed

int mgraph::find_trace_idx_from_trace_id( int id )
{
if( trce.size() == 0 ) return -1;

for( int i = 0; i < trce.size(); i++ )
    {
    if( trce[ i ].id == id ) return i;
    }

return -1;
}





//using 'trace' idx, get its trace_id
//returns trace_id is successful, else returns -1;

//note: the index a trace has is dependant on when it was push_back'd and can vary, the id though is fixed

int mgraph::find_trace_id_from_trace_idx( int trc )
{
if( trc < 0 ) return -1;
if( trc >= trce.size() ) return -1;

return trce[ trc ].id;
}











void mgraph::set_trace_visibility( int idx, bool visible )
{
if( idx >= trce.size() ) return;

trce[ idx ].vis = visible;

}








void mgraph::get_trace_maxx( int trc, int &idx, double &x )
{
idx = -1;

if( trc >= trce.size() ) return;

idx = idx_maxx;

double xmin, xmax, ymin, ymax;

get_trace_min_max( trc, xmin, xmax, ymin, ymax );

x = xmax;
}











void mgraph::get_trace_maxy( int trc, int &idx, double &y )
{
idx = -1;

if( trc >= trce.size() ) return;

idx = idx_maxy;

double xmin, xmax, ymin, ymax;

get_trace_min_max( trc, xmin, xmax, ymin, ymax );

y = ymax;
}











void mgraph::get_trace_min_max( int idx, double &xmin, double &xmax, double &ymin, double &ymax )
{
xmin = 0;
xmax = 0;
ymin = 0;
ymax = 0;

if( idx >= trce.size() ) return;

xmin = trce[ idx ].minx;
xmax = trce[ idx ].maxx;
ymin = trce[ idx ].miny;
ymax = trce[ idx ].maxy;

//cslpf( 5, "trace freq min: %lf, max: %lf\n", xmin, xmax );
}






void mgraph::get_trace_midxy( int idx, double &midx, double &midy )
{
midx = 0;
midy = 0;

if( idx >= trce.size() ) return;

midx = (trce[ idx ].maxx - trce[ idx ].minx) / 2.0 + trce[ idx ].minx;
midy = (trce[ idx ].maxy - trce[ idx ].miny) / 2.0 + trce[ idx ].miny;

//cslpf( 5, "trace freq min: %lf, max: %lf\n", xmin, xmax );
}






//get spec trace's sample count
//returns 1 on success, else 0
bool mgraph::get_sample_count_for_trc( int trc, int &count )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


count = trce[ trc ].pnt.size();
return 1;
}






//search traces for matching trc_id, get its samples count
//returns 1 on success, else 0
bool mgraph::get_sample_count_using_id( int trc_id, int &count )
{
count = 0;

if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
    if( trce[ i ].id == trc_id )
        {
        count = trce[ i ].pnt.size();
        return 1;
        }
    }
return 0;
}





//searches traces to find if the first one that has a selected sample, also gets the sample idx,
//NOTE: its possible that more than one trace has a sample selected,
//return trace index else -1 if none selected
int mgraph::get_selected_trace_and_sample_indexes( int &samp_idx )
{

for( int i = 0; i < trce.size(); i++ )
	{
	if( get_selected_idx( i, samp_idx ) )
		{
		return i;
		}
	}

return -1;
}




//for spec trace (not via trace_id, get its selected sample index count, see also:  get_selected_value_using_id(..)
//returns 1 on success, else 0
bool mgraph::get_selected_value( int trc, double &xx, double &yy )
{
xx = yy = 0;

if ( trce.size() == 0 ) return 0;


if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int idx = trce[ trc ].selected_idx;

if ( idx == -1 ) return 0;

xx = trce[ trc ].pnt[ idx ].x;
yy = trce[ trc ].pnt[ idx ].y;

return 1;

}





//this cycles all traces looking for matching id, if found, it then checks if trace has a selection,
//if so it returns selected sample's details
//id's are used incase ordering of push_back'd traces is not known
bool mgraph::get_selected_value_using_id( int trc_id, int &idx, double &xx, double &yy )
{
xx = yy = 0;

if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
    if( trce[ i ].id == trc_id )
        {
        idx = trce[ i ].selected_idx;

        if ( idx == -1 ) return 0;

        xx = trce[ i ].pnt[ idx ].x;
        yy = trce[ i ].pnt[ idx ].y;

        return 1;
        }
    }
return 0;
}










//this cycles all traces looking for matching id, if found, it then checks if trace has a selection,
//if so it returns hover sample's details
//id's are used incase ordering of push_back'd traces is not known
bool mgraph::get_hover_value_using_id( int trc_id, int &idx, double &xx, double &yy )
{
xx = yy = 0;

if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
    if( trce[ i ].id == trc_id )
        {
        idx = trce[ i ].hover_sample_idx;

        if ( idx == -1 ) return 0;

        xx = trce[ i ].pnt[ idx ].x;
        yy = trce[ i ].pnt[ idx ].y;

        return 1;
        }
    }
return 0;
}








//for spec trace (not via trace_id), get its selected sample index, see also:  get_selected_value_using_id(..)
//returns 1 on success, else 0
bool mgraph::get_selected_idx( int trc, int &sel_idx )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

sel_idx = trce[ trc ].selected_idx;
if( sel_idx == -1 ) return 0;

return 1;
}








//for spec trace (not via trace_id), get its spec sample values
//returns 1 on success, else 0
bool mgraph::get_sample_value( int trc, int idx, double &valx, double &valy )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

if( idx < 0 ) return 0;
if( idx >= trce[ trc ].pnt.size() ) return 0;

valx = trce[ trc ].pnt[ idx ].x;
valy = trce[ trc ].pnt[ idx ].y;

return 1;
}






//get leftmost visible sample index, NOTE: will produce out of bound indexes, so you need to check index is within 'get_sample_count()' limits
//returns 1 on success, else 0
bool mgraph::get_left_edge_sample_idx( int trc, int &left_idx )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int count;
if( !get_sample_count_for_trc( trc, count ) ) return 0;

int trc_reductionx = trce[trc].border_right + trce[trc].border_left;
int trc_reductiony = trce[trc].border_bottom + trce[trc].border_top;


int bkgd_reductionx = bkgd_border_right + bkgd_border_left + trc_reductionx;
int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top + trc_reductiony;

int bkgd_wid = w() - bkgd_reductionx;
int bkgd_hei = h() - bkgd_reductiony;

left_idx = nearbyint( (-trce[ trc ].posx / bkgd_wid * count ) / trce[ trc ].scalex );
//printf("left_idx: %d\n", left_idx );
//printf("bkgd_wid: %d, posx: %f, count %d\n", bkgd_wid, trce[ trc ].posx, count );
//left_idx *= trce[ trc ].scalex;
return 1;
}






//returns 1 on success, else 0
bool mgraph::set_left_edge_sample_idx( int trc, int left_idx )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int count;
if( !get_sample_count_for_trc( trc, count ) ) return 0;

if( left_idx < 0 ) left_idx = 0;

int wid, hei;
//get_vis_dimensions( wid, hei );
get_trace_vis_dimensions( trc, wid, hei );

trce[ trc ].posx = -(float)left_idx / count * wid * trce[ trc ].scalex;
return 1;
}










//get rightmost visible sample, NOTE: will produce out of bound indexes, so you need to check index is within 'get_sample_count()' limits
//returns 1 on success, else 0
bool mgraph::get_right_edge_sample_idx( int trc, int &right_idx )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int count;
if( !get_sample_count_for_trc( trc, count ) ) return 0;

int trc_reductionx = trce[trc].border_right + trce[trc].border_left;
int trc_reductiony = trce[trc].border_bottom + trce[trc].border_top;

int bkgd_reductionx = bkgd_border_right + bkgd_border_left + trc_reductionx;
int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top + trc_reductiony;

int bkgd_wid = w() - bkgd_reductionx;
int bkgd_hei = h() - bkgd_reductiony;

right_idx = nearbyint( ( (bkgd_wid - trce[ trc ].posx) / bkgd_wid * count ) / trce[ trc ].scalex );


return 1;
}





//get the actual background dimensions in pixels (i.e: borders removed)
void mgraph::get_background_dimensions( int &wid, int &hei )
{
int bkgd_reductionx = bkgd_border_right + bkgd_border_left;
int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top;

wid = w() - bkgd_reductionx;
hei = h() - bkgd_reductiony;
}






//get the actual graticle dimensions in pixels (i.e: borders removed)
void mgraph::get_graticle_dimensions( int &wid, int &hei )
{
int graticule_reductionx = graticule_border_right + graticule_border_left;
int graticule_reductiony = graticule_border_bottom + graticule_border_top;

wid = w() - (bkgd_border_right + bkgd_border_left + graticule_reductionx);
hei = h() - (bkgd_border_bottom + bkgd_border_top + graticule_reductiony);
}







//get the actual trace drawable dimensions in pixels (i.e: borders removed)
bool mgraph::get_trace_vis_dimensions( int trc, int &wid, int &hei )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int trc_reductionx = trce[trc].border_right + trce[trc].border_left;
int trc_reductiony = trce[trc].border_bottom + trce[trc].border_top;

wid = w() - (bkgd_border_right + bkgd_border_left + trc_reductionx);
hei = h() - (bkgd_border_bottom + bkgd_border_top + trc_reductiony);

return 1;
}


















// !!! NOTE: see code EXAMPLE below on how to use this function
//horizontally scales a trace by spec amount,
//'zoom_idx' - is the sample idx where zoom expand/contracts from, this sample will roughly stay at its current x position,
//'scale_change' - is the relative amount to scale by (not absolute), 1.0 is no change, 1.5 changes to current scale by 1.5 times,
//'force_extent_when_contracted_beyond' - if set, and the new scale would be below 1.0, the final scale is forced to 1.0, and posx is set to 0, so trace is fully visible and centred

/*
//EXAMPLE CODE - mousewheel changes zoom scale focussed at the currently selected sample

float gph0_scale_change = 1.0;													//global/static var


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

// !!! EXAMPLE CODE  is above
bool mgraph::zoom_h( int trc, int zoom_idx, float scale_change, bool force_extent_when_contracted_beyond )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

if( force_extent_when_contracted_beyond )
	{
	if( ( scale_change * trce[ trc ].scalex ) < 1.0 )
		{
		trce[ trc ].scalex = 1.0;
		trce[ trc ].posx = 0;

		if( trce[ trc ].selected_idx != -1 )
			{
			set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
			}

		redraw();
		return 1;
		}
	}

int wid, hei;
//get_vis_dimensions( wid, hei );
get_trace_vis_dimensions( trc, wid, hei );



int left_idx;
if( !get_left_edge_sample_idx( trc, left_idx ) ) return 0;


int right_idx;
if( !get_right_edge_sample_idx( trc, right_idx ) ) return 0;

//printf("zoom_h() left_idx: %d, right_idx: %d\n", left_idx, right_idx );


float new_total_sample_wnd = ( right_idx - left_idx ) / scale_change;

float samples_left_of_mouse = ( zoom_idx - left_idx ) / scale_change;


float new_left_sample = zoom_idx - 0 - samples_left_of_mouse;

//printf("zoom_h() zoom_idx %d new_total_sample_wnd: %f, new_left_sample: %f  samples_left_of_mouse %f\n", zoom_idx, new_total_sample_wnd, new_left_sample, samples_left_of_mouse );


//int posx = -(float)new_left_sample / new_total_sample_wnd * wid;

//printf("zoom_h() - scalex %f  scale_change %f\n", trce[ trc ].scalex, scale_change );

trce[ trc ].scalex *= scale_change;

//printf("zoom_h() - trce[ trc ].posx %f\n", trce[ trc ].posx );
//printf("zoom_h() - scalex %f  scale_change %f\n", trce[ trc ].scalex, scale_change );

set_left_edge_sample_idx( trc, new_left_sample );

//trce[ trc ].posx = +14;//trce[ trc ].minx;
//set_left_edge_sample_idx( trc, 0 );

//printf("zoom_h() trc: %d, new_left_sample: %f\n", trc, new_left_sample );

if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}

redraw();
return 1;
}







// !!! EXAMPLE CODE  is above, see 'zoom_h()'
//horizontally scales all traces by spec amount,
//all traces SHOULD have same number of samples, so that each trace's 'zoom_idx' is the same sample relative to all traces
void mgraph::zoom_h_all_traces( int zoom_idx, float scale_change, bool force_extent_when_contracted_beyond )
{

//int wid, hei;
//get_vis_dimensions( wid, hei );

for( int i = 0; i < trce.size(); i++ )
    {
	zoom_h( i, zoom_idx, scale_change, force_extent_when_contracted_beyond );
	}

}





bool mgraph::center_on_sample( int trc, int sample_idx )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int count;

if( !mgraph::get_sample_count_for_trc( trc, count ) ) return 0;

if( count == 0 ) return 0;
if( sample_idx >= count ) return 0 ;

int wid, hei;
//get_vis_dimensions( wid, hei );
get_trace_vis_dimensions( trc, wid, hei );

float samples_vis = (count - 1) / trce[ trc ].scalex;

//printf("wid: %d\n", wid );
//printf("samples_vis: %f\n", samples_vis );

float left_sample = (sample_idx) - samples_vis / 2.0;

//printf("left_sample: %f\n", left_sample );

//set_left_edge_sample_idx( trc, left_sample );

float posx = -(float)left_sample / samples_vis * wid;
//printf("trc: %d, posx: %f\n", trc, posx );
trce[ trc ].posx = posx;


if( trce[ trc ].selected_idx != -1 )							//added v1.14
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	redraw();
	}

return 1;
}






//returns -1 if not hoveting over a trace sample
int mgraph::get_hover_trace_idx()
{
return hover_trace_idx;
}






//for spec trace (not via trace_id), get its hover sample index, see also:  get_hover_value_using_id(..)
//returns 1 on success, else 0
bool mgraph::get_hover_idx( int trc, int &hov_idx )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

hov_idx = trce[ trc ].hover_sample_idx;
if( hov_idx == -1 ) return 0;

return 1;
}








//for spec trace (not via trace_id), get 2 sample index the mousex value is sitting between, see also:  get_mousex_sample_boundary_idx_using_id(..)
//returns 1 on success, else 0
bool mgraph::get_mousex_sample_boundary_idx( int trc, int &low_idx, int &high_idx )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

if( trce[ trc ].pnt.size() < 2 ) return 0;

low_idx = trce[ trc ].mousex_low_sample_idx;
high_idx = trce[ trc ].mousex_high_sample_idx;

if( low_idx < 0 ) return 0;
if( high_idx < 0 ) return 0;

return 1;
}




//for spec trace_id, get 2 sample index the mousex value is sitting between, see also:  get_mousex_sample_boundary_idx(..)
//returns 1 on success, else 0
bool mgraph::get_mousex_sample_boundary_idx_using_id( int trc_id, int &low_idx, int &high_idx )
{
if ( trce.size() == 0 ) return 0;

for( int i = 0; i < trce.size(); i++ )
    {
    if( trce[ i ].id == trc_id )
        {
		low_idx = trce[ i ].mousex_low_sample_idx;
		high_idx = trce[ i ].mousex_high_sample_idx;

		if( low_idx < 0 ) return 0;
		if( high_idx < 0 ) return 0;

        return 1;
        }
    }
return 0;
}






//issue_which_callback == 1, will call: left_click_cb_p_callback(..)
//issue_which_callback == 2, will call: right_click_cb_p_callback(..)
//issue_which_callback == 3, will call: left_click_anywhere_cb_p_callback(..)
//issue_which_callback == 4, will call: right_click_anywhere_cb_p_callback(..)

void mgraph::deselect_sample( int trc, int issue_which_callback )
{
if ( trce.size() == 0 ) return;


if ( trc < 0 ) return;
if( trc >= trce.size() ) return;


trce[ trc ].selected_idx = -1;


if( issue_which_callback != 0 )
	{
	if( issue_which_callback == 1 )
		{
		if( trce[ trc ].left_click_cb_p_callback )  trce[ trc ].left_click_cb_p_callback( trce[ trc ].left_click_cb_args );
		}

	if( issue_which_callback == 2 )
		{
		if( trce[ trc ].right_click_cb_p_callback )  trce[ trc ].right_click_cb_p_callback( trce[ trc ].right_click_cb_args );
		}


	if( issue_which_callback == 3 )
		{
		if( left_click_anywhere_cb_p_callback )  left_click_anywhere_cb_p_callback( left_click_anywhere_cb_args );
		}


	if( issue_which_callback == 4 )
		{
		if( right_click_anywhere_cb_p_callback )  right_click_anywhere_cb_p_callback( right_click_anywhere_cb_args );
		}
	}

}








//issue_which_callback == 1, will call: left_click_cb_p_callback(..)
//issue_which_callback == 2, will call: right_click_cb_p_callback(..)
//issue_which_callback == 3, will call: left_click_anywhere_cb_p_callback(..)
//issue_which_callback == 4, will call: right_click_anywhere_cb_p_callback(..)

void mgraph::deselect_all_samples( int issue_which_callback )
{
if ( trce.size() == 0 ) return;

for( int i = 0; i < trce.size(); i++ )
    {
	deselect_sample( i, issue_which_callback );
	}

}







//this a pixel offset value (not relative to sample values), positive means trace is rightward on screen
bool mgraph::get_posx( int trc, double &val )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

val = trce[ trc ].posx;

return 1;
}






//this value is relative to sample values (not pixel), positive means trace is rightward on screen
bool mgraph::get_posx_relative_to_trace( int trc, double &val )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

double scalex, minx, maxx;

scalex = trce[ trc ].scalex;
minx = trce[ trc ].minx;
maxx = trce[ trc ].maxx;

double vis_range = (maxx - minx) / scalex;

int wid, hei;
get_trace_vis_dimensions( trc, wid, hei );

double posx = trce[ trc ].posx;

double ratio = posx / wid;

val = vis_range * ratio;
//printf("posx: %f\n", trce[ trc ].posx);
return 1;
}











//this a pixel offset value (not relative to sample values), positive means trace is up-screen
bool mgraph::get_posy( int trc, double &val )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

val = trce[ trc ].posy;

return 1;
}




//this value is relative to sample values (not pixel), positive means trace is up-screen
bool mgraph::get_posy_relative_to_trace( int trc, double &val )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

double scaley, miny, maxy;

scaley = trce[ trc ].scaley;
miny = trce[ trc ].miny;
maxy = trce[ trc ].maxy;

double vis_range = (maxy - miny) / scaley;

int wid, hei;
get_trace_vis_dimensions( trc, wid, hei );

double posy = trce[ trc ].posy;

double ratio = posy / hei;

val = vis_range * ratio;
//printf("posy: %f\n", trce[ trc ].posy);
return 1;
}




bool mgraph::get_scalex( int trc, double &val )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

val = trce[ trc ].scalex;

return 1;
}




bool mgraph::get_scaley( int trc, double &val )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

val = trce[ trc ].scaley;

return 1;
}







bool mgraph::adj_posx_by( int trc, double delta, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].posx += delta;



if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}






bool mgraph::adj_posx_all_traces_by( double delta, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
	adj_posx_by( i, delta, 0 );
	}

if( need_redraw ) redraw();

return 1;
}














bool mgraph::adj_posy_by( int trc, double delta, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].posy += delta;


if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}









bool mgraph::adj_posy_all_traces_by( double delta, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
	adj_posy_by( i, delta, 0 );
	}

if( need_redraw ) redraw();

return 1;
}











//delta is multiplicative
bool mgraph::adj_scalex_by( int trc, double delta, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].scalex *= delta;


if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}









//delta is multiplicative
bool mgraph::adj_scalex_all_traces_by( double delta, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
	adj_scalex_by( i, delta, 0 );
	}

if( need_redraw ) redraw();

return 1;
}















//delta is multiplicative
bool mgraph::adj_scaley_by( int trc, double delta, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].scaley *= delta;


if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}









//delta is multiplicative
bool mgraph::adj_scaley_all_traces_by( double delta, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
	adj_scaley_by( i, delta, 0 );
	}

if( need_redraw ) redraw();

return 1;
}










//see also: 'adj_posx_by()'
bool mgraph::set_posx( int trc, double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].posx = val;



if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}






//see also: 'adj_posx_all_traces_by()'
bool mgraph::set_posx_all_traces( double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
	adj_posx_by( i, val, 0 );
	}

if( need_redraw ) redraw();

return 1;
}













//see also: 'adj_posy_by()'
bool mgraph::set_posy( int trc, double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].posy = val;



if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}






//see also: 'adj_posy_all_traces_by()'
bool mgraph::set_posy_all_traces( double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
	adj_posy_by( i, val, 0 );
	}

if( need_redraw ) redraw();

return 1;
}










//see also: 'adj_scalex_by()'
bool mgraph::set_scalex( int trc, double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].scalex = val;


if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}









//see also: 'adj_scalex_all_traces_by()'
bool mgraph::set_scalex_all_traces( double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
	set_scalex( i, val, 0 );
	}

if( need_redraw ) redraw();

return 1;
}











//see also: 'adj_scaley_by()'
bool mgraph::set_scaley( int trc, double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].scaley = val;


if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}









//see also: 'adj_scaley_all_traces_by()'
bool mgraph::set_scaley_all_traces( double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;


for( int i = 0; i < trce.size(); i++ )
    {
	set_scaley( i, val, 0 );
	}

if( need_redraw ) redraw();

return 1;
}




//see also: 'adj_plot_offsx_by()'
bool mgraph::set_plot_offsx( int trc, double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].plot_offsx = val;



if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}






//see also: 'set_plot_offsx_by()'
bool mgraph::adj_plot_offsx_by( int trc, double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].plot_offsx += val;



if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}









//this a pixel offset value (not relative to sample values), positive means trace is up-screen
bool mgraph::get_plot_offsy( int trc, int &val )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

val = trce[ trc ].plot_offsy;

return 1;
}














//see also: 'adj_plot_offsy_by()'
bool mgraph::set_plot_offsy( int trc, double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].plot_offsy = val;



if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}






//see also: 'set_plot_offsy()'
bool mgraph::adj_plot_offsy_by( int trc, double val, bool need_redraw )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


trce[ trc ].plot_offsy += val;



if( trce[ trc ].selected_idx != -1 )
	{
	set_selected_sample( trc, trce[ trc ].selected_idx, 0 );	//refresh sel sample coords as scale may have changed
	}
else{
	if( need_redraw ) redraw();
	}

return 1;
}








/*
//will call a callback if one is set, which callback depends on value of issue_which_callback,
//issue_which_callback == 1, will call: left_click_cb_p_callback(..)
//issue_which_callback == 2, will call: right_click_cb_p_callback(..)
//issue_which_callback == 3, will call: left_click_anywhere_cb_p_callback(..)
//issue_which_callback == 4, will call: right_click_anywhere_cb_p_callback(..)
void mgraph::set_selected_sample( int trc, int sel_sample_idx, int issue_which_callback )
{
if ( trce.size() == 0 ) return;


if ( trc < 0 ) return;
if( trc >= trce.size() ) return;

if( sel_sample_idx < 0 ) return;
if( sel_sample_idx >= trce[ trc ].pnt.size() ) return;


int i = trc;

int wid = w();
int hei = h();


//int bkgd_reductionx = bkgd_border_right + bkgd_border_left;
//int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top;

int bkgd_offx;// = woffx + bkgd_border_left;
int bkgd_offy;// = woffy + bkgd_border_top;

int bkgd_wid;// = wid - bkgd_reductionx;
int bkgd_hei;// = hei - bkgd_reductiony;

get_background_offsxy( bkgd_offx, bkgd_offy );

get_background_dimensions( bkgd_wid, bkgd_hei );

int bkgd_midx = bkgd_wid / 2;
int bkgd_midy = bkgd_hei / 2;


double x, y;



//calc trace sample point locations
int trace_offx;
int trace_offy;
int trace_wid;
int trace_hei;


//int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
//trace_offx = woffx + bkgd_border_left + trce[ i ].border_left;
//trace_offy = woffy + bkgd_border_top + trce[ i ].border_top;

//trace_wid = wid - trace_reductionx;
//trace_hei = hei - trace_reductiony;


get_trace_offsxy( i, trace_offx, trace_offy );

get_trace_vis_dimensions( 0, trace_wid, trace_hei );

double trace_midx = trace_wid / 2;
double trace_midy = trace_hei / 2;

int cutoffx = trace_wid;					//stop drawing if x gets to this


double deltax, deltay;
double scalex, scaley;
double d_midx, d_midy;


	double positionx = trce[ i ].posx;
	double positiony = -trce[ i ].posy;


	deltax = trce[ i ].maxx - trce[ i ].minx;
	deltay = trce[ i ].maxy - trce[ i ].miny;


	if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = (double)trace_wid / deltax;
			d_midx = deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = grat_pixels_x;
			d_midx = 0;//deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
		{
		scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
		d_midx = (double)trace_midx / scalex;
		}





	if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = (double)trace_hei / deltay;
			d_midy = deltay / 2.0 + trce[ i ].miny;		//this will move ypos to centre the trace
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}



        //enforce limits to auto y scaling if req
        if( trce[ i ].b_limit_auto_scale_max_for_y ) 
            {
            if( scaley > trce[ i ].limit_auto_scale_max_for_y ) scaley = trce[ i ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
            }
            
        if( trce[ i ].b_limit_auto_scale_min_for_y )
            {
            if( scaley < trce[ i ].limit_auto_scale_min_for_y ) scaley = trce[ i ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
            }
            
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
		}



	if( trce[ i ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = grat_pixels_y;
			d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}
		}


	if( trce[ i ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
		{
		double larg_y;
		if( fabs( trce[ i ].maxy ) > fabs( trce[ i ].miny ) ) larg_y = trce[ i ].maxy;
		else larg_y = trce[ i ].miny;
		
		scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
		d_midy = 0;
		}


	if( trce[ i ].yunits_perpxl > -1 )					//use user spec scale
		{
		scaley = 1.0 / trce[ i ].yunits_perpxl;			//scale as directed
		d_midy = 0;
		}

	if( i == 0 )										//1st trace?
		{
		ref_scalex = scalex;							//remember x scale
		ref_d_midx = d_midx;
		}


int j = sel_sample_idx;

double x1 = trce[ i ].pnt[ j ].x;
double y1 = trce[ i ].pnt[ j ].y;

//x = x1 * scalex - d_midx * scalex;
//y = y1 * scaley - d_midy * scaley;


double trace_scalex = trce[ i ].scalex;
double trace_scaley = trce[ i ].scaley;


x = x1 * scalex * trace_scalex - d_midx * scalex;
y = y1 * scaley * trace_scaley - d_midy * scaley;


int xx = nearbyint( trace_midx + x + trace_offx + positionx );
int yy = nearbyint( trace_midy - y + trace_offy + positiony );

rect_size = 10 / 2;

int left =  xx - rect_size / 2;
int top =  yy - rect_size / 2;


int right =  left + rect_size;
int bot = top + rect_size;

trce[ i ].selected_idx = j;
trce[ i ].selected_pixel_rect_left = left;
trce[ i ].selected_pixel_rect_wid = rect_size;
trce[ i ].selected_pixel_rect_top = top;
trce[ i ].selected_pixel_rect_hei = rect_size;

if( issue_which_callback != 0 )
	{
	if( issue_which_callback == 1 )
		{
		if( trce[ i ].left_click_cb_p_callback )  trce[ i ].left_click_cb_p_callback( trce[ i ].left_click_cb_args );
		}

	if( issue_which_callback == 2 )
		{
		if( trce[ i ].right_click_cb_p_callback )  trce[ i ].right_click_cb_p_callback( trce[ i ].right_click_cb_args );
		}


	if( issue_which_callback == 3 )
		{
		if( left_click_anywhere_cb_p_callback )  left_click_anywhere_cb_p_callback( left_click_anywhere_cb_args );
		}


	if( issue_which_callback == 4 )
		{
		if( right_click_anywhere_cb_p_callback )  right_click_anywhere_cb_p_callback( right_click_anywhere_cb_args );
		}
	}


redraw();
}
*/










//make factor and offset values used to suitable to make plot values
bool mgraph::get_trace_to_plot_factors( int trc, double &trace_scalex, double &d_midx, double &scalex, double &trace_midx, int &trace_offx, double &positionx, int &plot_offsx,   double &trace_scaley, double &d_midy, double &scaley, double &trace_midy, int &trace_offy, double &positiony, int &plot_offsy )
{
if ( trce.size() == 0 ) return 0;


if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int i = trc;

int wid = w();
int hei = h();


//int bkgd_reductionx = bkgd_border_right + bkgd_border_left;
//int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top;

int bkgd_offx;// = woffx + bkgd_border_left;
int bkgd_offy;// = woffy + bkgd_border_top;

int bkgd_wid;// = wid - bkgd_reductionx;
int bkgd_hei;// = hei - bkgd_reductiony;

get_background_offsxy( bkgd_offx, bkgd_offy );

get_background_dimensions( bkgd_wid, bkgd_hei );

int bkgd_midx = bkgd_wid / 2;
int bkgd_midy = bkgd_hei / 2;



//calc trace sample point locations
int trace_wid;
int trace_hei;


//int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
//trace_offx = woffx + bkgd_border_left + trce[ i ].border_left;
//trace_offy = woffy + bkgd_border_top + trce[ i ].border_top;

//trace_wid = wid - trace_reductionx;
//trace_hei = hei - trace_reductiony;


plot_offsx = trce[ i ].plot_offsx;					//not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
plot_offsy = -trce[ i ].plot_offsy;




int idx3, idx4;
idx3 = idx4 = i;

//set up referrence overrides
int z = trce[ i ].use_pos_y;                         	//get trace id to use, if any
int idx = get_trace_idx_from_id( z );           		//use id to get trace needed
if( idx != -1 )
	{
	if( idx != i  )
		{
		idx3 = idx;

//		positiony = trce[ idx ].posy;
//		vtrace_midx[ i ] = vtrace_midx[ idx ];
//		vtrace_offx[ i ] = vtrace_offx[ idx ];
		}
	}

z = trce[ i ].use_scale_y;                         		//get trace id to use as the ref, if any
idx = get_trace_idx_from_id( z );           			//use id to get trace needed
if( idx != -1  )
	{
	if( idx != i )										//ignore trace that matches ref
		{
		idx4 = idx;
//		vscaley[ i ] = vscaley[ idx ];
//		vtrace_scale_y[ i ] = vtrace_scale_y[ idx ];
//		vtrace_midy[ i ] = vtrace_midy[ idx ];
//		vtrace_dmidy[ i ] = vtrace_dmidy[ idx ];
		}
	}


get_trace_offsxy( idx3, trace_offx, trace_offy );

get_trace_vis_dimensions( idx3, trace_wid, trace_hei );

trace_midx = trace_wid / 2;
trace_midy = trace_hei / 2;

int cutoffx = trace_wid;					//stop drawing if x gets to this


double deltax, deltay;


positionx = trce[ i ].posx;
positiony = -trce[ idx3 ].posy;

deltax = trce[ i ].maxx - trce[ i ].minx;
deltay = trce[ idx4 ].maxy - trce[ idx4 ].miny;


if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
	{
	if( deltax != 0 )						//avoid poss div by zero
		{
		scalex = (double)trace_wid / deltax;
//		d_midx = deltax / 2.0 + trce[ i ].minx;
		d_midx = deltax / 2.0;											//v1.16
		}
	else{									//no deltax
		scalex = grat_pixels_x;
		d_midx = 0;
		}
	}


if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
	{
	if( deltax != 0 )						//avoid poss div by zero
		{
		scalex = grat_pixels_x;
		d_midx = 0;
		}
	else{									//no deltax
		scalex = grat_pixels_x;
		d_midx = 0;
		}
	}


if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
	{
	scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
	d_midx = (double)trace_midx / scalex;
	}





if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
	{
	if( deltay != 0 )							    //avoid poss div by zero
		{
		scaley = (double)trace_hei / deltay;
		d_midy = deltay / 2.0 + trce[ idx4 ].miny;		//this will move ypos to centre the trace
		}
	else{										    //no deltay
		scaley = grat_pixels_y;
		d_midy = 0;
		}



	//enforce limits to auto y scaling if req
	if( trce[ idx4 ].b_limit_auto_scale_max_for_y ) 
		{
		if( scaley > trce[ idx4 ].limit_auto_scale_max_for_y ) scaley = trce[ idx4 ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
		}
		
	if( trce[ idx4 ].b_limit_auto_scale_min_for_y )
		{
		if( scaley < trce[ idx4 ].limit_auto_scale_min_for_y ) scaley = trce[ idx4 ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
		}
		
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
	}



if( trce[ idx4 ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
	{
	if( deltay != 0 )							    //avoid poss div by zero
		{
		scaley = grat_pixels_y;
		d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
		}
	else{										    //no deltay
		scaley = grat_pixels_y;
		d_midy = 0;
		}
	}


if( trce[ idx4 ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
	{
	double larg_y;
	if( fabs( trce[ idx4 ].maxy ) > fabs( trce[ idx4 ].miny ) ) larg_y = trce[ idx4 ].maxy;
	else larg_y = trce[ idx4 ].miny;
	
	scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
	d_midy = 0;
	}


if( trce[ idx4 ].yunits_perpxl > -1 )					//use user spec scale
	{
	scaley = 1.0 / trce[ idx4 ].yunits_perpxl;			//scale as directed
	d_midy = 0;
	}

if( i == 0 )										//1st trace?
	{
	ref_scalex = scalex;							//remember x scale
	ref_d_midx = d_midx;
	}




trace_scalex = trce[ i ].scalex;
trace_scaley = trce[ idx4 ].scaley;

return 1;
}









//takes coord values relative to spec trace, and converts them to nearest integer plot values
bool mgraph::get_plot_values( int trc, double &x1, double &y1, double &x2, double &y2, double &x3, double &y3, double &x4, double &y4 )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int i = trc;

int trace_offx, trace_offy, plot_offsx, plot_offsy;
double trace_scalex, d_midx, scalex, trace_midx, positionx, trace_scaley, d_midy, scaley, trace_midy, positiony;

get_trace_to_plot_factors( i, trace_scalex, d_midx, scalex, trace_midx, trace_offx, positionx, plot_offsx,   trace_scaley, d_midy, scaley, trace_midy, trace_offy, positiony, plot_offsy );

/*

int wid = w();
int hei = h();


//int bkgd_reductionx = bkgd_border_right + bkgd_border_left;
//int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top;

int bkgd_offx;// = woffx + bkgd_border_left;
int bkgd_offy;// = woffy + bkgd_border_top;

int bkgd_wid;// = wid - bkgd_reductionx;
int bkgd_hei;// = hei - bkgd_reductiony;

get_background_offsxy( bkgd_offx, bkgd_offy );

get_background_dimensions( bkgd_wid, bkgd_hei );

int bkgd_midx = bkgd_wid / 2;
int bkgd_midy = bkgd_hei / 2;


double x, y;



//calc trace sample point locations
int trace_wid;
int trace_hei;


//int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
//trace_offx = woffx + bkgd_border_left + trce[ i ].border_left;
//trace_offy = woffy + bkgd_border_top + trce[ i ].border_top;

//trace_wid = wid - trace_reductionx;
//trace_hei = hei - trace_reductiony;






int idx3, idx4;
idx3 = idx4 = i;

//set up referrence overrides
int z = trce[ i ].use_pos_y;                         	//get trace id to use, if any
int idx = get_trace_idx_from_id( z );           		//use id to get trace needed
if( idx != -1 )
	{
	if( idx != i  )
		{
		idx3 = idx;

//		positiony = trce[ idx ].posy;
//		vtrace_midx[ i ] = vtrace_midx[ idx ];
//		vtrace_offx[ i ] = vtrace_offx[ idx ];
		}
	}

z = trce[ i ].use_scale_y;                         		//get trace id to use as the ref, if any
idx = get_trace_idx_from_id( z );           			//use id to get trace needed
if( idx != -1  )
	{
	if( idx != i )										//ignore trace that matches ref
		{
		idx4 = idx;
//		vscaley[ i ] = vscaley[ idx ];
//		vtrace_scale_y[ i ] = vtrace_scale_y[ idx ];
//		vtrace_midy[ i ] = vtrace_midy[ idx ];
//		vtrace_dmidy[ i ] = vtrace_dmidy[ idx ];
		}
	}


get_trace_offsxy( idx3, trace_offx, trace_offy );

get_trace_vis_dimensions( idx3, trace_wid, trace_hei );

trace_midx = trace_wid / 2;
trace_midy = trace_hei / 2;

int cutoffx = trace_wid;					//stop drawing if x gets to this


double deltax, deltay;


positionx = trce[ i ].posx;
positiony = -trce[ idx3 ].posy;

deltax = trce[ i ].maxx - trce[ i ].minx;
deltay = trce[ i ].maxy - trce[ i ].miny;


if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
	{
	if( deltax != 0 )						//avoid poss div by zero
		{
		scalex = (double)trace_wid / deltax;
		d_midx = deltax / 2.0 + trce[ idx3 ].minx;
		}
	else{									//no deltax
		scalex = grat_pixels_x;
		d_midx = 0;
		}
	}


if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
	{
	if( deltax != 0 )						//avoid poss div by zero
		{
		scalex = grat_pixels_x;
		d_midx = 0;//deltax / 2.0 + trce[ i ].minx;
		}
	else{									//no deltax
		scalex = grat_pixels_x;
		d_midx = 0;
		}
	}


if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
	{
	scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
	d_midx = (double)trace_midx / scalex;
	}





if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
	{
	if( deltay != 0 )							    //avoid poss div by zero
		{
		scaley = (double)trace_hei / deltay;
		d_midy = deltay / 2.0 + trce[ idx4 ].miny;		//this will move ypos to centre the trace
		}
	else{										    //no deltay
		scaley = grat_pixels_y;
		d_midy = 0;
		}



	//enforce limits to auto y scaling if req
	if( trce[ idx4 ].b_limit_auto_scale_max_for_y ) 
		{
		if( scaley > trce[ idx4 ].limit_auto_scale_max_for_y ) scaley = trce[ idx4 ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
		}
		
	if( trce[ idx4 ].b_limit_auto_scale_min_for_y )
		{
		if( scaley < trce[ idx4 ].limit_auto_scale_min_for_y ) scaley = trce[ idx4 ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
		}
		
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
	}



if( trce[ idx4 ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
	{
	if( deltay != 0 )							    //avoid poss div by zero
		{
		scaley = grat_pixels_y;
		d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
		}
	else{										    //no deltay
		scaley = grat_pixels_y;
		d_midy = 0;
		}
	}


if( trce[ idx4 ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
	{
	double larg_y;
	if( fabs( trce[ idx4 ].maxy ) > fabs( trce[ idx4 ].miny ) ) larg_y = trce[ idx4 ].maxy;
	else larg_y = trce[ i ].miny;
	
	scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
	d_midy = 0;
	}


if( trce[ idx4 ].yunits_perpxl > -1 )					//use user spec scale
	{
	scaley = 1.0 / trce[ idx4 ].yunits_perpxl;			//scale as directed
	d_midy = 0;
	}

if( i == 0 )										//1st trace?
	{
	ref_scalex = scalex;							//remember x scale
	ref_d_midx = d_midx;
	}




trace_scalex = trce[ i ].scalex;
trace_scaley = trce[ idx4 ].scaley;

*/

double x, y;

//x = x1 * scalex * trace_scalex - d_midx * scalex;
x = (x1-trce[ trc ].minx) * scalex * trace_scalex - d_midx * scalex;	//v1.16
y = y1 * scaley * trace_scaley - d_midy * scaley;

x1 = nearbyint( trace_midx + x + trace_offx + positionx + plot_offsx );
y1 = nearbyint( trace_midy - y + trace_offy + positiony + plot_offsy );

x = x2 * scalex * trace_scalex - d_midx * scalex;
y = y2 * scaley * trace_scaley - d_midy * scaley;

x2 = nearbyint( trace_midx + x + trace_offx + positionx + plot_offsx );
y2 = nearbyint( trace_midy - y + trace_offy + positiony + plot_offsy );

x = x3 * scalex * trace_scalex - d_midx * scalex;
y = y3 * scaley * trace_scaley - d_midy * scaley;

x3 = nearbyint( trace_midx + x + trace_offx + positionx + plot_offsx );
y3 = nearbyint( trace_midy - y + trace_offy + positiony + plot_offsy );

x = x4 * scalex * trace_scalex - d_midx * scalex;
y = y4 * scaley * trace_scaley - d_midy * scaley;

x4 = nearbyint( trace_midx + x + trace_offx + positionx + plot_offsx );
y4 = nearbyint( trace_midy - y + trace_offy + positiony + plot_offsy );

return 1;
}















//will call a callback if one is set, which callback depends on value of issue_which_callback,
//issue_which_callback == 1, will call: left_click_cb_p_callback(..)
//issue_which_callback == 2, will call: right_click_cb_p_callback(..)
//issue_which_callback == 3, will call: left_click_anywhere_cb_p_callback(..)
//issue_which_callback == 4, will call: right_click_anywhere_cb_p_callback(..)
void mgraph::set_selected_sample( int trc, int sel_sample_idx, int issue_which_callback )
{
if ( trce.size() == 0 ) return;


if ( trc < 0 ) return;
if( trc >= trce.size() ) return;

if( sel_sample_idx < 0 ) return;
if( sel_sample_idx >= trce[ trc ].pnt.size() ) return;






int i = trc;
/*
int wid = w();
int hei = h();


//int bkgd_reductionx = bkgd_border_right + bkgd_border_left;
//int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top;

int bkgd_offx;// = woffx + bkgd_border_left;
int bkgd_offy;// = woffy + bkgd_border_top;

int bkgd_wid;// = wid - bkgd_reductionx;
int bkgd_hei;// = hei - bkgd_reductiony;

get_background_offsxy( bkgd_offx, bkgd_offy );

get_background_dimensions( bkgd_wid, bkgd_hei );

int bkgd_midx = bkgd_wid / 2;
int bkgd_midy = bkgd_hei / 2;


double x, y;



//calc trace sample point locations
int trace_offx;
int trace_offy;
int trace_wid;
int trace_hei;


//int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
//trace_offx = woffx + bkgd_border_left + trce[ i ].border_left;
//trace_offy = woffy + bkgd_border_top + trce[ i ].border_top;

//trace_wid = wid - trace_reductionx;
//trace_hei = hei - trace_reductiony;


get_trace_offsxy( i, trace_offx, trace_offy );

get_trace_vis_dimensions( 0, trace_wid, trace_hei );

double trace_midx = trace_wid / 2;
double trace_midy = trace_hei / 2;

int cutoffx = trace_wid;					//stop drawing if x gets to this


double deltax, deltay;
double scalex, scaley;
double d_midx, d_midy;


	double positionx = trce[ i ].posx;
	double positiony = -trce[ i ].posy;


	deltax = trce[ i ].maxx - trce[ i ].minx;
	deltay = trce[ i ].maxy - trce[ i ].miny;


	if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = (double)trace_wid / deltax;
			d_midx = deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = grat_pixels_x;
			d_midx = 0;//deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
		{
		scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
		d_midx = (double)trace_midx / scalex;
		}





	if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = (double)trace_hei / deltay;
			d_midy = deltay / 2.0 + trce[ i ].miny;		//this will move ypos to centre the trace
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}



        //enforce limits to auto y scaling if req
        if( trce[ i ].b_limit_auto_scale_max_for_y ) 
            {
            if( scaley > trce[ i ].limit_auto_scale_max_for_y ) scaley = trce[ i ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
            }
            
        if( trce[ i ].b_limit_auto_scale_min_for_y )
            {
            if( scaley < trce[ i ].limit_auto_scale_min_for_y ) scaley = trce[ i ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
            }
            
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
		}



	if( trce[ i ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = grat_pixels_y;
			d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}
		}


	if( trce[ i ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
		{
		double larg_y;
		if( fabs( trce[ i ].maxy ) > fabs( trce[ i ].miny ) ) larg_y = trce[ i ].maxy;
		else larg_y = trce[ i ].miny;
		
		scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
		d_midy = 0;
		}


	if( trce[ i ].yunits_perpxl > -1 )					//use user spec scale
		{
		scaley = 1.0 / trce[ i ].yunits_perpxl;			//scale as directed
		d_midy = 0;
		}

	if( i == 0 )										//1st trace?
		{
		ref_scalex = scalex;							//remember x scale
		ref_d_midx = d_midx;
		}
*/


int j = sel_sample_idx;

double x1 = trce[ i ].pnt[ j ].x;
double y1 = trce[ i ].pnt[ j ].y;

double x2, x3, x4;
double y2, y3, y4;

get_plot_values( i, x1, y1, x2, y2, x3, y3, x4, y4 );

//x = x1 * scalex - d_midx * scalex;
//y = y1 * scaley - d_midy * scaley;

/*
double trace_scalex = trce[ i ].scalex;
double trace_scaley = trce[ i ].scaley;


x = x1 * scalex * trace_scalex - d_midx * scalex;
y = y1 * scaley * trace_scaley - d_midy * scaley;


int xx = nearbyint( trace_midx + x + trace_offx + positionx );
int yy = nearbyint( trace_midy - y + trace_offy + positiony );
*/


rect_size = 10 / 2;

int left =  x1 - rect_size / 2;
int top =  y1 - rect_size / 2;


int right =  left + rect_size;
int bot = top + rect_size;

trce[ i ].selected_idx = j;
trce[ i ].selected_pixel_rect_left = left;
trce[ i ].selected_pixel_rect_wid = rect_size;
trce[ i ].selected_pixel_rect_top = top;
trce[ i ].selected_pixel_rect_hei = rect_size;

if( issue_which_callback != 0 )
	{
	if( issue_which_callback == 1 )
		{
		if( trce[ i ].left_click_cb_p_callback )  trce[ i ].left_click_cb_p_callback( trce[ i ].left_click_cb_args );
		}

	if( issue_which_callback == 2 )
		{
		if( trce[ i ].right_click_cb_p_callback )  trce[ i ].right_click_cb_p_callback( trce[ i ].right_click_cb_args );
		}


	if( issue_which_callback == 3 )
		{
		if( left_click_anywhere_cb_p_callback )  left_click_anywhere_cb_p_callback( left_click_anywhere_cb_args );
		}


	if( issue_which_callback == 4 )
		{
		if( right_click_anywhere_cb_p_callback )  right_click_anywhere_cb_p_callback( right_click_anywhere_cb_args );
		}
	}


redraw();
}


















//render traces, using ref_id as reference for other traces to be plotted against
void mgraph::render( int ref_id_in )
{
ref_id = ref_id_in;

redraw();
}






void mgraph::draw_text()
{
string s1;

if( ( text_cursor_along_y.text.size() != 0 ) && ( show_vert_units_div ) )
    {
    fl_color( text_vert_units_div.r, text_vert_units_div.g, text_vert_units_div.b );

    int iF = fl_font();
    int iS = fl_size();

    fl_font( text_vert_units_div.font, text_vert_units_div.size );

    strpf( s1, "%s", text_vert_units_div.text.c_str() );
    
    int offx = woffx + bkgd_border_left;
    int offy = woffy + bkgd_border_top;
  
    fl_draw( s1.c_str(), offx + text_vert_units_div.x, offy + text_vert_units_div.y );
    fl_font( iF, iS );
    }


if( ( text_cursor_along_y.text.size() != 0 ) && ( show_horiz_units_div ) )
    {
    fl_color( text_horiz_units_div.r, text_horiz_units_div.g, text_horiz_units_div.b );

    int iF = fl_font();
    int iS = fl_size();

    fl_font( text_horiz_units_div.font, text_horiz_units_div.size );

    strpf( s1, "%s", text_horiz_units_div.text.c_str() );
    
    int offx = woffx + bkgd_border_left;
    int offy = woffy + bkgd_border_top;
  
    fl_draw( s1.c_str(), offx + text_horiz_units_div.x, offy + text_horiz_units_div.y );
    fl_font( iF, iS );
    }


if( ( text_cursor_along_x.text.size() != 0 ) && ( show_cursors_along_y ) )
    {
    fl_color( text_cursor_along_y.r, text_cursor_along_y.g, text_cursor_along_y.b );

    int iF = fl_font();
    int iS = fl_size();

    fl_font( text_cursor_along_x.font, text_cursor_along_x.size );

    strpf( s1, "%s", text_cursor_along_y.text.c_str() );
    
    int offx = woffx + bkgd_border_left;
    int offy = woffy + bkgd_border_top;
  
    fl_draw( s1.c_str(), offx + text_cursor_along_y.x, offy + text_cursor_along_y.y );
    fl_font( iF, iS );
    }


if( ( text_cursor_along_x.text.size() != 0 ) && ( show_cursors_along_x ) )
    {
    fl_color( text_cursor_along_x.r, text_cursor_along_x.g, text_cursor_along_x.b );

    int iF = fl_font();
    int iS = fl_size();

    fl_font( text_cursor_along_x.font, text_cursor_along_x.size );

    strpf( s1, "%s", text_cursor_along_x.text.c_str() );
    
    int offx = woffx + bkgd_border_left;
    int offy = woffy + bkgd_border_top;
  
    fl_draw( s1.c_str(), offx + text_cursor_along_x.x, offy + text_cursor_along_x.y );
    fl_font( iF, iS );
    }

}










//given a trace id, search for its matching trace and return its trce[x] idx
//returns -1 if id not found
int mgraph::get_trace_idx_from_id( int id )
{

for( int i = 0; i < trce.size(); i++ )
    {
    if ( trce[ i ].id == id ) return i;
    }
return -1;
}





/*

//this does a rough line coord limit check, it does not clip but rather
//drops plotting a line outright if its coord are out of some range, this helps
//stop fltk being slowed down by long lines that extend way past fltk's clipping rects
int mgraph::clip_draw_line( int xx, int yy, int xxx, int yyy )
{

bool skip = 0;

if( use_line_clip )
    {
    if( xx < clip_minx ) skip = 1;
    if( xx > clip_maxx ) skip = 1;

    if( yy < clip_miny ) skip = 1;
    if( yy > clip_maxy ) skip = 1;

    if( xxx < clip_minx ) skip = 1;
    if( xxx > clip_maxx ) skip = 1;

    if( yyy < clip_miny ) skip = 1;
    if( yyy > clip_maxy ) skip = 1;

    if( !skip ) fl_line( xx, yy, xxx, yyy );
    }
else{
    fl_line( xx, yy, xxx, yyy );
    }
    
}
*/




void mgraph::clip_draw_line( int xx, int yy, int xxx, int yyy )
{
if( line_clip( xx, yy, xxx, yyy ) )
	{
	fl_line( xx, yy, xxx, yyy );
	}
}





void mgraph::clip_draw_point( int xx, int yy )
{
if( xx < clip_minx )  return;
if( xx >= clip_maxx ) return;

if( yy < clip_miny )  return;
if( yy >= clip_maxy ) return;

fl_point( xx, yy );
}








//this does a rough rect coord limit check, it does not clip but rather
//drops plotting some rect lines outright if their coords are out of widget range, this 
//helps stop fltk drawing incorrect lines (portions of rect) when they are offscreen, which caused long horizontal lines
int mgraph::rect_clip( int xx, int yy, int dx, int dy )
{

bool skip = 0;

if( use_line_clip )
    {
	int x2 = xx + dx;
	int y2 = yy + dy;
	int ll = 0;
	int rr = 0;
	int tt = 0;
	int bb = 0;


	if( ( xx >= clip_minx ) & ( xx < clip_maxx ) ) ll = 1;
	if( ( x2 >= clip_minx ) & ( x2 < clip_maxx ) ) rr = 1;
	if( ( yy >= clip_miny ) & ( yy < clip_maxy ) ) tt = 1;
	if( ( y2 >= clip_miny ) & ( y2 < clip_maxy ) ) bb = 1;
	
	if( ll | bb ) fl_line( xx, yy, xx, y2-1 );					//left
	if( ll | tt ) fl_line( xx, yy, x2-1, yy );					//top
	if( rr | bb ) fl_line( x2-2, yy, x2-2, y2-1 );					//right
	if( ll | bb ) fl_line( xx, y2-2, x2-2, y2-2 );					//bott

    }
else{
    fl_rect( xx, yy, dx, dy );
    }
    
return 1;																//v1.17	
}















//--------------- line/polygon clipping -----------------









//for sutherland-hodgmen clipping algorithm
int mgraph::inside_window( int x, int y, int edge )
{

switch( edge )
	{
	case 0:									//left edge test?
		return ( x > clip_vportleft) ;		//if edge=left, return 1 if x>left viewport edge
	case 1:									//right edge test?
		return ( x < clip_vportright );		//if edge=right, return 1 if x<right viewport edge
	case 2:									//bottom edge test?
		return ( y > clip_vportbot );		//if edge=bott, return 1 if y>bottom viewport edge
	case 3:									//top edge test?
		return ( y < clip_vporttop) ;		//if edge=top, return 1 if y<top viewport edge
	}
return 0;
}





//for sutherland-hodgmen clipping algorithm
int mgraph::calc_condition( int x1, int y1, int x2, int y2, int edge )
{
	int stat1 = inside_window( x1, y1, edge );	//test vertex x1 or y1 is within specified viewport edge
	int stat2 = inside_window( x2, y2, edge );	//test vertex x2 or y2 is within specified viewport edge
	if( !stat1 && stat2 ) return 1;				//x1,y1 is beyond, x2,y2 is within specified viewport edge
	if( stat1  && stat2 ) return 2;				//line between vertices is within specified viewport edge   		
	if( stat1  && !stat2 ) return 3;			//x2,y2 is beyond, x1,y1 is within specified viewport edge
	if( !stat1 && !stat2 ) return 4;			//line between vertices is beyond specified viewport edge 
	return 0; 									//never executed
}






//for sutherland-hodgmen clipping algorithm
void mgraph::solve_intersection( int edge, int x1, int y1, int x2, int y2, int &x, int &y )
{
double m = 0;
if(x2 != x1) m = ((double)(y2-y1)/(double)(x2-x1));
	switch(edge)
	{
	case 0:							//left
		x = clip_vportleft;
		y = y1 + m * (x - x1);
		break;
	case 1:							//right
		x = clip_vportright;
		y = y1 + m * (x - x1);
		break;
	case 2:							//bottom
		y = clip_vportbot;
		if(x1 != x2)
		x = x1 + (double)(1/m) * (y - y1);
		else
		x = x1;
		break;
	case 3:							//top
		y = clip_vporttop;
		if(x1 != x2)
		x = x1 + (double)(1/m) * (y - y1);
		else
		x = x1;
		break;
	}
//printf("\nIntS x1=%lf,y1=%lf,x2=%lf,y2=%lf,newx=%lf,newx=%lf\n",x1,y1,x2,y2,x,y);
}








//for sutherland-hodgmen clipping algorithm
void mgraph::add_point( int &ptr, vector<clip_coord_tag> &vo, int xx, int yy )
{
clip_coord_tag crd;
	
crd.x = xx;
crd.y = yy;


if( ptr >= vo.size() )
	{
	vo.push_back( crd );
	}
else{
//	vo.insert( vo.begin() + ptr, crd );
	vo[ ptr ] = crd;
	}
ptr++;
}





/*
void mgraph::check_sutherland_hodgmen()
{
vportleft = -10;
vporttop = 10;
vportright = 10;
vportbot = -10;


//vector<dwg_tag> vobj;
//vector<dwg_tag> vobj_clip;

//dwg_tag oo;

//oo.dwg_type = dt_poly;

//oo.orgx = 0.5;
//oo.orgy = 0.6;


clip_coord_tag crd;

crd.x = -20;
crd.y = -5;

crd.x = -1;
crd.y = -2;
oo.vline.push_back( crd );


crd.x = 1;
crd.y = 2;
oo.vline.push_back( crd );


vobj.push_back( oo );


//clip_poly( vobj, vobj_clip );
}

*/






//clip polygons to viewport edges, making a new polygons for rasterizer
//sutherland-hodgmen method
bool mgraph::clip_poly( vector<clip_coord_tag> vcrd_in, vector<clip_coord_tag> &vcrd_clip )
{
int point_count = vcrd_in.size();
if( point_count < 2 ) return  0;

//		flts forgx = oo.orgx;
//		flts forgy = oo.orgy;

int cond;
int x1;
int y1;
int x2;
int y2;



clip_coord_tag crd;

crd.x = vcrd_in[ 0 ].x;
crd.y = vcrd_in[ 0 ].y;
vcrd_in.push_back( crd );


int _x, _y;

//cycle through the 4 screen edges: left,right,top,bottom
for( int i = 0; i <= 3; i++ )			//0=left, 1=right, 2=top, 3=bottom
	{
	point_count = vcrd_in.size();
	int ptr = 0;
	vcrd_clip.clear();

	for( int j = 0; j < ( point_count - 1 ); j++ )
		{
		_x=0;
		_y=0;

		x1 = vcrd_in[ j ].x;			//get initial vertex point
		y1 = vcrd_in[ j ].y;
		x2 = vcrd_in[ j + 1 ].x;		//fetch next vertex point
		y2 = vcrd_in[ j + 1 ].y;

		//calc state (relative to an edge) of the line between the two vertex points 
		cond = calc_condition( x1, y1, x2, y2, i );

//			printf("\nJ=%d, %lf, %lf, %lf, %lf Condition=%d\n",j,x1,y1,x2,y2,cond);
		switch( cond )
			{
			case 1:								//x1,y1 is beyond, x2,y2 is within specified viewport edge																				//add intersection vertex _x,_y and vertex x2,y2
				solve_intersection( i, x1, y1, x2, y2, _x, _y );
				add_point( ptr, vcrd_clip, _x, _y );
				add_point( ptr, vcrd_clip, x2, y2 );
			break;
			
			case 2:								//line is fully within current edge, add vertex
				add_point( ptr, vcrd_clip, x2, y2 );
			break;
			
			case 3:								//x2,y2 is beyond, x1,y1 is within specified viewport edge
												//add intersection vertex _x,_y
				solve_intersection( i, x1, y1, x2, y2, _x, _y );
				add_point( ptr, vcrd_clip, _x, _y );
			break;

			case 4:								//line is beyond current edge, dont add vertex 
			break;
			}
		}
	if( vcrd_clip.size() != 0 )
		{
		add_point( ptr, vcrd_clip, vcrd_clip[ 0 ].x, vcrd_clip[ 0 ].y );
		}
	vcrd_in = vcrd_clip;
	}

//	printf("not inview\n" );
		

int final_vertex_count = vcrd_in.size();
vcrd_clip = vcrd_in;

if( final_vertex_count == 0 ) return 0;
else return 1;
//vobj_clip.push_back( oclip );
//	convert_obj_vertices_to_relative( vobj_clip, vobj_clip.size() - 1 );

}









enum
{
en_lineclip_top = 0x1,
en_lineclip_bot = 0x2, 
en_lineclip_right = 0x4,
en_lineclip_left = 0x8,
};





//for cohen-sutherland line clip algorithm
int mgraph::compute_outcode( int xx, int yy )
{
int oc = 0;
 
if ( yy > clip_vporttop ) oc |= en_lineclip_top;
else if ( yy < clip_vportbot ) oc |= en_lineclip_bot;
 
 
if ( xx > clip_vportright ) oc |= en_lineclip_right;
else if ( xx < clip_vportleft ) oc |= en_lineclip_left;
 
return oc;
}
 







//for cohen-sutherland line clip algorithm
//returns 1 if part or all of line visible, else 0
bool mgraph::line_clip( int &x1_in, int &y1_in, int &x2_in, int &y2_in )
{
int accept;
int done;
int outcode1, outcode2;

accept = 0;
done = 0;

double x1 = x1_in;
double x2 = x2_in;
double y1 = y1_in;
double y2 = y2_in;

outcode1 = compute_outcode ( x1, y1 );
outcode2 = compute_outcode ( x2, y2 );
do
	{
	if (outcode1 == 0 && outcode2 == 0)
		{
		accept = 1;
		done = 1;
		}
else if (outcode1 & outcode2)
	{
	done = 1;
	}
else{
	double x, y;
	int outcode_ex = outcode1 ? outcode1 : outcode2;
	if (outcode_ex & en_lineclip_top)
		{
		x = x1 + (x2 - x1) * ( clip_vporttop - y1 ) / ( y2 - y1 );
		y = clip_vporttop;
		}

	else if ( outcode_ex & en_lineclip_bot )
			{
			x = x1 + ( x2 - x1 ) * ( clip_vportbot - y1 ) / ( y2 - y1 );
			y = clip_vportbot;
			}
	else if (outcode_ex & en_lineclip_right )
			{
			y = y1 + ( y2 - y1 ) * ( clip_vportright - x1 ) / ( x2 - x1 );
			x = clip_vportright;
			}
	else{
		y = y1 + ( y2 - y1 ) * ( clip_vportleft - x1 ) / ( x2 - x1 );
		x = clip_vportleft;
		}
	if ( outcode_ex == outcode1 )
		{
		x1 = x;
		y1 = y;
		outcode1 = compute_outcode( x1, y1 );
		}
	else
		{
		x2 = x;
		y2 = y;
		outcode2 = compute_outcode( x2, y2 );
		}
	}	
} while ( done == 0 );

x1_in = x1;
y1_in = y1;
x2_in = x2;
y2_in = y2;

return accept;

}




//-------------------------------------------------------























//this will be called multiple times to allow overlaying etc
void mgraph::draw_user_objs( int at_draw_ordering )
{
//bool b_drawn_objs;

//---------------
//draw list of objs, use requested x, y scaling if req
//if( !b_drawn_objs )
    {
//    b_drawn_objs = 1;
    int iF = fl_font();
    int iS = fl_size();

clip_coord_tag crd;
vector<clip_coord_tag>vcrd;
vector<clip_coord_tag>vclipcrd;


    //draw additional objs
    for( int i = 0; i < vdrwobj.size(); i++ )
        {
        if( !vdrwobj[ i ].visible ) continue;

		if( vdrwobj[ i ].draw_ordering != at_draw_ordering ) continue;		//not yet the time to draw this user obj?

		vcrd.clear();
		vclipcrd.clear();

        //get vals
        int style = vdrwobj[ i ].line_style;
        int thick = vdrwobj[ i ].line_thick;
        double x1 = vdrwobj[ i ].x1;
        double y1 = vdrwobj[ i ].y1;
        double x2 = vdrwobj[ i ].x2;
        double y2 = vdrwobj[ i ].y2;
        double arc1 = vdrwobj[ i ].arc1;
        double arc2 = vdrwobj[ i ].arc2;
        string stext = vdrwobj[ i ].stext;
        int font = vdrwobj[ i ].font;
        int font_size = vdrwobj[ i ].font_size;

        int offx;
        int offy;
        int wid;
        int hei;

		get_background_offsxy( offx, offy );
		get_background_dimensions( wid, hei );
     
		clip_minx = offx + vdrwobj[ i ].clip_left;
		clip_maxx = offx + wid - vdrwobj[ i ].clip_right;
		clip_miny = offy + vdrwobj[ i ].clip_top;
		clip_maxy = offy + hei - vdrwobj[ i ].clip_bottom;


		clip_vportleft = clip_minx;									//these coords 0,0  would be: left,bottom -- used for: 'line_clip()', 'clip_poly()'
		clip_vportright = clip_maxx;
		clip_vporttop = clip_maxy;
		clip_vportbot = clip_miny;

		int clip_wid = wid - vdrwobj[ i ].clip_right - vdrwobj[ i ].clip_left;
		int clip_hei = hei - vdrwobj[ i ].clip_bottom - vdrwobj[ i ].clip_top;

        int r =  vdrwobj[ i ].r;
        int g =  vdrwobj[ i ].g;
        int b =  vdrwobj[ i ].b;
        
        fl_color( r, g, b );
        fl_line_style ( style, thick );         //for windows you must set line setting after the colour, see the manual



//		double x, y;
        double xx1 = x1;
        double yy1 = y1;
        double xx2 = x2;
        double yy2 = y2;

        int ix = x1;
        int iy = y1;
        int ixx = x2;
        int iyy = y2;

        double positionx = 0;
        double positiony = 0;
        double scalex = 1.0;
        double scaley = 1.0;
        double d_midx = 0;
        double d_midy = 0;
        int mid_x = 0;
        int mid_y = 0;
        double sign = 1;

		double trace_scalex = 1.0;
		double trace_scaley = 1.0;

		double trcminx = 0;												//v1.16
			
//if( dbg_graph_id == 3 )
//	{
//if( vdrwobj[ i ].type == en_dobt_rectf ) printf("rectpnt xx: %f, yy: %f, x2: %f, y2: %f\n", x1, y1, x2, y2 );
//if( vdrwobj[ i ].type == en_dobt_text )  printf("txt pnt xx: %f, yy: %f, x2: %f, y2: %f\n", x1, y1, x2, y2 );

//if( vdrwobj[ i ].type == en_dobt_rectf ) printf("rect use_scalex: %d\n", vdrwobj[ i ].use_scale_x );
//if( vdrwobj[ i ].type == en_dobt_text )  printf("txt  use_scalex: %d\n", vdrwobj[ i ].use_scale_x );

//	}
//exit(0);


        //if req try to get a specific trace's scale x
        int z = vdrwobj[ i ].use_scale_x;                   //get trace id to use, if any
        if( z != -1 )
            {
            int idx = get_trace_idx_from_id( z );           //use id to get trace needed
            if( idx != -1 )
                {
				scalex = vscalex[ 0 ];
//if( vdrwobj[ i ].type == en_dobt_rectf ) printf("rect %d got idx: %d, scalex: %f\n", i, idx, scalex );
//if( vdrwobj[ i ].type == en_dobt_text )  printf("txt  %d got idx: %d, scalex: %f\n", i, idx, scalex );
				trace_scalex = trce[ idx ].scalex;
                d_midx = vtrace_dmidx[ idx ];

                xx1 = x1 * scalex - d_midx * scalex;
                xx2 = x2 * scalex - d_midx * scalex;
                offx = vtrace_offx[ idx ];

                mid_x = vtrace_midx[ idx ];
                }
             }




        //if req try to get a specific trace's scale y
        z = vdrwobj[ i ].use_scale_y;                           //get trace id to use, if any
        if( z != -1 )
            {
            int idx = get_trace_idx_from_id( z );               //use id to get trace needed
            if( idx != -1 )
                {
                scaley = vscaley[ idx ];
				trace_scaley = trce[ 0 ].scaley;
                d_midy = vtrace_dmidy[ idx ];

                yy1 = y1 * scaley - d_midy * scaley;
                yy2 = y2 * scaley - d_midy * scaley;
                offy = vtrace_offy[ idx ];

                mid_y = vtrace_midy[ idx ];
                sign = -1;
                }
            }


        //if req try to get a specific trace's position x
        z = vdrwobj[ i ].use_pos_x;                             //get trace id to use, if any
        if( z != -1 )
            {
            int idx = get_trace_idx_from_id( z );               //use id to get trace needed
            if( idx != -1 )
                {
//                scalex = vscalex[ idx ];
//                d_midx = vtrace_dmidx[ idx ];

                positionx = vtrace_positionx[ idx ];

                mid_x = vtrace_midx[ idx ];
                offx = vtrace_offx[ idx ];
                
                trcminx = vtrace_minx[ idx ];							//v1.16
                }
            }


        //if req try to get a specific trace's position y
        z = vdrwobj[ i ].use_pos_y;                             //get trace id to use, if any
        if( z != -1 )
            {
            int idx = get_trace_idx_from_id( z );               //use id to get trace needed
            if( idx != -1 )
                {
//                scaley = vscaley[ idx ];
                positiony = vtrace_positiony[ idx ];
                mid_y = vtrace_midy[ idx ];
                offy = vtrace_offy[ idx ];
                sign = -1;
                }
            }



//			x = x1 * scalex * trace_scalex - d_midx * scalex;   //if you change here also change same in: get_trace_to_plot_factors()
//			y = y1 * scaley * trace_scaley - d_midy * scaley;


		//x = x1 * scalex * trace_scalex - d_midx * scalex;

//if( dbg_graph_id == 3 )
//	{
//if( vdrwobj[ i ].type == en_dobt_rectf ) printf("rect mid_x: %d, mid_y: %d, offx: %d, offy: %d, scalex: %f, scaley: %f, trace_scalex: %f, trace_scaley: %f, posx: %f, posy: %f\n", mid_x, mid_y, offx, offy, scalex, scaley, trace_scalex, trace_scaley, positionx, positiony );
//if( vdrwobj[ i ].type == en_dobt_text )  printf("text mid_x: %d, mid_y: %d, offx: %d, offy: %d, scalex: %f, scaley: %f, trace_scalex: %f, trace_scaley: %f, posx: %f, posy: %f\n", mid_x, mid_y, offx, offy, scalex, scaley, trace_scalex, trace_scaley, positionx, positiony );
//	}


//		xx1 = x1 * scalex * trace_scalex - d_midx * scalex;
		xx1 = (x1 - trcminx) * scalex * trace_scalex - d_midx * scalex;	//v1.16
		yy1 = y1 * scaley * trace_scaley - d_midy * scaley;

//		xx2 = x2 * scalex * trace_scalex - d_midx * scalex;
		xx2 = (x2 - trcminx) * scalex * trace_scalex - d_midx * scalex;	//v1.16
		yy2 = y2 * scaley * trace_scaley - d_midy * scaley;

//if( dbg_graph_id == 3 )
//	{
//if( vdrwobj[ i ].type == en_dobt_rectf ) printf("rectpntscl xx: %f, yy: %f, xxx: %f, yyy: %f\n", xx1, yy1, xx2, yy2 );
//if( vdrwobj[ i ].type == en_dobt_text )  printf("txtpntscl  xx: %f, yy: %f, xxx: %f, yyy: %f\n", xx1, yy1, xx2, yy2 );
//	}

        ix = nearbyint( mid_x + xx1 + offx + positionx );
        iy = nearbyint( mid_y + ( sign * yy1 ) + offy + positiony );
        ixx = nearbyint( mid_x + xx2 + offx + positionx );
        iyy = nearbyint( mid_y + ( sign * yy2 ) + offy + positiony );

		int type = vdrwobj[ i ].type;

		if( ( type == en_dobt_rect ) | ( type == en_dobt_rectf ) )
			{
			//note that if clipped at 'clip_maxx-1' or clip_maxy-1' the rect is slighly smaller, so have allowed 1 pixel margin
			if( ix < clip_minx ) ix = clip_minx;			//this clipping helps when using a rect as a selection area indicator, when zooming right in, 	
			if( ix >= clip_maxx ) ix = clip_maxx - 0;		//the clipping stops rectangle coords becoming erroneous which would cause the user rect to dissappear	
			if( iy < clip_miny ) iy = clip_miny;
			if( iy >= clip_maxy ) iy = clip_maxy - 0;

			if( ixx < clip_minx ) ixx = clip_minx;
			if( ixx >= clip_maxx ) ixx = clip_maxx - 0;
			if( iyy < clip_miny ) iyy = clip_miny;
			if( iyy >= clip_maxy ) iyy = clip_maxy - 0;
			}
	
        switch ( type )
            {
            case en_dobt_point:

				clip_draw_point( ix, iy );
//if( ( ix >= clip_minx ) & ( ix < clip_maxx ) )
//					{
//					if( ( iy >= clip_miny ) & ( iy < clip_maxy ) )
//						{
//						fl_point( ix, iy );
//						}
//					}
            break;


            case en_dobt_line:
				clip_draw_line( ix, iy, ixx, iyy );
             break;


            case en_dobt_rect:

				fl_begin_line();
					fl_vertex( ix, iy );
					fl_vertex( ixx, iy );
					fl_vertex( ixx, iyy );
					fl_vertex( ix, iyy );
					fl_vertex( ix, iy );
printf( "xx1: %f, yy1: %f  xx2: %f, yy2: %f\n", xx1, yy1, xx2, yy2 );
				fl_end_line();
            break;


            case en_dobt_rectf:

				fl_begin_polygon();
					fl_vertex( ix, iy );
					fl_vertex( ixx, iy );
					fl_vertex( ixx, iyy );
					fl_vertex( ix, iyy );
					fl_vertex( ix, iy );
				fl_end_polygon();
/*
if(i==1)
{
double xxxx = 13 * scalex * trace_scalex - d_midx * scalex;
double yyyy = 0.007812 * scaley * trace_scaley - d_midy * scaley;

int xx = nearbyint( mid_x + xxxx + offx + positionx );
int yy = nearbyint( mid_y - yyyy + offy + positiony );
printf( "positionx: %f, positiony: %f\n", positionx, positiony );
printf( "scalex: %f, d_midy: %f\n", d_midx, d_midy );
printf( "scalex: %f, trace_scalex: %f\n", scalex, trace_scalex );
printf( "scaley: %f, trace_scaley: %f\n", scaley, trace_scaley );
printf( "trace_midx: %d, offx: %d\n", mid_x, offx );
printf( "trace_midy: %d, offy: %d\n", mid_y, offy );
printf( "x1: %f, y1: %f\n", x1, y1 );
printf( "ix: %d, iy: %d\n", ix, iy );
fl_rect(  ix, iy, 10, 10 );
}
*/
            break;


            case en_dobt_arc:
            case en_dobt_pie:
            
//				ixx = nearbyint( xx2 );					//just need wid,hei, no offx,offsy
//				iyy = nearbyint( yy2 );
				if ( ix > ixx )
					{
					int itmp = ix;
					ix = ixx;
					ixx = itmp;	
					}
					
				if ( iy > iyy )
					{
					int itmp = iy;
					iy = iyy;
					iyy = itmp;	
					}

				if( ( ix >= clip_minx ) || ( (ix + ixx) < clip_maxx ) )
					{
					if( ( iy >= clip_miny ) || ( (iy + iyy) < clip_maxy ) )
						{
fl_push_clip( clip_minx, clip_miny, clip_wid, clip_hei );

						if( type == en_dobt_arc ) fl_arc( ix, iy, (ixx - ix),  (iyy - iy), arc1, arc2 );		//x,y,dx,dy,arc1,arc2						
						if( type == en_dobt_pie ) fl_pie( ix, iy, (ixx - ix),  (iyy - iy), arc1, arc2 );		//x,y,dx,dy,arc1,arc2

//printf( "xx1: %f, yy1: %f  xx2: %f, yy2: %f\n", xx1, yy1, xx2, yy2 );
//printf( "ix: %d, iy: %d  xx2: %d, yy2: %d\n", ix, iy, ixx, -(int)fabs(((iyy) -(iy) )) );
//						fl_arc( ix, iy, 30, 50, arc1, arc2 );		//x,y,dx,dy,arc1,arc2
fl_pop_clip();
						}
					}
            break;

/*
            case en_dobt_pie:
 				ixx = nearbyint( xx2 );					//just need wid,hei, no offx,offsy
				iyy = nearbyint( yy2 );
				if( ( ix >= clip_minx ) || ( (ix + ixx) < clip_maxx ) )
					{
					if( ( iy >= clip_miny ) || ( (iy + iyy) < clip_maxy ) )
						{
fl_push_clip( clip_minx, clip_miny, clip_wid, clip_hei );
						fl_pie( ix, iy, ixx, iyy, arc1, arc2 );		//x,y,dx,dy,arc1,arc2
fl_pop_clip();
						}
					}
            break;
*/        

            case en_dobt_text:
                fl_font( font, font_size );

				{
				int descent = fl_descent();

				int width, height;
				width = 0;
				fl_measure( stext.c_str(), width, height );
				
				int nx = ix;											//left justify by default
				int ny = iy;											//text bottom sitting at coord
				if( vdrwobj[ i ].justify & en_tj_horiz_center ) nx -= width/2;
				if( vdrwobj[ i ].justify & en_tj_horiz_right ) 	nx -= width;

				if( vdrwobj[ i ].justify & en_tj_vert_center ) ny = iy + height/2 - descent;
				if( vdrwobj[ i ].justify & en_tj_vert_top ) ny = iy + height - descent;

//fl_rect( ix, iy - (height)/1, width, height );
//fl_rect( ix, iy - (height)/2, width, height );
//fl_line( ix, iy + descent, ix+width, iy + descent );
fl_push_clip( clip_minx, clip_miny, clip_wid, clip_hei );		//need this for items like text
				fl_draw( stext.c_str(), nx, ny );
fl_pop_clip();
				}
            break;

            case en_dobt_polypoints:
            case en_dobt_polypoints_wnd_coords:
            
				for( int j = 0; j < vdrwobj[ i ].vpolyx.size(); j++ )
					{
					x1 = vdrwobj[ i ].vpolyx[ j ];
					y1 = vdrwobj[ i ].vpolyy[ j ];

					if( type == en_dobt_polypoints )
						{
						xx1 = (x1 - trcminx) * scalex * trace_scalex - d_midx * scalex;
						yy1 = y1 * scaley * trace_scaley - d_midy * scaley;
						}

					if( type == en_dobt_polypoints_wnd_coords )
						{
						xx1 = x1;
						yy1 = y1;
						}
					
					ix = nearbyint( mid_x + xx1 + offx + positionx );
					iy = nearbyint( mid_y + ( sign * yy1 ) + offy + positiony );

					if( ( ix >= clip_minx ) & ( ix < clip_maxx ) )
						{
						if( ( iy >= clip_miny ) & ( iy < clip_maxy ) )
							{
							fl_begin_points();
								fl_point( ix, iy );
							fl_end_points();
							}
					
						}
					}
            break;



           case en_dobt_polyline:
           case en_dobt_polyline_wnd_coords:
				int first_x, first_y;
				int last_x, last_y;

				for( int j = 0; j < vdrwobj[ i ].vpolyx.size(); j++ )
					{
					x1 = vdrwobj[ i ].vpolyx[ j ];
					y1 = vdrwobj[ i ].vpolyy[ j ];

					if( type == en_dobt_polyline )
						{
						xx1 = (x1 - trcminx) * scalex * trace_scalex - d_midx * scalex;
						yy1 = y1 * scaley * trace_scaley - d_midy * scaley;
						}

					if( type == en_dobt_polyline_wnd_coords )
						{
						xx1 = x1;
						yy1 = y1;
						}
					
					ix = nearbyint( mid_x + xx1 + offx + positionx );
					iy = nearbyint( mid_y + ( sign * yy1 ) + offy + positiony );

					if( j == 0 )
						{
						first_x = ix;
						first_y = iy;
						}
					else{
						int clipx = ix;
						int clipy = iy;
						if( line_clip( clipx, clipy, last_x, last_y ) )
							{
							fl_begin_line();
								fl_vertex( last_x, last_y );
								fl_vertex( clipx, clipy );
							fl_end_line();
							}
						}
					last_x = ix;
					last_y = iy;
					}
            break;






            case en_dobt_polygon:
            case en_dobt_polygon_wnd_coords:
				first_x, first_y;
				last_x, last_y;

				for( int j = 0; j < vdrwobj[ i ].vpolyx.size(); j++ )
					{
					x1 = vdrwobj[ i ].vpolyx[ j ];
					y1 = vdrwobj[ i ].vpolyy[ j ];

					if( type == en_dobt_polygon )
						{
						xx1 = (x1 - trcminx) * scalex * trace_scalex - d_midx * scalex;
						yy1 = y1 * scaley * trace_scaley - d_midy * scaley;
						}

					if( type == en_dobt_polygon_wnd_coords )
						{
						xx1 = x1;
						yy1 = y1;
						}
					
					ix = nearbyint( mid_x + xx1 + offx + positionx );
					iy = nearbyint( mid_y + ( sign * yy1 ) + offy + positiony );

					if( j == 0 )
						{
						first_x = ix;
						first_y = iy;
						}
					else{
						int clipx = ix;
						int clipy = iy;
						if( line_clip( clipx, clipy, last_x, last_y ) )
							{
							fl_begin_line();
								fl_vertex( last_x, last_y );
								fl_vertex( clipx, clipy );
							fl_end_line();
							}
						}
					last_x = ix;
					last_y = iy;
					}

				if( line_clip( last_x, last_y, first_x, first_y ) )
					{
					fl_begin_line();
						fl_vertex( last_x, last_y );
						fl_vertex( first_x, first_y );
					fl_end_line();
					}
            break;


            case en_dobt_polygonf:
			case en_dobt_polygonf_wnd_coords:
            
				for( int j = 0; j < vdrwobj[ i ].vpolyx.size(); j++ )
					{
					x1 = vdrwobj[ i ].vpolyx[ j ];
					y1 = vdrwobj[ i ].vpolyy[ j ];

					if( type == en_dobt_polygonf )
						{
						xx1 = (x1 - trcminx) * scalex * trace_scalex - d_midx * scalex;
						yy1 = y1 * scaley * trace_scaley - d_midy * scaley;
						}
					
					if( type == en_dobt_polygonf_wnd_coords )
						{
						xx1 = x1;
						yy1 = y1;
						}
					ix = nearbyint( mid_x + xx1 + offx + positionx );
					iy = nearbyint( mid_y + ( sign * yy1 ) + offy + positiony );

					crd.x = ix;
					crd.y = iy;
					
					vcrd.push_back(crd);
					}

					clip_poly( vcrd, vclipcrd );							//clip to background rect

					fl_begin_polygon();
						for( int j = 0; j < vclipcrd.size(); j++ )
							{							
							ix = vclipcrd[ j ].x;
							iy = vclipcrd[ j ].y;

							fl_vertex( ix, iy );
							}
					fl_end_polygon();
			break;


            default:
            break;
            }

        }

    fl_font( iF, iS );
    }
//----------------------

}







int plotx_max = 0;
int ploty_max = 0;
int plotx_min = 0;
int ploty_min = 0;






void mgraph::draw()
{

//draw_user_objs_cnt = 0;
at_draw_ordering = 0;						//for user draw obj ordering

Fl_Box::draw();

bool lissa_status_x = 0;
bool lissa_status_y = 0;
vector<int> lissa_x;
vector<int> lissa_y;
Fl_Color lissa_col;
int lissa_line_style;
int lissa_line_thick ;
bool b_drawn_objs = 0;


int wid = w();
int hei = h();

int id;



//int bkgd_reductionx = bkgd_border_right + bkgd_border_left;
//int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top;

int bkgd_offx;// = woffx + bkgd_border_left;
int bkgd_offy;// = woffy + bkgd_border_top;

int bkgd_wid;// = wid - bkgd_reductionx;
int bkgd_hei;// = hei - bkgd_reductiony;

int grat_offx;
int grat_offy;
int grat_wid;
int grat_hei;

get_background_offsxy( bkgd_offx, bkgd_offy );
get_background_dimensions( bkgd_wid, bkgd_hei );

get_graticle_offsxy( grat_offx, grat_offy );
get_graticle_dimensions( grat_wid, grat_hei );

int bkgd_midx = bkgd_wid / 2;
int bkgd_midy = bkgd_hei / 2;

clip_minx = bkgd_offx + 0;									//the coords  0,0  would be: left,top
clip_miny = bkgd_offy + 0;
clip_maxx = bkgd_offx + bkgd_wid + 1;						//plus one, as fl_rect() uses: wid/hei
clip_maxy = bkgd_offy + bkgd_hei + 1;

clip_vportleft = clip_minx;									//these coords 0,0  would be: left,bottom -- used for: 'line_clip()', 'clip_poly()'
clip_vportright = clip_maxx;
clip_vporttop = clip_maxy;
clip_vportbot = clip_miny;

//printf("clip_wdg_minx: %d\n",clip_wdg_minx);
//printf("clip_wdg_miny: %d\n",clip_wdg_miny);
//printf("clip_wdg_maxx: %d\n",clip_wdg_maxx);
//printf("clip_wdg_maxy: %d\n",clip_wdg_maxy);
fl_color( border_col.r, border_col.g, border_col.g );
fl_rect( woffx, woffy, w(), h() );

fl_push_clip( bkgd_offx, bkgd_offy, bkgd_wid, bkgd_hei );		//clip

fl_color( background.r, background.g, background.b );

fl_rectf( bkgd_offx, bkgd_offy, bkgd_wid, bkgd_hei );






//return;

//fl_rect( woffx, woffy, w() - woffx , h() - woffy + 3 );



double x, y;

//	fl_line( axisx + t * factorx , midy - y, axisx + oldx, midy - oldy );


vscalex.clear();
vscaley.clear();
vtrace_scale_x.clear();
vtrace_scale_y.clear();
vtrace_positionx.clear();
vtrace_positiony.clear();
vtrace_dmidx.clear();
vtrace_dmidy.clear();
vtrace_offx.clear();
vtrace_offy.clear();
vtrace_wid.clear();
vtrace_hei.clear();
vtrace_midx.clear();
vtrace_midy.clear();
vtrace_id.clear();
vplot_offsx.clear();
vplot_offsy.clear();
vtrace_minx.clear();

//----- setup trace values ------
//do these first as user objs can be drawn at 3 different points below (this allows objs to appear under or over graticle/traces)
for( int i = 0; i < trce.size(); i++ )						//cycle each trace
	{
    id = trce[ i ].id;

	int trace_offx;
	int trace_offy;
	int trace_wid;
	int trace_hei;

	//get_vis_dimensions( wid, hei );

//	int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//	int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
//	trace_offx = woffx + bkgd_border_left + trce[ i ].border_left;
//	trace_offy = woffy + bkgd_border_top + trce[ i ].border_top;

	get_trace_offsxy( i, trace_offx, trace_offy );

//	trace_wid = wid - trace_reductionx;
//	trace_hei = hei - trace_reductiony;
	get_trace_vis_dimensions( 0, trace_wid, trace_hei );

//printf("w(): %d %d\n", w(), h());
//printf("wid: %d %d\n", wid, hei);
//printf("trace_wid: %d %d\n", trace_wid, trace_hei);
//printf("woffx: %d %d\n", woffx, woffy);
//printf("trace_offx: %d %d\n", trace_offx, trace_offy);
	double trace_midx = trace_wid / 2;
	double trace_midy = trace_hei / 2;


	double deltax, deltay;
	double scalex, scaley;
	double d_midx, d_midy;
	double positionx = trce[ i ].posx;
	double positiony = -trce[ i ].posy;


	int plot_offsx = trce[ i ].plot_offsx;					//not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
	int plot_offsy = -trce[ i ].plot_offsy;

	double trcminx = trce[ i ].minx;									//v1.16


//    int z = trce[ i ].use_pos_y;                         	//get trace id to use, if any
//	  int idx = get_trace_idx_from_id( z );           		//use id to get trace needed

//	if( idx != -1 )
//		{
//		positionx = trce[ idx ].posx;
//		}

	deltax = trce[ i ].maxx - trce[ i ].minx;
	deltay = trce[ i ].maxy - trce[ i ].miny;


	if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = (double)trace_wid / deltax;
//			d_midx = deltax / 2.0 + trce[ i ].minx;
			d_midx = deltax / 2.0 + 0;									//v1.16
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = grat_pixels_x;
			d_midx = 0;//deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
		{
		scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
		d_midx = (double)trace_midx / scalex;
		}



	if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = (double)trace_hei / deltay;
			d_midy = deltay / 2.0 + trce[ i ].miny;		//this will move ypos to centre the trace
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}



        //enforce limits to auto y scaling if req
        if( trce[ i ].b_limit_auto_scale_max_for_y ) 
            {
            if( scaley > trce[ i ].limit_auto_scale_max_for_y ) scaley = trce[ i ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
            }
            
        if( trce[ i ].b_limit_auto_scale_min_for_y )
            {
            if( scaley < trce[ i ].limit_auto_scale_min_for_y ) scaley = trce[ i ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
            }
            
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
		}



	if( trce[ i ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = grat_pixels_y;
			d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}
		}


	if( trce[ i ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
		{
		double larg_y;
		if( fabs( trce[ i ].maxy ) > fabs( trce[ i ].miny ) ) larg_y = trce[ i ].maxy;
		else larg_y = trce[ i ].miny;
		
		scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
		d_midy = 0;
		}


	if( trce[ i ].yunits_perpxl > -1 )					//use user spec scale
		{
		scaley = 1.0 / trce[ i ].yunits_perpxl;			//scale as directed
		d_midy = 0;
		}

	if( i == 0 )										//1st trace?
		{
		ref_scalex = scalex;							//remember x scale
		ref_d_midx = d_midx;
		}

//printf( "draw() - ref %d, scalex= %g, trce[ i ].xunits_perpxl= %g\n", i, scalex, trce[ i ].xunits_perpxl );

//printf("deltay= %lf, scaley= %lf, d_midy= %lf\n", deltay, scaley, d_midy );

	if( i != 0 )										//not 1st trace?
		{
		scalex = ref_scalex;							//use 1st trace's x scale
		d_midx = ref_d_midx;
		}


    //!!! ALL these MUST have same number of entries !!!!
    vscalex.push_back( scalex );                 		//store trace params for later, MAKE sure you also do this for vtrace_scale_y, vtrace_positionx, vtrace_positiony, vtrace_dmidx, vtrace_dmidy, vtrace_minx, vtrace_id
    vscaley.push_back( scaley );                 		//MAKE sure you also do this for trace_offx, trace_offy, trace_midx, trace_midy, trace_id
    vtrace_scale_x.push_back( trce[ i ].scalex );
    vtrace_scale_y.push_back( trce[ i ].scaley );
    vtrace_positionx.push_back( positionx );
    vtrace_positiony.push_back( positiony );
	vtrace_dmidx.push_back( d_midx );
	vtrace_dmidy.push_back( d_midy );
    vtrace_offx.push_back( trace_offx );
    vtrace_offy.push_back( trace_offy );
    vtrace_wid.push_back( trace_wid );
    vtrace_hei.push_back( trace_hei );
    vtrace_midx.push_back( trace_midx );
    vtrace_midy.push_back( trace_midy );
    vplot_offsx.push_back( plot_offsx );				//not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
    vplot_offsy.push_back( plot_offsy );
	vtrace_minx.push_back( trcminx );									//v1.16

    vtrace_id.push_back( id );                          //!!! ALL these MUST have same number of entries !!!!

	}



//set up referrence overrides
for( int i = 0; i < trce.size(); i++ )						//cycle each trace
	{
    int z = trce[ i ].use_pos_y;                         	//get trace id to use, if any
    int idx = get_trace_idx_from_id( z );           		//use id to get trace needed

	if( idx != -1 )
		{
		if( idx != i  )
			{
			vtrace_positiony[ i ] = vtrace_positiony[ idx ];
			vtrace_midx[ i ] = vtrace_midx[ idx ];
			vtrace_offx[ i ] = vtrace_offx[ idx ];
			}
		}

	z = trce[ i ].use_scale_y;                         		//get trace id to use as the ref, if any
	idx = get_trace_idx_from_id( z );           			//use id to get trace needed
	if( idx != -1  )
		{
		if( idx != i )										//ignore trace that matches ref
			{
			vscaley[ i ] = vscaley[ idx ];
			vtrace_scale_y[ i ] = vtrace_scale_y[ idx ];
			vtrace_midy[ i ] = vtrace_midy[ idx ];
			vtrace_dmidy[ i ] = vtrace_dmidy[ idx ];
			}
		}
	}

//grat_offx = woffx + graticule_border_left;
//grat_offy = woffy + graticule_border_top;


//draw user objs that need to appear before graticle
draw_user_objs( at_draw_ordering );
//printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!draw_user_objs_cnt: %d\n", draw_user_objs_cnt );
//draw_user_objs_cnt++;
at_draw_ordering = 1;									//for user draw obj ordering
//-------------------------------



// ------ draw graticles --------
if( cro_graticle )
	{
	clip_minx = grat_offx;
	clip_maxx = grat_offx + grat_wid;
	clip_miny = grat_offy;
	clip_maxy = grat_offy + grat_hei;

	clip_vportleft = clip_minx;									//these coords 0,0  would be: left,bottom -- used for: 'line_clip()', 'clip_poly()'
	clip_vportright = clip_maxx;
	clip_vporttop = clip_maxy;
	clip_vportbot = clip_miny;

	int grat_midx = grat_wid / 2;
	int grat_midy = grat_hei / 2;

	//draw 2 centre graticules
	int x1 = 0;
	int x2 = grat_wid;

	int y1 = grat_hei / 2;
	int y2 = y1;

	fl_color( 150, 150, 150 );
    fl_line_style ( FL_SOLID, 1 );         					//for windows you must set line setting after the colour, see the manual
	fl_line( grat_offx + x1, grat_offy + y1, grat_offx + x2 , grat_offy + y2 );

	x1 = grat_wid / 2;
	x2 = x1;

	y1 = 0;
	y2 = grat_hei;
	fl_line( grat_offx + x1, grat_offy + y1, grat_offx + x2 , grat_offy + y2 );


	int cnt = graticle_count_y / 2;

	fl_color( 90, 90, 90 );
    fl_line_style ( FL_SOLID, 1 );         					//for windows you must set line setting after the colour, see the manual
	for( int i = 0; i < cnt - 1; i++ )						//horiz grats
		{
		
		x1 = 0;
		x2 = grat_wid;

		y1 = grat_pixels_y * ( i + 1 );
		y2 = y1;
		fl_line( grat_offx + x1, grat_offy + grat_midy - y1, grat_offx + x2 , grat_offy + grat_midy - y2 );

		fl_line( grat_offx + x1, grat_offy + grat_midy + y1, grat_offx + x2 , grat_offy + grat_midy + y2 );
		}

	cnt = graticle_count_x / 2;
	for( int i = 0; i < cnt - 1; i++ )						//vert grats
		{

		x1 = grat_pixels_x * ( i + 1 );
		x2 = x1;

		y1 = 0;
		y2 = grat_hei;
		fl_line( grat_offx + grat_midx - x1, grat_offy + y1, grat_offx + grat_midx - x2 , grat_offy + y2 );

		fl_line( grat_offx + grat_midx + x1, grat_offy + y1, grat_offx + grat_midx + x2 , grat_offy + y2 );
		}

	}
//-------------------------------



//draw user objs that need to appear after graticle but before traces
draw_user_objs( at_draw_ordering );
//printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!draw_user_objs_cnt: %d\n", draw_user_objs_cnt );
//draw_user_objs_cnt++;
at_draw_ordering = 2;										//for user draw obj ordering


int hints_distancex;
int hints_distance_lastx;
int hints_distancey;
int hints_distance_lasty;





//-------- draw traces ---------
for( int i = 0; i < trce.size(); i++ )						//cycle each trace
	{
	bool bshowrect_hints = 0;

	if( sample_rect_showing[i] )							//v1.22
		{
		bshowrect_hints = 1;	
		}

	if( trce[ i ].sample_rect_flicker )						//v1.22
		{
		if( (mousemove_cnt&0x0f) < 0x04 )					//make an infrequent show of sample rectangle hinting
			{
			bshowrect_hints = 1;	
			}
		}



	hints_distancex = -1;									//flag for init far below, used to detect sample rectangle congestion
	hints_distance_lastx;
	hints_distancey = -1;									//flag for init far below, used to detect sample rectangle congestion
	hints_distance_lasty;
	trce[ i ].vrect_hints_px.clear();						//clear sample point hint rects
	trce[ i ].vrect_hints_py.clear();
	trce[ i ].vrect_hints_sel.clear();

	bool lineplot = trce[ i ].lineplot; 

    id = trce[ i ].id;

	int trace_offx = vtrace_offx[ i ];						//get prev calc vals
	int trace_offy = vtrace_offy[ i ];
	int trace_wid;
	int trace_hei;


	int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
	int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
//!	trace_offx = woffx + trce[ i ].border_left;
//!	trace_offy = woffy + trce[ i ].border_top;

	trace_wid = vtrace_wid[ i ];//wid - trace_reductionx;
	trace_hei = vtrace_hei[ i ];//hei - trace_reductiony;

//	sizex = trace_wid;
//	sizey = trace_hei;

	int plot_offsx = vplot_offsx[ i ];							//not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
	int plot_offsy = vplot_offsy[ i ];

//!	double trace_midx = trace_wid / 2;
//!	double trace_midy = trace_hei / 2;

	double trace_midx = vtrace_midx[ i ];
	double trace_midy = vtrace_midy[ i ];

	int cutoffx = trace_wid;// - trce[ i ].border_right;		//stop drawing if x gets to this


	clip_minx = trace_offx;
	clip_maxx = trace_offx + trace_wid;
	clip_miny = trace_offy;
	clip_maxy = trace_offy + trace_hei;

	clip_vportleft = clip_minx;									//these coords 0,0  would be: left,bottom -- used for: 'line_clip()', 'clip_poly()'
	clip_vportright = clip_maxx;
	clip_vporttop = clip_maxy;
	clip_vportbot = clip_miny;



	int style = FL_SOLID;
    style = trce[ i ].line_style;



//	fl_color( trce[ i ].col.r, trce[ i ].col.g, trce[ i ].col.b );
//	fl_line_style ( style, trce[ i ].line_thick );         //for windows you must set line setting after the colour, see the manual


	double deltax, deltay;
	double scalex, scaley;
	double d_midx, d_midy;

//!	double positionx = trce[ i ].posx;
//!	double positiony = -trce[ i ].posy;

	double positionx = vtrace_positionx[ i ];
	double positiony = vtrace_positiony[ i ];

	deltax = trce[ i ].maxx - trce[ i ].minx;
	deltay = trce[ i ].maxy - trce[ i ].miny;


	scalex = vscalex[ i ];
	scaley = vscaley[ i ];

	d_midx = vtrace_dmidx[ i ];
	d_midy = vtrace_dmidy[ i ];

	if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
//!			scalex = (double)trace_wid / deltax;
//!			d_midx = deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
//!			scalex = grat_pixels_x;
//!			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
//!			scalex = grat_pixels_x;
//!			d_midx = 0;//deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
//!			scalex = grat_pixels_x;
//!			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
		{
//!		scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
//!		d_midx = (double)trace_midx / scalex;
		}





	if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
//!				scaley = (double)trace_hei / deltay;
//!			d_midy = deltay / 2.0 + trce[ i ].miny;		//this will move ypos to centre the trace
			}
		else{										    //no deltay
//!				scaley = grat_pixels_y;
//!			d_midy = 0;
			}



        //enforce limits to auto y scaling if req
        if( trce[ i ].b_limit_auto_scale_max_for_y ) 
            {
//!            if( scaley > trce[ i ].limit_auto_scale_max_for_y ) scaley = trce[ i ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
            }
            
        if( trce[ i ].b_limit_auto_scale_min_for_y )
            {
//!            if( scaley < trce[ i ].limit_auto_scale_min_for_y ) scaley = trce[ i ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
            }
            
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
		}



	if( trce[ i ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
//!				scaley = grat_pixels_y;
//!			d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
			}
		else{										    //no deltay
//!				scaley = grat_pixels_y;
//!			d_midy = 0;
			}
		}


	if( trce[ i ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
		{
		double larg_y;
		if( fabs( trce[ i ].maxy ) > fabs( trce[ i ].miny ) ) larg_y = trce[ i ].maxy;
		else larg_y = trce[ i ].miny;
		
//!			scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
//!		d_midy = 0;
		}


	if( trce[ i ].yunits_perpxl > -1 )					//use user spec scale
		{
//!			scaley = 1.0 / trce[ i ].yunits_perpxl;			//scale as directed
//!		d_midy = 0;
		}

	if( i == 0 )										//1st trace?
		{
//!		ref_scalex = scalex;							//remember x scale
//!		ref_d_midx = d_midx;
		}

//printf( "draw() - ref %d, scalex= %g, trce[ i ].xunits_perpxl= %g\n", i, scalex, trce[ i ].xunits_perpxl );

//printf("deltay= %lf, scaley= %lf, d_midy= %lf\n", deltay, scaley, d_midy );
	


	if( i != 0 )										//not 1st trace?
		{
//!		scalex = ref_scalex;							//use 1st trace's x scale
//!		d_midx = ref_d_midx;
		}


	double oldx = 0;
	double oldy = 0;

//scalex = vtrace_scale_x[ i ];
//scaley = vtrace_scale_y[ i ];

/*
    //!!! ALL these MUST have same number of entries !!!!
    vtrace_scale_x.push_back( scalex );                 //store trace params for later, MAKE sure you also do this for vtrace_scale_y, vtrace_positionx, vtrace_positiony, vtrace_dmidx, vtrace_dmidy, vtrace_minx, vtrace_id
    vtrace_scale_y.push_back( scaley );                 //MAKE sure you also do this for trace_offx, trace_offy, trace_midx, trace_midy, trace_id
    vtrace_positionx.push_back( positionx );
    vtrace_positiony.push_back( positiony );
	vtrace_dmidx.push_back( d_midx );
	vtrace_dmidy.push_back( d_midy );
    vtrace_offx.push_back( trace_offx );
    vtrace_offy.push_back( trace_offy );
    vtrace_midx.push_back( trace_midx );
    vtrace_midy.push_back( trace_midy );
    vtrace_minx.push_back( trcmidx );

    vtrace_id.push_back( id );                          //!!! ALL these MUST have same number of entries !!!!
*/

//	double trace_scalex = trce[ i ].scalex;
//	double trace_scaley = trce[ i ].scaley;
	double trace_scalex = vtrace_scale_x[ i ];
	double trace_scaley = vtrace_scale_y[ i ];
			
	double trcminx = vtrace_minx[ i ];									//v1.16

	if( trce[ i ].vis )
		{
        double baseline_y;
        if( trce[ i ].show_as_spectrum )
            {
            double spectrum_baseline_y = trce[ i ].miny; 
            baseline_y = spectrum_baseline_y * scaley - d_midy * scaley;
            }

			//NOTE: the trace colour will be changed at bottom of the pnt plotting loop, dependent on selection, see further down: 'if( pnt_sel )'
			fl_color( trce[ i ].col.r, trce[ i ].col.g, trce[ i ].col.b );
			fl_line_style ( style, trce[ i ].line_thick );         //for windows you must set line setting after the colour, see the manual


//printf("trce[ %d ].minx %f, maxx %f\n", i, trce[ i ].minx, trce[ i ].maxx );
//printf("trace_wid %d, positionx %f, scalex %f, trace_scalex %f d_midx %f d_midx*scalex %f\n", trace_wid, positionx, scalex, trace_scalex, d_midx, d_midx*scalex );
//printf("scalex*trace_scalex %f\n", scalex * trace_scalex );
//exit(0);

//printf( "mgraph::draw() trce %d, d_midx %f\n", i, d_midx );
//printf( "mgraph::draw() trce %d, trace_offx %d positionx %f, plot_offsx %d \n", i, trace_offx, positionx, plot_offsx );

//		double trcminx = trce[ i ].minx;
		for( int j = 0; j < trce[ i ].pnt.size(); j++ )			//cycle each coord point
			{

//			double x1 = trce[ i ].pnt[ j ].x - trce[ i ].minx;
			double x1 = trce[ i ].pnt[ j ].x;
			double y1 = trce[ i ].pnt[ j ].y;
			bool pnt_sel = trce[ i ].pnt[ j ].sel;

//			if( pnt_sel ) fl_color( trce[ i ].group_sel_trace_col.r, trce[ i ].group_sel_trace_col.g, trce[ i ].group_sel_trace_col.b );
//			else fl_color( trce[ i ].col.r, trce[ i ].col.g, trce[ i ].col.b );

			fl_line_style ( style, trce[ i ].line_thick );         //for windows you must set line setting after the colour, see the manual

//printf( "mgraph::draw() trce %d, d_midx %f\n", i, d_midx );

//			x = x1 * scalex * trace_scalex - d_midx * scalex*1; 		//if you change here also change same in: get_trace_to_plot_factors()
			x = (x1-trcminx) * scalex * trace_scalex - d_midx * scalex; //v1.16  -- if you change here also change same in: get_trace_to_plot_factors()
			y = y1 * scaley * trace_scaley - d_midy * scaley;

/*
if( i==-1 )
if( j == 13)
{
double xxxx = 13 * scalex * trace_scalex - d_midx * scalex;
double yyyy = 0.007812 * scaley * trace_scaley - d_midy * scaley;

int xx = nearbyint( trace_midx + xxxx + trace_offx + positionx );
int yy = nearbyint( trace_midy - yyyy + trace_offy + positiony );
printf( "positionx: %f, positiony: %f\n", positionx, positiony );
printf( "scalex: %f, d_midy: %f\n", d_midx, d_midy );
printf( "scalex: %f, trace_scalex: %f\n", scalex, trace_scalex );
printf( "scaley: %f, trace_scalex: %f\n", scaley, trace_scaley );
printf( "trace_midx: %f, trace_offx: %d\n", trace_midx, trace_offx );
printf( "trace_midy: %f, trace_offy: %d\n", trace_midy, trace_offy );
printf( "xx: %d, yy: %d\n", xx, yy );
fl_rect(  xx, yy, 10, 10 );
}
*/
//			if( dbg_graph_id == 3 )
//				{
//				if( j == 50 )
//					{
//			printf("wfmpntscl x: %f, y: %f\n", x, y );
//					}
//				}
            
//printf("y1= %f, scaley= %f, d_midy= %f\n", y1, scaley, d_midy );

//y = y1 * unit_per_div_y - trace_midy * scaley;

			if( j == 0 )
				{
				oldx = x;
				oldy = y;
//				continue;
				}
	//		fl_point( axisx + t * factorx , midy - y );


			int xx = nearbyint( trace_midx + x + trace_offx + positionx + plot_offsx );
			int yy = nearbyint( trace_midy - y + trace_offy + positiony + plot_offsy );

			int xxx = nearbyint( trace_midx + oldx + trace_offx + positionx + plot_offsx );
			int yyy = nearbyint( trace_midy - oldy  + trace_offy + positiony + plot_offsy );


            //store lissajous plot values for later
            if( htime_lissajous != 0 )                      //lissajous req?
                {
                switch( htime_lissajous )
                    {
                    case 1:                 //1:  channel 1-> H , channel 2-> V                              
                        if( id == 0 )                       //channel 1 ?
                            {
                            lissa_col = fl_rgb_color( trce[ i ].col.r, trce[ i ].col.g, trce[ i ].col.b );
                            lissa_line_style = style;
                            lissa_line_thick = trce[ i ].line_thick;
                            
                            lissa_x.push_back( yy );
                            lissa_status_x = 1;
                            }

                        if( id == 1 )                       //channel 2 ?
                            {
                            lissa_y.push_back( yy );
                            lissa_status_y = 1;
                            }
                    break;

                    case 2:                 //2:  channel 2-> H , channel 1-> V                              
                        if( id == 0 )                       //channel 1 ?
                            {
                            lissa_y.push_back( yy );
                            lissa_status_y = 1;
                            }

                        if( id == 1 )                       //channel 2 ?
                            {
                            lissa_col = fl_rgb_color( trce[ i ].col.r, trce[ i ].col.g, trce[ i ].col.b );
                            lissa_line_style = style;
                            lissa_line_thick = trce[ i ].line_thick;
                            lissa_x.push_back( yy );
                            lissa_status_x = 1;
                            }
                    break;


                    case 3:                 //3:   channel 3-> H , channel 4-> V                              
                        if( id == 2 )                       //channel 3 ?
                            {
                            lissa_col = fl_rgb_color( trce[ i ].col.r, trce[ i ].col.g, trce[ i ].col.b );
                            lissa_line_style = style;
                            lissa_line_thick = trce[ i ].line_thick;
                            lissa_x.push_back( yy );
                            lissa_status_x = 1;
                            }

                        if( id == 3 )                       //channel 4 ?
                            {
                            lissa_y.push_back( yy );
                            lissa_status_y = 1;
                            }
                    break;

                    case 4:                 //4:   channel 4-> H , channel 3-> V                               
                        if( id == 2 )                       //channel 3 ?
                            {
                            lissa_y.push_back( yy );
                            lissa_status_y = 1;
                            }

                        if( id == 3 )                       //channel 4 ?
                            {
                            lissa_col = fl_rgb_color( trce[ i ].col.r, trce[ i ].col.g, trce[ i ].col.b );
                            lissa_line_style = style;
                            lissa_line_thick = trce[ i ].line_thick;
                            lissa_x.push_back( yy );
                            lissa_status_x = 1;
                            }
                    break;
                    
                    default:
                    break;
                    }
                }

//printf("trace_midy= %lf, trace_offy= %lf, positiony= %lf\n", trace_midy, trace_offy, positiony );

//			if( xxx > cutoffx ) break;					//don't go past right border

            if( trce[ i ].show_as_spectrum )
                {
                yyy = nearbyint( trace_midy - baseline_y + trace_offy + positiony );

//                yyy = trace_midy - (int)spectrum_baseline_y + trace_offy + positiony;

                if ( lineplot ) clip_draw_line( xx, yy, xx, yyy );                     //draw spectrum
                else fl_point( xx, yy );
                }
            else{
                if( htime_lissajous == 0 ) 
                    {
                    if( xx > plotx_max ) plotx_max = xx;
                    if( yy > ploty_max ) ploty_max = yy;
                    if( xx < plotx_min ) plotx_min = xx;
                    if( yy < ploty_min ) ploty_min = yy;

                    if( xx > plotx_max ) plotx_max = xxx;
                    if( yy > ploty_max ) ploty_max = yyy;
                    if( xx < plotx_min ) plotx_min = xxx;
                    if( yy < ploty_min ) ploty_min = yyy;
                    
					if ( lineplot ) clip_draw_line( xx, yy, xxx, yyy );                  //draw normal trace
					else clip_draw_point( xx, yy );
                    }
                oldx = x;
                oldy = y;
                }

//			if( ( double_click_left && trce[ i ].sample_rect_hints_double_click ) || ( trce[ i ].sample_rect_hints ) )			//show all sample point bounding rect hints?
//				{
				
//				if( trce[ i ].sample_rect_showing ) bshowrect_hints = 1;
//				}

			
			if( bshowrect_hints )										//show all sample point bounding rect hints?
				{
				//simple sample point rect culling
				bool skip = 0;
				if( xx < (clip_minx-1) ) skip = 1;								//minor fudge of -1 to allow samples at 'zero x' to be shown
				if( xx > clip_maxx ) skip = 1;

				if( yy < clip_miny ) skip = 1;
				if( yy > clip_maxy ) skip = 1;


				if( !skip)
					{
					if( hints_distancex == -1 ) { hints_distancex = 0; hints_distance_lastx = xx; }				//init once for this trace
					if( hints_distancey == -1 ) { hints_distancey = 0;hints_distance_lasty = yy; }

	
					hints_distancex += fabs(xx) - fabs(hints_distance_lastx);
//if( i == 0 ) printf("%d, xx: %d - lst %d\n", hints_distancex, xx, hints_distance_lastx);
//if( i == 0 ) printf("%d, yy: %d - lst %d\n", hints_distancey, yy, hints_distance_lasty);
					hints_distance_lastx = xx;
					hints_distancey += fabs(yy) - fabs(hints_distance_lasty);
					hints_distance_lasty = yy;
					
					trce[ i ].vrect_hints_px.push_back( xx );							//store sample point pixel coord for later drawing of bounding rects
					trce[ i ].vrect_hints_py.push_back( yy );
					if( pnt_sel ) trce[ i ].vrect_hints_sel.push_back( 1 );				//flag this rect is sel
					else trce[ i ].vrect_hints_sel.push_back( 0 );
					}
				}

			//change trace col if trace in a selected area
			if( pnt_sel ) fl_color( trce[ i ].group_sel_trace_col.r, trce[ i ].group_sel_trace_col.g, trce[ i ].group_sel_trace_col.b );
			else fl_color( trce[ i ].col.r, trce[ i ].col.g, trce[ i ].col.b );

			fl_line_style ( style, trce[ i ].line_thick );         //for windows you must set line setting after the colour, see the manual
			}
		}



//check if sample point rect hinting congestion is happening (and user optioned to turn it off when it happens)
//clear hinting if distances are too close
if( !(double_click_left & trce[ i ].sample_rect_hints_double_click) )
	{
	if( trce[ i ].vrect_hints_px.size() > 2  )						//is it unlikey there will be hinting congestion
		{
		if( trce[ i ].sample_rect_hints_distancex > 0 )		//option enabled?
			{
//	if(i == 0 ) printf("xx %d, hints_distancex: %d\n", trce[ i ].vrect_hints_px.size(), hints_distancex );
			if( (hints_distancex / trce[ i ].vrect_hints_px.size()) < trce[ i ].sample_rect_hints_distancex )
				{
				trce[ i ].vrect_hints_px.clear();
				trce[ i ].vrect_hints_py.clear();
				}
			}
//		else{
//			trce[ i ].vrect_hints_px.clear();
//			trce[ i ].vrect_hints_py.clear();
//			}
		}

	if( trce[ i ].vrect_hints_py.size() > 2 )						//don't process if just cleared above 
		{
		if( trce[ i ].sample_rect_hints_distancey > 0 )		//option enabled?
			{
//	if(i == 0 ) printf("yy %d, hints_distancey: %d\n", trce[ i ].vrect_hints_px.size(), hints_distancey );

			if( ( hints_distancey / trce[ i ].vrect_hints_py.size()) < trce[ i ].sample_rect_hints_distancey )
				{
				trce[ i ].vrect_hints_px.clear();
				trce[ i ].vrect_hints_py.clear();
				}
			}
		}
	}
/*
    if( htime_lissajous == 1 )
        {
        int x = 0;
        int y = 0;

        int oldy = 100;
        int oldx = -100;
        
        fl_color( 255,255,255);

                int xx = nearbyint( trace_midx + x + trace_offx + positionx );
                int yy = nearbyint( trace_midy - y + trace_offy + positiony );

                int xxx = nearbyint( trace_midx + oldx + trace_offx + positionx );
                int yyy = nearbyint( trace_midy - oldy  + trace_offy + positiony );

        fl_line( xx, yy, xxx, yyy );
    //    fl_line( x1, y1, x2, y2 );
        }
*/

	}
//-------------------------------



//draw user objs that need to appear after graticle and traces
draw_user_objs( at_draw_ordering );
//printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!draw_user_objs_cnt: %d\n", draw_user_objs_cnt );





//------- draw cursors -------
clip_minx = grat_offx;
clip_maxx = grat_offx + grat_wid;
clip_miny = grat_offy;
clip_maxy = grat_offy + grat_hei;

clip_vportleft = clip_minx;									//these coords 0,0  would be: left,bottom -- used for: 'line_clip()', 'clip_poly()'
clip_vportright = clip_maxx;
clip_vporttop = clip_maxy;
clip_vportbot = clip_miny;

//vertical cursors - i.e. measuring time
if( show_cursors_along_y )
	{
	int style;
	double brighter;
	
	int xx, yy;
	calc_draw_coords_internal( ref_id, cursor_along_y0, 0.1, xx, yy );

//printf( "scalex= %g, xxx= %g\n", scalex, 0.005 / scalex );
//printf( "xx= %d, yy= %d\n", xx, yy );

	style = FL_DOT;
	brighter = 1.0;
	if( cursor_sel == 0 )
		{
		style = FL_DASH;
		brighter = 1.1;
		}

	fl_color( col_cursor_along_y.r * brighter, col_cursor_along_y.g * brighter, col_cursor_along_y.b * brighter );
	fl_line_style ( style, 1 );         //for windows you must set line setting after the colour, see the manual

	int y1 = 0;
	int y2 = grat_hei;
printf("grat_offy: %d, %d\n", bkgd_offy, grat_offy );
fl_line( xx, grat_offy + y1, xx, grat_offy + y2 );
//	clip_draw_line( xx, grat_offy + y1, xx, grat_offy + y2 );
	
	style = FL_DOT;
	brighter = 1.0;
	if( cursor_sel == 1 )
		{
		style = FL_DASH;
		brighter = 1.1;
		}

	fl_color( col_cursor_along_y.r * brighter, col_cursor_along_y.g * brighter, col_cursor_along_y.b * brighter );
	fl_line_style ( style, 1 );         //for windows you must set line setting after the colour, see the manual

	calc_draw_coords_internal( ref_id, cursor_along_y1, 0.1, xx, yy );
	y1 = 0;
	y2 = grat_hei;
//	fl_line( xx, grat_offy + y1, xx, grat_offy + y2 );
	clip_draw_line( xx, grat_offy + y1, xx, grat_offy + y2 );
	}


//horiz cursors - i.e. measuring amplitude
if( show_cursors_along_x )
	{
	int style;
	double brighter;
	
	int xx, yy;
	style = FL_DOT;
	brighter = 1.0;
	if( cursor_sel == 2 )
		{
		style = FL_DASH;
		brighter = 1.1;
		}

	fl_color( (double)col_cursor_along_x.r * brighter, (double)col_cursor_along_x.g * brighter, (double)col_cursor_along_x.b * brighter );
	fl_line_style( style, 1 );         //for windows you must set line setting after the colour, see the manual

	calc_draw_coords_internal( ref_id, 0, cursor_along_x0, xx, yy );
	int x1 = 0;
	int x2 = grat_wid;
//	fl_line( grat_offx + x1, yy, grat_offx + x2, yy );
	clip_draw_line( grat_offx + x1, yy, grat_offx + x2, yy );

	style = FL_DOT;
	brighter = 1.0;
	if( cursor_sel == 3 )
		{
		style = FL_DASH;
		brighter = 1.1;
		}

	fl_color( (double)col_cursor_along_x.r * brighter, (double)col_cursor_along_x.g * brighter, (double)col_cursor_along_x.b * brighter );
	fl_line_style ( style, 1 );         //for windows you must set line setting after the colour, see the manual

	calc_draw_coords_internal( ref_id, 0, cursor_along_x1, xx, yy );
	x1 = 0;
	x2 = grat_wid;
//	fl_line( grat_offx + x1, yy, grat_offx + x2, yy );
	clip_draw_line( grat_offx + x1, yy, grat_offx + x2, yy );
	}
//-------------------------------



fl_line_style( FL_SOLID, 1 );


// --- show sample point bounding rects ---
for( int i = 0; i < trce.size(); i++ )
	{
//	if( ( double_click_left & trce[ i ].sample_rect_hints_double_click ) || ( trce[ i ].sample_rect_hints ) ) //v1.22
		{

		fl_color( trce[ i ].sample_rect_hints_col.r, trce[ i ].sample_rect_hints_col.g, trce[ i ].sample_rect_hints_col.b );

		for( int j = 0; j < trce[ i ].vrect_hints_px.size(); j++ )
			{
			if( trce[ i ].vrect_hints_sel[ j ] ) fl_color( trce[ i ].group_sel_rect_col.r, trce[ i ].group_sel_rect_col.g, trce[ i ].group_sel_rect_col.b );	//if selected use this col
			else fl_color( trce[ i ].sample_rect_hints_col.r, trce[ i ].sample_rect_hints_col.g, trce[ i ].sample_rect_hints_col.b );

			int left = trce[ i ].vrect_hints_px[ j ] - rect_size / 2;
			int top = trce[ i ].vrect_hints_py[ j ] - rect_size / 2;

			int right =  left + rect_size;
			int bot = top + rect_size;


//			rect_clip( left , top, rect_size, rect_size );			//clipping dont when 'vrect_hints_px' etc are loaded above, not need to use this here
			fl_rect( left , top, rect_size, rect_size );
			}
		}
	}

fl_line_style( FL_SOLID, 1 );        				 //for windows you must set line setting after the colour, see the manual

//-------------------------------


// ---- show single selected sample ----
for( int i = 0; i < trce.size(); i++ )
	{
	if( trce[ i ].selected_idx != -1 )
		{
		int trace_offx = vtrace_offx[ i ];						//get prev calc vals
		int trace_offy = vtrace_offy[ i ];

		int trace_wid = vtrace_wid[ i ];
		int trace_hei = vtrace_hei[ i ];

		clip_minx = trace_offx;
		clip_maxx = trace_offx + trace_wid;
		clip_miny = trace_offy;
		clip_maxy = trace_offy + trace_hei;

		clip_vportleft = clip_minx;									//these coords 0,0  would be: left,bottom -- used for: 'line_clip()', 'clip_poly()'
		clip_vportright = clip_maxx;
		clip_vporttop = clip_maxy;
		clip_vportbot = clip_miny;

		int xx = trce[ i ].selected_pixel_rect_left + trce[ i ].selected_pixel_rect_wid / 2;		//get back to a coord
		int yy = trce[ i ].selected_pixel_rect_top + trce[ i ].selected_pixel_rect_hei / 2;

		bool skip = 0;
		if( xx < clip_minx ) skip = 1;										//simple sample point rect culling
		if( xx > clip_maxx ) skip = 1;

		if( yy < clip_miny ) skip = 1;
		if( yy > clip_maxy ) skip = 1;

		if( !skip )
			{
			fl_color( trce[ i ].single_sel_col.r, trce[ i ].single_sel_col.g, trce[ i ].single_sel_col.b );
			fl_line_style( FL_SOLID, 1 );        				 //for windows you must set line setting after the colour, see the manual
			rect_clip( trce[ i ].selected_pixel_rect_left , trce[ i ].selected_pixel_rect_top, trce[ i ].selected_pixel_rect_wid + 1, trce[ i ].selected_pixel_rect_hei + 1 );
	//		fl_rect(  trce[ i ].selected_pixel_rect_left , trce[ i ].selected_pixel_rect_top, trce[ i ].selected_pixel_rect_right, trce[ i ].selected_pixel_rect_bot );
			}
		}
	}
//-------------------------------


// ----- draw lissajous captured earlier if any -----
if( lissa_status_x & lissa_status_y )
    {
   
    fl_line_style( lissa_line_style, lissa_line_thick );
//    printf( "htime_lissajous: %d %d\n", lissa_x.size(), lissa_y.size() );

    if( lissa_x.size() > 1 )
        {
        if( lissa_x.size()  == lissa_y.size() )
            {
            fl_color( lissa_col );
			fl_line_style( FL_SOLID, 1 );
           int oldx = lissa_x[ 0 ];
            int oldy = lissa_y[ 0 ];
            
            for( int i = 1; i < lissa_x.size(); i++ )						//cycle each trace
                {
                int x1 = lissa_x[ i ];
                int y1 = lissa_y[ i ];
                
                clip_draw_line( oldx, oldy, x1, y1 );
//                fl_line( oldx, oldy, x1, y1 );
                oldx = x1;
                oldy = y1;
                }

            }
        }
    fl_line_style( FL_SOLID, 1 );
    }

//-------------------------------





fl_pop_clip();




//printf( "plotx_max: %d, ploty_max: %d\n", plotx_max, ploty_max );
//printf( "plotx_min: %d, ploty_min: %d\n", plotx_min, ploty_min );

//double xx, yy;
//get_mouse_vals( mousex, mousey, xx, yy, 1 );

//exit(0);

//draw_cursors();

draw_text();

drw_cnt++;
}













//callback to call when left mouse button is clicked
//non trace specific
void mgraph::set_left_click_anywhere_cb( void (*p_cb)( void* ), void *args_in )
{
left_click_anywhere_cb_p_callback = p_cb;
left_click_anywhere_cb_args = args_in;
}




//callback to call when left mouse button is double clicked
//non trace specific
void mgraph::set_left_double_click_anywhere_cb( void (*p_cb)( void* ), void *args_in )
{
left_double_click_anywhere_cb_p_callback = p_cb;
left_double_click_anywhere_cb_args = args_in;
}




//callback to call when right mouse button is clicked
//non trace specific
void mgraph::set_right_click_anywhere_cb( void (*p_cb)( void* ), void *args_in )
{
right_click_anywhere_cb_p_callback = p_cb;
right_click_anywhere_cb_args = args_in;
}




//callback to call when right mouse button is double clicked
//non trace specific
void mgraph::set_right_double_click_anywhere_cb( void (*p_cb)( void* ), void *args_in )
{
right_double_click_anywhere_cb_p_callback = p_cb;
right_double_click_anywhere_cb_args = args_in;
}






//callback to call when left mouse button is clicked
bool mgraph::set_left_click_cb( int trc, void (*p_cb)( void* ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].left_click_cb_p_callback = p_cb;
trce[ trc ].left_click_cb_args = args_in;

return 1;
}





//callback to call when left mouse button is double clicked
bool mgraph::set_left_double_click_cb( int trc, void (*p_cb)( void* ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].left_double_click_cb_p_callback = p_cb;
trce[ trc ].left_double_click_cb_args = args_in;

return 1;
}







//callback to call when left mouse button is released
void mgraph::set_left_click_release_cb( void (*p_cb)( void* ), void *args_in )
{
left_click_release_cb_p_callback = p_cb;
left_click_release_cb_args = args_in;
}


//when mouse enters the control's border rectangle
void mgraph::set_mouseenter_cb( void (*p_cb)( void* ), void *args_in )
{
mouseenter_cb_p_callback = p_cb;
mouseenter_cb_args = args_in;
}



//when mouse leaves the control's border rectangle
void mgraph::set_mouseleave_cb( void (*p_cb)( void* ), void *args_in )
{
mouseleave_cb_p_callback = p_cb;
mouseleave_cb_args = args_in;
}







//callback to call when middle mouse button is clicked
bool mgraph::set_middle_click_cb( int trc, void (*p_cb)( void* ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].middle_click_cb_p_callback = p_cb;
trce[ trc ].middle_click_cb_args = args_in;

return 1;
}




//callback to call when right mouse button is clicked
bool mgraph::set_right_click_cb( int trc, void (*p_cb)( void* ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].right_click_cb_p_callback = p_cb;
trce[ trc ].right_click_cb_args = args_in;

return 1;
}






//callback to call when right mouse button is double clicked
bool mgraph::set_right_double_click_cb( int trc, void (*p_cb)( void* ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].right_double_click_cb_p_callback = p_cb;
trce[ trc ].right_double_click_cb_args = args_in;

return 1;
}







//callback to call when right mouse button is released
void mgraph::set_right_click_release_cb( void (*p_cb)( void* ), void *args_in )
{
right_click_release_cb_p_callback = p_cb;
right_click_release_cb_args = args_in;
}







//callback to call when keydown
bool mgraph::set_keydown_cb( int trc, void (*p_cb)( void*, int int0 ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].keydown_cb_p_callback = p_cb;
trce[ trc ].keydown_cb_args = args_in;

return 1;
}







//callback to call when keyup
bool mgraph::set_keyup_cb( int trc, void (*p_cb)( void*, int int0 ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].keyup_cb_p_callback = p_cb;
trce[ trc ].keyup_cb_args = args_in;

return 1;
}







//callback to call when mousewheel changes
bool mgraph::set_mousewheel_cb( int trc, void (*p_cb)( void*, int int0 ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].mousewheel_cb_p_callback = p_cb;
trce[ trc ].mousewheel_cb_args = args_in;

return 1;
}







//callback to call when mouse move occurs, int0 is: mousex, int1 is: mousey
bool mgraph::set_mousemove_cb( int trc, void (*p_cb)( void*, int int0, int int1, double dble0, double dble1 ), void *args_in )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

trce[ trc ].mousemove_cb_p_callback = p_cb;
trce[ trc ].mousemove_cb_args = args_in;

return 1;
}





//get background widget pixel offset
void mgraph::get_background_offsxy( int &offx, int &offy )
{
offx = woffx + bkgd_border_left;
offy = woffy + bkgd_border_top;
}



//get graticle widget pixel offset
void mgraph::get_graticle_offsxy( int &offx, int &offy )
{
offx = woffx + bkgd_border_left + graticule_border_left;
offy = woffy + bkgd_border_top + graticule_border_top;
}





//get trace widget pixel offset
bool mgraph::get_trace_offsxy( int trc, int &offx, int &offy )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


offx = woffx + bkgd_border_left + trce[ trc ].border_left;
offy = woffy + bkgd_border_top + trce[ trc ].border_top;
return 1;
}





//relative to widget top/left position, does not remove any borders
void mgraph::get_mouse( int &mx, int &my )
{
mx = mousex;
my = mousey;
}






//the values are pixel coords left is 0, right is wid
void mgraph::get_mouse_pixel_position_on_background( int &mx, int &my )
{
int offx, offy;

get_background_offsxy( offx, offy );

mx = mousex - offx;
my = mousey - offy;
}





//the values are pixel coords left is 0, right is wid
bool mgraph::get_mouse_pixel_position_on_trace( int trc, int &mx, int &my )
{
int offx, offy;

if( !get_trace_offsxy( trc, offx, offy ) ) return 0;

mx = mousex - offx;
my = mousey - offy;

return 1;
}





//SEE !!!! 'get_mouse_position_relative_to_trc_new()'
//these are values relative to trace sample values (not pixel values, see also : 'get_mouse_pixel_position()' )
bool mgraph::get_mouse_position_relative_to_trc( int trc, double &mx, double &my )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

if( trce[ trc ].pnt.size() < 2 ) return 0;

int left_idx, right_idx;

if( !get_left_edge_sample_idx( trc, left_idx ) ) return 0;
if( !get_right_edge_sample_idx( trc, right_idx ) ) return 0;

if( left_idx < 0 ) left_idx = 0;
if( left_idx >= trce[ trc ].pnt.size() ) left_idx = trce[ trc ].pnt.size() - 1;

if( right_idx < 0 ) right_idx = 0;
if( right_idx >= trce[ trc ].pnt.size() ) right_idx = trce[ trc ].pnt.size() - 1;

double x1 = trce[ trc ].pnt[ left_idx ].x;
double x2 = trce[ trc ].pnt[ right_idx ].x;


double xmin, xmax, ymin, ymax;
get_trace_min_max( trc, xmin, xmax, ymin, ymax );

//printf( "trcymin: %f, trcymax: %f\n", ymin, ymax );

double maxy = fabs(ymax);
if( fabs(ymin) > maxy )  maxy = fabs(ymin);									//find which y extreme to use



int offx, offy;
get_trace_offsxy( trc, offx, offy );
//printf( "mgraph::get_mouse_position_relative_to_trc() - offx: %d %d\n", offx, offy );

int wid, hei;
//get_vis_dimensions( wid, hei );
get_trace_vis_dimensions( trc, wid, hei );

//printf( "mgraph::get_mouse_position_relative_to_trc() - wid: %d %d\n", wid, hei );


//printf( "mgraph::get_mouse_position_relative_to_trc() - trce[ trc ].posx: %f %f\n", trce[ trc ].posx, trce[ trc ].posy );

//int grat_offx = woffx + graticule_border_left;
//int grat_offy = woffy + graticule_border_top;

mx = x1 + (x2 - x1) * ( (float)(mousex - offx) / wid );

double offsety = trce[ trc ].posy / (hei / 2);// * trce[ trc ].scaley;

maxy /= trce[ trc ].scaley;

offsety *= maxy;

//printf( "offsety: %f\n", offsety );
my = -maxy + (2.0*maxy) * ( 1.0 - (float)(mousey - offy) / hei );
my -= offsety;

return 1;
}







//based on 'get_mouse_position_relative_to_trc()', but has better 'my' results
//these are values relative to trace sample values (not pixel values, see also : 'get_mouse_pixel_position()' )
bool mgraph::get_mouse_position_relative_to_trc_new( int trc, double &mx, double &my )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

if( trce[ trc ].pnt.size() < 2 ) return 0;

int left_idx, right_idx;

if( !get_left_edge_sample_idx( trc, left_idx ) ) return 0;
if( !get_right_edge_sample_idx( trc, right_idx ) ) return 0;

if( left_idx < 0 ) left_idx = 0;
if( left_idx >= trce[ trc ].pnt.size() ) left_idx = trce[ trc ].pnt.size() - 1;

if( right_idx < 0 ) right_idx = 0;
if( right_idx >= trce[ trc ].pnt.size() ) right_idx = trce[ trc ].pnt.size() - 1;

double x1 = trce[ trc ].pnt[ left_idx ].x;
double x2 = trce[ trc ].pnt[ right_idx ].x;


double xmin, xmax, ymin, ymax;
get_trace_min_max( trc, xmin, xmax, ymin, ymax );

//printf("mgraph::get_mouse_position_relative_to_trc_new() - ymin %f %f  posx %f  plot_offsx %f\n", ymin, ymax, trce[ trc ].posx, trce[ trc ].plot_offsx );


int offx, offy;
get_trace_offsxy( trc, offx, offy );

int wid, hei;
//get_vis_dimensions( wid, hei );
get_trace_vis_dimensions( trc, wid, hei );

mx = x1 + (x2 - x1) * ( (float)(mousex - offx) / wid );

//mx -= trce[ trc ].posx;




//ths is a different method for calc of 'my'
float ctrl = ( ( 1.0 - (float)(mousey - offy) / hei ) - 0.5 ) * 2.0;		//this is 0 at middle of y trace, +1 at top, -1 at bottom

//printf( "mgraph::get_mouse_position_relative_to_trc_new() - ctrl: %f \n", ctrl );
//printf( "mgraph::get_mouse_position_relative_to_trc_new() - [].posy: %f \n",  trce[ trc ].posy );
//printf( "mgraph::get_mouse_position_relative_to_trc_new() - [].scaley: %f \n",  trce[ trc ].scaley );

double midx, midy;
get_trace_midxy( trc, midx, midy );
//printf( "mgraph::get_mouse_position_relative_to_trc_new() - midx: %f %f \n",  midx, midy );


ymax /= trce[ trc ].scaley;
midy /= trce[ trc ].scaley;

my = ( ymax - midy )*ctrl + midy;

//printf( "mgraph::get_mouse_position_relative_to_trc_new() - dd: %f\n",  dd );

return 1;
}

















// 'px, py' are pixel coords
// 'valx, valy' are values the pixel coords would translate to on trace
bool mgraph::get_pixel_position_as_trc_values( int trc, int px, int py, double &valx, double &valy )
{
	
	
px = mousex;
py = mousey;

if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

if( trce[ trc ].pnt.size() < 2 ) return 0;

int left_idx, right_idx;

if( !get_left_edge_sample_idx( trc, left_idx ) ) return 0;
if( !get_right_edge_sample_idx( trc, right_idx ) ) return 0;

if( left_idx < 0 ) left_idx = 0;
if( left_idx >= trce[ trc ].pnt.size() ) left_idx = trce[ trc ].pnt.size() - 1;

if( right_idx < 0 ) right_idx = 0;
if( right_idx >= trce[ trc ].pnt.size() ) right_idx = trce[ trc ].pnt.size() - 1;

double x1 = trce[ trc ].pnt[ left_idx ].x;
double x2 = trce[ trc ].pnt[ right_idx ].x;





double xmin, xmax, ymin, ymax;
get_trace_min_max( trc, xmin, xmax, ymin, ymax );

//printf("mgraph::get_pixel_position_as_trc_values() - ymin %f %f  posx %f  plot_offsx %f\n", ymin, ymax, trce[ trc ].posx, trce[ trc ].plot_offsx );


int offx, offy;
get_trace_offsxy( trc, offx, offy );



int wid, hei;
//get_vis_dimensions( wid, hei );
get_trace_vis_dimensions( trc, wid, hei );

valx = x1 + (x2 - x1) * ( (float)(px - offx) / wid );

//mx -= trce[ trc ].posx;








//offy = trce[ trc ].border_left;


get_background_offsxy( offx, offy );

//printf( "get_pixel_position_as_trc_values() -  px %d %d  offsy %d\n", px, py, offy );
//printf( "get_pixel_position_as_trc_values() -  x1 %f  x2 %f\n", x1, x2 );


get_background_dimensions( wid, hei );

float scle_trc_y = trce[trc].scaley;


//float ctrly = (float)(my ) / (gph[ ii ]->h() - offy) ;
float ctrly = (float)( py - offy ) / (hei) ;

//double dposy;
//gph[ ii ]->get_posx_relative_to_trace( 0, dposy );

float yunits_pp = trce[trc].yunits_perpxl;

float shfty = trce[trc].posy * yunits_pp / scle_trc_y;

//float vis_maxy = (gph[ ii ]->h() - offy) * yunits_pp / scle_trc_y;
float vis_maxy = (hei) * yunits_pp / scle_trc_y;


float vis_miny = -vis_maxy / 2.0f;

valy = vis_miny + vis_maxy * (1.0f - ctrly) - shfty;

//printf( "get_pixel_position_as_trc_values() -  valx %f %f\n", valx, valy );


return 1;
}



















/*//NOTE: NOT for external calling,
//using mouse pos, find if a sample was within a wnd around mouse pos, if so return sample's details (in either:  'trce[ i ].selected_idx'  or  'trce[ i ].hover_sample_idx' )

//v1.08... this function is called by either FL_MOVE, FL_PUSH events, if FL_MOVE event was caller 'get_as_hover' will be set, so that so only 'trce[ i ].hover_sample_idx' is modified

bool mgraph::get_mouse_vals_internal( int trc, int mx, int my, double &xx, double &yy, bool get_as_hover )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


int i = trc;


if( get_as_hover ) trce[ i ].hover_sample_idx = -1;
else trce[ i ].selected_idx = -1;

trce[ i ].mousex_low_sample_idx = -1;
trce[ i ].mousex_high_sample_idx = -1;

//int wid = w();
//int hei = h();

//int trc_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//int tcr_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;



//int bkgd_reductionx = bkgd_border_right + bkgd_border_left + trc_reductionx;
//int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top + trc_reductionx;

//int bkgd_offx;// = woffx + bkgd_border_left;
//int bkgd_offy;// = woffy + bkgd_border_top;

//int bkgd_wid;// = wid - bkgd_reductionx;
//int bkgd_hei;// = hei - bkgd_reductiony;


//get_background_offsxy( bkgd_offx, bkgd_offy );

//get_background_dimensions( bkgd_wid, bkgd_hei );

//int bkgd_midx = bkgd_wid / 2;
//int bkgd_midy = bkgd_hei / 2;




double x, y;






//calc trace sample point locations

//	bool lineplot = trce[ i ].lineplot; 

	int trace_offx;
	int trace_offy;
	int trace_wid;
	int trace_hei;

	get_trace_offsxy( i, trace_offx, trace_offy );

//	trace_wid = wid - trace_reductionx;
//	trace_hei = hei - trace_reductiony;
	get_trace_vis_dimensions( i, trace_wid, trace_hei );


//	int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//	int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
//	trace_offx = woffx + bkgd_border_left + trce[ i ].border_left;
//	trace_offy = woffy + bkgd_border_top + trce[ i ].border_top;

//	trace_wid = wid - trace_reductionx;
//	trace_hei = hei - trace_reductiony;


	double trace_midx = trace_wid / 2;
	double trace_midy = trace_hei / 2;

//	int cutoffx = trace_wid;					//stop drawing if x gets to this


	double deltax, deltay;
	double scalex, scaley;
	double d_midx, d_midy;


	double positionx = trce[ i ].posx;
	double positiony = -trce[ i ].posy;

//if( i == 1 ) printf("trce[ i ].posx: %f\n", trce[ i ].posx );

	deltax = trce[ i ].maxx - trce[ i ].minx;
	deltay = trce[ i ].maxy - trce[ i ].miny;


	if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = (double)trace_wid / deltax;
			d_midx = deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = grat_pixels_x;
			d_midx = 0;//deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
		{
		scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
		d_midx = (double)trace_midx / scalex;
		}





	if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = (double)trace_hei / deltay;
			d_midy = deltay / 2.0 + trce[ i ].miny;		//this will move ypos to centre the trace
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}



        //enforce limits to auto y scaling if req
        if( trce[ i ].b_limit_auto_scale_max_for_y ) 
            {
            if( scaley > trce[ i ].limit_auto_scale_max_for_y ) scaley = trce[ i ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
            }
            
        if( trce[ i ].b_limit_auto_scale_min_for_y )
            {
            if( scaley < trce[ i ].limit_auto_scale_min_for_y ) scaley = trce[ i ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
            }
            
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
		}



	if( trce[ i ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = grat_pixels_y;
			d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}
		}


	if( trce[ i ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
		{
		double larg_y;
		if( fabs( trce[ i ].maxy ) > fabs( trce[ i ].miny ) ) larg_y = trce[ i ].maxy;
		else larg_y = trce[ i ].miny;
		
		scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
		d_midy = 0;
		}


	if( trce[ i ].yunits_perpxl > -1 )					//use user spec scale
		{
		scaley = 1.0 / trce[ i ].yunits_perpxl;			//scale as directed
		d_midy = 0;
		}

	if( i == 0 )										//1st trace?
		{
		ref_scalex = scalex;							//remember x scale
		ref_d_midx = d_midx;
		}

//printf( "draw() - ref %d, scalex= %g, trce[ i ].xunits_perpxl= %g\n", i, scalex, trce[ i ].xunits_perpxl );

//printf("deltay= %lf, scaley= %lf, d_midy= %lf\n", deltay, scaley, d_midy );
	
//	double oldx = 0;
//double oldy = 0;


	if( i != 0 )										//not 1st trace?
		{
		scalex = ref_scalex;							//use 1st trace's x scale
		d_midx = ref_d_midx;
		}


	double trace_scalex = trce[ i ].scalex;
	double trace_scaley = trce[ i ].scaley;
	
	if( trce[ i ].vis )
		{
		for( int j = 0; j < trce[ i ].pnt.size(); j++ )			//cycle each coord point
			{
			double x1 = trce[ i ].pnt[ j ].x;
			double y1 = trce[ i ].pnt[ j ].y;

			x = x1 * scalex * trace_scalex - d_midx * scalex;
			y = y1 * scaley * trace_scaley - d_midy * scaley;

			int xx = nearbyint( trace_midx + x + trace_offx + positionx );
			int yy = nearbyint( trace_midy - y + trace_offy + positiony );


			int left =  xx - rect_size / 2;
			int top =  yy - rect_size / 2;


			int right =  left + rect_size;
			int bot = top + rect_size;


			//check if sample's X value is beyond mousex (note these value are overwritten further below if mouse is actually within a sample's rect)
			if( ( trce[ i ].mousex_low_sample_idx == -1 ) & ( xx > ( mx + woffx ) ) )
				{
				trce[ i ].mousex_low_sample_idx = j - 1;
				trce[ i ].mousex_high_sample_idx = j;

				if( trce[ i ].mousex_low_sample_idx == -1 )			//left of first sample?
					{
					trce[ i ].mousex_low_sample_idx = 0;		
					trce[ i ].mousex_high_sample_idx = 0;
					}
//if( i == 0 )printf("j: %d, grrr %d %d\n", j, trce[ i ].mousex_low_sample_idx, trce[ i ].mousex_high_sample_idx );
				}

			//check if mouse within a sample point
			if( ( ( mx + woffx ) >= left ) && (( mx + woffx ) <=  right ) )
				{

				if( ( ( my + woffy )  >= top ) && ( ( my + woffy ) <=  bot ) )
					{
					trce[ i ].mousex_low_sample_idx = j;
					trce[ i ].mousex_high_sample_idx = j + 1;

					if( trce[ i ].mousex_high_sample_idx >= trce[ i ].pnt.size() ) trce[ i ].mousex_high_sample_idx = j;	//bounds limit

					if( get_as_hover ) 
						{
//printf("hov i: %d\n", i );
//exit(0);
						hover_trace_idx = i;
						trce[ i ].hover_sample_idx = j;
						}
					else{
						trce[ i ].selected_idx = j;

						trce[ i ].selected_pixel_rect_left = left;
						trce[ i ].selected_pixel_rect_wid = rect_size;
						trce[ i ].selected_pixel_rect_top = top;
						trce[ i ].selected_pixel_rect_hei = rect_size;
						}
					break;
					}
				}
			}
		}

if( trce[ i ].mousex_low_sample_idx == -1 )					//no left/right found, then assume past right sample and use its idx
	{
	trce[ i ].mousex_low_sample_idx = trce[ i ].pnt.size() - 1;
	trce[ i ].mousex_high_sample_idx = trce[ i ].mousex_low_sample_idx;
	}

if( !get_as_hover )
	{

	int idx = trce[ i ].selected_idx;

	if( trce[ i ].selected_idx != -1 )
		{
		xx = trce[ i ].pnt[ idx ].x;
		yy = trce[ i ].pnt[ idx ].y;

//		printf( "get_mouse_vals_internal() - pressed sample on trace: %d, selected_idx: %d\n", i, trce[ i ].selected_idx );
		return 1;
		}
	}

return 0;
}

*/












//NOTE: NOT for external calling,
//using mouse pos, find if a sample was within a wnd around mouse pos, if so return sample's details (in either:  'trce[ i ].selected_idx'  or  'trce[ i ].hover_sample_idx' )

//v1.08... this function is called by either FL_MOVE, FL_PUSH events, if FL_MOVE event was caller 'get_as_hover' will be set, so that so only 'trce[ i ].hover_sample_idx' is modified

bool mgraph::get_mouse_vals_internal( int trc, int mx, int my, double &xx, double &yy, bool get_as_hover )
{
if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;


int i = trc;


if( get_as_hover ) trce[ i ].hover_sample_idx = -1;
else trce[ i ].selected_idx = -1;

trce[ i ].mousex_low_sample_idx = -1;
trce[ i ].mousex_high_sample_idx = -1;

//int wid = w();
//int hei = h();

//int trc_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//int tcr_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;



//int bkgd_reductionx = bkgd_border_right + bkgd_border_left + trc_reductionx;
//int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top + trc_reductionx;

//int bkgd_offx;// = woffx + bkgd_border_left;
//int bkgd_offy;// = woffy + bkgd_border_top;

//int bkgd_wid;// = wid - bkgd_reductionx;
//int bkgd_hei;// = hei - bkgd_reductiony;


//get_background_offsxy( bkgd_offx, bkgd_offy );

//get_background_dimensions( bkgd_wid, bkgd_hei );

//int bkgd_midx = bkgd_wid / 2;
//int bkgd_midy = bkgd_hei / 2;




double x, y;



/*


//calc trace sample point locations

//	bool lineplot = trce[ i ].lineplot; 

	int trace_offx;
	int trace_offy;
	int trace_wid;
	int trace_hei;

	get_trace_offsxy( i, trace_offx, trace_offy );

//	trace_wid = wid - trace_reductionx;
//	trace_hei = hei - trace_reductiony;
	get_trace_vis_dimensions( i, trace_wid, trace_hei );


//	int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
//	int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
//	trace_offx = woffx + bkgd_border_left + trce[ i ].border_left;
//	trace_offy = woffy + bkgd_border_top + trce[ i ].border_top;

//	trace_wid = wid - trace_reductionx;
//	trace_hei = hei - trace_reductiony;


	double trace_midx = trace_wid / 2;
	double trace_midy = trace_hei / 2;

//	int cutoffx = trace_wid;					//stop drawing if x gets to this


	double deltax, deltay;
	double scalex, scaley;
	double d_midx, d_midy;


	double positionx = trce[ i ].posx;
	double positiony = -trce[ i ].posy;

//if( i == 1 ) printf("trce[ i ].posx: %f\n", trce[ i ].posx );

	deltax = trce[ i ].maxx - trce[ i ].minx;
	deltay = trce[ i ].maxy - trce[ i ].miny;


	if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = (double)trace_wid / deltax;
			d_midx = deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = grat_pixels_x;
			d_midx = 0;//deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
		{
		scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
		d_midx = (double)trace_midx / scalex;
		}





	if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = (double)trace_hei / deltay;
			d_midy = deltay / 2.0 + trce[ i ].miny;		//this will move ypos to centre the trace
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}



        //enforce limits to auto y scaling if req
        if( trce[ i ].b_limit_auto_scale_max_for_y ) 
            {
            if( scaley > trce[ i ].limit_auto_scale_max_for_y ) scaley = trce[ i ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
            }
            
        if( trce[ i ].b_limit_auto_scale_min_for_y )
            {
            if( scaley < trce[ i ].limit_auto_scale_min_for_y ) scaley = trce[ i ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
            }
            
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
		}



	if( trce[ i ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = grat_pixels_y;
			d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}
		}


	if( trce[ i ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
		{
		double larg_y;
		if( fabs( trce[ i ].maxy ) > fabs( trce[ i ].miny ) ) larg_y = trce[ i ].maxy;
		else larg_y = trce[ i ].miny;
		
		scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
		d_midy = 0;
		}


	if( trce[ i ].yunits_perpxl > -1 )					//use user spec scale
		{
		scaley = 1.0 / trce[ i ].yunits_perpxl;			//scale as directed
		d_midy = 0;
		}

	if( i == 0 )										//1st trace?
		{
		ref_scalex = scalex;							//remember x scale
		ref_d_midx = d_midx;
		}

//printf( "draw() - ref %d, scalex= %g, trce[ i ].xunits_perpxl= %g\n", i, scalex, trce[ i ].xunits_perpxl );

//printf("deltay= %lf, scaley= %lf, d_midy= %lf\n", deltay, scaley, d_midy );
	
//	double oldx = 0;
//double oldy = 0;


	if( i != 0 )										//not 1st trace?
		{
		scalex = ref_scalex;							//use 1st trace's x scale
		d_midx = ref_d_midx;
		}


	double trace_scalex = trce[ i ].scalex;
	double trace_scaley = trce[ i ].scaley;
	
*/

int trace_offx, trace_offy, plot_offsx, plot_offsy;
double trace_scalex, d_midx, scalex, trace_midx, positionx, trace_scaley, d_midy, scaley, trace_midy, positiony;

get_trace_to_plot_factors( i, trace_scalex, d_midx, scalex, trace_midx, trace_offx, positionx, plot_offsx,   trace_scaley, d_midy, scaley, trace_midy, trace_offy, positiony, plot_offsy );



	if( trce[ i ].vis )
		{
		for( int j = 0; j < trce[ i ].pnt.size(); j++ )			//cycle each coord point
			{
			double x1 = trce[ i ].pnt[ j ].x;
			double y1 = trce[ i ].pnt[ j ].y;

//			x = (x1) * scalex * trace_scalex - d_midx * scalex;
			x = (x1-trce[ i ].minx) * scalex * trace_scalex - d_midx * scalex;	//v1.16
			y = y1 * scaley * trace_scaley - d_midy * scaley;

			int xx = nearbyint( trace_midx + x + trace_offx + positionx + plot_offsx );
			int yy = nearbyint( trace_midy - y + trace_offy + positiony + plot_offsy );


			int left =  xx - rect_size / 2;
			int top =  yy - rect_size / 2;


			int right =  left + rect_size;
			int bot = top + rect_size;


			//check if sample's X value is beyond mousex (note these value are overwritten further below if mouse is actually within a sample's rect)
			if( ( trce[ i ].mousex_low_sample_idx == -1 ) & ( xx > ( mx + woffx ) ) )
				{
				trce[ i ].mousex_low_sample_idx = j - 1;
				trce[ i ].mousex_high_sample_idx = j;

				if( trce[ i ].mousex_low_sample_idx == -1 )			//left of first sample?
					{
					trce[ i ].mousex_low_sample_idx = 0;		
					trce[ i ].mousex_high_sample_idx = 0;
					}
//if( i == 0 )printf("j: %d, grrr %d %d\n", j, trce[ i ].mousex_low_sample_idx, trce[ i ].mousex_high_sample_idx );
				}

			//check if mouse within a sample point
			if( ( ( mx + woffx ) >= left ) && (( mx + woffx ) <=  right ) )
				{

				if( ( ( my + woffy )  >= top ) && ( ( my + woffy ) <=  bot ) )
					{
					trce[ i ].mousex_low_sample_idx = j;
					trce[ i ].mousex_high_sample_idx = j + 1;

					if( trce[ i ].mousex_high_sample_idx >= trce[ i ].pnt.size() ) trce[ i ].mousex_high_sample_idx = j;	//bounds limit

					if( get_as_hover ) 
						{
//printf("hov i: %d\n", i );
//exit(0);
						hover_trace_idx = i;
						trce[ i ].hover_sample_idx = j;
						}
					else{
						trce[ i ].selected_idx = j;

						trce[ i ].selected_pixel_rect_left = left;
						trce[ i ].selected_pixel_rect_wid = rect_size;
						trce[ i ].selected_pixel_rect_top = top;
						trce[ i ].selected_pixel_rect_hei = rect_size;
						}
					break;
					}
				}
			}
		}

if( trce[ i ].mousex_low_sample_idx == -1 )					//no left/right found, then assume past right sample and use its idx
	{
	trce[ i ].mousex_low_sample_idx = trce[ i ].pnt.size() - 1;
	trce[ i ].mousex_high_sample_idx = trce[ i ].mousex_low_sample_idx;
	}

if( !get_as_hover )
	{

	int idx = trce[ i ].selected_idx;

	if( trce[ i ].selected_idx != -1 )
		{
		xx = trce[ i ].pnt[ idx ].x;
		yy = trce[ i ].pnt[ idx ].y;

//		printf( "get_mouse_vals_internal() - pressed sample on trace: %d, selected_idx: %d\n", i, trce[ i ].selected_idx );
		return 1;
		}
	}

return 0;
}















//calc integer trace pixel location, for spec trace's sample,
//NOTE: the coords for are suitable for use with: 'draw_user_objs()', e.g: the returned pixel: 0,0  would be top left corner of visble graticle,
//SEE ALSO: 'calc_draw_coords()'
bool mgraph::calc_draw_coords_for_trc_sample( int trc, int samp_idx, int &ix, int &iy )
{
double dx, dy;

if ( trce.size() == 0 ) return 0;

if ( trc < 0 ) return 0;
if( trc >= trce.size() ) return 0;

int count;
if( !get_sample_count_for_trc( trc, count ) ) return 0;

if( count == 0 ) return 0;
if( samp_idx < 0 ) return 0;
if( samp_idx >= count ) return 0;

double xx = trce[ trc ].pnt[ samp_idx ].x;
double yy = trce[ trc ].pnt[ samp_idx ].y;


int id = find_trace_id_from_trace_idx( trc );

if( id == -1 ) return 0;

if( !calc_draw_coords( id, xx, yy, ix, iy ) )  return 0;

return 1;
}









//SEE ALSO: 'calc_draw_coords_for_trc_sample()'
//NOTE: the coords for are suitable for use with: 'draw_user_objs()', e.g: the returned pixel: 0,0  would be top left corner of visble graticle,
bool mgraph::calc_draw_coords( int trc_id, double inx, double iny, int &xx, int &yy )
{

if( !calc_draw_coords_internal( trc_id, inx, iny, xx, yy ) ) return 0;

int bkgd_reductionx = bkgd_border_right + bkgd_border_left;
int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top;

int bkgd_offx = woffx + bkgd_border_left;
int bkgd_offy = woffy + bkgd_border_top;

//printf("calc_draw_coords: %d, %d, %d, %d\n", bkgd_offx, bkgd_offy, xx, yy);
xx -= bkgd_offx;
yy -= bkgd_offy;
//printf("calc_draw_coords1: %d, %d\n", xx, yy);
//exit(0);

return 1;

}





// !!FOR INTERNAL USE ONLY
//calc integer trace pixel location, uses spec id to find the trace to use as reference,
//NOTE: the coords will be OFFSET to take widget's position + border into account, used for h/v cursor drawing for example in the 'draw()' function,
//NOTE: the coords are not sutable for use in 'draw_user_objs()' as they are offset by widget's position + border, INSTEAD USE: 'calc_draw_coords()'
bool mgraph::calc_draw_coords_internal( int trc_id, double inx, double iny, int &xx, int &yy )
{
if ( trce.size() == 0 ) return 0;

int i;

for( i = 0; i < trce.size(); i++ )
    {
    if( trc_id == trce[ i ].id  ) goto found_id;
    }

return 0;


found_id:

/*
int wid = w();
int hei = h();


int bkgd_reductionx = bkgd_border_right + bkgd_border_left;
int bkgd_reductiony = bkgd_border_bottom + bkgd_border_top;

int bkgd_offx = woffx + bkgd_border_left;
int bkgd_offy = woffy + bkgd_border_top;

int bkgd_wid = wid - bkgd_reductionx;
int bkgd_hei = hei - bkgd_reductiony;

int bkgd_midx = bkgd_wid / 2;
int bkgd_midy = bkgd_hei / 2;


double x, y;






//calc trace sample point location

//bool lineplot = trce[ i ].lineplot; 

int trace_offx;
int trace_offy;
int trace_wid;
int trace_hei;


int trace_reductionx = trce[ i ].border_right + trce[ i ].border_left;
int trace_reductiony = trce[ i ].border_bottom + trce[ i ].border_top;
trace_offx = woffx + bkgd_border_left + trce[ i ].border_left;
trace_offy = woffy + bkgd_border_top + trce[ i ].border_top;

trace_wid = wid - trace_reductionx;
trace_hei = hei - trace_reductiony;


double trace_midx = trace_wid / 2;
double trace_midy = trace_hei / 2;

int cutoffx = trace_wid;					//stop drawing if x gets to this


double deltax, deltay;
double scalex, scaley;
double d_midx, d_midy;


	double positionx = trce[ i ].posx;
	double positiony = -trce[ i ].posy;


	deltax = trce[ i ].maxx - trce[ i ].minx;
	deltay = trce[ i ].maxy - trce[ i ].miny;


	if( trce[ i ].xunits_perpxl == -1 )			//use auto scaling, to span x raster
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = (double)trace_wid / deltax;
			d_midx = deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl == -2 )		    //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltax != 0 )						//avoid poss div by zero
			{
			scalex = grat_pixels_x;
			d_midx = 0;//deltax / 2.0 + trce[ i ].minx;
			}
		else{									//no deltax
			scalex = grat_pixels_x;
			d_midx = 0;
			}
		}


	if( trce[ i ].xunits_perpxl > -1 )					//use user spec scale
		{
		scalex = 1.0 / trce[ i ].xunits_perpxl;			//scale as directed
		d_midx = (double)trace_midx / scalex;
		}





	if( trce[ i ].yunits_perpxl == -1 )					//use auto scaling
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = (double)trace_hei / deltay;
			d_midy = deltay / 2.0 + trce[ i ].miny;		//this will move ypos to centre the trace
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}



        //enforce limits to auto y scaling if req
        if( trce[ i ].b_limit_auto_scale_max_for_y ) 
            {
            if( scaley > trce[ i ].limit_auto_scale_max_for_y ) scaley = trce[ i ].limit_auto_scale_max_for_y;      //limit how much scale magnification is displayed for small sample transitions
            }
            
        if( trce[ i ].b_limit_auto_scale_min_for_y )
            {
            if( scaley < trce[ i ].limit_auto_scale_min_for_y ) scaley = trce[ i ].limit_auto_scale_min_for_y;      //limit how much scale reduction is displayed for large sample transitions
            }
            
//        printf(" scaley: %g, %g\n", trace_hei / deltay, scaley );
		}



	if( trce[ i ].yunits_perpxl == -2 )		            //use graticle unit/division, so 1.0 amounts to 1 div
		{
		if( deltay != 0 )							    //avoid poss div by zero
			{
			scaley = grat_pixels_y;
			d_midy = 0;//deltay / 2.0 + trce[ i ].miny;
			}
		else{										    //no deltay
			scaley = grat_pixels_y;
			d_midy = 0;
			}
		}


	if( trce[ i ].yunits_perpxl == -3 )					//use which ever is greater absolute wise: maxy or miny, to calc the scaley to use
		{
		double larg_y;
		if( fabs( trce[ i ].maxy ) > fabs( trce[ i ].miny ) ) larg_y = trce[ i ].maxy;
		else larg_y = trce[ i ].miny;
		
		scaley = (double)trace_hei / ( ( fabs( larg_y  ) * 2.0 )  );					//the the larger and set scale to suit
		d_midy = 0;
		}


	if( trce[ i ].yunits_perpxl > -1 )					//use user spec scale
		{
		scaley = 1.0 / trce[ i ].yunits_perpxl;			//scale as directed
		d_midy = 0;
		}

	if( i == 0 )										//1st trace?
		{
		ref_scalex = scalex;							//remember x scale
		ref_d_midx = d_midx;
		}





//x = x1 * scalex - d_midx * scalex;
//y = y1 * scaley - d_midy * scaley;

double trace_scalex = trce[ i ].scalex;
double trace_scaley = trce[ i ].scaley;
*/

double x1 = inx;
double y1 = iny;

int trace_offx, trace_offy, plot_offsx, plot_offsy;
double trace_scalex, d_midx, scalex, trace_midx, positionx, trace_scaley, d_midy, scaley, trace_midy, positiony;

get_trace_to_plot_factors( i, trace_scalex, d_midx, scalex, trace_midx, trace_offx, positionx, plot_offsx,   trace_scaley, d_midy, scaley, trace_midy, trace_offy, positiony, plot_offsy );


//double x = x1 * scalex * trace_scalex - d_midx * scalex;
double x = (x1 - trce[ i ].minx) * scalex * trace_scalex - d_midx * scalex;				//v1.16
double y = y1 * scaley * trace_scaley - d_midy * scaley;


xx = nearbyint( trace_midx + x + trace_offx + positionx + plot_offsx );
yy = nearbyint( trace_midy - y + trace_offy + positiony + plot_offsy );

/*
if(0)
{
double xxxx = 13 * scalex * trace_scalex - d_midx * scalex;
double yyyy = 0.007812 * scaley * trace_scaley - d_midy * scaley;

int xx = nearbyint( trace_midx + xxxx + trace_offx + positionx );
int yy = nearbyint( trace_midy - yyyy + trace_offy + positiony );
printf( "positionx: %f, positiony: %f\n", positionx, positiony );
printf( "scalex: %f, d_midy: %f\n", d_midx, d_midy );
printf( "scalex: %f, trace_scalex: %f\n", scalex, trace_scalex );
printf( "scaley: %f, trace_scalex: %f\n", scaley, trace_scaley );
printf( "trace_midx: %f, trace_scalex: %d\n", trace_midx, trace_offx );
printf( "trace_midy: %f, trace_scalex: %d\n", trace_midy, trace_offy );
printf( "xx: %d, yy: %d\n", xx, yy );
fl_rect(  xx, yy, 10, 10 );
}
*/

return 1;
}




void mgraph::resize( int xx, int yy, int ww, int hh )
{
rect_size = 10 / 2;

woffx = xx;
woffy = yy;

Fl_Box::resize( xx, yy, ww, hh );
redraw();
}





int mgraph::handle( int e )
{
bool need_redraw = 0;
bool dont_pass_on = 0;

double xx, yy;

if ( e == FL_FOCUS )	
	{
    dont_pass_on = 1;
    }


if ( e == FL_ENTER )	
	{
	inside_control = 1;

	if( mouseenter_cb_p_callback )
		{
		mouseenter_cb_p_callback( mouseenter_cb_args );
		}
	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e == FL_LEAVE )	
	{
	inside_control = 0;
	mousemove_cnt = 4;													//set this to a value that won't show sample rect hinting								

	if( mouseleave_cb_p_callback )
		{
		mouseleave_cb_p_callback( mouseleave_cb_args );
		}
	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e & FL_MOVE )
	{
	int msex = Fl::event_x();
	int msey = Fl::event_y();
	
	bool mse_mve = 0;
	
	if( msex != mousex ) mse_mve = 1;
	if( msey != mousey ) mse_mve = 1;
	
	mousex = msex;
	mousey = msey;

	if ( inside_control )
		{
        hover_trace_idx = -1;															//this is set in: get_mouse_vals_internal()

        for( int i = 0; i < trce.size(); i++ )
            {
			get_mouse_vals_internal( i, mousex - woffx, mousey - woffy, xx, yy, 1 );	   //find if a sample is within a wnd around mouse pos, set 'hov_sample_idx' in 'trace_tag'


			if( trce[ i ].mousemove_cb_p_callback )
				{
				int graticule_reductionx = graticule_border_right + graticule_border_left;
				int graticule_reductiony = graticule_border_bottom + graticule_border_top;

				int grat_offx = woffx + graticule_border_left;
				int grat_offy = woffy + graticule_border_top;

				int grat_wid = w() - graticule_reductionx;
				int grat_hei = h() - graticule_reductiony;

				if( ( mousex >= grat_offx ) & ( mousex <= grat_offx + grat_wid ) )
					{
					if( ( mousey >= grat_offy ) & ( mousey <= grat_offy + grat_hei ) )
						{
						last_mousex = mousex - grat_offx;								//express mouse xpos as value between (left) 0 -->??(right)
						last_mousey = mousey - grat_offy;								//express mouse xpos as value between (top) 0 -->??(botm)
						last_nomalised_mousex = 2.0 * (double)last_mousex / (double)grat_wid - 1.0;	 	//express mouse xpos as value between (left)-1.0 --> 0.0 --> +1.0 (right)
						last_nomalised_mousey = -(2.0 * (double)last_mousey / (double)grat_hei - 1.0);	//express mouse ypos as value between (botm)-1.0 --> 0.0 --> +1.0 (top)

						trce[ i ].mousemove_cb_p_callback( trce[ i ].mousemove_cb_args, last_mousex,  last_mousey, last_nomalised_mousex, last_nomalised_mousey );


						}
					}
				}
            }
		if( mse_mve ) mousemove_cnt++;	//v1.22

        need_redraw = 1;
        dont_pass_on = 1;
        }
    }


if ( e == FL_PUSH )	
	{
	mousex = Fl::event_x() - woffx;
	mousey = Fl::event_y() - woffy;
	double_click_left = Fl::event_clicks();

    last_clicked_trace = -1;


	for( int i = 0; i < trce.size(); i++ )								//toggle sample rectangle hinting
		{
		if( double_click_left == 2 )
			{
			if( trce[ i ].sample_rect_hints_double_click ) sample_rect_showing[ i ] = !sample_rect_showing[ i ];		//v1.22
			}
		}

	for( int i = 0; i < trce.size(); i++ )
		{
        if( get_mouse_vals_internal( i, mousex, mousey, xx, yy, 0 ) )   //find if a sample is within a wnd around mouse pos, if so get sample's details
            {
            last_clicked_trace = i;
            break;
            }
        }


    //these are non trace specific callbacks
    if( Fl::event_button() == 1 )
        {
		left_button = 1;

        if( double_click_left == 0 )
            {
            if( left_click_anywhere_cb_p_callback ) left_click_anywhere_cb_p_callback( left_click_anywhere_cb_args );
            }
        else{
            if( left_double_click_anywhere_cb_p_callback ) left_double_click_anywhere_cb_p_callback( left_double_click_anywhere_cb_args );
            }
        }



    if( Fl::event_button() == 3 )
        {
		right_button = 1;
        if( double_click_left == 0 )
            {
            if( right_click_anywhere_cb_p_callback ) right_click_anywhere_cb_p_callback( right_click_anywhere_cb_args );
            }
        else{
            if( right_double_click_anywhere_cb_p_callback ) right_double_click_anywhere_cb_p_callback( right_double_click_anywhere_cb_args );
            }
        }



    //these are trace specific callbacks
    if( last_clicked_trace != -1 )                  //was a trace clicked?
        {
        int i = last_clicked_trace;

        if( Fl::event_button() == 1 )
            {

            if( double_click_left == 0 )
                {
                if( trce[ i ].left_click_cb_p_callback ) trce[ i ].left_click_cb_p_callback( trce[ i ].left_click_cb_args );
                }
            else{
                if( trce[ i ].left_double_click_cb_p_callback ) trce[ i ].left_double_click_cb_p_callback( trce[ i ].left_double_click_cb_args );
                }
            }


        if( Fl::event_button() == 2 )
            {

            if( double_click_left == 0 )
                {
                if( trce[ i ].middle_click_cb_p_callback ) trce[ i ].middle_click_cb_p_callback( trce[ i ].middle_click_cb_args );
                }
            else{
//                if( trce[ i ].left_double_click_cb_p_callback ) trce[ i ].left_double_click_cb_p_callback( trce[ i ].left_double_click_cb_args );
                }
            }


        if( Fl::event_button() == 3 )
            {

            if( double_click_left == 0 )
                {
                if( trce[ i ].right_click_cb_p_callback ) trce[ i ].right_click_cb_p_callback( trce[ i ].right_click_cb_args );
                }
            else{
                if( trce[ i ].right_double_click_cb_p_callback ) trce[ i ].right_double_click_cb_p_callback( trce[ i ].right_double_click_cb_args );
                }
            }
        }

    take_focus();
	need_redraw = 1;
	dont_pass_on = 1;
	}




if ( e == FL_RELEASE )	
	{

    //these are non trace specific callbacks
    if( Fl::event_button() == 1 )
        {
		left_button = 0;
        if( left_click_release_cb_p_callback ) left_click_release_cb_p_callback( left_click_release_cb_args );
        }

	if( Fl::event_button() == 2 )
		{
		middle_button = 0;
		}

    if( Fl::event_button() == 3 )
        {
 		right_button = 0;
		if( right_click_release_cb_p_callback ) right_click_release_cb_p_callback( right_click_release_cb_args );
        }

	need_redraw = 1;
	dont_pass_on = 1;
	}




if ( ( e == FL_KEYDOWN ) || ( e == FL_SHORTCUT ) )						//key press?
	{
	int key = Fl::event_key();
	for( int i = 0; i < trce.size(); i++ )
		{
        if( trce[ i ].keydown_cb_p_callback ) trce[ i ].keydown_cb_p_callback( trce[ i ].keydown_cb_args, key );
		}
	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( ( e == FL_KEYUP) || ( e == FL_SHORTCUT ) )							//key release?
	{
	int key = Fl::event_key();
	for( int i = 0; i < trce.size(); i++ )
		{
        if( trce[ i ].keyup_cb_p_callback ) trce[ i ].keyup_cb_p_callback( trce[ i ].keyup_cb_args, key );
		}
	need_redraw = 1;
	dont_pass_on = 1;
	}


if ( e == FL_MOUSEWHEEL )
	{
	if ( inside_control )
		{
        int dir = Fl::event_dy();
                
        if( b_invert_wheel ) dir = -dir;
        
		mousewheel += dir;

        for( int i = 0; i < trce.size(); i++ )
            {
            if( trce[ i ].mousewheel_cb_p_callback ) trce[ i ].mousewheel_cb_p_callback( trce[ i ].mousewheel_cb_args, dir );
            }
        need_redraw = 1;
        dont_pass_on = 1;
        }
    }



if ( need_redraw ) redraw();

if( dont_pass_on ) return 1;

return Fl_Widget::handle( e );
}


//----------------------------------------------------------











//------------------------------ fast_mgraph ----------------------------------
//SEE EXAMPLE CODE: 'test_fast_mgraph()'

//simple to use graphing class


void cb_fast_mgraph_menu_open_save(Fl_Widget *w, void *v )
{
int which = (int)v;

fl_alert( "Not yet implemented", 0 );
if( which == 0 )
	{

	}
}



void cb_fast_mgraph_menu_edit(Fl_Widget *w, void *v )
{
int which = (int)v;

fl_alert( "Not yet implemented", 0 );
if( which == 0 )
	{

	}
}






void cb_fast_mgraph_menu_sample(Fl_Widget *w, void *v )
{
string s1;
	
int which = (int)v;

Fl_Widget *ow = (Fl_Widget*)w;
fast_mgraph *op = (fast_mgraph*)w->parent();


if( which == 0 )
	{
	op->b_prev_plot_recording = !op->b_prev_plot_recording ;
	
	}

if( which == 1 )
	{
	op->plot_updating_state = 1;
	}

if( which == 2 )
	{
	op->plot_updating_state = 0;
	}

if( which ==10 )
	{
	op->plot_updating_state = 2;
	op->prev_plot_show_choice = 0;
	}

if( which == 11 )
	{
	op->plot_updating_state = 2;
	op->prev_plot_show_choice = 1;
	}

if( which == 12 )
	{
	op->plot_updating_state = 2;
	op->prev_plot_show_choice = 2;
	}

if( which == 13 )
	{
	op->plot_updating_state = 2;
	op->prev_plot_show_choice = 3;
	}

}






void cb_fast_mgraph_help( Fl_Widget *w, void *v )
{
string s1, st;
s1 = "Left click and drag to move traces on x-axis.\n\n";
st += s1;

s1 = "Press 's' to select first sample on first trace if nothing is currently selected.\n";
st += s1;

s1 = "Press 't' to toggle between traces. Use cursor keys to navigate samples.\n";
st += s1;

s1 = "Use mousewheel to change 'x' scale (press 'x' to restore x scale to unity).\n\n";
st += s1;

s1 = "Press 'f' to find/center trace.\n";
st += s1;

s1 = "Press 'm' and spin mousewheel to change value that will display with max y deflection.\n";
st += s1;

s1 = "Press 'a' on a trace and spin mousewheel to change its amplitude (y scaling), 'z' will force any trace scaling to unity.\n";
st += s1;

s1 = "Press 'y' and spin mousewheel to change y position of trace, on a per trace basis ('z' to restore).\n\nClick 2 sample points in succession to see delta x,y and Hz.\n";
st += s1;

s1 = "Tripple click to toggle rectangle hinting of all sample points.";
st += s1;

fl_alert( st.c_str(), 0 );
}

//----------------------------- Main Menu --------------------------
Fl_Menu_Item fast_mgraph_menuitems[] =
{
	{ "&File",              0, 0, 0, FL_SUBMENU },
//		{ "&OpenFolderTest", FL_CTRL + 'o'	, (Fl_Callback *)cb_open_folder, 0 },
		{ "&SaveWve...", 0	, (Fl_Callback *)cb_fast_mgraph_menu_open_save,(void*) 0 },
		{ "&SaveAu...", 0	, (Fl_Callback *)cb_fast_mgraph_menu_open_save, (void*) 1 },
		{ "&SaveCsv...", 0	, (Fl_Callback *)cb_fast_mgraph_menu_open_save, (void*) 2 },
		{ "&SaveRaw...", 0	, (Fl_Callback *)cb_fast_mgraph_menu_open_save, (void*) 2 },
//		{ "&Save", FL_CTRL + 's'	, (Fl_Callback *)cb_btSave, 0, FL_MENU_DIVIDER },
//		{ "E&xit", FL_CTRL + 'q'	, (Fl_Callback *)cb_btQuit, 0 },
		{ 0 },

	{ "&Edit", 0, 0, 0, FL_SUBMENU },
//		{ "&Undo",  FL_CTRL + 'z', (Fl_Callback *)cb_fast_mgraph_menu_edit},
//		{ "&Redo",  FL_CTRL + 'y', (Fl_Callback *)cb_edit_redo, 0, FL_MENU_DIVIDER },
//		{ "Cu&t",  FL_CTRL + 'x', (Fl_Callback *)cb_edit_cut },
//		{ "&Copy",  FL_CTRL + 'c', (Fl_Callback *)cb_edit_copy },
//		{ "Past&e",  FL_CTRL + 'v', (Fl_Callback *)cb_edit_paste, 0, FL_MENU_DIVIDER },
//		{ "&Preferences..",  0, (Fl_Callback *)cb_pref},
//		{ "&Preferences2..",  0, (Fl_Callback *)cb_pref2},
//		{ "&Font Pref..",  0, (Fl_Callback *)cb_font_pref},	
		{ 0 },


	{ "&Sample", 0, 0, 0, FL_SUBMENU },
		{ "&AllowPlotRecording",  0, (Fl_Callback *)cb_fast_mgraph_menu_sample, (void*)0,  FL_MENU_TOGGLE | FL_MENU_DIVIDER },
		{ "&ContinuousPlot",  0, (Fl_Callback *)cb_fast_mgraph_menu_sample, (void*)1,  FL_MENU_RADIO|FL_MENU_VALUE },
		{ "&PausePlot",  0, (Fl_Callback *)cb_fast_mgraph_menu_sample, (void*)2,  FL_MENU_RADIO },
		{ "&LastRecPlot",  0, (Fl_Callback *)cb_fast_mgraph_menu_sample, (void*)10,  FL_MENU_RADIO },
		{ "&2ndLastRecPlot",  0, (Fl_Callback *)cb_fast_mgraph_menu_sample, (void*)11,  FL_MENU_RADIO },
		{ "&3rdLastRecPlot",  0, (Fl_Callback *)cb_fast_mgraph_menu_sample, (void*)12,  FL_MENU_RADIO },
		{ "&4thLastRecPlot",  0, (Fl_Callback *)cb_fast_mgraph_menu_sample, (void*)13,  FL_MENU_RADIO },
//		{ "&Redo",  FL_CTRL + 'y', (Fl_Callback *)cb_edit_redo, 0, FL_MENU_DIVIDER },
//		{ "Cu&t",  FL_CTRL + 'x', (Fl_Callback *)cb_edit_cut },
//		{ "&Copy",  FL_CTRL + 'c', (Fl_Callback *)cb_edit_copy },
//		{ "Past&e",  FL_CTRL + 'v', (Fl_Callback *)cb_edit_paste, 0, FL_MENU_DIVIDER },
//		{ "&Preferences..",  0, (Fl_Callback *)cb_pref},
//		{ "&Preferences2..",  0, (Fl_Callback *)cb_pref2},
//		{ "&Font Pref..",  0, (Fl_Callback *)cb_font_pref},	
		{ 0 },



//	{ "&Sel",              0, 0, 0, FL_SUBMENU },
//		{ "&ToggleSelTrace", 0	, (Fl_Callback *)cb_fast_mgraph_toggle_sel_trace,(void*) 0 },
//		{ 0 },
		
//	{ "&DropWnd", 0, (Fl_Callback *)cb_show_mywnd, 0, },
//	{ "&UnicodeWnd", 0, (Fl_Callback *)cb_show_unicode, 0, },

	{ "&Help", 0, 0, 0, FL_SUBMENU },
		{ "&Help", 0				, (Fl_Callback *)cb_fast_mgraph_help, 0  },
//		{ "&About", 0				, (Fl_Callback *)cb_btAbout, 0 },
		{ 0 },


	{ 0 }
};
//--------------------------------------------------------------------






void cb_fast_gph_left_click_anywhere( void*v )
{
mgraph *o = (mgraph*)v;
fast_mgraph *p = (fast_mgraph*)o->parent();



int my;
o->get_mouse_pixel_position_on_background( p->drag_start_mousex, my );					//get values for dragging
o->get_posx( 0, p->drag_start_posx );
p->drag_started = 0;


int ii = o->graph_id;

//selection box
if( p->keyshift[ ii ] )
	{
	int left, right;
	o->get_mousex_sample_boundary_idx( 0, left, right );		//get right sample for starters, see mouse move cb

	if( p->sel_idx1[ ii ] == -1 ) 
		{
		p->sel_idx1[ ii ] = left;
		p->sel_idx2[ ii ] = left + 1;
		}
	else{
		if( right <= p->sel_idx1[ ii ] ) p->sel_idx2[ ii ] = right - 1;
		else  p->sel_idx2[ ii ] = right;
		}
//printf( "left: %d, %d\n",p->sel_idx1[ ii ],p->sel_idx2[ ii ] );

	}
else{
//	p->last_sel_trc = -1;												//v1.22
//	p->sel_idx1[ ii ] = -1;
//	p->sel_idx2[ ii ] = -1;
	}

}








void cb_fast_gph_right_click_anywhere( void*v )
{
mgraph *o = (mgraph*)v;
fast_mgraph *p = (fast_mgraph*)o->parent();

int ii = o->graph_id;

p->last_sel_trc = -1;													//v1.22
p->sel_idx1[ ii ] = -1;
p->sel_idx2[ ii ] = -1;
}







void cb_fast_gph_left_click_release( void*v )
{
mgraph *o = (mgraph*)v;
fast_mgraph *p = (fast_mgraph*)o->parent();

int ii = o->graph_id;

int trc = p->last_sel_trc;

if( trc >= 0 )
	{
	int samp_idx = p->last_sel0_idx[ ii ];
	
	o->set_selected_sample( trc, samp_idx, 0 );
	}

}





void cb_fast_gph_mouseenter( void* args )
{
//Fl::focus( (Fl_Window*)args );

//mgraph *o = (mgraph* )args;	


mgraph *o = (mgraph*)args;
fast_mgraph *p = (fast_mgraph*)o->parent();

if( p->b_bring_window_to_front ) Fl::focus( (Fl_Window*)args );
}






void cb_fast_gph_mouseleave( void* args )
{
}



void cb_fast_gph_left_click( void *args )
{
mgraph *o = (mgraph*)args;
fast_mgraph *p = (fast_mgraph*)o->parent();

int ii = o->graph_id;

//int sel_idx;
//o->get_selected_idx( 0, sel_idx );
//o->set_selected_sample( 1, sel_idx, 0 );



if( p->left_click_cb_p ) p->left_click_cb_p( args, p->left_click_cb_args );				//call user callback if set


int samp_idx;
int trc = o->get_selected_trace_and_sample_indexes( samp_idx );
if( trc != -1 )
	{
	p->last_sel_trc = trc;
	p->last_sel1_idx[ ii ] = p->last_sel0_idx[ ii ];					//v1.22
	p->last_sel1_x[ ii ] = p->last_sel0_x[ ii ];
	p->last_sel1_y[ ii ] = p->last_sel0_y[ ii ];
	o->deselect_all_samples(0);
	o->set_selected_sample( trc, samp_idx, 0 );

	p->last_sel0_idx[ ii ] = samp_idx;
	o->get_selected_value( trc, p->last_sel0_x[ ii ], p->last_sel0_y[ ii ] );

//printf("trc: %d\n",trc);
	}
else{
//	p->last_sel_trc = -1;												//v1.22
	}

int my;
o->get_mouse_pixel_position_on_background( p->drag_start_mousex, my );					//get values for dragging
o->get_posx( 0, p->drag_start_posx );

}









void cb_fast_gph_middle_click( void *args )
{
mgraph *o = (mgraph*)args;
fast_mgraph *p = (fast_mgraph*)o->parent();

int ii = o->graph_id;

//int sel_idx;
//o->get_selected_idx( 0, sel_idx );
//o->set_selected_sample( 1, sel_idx, 0 );



if( p->middle_click_cb_p ) p->middle_click_cb_p( args, p->middle_click_cb_args );			//call user callback if set


int samp_idx;
int trc = o->get_selected_trace_and_sample_indexes( samp_idx );
if( trc != -1 )
	{
	p->last_sel_trc = trc;
	p->last_sel1_idx[ ii ] = p->last_sel0_idx[ ii ];					//v1.22
	p->last_sel1_x[ ii ] = p->last_sel0_x[ ii ];
	p->last_sel1_y[ ii ] = p->last_sel0_y[ ii ];
	o->deselect_all_samples(0);
	o->set_selected_sample( trc, samp_idx, 0 );

	p->last_sel0_idx[ ii ] = samp_idx;
	o->get_selected_value( trc, p->last_sel0_x[ ii ], p->last_sel0_y[ ii ] );
	}
else{
	p->last_sel_trc = -1;
	}


int my;
o->get_mouse_pixel_position_on_background( p->drag_start_mousex, my );					//get values for dragging
o->get_posx( 0, p->drag_start_posx );

}














void cb_fast_gph_right_click( void *args )
{
mgraph *o = (mgraph*)args;
fast_mgraph *p = (fast_mgraph*)o->parent();

int ii = o->graph_id;

//int sel_idx;
//o->get_selected_idx( 0, sel_idx );
//o->set_selected_sample( 1, sel_idx, 0 );



if( p->right_click_cb_p ) p->right_click_cb_p( args, p->right_click_cb_args );			//call user callback if set


int samp_idx;
int trc = o->get_selected_trace_and_sample_indexes( samp_idx );
if( trc != -1 )
	{
	p->last_sel_trc = trc;
	p->last_sel1_idx[ ii ] = p->last_sel0_idx[ ii ];					//v1.22
	p->last_sel1_x[ ii ] = p->last_sel0_x[ ii ];
	p->last_sel1_y[ ii ] = p->last_sel0_y[ ii ];
	o->deselect_all_samples(0);
	o->set_selected_sample( trc, samp_idx, 0 );

	p->last_sel0_idx[ ii ] = samp_idx;
	o->get_selected_value( trc, p->last_sel0_x[ ii ], p->last_sel0_y[ ii ] );
	}
else{
	p->last_sel_trc = -1;
	}

int my;
o->get_mouse_pixel_position_on_background( p->drag_start_mousex, my );					//get values for dragging
o->get_posx( 0, p->drag_start_posx );

}



	




//adj params so graph trace is visible
void fast_mgraph::fit_plot( int grph_idx )
{

int ii = 0;

if( grph_idx >= 0 )		//multiple graphs?
	{
	ii = grph_idx;
	}


	int samp_idx;
	int sel_trc = gph[ii]->get_selected_trace_and_sample_indexes( samp_idx );
	
	int iw, ih;
	gph[ii]->get_trace_vis_dimensions( 0, iw, ih );
	
	double xmin, xmax, ymin, ymax;
	gph[ii]->get_trace_min_max( 0, xmin, xmax, ymin, ymax );

	gph[ii]->set_scaley( 0, 1.0f, 1 );										//first remove any 'a' key scaling
	scale_trc_y[ii][0] = 1.0f;

	gph[ii]->set_posy( 0, shift_trc_y[ii][0], 1 );

	float dy = ymax - ymin;												//calc max 'y' range
	float middley = dy/2 + ymin;										//find mid point


	max_defl_y[ii] = dy/2.0f * 1.2f;									//set a max range just over actual max range

	plot_grph_internal( gph[ii]->graph_id );							//need to replot graph twice (using cached vectors) as 'p->max_defl_y[ii]' has been changed
	plot_grph_internal( gph[ii]->graph_id );							//need to replot graph twice (using cached vectors) as 'p->max_defl_y[ii]' has been changed


	float shfty = -middley/gph[ii]->trce[0].yunits_perpxl;					//make a pixel val offset to center trace in the middle of its 'y' range values
	shift_trc_y[ii][0] = shfty;									

//printf("cb_fast_gph_keydown() - shift_trc_y %f  max_defl_y %f   yunits_perpxl %f  scale_trc_y %f\n", p->shift_trc_y[ii][0], p->max_defl_y[ii], o->trce[0].yunits_perpxl, p->scale_trc_y[ii][ 0 ] );

//o->adj_posy_by( sel_trc, 40.0, 1 );

	plot_grph_internal( gph[ii]->graph_id );									//do it again: as found when 'multi_trace' is in use the 'ctl-m' adj was one mousewheel detent behind, doing this twice here hides this - to be FIXED
}







void cb_fast_gph_keydown( void*v, int int0 )
{
mgraph *o = (mgraph*)v;
fast_mgraph *p = (fast_mgraph*)o->parent();

int ii = o->graph_id;


if( int0 == 'a' ) p->keya[ ii ] = 1;
if( int0 == 'y' ) p->keyy[ ii ] = 1;
if( int0 == 'm' ) p->keym[ ii ] = 1;


if( int0 == 's' ) 														//select a sample if none sel
	{
	p->keys[ ii ] = 1;

	p->select_first_sample_on_trc0_if_nothing_is_selected();
	}


if( int0 == 'f' ) 
	{
	
	p->keyf[ ii ] = 1;

	if( p->multi_trace != 0 ) ii = 0;									//only one graph?

	p->fit_plot( ii );

/*
	int samp_idx;
	int sel_trc = o->get_selected_trace_and_sample_indexes( samp_idx );
	
	int iw, ih;
	o->get_trace_vis_dimensions( 0, iw, ih );
	
	double xmin, xmax, ymin, ymax;
	o->get_trace_min_max( 0, xmin, xmax, ymin, ymax );

	o->set_scaley( 0, 1.0f, 1 );										//first remove any 'a' key scaling
	p->scale_trc_y[ii][0] = 1.0f;

	o->set_posy( 0, p->shift_trc_y[ii][0], 1 );

	float dy = ymax - ymin;												//calc max 'y' range
	float middley = dy/2 + ymin;										//find mid point



//	double scly;
//	o->get_scaley( 0, scly );

	

	p->max_defl_y[ii] = dy/2.0f * 1.2f;									//set a max range just over actual max range

	p->plot_grph_internal( o->graph_id );									//need to replot graph twice (using cached vectors) as 'p->max_defl_y[ii]' has been changed
	p->plot_grph_internal( o->graph_id );									//need to replot graph twice (using cached vectors) as 'p->max_defl_y[ii]' has been changed


	float shfty = -middley/o->trce[0].yunits_perpxl;					//make a pixel val offset to center trace in the middle of its 'y' range values
	p->shift_trc_y[ii][0] = shfty;									

//printf("cb_fast_gph_keydown() - shift_trc_y %f  max_defl_y %f   yunits_perpxl %f  scale_trc_y %f\n", p->shift_trc_y[ii][0], p->max_defl_y[ii], o->trce[0].yunits_perpxl, p->scale_trc_y[ii][ 0 ] );

//o->adj_posy_by( sel_trc, 40.0, 1 );

	p->plot_grph_internal( o->graph_id );									//do it again: as found when 'multi_trace' is in use the 'ctl-m' adj was one mousewheel detent behind, doing this twice here hides this - to be FIXED
*/
	}



if( ( int0 == FL_Shift_L ) | ( int0 == FL_Shift_R ) )  p->keyshift[ ii ] = 1;


if( int0 == 'h' )
	{
	cb_fast_mgraph_help(0,0);
	}

if( int0 == 't' )
	{
	int samp_idx;
	int trc = o->get_selected_trace_and_sample_indexes( samp_idx );
	
	if( trc != -1 )
		{
//		int count;
//		o->get_sample_count_for_trc( trc, count );

		p->last_sel1_idx[ 0 ] = p->last_sel0_idx[ 0 ];					//NOTE: multi_trace only exists when there is a single graph, so using xxx[0] here as always refering to the first and only graph
		p->last_sel1_x[ 0 ] = p->last_sel0_x[ 0 ];
		p->last_sel1_y[ 0 ] = p->last_sel0_y[ 0 ];
		o->deselect_all_samples( 0 );
		
		int trc_new = trc + 1;
		if( trc_new >= p->multi_trace ) trc_new = 0;
		
		p->last_sel_trc = trc_new;
		
		o->set_selected_sample( trc_new, samp_idx, 0 );
		
		p->last_sel0_idx[ 0 ] = samp_idx;
		o->get_selected_value( trc_new, p->last_sel0_x[ 0 ], p->last_sel0_y[ 0 ] );
		
//		printf("trc: %d\n", p->multi_trace);
		}
	}

if( int0 == 'x' ) 
	{
//	p->scale_change_h = 1.0/p->scale_change_h;
//	o->zoom_h_all_traces( p->left_hov_idx, p->scale_change_h, 1 );
	
	o->set_scalex_all_traces( 1.0, 1 );

	o->center_on_sample( 0, p->left_hov_idx );
	o->center_on_sample( 1, p->left_hov_idx );
	o->center_on_sample( 2, p->left_hov_idx );
	o->center_on_sample( 3, p->left_hov_idx );
	
//	p->scale_x = 1.0;
	}

if( int0 == 'z' ) 
	{
	p->shift_trc_y[ii][0] = 0;
	p->shift_trc_y[ii][1] = 0;
	p->shift_trc_y[ii][2] = 0;
	p->shift_trc_y[ii][3] = 0;
	
	p->scale_trc_y[ii][ 0 ] = 1.0;
	p->scale_trc_y[ii][ 1 ] = 1.0;
	p->scale_trc_y[ii][ 2 ] = 1.0;
	p->scale_trc_y[ii][ 3 ] = 1.0;

	p->plot_grph_internal( ii );				//force a redraw using cached vectors
	}


int samp_idx;
int trc = o->get_selected_trace_and_sample_indexes( samp_idx );

if( trc == -1 )
	{
	p->select_first_sample_on_trc0_if_nothing_is_selected();			//v1.22
	}

trc = o->get_selected_trace_and_sample_indexes( samp_idx );

if( trc != -1 )
	{
	int count;
	o->get_sample_count_for_trc( trc, count );

	bool cursor_key = 0;
	if ( ( int0 == FL_Left ) | ( int0 == FL_Right ) | ( int0 == FL_Down ) | ( int0 == FL_Up ) | ( int0 == FL_Page_Down ) | ( int0 == FL_Page_Up ) | ( int0 == FL_Home ) | ( int0 == FL_End ) )
		{
		cursor_key = 1;
		p->last_sel1_idx[ ii ] = p->last_sel0_idx[ ii ];					//v1.22
		p->last_sel1_x[ ii ] = p->last_sel0_x[ ii ];
		p->last_sel1_y[ ii ] = p->last_sel0_y[ ii ];
		}


	if( int0 == FL_Left )
		{
		samp_idx -= 1;
		if( samp_idx < 0 ) samp_idx = 0;

//		o->set_selected_sample( trc, samp_idx, 0 );
		}

	if( int0 == FL_Right )
		{
		samp_idx += 1;
		if( samp_idx >= count ) samp_idx = count - 1;
		
//		o->set_selected_sample( trc, samp_idx , 0 );
		}


	if( int0 == FL_Down )
		{
		samp_idx -= 10;
		if( samp_idx < 0 ) samp_idx = 0;
//		o->set_selected_sample( trc, samp_idx, 0 );
		}

	if( int0 == FL_Up )
		{
		samp_idx += 10;
		if( samp_idx >= count ) samp_idx = count - 1;
//		o->set_selected_sample( trc, samp_idx, 0 );
		}


	if( int0 == FL_Page_Down )
		{
		samp_idx -= 100;
		if( samp_idx < 0 ) samp_idx = 0;
//		o->set_selected_sample( trc, samp_idx, 0 );
		}

	if( int0 == FL_Page_Up )
		{
		samp_idx += 100;
		if( samp_idx >= count ) samp_idx = count - 1;
//		o->set_selected_sample( trc, samp_idx , 0 );
		}

	if( int0 == FL_Home )
		{
		samp_idx -= 10000;
		if( samp_idx < 0 ) samp_idx = 0;
//		o->set_selected_sample( trc, samp_idx, 0 );
		}

	if( int0 == FL_End )
		{
		samp_idx += 10000;
		if( samp_idx >= count ) samp_idx = count - 1;
//		o->set_selected_sample( trc, samp_idx, 0 );
		}
	
	if( cursor_key )
		{
		p->last_sel0_idx[ ii ] = samp_idx;
		o->set_selected_sample( trc, samp_idx, 0 );
		o->get_selected_value( trc, p->last_sel0_x[ ii ], p->last_sel0_y[ ii ] );
		}
	}
p->update_fg_user_obj( ii );
}





void cb_fast_gph_keyup( void*v, int int0 )
{
mgraph *o = (mgraph*)v;
fast_mgraph *p = (fast_mgraph*)o->parent();

int ii = o->graph_id;

if( int0 == 'a' ) p->keya[ ii ] = 0;
if( int0 == 'f' ) p->keyf[ ii ] = 0;
if( int0 == 'y' ) p->keyy[ ii ] = 0;
if( int0 == 'm' ) p->keym[ ii ] = 0;
if( int0 == 's' ) p->keys[ ii ] = 0;

if( ( int0 == FL_Shift_L ) | ( int0 == FL_Shift_R ) )  p->keyshift[ ii ] = 0;
}
















//double gph0_scalex = 1;
//double gph0_scaley0 = 1;
//double gph0_posy0 = 0;

//max_defl_y

void cb_fast_gph_mousewheel( void*v, int int0 )
{
mgraph *o = (mgraph*)v;
fast_mgraph *p = (fast_mgraph*)o->parent();

int ii = o->graph_id;

if( p->multi_trace != 0 ) ii = 0;										//only one graph?

int samp_idx;
int sel_trc = o->get_selected_trace_and_sample_indexes( samp_idx );
bool zoom_in = 0;



if( p->keym[ ii ] )			//max deflection y
	{
	float factor = 1;
	
	factor = p->max_defl_y[ii]/10 ;										//make a suitable factor for adj that is dependent on cur max deflection level
	
	if( p->max_defl_y[ii] == 0.0 ) factor = 0.1;
	
	if( int0 > 0.0 ) p->max_defl_y[ii] += 1 * factor;
	if( int0 < 0.0 ) p->max_defl_y[ii] -= 1 * factor;
	
	if( p->max_defl_y[ii] < 0.0 ) p->max_defl_y[ii] = 0.0;				//0.0 reverts back to auto scaling
	
	goto done;
	}


if( p->keya[ ii ] )			//amplitude adj
	{
//printf("sel_trc: %d\n",sel_trc);
//	if( int0 > 0.0 ) o->adj_scaley_by( sel_trc, 1.2, 1 );
//	if( int0 < 0.0 ) o->adj_scaley_by( sel_trc, 1.0/1.2, 1 );


	p->select_first_sample_on_trc0_if_nothing_is_selected();

	if( sel_trc == 0 )			//do to all
		{
		double dd;
	//	o->get_scaley( 0, dd );

	//	p->scale_trc_y[ii][sel_trc] = dd;
		if( int0 > 0.0 ) o->adj_scaley_by( sel_trc, 1.2, 1 );
		if( int0 < 0.0 ) o->adj_scaley_by( sel_trc, 1.0/1.2, 1 );
		
		o->get_scaley( 0, dd );
		p->scale_trc_y[ii][0] = dd;
		}
	else{
		double dd = p->scale_trc_y[ii][ sel_trc ];
		if( int0 > 0.0 ) dd *= 1.2;
		if( int0 < 0.0 ) dd *= 1.0/1.2;
		
		p->scale_trc_y[ii][ sel_trc ] = dd;
		
		p->plot_grph_internal( ii );			//force a redraw using cached vectors
		}

//	o->
//	scale_trc_y[ii] = 
	goto done;
	}


if( p->keyy[ ii ] )			//y pos adj
	{
double dd;

	if( sel_trc == 0 )
		{
		if( int0 > 0.0 ) o->adj_posy_by( sel_trc, 10.0, 1 );		//in unison
		if( int0 < 0.0 ) o->adj_posy_by( sel_trc, -10.0, 1 );
		
		o->get_posy( sel_trc, dd );
		p->shift_trc_y[ii][sel_trc] = dd;
		}
	else{

		if( int0 > 0.0 ) o->adj_plot_offsy_by( sel_trc, 10, 1 );	//individual
		if( int0 < 0.0 ) o->adj_plot_offsy_by( sel_trc, -10, 1 );

		int iv;
		o->get_plot_offsy( sel_trc, iv );

		p->shift_trc_y[ii][ sel_trc ] = iv;
//printf(" shift_trc_y[] %f\n",  p->shift_trc_y[ sel_trc ] );
		}

	goto done;
	}


/*
//if( int0 > 0.0 ) { p->scale_change = 1.5; zoom_in = 1; }
//else p->scale_change = 1.5;
if( int0 > 0.0 ) zoom_in = 1;


//if( zoom_in ) o->zoom_h_all_traces(  p->left_hov_idx,  p->scale_change, 1 );
//else o->zoom_h_all_traces(  p->left_hov_idx, 1.0/p->scale_change, 1 );

if( zoom_in ) o->zoom_h_all_traces(  p->left_hov_idx,  1.5, 1 );
else o->zoom_h_all_traces(  p->left_hov_idx, 1.0/1.5, 1 );
*/





//bool zoom_in = 0;
if( int0 > 0.0 ) { p->scale_change_h = 1.5; zoom_in = 1; }
else p->scale_change_h = 1.5;


if( zoom_in )  o->zoom_h_all_traces( p->left_hov_idx, p->scale_change_h, 1 );
else  o->zoom_h_all_traces( p->left_hov_idx, 1.0/p->scale_change_h, 1 );




//o->trce[ 0 ].scalex = 1;///= 2;

//if( zoom_in ) o->zoom_h( 0, p->left_hov_idx, 1.5, 1 );
//else  o->zoom_h( 0, p->left_hov_idx, 1.0/1.5, 1 );

//o->trce[ 0 ].scalex = 1.5;
//o->set_left_edge_sample_idx( 0, 400 );

//o->trce[ 0 ].scalex = 2;///= 2;
//o->trce[ 0 ].posx = 0;
//o->redraw();
//o->trce[ 0 ].posx = -355;

//printf("cb_fast_gph_mousewheel() - posx %f\n",  o->trce[ 0 ].posx );

//printf("cb_fast_gph_mousewheel() - o->trce[ 0 ].scalex %f\n", o->trce[ 0 ].scalex );

//int left;

//o->get_left_edge_sample_idx( 0, left );
//o->set_left_edge_sample_idx( 0, left);
//printf("cb_fast_gph_mousewheel() - left %d\n", left );

//o->trce[ 0 ].posx = 300;
//o->trce[ 1 ].posx = 0;

//o->trce[ 0 ].plot_offsx = 0;
//o->trce[ 1 ].plot_offsx = 0;

//printf("minx %f\n", o->trce[ 0 ].minx );

if( p->center_sel_sample_on_zoom_change )
	{
//	o->center_on_sample( sel_trc, samp_idx );		//v1.16
	
//	o->trce[ 1 ].posx = o->trce[ 0 ].posx ;
//	o->center_on_sample( 1, samp_idx );		//v1.16
	}

//double ff;
//if( o->get_posx( 0, ff ) )
//	{
//	double scalex;
//	o->get_scalex( 0, scalex);
//	float offx = ( ff - p->x_axis_offs);
//	o->set_posx( 0, offx, 1 );
//	o->set_posx( 0, -offx + -o->trce[0].border_left, 1 );
//	}

done:
//p->update_fg_user_obj( o->graph_id );
p->plot_grph_internal( o->graph_id );									//need to replot graph (using cached vectors) as 'p->max_defl_y[ii]' may have been changed by mousewheel
p->plot_grph_internal( o->graph_id );									//do it again: as found when 'multi_trace' is in use the 'ctl-m' adj was one mousewheel detent behind, doing this twice here hides this - to be FIXED

return;
}
















void cb_fast_gph_mousemove( void*v, int int0, int int1, double dble0, double dble1 )
{
mgraph *o = (mgraph*)v;
fast_mgraph *p = (fast_mgraph*)o->parent();


int left, right;
if(  o->get_mousex_sample_boundary_idx( 0, left, right ) )
	{
	p->left_hov_idx = left;												//get sample the mouse is nearest to, used when zooming (see mousewheel cb)
//	printf("left: %d, %d\n", left, right);
	}



//int left, right;
if( o->get_mousex_sample_boundary_idx( 0, left, right ) )
	{
//printf( "left: %d\n", left );

//	printf( "left: %d, right: %d\n", left, right );

	int ll,rr;
	o->get_left_edge_sample_idx( 0, ll );
//	o->get_right_edge_sample_idx( 0, rr );
//	printf( "ll: %d, rr: %d\n", ll, 0 );

//	printf( "cb_fast_gph_mousemove() - left: %d, right: %d\n", left, right );

	p->left_hov_idx = left;												//get sample the mouse is nearest to, used when zooming (see mousewheel cb)
	}


if( o->left_button )													//dragging x-axis?
	{
	int mx, my;
	o->get_mouse_pixel_position_on_background( mx, my );
	int mouse_delta = mx - p->drag_start_mousex;

	if( fabs(mouse_delta) > 5) p->drag_started = 1;						//allow some dead band

	if( p->drag_started )
		{
		p->drag_started = 1;
		int wid, hei;
		o->get_trace_vis_dimensions( 0, wid, hei );

		int count;

		if( !o->get_sample_count_for_trc( 0, count ) );

		//if( count == 0 ) return 0;
		double scalex;
		o->get_scalex(0, scalex );

		float samples_vis = (count - 1) / scalex;
		float samp_pixel_ratio = (float)samples_vis / wid;						//how many samples visible

		float slip = (float)mouse_delta / wid;
		float slip2 = samples_vis * slip;

		float posx_shift = (float)slip2 / count * wid * scalex;

		o->set_posx( 0, p->drag_start_posx + posx_shift, 1 );					//drag traces
//o->set_posx( 0, 10, 1 );
		
		for( int i = 1; i < p->multi_trace; i++ )
			{
			o->set_posx( i, p->drag_start_posx + posx_shift, 1 );					//drag traces
			}
		}
	}


//double posy;
//p->gph[ 0 ]->get_posy( 0, posy );

//printf("posy: %f\n", posy );
//int wid, hei;
//o->get_trace_vis_dimensions( 0, wid, hei );

//printf("vis x: %d %d\n", wid, hei );
//printf("clip_wdg_maxx: %d clip_wdg_maxy: %d\n", o->clip_maxx, o->clip_maxy);

p->update_fg_user_obj( o->graph_id );
//o->redraw();
}







/*

void cb_fast_gph_mousemove( void*v, int int0, int int1, double dble0, double dble1 )
{
mgraph *o = (mgraph*)v;
fast_mgraph *p = (fast_mgraph*)o->parent();


int left, right;
if( o->get_mousex_sample_boundary_idx( 0, left, right ) )
	{
//printf( "left: %d\n", left );

//	printf( "left: %d, right: %d\n", left, right );

	int ll,rr;
	o->get_left_edge_sample_idx( 0, ll );
//	o->get_right_edge_sample_idx( 0, rr );
//	printf( "ll: %d, rr: %d\n", ll, 0 );

//	printf( "cb_fast_gph_mousemove() - left: %d, right: %d\n", left, right );

	p->left_hov_idx = left;												//get sample the mouse is nearest to, used when zooming (see mousewheel cb)
	}


if( o->left_button )													//dragging x-axis?
	{
	int mx, my;
	o->get_mouse_pixel_position_on_background( mx, my );
	int mouse_delta = mx - p->drag_start_mousex;

	if( fabs(mouse_delta) > 5) p->drag_started = 1;						//allow some dead band

	if( p->drag_started )
		{
		p->drag_started = 1;
		int wid, hei;
		o->get_trace_vis_dimensions( 0, wid, hei );

		int count;

		if( !o->get_sample_count_for_trc( 0, count ) );

		//if( count == 0 ) return 0;
		double scalex;
		o->get_scalex(0, scalex );

		float samples_vis = (count - 1) / scalex;
		float samp_pixel_ratio = (float)samples_vis / wid;						//how many samples visible

		float slip = (float)mouse_delta / wid;
		float slip2 = samples_vis * slip;

		float posx_shift = (float)slip2 / count * wid * scalex;

		o->set_posx( 0, p->drag_start_posx + posx_shift, 1 );					//drag traces
		
		for( int i = 1; i < p->multi_trace; i++ )
			{
			o->set_posx( i, p->drag_start_posx + posx_shift, 1 );					//drag traces
			}
		}
	}


double posy;
p->gph[ 0 ]->get_posy( 0, posy );

//printf("posy: %f\n", posy );
//int wid, hei;
//o->get_trace_vis_dimensions( 0, wid, hei );

//printf("vis x: %d %d\n", wid, hei );
//printf("clip_wdg_maxx: %d clip_wdg_maxy: %d\n", o->clip_maxx, o->clip_maxy);

p->update_fg_user_obj( o->graph_id );
//o->redraw();
}

*/






void fast_mgraph::scale_y( int gph_idx, double trc1, double trc2, double trc3, double trc4 )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

scale_trc_y[gph_idx][ 0 ] = trc1;
scale_trc_y[gph_idx][ 1 ] = trc2;
scale_trc_y[gph_idx][ 2 ] = trc3;
scale_trc_y[gph_idx][ 3 ] = trc4;
}









//this is the value that would be displayed at graph's 'y' extremes,
//e.g. a sinewave of amplitude 1, would have a +/- 0.5 max deflection, so 'y_max_delf' should
//be set to 0.5 to make full use of graph's display area for the y axis
void fast_mgraph::set_ymax_deflection_value( int gph_idx, double y_max_delf )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

max_defl_y[gph_idx] = y_max_delf;
}






void fast_mgraph::shift_y( int gph_idx, double trc1, double trc2, double trc3, double trc4 )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

shift_trc_y[gph_idx][ 0 ] = trc1;
shift_trc_y[gph_idx][ 1 ] = trc2;
shift_trc_y[gph_idx][ 2 ] = trc3;
shift_trc_y[gph_idx][ 3 ] = trc4;
}


void fast_mgraph::font_type( int ii )
{
if( ii < 0 ) ii = 0;
fnt_type = ii;
}

void fast_mgraph::font_size( int ii )
{
if( ii < 3 ) ii = 3;
fnt_size = ii;
}



void fast_mgraph::set_sig_dig( int ii )
{
sig_dig = ii;
}






void fast_mgraph::set_xunits_perpxl( double trc1, double trc2, double trc3, double trc4 )
{
xunits_perpxl[ 0 ] = trc1;
xunits_perpxl[ 1 ] = trc2;
xunits_perpxl[ 2 ] = trc3;
xunits_perpxl[ 3 ] = trc4;
}



void fast_mgraph::set_yunits_perpxl( double trc1, double trc2, double trc3, double trc4 )
{
yunits_perpxl[ 0 ] = trc1;
yunits_perpxl[ 1 ] = trc2;
yunits_perpxl[ 2 ] = trc3;
yunits_perpxl[ 3 ] = trc4;
}



//SEE EXAMPLE CODE: 'test_fast_mgraph()'
fast_mgraph::fast_mgraph( int gph_cnt_in, int xx, int yy, int wid, int hei, const char *label ) : Fl_Double_Window( xx, yy, wid, hei, label )
{
string s1;

mnu = new Fl_Menu_Bar(0, 0, wid, 25);

mnu->menu( fast_mgraph_menuitems ) ;
mnu->textsize(12);

mnu->hide();

fnt_type = 4;
fnt_size = 9;
sig_dig = 4;

scale_change_h = 0;
multi_trace = 0;

gph_cnt = gph_cnt_in;

if( gph_cnt >= cn_fast_mgraph_cnt_max ) gph_cnt = cn_fast_mgraph_cnt_max - 1;
if( gph_cnt < 1 ) gph_cnt = 1;

reduction = 5;
gap = 3;
//printf( "gph_cnt: %d\n", gph_cnt);

wnd_cur_wid = wid;
wnd_cur_hei = hei;

left_click_cb_p = 0;
middle_click_cb_p = 0;
right_click_cb_p = 0;

center_sel_sample_on_zoom_change = 1;

b_bring_window_to_front = 0;

plot_updating_state = 1;
b_prev_plot_recording = 0;
prev_plot_show_choice = 0;


//init vars
pos_x = 0;
scale_x = 1;
last_sel_trc = 0;
for( int i = 0; i < cn_fast_mgraph_cnt_max; i++ )
	{
	last_sel0_idx[i] = 0;
	last_sel1_idx[i] = 0;
	last_sel0_x[i] = 0;
	last_sel0_y[i] = 0;
	last_sel1_x[i] = 0;
	last_sel1_y[i] = 0;



	keya[i] = 0;
	keyf[i] = 0;
	keyy[i] = 0;
	keym[i] = 0;
	keys[i] = 0;
	keyshift[i] = 0;
	sel_idx1[i] = -1;
	sel_idx2[i] = -1;

	scale_trc_y[i][0] = 1.0;
	scale_trc_y[i][1] = 1.0;
	scale_trc_y[i][2] = 1.0;
	scale_trc_y[i][3] = 1.0;

	shift_trc_y[i][0] = 0.0;
	shift_trc_y[i][1] = 0.0;
	shift_trc_y[i][2] = 0.0;
	shift_trc_y[i][3] = 0.0;
	
	xunits_perpxl[i] = -1;
	yunits_perpxl[i] = -1;
	
	max_defl_y[i] = 0.0;

	b_x_axis_values_derived[i] = 0;
	x_axis_values_derived_left_value[i] = 0.0;
	x_axis_values_derived_inc_value[i] = 1.0;
	b_x_axis_vector_supplied[i] = 0;									//set this to something, though its actually adj whenever a plot function is called

//	trc_label[i] = "";			//if user wants to label a trace using the trace's colour key,
	}

if( gph_cnt == 1 )										//one graph, multi trace?
	{
	col_hover_text_multi_1[0].r = 229;
	col_hover_text_multi_1[0].g = 142;
	col_hover_text_multi_1[0].b = 0;

	col_hover_text_multi_2[0].r = 229;
	col_hover_text_multi_2[0].g = 142;
	col_hover_text_multi_2[0].b = 0;

	col_hover_text_multi_3[0].r = 229;
	col_hover_text_multi_3[0].g = 142;
	col_hover_text_multi_3[0].b = 0;

	col_hover_text_multi_4[0].r = 229;
	col_hover_text_multi_4[0].g = 142;
	col_hover_text_multi_4[0].b = 0;

	set_col_axis1( 0, "grey" );

	set_obj_col_text1_rgb( 0, 80, 80, 200 );
	set_obj_col_text2_rgb( 0, 80, 80, 200 );
	set_obj_col_text3_rgb( 0, 80, 80, 200 );
	set_obj_col_text4_rgb( 0, 80, 80, 200 );

	set_col_trc1( 0, "drkgreen" );
	set_col_trc2( 0, "drkred" );
	set_col_trc3( 0, "drkpink" );
	set_col_trc4( 0, "orange" );

	set_trc_label1( 0, "trc0" );
	set_trc_label2( 0, "trc1" );
	set_trc_label3( 0, "trc2" );
	set_trc_label4( 0, "trc3" );

	xtick_marks[0] = 16;
	ytick_marks[0] = 20;
	}

//size graphs, see also FL_MOVE, where window resizing is detected and graphs are resized
int dy = hei / gph_cnt;
for( int i = 0; i < gph_cnt; i++ )
	{
	int between_gap = 0;
	if( ( i >= 1 )&( i <= (gph_cnt - 1) ) ) between_gap = 1 * gap;
	int final_reduction = 0;
	if( i == (gph_cnt - 1) ) final_reduction = 2 * reduction;
	gph[i] = (mgraph*)new mgraph( reduction, reduction + i*dy + i*between_gap, wid - 2 * reduction, dy - 0 - final_reduction, "" );

	gph[i]->graph_id = i;

	if(i == 0) graph0_hei = gph[i]->h();					//for menu display shrink/restore

	gph[i]->b_invert_wheel = 0;


	gph[i]->htime_lissajous = 0;				//0 means no lissajous

	gph[i]->background.r = 255;
	gph[i]->background.g = 255;
	gph[i]->background.b = 255;
	gph[i]->border_col.r = 0;
	gph[i]->border_col.g = 0;
	gph[i]->border_col.b = 0;



	if( gph_cnt >= 2 )							//if multi graph, set suitable details
		{
		set_col_axis1( i, "grey" );
		set_col_axis2( i, "grey" );
		set_col_axis3( i, "grey" );
		set_col_axis4( i, "grey" );

		col_hover_text_multi_1[i].r = 229;
		col_hover_text_multi_1[i].g = 142;
		col_hover_text_multi_1[i].b = 0;

		xtick_marks[i] = 16;
		ytick_marks[i] = 20;


	/*
		col_hover_text_multi_2[i].r = 229;
		col_hover_text_multi_2[i].g = 142;
		col_hover_text_multi_2[i].b = 0;

		col_hover_text_multi_3[i].r = 229;
		col_hover_text_multi_3[i].g = 142;
		col_hover_text_multi_3[i].b = 0;

		col_hover_text_multi_4[i].r = 229;
		col_hover_text_multi_4[i].g = 142;
		col_hover_text_multi_4[i].b = 0;
	*/
		set_col_trc1( i, "drkred" );

		strpf( s1, "graph %d", i );
		set_trc_label1( i, s1 );
		}

	int border = reduction;
	gph[i]->bkgd_border_left = border;
	gph[i]->bkgd_border_top = border;
	gph[i]->bkgd_border_right = border;
	gph[i]->bkgd_border_bottom = border;

	gph[i]->graticule_border_left = 0;
	gph[i]->graticule_border_top = 0;
	gph[i]->graticule_border_right = 0;
	gph[i]->graticule_border_bottom = 0;

	gph[i]->cro_graticle = 0;
	gph[i]->graticle_count_x = 6;
	gph[i]->grat_pixels_x = 25;

	gph[i]->graticle_count_y = 4;
	gph[i]->grat_pixels_y = 25;

	gph[i]->show_cursors_along_y = 0;
	gph[i]->show_cursors_along_x = 0;
	gph[i]->cursor_along_y0 = 10;         		//cursors along y-axis (i.e. measuring time)
	gph[i]->cursor_along_y1 = 20;

	gph[i]->cursor_along_x0 = 0.5;        	 	//cursors along x-axis (i.e. measuring amplitude)
	gph[i]->cursor_along_x1 = -0.3;
	}


use_logx = use_logy = 0;									//v1.13
disp_vals_undo_logx = disp_vals_undo_logy = 0;				//v1.13, useful to keep a log trace's displayed values linear (e.g. for freq values)
disp_vals_multiplier_x = disp_vals_multiplier_y = 1;		//v1.13, e.g: for logarithmic scale: 20*log10, set to 20

sample_rect_hints_double_click = 1;
sample_rect_hints = 0;					//if set the individual sample points are shown with a rect (if they don't cause congestion see below)
sample_rect_hints_col.r = 150;
sample_rect_hints_col.g = 100;
sample_rect_hints_col.b = 80;
sample_rect_hints_distancex = 12;							//v1.13, helps stop congestion
sample_rect_hints_distancey = 0;


Fl_Box *bx = new Fl_Box( 50, 50, wid - 50, hei - 50 );
resizable( bx );
bx->hide();
end();
show();

}




fast_mgraph::~fast_mgraph()
{
for( int i = 0; i < gph_cnt; i++ ) delete gph[i];
}



bool fast_mgraph::set_left_click_cb( void (*p_cb)( void* ), void *args_in )
{
//if ( which_grp >= gph_cnt ) return 0;

//if ( which_grp < 0 ) return 0;

left_click_cb_p = p_cb;
left_click_cb_args = args_in;

return 1;
}



bool fast_mgraph::set_middle_click_cb( void (*p_cb)( void* ), void *args_in )
{
//if ( which_grp >= gph_cnt ) return 0;

//if ( which_grp < 0 ) return 0;

middle_click_cb_p = p_cb;
middle_click_cb_args = args_in;

return 1;
}




bool fast_mgraph::set_right_click_cb( void (*p_cb)( void* ), void *args_in )
{
//if ( which_grp >= gph_cnt ) return 0;

//if ( which_grp < 0 ) return 0;

right_click_cb_p = p_cb;
right_click_cb_args = args_in;

return 1;
}





int fast_mgraph::handle( int e )
{
bool need_redraw = 0;
bool dont_pass_on = 0;



if ( e == FL_ENTER )
	{
//	mnu->show();
	}

		

if ( e == FL_LEAVE )
	{
	mnu->hide();
	gph[ 0 ]->resize( gph[0]->x(), reduction, gph[0]->w(), graph0_hei );
	}

if ( e == FL_MOVE )
	{
	int mousex = Fl::event_x();
	int mousey = Fl::event_y();

	if( ( wnd_cur_wid != w() ) | ( wnd_cur_hei != h() ) ) 					//window resized?
		{
		wnd_cur_wid = w();
		wnd_cur_hei = h();
		
		//resize graphs
		int dy = wnd_cur_hei / gph_cnt;
		for( int i = 0; i < gph_cnt; i++ )
			{
			gph[ i ]->hide();
			int between_gap = 0;
			if( ( i >= 1 )&( i <= (gph_cnt - 1) ) ) between_gap = 1 * gap;
			int final_reduction = 0;
			if( i == (gph_cnt - 1) ) final_reduction = 2 * reduction;

			gph[i]->resize( reduction, reduction + i*dy + i*between_gap, wnd_cur_wid - 2 * reduction, dy - 0 - final_reduction );

			if(i == 0) graph0_hei = gph[i]->h();					//for menu display shrink/restore
//			update_fg_user_obj( i );
			gph[ i ]->show();
			}
		}

	if( mousey < 10 ) 
		{
		gph[ 0 ]->hide();
		gph[ 0 ]->resize( gph[0]->x(), 25 + reduction, gph[0]->w(), graph0_hei - 25 );		//shrink the first(top) graph
		gph[ 0 ]->show();
		mnu->show();
		}

	if( mousey > 20 )
		{
		mnu->hide();
		gph[ 0 ]->resize( gph[0]->x(), reduction, gph[0]->w(), graph0_hei );
		}
	}

if ( need_redraw ) redraw();

if( dont_pass_on ) return 1;

return Fl_Double_Window::handle(e);
}










void fast_mgraph::update_fg_user_obj( int graph_idx )
{
bool vb = 0;
string s1, s2, s3, s4;
mystr m1;
//bool mgraph::get_scaley( int trc, double &val )

int ii = graph_idx;

//printf("update_fg_user_obj() - graph_idx: %d\n", graph_idx );
//user draw objs

if( gph[ ii ]->trce.size() == 0 ) return;



gph[ ii ]->vdrwobj.clear();

//if( ii == 0 ) return;

int bkg_wid, bkg_hei;
gph[ ii ]->get_background_dimensions( bkg_wid, bkg_hei );

double scaley;
gph[ ii ]->get_scaley( 0, scaley );

double scalex;
gph[ ii ]->get_scalex( 0, scalex );

double posy;
gph[ ii ]->get_posy( 0, posy );


st_mgraph_draw_obj_tag mdo;

mdo.type = (en_mgraph_draw_obj_type)en_dobt_text;

mdo.visible = 1;
mdo.draw_ordering = 2;				//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

mdo.use_pos_x = -1;                 //if not set to -1: then use this trace id to get a specific x position
mdo.use_scale_x = -1;               //if not set to -1: then use this trace id to get a specific x scale

mdo.use_pos_y = -1;                 //if not set to -1: then use this trace id to get a specific y position
mdo.use_scale_y = -1;               //if not set to -1: then use this trace id to get a specific y scale

mdo.clip_left = 0;
mdo.clip_right = 0;
mdo.clip_top = 0;
mdo.clip_bottom = 0;

//mdo.wid = 10;
//mdo.hei = 10;

																						//this will be used to adj y tick values, so they follow trace's y position



//mouse hov values
mdo.type = en_dobt_text;







double minx, maxx;
double miny, maxy;

gph[ii]->get_trace_min_max( 0, minx, maxx, miny, maxy );

float extreme_y;

if( fabs( maxy ) >= fabs( miny ) ) extreme_y = maxy;
else extreme_y = fabs( miny );

if( max_defl_y[ ii ] == 0.0 ) max_defl_y[ ii ] = extreme_y;				//this happens only on the initial (first) draw





//printf( "update_fg_user_obj() max_defl_y %f\n", max_defl_y[ii] );




double valx, valy;
//gph[ ii ]->get_mouse_position_relative_to_trc_new( 0, valx, valy );		//this does not work for 'fast_graph' due to various offsets, such as: 'shift_trc_y[][]',  'x_axis_offs' and 'posx', see below for fixes

//double posxxxxx;
//gph[ii]->get_posx(0, posxxxxx );


int mx, my;
//gph[ ii ]->get_mouse( mx, my );

gph[ ii ]->get_mouse_pixel_position_on_background( mx, my );

double minxx, maxxx, minyy, maxyy;
gph[ ii ]->get_trace_min_max( 0, minxx, maxxx, minyy, maxyy );


float scle_trc_y = scale_trc_y[ ii ][0];


//printf( "update_fg_user_obj() mousex %d %d\n", mx, my );

//printf( "update_fg_user_obj() minxx %f %f   minyy %f %f\n", minxx, maxxx,  minyy, maxyy );






{
//----	v1.22 calc 'y' hover value

//int vis_wid, vis_hei;	
//gph[ii]->get_trace_vis_dimensions( 0, vis_wid, vis_hei );
//float shfty = (maxyy - minyy)/2  * ((float)shift_trc_y[ ii ][0] / ((float)vis_hei/2 ));	//v1.22, make a value from 'shift_trc_y[][]' (which is in pixels), so that it represents how much a trace has be shifted in y sample values,




int offx, offy;
gph[ ii ]->get_background_offsxy( offx, offy );


int wid, hei;
gph[ ii ]->get_background_dimensions( wid, hei );


//float ctrly = (float)(my ) / (gph[ ii ]->h() - offy) ;
float ctrly = (float)(my ) / (hei) ;

//double dposy;
//gph[ ii ]->get_posx_relative_to_trace( 0, dposy );

float yunits_pp = yunits_perpxl[ii];

float shfty = shift_trc_y[ ii ][0] * yunits_pp / scle_trc_y;

//float vis_maxy = (gph[ ii ]->h() - offy) * yunits_pp / scle_trc_y;
float vis_maxy = (hei) * yunits_pp / scle_trc_y;


float vis_miny = -vis_maxy / 2.0f;

valy = vis_miny + vis_maxy * (1.0f - ctrly) - shfty;

//printf( "update_fg_user_obj() minyy %f %f   vis_miny %f shfty %f\n", minyy, maxyy, vis_miny, shfty );

//printf( "update_fg_user_obj()  offy %d h() %d, my %d  ctrly %f  yunits_pp %f  scle_trc_y %f vis_maxy %f valy %f\n", offy,  gph[ ii ]->h(), my, ctrly, yunits_pp, scle_trc_y, vis_maxy, valy  );

//printf( "update_fg_user_obj()  valy %f  shfty %f    valy %f\n", valy, shfty, valy - shfty );
//----
}







{
//----	v1.22 calc 'x' hover value

//int mx, my;
//gph[ ii ]->get_mouse( mx, my );

//int offx, offy;
//gph[ ii ]->get_trace_offsxy( 0, offx, offy );




//double dposx;
//gph[ ii ]->get_posx_relative_to_trace( 0, dposx );

double dmx, dmy;
gph[ ii ]->get_mouse_position_relative_to_trc( 0, dmx, dmy );

//printf( "update_fg_user_obj()  dmy %f %f\n", dmx, dmy );

//printf( "update_fg_user_obj()  minxx %f %f\n", minxx, maxxx );
//printf( "update_fg_user_obj()  offx %d\n", offx );
//printf( "update_fg_user_obj()  posx %f\n", dposx );

//float ctrlx = (float)(mx - offx ) / (gph[ ii ]->w() - offx) ;
//valx = (maxxx-minxx) * ctrlx + minxx;

//valx -= dposx;

valx = dmx;

//printf( "update_fg_user_obj()  offx %d mx %d  wid %d  ctrlx %f valx %f  dposx %f\n", offx, mx, gph[ ii ]->w(), ctrlx, valx, dposx );
//----
}



if( b_x_axis_values_derived[ii] )									//does user want to display specific x axis values ? (only works for plots without a supplied x vector)
	{
	if( !b_x_axis_vector_supplied[ii] )
		{

		valx = x_axis_values_derived_left_value[ii] + ( x_axis_values_derived_inc_value[ii] * valx );
		}
	}



if( ( use_logx ) && ( disp_vals_undo_logx ) ) valx = pow(10, valx);	//v1.13
valx *= disp_vals_multiplier_x;

if( ( use_logy ) && ( disp_vals_undo_logy ) ) valy = pow(10, valy);	//v1.13
valy *= disp_vals_multiplier_y;

string snum, sunits, scombined, sappend_num, sappend_units;
sappend_units = "";
sappend_num = "";

m1.make_engineering_str_exp( snum, sunits, scombined, 2, valx, sappend_num, sappend_units, 0 );
s1 = scombined;

m1.make_engineering_str_exp( snum, sunits, scombined, 2, valy, sappend_num, sappend_units, 0 );

s2 = scombined;
strpf( s1, "hov x:%s  y:%s", s1.c_str(), s2.c_str() );

double dposx;
gph[ ii ]->get_posx_relative_to_trace( 0, dposx );

int iposx;
gph[ ii ]->get_plot_offsy( 0, iposx );


mdo.x1 = 70;
mdo.y1 = 9;
mdo.x2;
mdo.y2;
mdo.arc1 = 0;
mdo.arc2 = 0;
mdo.stext = s1;
mdo.r = col_hover_text[ii].r;
mdo.g = col_hover_text[ii].g;
mdo.b = col_hover_text[ii].b;
mdo.font = fnt_type;
mdo.font_size = fnt_size;
mdo.justify = en_tj_none;
mdo.line_style = (en_mgraph_line_style)FL_SOLID;
mdo.line_thick = 1;



gph[ii]->vdrwobj.push_back( mdo );


//sel sample values
int sel_idx= 0;
int trc = gph[ ii ]->get_selected_trace_and_sample_indexes( sel_idx );
//printf("fast_mgraph::update_fg_user_obj() - ii %d  trc %d  sel_idx %d\n", ii, trc, sel_idx );
if( trc != -1 )
	{
	int col_idx = ii;
	if( multi_trace != 0 ) col_idx = trc;
	
	gph[ ii ]->get_selected_value( trc, valx, valy );


	if( b_x_axis_values_derived[ii] )									//does user want to display specific x axis values ? (only works for plots without a supplied x vector)
		{
		if( !b_x_axis_vector_supplied[ii] )
			{
			int idx = last_sel0_idx[ ii ];								//v1.22

			valx = x_axis_values_derived_left_value[ii] + ( x_axis_values_derived_inc_value[ii] * idx );
			}
		}



	if( ( use_logx ) && ( disp_vals_undo_logx ) ) valx = pow(10, valx);	//v1.13
	valx *= disp_vals_multiplier_x;

	if( ( use_logy ) && ( disp_vals_undo_logy ) ) valy = pow(10, valy);	//v1.13
	valy *= disp_vals_multiplier_y;

	m1.make_engineering_str_exp( snum, sunits, scombined, sig_dig, valx, sappend_num, sappend_units, 0 );
	s1 = scombined;

	m1.make_engineering_str_exp( snum, sunits, scombined, sig_dig, valy, sappend_num, sappend_units, 0 );
	s2 = scombined;

	int sel_idx;
	gph[ii]->get_selected_idx( trc, sel_idx );





	strpf( s1, "sel%d[%d]  x:%s  y:%s", trc, sel_idx, s1.c_str(), s2.c_str() );
	mdo.x1 = 220;
	mdo.y1 = 9;
	mdo.x2;
	mdo.y2;
	mdo.arc1 = 0;
	mdo.arc2 = 0;
	mdo.stext = s1;
	mdo.r = color_trc[col_idx].r;
	mdo.g = color_trc[col_idx].g;
	mdo.b = color_trc[col_idx].b;
	mdo.font = fnt_type;
	mdo.font_size = fnt_size;
	mdo.justify = en_tj_none;
	mdo.line_style = (en_mgraph_line_style)FL_SOLID;
	mdo.line_thick = 1;

	gph[ii]->vdrwobj.push_back( mdo );


	//deltas sample values
//	double dx = last_sel0_x[0] - last_sel1_x[0];
//	double dy = last_sel0_y[0] - last_sel1_y[0];
	
	double dx0 = last_sel0_x[ii];									//v1.22
	double dx1 = last_sel1_x[ii];


	if( b_x_axis_values_derived[ii] )									//does user want to display specific x axis values ? (only works for plots without a supplied x vector)
		{
		if( !b_x_axis_vector_supplied[ii] )
			{
			int idx0 = last_sel0_idx[ ii ];								//v1.22
			int idx1 = last_sel1_idx[ ii ];

			dx0 = x_axis_values_derived_left_value[ii] + ( x_axis_values_derived_inc_value[ii] * idx0 );
			dx1 = x_axis_values_derived_left_value[ii] + ( x_axis_values_derived_inc_value[ii] * idx1 );
			}
		}



	if( ( use_logx ) && ( disp_vals_undo_logx ) )  	//v1.13
		{
		dx0 = pow(10, dx0);
		dx1 = pow(10, dx1);
		}

	dx0 *= disp_vals_multiplier_x;
	dx1 *= disp_vals_multiplier_x;

	double dx = dx0 - dx1;


	double dy0 = last_sel0_y[0];										//v1.22
	double dy1 = last_sel1_y[0];

	if( ( use_logy ) && ( disp_vals_undo_logy ) ) 						//v1.13
		{
		dy0 = pow(10, dy0);
		dy1 = pow(10, dy1);
		}

	dy0 *= disp_vals_multiplier_y;
	dy1 *= disp_vals_multiplier_y;

	double dy = dy0 - dy1;

	
	m1.make_engineering_str_exp( snum, sunits, scombined, sig_dig, dx, sappend_num, sappend_units, 0 );
	s2 = scombined;

	m1.make_engineering_str_exp( snum, sunits, scombined, sig_dig, dy, sappend_num, sappend_units, 0 );
	s3 = scombined;

	int dddx = 0;
	if( dx != 0.0 )  dddx = 1.0/dx;			//avoid div by zero
	m1.make_engineering_str_exp( snum, sunits, scombined, sig_dig, dddx, sappend_num, sappend_units, 0 );
	s4 = scombined;

	int delta_cnt = last_sel0_idx[ii] - last_sel1_idx[ii];				//v1.22
	strpf( s1, "dcnt:%d  dx:%s  dy:%s (Freq:%s Hz)", delta_cnt, s2.c_str(), s3.c_str(), s4.c_str() );
//	strpf( s1, "dx:%.2f dy:%.2f (Freq:%.2f Hz)", dx, dy, 1.0/dx );
	mdo.x1 = 460;
	mdo.y1 = 9;
	mdo.stext = s1;
	mdo.r = col_hover_text[ii].r;
	mdo.g = col_hover_text[ii].g;
	mdo.b = col_hover_text[ii].b;
	mdo.line_style = (en_mgraph_line_style)FL_SOLID;

	gph[ii]->vdrwobj.push_back( mdo );
	}




//draw trace labels if any
int text_line_gap = fnt_size;
int txty = 9;

//if( multi_trace == 0 ) label_cnt = gph_cnt;

if( multi_trace != 0 )							//v1.11
	{
	int label_cnt = multi_trace;
	
	for( int i = 0; i < label_cnt; i++ )		//for one graph multiple traces, cycle all traces
		{
//printf( "update_fg_user_obj() id0 %d label i %d   '%s'\n", id0, i, trc_label[i].c_str() );

		if( trc_label[i].length() != 0 )
			{

			mdo.x1 = w() - 100;
			mdo.y1 = txty;
			mdo.x2;
			mdo.y2;
			mdo.arc1 = 0;
			mdo.arc2 = 0;
			mdo.stext = trc_label[i];
			mdo.r = color_trc[i].r;
			mdo.g = color_trc[i].g;
			mdo.b = color_trc[i].b;
			mdo.font = fnt_type;
			mdo.font_size = fnt_size;
			mdo.justify = en_tj_none;
			mdo.line_style = (en_mgraph_line_style)FL_SOLID;
			mdo.line_thick = 1;
			
			txty += fnt_size;
			gph[ii]->vdrwobj.push_back( mdo );
			}
		}
	}
else{								//for muliple graphs, one trace, just update req graph
	int idx = ii;
	
	if( trc_label[idx].length() != 0 )
		{
		mdo.x1 = w() - 100;
		mdo.y1 = txty;
		mdo.x2;
		mdo.y2;
		mdo.arc1 = 0;
		mdo.arc2 = 0;
		mdo.stext = trc_label[idx];
		mdo.r = color_trc[idx].r;
		mdo.g = color_trc[idx].g;
		mdo.b = color_trc[idx].b;
		mdo.font = fnt_type;
		mdo.font_size = fnt_size;
		mdo.justify = en_tj_none;
		mdo.line_style = (en_mgraph_line_style)FL_SOLID;
		mdo.line_thick = 1;
		
		txty += fnt_size;
		gph[ii]->vdrwobj.push_back( mdo );
		}
	}



/*
double minx, maxx;
double miny, maxy;

gph[ii]->get_trace_min_max( 0, minx, maxx, miny, maxy );

float extreme_y;

if( fabs( maxy ) >= fabs( miny ) ) extreme_y = maxy;
else extreme_y = fabs( miny );

if( max_defl_y[ ii ] == 0.0 ) max_defl_y[ ii ] = extreme_y;				//this happens only on the initial (first) draw
*/





//------ find a suitable ymax to use and round it up or down --------
if(vb)printf( "update_fg_user_obj()0  extreme_y %f   max_defl_y %f  maxy %f   miny %f\n", extreme_y, max_defl_y[ ii ], maxy, miny );

int hh = gph[ii]->h();
float fmaxy = 0.0f;
float fminy = -0.0f;


if( max_defl_y[ ii ] != 0.0 )											//has a 'max_defl_y[]'
	{
	yunits_perpxl[ii] = max_defl_y[ ii ] / (hh/2 - 20);					//calc suitable 'yunits_perpxl[]'
		
	fmaxy = fabs( max_defl_y[ ii ] );
	}
//else{
//	fmaxy = maxy * 0.95;
//	}
if(vb)printf( "update_fg_user_obj()1  max_defl_y %f  fmaxy %f  yunits_perpxl[%d] %f\n", max_defl_y[ ii ], fmaxy, ii, yunits_perpxl[ii] );



bool done_scaley = 0;

float factor = 1.0f;
//printf( "update_fg_user_obj()2  max_defl_y %f  fmaxy %f\n", max_defl_y[ ii ], fmaxy );


float max_tmp = max_defl_y[ ii ];

if( max_defl_y[ ii ] != 0.0 )						//have a max delfection value ? (note this is always true as its set above)
	{
	max_tmp = max_defl_y[ ii ];
	}
//else{
//	max_tmp = maxy;
//	}

//scale 'max_tmp' to between 0.1 --> 1.0, by adj a suitable factor
while( max_tmp != 0.0 )
	{
	if( max_tmp < 0.1 ) 
		{
		max_tmp *= 10.0f;
		factor *= 10;
		continue;
		}

	if( max_tmp > 1.0 ) 
		{
		max_tmp /= 10.0f;
		factor /= 10;
		continue;
		}
	break;
	}

if(vb)printf( "update_fg_user_obj()3  max_defl_y %f  max_tmp %f  maxy %f  factor %f\n", max_defl_y[ ii ], max_tmp, fmaxy, factor );

float max_tmp2 = max_tmp*100;

int iv = nearbyint( max_tmp2 );			//remove less significant digits

max_tmp = iv/100.0f;

max_tmp /= factor;


fmaxy = max_tmp;	
fminy = -fmaxy;


maxy = fmaxy;
miny = fminy;

//----------------------------------------------------------------------




//x-axis
mdo.use_pos_x = -1;                 //if not set to -1: then use this trace id to get a specific y position
mdo.use_scale_x = -1;               //if not set to -1: then use this trace id to get a specific y scale

mdo.use_pos_y = -1;                 //if not set to -1: then use this trace id to get a specific y position
mdo.use_scale_y = 0;               	//if not set to -1: then use this trace id to get a specific y scale

mdo.type  = en_dobt_line;
mdo.draw_ordering = 2;				//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

mdo.x1 = x_axis_offs;

mdo.y1 = miny + (maxy - miny)/2;

mdo.x2 = gph[ii]->w();
mdo.y2 = mdo.y1;
mdo.r = col_obj_axis[ii].r;
mdo.g = col_obj_axis[ii].g;
mdo.b = col_obj_axis[ii].b;

gph[ii]->vdrwobj.push_back( mdo );








int dummy;

double posx;
gph[ii]->get_posx_relative_to_trace(0, posx );

int tick = xtick_marks[ii];
float dt = (maxx/scalex - minx/scalex ) / (float)tick;



int cnt = vtrc_valy1[ii].size();
float minx2 = x_axis_values_derived_left_value[ii];						//these are used when 'b_x_axis_values_derived[]' is set, and no 'x' vector has been supplied
float maxx2 = minx2 + cnt * x_axis_values_derived_inc_value[ii];
float dt2_tick = (maxx2/scalex - minx2/scalex ) / (float)tick;
float dt3 = (maxx2/scalex - minx2/scalex ) / cnt;

int left_idx;
gph[ii]->get_left_edge_sample_idx( 0, left_idx );
float left_valuex = left_idx * (dt3*scalex);




for( int i = 0; i < tick; i++ )
	{
	//x-axis ticks
	mdo.use_pos_x = 0;
	mdo.use_scale_x = 0;               	//if not set to -1: then use this trace id to get a specific y scale

	mdo.type  = en_dobt_line;
	mdo.draw_ordering = 2;				//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

	mdo.clip_left = x_axis_offs - 35;
	mdo.clip_right = 0;
	mdo.clip_top = 0;
	mdo.clip_bottom = 0;

	
	float fscly_tickx = scale_trc_y[ii][ 0 ];							//v1.22

	mdo.x1 = minx + i * dt - posx;										//v1.16
	mdo.y1 = miny + (maxy - miny)/2 + (maxy - miny) * 0.003/fscly_tickx;	//v1.22

	mdo.x2 = mdo.x1;
	mdo.y2 = miny + (maxy - miny)/2 - (maxy - miny) * 0.004/fscly_tickx;

//printf( "mdo.y2: %f %f\n", mdo.y1, mdo.y2);
	mdo.r = col_obj_axis[ii].r;
	mdo.g = col_obj_axis[ii].g;
	mdo.b = col_obj_axis[ii].b;

	gph[ii]->vdrwobj.push_back( mdo );

	//x-axis text vals
	string snum, sunits, scombined, sappend_num, sappend_units;
	sappend_units = "";
	sappend_num = "";
	
	double valx = mdo.x1;
	
	if( b_x_axis_values_derived[ii] )									//does user want to display specific x axis values ? (only works for plots without a supplied x vector)
		{
		if( !b_x_axis_vector_supplied[ii] )
			{
			valx = x_axis_values_derived_left_value[ii] + left_valuex + ( dt2_tick * i ) ;
			}
		}


	if( ( use_logx ) && ( disp_vals_undo_logx ) ) valx = pow(10, valx);
	valx *= disp_vals_multiplier_x;
	
	m1.make_engineering_str_exp( snum, sunits, scombined, sig_dig, valx, sappend_num, sappend_units, 1 );
	mdo.type = en_dobt_text;

	mdo.draw_ordering = 2;				//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces
	strpf( s1, "%s", scombined.c_str() );
	mdo.arc1 = 0;
	mdo.arc2 = 0;
	mdo.stext = s1;



	mdo.x1 -= 0;
	mdo.y1 = miny + (maxy - miny)/2 - (maxy - miny) * 0.025/fscly_tickx;	//v1.22;
	mdo.r = col_obj_text[ii].r;
	mdo.g = col_obj_text[ii].g;
	mdo.b = col_obj_text[ii].b;

	mdo.font = fnt_type;
	mdo.font_size = fnt_size;
	mdo.justify = en_tj_horiz_center;
	mdo.line_style = (en_mgraph_line_style)FL_SOLID;
	mdo.line_thick = 1;

	gph[ii]->vdrwobj.push_back( mdo );
	}



//y-axis
mdo.use_pos_x = -1;                 //if not set to -1: then use this trace id to get a specific y position
mdo.use_scale_x = -1;               //if not set to -1: then use this trace id to get a specific y scale

mdo.use_pos_y = -1;                 //if not set to -1: then use this trace id to get a specific y position
mdo.use_scale_y = -1;               //if not set to -1: then use this trace id to get a specific y scale

mdo.type  = en_dobt_line;
mdo.draw_ordering = 2;				//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces


mdo.x1 = x_axis_offs;
mdo.y1 = 0;

mdo.x2 = mdo.x1;
mdo.y2 = gph[ii]->h();

mdo.r = col_obj_axis[ii].r;
mdo.g = col_obj_axis[ii].g;
mdo.b = col_obj_axis[ii].b;

gph[ii]->vdrwobj.push_back( mdo );


mdo.use_scale_y = 0;               	//if not set to -1: then use this trace id to get a specific y scale

tick = ytick_marks[ii];

bool fixed_axis_y = 1;													//v1.19



//int vis_wid, vis_hei;	
//gph[ii]->get_trace_vis_dimensions( 0, vis_wid, vis_hei );
//float shfty = (maxy - miny)/2.0f  * ((float)shift_trc_y[ ii ][0] / ((float)vis_hei/2.0f ));	//v1.22, make a value from 'shift_trc_y[][]' (which is in pixels), so that it represents how much a trace has be shifted in y sample values,

float shfty = shift_trc_y[ ii ][0]*gph[ii]->trce[0].yunits_perpxl;

if( fixed_axis_y ) dt = (fmaxy/1 - fminy/1 ) / (float)tick;
else dt = (maxy/scaley - miny/scaley ) / (float)tick;					//NOTE: this never executes



//printf( "update_fg_user_obj()  fminy %f %f   dt n", fminy, fmaxy );

if(vb)printf( "update_fg_user_obj()4  max_defl_y %f  max_tmp %f  fmaxy %f   dt %f  factor %f\n", max_defl_y[ ii ], max_tmp, fmaxy, dt, factor );

fmaxy = dt*(tick/2);
fminy = -dt*(tick/2);

if(vb)printf( "update_fg_user_obj()5  max_defl_y %f  fmaxy %f   dt %f  factor %f\n", max_defl_y[ ii ], fmaxy, dt, factor );


//double ddx, ddy;

//gph[ii]->get_pixel_position_as_trc_values( 0, 0, 10, ddx, ddy );
//gph[ii]->get_mouse_position_relative_to_trc_new( 0, ddx, ddy );
//printf( "update_fg_user_obj() -  ddx %f %f\n", ddx, ddy );


for( int i = 0; i <= tick; i++ )
	{
	//y-axis ticks
	mdo.type  = en_dobt_line;
	mdo.draw_ordering = 2;				//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

	mdo.clip_left = 0;
	mdo.clip_right = 0;
	mdo.clip_top = 0;
	mdo.clip_bottom = 0;

	mdo.x1 = x_axis_offs + 2;
	
	if( fixed_axis_y ) mdo.y1 = fminy + i * dt + 0;
	else mdo.y1 = miny/scaley + i * dt;									//NOTE: this never executes

	mdo.x2 = x_axis_offs - 1;
	mdo.y2 = mdo.y1;

	mdo.r = col_obj_axis[ii].r;
	mdo.g = col_obj_axis[ii].g;
	mdo.b = col_obj_axis[ii].b;

	gph[ii]->vdrwobj.push_back( mdo );

//double posy;
//gph[ii]->get_posy_relative_to_trace(0, posy );						
//posy = shfty;
	//y-axis text vals
	string snum, sunits, scombined, sappend_num, sappend_units;
	sappend_units = "";
	sappend_num = "";

//	int vis_wid, vis_hei;	
//	gph[ii]->get_trace_vis_dimensions( 0, vis_wid, vis_hei );

//	float shfty = (maxy - miny)  * ((float)shift_trc_y[ ii ][0] / vis_hei);									//v1.22
//	float shfty = (maxy - miny)/2  * ((float)shift_trc_y[ ii ][0] / ((float)vis_hei/2 ));									//v1.22



//printf("fast_mgraph::update_fg_user_obj() - shift_trc_y %f shfty %f\n", shift_trc_y[ ii ][0], shfty );



	double valy = mdo.y1;												//v1.13
//	double valy = mdo.y1;										//v1.13
	if( ( use_logy ) && ( disp_vals_undo_logy ) ) valy = pow(10, valy);
	valy *= disp_vals_multiplier_y;

	m1.make_engineering_str_exp( snum, sunits, scombined, sig_dig, valy - shfty/scle_trc_y, sappend_num, sappend_units, 1 );

	mdo.type = en_dobt_text;
	mdo.draw_ordering = 1;				//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

	mdo.type = en_dobt_text;
	strpf( s1, "%s", scombined.c_str() );
	mdo.arc1 = 0;
	mdo.arc2 = 0;
	mdo.stext = s1;

	mdo.clip_left = 0;
	mdo.clip_right = 0;
	mdo.clip_top = 0;
	mdo.clip_bottom = 0;

	mdo.x1 = 8;
	if( fixed_axis_y ) mdo.y1 = fminy/1 + i * dt;//- (maxy - miny ) * 0.02;
	else  mdo.y1 = miny/scaley + i * dt;								//NOTE: this never executes
	
	mdo.r = col_obj_text[ii].r;
	mdo.g = col_obj_text[ii].g;
	mdo.b = col_obj_text[ii].b;
	mdo.font = fnt_type;
	mdo.font_size = fnt_size;
	mdo.justify = en_tj_vert_center;
	mdo.line_style = (en_mgraph_line_style)FL_SOLID;
	mdo.line_thick = 1;

	gph[ii]->vdrwobj.push_back( mdo );
	}




//selection rect
if( sel_idx1[ ii ] != -1 )
	{
	int wid, hei;
	gph[ii]->get_trace_vis_dimensions( 0, wid, hei );


	mdo.type  = en_dobt_rectf;
	mdo.draw_ordering = 0;										//draw before graticle, 0 = before grid,  1 = after graticle but before traces,   2 = after traces

	int ix, iy;

	mdo.use_pos_x = 0;                 //if not set to -1: then use this trace id to get a specific y position
	mdo.use_scale_x = 0;               //if not set to -1: then use this trace id to get a specific y scale

	mdo.use_pos_y = -1;                 //if not set to -1: then use this trace id to get a specific y position
	mdo.use_scale_y = -1;               	//if not set to -1: then use this trace id to get a specific y scale

	mdo.clip_left = 0;
	mdo.clip_right = 0;
	mdo.clip_top = 0;
	mdo.clip_bottom = 0;


	gph[ii]->get_sample_value( 0, sel_idx1[ ii ], valx, valy );

	mdo.x1 = valx;
	mdo.y1 = 0;

	gph[ii]->get_sample_value( 0, sel_idx2[ ii ], valx, valy );

	mdo.x2 = valx;
	mdo.y2 = gph[ii]->h();

	mdo.r = 220;
	mdo.g = 220;
	mdo.b = 180;

	gph[ii]->vdrwobj.push_back( mdo );		//only show if valid
	}




if( vextra_drwobj[ii].size() != 0 )
	{
	for( int jj = 0; jj < vextra_drwobj[ii].size(); jj++ )
		{
		gph[ii]->vdrwobj.push_back( vextra_drwobj[ii][jj] );
		}	
	}

gph[ ii ]->redraw();
}




void fast_mgraph::user_col( const char* szcol, mg_col_tag &col )
{

col.r = 255;
col.g = 255;
col.b = 255;



if( strcmp( szcol, "r" ) == 0 )
	{
	col.r = 255;
	col.g = 0;
	col.b = 0;
	}

if( strcmp( szcol, "drkr" ) == 0 )
	{
	col.r = 200;
	col.g = 0;
	col.b = 0;
	}

if( strcmp( szcol, "red" ) == 0 )
	{
	col.r = 255;
	col.g = 0;
	col.b = 0;
	}

if( strcmp( szcol, "drkred" ) == 0 )
	{
	col.r = 200;
	col.g = 0;
	col.b = 0;
	}



if( strcmp( szcol, "pink" ) == 0 )
	{
	col.r = 244;
	col.g = 194;
	col.b = 197;
	}

if( strcmp( szcol, "drkpink" ) == 0 )
	{
	col.r = 202;
	col.g = 142;
	col.b = 146;
	}


if( strcmp( szcol, "g" ) == 0 )
	{
	col.r = 0;
	col.g = 255;
	col.b = 0;
	}
if( strcmp( szcol, "drkg" ) == 0 )
	{
	col.r = 0;
	col.g = 150;
	col.b = 0;
	}

if( strcmp( szcol, "green" ) == 0 )
	{
	col.r = 0;
	col.g = 255;
	col.b = 0;
	}
if( strcmp( szcol, "drkgreen" ) == 0 )
	{
	col.r = 0;
	col.g = 150;
	col.b = 0;
	}



if( strcmp( szcol, "yellow" ) == 0 )
	{
	col.r = 255;
	col.g = 239;
	col.b = 21;
	}
if( strcmp( szcol, "drkyellow" ) == 0 )
	{
	col.r = 236;
	col.g = 224;
	col.b = 61;
	}



if( strcmp( szcol, "yel" ) == 0 )
	{
	col.r = 255;
	col.g = 239;
	col.b = 21;
	}

if( strcmp( szcol, "drkyel" ) == 0 )
	{
	col.r = 236;
	col.g = 224;
	col.b = 61;
	}



if( strcmp( szcol, "b" ) == 0 )
	{
	col.r = 0;
	col.g = 0;
	col.b = 255;
	}

if( strcmp( szcol, "drkb" ) == 0 )
	{
	col.r = 0;
	col.g = 0;
	col.b = 200;
	}


if( strcmp( szcol, "blue" ) == 0 )
	{
	col.r = 0;
	col.g = 0;
	col.b = 255;
	}

if( strcmp( szcol, "drkblue" ) == 0 )
	{
	col.r = 0;
	col.g = 0;
	col.b = 200;
	}

if( strcmp( szcol, "cy" ) == 0 )
	{
	col.r = 0;
	col.g = 255;
	col.b = 255;
	}

if( strcmp( szcol, "drkcy" ) == 0 )
	{
	col.r = 20;
	col.g = 184;
	col.b = 210;
	}



if( strcmp( szcol, "cyan" ) == 0 )
	{
	col.r = 0;
	col.g = 255;
	col.b = 255;
	}

if( strcmp( szcol, "drkcyan" ) == 0 )
	{
	col.r = 20;
	col.g = 184;
	col.b = 210;
	}


if( strcmp( szcol, "mg" ) == 0 )
	{
	col.r = 255;
	col.g = 0;
	col.b = 255;
	}



if( strcmp( szcol, "drkmg" ) == 0 )
	{
	col.r = 200;
	col.g = 0;
	col.b = 200;
	}

if( strcmp( szcol, "brwn" ) == 0 )
	{
	col.r = 176;
	col.g = 113;
	col.b = 22;
	}


if( strcmp( szcol, "y" ) == 0 )
	{
	col.r = 255;
	col.g = 255;
	col.b = 0;
	}


if( strcmp( szcol, "drky" ) == 0 )
	{
	col.r = 223;
	col.g = 201;
	col.b = 0;
	}


if( strcmp( szcol, "orange" ) == 0 )
	{
	col.r = 255;
	col.g = 155;
	col.b = 6;
	}


if( strcmp( szcol, "drkorange" ) == 0 )
	{
	col.r = 246;
	col.g = 173;
	col.b = 84;
	}


if( strcmp( szcol, "brown" ) == 0 )
	{
	col.r = 187;
	col.g = 125;
	col.b = 73;
	}
	

if( strcmp( szcol, "drkbrown" ) == 0 )
	{
	col.r = 159;
	col.g = 97;
	col.b = 45;
	}

if( strcmp( szcol, "mg" ) == 0 )
	{
	col.r = 255;
	col.g = 0;
	col.b = 255;
	}

if( strcmp( szcol, "blk" ) == 0 )
	{
	col.r = 0;
	col.g = 0;
	col.b = 0;
	}

if( strcmp( szcol, "gry" ) == 0 )
	{
	col.r = 150;
	col.g = 150;
	col.b = 150;
	}

if( strcmp( szcol, "grey" ) == 0 )
	{
	col.r = 150;
	col.g = 150;
	col.b = 150;
	}

if( strcmp( szcol, "drkgry" ) == 0 )
	{
	col.r = 80;
	col.g = 80;
	col.b = 80;
	}

if( strcmp( szcol, "drkgrey" ) == 0 )
	{
	col.r = 80;
	col.g = 80;
	col.b = 80;
	}

if( strcmp( szcol, "ofw" ) == 0 )
	{
	col.r = 240;
	col.g = 240;
	col.b = 240;
	}

}







//bring supplied vector up to required count
void fast_mgraph::back_fill( int req_cnt, vector<int> &vv )
{
int cnt = req_cnt - vv.size();

if( cnt <= 0 ) return;

double fill = 0;
int last = vv.size() - 1;

if( last > 0 ) fill = vv[ last ];				//use last avail sample val

for( int i = 0; i < cnt; i++ )
	{
	vv.push_back( fill );	
	}
}






//bring supplied vector up to required count
void fast_mgraph::back_fill( int req_cnt, vector<float> &vv )
{
int cnt = req_cnt - vv.size();

if( cnt <= 0 ) return;

double fill = 0;
int last = vv.size() - 1;

if( last > 0 ) fill = vv[ last ];				//use last avail sample val

for( int i = 0; i < cnt; i++ )
	{
	vv.push_back( fill );	
	}
}







//bring supplied vector up to required count
void fast_mgraph::back_fill( int req_cnt, vector<double> &vv )
{
int cnt = req_cnt - vv.size();

if( cnt <= 0 ) return;

double fill = 0;
int last = vv.size() - 1;

if( last > 0 ) fill = vv[ last ];				//use last avail sample val

for( int i = 0; i < cnt; i++ )
	{
	vv.push_back( fill );	
	}
}









//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_bkgd_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;


gph[gph_idx]->background.r = rr;
gph[gph_idx]->background.g = gg;
gph[gph_idx]->background.b = bb;

}








//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_bkgd( int gph_idx, const char *col_bkgd )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_bkgd, col );

gph[gph_idx]->background.r = col.r;
gph[gph_idx]->background.g = col.g;
gph[gph_idx]->background.b = col.b;

}





//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_axis1_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_axis_multi_1[gph_idx] = col;

}





//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_axis1( int gph_idx, const char *col_axis )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_axis, col );

col_axis_multi_1[gph_idx] = col;

}





//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_axis2( int gph_idx, const char *col_axis )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_axis, col );

col_axis_multi_2[gph_idx] = col;

}






//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_axis3( int gph_idx, const char *col_axis )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_axis, col );

col_axis_multi_3[gph_idx] = col;

}






//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_axis4( int gph_idx, const char *col_axis )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_axis, col );

col_axis_multi_4[gph_idx] = col;

}





//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_hover_text1_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_hover_text_multi_1[gph_idx] = col;
}








//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_hover_text2_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_hover_text_multi_2[gph_idx] = col;
}









//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_hover_text3_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_hover_text_multi_3[gph_idx] = col;
}










//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_hover_text4_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_hover_text_multi_4[gph_idx] = col;
}






//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_hover_text1( int gph_idx, const char *col_axis )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_axis, col );

col_hover_text_multi_1[gph_idx] = col;

}






//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_hover_text2( int gph_idx, const char *col_axis )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_axis, col );

col_hover_text_multi_2[gph_idx] = col;

}








//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_hover_text3( int gph_idx, const char *col_axis )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_axis, col );

col_hover_text_multi_3[gph_idx] = col;

}







//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_hover_text4( int gph_idx, const char *col_axis )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_axis, col );

col_hover_text_multi_4[gph_idx] = col;

}










//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_obj_col_text1_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;


col.r = rr;
col.g = gg;
col.b = bb;

col_obj_text_multi_1[gph_idx] = col;

}






//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_obj_col_text2_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;


col.r = rr;
col.g = gg;
col.b = bb;

col_obj_text_multi_2[gph_idx] = col;

}




//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_obj_col_text3_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;


col.r = rr;
col.g = gg;
col.b = bb;

col_obj_text_multi_3[gph_idx] = col;

}




//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_obj_col_text4_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;


col.r = rr;
col.g = gg;
col.b = bb;

col_obj_text_multi_4[gph_idx] = col;

}








//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_obj_col_text1( int gph_idx, const char *col_text )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_text, col );

col_obj_text_multi_1[gph_idx] = col;

}







void fast_mgraph::set_obj_col_text2( int gph_idx, const char *col_text )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_text, col );

col_obj_text_multi_2[gph_idx] = col;

}








void fast_mgraph::set_obj_col_text3( int gph_idx, const char *col_text )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_text, col );

col_obj_text_multi_3[gph_idx] = col;

}









void fast_mgraph::set_obj_col_text4( int gph_idx, const char *col_text )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_text, col );

col_obj_text_multi_4[gph_idx] = col;

}






//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_trc1_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_trc_multi_1[gph_idx] = col;
}







//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_trc2_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_trc_multi_2[gph_idx] = col;
}





//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_trc3_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_trc_multi_3[gph_idx] = col;
}







//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_trc4_rgb( int gph_idx, uint8_t rr, uint8_t gg, uint8_t bb )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

col.r = rr;
col.g = gg;
col.b = bb;

col_trc_multi_4[gph_idx] = col;
}







//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_trc1( int gph_idx, const char *col_trc1 )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_trc1, col );

col_trc_multi_1[gph_idx] = col;
}






//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_trc2( int gph_idx, const char *col_trc2 )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_trc2, col );

col_trc_multi_2[gph_idx] = col;
}






//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_trc3( int gph_idx, const char *col_trc3 )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_trc3, col );

col_trc_multi_3[gph_idx] = col;
}







//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_col_trc4( int gph_idx, const char *col_trc4 )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

mg_col_tag col;

user_col( col_trc4, col );

col_trc_multi_4[gph_idx] = col;
}





void fast_mgraph::set_trc_label1( int grph_idx, string ss )
{
if ( grph_idx >= gph_cnt ) return;
if ( grph_idx < 0 ) return;

trc_label_multi_1[grph_idx] = ss;
}




void fast_mgraph::set_trc_label2( int grph_idx, string ss )
{
if ( grph_idx >= gph_cnt ) return;
if ( grph_idx < 0 ) return;

trc_label_multi_2[grph_idx] = ss;
}




void fast_mgraph::set_trc_label3( int grph_idx, string ss )
{
if ( grph_idx >= gph_cnt ) return;
if ( grph_idx < 0 ) return;

trc_label_multi_3[grph_idx] = ss;
}



void fast_mgraph::set_trc_label4( int grph_idx, string ss )
{
if ( grph_idx >= gph_cnt ) return;
if ( grph_idx < 0 ) return;

trc_label_multi_4[grph_idx] = ss;
}



/*
//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_trc_label2( int gph_idx, string ss )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

trc_label_multi_2[gph_idx] = ss;
}



//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_trc_label3( int gph_idx, string ss )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

trc_label_multi_3[gph_idx] = ss;
}




//set 'gph_idx' to zero, unless there are multiple graphs
void fast_mgraph::set_trc_label4( int gph_idx, string ss )
{
if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;

trc_label_multi_4[gph_idx] = ss;
}
*/


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 














//multiple graphs
void fast_mgraph::plotxy_vfloat( int gph_idx, vector<float> &vx, vector<float> &vy1 )
{
if( plot_updating_state == 0 ) return;
if( gph_idx < 0 ) return;
if( gph_idx >= gph_cnt ) return;

multi_trace = 0;

b_x_axis_vector_supplied[gph_idx] = 1;

vtrc_valx[ gph_idx ].clear();
vtrc_valy1[ gph_idx ].clear();

for( int i = 0; i < vx.size(); i++ )
	{
	if( i >= vy1.size()  ) break;

	vtrc_valx[ gph_idx ].push_back( vx[i] );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ gph_idx ].push_back( vy1[i] );
	}


plot_grph_internal( gph_idx );
}











//multiple graphs, (vector x not suppied)
void fast_mgraph::plot_vfloat( int gph_idx, vector<float> &vy1 )
{
if( plot_updating_state == 0 ) return;
if( gph_idx < 0 ) return;
if( gph_idx >= gph_cnt ) return;

multi_trace = 0;

b_x_axis_vector_supplied[gph_idx] = 0;

vtrc_valx[ gph_idx ].clear();
vtrc_valy1[ gph_idx ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ gph_idx ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ gph_idx ].push_back( vy1[i] );
	}


plot_grph_internal( gph_idx );
}










//multiple graphs
void fast_mgraph::plotxy_vdouble( int gph_idx, vector<double> &vx, vector<double> &vy1 )
{
if( plot_updating_state == 0 ) return;
if( gph_idx < 0 ) return;
if( gph_idx >= gph_cnt ) return;

multi_trace = 0;

b_x_axis_vector_supplied[gph_idx] = 1;

vtrc_valx[ gph_idx ].clear();
vtrc_valy1[ gph_idx ].clear();

for( int i = 0; i < vx.size(); i++ )
	{
	if( i >= vy1.size()  ) break;

	vtrc_valx[ gph_idx ].push_back( vx[i] );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ gph_idx ].push_back( vy1[i] );
	}


plot_grph_internal( gph_idx );
}












//multiple graphs, (vector x not suppied)
void fast_mgraph::plot_vdouble( int gph_idx, vector<double> &vy1 )
{
if( plot_updating_state == 0 ) return;

if( gph_idx < 0 ) return;
if( gph_idx >= gph_cnt ) return;

multi_trace = 0;

b_x_axis_vector_supplied[gph_idx] = 0;


vtrc_valx[ gph_idx ].clear();
vtrc_valy1[ gph_idx ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ gph_idx ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ gph_idx ].push_back( vy1[i] );
	}


plot_grph_internal( gph_idx );
}









//one graph, single trace (vector x not suppied)
void fast_mgraph::plot_vfloat_1( vector<float> &vy1 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 1;

b_x_axis_vector_supplied[0] = 0;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ 0 ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}











//one graph, 2 traces (vector x not suppied)
void fast_mgraph::plot_vfloat_2( vector<float> &vy1, vector<float> &vy2 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 2;

b_x_axis_vector_supplied[0] = 0;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ 0 ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}










//one graph, 2 traces (vector x not suppied)
void fast_mgraph::plot_vfloat_3( vector<float> &vy1, vector<float> &vy2, vector<float> &vy3 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 3;

b_x_axis_vector_supplied[0] = 0;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();
vtrc_valy3[ 0 ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ 0 ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );

	if( i < vy3.size()  ) vtrc_valy3[ 0 ].push_back( vy3[i] );
	else vtrc_valy3[ 0 ].push_back( 0 );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}





//one graph, 3 traces (vector x not suppied)
void fast_mgraph::plot_vfloat_4( vector<float> &vy1, vector<float> &vy2, vector<float> &vy3, vector<float> &vy4 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 4;

b_x_axis_vector_supplied[0] = 0;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();
vtrc_valy3[ 0 ].clear();
vtrc_valy4[ 0 ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ 0 ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );

	if( i < vy3.size()  ) vtrc_valy3[ 0 ].push_back( vy3[i] );
	else vtrc_valy3[ 0 ].push_back( 0 );

	if( i < vy4.size()  ) vtrc_valy4[ 0 ].push_back( vy4[i] );
	else vtrc_valy4[ 0 ].push_back( 0 );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}








//one graph, single trace (vector x not suppied)
void fast_mgraph::plot_vdouble_1( vector<double> &vy1 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 1;

b_x_axis_vector_supplied[0] = 0;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ 0 ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}






//one graph, 2 traces (vector x not suppied)
void fast_mgraph::plot_vdouble_2( vector<double> &vy1, vector<double> &vy2 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 2;

b_x_axis_vector_supplied[0] = 0;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ 0 ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}





//one graph, 2 traces (vector x not suppied)
void fast_mgraph::plot_vdouble_3( vector<double> &vy1, vector<double> &vy2, vector<double> &vy3 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 3;

b_x_axis_vector_supplied[0] = 0;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();
vtrc_valy3[ 0 ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ 0 ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );

	if( i < vy3.size()  ) vtrc_valy3[ 0 ].push_back( vy3[i] );
	else vtrc_valy3[ 0 ].push_back( 0 );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}





//one graph, 3 traces (vector x not suppied)
void fast_mgraph::plot_vdouble_4( vector<double> &vy1, vector<double> &vy2, vector<double> &vy3, vector<double> &vy4 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 4;

b_x_axis_vector_supplied[0] = 0;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();
vtrc_valy3[ 0 ].clear();
vtrc_valy4[ 0 ].clear();

for( int i = 0; i < vy1.size(); i++ )
	{
	vtrc_valx[ 0 ].push_back( i );						//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );

	if( i < vy3.size()  ) vtrc_valy3[ 0 ].push_back( vy3[i] );
	else vtrc_valy3[ 0 ].push_back( 0 );

	if( i < vy4.size()  ) vtrc_valy4[ 0 ].push_back( vy4[i] );
	else vtrc_valy4[ 0 ].push_back( 0 );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}







//one graph, single trace
void fast_mgraph::plotxy_vfloat_1( vector<float> &vx, vector<float> &vy1 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 1;

b_x_axis_vector_supplied[0] = 1;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();

for( int i = 0; i < vx.size(); i++ )
	{
	if( i >= vy1.size()  )break;
	
	vtrc_valx[ 0 ].push_back( vx[i] );					//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
	vtrc_valy1[ 0 ].push_back( vy1[i] );
	}

//if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitute adjs are performed

plot_grph_internal( -1 );
}




//one graph, 2 traces
void fast_mgraph::plotxy_vfloat_2( vector<float> &vx, vector<float> &vy1, vector<float> &vy2 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 2;

b_x_axis_vector_supplied[0] = 1;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();

for( int i = 0; i < vx.size(); i++ )
	{	
	vtrc_valx[ 0 ].push_back( vx[i] );					//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed

	if( i < vy1.size()  ) vtrc_valy1[ 0 ].push_back( vy1[i] );
	else vtrc_valy1[ 0 ].push_back( 0 );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );
	}

plot_grph_internal( -1 );
}






//one graph, 3 traces
void fast_mgraph::plotxy_vfloat_3( vector<float> &vx, vector<float> &vy1, vector<float> &vy2, vector<float> &vy3 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 3;

b_x_axis_vector_supplied[0] = 1;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();
vtrc_valy3[ 0 ].clear();

for( int i = 0; i < vx.size(); i++ )
	{	
	vtrc_valx[ 0 ].push_back( vx[i] );					//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed

	if( i < vy1.size()  ) vtrc_valy1[ 0 ].push_back( vy1[i] );
	else vtrc_valy1[ 0 ].push_back( 0 );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );

	if( i < vy3.size()  ) vtrc_valy3[ 0 ].push_back( vy3[i] );
	else vtrc_valy3[ 0 ].push_back( 0 );
	}

plot_grph_internal( -1 );
}




//one graph, 4 traces
void fast_mgraph::plotxy_vfloat_4( vector<float> &vx, vector<float> &vy1, vector<float> &vy2, vector<float> &vy3, vector<float> &vy4 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 4;

b_x_axis_vector_supplied[0] = 1;

vtrc_valx[ 0 ].clear();
vtrc_valy1[ 0 ].clear();
vtrc_valy2[ 0 ].clear();
vtrc_valy3[ 0 ].clear();
vtrc_valy4[ 0 ].clear();

for( int i = 0; i < vx.size(); i++ )
	{	
	vtrc_valx[ 0 ].push_back( vx[i] );					//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed

	if( i < vy1.size()  ) vtrc_valy1[ 0 ].push_back( vy1[i] );
	else vtrc_valy1[ 0 ].push_back( 0 );

	if( i < vy2.size()  ) vtrc_valy2[ 0 ].push_back( vy2[i] );
	else vtrc_valy2[ 0 ].push_back( 0 );

	if( i < vy3.size()  ) vtrc_valy3[ 0 ].push_back( vy3[i] );
	else vtrc_valy3[ 0 ].push_back( 0 );

	if( i < vy4.size()  ) vtrc_valy4[ 0 ].push_back( vy4[i] );
	else vtrc_valy4[ 0 ].push_back( 0 );
	}

plot_grph_internal( -1 );
}







//one graph, single trace
void fast_mgraph::plotxy_vdouble_1( vector<double> &vx, vector<double> &vy1 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 1;

b_x_axis_vector_supplied[0] = 1;

vtrc_valx[ 0 ] = vx;
vtrc_valy1[ 0 ] = vy1;

if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed

plot_grph_internal( -1 );
}








//one graph, 2 traces, first trace x, y is supplied, for 2nd trace only y is supplied and must be same length as first trace
void fast_mgraph::plotxy_vdouble_2( vector<double> &vx, vector<double> &vy1, vector<double> &vy2 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 2;

b_x_axis_vector_supplied[0] = 1;

vtrc_valx[ 0 ] = vx;
vtrc_valy1[ 0 ] = vy1;
vtrc_valy2[ 0 ] = vy2;

if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
if( vtrc_valy2[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy2[0] );

plot_grph_internal( -1 );
}










//one graph, 3 traces, first trace x, y is supplied, for 2nd/3rd traces only y is supplied and must be same length as first trace
void fast_mgraph::plotxy_vdouble_3( vector<double> &vx, vector<double> &vy1, vector<double> &vy2, vector<double> &vy3 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 3;

b_x_axis_vector_supplied[0] = 1;

vtrc_valx[ 0 ] = vx;
vtrc_valy1[ 0 ] = vy1;
vtrc_valy2[ 0 ] = vy2;
vtrc_valy3[ 0 ] = vy3;

if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
if( vtrc_valy2[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy2[0] );
if( vtrc_valy3[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy3[0] );

plot_grph_internal( -1 );
}




//one graph, 4 traces, first trace x, y is supplied, for 2nd/3rd/4th traces only y is supplied and must be same length as first trace
void fast_mgraph::plotxy_vdouble_4( vector<double> &vx, vector<double> &vy1, vector<double> &vy2, vector<double> &vy3, vector<double> &vy4 )
{
if( plot_updating_state == 0 ) return;
multi_trace = 4;

b_x_axis_vector_supplied[0] = 1;

vtrc_valx[ 0 ] = vx;
vtrc_valy1[ 0 ] = vy1;
vtrc_valy2[ 0 ] = vy2;
vtrc_valy3[ 0 ] = vy3;
vtrc_valy4[ 0 ] = vy4;

if( vtrc_valy1[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy1[0] );	//cache vectors to allow internal redrawing from callbacks, e.g when mousewheel amplitude adjs are performed
if( vtrc_valy2[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy2[0] );
if( vtrc_valy3[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy3[0] );
if( vtrc_valy4[0].size() < vtrc_valx[0].size() ) back_fill( vtrc_valx[0].size(), vtrc_valy4[0] );

plot_grph_internal( -1 );
}
//----------------






//multiple graphs with 1 trace only, OR, single graph with up to 4 traces
void fast_mgraph::plot_grph_internal( int grph_idx )
{
	

int ii = 0;

if( grph_idx >= 0 )		//multiple graphs?
	{
	ii = grph_idx;
	}







//---- record recent plots histories ----
if( ( b_prev_plot_recording ) && ( plot_updating_state == 1 ) )
	{
	if( prev_plot_rec_idx == 0 )
		{
		vtrc_recx1[0] = vtrc_valx[ii];	
		vtrc_recy1[0] = vtrc_valy1[ii];	
		vtrc_recy2[0] = vtrc_valy2[ii];	
		vtrc_recy3[0] = vtrc_valy3[ii];	
		vtrc_recy4[0] = vtrc_valy4[ii];	
		}

	if( prev_plot_rec_idx == 1 )
		{
		vtrc_recx1[1] = vtrc_valx[ii];	
		vtrc_recy1[1] = vtrc_valy1[ii];	
		vtrc_recy2[1] = vtrc_valy2[ii];	
		vtrc_recy3[1] = vtrc_valy3[ii];	
		vtrc_recy4[1] = vtrc_valy4[ii];	
		}


	if( prev_plot_rec_idx == 2 )
		{
		vtrc_recx1[2] = vtrc_valx[ii];	
		vtrc_recy1[2] = vtrc_valy1[ii];	
		vtrc_recy2[2] = vtrc_valy2[ii];	
		vtrc_recy3[2] = vtrc_valy3[ii];	
		vtrc_recy4[2] = vtrc_valy4[ii];	
		}

	if( prev_plot_rec_idx == 3 )
		{
		vtrc_recx1[3] = vtrc_valx[ii];	
		vtrc_recy1[3] = vtrc_valy1[ii];	
		vtrc_recy2[3] = vtrc_valy2[ii];	
		vtrc_recy3[3] = vtrc_valy3[ii];	
		vtrc_recy4[3] = vtrc_valy4[ii];	
		}

//printf( "plot_grph_internal() -  recorded in prev_plot_rec_idx %d\n", prev_plot_rec_idx );
	prev_plot_rec_idx++;
	if( prev_plot_rec_idx >= cn_prev_plot_rec_max ) prev_plot_rec_idx = 0;

	}
//----


//---- recall recent plot history ----
if( plot_updating_state == 2 )
	{
	int show_idx = 0;
	
	if( prev_plot_show_choice == 0 )									//last plot?
		{		
		show_idx = prev_plot_rec_idx - 1;								//one behind, which is the current plot gathered just above
		if( show_idx < 0 ) show_idx += cn_prev_plot_rec_max;
		}

	if( prev_plot_show_choice == 1 )									//2nd last plot?
		{		
		show_idx = prev_plot_rec_idx - 2;								//one behind, which is the current plot gathered just above
		if( show_idx < 0 ) show_idx += cn_prev_plot_rec_max;
		}

	if( prev_plot_show_choice == 2 )									//2nd last plot?
		{		
		show_idx = prev_plot_rec_idx - 3;								//one behind, which is the current plot gathered just above
		if( show_idx < 0 ) show_idx += cn_prev_plot_rec_max;
		}

	if( prev_plot_show_choice == 3 )									//2nd last plot?
		{		
		show_idx = prev_plot_rec_idx - 4;								//one behind, which is the current plot gathered just above
		if( show_idx < 0 ) show_idx += cn_prev_plot_rec_max;
		}

//printf( "plot_grph_internal() -  prev_plot_rec_idx %d   show_idx %d\n", prev_plot_rec_idx, show_idx );

	vtrc_valx[ii] = vtrc_recx1[show_idx];
	vtrc_valy1[ii] = vtrc_recy1[show_idx];
	vtrc_valy2[ii] = vtrc_recy2[show_idx];
	vtrc_valy3[ii] = vtrc_recy3[show_idx];
	vtrc_valy4[ii] = vtrc_recy4[show_idx];
	}
//----



//if (!b_plot_allow_updating) return;










string s1;
mystr m1;

//multi_trace = trc_cnt;

double dd;


//if( b_refresh_traces )
	{
//	vtrc_valx[ 0 ] = vx;
//	vtrc_valy1[ 0 ] = vy1;
//	vtrc_valy2[ 0 ] = vy2;
//	vtrc_valy3[ 0 ] = vy3;
//	vtrc_valy4[ 0 ] = vy4;
	}

x_axis_offs = 65;

gph[ii]->get_scalex( 0, scale_x );						//get current state, before refreshing traces
gph[ii]->get_posx( 0, pos_x );

//gph[ii]->get_scaley( 0, gph0_scaley0 );

//scale_trc_y[0] = gph0_scaley0;

//get_posx( int trc, double &val )


gph[ii]->clear_traces();

mg_col_tag col;
trace_tag tr1;

//user_col( col_axis, col );
//col_obj_axis = col;

//user_col( col_text, col );
//col_obj_text = col;


//user_col( col_bkgd, col );

//gph[ii]->background.r = col.r;
//gph[ii]->background.g = col.g;
//gph[ii]->background.b = col.b;


//user_col( col_trc1, col );
//trc_label[ 0 ] = trc_label1;



//user_col( col_trc1, color_trc[0] );

if( multi_trace != 0 )								//one graph, multiple traces?
	{
//printf("here 000\n" );


	col_obj_axis[0] = col_axis_multi_1[0];
	col_obj_axis[1] = col_axis_multi_2[0];
	col_obj_axis[2] = col_axis_multi_3[0];
	col_obj_axis[3] = col_axis_multi_4[0];

	trc_label[ 0 ] = trc_label_multi_1[0];
	trc_label[ 1 ] = trc_label_multi_2[0];
	trc_label[ 2 ] = trc_label_multi_3[0];
	trc_label[ 3 ] = trc_label_multi_4[0];

	col_obj_text[0] = col_obj_text_multi_1[0];
	col_obj_text[1] = col_obj_text_multi_2[0];
	col_obj_text[2] = col_obj_text_multi_3[0];
	col_obj_text[3] = col_obj_text_multi_4[0];

	col_hover_text[0] = col_hover_text_multi_1[0];
	col_hover_text[1] = col_hover_text_multi_2[0];
	col_hover_text[2] = col_hover_text_multi_3[0];
	col_hover_text[3] = col_hover_text_multi_4[0];

	color_trc[0] = col_trc_multi_1[0];
	color_trc[1] = col_trc_multi_2[0];
	color_trc[2] = col_trc_multi_3[0];
	color_trc[3] = col_trc_multi_4[0];
	}
else{
//printf("here 001\n" );

	col_obj_axis[0] = col_axis_multi_1[0];
	col_obj_axis[1] = col_axis_multi_1[1];
	col_obj_axis[2] = col_axis_multi_1[2];
	col_obj_axis[3] = col_axis_multi_1[3];

	trc_label[ 0 ] = trc_label_multi_1[0];
	trc_label[ 1 ] = trc_label_multi_1[1];
	trc_label[ 2 ] = trc_label_multi_1[2];
	trc_label[ 3 ] = trc_label_multi_1[3];

	col_obj_text[0] = col_obj_text_multi_1[0];
	col_obj_text[1] = col_obj_text_multi_1[1];
	col_obj_text[2] = col_obj_text_multi_1[2];
	col_obj_text[3] = col_obj_text_multi_1[3];

	col_hover_text[0] = col_hover_text_multi_1[0];
	col_hover_text[1] = col_hover_text_multi_1[1];
	col_hover_text[2] = col_hover_text_multi_1[2];
	col_hover_text[3] = col_hover_text_multi_1[3];

	color_trc[0] = col_trc_multi_1[0];				//multiple graphs, one trace?
	color_trc[1] = col_trc_multi_1[1];				//multiple graphs, one trace?
	color_trc[2] = col_trc_multi_1[2];				//multiple graphs, one trace?
	color_trc[3] = col_trc_multi_1[3];				//multiple graphs, one trace?
	}


int color_trc_idx = 0;
if( multi_trace == 0 ) color_trc_idx = ii;			//multiple graphs, one trace ?


tr1.id = 0;                      //identify which trace this is, helps when traces are push_back'd in unknown order
tr1.vis = 1;
tr1.col = color_trc[color_trc_idx];
tr1.line_thick = 1;
tr1.lineplot = 1;
tr1.line_style = (en_mgraph_line_style) en_mls_solid;
tr1.border_left = x_axis_offs;
tr1.border_right = 0;
tr1.border_top = 10;
tr1.border_bottom = 10;

tr1.show_as_spectrum = 0;                           //not a spectra plot
tr1.spectrum_baseline_y = 0;

tr1.b_limit_auto_scale_min_for_y = 0;
tr1.b_limit_auto_scale_max_for_y = 0;

tr1.xunits_perpxl = xunits_perpxl[ii];
tr1.yunits_perpxl = yunits_perpxl[ii];

//tr1.posx = 0;//-tr1.border_left + x_axis_offs;
tr1.posx = pos_x;
tr1.posy = shift_trc_y[ii][ 0 ];

tr1.plot_offsx = 0;                 				//not affected by override: 'use_pos_y', still allows independent trace offsets at pixel level
tr1.plot_offsy = 0;

//tr1.scalex = 1;
//tr1.scaley = 1;

tr1.scalex = scale_x;
tr1.scaley = scale_trc_y[ii][0];



tr1.single_sel_col.r = 255;
tr1.single_sel_col.g = 0;
tr1.single_sel_col.b = 255;

tr1.group_sel_trace_col.r = 230;
tr1.group_sel_trace_col.g = 230;
tr1.group_sel_trace_col.b = 230;

tr1.group_sel_rect_col.r = 255;
tr1.group_sel_rect_col.g = 153;
tr1.group_sel_rect_col.b = 0;

//tr1.sample_rect_hints_double_click = 1;
//tr1.sample_rect_hints = 1;
//tr1.sample_rect_hints_col.r = 150;
//tr1.sample_rect_hints_col.g = 120;
//tr1.sample_rect_hints_col.b = 150;
//tr1.sample_rect_hints_distancex = 12;						//helps stop over hinting on x-axis
//tr1.sample_rect_hints_distancey = 0;						//disable over hinting test/clearing for y-axis 

tr1.clip_left = tr1.border_left;
tr1.clip_right = tr1.border_right;
tr1.clip_top = tr1.border_top;
tr1.clip_bottom = tr1.border_bottom;


tr1.use_pos_y = -1;				//if not -1, use this trace's id as a reference for this val
tr1.use_scale_y = -1;			//if not -1, use this trace's id as a reference for this val

tr1.sample_rect_hints_double_click = sample_rect_hints_double_click;//v1.13
tr1.sample_rect_hints = sample_rect_hints;							//v1.13
tr1.sample_rect_hints_col = sample_rect_hints_col;					//v1.13
tr1.sample_rect_hints_distancex = sample_rect_hints_distancex;		//v1.13
tr1.sample_rect_hints_distancey = sample_rect_hints_distancey;		//v1.13

tr1.sample_rect_flicker = 0;										//v1.22


pnt_tag pnt1;

double minx, miny, maxx, maxy;

minx = miny = maxx = maxy = 0;






//trace1 [0]
for( int i = 0; i < vtrc_valx[ii].size(); i++ )
	{
//printf("y: %f\n", vf[ i ] );

	pnt1.x = vtrc_valx[ii][ i ];
	pnt1.y = vtrc_valy1[ii][ i ];



	if( use_logx )
		{
		pnt1.x = log10( pnt1.x );									//v1.13	
		}


	if( use_logy )
		{
		pnt1.y = log10( pnt1.y );									//v1.13	
		}


	pnt1.y = pnt1.y;// * scale_trc_y[ 0 ] + shift_trc_y[ 0 ];
	pnt1.sel = 0;
//if( ( i >= 5000 ) & ( i <= 20000) ) pnt1.sel = 1;

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


gph[ii]->add_trace( tr1 );



//trace2 [1]
if( multi_trace >= 2 )
	{
	//user_col( col_trc2, col );
	//trc_label[ 1 ] = trc_label2;
	//user_col( col_trc2, color_trc[1] );

	tr1.id = 1;                      //identify which trace this is, helps when traces are push_back'd in unknown order
	tr1.vis = 1;
	tr1.col = color_trc[1];

	tr1.use_pos_y = 0;				//if not -1, use this trace's id as a reference for this val
	tr1.use_scale_y = 0;			//if not -1, use this trace's id as a reference for this val

	tr1.plot_offsy = shift_trc_y[ii][ 1 ];

	tr1.pnt.clear();

	minx = miny = maxx = maxy = 0;

	for( int i = 0; i < vtrc_valx[ii].size(); i++ )
		{
	//printf("y: %f\n", vf[ i ] );

		pnt1.x = vtrc_valx[ii][ i ];
//		pnt1.y = vy2[ i ];
		pnt1.y = vtrc_valy2[ii][ i ];

		if( use_logx )
			{
			pnt1.x = log10( pnt1.x );									//v1.13	
			}

		if( use_logy )
			{
			pnt1.y = log10( pnt1.y );									//v1.13	
			}


		pnt1.y = pnt1.y * scale_trc_y[ii][ 1 ];// + shift_trc_y[ 1 ];
		pnt1.sel = 0;
	//if( ( i >= 5000 ) & ( i <= 20000) ) pnt1.sel = 1;

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


	gph[ii]->add_trace( tr1 );
	}


//trace3 [2]
if( multi_trace >= 3 )
	{
	//user_col( col_trc3, col );
	//trc_label[ 2 ] = trc_label3;
	//user_col( col_trc3, color_trc[2] );

	tr1.id = 2;                      //identify which trace this is, helps when traces are push_back'd in unknown order
	tr1.vis = 1;
	tr1.col = color_trc[2];

	tr1.use_pos_y = 0;				//if not -1, use this trace's id as a reference for this val
	tr1.use_scale_y = 0;			//if not -1, use this trace's id as a reference for this val

	tr1.plot_offsy = shift_trc_y[ii][ 2 ];

	tr1.pnt.clear();

	minx = miny = maxx = maxy = 0;

	for( int i = 0; i < vtrc_valx[ii].size(); i++ )
		{
	//printf("y: %f\n", vf[ i ] );

		pnt1.x = vtrc_valx[ii][ i ];
//		pnt1.y = vy3[ i ];
		pnt1.y = vtrc_valy3[ii][ i ];

		if( use_logx )
			{
			pnt1.x = log10( pnt1.x );									//v1.13	
			}

		if( use_logy )
			{
			pnt1.y = log10( pnt1.y );									//v1.13	
			}

		pnt1.y = pnt1.y * scale_trc_y[ii][ 2 ];// + shift_trc_y[ 2 ];
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


	gph[ii]->add_trace( tr1 );
	}





//trace4 [3]
if( multi_trace >= 4 )
	{

	tr1.id = 3;                      //identify which trace this is, helps when traces are push_back'd in unknown order
	tr1.vis = 1;
	tr1.col = color_trc[3];

	tr1.use_pos_y = 0;				//if not -1, use this trace's id as a reference for this val
	tr1.use_scale_y = 0;			//if not -1, use this trace's id as a reference for this val

	tr1.plot_offsy = shift_trc_y[ii][ 3 ];

	tr1.pnt.clear();

	minx = miny = maxx = maxy = 0;

	for( int i = 0; i < vtrc_valx[ii].size(); i++ )
		{
	//printf("y: %f\n", vf[ i ] );

		pnt1.x = vtrc_valx[ii][ i ];
//		pnt1.y = vy4[ i ];
		pnt1.y = vtrc_valy4[ii][ i ];

		if( use_logx )
			{
			pnt1.x = log10( pnt1.x );									//v1.13	
			}

		if( use_logy )
			{
			pnt1.y = log10( pnt1.y );									//v1.13	
			}

		pnt1.y = pnt1.y * scale_trc_y[ii][ 3 ];// + shift_trc_y[ 2 ];
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


	gph[ii]->add_trace( tr1 );
	}






//non trace specific cb
gph[ii]->set_left_click_anywhere_cb( cb_fast_gph_left_click_anywhere, (void*)gph[ii] );
gph[ii]->set_right_click_anywhere_cb( cb_fast_gph_right_click_anywhere, (void*)gph[ii] );
gph[ii]->set_left_click_release_cb( cb_fast_gph_left_click_release, (void*)gph[ii] );
gph[ii]->set_mouseenter_cb( cb_fast_gph_mouseenter, (void*)gph[ii] );
gph[ii]->set_mouseleave_cb( cb_fast_gph_mouseleave, (void*)gph[ii] );


//non specific cb
gph[ii]->set_left_click_cb( 0, cb_fast_gph_left_click, (void*)gph[ii] );
gph[ii]->set_left_click_cb( 1, cb_fast_gph_left_click, (void*)gph[ii] );
gph[ii]->set_left_click_cb( 2, cb_fast_gph_left_click, (void*)gph[ii] );
gph[ii]->set_left_click_cb( 3, cb_fast_gph_left_click, (void*)gph[ii] );

gph[ii]->set_middle_click_cb( 0, cb_fast_gph_middle_click, (void*)gph[ii] );
gph[ii]->set_middle_click_cb( 1, cb_fast_gph_middle_click, (void*)gph[ii] );
gph[ii]->set_middle_click_cb( 2, cb_fast_gph_middle_click, (void*)gph[ii] );
gph[ii]->set_middle_click_cb( 3, cb_fast_gph_middle_click, (void*)gph[ii] );

gph[ii]->set_right_click_cb( 0, cb_fast_gph_right_click, (void*)gph[ii] );
gph[ii]->set_right_click_cb( 1, cb_fast_gph_right_click, (void*)gph[ii] );
gph[ii]->set_left_click_cb( 2, cb_fast_gph_left_click, (void*)gph[ii] );
gph[ii]->set_left_click_cb( 3, cb_fast_gph_left_click, (void*)gph[ii] );

//gph->set_right_click_cb( trace_idx,  cb_fast_gph_right_click, this );
gph[ii]->set_keydown_cb( 0, cb_fast_gph_keydown, (void*)gph[ii] );
gph[ii]->set_keyup_cb( 0, cb_fast_gph_keyup, (void*)gph[ii] );
gph[ii]->set_mousewheel_cb( 0, cb_fast_gph_mousewheel, (void*)gph[ii] );
gph[ii]->set_mousemove_cb( 0, cb_fast_gph_mousemove, (void*)gph[ii] );




int idx = last_sel0_idx[ ii ];											//v1.22

if( idx < 0 ) idx = 0;
gph[ii]->set_selected_sample( last_sel_trc, idx, 0 );					//trigger: left_click_anywhere_cb_p_callback


if( grph_idx >= 0 ) update_fg_user_obj( grph_idx );						//multiple graphs, one trace on each?
else update_fg_user_obj( 0 );											//single graph, multiple tracers

//gph[ii]->center_on_sample( 0, 1000 );

//gph[ii]->render( 0 );
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 



















void fast_mgraph::set_selected_sample( int gph_idx, int trc, int sel_sample_idx )
{

if ( gph_idx >= gph_cnt ) return;
if ( gph_idx < 0 ) return;


gph[gph_idx]->set_selected_sample( trc, sel_sample_idx, 0 );
}





void fast_mgraph::set_selected_sample( int trc, int sel_sample_idx )
{

gph[0]->set_selected_sample( trc, sel_sample_idx, 0 );
}






void fast_mgraph::select_first_sample_on_trc0_if_nothing_is_selected()
{
mgraph *o = gph[0];


int samp_idx;
int trc = o->get_selected_trace_and_sample_indexes( samp_idx );

last_sel_trc = 0;

if( trc == -1 )
	{
	set_selected_sample( last_sel_trc, 0, 0 );
	}
}


//-------------------------------------------------------------------------------





/*

//---- fast_mgraph usage examples ----

fast_mgraph gp(3);											//2 graphs, vert stacked, one trace on each
fast_mgraph gp2;											//1 graph, with 1 or more traces
fast_mgraph gp3( 1, 200, 200, 900, 600, "graph3" );			//1 graph, with 1 or more traces (referenced to trace 1), dimensions and window label specified
fast_mgraph gp4( 1, 250, 230, 900, 600, "graph4" );
fast_mgraph gp5( 1, 300, 260, 900, 600, "graph5" );
fast_mgraph gp6( 1, 350, 290, 900, 600, "graph6" );

gp.use_logx = 1;											//for log10 graph format
gp.use_logy = 1;
gp.disp_vals_undo_logx = 1;
gp.disp_vals_undo_logy =  0;
gp.disp_vals_multiplier_x = 1;
gp.disp_vals_multiplier_y = 20;								//as '20*log10', set this to = 20


void test_fast_mgraph()
{
vector<int> vi1, vi2, vi3, vi4;
vector<float> vf1, vf2, vf3, vf4;
vector<double> vd1, vd2, vd3, vd4;

for(int i = 0; i < 1001; i++ )
	{
	double whtnz =  (double)( RAND_MAX / 2 - rand() ) / (double)( RAND_MAX / 2 );

	float ff = 0.0 + sin( 2.0 * i / 1001.0 * 2.0*M_PI );

	vd1.push_back( i / 1000.0 );			//these are the x vals
	vf1.push_back( i / 1000.0 );
	vi1.push_back( i );

	vd2.push_back( ff );					//these are the y vals
	vf2.push_back( ff );
	vi2.push_back( ff * 32767 );

	ff = 0.0 + sin( 1.0 * i/ 500.0 * 2.0*M_PI ) + 1/3.0*sin( 3.0 * i/ 500.0 * 2.0*M_PI )+ 1/5.0*sin( 5.0 * i/ 500.0 * 2.0*M_PI );
	vd3.push_back( ff * 1.0 );
	vf3.push_back( ff * 1.0 );
	vi3.push_back( ff * 32767 );

	vd4.push_back( ff * 0.8 + whtnz * 0.05 );
	vf4.push_back( ff * 0.8 + whtnz * 0.05 );
	vi4.push_back( (ff * 0.8 + whtnz * 0.05) * 32767  );
	}


int iarr1[16384];
int iarr2[16384];
int iarr3[16384];
int iarr4[16384];
float farr1[16384];
float farr2[16384];
float farr3[16384];
double darr1[16384];
double darr2[16384];
double darr3[16384];
for(int i = 0; i < 16384; i++ )
	{
	double whtnz =  (double)( RAND_MAX / 2 - rand() ) / (double)( RAND_MAX / 2 );

	float ff = 0.0 + sin( 2.0 * i / 16385 * 2.0*M_PI );
	iarr1[ i ] = i;
	iarr2[ i ] = ff * 32767;

	ff = 0.0 + sin(  1.0 * i/ 16385 * 2.0*M_PI ) + 1/3.0*sin( 3.0 * i/ 16385 * 2.0*M_PI ) + 1/5.0*sin( 5.0 * i/ 16385 * 2.0*M_PI ) + 1/7.0*sin( 7.0 * i/ 16385 * 2.0*M_PI );

	iarr3[ i ] = ff * 32767;

	ff += whtnz * 0.05;
	iarr4[ i ] = ff * 16383;
	}


gp.plotxy( 0, vd1, vd2 );						//vd1 is x
gp.plot( 1, vd2 );
gp.plot( 2, vd3 );

gp2.plot( 0, vd2 );
gp3.plotxy( vf1, vf2, vf3, vf4, "drkr" );

gp4.scale_y( 1.0, 0.5, 0.3 );												//scale for 3 traces, NOTE: values are scaled BEFORE being fed to mgraph obj, so sample values are scale
gp4.shift_y( 0.0, -0.5, -0.7 );												//y trace shift for 3 traces
gp4.plotxy( vf1, vf2, vf3, vf4, "y", "g", "cy", "drkgry" );					//3 traces cols, bkgd col, see defined colours: 'user_col '

gp5.scale_y( 0.3, 0.2, 0.1 );													//scale for 1st 2 traces, 3rd trace unchanged
gp5.plotxy( vf1, vf2, vf3, vf4, "y", "g", "cy", "drkgry", "drkr", "g", "trace 1", "", "trace 3" );	//3 traces cols, bkgd col, axis col, text col, and trace colour coded labels, see defined colours: 'user_col'

gp6.font_type ( 7 );
gp6.font_size( 8 );															//smaller font example
gp6.set_sig_dig( 3 );
gp6.plotxy( iarr1, iarr2, iarr3, iarr4, 16384 );							//3 traces cols using int arrays
}
//------------------------------------
*/


//-----------------------------------------------------------------------------

