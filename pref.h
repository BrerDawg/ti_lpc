//pref.h
//v1.13

#ifndef pref_h
#define pref_h


#include <stdio.h>
#include <string>

#include <FL/Fl_Window.H>
//#include <FL/Fl_Box.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Pack.H>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include "GCProfile.h"
#include "GCCol.h"
#include "GCLed.h"

#define cnControlHeight 20


using namespace std;


enum							//type of control
{
cnNone,
cnStaticTextPref,
cnInputPref,					//edit text box for strings
cnInputIntPref,					//edit text box for integers
cnInputHexPref,					//edit text box for hex nums
cnInputDoublePref,				//edit text box for floats/doubles
cnEditHistPref,					//edit text ?????
cnButtonPref,
cnLightButtonPref,
cnRoundButtonPref,
cnRadioButtonPref,
cnToggleButtonPref,
cnCheckPref,
cnMenuChoicePref,
cnGCColColour,
cnGCLed,
cnEndMarkerPref					//make sure this is last enum
};

#define cnMaxCtrl 30			//max controls in an row
#define cnMaxRow 200			//max row entries

#define cn_empty_control_uniq_id 0xffffffff         //value used to clear scontrol[][].uniq_id, it shows no control is set at a particular row and column
                                                    //don't use this value as find_control_from_its_id() won't find control
                                                    //see also cn_dontcare_control_uniq_id

#define cn_dontcare_control_uniq_id 0xfffffffe      //highest allowable value, used to show uniq_id is set to don't care

//void PrefWnd;			//need this for struct below, see further down for real PrefWnd class def



struct sControl					//a control's params
{
string label;
string tooltip;
string section;
string key;
int keypostfix;

int label_type;
int label_align;
string options;
int ctrl_style;                         //e.g: for GCLed could be: cn_gcled_style_square, cn_gcled_style_round

int type;
int x,y,w,h;
int labelfont;
int labelsize;
int textfont;
int textsize;
bool dynamic;
string def;

string radio_group_name;				//if a name assigned, any other radio button with matching name is toggled off when
										// - pressed, allows radio button 'groups' to span across many rows

int *iretval;
double *dretval;
string *sretval;

unsigned int uniq_id;                   //this is a user value that may be used to identify which control was pressed, apart from row and column values callback receives
                                        //don't use 0xffffffff (cn_empty_control_uniq_id) as this is the value to show no control exists, find_control_from_its_id() won't find control if you use it
void (*cb)(void*,int,int);
};





class fl_input_change : public Fl_Input
{
private:
bool ctrl_key;
bool left_button;
string dropped_str;

public:
int row;							//v1.09
int clmn;


public:
fl_input_change( int xx, int yy, int ww, int hh, const char *label );

private:
int handle( int e );

};





class PrefWnd : public Fl_Double_Window
{
private:
int label_w;
int control_h;
int control_cnt;
int xpos,ypos;

//int typelist[cnMaxRow][cnMaxCtrl+1];	//first number is num of controls in this entry
Fl_Group *entry_wnd[cnMaxRow];
int entry_count;

public:
Fl_Pack* pck;
sControl sc;
sControl ctrl_list[cnMaxRow][cnMaxCtrl];
void* ctrl_ptrs[cnMaxRow][cnMaxCtrl];
int ctrl_count;
int row_count;
sControl ctrl;

string sSectionPos;						//set if you need pos of pref wnd to be saved/recalled
string sKeyPos;


public:
Fl_Scroll* scroll;
void (*p_cb_on_ok)( void * ); 			//for final OK button press 'cb_btPrefOK()', allows user set callback function


public:
void ClearToDefCtrl();
void End();
void CreateRow(int height=cnControlHeight);
bool AddControl();
void Show(bool modal);
void Save(GCProfile &p);
void Load(GCProfile &p);
bool find_control_from_its_id( unsigned int id, int &row, int &column );
void set_callback_on_ok( void (*p_cb)( void* ) );
void refresh_controls_from_user_vars();

//void AddVar(string s);
PrefWnd(int x,int y,int w, int h,const char *label,string sSectionPosIn,string sKeyPosIn);

private:
int handle(int e);

};


#endif

