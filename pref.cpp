//pref.cpp
//v1.01		23-08-10
//v1.02		19-09-10				//added dynamic user var update option, and 
									//save/recall of pref window pos
//v1.03		03-11-11				//added removal of CRs/LFs in Load() for cnInputPref controls
									//added GCCol control
									 
//v1.04		05-11-11				//added ability to group radio buttons via 'sc.radio_group_name'
									//this allows radio button groups to exist across rows, where only one radio button will stay enabled
									//you must set a string with a radio_group_name and
									//also set a user callback 'sc.cb' (that need do nothing), as the callback internal to this code
									//will toggle off other radio buttons with a matching radio_group_name string
									//see code in void cb_pref_usr_callback()


//v1.05     20-09-2014              //added uniq_id to struct sControl to allow easy identification of control a other than row and column
                                    //change FL_Input to fl_input_change, so key presses trigger callback
                                    //added GCLed
                                    //added: label_align, fixed: label_type
                                    //added: find_control_from_its_id(..)

//v1.06		08-feb-2017				//removed 'if( bDonePaste == 0)' code block in 'fl_input_change::handle()' which called 'fl_paste()', as
									//it was causing an infinite loop and finally a crash (when pasting)

//v1.07		15-jul-2018				//used 'copy_label()' to stop string corruption

//v1.08		06-aug-2018				//modified usage of tooltip, to stop string corruption, now uses passed string, see: o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08

//v1.09		26-feb-2019				//in 'fl_input_change::handle( int e )' added call base function first to process poss changes,
									//added: 'set_callback_on_ok()'

//v1.10		04-nov-2019				//fixed crashing bug with 'cnInputDoublePref' ctrl, in 'CreateRow()' was not setting 'fl_input_change::row' and 'fl_input_change::clmn'
//v1.11		16-jan-2021				//added: 'refresh_controls_from_user_vars()', call this in 'Save()' to ensure externally modified user vars are saved
//v1.12		27-nov-2021				//change: 'int Show(bool modal)' to 'void Show(bool modal)', as was not returing a value, this would cause a crash with 'O3' optimization
//v1.13		02-dec-2021				//moded to get to the ultimate parent 'PrefWnd', was getting a crash when 'fl_input_change::handle() FL_KEYDOWN' was calling 'pref_update_user_vars()'
 

#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_File_Chooser.H>


#ifndef compile_for_windows

#endif


#include "pref.h"
#include "GCProfile.h"

//#define compile_for_windows		//!!!!!!!!!!! uncomment for a dos/windows compile

//function prototypes
void pref_update_user_vars( PrefWnd *o, bool all);



//-----------------------------------------------------------------------------
fl_input_change::fl_input_change( int xx, int yy, int ww, int hh, const char *label ) : Fl_Input( xx, yy, ww, hh, label )
{
ctrl_key = 0;
left_button = 0;

}







int fl_input_change::handle( int e )
{
string s1;

bool need_redraw = 0;
bool dont_pass_on = 0;
bool bDonePaste = 0;

Fl_Input::handle(e);												//v1.09, call base function to process poss changes
 

//printf( "fl_input_change::handle()\n" );

if ( e == FL_MOVE )	
	{
//	mouse_x = Fl::event_x();
//	mouse_y = Fl::event_y();

	need_redraw = 0;
	dont_pass_on = 0;
	}


if ( e == FL_PUSH )	
	{

//	mouse_x = Fl::event_x() - x();
//	mouse_y = Fl::event_y();
	need_redraw = 0;
	dont_pass_on = 0;
	}


if ( e == FL_RELEASE )	
	{
//	string s1;
//	mouse_x = Fl::event_x();
//	mouse_y = Fl::event_y();
	
//printf("xx= %g, yy= %g\n", xx, yy );
	need_redraw = 0;
	dont_pass_on = 0;
	}

if ( e == FL_ENTER )	
	{
//	parent()->hide();
//	parent()->show();
//	take_focus();
//    do_callback();
//	dont_pass_on = 1;
//	exit(0);
	need_redraw = 0;
	dont_pass_on = 0;
	}



if ( ( e == FL_KEYDOWN ) || ( e == FL_SHORTCUT ) )					//key pressed?
	{
	int key = Fl::event_key();

/*
	if( ( key >= '0' ) && ( key <= '9' ) || ( key == '.' ) ) 
		{
		key_in_str += key;
		fi_scan_keyinfreq->value( key_in_str.c_str() );
		dont_pass_on = 1;
		}

	if( key == FL_Delete )
		{
		key_in_str = "";
		fi_scan_keyinfreq->value( key_in_str.c_str() );
		dont_pass_on = 1;
		}

	if( key == FL_BackSpace )
		{
		int len = key_in_str.length();
		if( len > 0 ) key_in_str = key_in_str.substr( 0, len - 1 );
		fi_scan_keyinfreq->value( key_in_str.c_str() );
		dont_pass_on = 1;
		}

	if( key == FL_Enter )
		{
		key_in_cnt = 0;
		keyin_ctrl_to_tuner_i2s( );
		dont_pass_on = 1;
		}

	if( key == FL_Left )
		{
		s1 = fi_scan_step->value( );
		double dstep;
		sscanf( s1.c_str(), "%lf", &dstep );

		s1 = fi_scan_keyinfreq->value( );
		double dval;
		sscanf( s1.c_str(), "%lf", &dval );

		dval -= 10.0 * dstep / 1e6;

		strpf( s1, "%.3f", dval );
		fi_scan_keyinfreq->value( s1.c_str() );
		key_in_str = s1;

		keyin_ctrl_to_tuner_i2s( );
		dont_pass_on = 1;
		}


	if( key == FL_Right )
		{
		s1 = fi_scan_step->value( );
		double dstep;
		sscanf( s1.c_str(), "%lf", &dstep );

		s1 = fi_scan_keyinfreq->value( );
		double dval;
		sscanf( s1.c_str(), "%lf", &dval );

		dval += 10.0 * dstep / 1e6;

		strpf( s1, "%.3f", dval );
		fi_scan_keyinfreq->value( s1.c_str() );
		key_in_str = s1;

		keyin_ctrl_to_tuner_i2s( );
		dont_pass_on = 1;
		}


	if( key == FL_Up )
		{
		s1 = fi_scan_step->value( );
		double dstep;
		sscanf( s1.c_str(), "%lf", &dstep );

		s1 = fi_scan_keyinfreq->value( );
		double dval;
		sscanf( s1.c_str(), "%lf", &dval );

		dval += dstep / 1e6;

		strpf( s1, "%.3f", dval );
		fi_scan_keyinfreq->value( s1.c_str() );
		key_in_str = s1;

		keyin_ctrl_to_tuner_i2s( );
		dont_pass_on = 1;
		}




	if( key == FL_Down )
		{
		s1 = fi_scan_step->value( );
		double dstep;
		sscanf( s1.c_str(), "%lf", &dstep );

		s1 = fi_scan_keyinfreq->value( );
		double dval;
		sscanf( s1.c_str(), "%lf", &dval );

		dval -=  dstep / 1e6;

		strpf( s1, "%.3f", dval );
		fi_scan_keyinfreq->value( s1.c_str() );
		key_in_str = s1;

		keyin_ctrl_to_tuner_i2s( );
		dont_pass_on = 1;
		}




	if( key == FL_Page_Up )
		{
		s1 = fi_scan_step->value( );
		double dstep;
		sscanf( s1.c_str(), "%lf", &dstep );

		s1 = fi_scan_keyinfreq->value( );
		double dval;
		sscanf( s1.c_str(), "%lf", &dval );

		dval +=  100 * dstep / 1e6;

		strpf( s1, "%.3f", dval );
		fi_scan_keyinfreq->value( s1.c_str() );
		key_in_str = s1;

		keyin_ctrl_to_tuner_i2s( );
		dont_pass_on = 1;
		}


	if( key == FL_Page_Down )
		{
		s1 = fi_scan_step->value( );
		double dstep;
		sscanf( s1.c_str(), "%lf", &dstep );

		s1 = fi_scan_keyinfreq->value( );
		double dval;
		sscanf( s1.c_str(), "%lf", &dval );

		dval -=  100 * dstep / 1e6;

		strpf( s1, "%.3f", dval );
		fi_scan_keyinfreq->value( s1.c_str() );
		key_in_str = s1;

		keyin_ctrl_to_tuner_i2s( );
		dont_pass_on = 1;
		}
*/

//	PrefWnd* o = (PrefWnd*) parent();

	Fl_Group* gp = (PrefWnd*) parent();									//v1.13, find ultimate parent 'PrefWnd'
	Fl_Pack* pk = (Fl_Pack*)gp->parent();
	Fl_Scroll* sc = (Fl_Scroll*)pk->parent();

	PrefWnd* o = (PrefWnd*) sc->parent();

//printf("PrefWnd row: %d, clmn: %d, dynamic: %d\n", row, clmn, o->ctrl_list[row][clmn].dynamic );

	if( o->ctrl_list[row][clmn].dynamic )
		{
		pref_update_user_vars( o, 0 );
		do_callback();	
		}

//bool PrefWnd::do_callback_if_dynamic()
//{
//}


//	printf("pref.cpp fl_input_change:::handle() - row: %d, clmn: %d\n", row, clmn );

	
//    do_callback_if_dynamic();
	dont_pass_on = 1;								//already called base function above
	need_redraw = 0;
	}



if ( e == FL_PASTE )	//needed below code because on drag release the first FL_PASTE call does not have valid text as yet,
	{					//possibly because of a delay in X windows, so have used Fl:paste(..) to send another paste event and 
	string s = Fl::event_text();
	int len = Fl::event_length();

/*
	if( bDonePaste == 0)	//this seems to work ok (does not seem to happen in Windows)
		{
		Fl::paste( *this, 0 );					//passing second var as 0 uses currently selected text, (not prev cut text)
//		printf("\nDropped1\n" );
		bDonePaste = 1;
		}
	else{
		bDonePaste = 0;
		string s = Fl::event_text();
		int len = Fl::event_length();
		printf("\nDropped Len=%d, Str=%s\n", len, s.c_str() );
		if( len )								//anything dropped/pasted?
			{
			dropped_str = s;
            dont_pass_on = 0;
			need_redraw = 0;
			}
		}
*/
    dont_pass_on = 0;
    need_redraw = 0;
	}


if (e == FL_DND_DRAG)
	{
//	printf("\nDrag\n");
//	return 1;
    dont_pass_on = 0;
    need_redraw = 0;
	}

if (e == FL_DND_ENTER)
	{
//	printf("\nDrag Enter\n");
    dont_pass_on = 0;
    need_redraw = 0;
	}

if (e == FL_DND_RELEASE)
	{
//	printf("\nDrag Release\n");
    dont_pass_on = 0;
    need_redraw = 0;
	}




if ( e == FL_PUSH )
	{
	if( Fl::event_button() == 1 )
		{
		left_button = 1;
		}
    dont_pass_on = 0;
    need_redraw = 0;
	}

if ( e == FL_RELEASE )
	{
	
	if(Fl::event_button()==1)
		{
		left_button = 0;
		}
    dont_pass_on = 0;
    need_redraw = 0;
	}


/* v1.09
if ( ( e == FL_KEYDOWN ) || ( e == FL_SHORTCUT ) )					//key pressed?
	{
	int key = Fl::event_key();
	
	if( ( key == FL_Control_L ) || (  key == FL_Control_R ) ) ctrl_key = 1;
	
    dont_pass_on = 1;
    need_redraw = 0;
	}


if ( e == FL_KEYUP )												//key release?
	{
	int key = Fl::event_key();
	
	if( ( key == FL_Control_L ) || ( key == FL_Control_R ) ) ctrl_key = 0;

    dont_pass_on = 0;
    need_redraw = 0;
	}
*/


if ( e == FL_MOUSEWHEEL )
	{
	int dir = -Fl::event_dy();
//	printf("wheel %g\n", dval[ 0 ] );

	need_redraw = 0;
	dont_pass_on = 0;
	}

if ( need_redraw ) redraw();

if( dont_pass_on ) return 1;

return Fl_Input::handle( e );
}


//-----------------------------------------------------------------------------




//cycle and load all control states into user variables if 'all' is set
//else only up user vars that have 'dynamic' set
void pref_update_user_vars( PrefWnd *o, bool all )
{
int d;
double dble;
	printf("pref_update_user_vars() %x\n", o );

//gather and store each control's state
for(int j=0;j<o->row_count;j++)									//cycle all rows
	{
	for(int i=0;i<cnMaxCtrl;i++)								//cycle this row's controls
		{
	printf("pref_update_user_vars() - ij %d %d\n", i, j );
		
		if(o->ctrl_ptrs[j][i]!=0)								//control exists?			
			{

			int type=o->ctrl_list[j][i].type;					//get control type

			if(type==cnStaticTextPref)
				{
				}

			if( (type==cnInputPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				fl_input_change *con=(fl_input_change*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].sretval!=-1) *o->ctrl_list[j][i].sretval = con->value();
				}

			if( (type==cnInputIntPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				fl_input_change *con=(fl_input_change*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					sscanf(con->value(),"%d",&d);
					*o->ctrl_list[j][i].iretval = d;
					}
				}

			if( (type==cnInputHexPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				fl_input_change *con=(fl_input_change*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					sscanf(con->value(),"%x",&d);
					*o->ctrl_list[j][i].iretval = d;
					}
				}

			if( (type==cnInputDoublePref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				fl_input_change *con=(fl_input_change*)o->ctrl_ptrs[j][i];
				if(o->ctrl_list[j][i].dretval!=(double*)-1)
					{
					sscanf(con->value(),"%lf",&dble);
					*o->ctrl_list[j][i].dretval = dble;
					}
				}

			if( (type==cnEditHistPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				Fl_Input *con=(Fl_Input*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].sretval!=-1) *o->ctrl_list[j][i].sretval = con->value();
				}

			if( (type==cnButtonPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				Fl_Button *con=(Fl_Button*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1) *o->ctrl_list[j][i].iretval = con->value();
				}

			if( (type==cnLightButtonPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				Fl_Light_Button *con=(Fl_Light_Button*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1) *o->ctrl_list[j][i].iretval = con->value();
				}

			if( (type==cnRoundButtonPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				Fl_Round_Button *con=(Fl_Round_Button*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1) *o->ctrl_list[j][i].iretval = con->value();
				}

			if( (type==cnRadioButtonPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				Fl_Button *con=(Fl_Button*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1) *o->ctrl_list[j][i].iretval = con->value();
				}

			if( (type==cnToggleButtonPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				Fl_Button *con=(Fl_Button*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1) *o->ctrl_list[j][i].iretval = con->value();
				}

			if( (type==cnCheckPref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				Fl_Check_Button *con=(Fl_Check_Button*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1) *o->ctrl_list[j][i].iretval = con->value();
				}

			if( (type==cnMenuChoicePref) && (all | o->ctrl_list[j][i].dynamic) )
				{
				Fl_Choice *con=(Fl_Choice*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].iretval!=-1) *o->ctrl_list[j][i].iretval = con->value();

				}

			if( ( type == cnGCColColour) && ( all | o->ctrl_list[j][i].dynamic ) )
				{
				GCCol *con=(GCCol*)o->ctrl_ptrs[j][i];
				if((int)o->ctrl_list[j][i].sretval!=-1) *o->ctrl_list[j][i].sretval = con->value();
				}

			if( ( type == cnGCLed) && ( all | o->ctrl_list[j][i].dynamic ) )
				{
				GCLed *con = (GCLed*) o->ctrl_ptrs[ j ][ i ];
//				if( (int) o->ctrl_list[ j ][ i ].siretval != -1 ) *o->ctrl_list[j][i].iretval = con->GetColIndex();	//v1.10
				if( (int) o->ctrl_list[ j ][ i ].iretval != -1 ) *o->ctrl_list[j][i].iretval = con->GetColIndex();	//v1.11
				}

			}
		}
	}

}




void cb_pref_usr_callback(Fl_Widget* w, void* v)
{
bool radio_group_found = 0;
int radio_group_j = 0;
int radio_group_i = 0;

string radio_group_name = "";

Fl_Group*  grp =(Fl_Group*)     w->parent();		//track parents from control widget received
Fl_Pack*   pk  =(Fl_Pack*)      grp->parent();
Fl_Scroll* scl =(Fl_Scroll*)    pk->parent();

PrefWnd* o=(PrefWnd*)scl->parent();					//this this the Pref Wnd obj that hold everyting

pref_update_user_vars( o, 0 );							//update any vars that are assigned 'dynamic`

//find control pos from PrefWnd's 'ctrl_list' list
for(int j=0;j<o->row_count;j++)							//cycle all rows
	{
	for(int i=0;i<cnMaxCtrl;i++)						//cycle this row's controls
		{
		if(o->ctrl_ptrs[j][i]==w)						//control found in ctrl_list?
			{
			if(o->ctrl_list[j][i].cb!=(void*)-1)		//user defined a callback address?
				{
//				printf("\nPref control was pressed j=%d, i=%d\n",j,i);

				void (*cb_user)(void*,int,int);			//define a function ptr for calling
				cb_user=o->ctrl_list[j][i].cb;			//retrieve user callback function ptr
				cb_user((void*)o,j,i);			//call user callback with variables: (PrefWnd*,row,ctrl)

				if( o->ctrl_list[ j ][ i ].radio_group_name.length() != 0  )
					{
					radio_group_found = 1;			//flag we need to find any other radio button with
													//matching radio_group_name and toggle them off
					radio_group_j = j;				//remember this radio button is to stay on 		
					radio_group_i = i;
					radio_group_name = o->ctrl_list[ j ][ i ].radio_group_name;
//					printf("got radio group name: %s\n", o->ctrl_list[ j ][ i ].radio_group_name.c_str() );
					}
				}
			
			}
		}
	}


//need to toggle off all other radio buttons with matching radio_group_name? 
if( radio_group_found )	
	{
	for(int j = 0;j < o->row_count; j++ )					//cycle all rows
		{
		for( int i=0; i < cnMaxCtrl; i++ )						//cycle this row's controls
			{
			
			//skip the pressed radio button as it is to stay on
			if( j == radio_group_j )
				{
				if( i == radio_group_i )
					{
					continue;
					}	
				}
			
			//check for matching radio_group_name, and toggle off if a radio button
			if( o->ctrl_list[ j ][ i ].radio_group_name.compare( radio_group_name ) == 0 )
				{
				if( o->ctrl_list[ j ][ i ].type == cnRadioButtonPref )
					{
					Fl_Round_Button* oo = (Fl_Round_Button*)o->ctrl_ptrs[ j ][ i ];
					oo->value( 0 );
					}
				}
			}
		}
	}

}






//when user hots ok to close pref wnd
void PrefWnd::set_callback_on_ok( void (*p_cb)( void * ) )
{
p_cb_on_ok = p_cb;
}





void PrefWnd::ClearToDefCtrl()
{
//load up default values
sc.type=cnNone;
sc.x=0;
sc.y=0;
sc.w=60;
sc.h=20;
sc.label="";
sc.label_type = FL_NORMAL_LABEL;	//FL_NORMAL_LABEL, FL_SHADOW_LABEL - for more see fltk manual
sc.label_align = FL_ALIGN_CENTER;	//FL_ALIGN_CENTER,FL_ALIGN_LEFT,FL_ALIGN_RIGHT
sc.tooltip="";						//tool tip
sc.options="";						//menu button drop down options
sc.labelfont=-1;					//-1 means use fltk default
sc.labelsize=-1;					//-1 means use fltk default
sc.ctrl_style = 0;                  //e.g: for GCLed can be: cn_gcled_style_square, cn_gcled_style_round
sc.textfont=-1;
sc.textsize=-1;
sc.section="";						//ini file Section
sc.key="";							//ini file Key
sc.keypostfix=-1;					//ini file Key post fix
sc.def="";							//default to use if ini value not avail
sc.iretval=(int*)-1;				//address of int to be modified, -1 means none
sc.dretval=(double*)-1;				//address of double to be modified, -1 means none
sc.sretval=(string*)-1;				//address of string to be modified, -1 means none
sc.dynamic=0;						//dynamic change of user: int, double or string  retval

sc.radio_group_name = "";
sc.cb=0;							//address of a callback if any, 0 means none
}






void PrefWnd::Save(GCProfile &p)										//v1.11
{
string section,key,data;
int d;
string newkey;


refresh_controls_from_user_vars();										//v1.11

//save pref window pos if req
if( (sSectionPos.length()>0) && (sKeyPos.length()>0) )
	{
	string sdata;
	strpf(sdata,"x=%d,y=%d",x(),y());

	p.WritePrivateProfileStr(sSectionPos,sKeyPos,sdata);
	}


//check each control's ini params and save control's state to file
for(int j=0;j<row_count;j++)									//cycle all rows
	{
	for(int i=0;i<cnMaxCtrl;i++)								//cycle this row's controls
		{
		if(ctrl_ptrs[j][i]!=0)									//control exists?			
			{
			section=ctrl_list[j][i].section;
			key=ctrl_list[j][i].key;
			int type=ctrl_list[j][i].type;						//get control type
			int postfix=ctrl_list[j][i].keypostfix;
			if(postfix!=-1)										//postfix number avail?
				{
				strpf(newkey,"%s_%d",key.c_str(),postfix);
				}
			else newkey=key;

			if( (section.length()==0) || (key.length()==0) )	//missing section or key?
				{
				continue;
				}


			if(type==cnStaticTextPref)
				{
				}

			if(type==cnInputPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnInputIntPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnInputHexPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnInputDoublePref)
				{
				Fl_Input *con=(Fl_Input*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnEditHistPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnLightButtonPref)
				{
				Fl_Light_Button *con=(Fl_Light_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnRoundButtonPref)
				{
				Fl_Round_Button *con=(Fl_Round_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnRadioButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnToggleButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnCheckPref)
				{
				Fl_Check_Button *con=(Fl_Check_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnMenuChoicePref)
				{
				Fl_Choice *con=(Fl_Choice*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if( type == cnGCColColour )
				{
				GCCol *con=(GCCol*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if( type == cnGCLed )
				{
				GCLed *con = (GCLed*) ctrl_ptrs[ j ][ i ];
				strpf( data, "%d", con->GetColIndex() );
				}

			p.WritePrivateProfileStr(section,newkey,data);
			}
		}
	}

}











/*
void PrefWnd::Save(GCProfile &p)										v1.10
{
string section,key,data;
int d;
string newkey;


//save pref window pos if req
if( (sSectionPos.length()>0) && (sKeyPos.length()>0) )
	{
	string sdata;
	strpf(sdata,"x=%d,y=%d",x(),y());

	p.WritePrivateProfileStr(sSectionPos,sKeyPos,sdata);
	}


//check each control's ini params and save control's state to file
for(int j=0;j<row_count;j++)									//cycle all rows
	{
	for(int i=0;i<cnMaxCtrl;i++)								//cycle this row's controls
		{
		if(ctrl_ptrs[j][i]!=0)									//control exists?			
			{
			section=ctrl_list[j][i].section;
			key=ctrl_list[j][i].key;
			int type=ctrl_list[j][i].type;						//get control type
			int postfix=ctrl_list[j][i].keypostfix;
			if(postfix!=-1)										//postfix number avail?
				{
				strpf(newkey,"%s_%d",key.c_str(),postfix);
				}
			else newkey=key;

			if( (section.length()==0) || (key.length()==0) )	//missing section or key?
				{
				continue;
				}


			if(type==cnStaticTextPref)
				{
				}

			if(type==cnInputPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnInputIntPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnInputHexPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnInputDoublePref)
				{
				Fl_Input *con=(Fl_Input*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnEditHistPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if(type==cnButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnLightButtonPref)
				{
				Fl_Light_Button *con=(Fl_Light_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnRoundButtonPref)
				{
				Fl_Round_Button *con=(Fl_Round_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnRadioButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnToggleButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnCheckPref)
				{
				Fl_Check_Button *con=(Fl_Check_Button*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if(type==cnMenuChoicePref)
				{
				Fl_Choice *con=(Fl_Choice*)ctrl_ptrs[j][i];
				d=con->value();
				strpf(data,"%d",con->value());
				}

			if( type == cnGCColColour )
				{
				GCCol *con=(GCCol*)ctrl_ptrs[j][i];
				strpf(data,"%s",con->value());
				}

			if( type == cnGCLed )
				{
				GCLed *con = (GCLed*) ctrl_ptrs[ j ][ i ];
				strpf( data, "%d", con->GetColIndex() );
				}

			p.WritePrivateProfileStr(section,newkey,data);
			}
		}
	}

}
*/









void PrefWnd::Load(GCProfile &p)
{
string s1, section,key,data;
int d,x,y;
double dble;
string newkey;
mystr m1;


//load pref window pos if req
if( (sSectionPos.length()>0) && (sKeyPos.length()>0) )
	{
	string s1, sdata;

	p.GetPrivateProfileStr(sSectionPos,sKeyPos,"x=100,y=100",&sdata);
	
	m1=sdata;
	
	m1.ExtractParamInt("x=",x);
	m1.ExtractParamInt("y=",y);
	position(x,y);	
	}



//get each control's ini file params and set control's state
for(int j=0;j<row_count;j++)									//cycle all rows
	{
	for(int i=0;i<cnMaxCtrl;i++)								//cycle this row's controls
		{
		if(ctrl_ptrs[j][i]!=0)									//control exists?			
			{
			section=ctrl_list[j][i].section;
			key=ctrl_list[j][i].key;
			int type=ctrl_list[j][i].type;						//get control type
			int postfix=ctrl_list[j][i].keypostfix;
			string def=ctrl_list[j][i].def;
			
			if(postfix!=-1)										//postfix number avail?
				{
				strpf(newkey,"%s_%d",key.c_str(),postfix);
				}
			else newkey=key;

			if( (section.length()==0) || (key.length()==0) )	//missing section or key?
				{
				continue;
				}

			p.GetPrivateProfileStr(section,newkey,def.c_str(),&data);


			if(type==cnStaticTextPref)
				{
				}

			if(type==cnInputPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				
				m1 = data;
				m1.FindReplace( s1, "\r", "",0 );		//remove all occurrances of CR
				m1 = s1;
				m1.FindReplace( s1, "\n", "",0);		//remove all occurrances of LF

				con->value( s1.c_str());
				if((int)ctrl_list[j][i].sretval!=-1) *ctrl_list[j][i].sretval = s1;
				}

			if(type==cnInputIntPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				con->value(data.c_str());
				sscanf(data.c_str(),"%d",&d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if(type==cnInputHexPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				con->value(data.c_str());
				sscanf(data.c_str(),"%x",&d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if(type==cnInputDoublePref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				con->value(data.c_str());
				sscanf(data.c_str(),"%lf",&dble);
				if(ctrl_list[j][i].dretval!=(double*)-1) *ctrl_list[j][i].dretval = dble;
				}

			if(type==cnEditHistPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				con->value(data.c_str());
				if((int)ctrl_list[j][i].sretval!=-1) *ctrl_list[j][i].sretval = data;
				}

			if(type==cnButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				sscanf(data.c_str(),"%d",&d);
				con->value(d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if(type==cnLightButtonPref)
				{
				Fl_Light_Button *con=(Fl_Light_Button*)ctrl_ptrs[j][i];
				sscanf(data.c_str(),"%d",&d);
				con->value(d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if(type==cnRoundButtonPref)
				{
				Fl_Round_Button *con=(Fl_Round_Button*)ctrl_ptrs[j][i];
				sscanf(data.c_str(),"%d",&d);
				con->value(d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if(type==cnRadioButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				sscanf(data.c_str(),"%d",&d);
				con->value(d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if(type==cnToggleButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				sscanf(data.c_str(),"%d",&d);
				con->value(d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if(type==cnCheckPref)
				{
				Fl_Check_Button *con=(Fl_Check_Button*)ctrl_ptrs[j][i];
				sscanf(data.c_str(),"%d",&d);
				con->value(d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if(type==cnMenuChoicePref)
				{
				Fl_Choice *con=(Fl_Choice*)ctrl_ptrs[j][i];
				sscanf(data.c_str(),"%d",&d);
				con->value(d);
				if((int)ctrl_list[j][i].iretval!=-1) *ctrl_list[j][i].iretval = d;
				}

			if( type == cnGCColColour )
				{
				GCCol *con=(GCCol*)ctrl_ptrs[j][i];

				m1 = data;
				m1.FindReplace( s1, "\r", "",0 );		//remove all occurrances of CR
				m1 = s1;
				m1.FindReplace( s1, "\n", "",0);		//remove all occurrances of LF

				con->value( s1.c_str());
				if((int)ctrl_list[j][i].sretval!=-1) *ctrl_list[j][i].sretval = s1;
				}

			if( type == cnGCLed )
				{
				GCLed *con = (GCLed*) ctrl_ptrs[ j ][ i ];
				sscanf( data.c_str(), "%d", &d );
				con->ChangeCol( d );
				if( (int)ctrl_list[j][i].iretval != -1 ) *ctrl_list[ j ][ i ].iretval = d;
				}

			}
		}
	}
}







//show pref window
//note: will only load control with user ok'd vals if user ptrs were defined,
//if no user ptrs supplied, control will show last modified state even if not ok'd
void PrefWnd::Show(bool modal)											//v1.12 (now 'void' was 'int')
{
string s;
int d;
double dble;



//cycle all controls and set their state to user value,
for(int j=0;j<row_count;j++)									//cycle all rows
	{
	for(int i=0;i<cnMaxCtrl;i++)								//cycle this row's controls
		{
		if(ctrl_ptrs[j][i]!=0)									//control exists?			
			{

			int type=ctrl_list[j][i].type;						//get control type

			if(type==cnStaticTextPref)
				{
				}

			if(type==cnInputPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].sretval!=-1) con->value((*ctrl_list[j][i].sretval).c_str());
				}

			if(type==cnInputIntPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1)
					{
					d=*ctrl_list[j][i].iretval;
					strpf(s,"%d",d);
					con->value(s.c_str());
					}
				}

			if(type==cnInputHexPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1)
					{
					d=*ctrl_list[j][i].iretval;
					strpf(s,"%x",d);
					con->value(s.c_str());
					}
				}

			if(type==cnInputDoublePref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				if(ctrl_list[j][i].dretval!=(double*)-1)
					{
					dble=*ctrl_list[j][i].dretval;
					strpf(s,"%.10g",dble);
					con->value(s.c_str());
					}
				}

			if(type==cnEditHistPref)
				{
				fl_input_change *con=(fl_input_change*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].sretval!=-1) con->value((*ctrl_list[j][i].sretval).c_str());
				}

			if(type==cnButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1) con->value(*ctrl_list[j][i].iretval);
				}

			if(type==cnLightButtonPref)
				{
				Fl_Light_Button *con=(Fl_Light_Button*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1) con->value(*ctrl_list[j][i].iretval);
				}

			if(type==cnRoundButtonPref)
				{
				Fl_Round_Button *con=(Fl_Round_Button*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1) con->value(*ctrl_list[j][i].iretval);
				}

			if(type==cnRadioButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1) con->value(*ctrl_list[j][i].iretval);
				}

			if(type==cnToggleButtonPref)
				{
				Fl_Button *con=(Fl_Button*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1) con->value(*ctrl_list[j][i].iretval);
				}

			if(type==cnCheckPref)
				{
				Fl_Check_Button *con=(Fl_Check_Button*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1) con->value(*ctrl_list[j][i].iretval);
				}

			if(type==cnMenuChoicePref)
				{
				Fl_Choice *con=(Fl_Choice*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].iretval!=-1) con->value(*ctrl_list[j][i].iretval);
				}


			if( type == cnGCColColour )
				{
				GCCol *con=(GCCol*)ctrl_ptrs[j][i];
				if((int)ctrl_list[j][i].sretval!=-1) con->value((*ctrl_list[j][i].sretval).c_str());
				}

			if( type == cnGCLed )
				{
				GCLed *con = (GCLed*) ctrl_ptrs[ j ][ i ];
				if( (int) ctrl_list[j][i].iretval !=-1 ) con->ChangeCol( *ctrl_list[j][i].iretval );
				}

			}
		}
	}


if(modal)set_modal();
show();
return 1;

}






void PrefWnd::End()
{
pck->end();
scroll->end();
end();
}






//create a group of controls for adding to a pack
void PrefWnd::CreateRow(int height)
{
void *ctrl_wnd;
int ivalue;
double dvalue;
string svalue;

if(row_count>=cnMaxRow) return;
if(ctrl_count<=0) return;

pck->begin();

entry_wnd[entry_count]=new Fl_Group(0,0,this->w(),height,"");	//make entry window to hold controls for entry
Fl_Group *wnd=entry_wnd[entry_count];


//create controls
for(int i=0;i<ctrl_count;i++)						//cycle this entries control types
	{
	int x=ctrl_list[row_count][i].x;
	int y=ctrl_list[row_count][i].y;
	int w=ctrl_list[row_count][i].w;
	int h=ctrl_list[row_count][i].h;
	if(h<=0) h=1;									//place some limit on control height
	int label_type=ctrl_list[row_count][i].label_type;
	int label_align = ctrl_list[row_count][i].label_align;
	
	int type=ctrl_list[row_count][i].type;			//get control type
	string label=ctrl_list[row_count][i].label;
	string tooltip=ctrl_list[row_count][i].tooltip;
	string options=ctrl_list[row_count][i].options;
	int lfont=ctrl_list[row_count][i].labelfont;
	int lsize=ctrl_list[row_count][i].labelsize;
	int tfont=ctrl_list[row_count][i].textfont;
	int tsize=ctrl_list[row_count][i].textsize;

	int ctrl_style = ctrl_list[row_count][i].ctrl_style;

	void (*cb)(void*,int,int);
	cb=ctrl_list[row_count][i].cb;						//get callback if any

	if((int)ctrl_list[row_count][i].iretval!=-1)		//user int ptr avail?
		{
		ivalue=*ctrl_list[row_count][i].iretval;
		}
	else ivalue=0;


	if((int)ctrl_list[row_count][i].dretval!=-1)		//user 'double' ptr avail?
		{
		dvalue=*ctrl_list[row_count][i].dretval;
		}
	else dvalue=0;

	if((int)ctrl_list[row_count][i].sretval!=-1)		//user string ptr avail?
		{
		svalue=*ctrl_list[row_count][i].sretval;
		}
	else svalue="";



	if(type==cnNone)
		{
		}

	if(type==cnStaticTextPref)
		{
		Fl_Box *o;
		o=new Fl_Box(x,wnd->y()+y,w,h,"");					//v1.07
		o->copy_label( label.c_str() );						//v1.07
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1)o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnInputPref)
		{
		fl_input_change *o;
		o=new fl_input_change(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		o->row = row_count;									//v1.09
		o->clmn = i;
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
		if(tfont!=-1) o->textfont(tfont);
		if(tsize!=-1) o->textsize(tsize);
//		o->value(svalue.c_str());
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnInputIntPref)
		{
		fl_input_change *o;
		o=new fl_input_change(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		o->row = row_count;									//v1.09
		o->clmn = i;
		o->when(FL_WHEN_NEVER);								//v1.09
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
		if(tfont!=-1) o->textfont(tfont);
		if(tsize!=-1) o->textsize(tsize);
//		o->value(svalue.c_str());
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnInputHexPref)
		{
		fl_input_change *o;
		o=new fl_input_change(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		ctrl_ptrs[row_count][i]=o;
		o->row = row_count;									//v1.09
		o->clmn = i;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
		if(tfont!=-1) o->textfont(tfont);
		if(tsize!=-1) o->textsize(tsize);
//		o->value(svalue.c_str());
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnInputDoublePref)
		{
		fl_input_change *o;
		o=new fl_input_change(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		o->row = row_count;									//v1.10
		o->clmn = i;										//v1.10
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
		if(tfont!=-1) o->textfont(tfont);
		if(tsize!=-1) o->textsize(tsize);
//		o->value(svalue.c_str());
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnEditHistPref)
		{
		fl_input_change *o;
		o=new fl_input_change(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		o->row = row_count;									//v1.09
		o->clmn = i;
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
		if(tfont!=-1) o->textfont(tfont);
		if(tsize!=-1) o->textsize(tsize);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnButtonPref)
		{
		Fl_Button *o;
		o=new Fl_Button(x,wnd->y()+y,w,h,"");				//v1.07
		o->copy_label( label.c_str() );						//v1.07
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
//		o->value(ivalue);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnLightButtonPref)
		{
		Fl_Light_Button *o;
		o=new Fl_Light_Button(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
//		o->value(ivalue);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnRoundButtonPref)
		{
		Fl_Round_Button *o;
		o=new Fl_Round_Button(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		o->type(FL_RADIO_BUTTON);
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
//		o->value(ivalue);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnRadioButtonPref)
		{
		Fl_Round_Button *o;
		o=new Fl_Round_Button(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		o->type(FL_RADIO_BUTTON);
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
//		o->value(ivalue);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnToggleButtonPref)
		{
		Fl_Button *o;
		o=new Fl_Button(x,wnd->y()+y,w,h,"");				//v1.07
		o->copy_label( label.c_str() );						//v1.07
		o->type(FL_TOGGLE_BUTTON);
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
//		o->value(ivalue);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnCheckPref)
		{
		Fl_Check_Button *o;
		o=new Fl_Check_Button(x,wnd->y()+y,w,h,"");			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
//		o->value(ivalue);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}
		}

	if(type==cnMenuChoicePref)
		{
		Fl_Choice *o;
		o=new Fl_Choice(x,wnd->y()+y,w,h,"");				//v1.07
		o->copy_label( label.c_str() );						//v1.07
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
		if(tfont!=-1) o->textfont(tfont);
		if(tsize!=-1) o->textsize(tsize);

		mystr ms=options;
		string elem;

		for(int j=0;;j++)							//load menu button drop down choices
			{
			if(ms.ExtractElement(j,',',elem)!=0)
				{
				o->add(elem.c_str(),0,0,0);
				}
			else break;
			}
//		o->value(ivalue);
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback(cb_pref_usr_callback);
			}

		}



	if( type == cnGCColColour )
		{
		GCCol *o;
		o=new GCCol(x,wnd->y()+y,w,h,"");					//v1.07
		o->copy_label( label.c_str() );						//v1.07
		ctrl_ptrs[row_count][i]=o;
		o->labeltype( (Fl_Labeltype)label_type);
		o->align( (Fl_Align)label_align );
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if(lfont!=-1) o->labelfont(lfont);
		if(lsize!=-1) o->labelsize(lsize);
//		if(tfont!=-1) o->textfont(tfont);
//		if(tsize!=-1) o->textsize(tsize);
//		o->value(svalue.c_str());
		ctrl_wnd=(void*)o;
		if(cb!=0)											//callback defined by user?
			{
			o->callback( cb_pref_usr_callback );
//            o->set_event_callback(  cb_pref_usr_callback );
			}

		}


	if( type == cnGCLed )
		{
		GCLed *o;
		o = new GCLed( x, wnd->y() + y, w, h, "" );			//v1.07
		o->copy_label( label.c_str() );						//v1.07
		ctrl_ptrs[ row_count ][ i ] = o;
		o->labeltype( (Fl_Labeltype)label_type );
		o->align( (Fl_Align)label_align );
        o->led_style = ctrl_style;
        
		o->tooltip( ctrl_list[row_count][i].tooltip.c_str() );	//v1.08
		if( lfont != -1 ) o->labelfont(lfont);
		if( lsize != -1 ) o->labelsize(lsize);
        o->set_col_from_str( options, ',' );
//		o->value(ivalue);
		ctrl_wnd = (void*) o;
		if( cb != 0 )											//callback defined by user?
			{
			o->callback( cb_pref_usr_callback );
			}
		}

	ctrl_ptrs[row_count][i]=ctrl_wnd;
	}
wnd->end();									//end group wnd


//pck->end();

row_count++;
ctrl_count=0;
}







//won't allow a uniq_id of 0xffffffff (cn_empty_control_uniq_id )
bool PrefWnd::AddControl()
{
if(ctrl_count>=cnMaxCtrl) return 0;
if(row_count>=cnMaxRow) return 0;

if( sc.uniq_id == cn_empty_control_uniq_id  ) return 0;

ctrl_list[row_count][ctrl_count]=sc;
ctrl_count++;
return 1;
}







//searches control list for matching id, returns its row, column within 'ctrl_list[ row ][ column ]'
bool PrefWnd::find_control_from_its_id( unsigned int id, int &row, int &column )
{

//printf( "find_control_from_its_id()0 - rows: %d, columns: %d\n", row_count, ctrl_count );

for( int i = 0; i < row_count; i++ )						//cycle each row of controls
	{
    for( int j = 0; j < cnMaxCtrl; j++ )					//cycle each column to see if control can be found
        {
//         printf( "find_control_from_its_id()1 - row: %d, column: %d, id: %u \n", i, j, ctrl_list[ i ][ j ].uniq_id );
       if( ctrl_list[ i ][ j ].uniq_id == id )
            {
            row = i;
            column = j;
//            printf( "find_control_from_its_id()2 - row: %d, column: %d, id: %u \n", i, j, ctrl_list[ i ][ j ].uniq_id );
            return 1;
            }
        
        if( ctrl_list[ i ][ j ].uniq_id == cn_empty_control_uniq_id ) break;     //hit an empty control in this row?
        }
    }
return 0;
}












//cycle all controla and refresh their vals from user vars
void PrefWnd::refresh_controls_from_user_vars()
{
int d;
double dble;
string s1;

PrefWnd *o = this;

//gather and store each control's state
for( int j = 0; j< o->row_count; j++)							//cycle all rows
	{
	for(int i=0;i<cnMaxCtrl;i++)								//cycle this row's controls
		{
		
		if(o->ctrl_ptrs[j][i]!=0)								//control exists?			
			{
			int type=o->ctrl_list[j][i].type;					//get control type

			if(type==cnStaticTextPref)
				{
				}

			if( (type==cnInputPref) )
				{
				if((int)o->ctrl_list[j][i].sretval != -1 )
					{
					fl_input_change *con=(fl_input_change*)o->ctrl_ptrs[j][i];
				
					s1 = *o->ctrl_list[j][i].sretval;
					con->value( s1.c_str() );
					}
				}

			if( (type==cnInputIntPref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					fl_input_change *con=(fl_input_change*)o->ctrl_ptrs[j][i];
					strpf( s1, "%d", *o->ctrl_list[j][i].iretval );
					
					con->value( s1.c_str() );
//					sscanf(con->value(),"%d",&d);
//					*o->ctrl_list[j][i].iretval = d;
					}
				}

			if( (type==cnInputHexPref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					fl_input_change *con=(fl_input_change*)o->ctrl_ptrs[j][i];
					strpf( s1, "%x", *o->ctrl_list[j][i].iretval );
					con->value( s1.c_str() );
					}
				}

			if( (type==cnInputDoublePref) )
				{
				if(o->ctrl_list[j][i].dretval!=(double*)-1)
					{
					fl_input_change *con=(fl_input_change*)o->ctrl_ptrs[j][i];
					strpf( s1, "%.10g", *o->ctrl_list[j][i].dretval );
					con->value( s1.c_str() );
					}
				}

			if( (type==cnEditHistPref) )
				{
				if((int)o->ctrl_list[j][i].sretval != -1 )
					{
					Fl_Input *con=(Fl_Input*)o->ctrl_ptrs[j][i];

					s1 = *o->ctrl_list[j][i].sretval;
					con->value( s1.c_str() );
					}
				}

			if( (type==cnButtonPref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					Fl_Button *con=(Fl_Button*)o->ctrl_ptrs[j][i];
					con->value( *o->ctrl_list[j][i].iretval );
					}
				}

			if( (type==cnLightButtonPref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					Fl_Light_Button *con=(Fl_Light_Button*)o->ctrl_ptrs[j][i];
					con->value( *o->ctrl_list[j][i].iretval );
					}
				}

			if( (type==cnRoundButtonPref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					Fl_Round_Button *con=(Fl_Round_Button*)o->ctrl_ptrs[j][i];
					con->value( *o->ctrl_list[j][i].iretval );
					}
				}

			if( (type==cnRadioButtonPref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					Fl_Button *con=(Fl_Button*)o->ctrl_ptrs[j][i];
					con->value( *o->ctrl_list[j][i].iretval );
					}
				}

			if( (type==cnToggleButtonPref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					Fl_Button *con=(Fl_Button*)o->ctrl_ptrs[j][i];
					con->value( *o->ctrl_list[j][i].iretval );
					}
				}

			if( (type==cnCheckPref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					Fl_Check_Button *con=(Fl_Check_Button*)o->ctrl_ptrs[j][i];
					con->value( *o->ctrl_list[j][i].iretval );
					}
				}

			if( (type==cnMenuChoicePref) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					Fl_Choice *con=(Fl_Choice*)o->ctrl_ptrs[j][i];
					con->value( *o->ctrl_list[j][i].iretval );
					}
				}

			if( ( type == cnGCColColour) )
				{
				if((int)o->ctrl_list[j][i].sretval!=-1)
					{
					GCCol *con=(GCCol*)o->ctrl_ptrs[j][i];

					s1 = *o->ctrl_list[j][i].sretval;
					con->value( s1.c_str() );
					}
				}

			if( ( type == cnGCLed) )
				{
				if((int)o->ctrl_list[j][i].iretval!=-1)
					{
					GCLed *con = (GCLed*) o->ctrl_ptrs[ j ][ i ];
					con->ChangeCol( *o->ctrl_list[j][i].iretval );
					}
				}

			}
		}
	}

}

















//cycle and load all control states into user variables

void cb_btPrefOK(Fl_Widget*, void* v)
{
PrefWnd* o = (PrefWnd*)v;
int d;
double dble;

pref_update_user_vars( o, 1 );		//update all user vars

if( o->p_cb_on_ok != 0 ) o->p_cb_on_ok( o );							//v1.09

o->hide();
}






void cb_btPrefCancel(Fl_Widget*, void* v)
{

PrefWnd* wnd = (PrefWnd*)v;
wnd->hide();
}









PrefWnd::PrefWnd(int x,int y,int w, int h,const char *label,string sSectionPosIn,string sKeyPosIn):Fl_Double_Window(x,y,w,h,label)
{
p_cb_on_ok = 0;

label_w=100;
control_h=cnControlHeight;
entry_count=0;
ctrl_count=0;
row_count=0;
sSectionPos=sSectionPosIn;							//set if you need pos of pref wnd to be saved/recalled
sKeyPos=sKeyPosIn;
xpos=x;
ypos=y;

ClearToDefCtrl();



for(int j=0;j<cnMaxRow;j++)							//clear control window ptr array;
	{
	entry_wnd[j]=0;
	for(int i=0;i<cnMaxCtrl;i++)
		{
		ctrl_ptrs[j][i]=0;
		ctrl_list[j][i].uniq_id = 0xffffffff;
		}
	}


scroll = new Fl_Scroll( 0, 0, this->w() - 0, this->h() - 40 );

pck = new Fl_Pack(0,0,this->w()-17,this->h()-40 );
pck->box(FL_DOWN_FRAME);
 
resizable(scroll);
pck->end();											//end of pack wnd
scroll->end();										//end of scroll wnd

Fl_Button *btOK = new Fl_Button(this->w()-137,this->h()-33,55,25,"OK");
//btOK->labelsize(12);
btOK->callback(cb_btPrefOK,this);

Fl_Button *btCancel = new Fl_Button(btOK->x()+btOK->w()+10,btOK->y(),55,25,"Cancel");
//btCancel->labelsize(12);
btCancel->callback(cb_btPrefCancel,this);
end();

}









int PrefWnd::handle(int e)
{

return Fl_Double_Window::handle(e);
}


























