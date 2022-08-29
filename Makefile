# A Makefile for both Linux and Windows,  10-nov-2016

#define all executables here


#	OPTMZ=-O0						# !!!!!!!!!!! -O0 for fast compile, see below specific optimization for certain sources
#	  or =-O3 -msse3
app_name= ti_lpc


all: ${app_name}

#define compiler options	
CC=g++

ifneq ($(OS),Windows_NT)			#linux?
	OPTMZ=-O0						# !!!!!!!!!!! -O0 for fast compile, see below specific optimization for certain sources
	SIMD=

	CFLAGS=-g -fpermissive -Wno-narrowing -fno-inline -Dbuild_date="\"`date +%Y-%b-%d`\"" #-Dbuild_date="\"2016-Mar-23\"" `pkg-config --cflags libpulse-simple` `pkg-config --cflags rtaudio`
#	LIBS=-L/usr/X11/lib -L/usr/local/lib /usr/local/lib/libfltk_gl.a /usr/local/lib/libfltk_images.a /usr/local/lib/libfltk.a /usr/local/lib/libfltk_png.a -lz -ljpeg -lrt -lm -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lpthread -ldl -lX11 -lasound -ljack -lfluidsynth -lglut -lGL -lGLU #-lglfw -lfftw3
#	INCLUDE= -I/usr/local/include
	LIBS=-lfltk -lX11 -lrt -lm -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lXrender -lpthread -ldl -lX11 -lasound -ljack -lasound `pkg-config --libs rtaudio`	#64 bit
	INCLUDE= -I/usr/include/cairo	#64 bit
else								#windows?
	OPTMZ=-O0						# !!!!!!!!!!! -O0 for fast compile, see below specific optimization for certain sources
	SIMD=
	CFLAGS=-g -fno-inline -DWIN32 -mms-bitfields -Dcompile_for_windows -Dbuild_date="\"`date +%Y\ %b\ %d`\""
LIBS= -L/usr/local/lib -static -mwindows -lfltk -lole32 -luuid -lcomctl32 -lm -lwinmm
	INCLUDE= -I/usr/local/include
endif

#define object files for each executable, see dependancy list at bottom
obj1= ti_lpc.o GCProfile.o pref.o GCLed.o GCCol.o audio_formats.o tms5220.o gclog.o gcthrd.o pcaudio.o synth_eng.o fluid.o wfm.o mgraph.o filter_code.o gc_srateconv.o gc_rtaudio.o #pulseaud.o
#obj2= backprop.o layer.o



#linker definition
ti_lpc: $(obj1)
	$(CC) $(CFLAGS) -o $@ $(obj1) $(LIBS)


#linker definition
#backprop: $(obj2)
#	$(CC) $(CFLAGS) -o $@ $(obj2) $(LIBS)



#compile definition for all cpp files to be complied into .o files
%.o: %.c
	$(CC) $(CFLAGS) $(OPTMZ) $(INCLUDE) -c $<

%.o: %.cpp
	$(CC) $(CFLAGS) $(OPTMZ) $(INCLUDE) -c $<

%.o: %.cxx
	$(CC) $(CFLAGS) $(OPTMZ) $(INCLUDE) -c $<



#dependancy list per each .o file
ti_lpc.o: ti_lpc.h globals.h GCProfile.h pref.h GCCol.h GCLed.h tms5220.h gclog.h gcthrd.h pcaudio.h synth_eng.h fluid.h wfm.h mgraph.h filter_code.h gc_srateconv.h gc_rtaudio.h #pulseaud.h
GCProfile.o: GCProfile.h
pref.o: pref.h GCCol.h GCLed.h
GCCol.o:  GCCol.h
GCLed.o: GCLed.h
audio_formats.o: audio_formats.h
#filter.o: filter.h globals.h
tms5520.o: tms5220.h GCProfile.h
gclog.o: gclog.h GCProfile.h
gcthrd.o: gcthrd.h GCProfile.h gclog.h
pcaudio.o: globals.h pcaudio.h
fuild.o: fluid.cxx fluid.h
synth_eng.o: synth_eng.h globals.h
wfm.o: wfm.h globals.h GCProfile.h mgraph.h
mgraph.o: mgraph.h GCProfile.h 
filter_code.o: filter_code.h GCProfile.h
gc_srateconv.o: gc_srateconv.h GCProfile.h 
#pulseaud.o: globals.h pulseaud.h GCProfile.h 
gc_rtaudio.o: gc_rtaudio.h

#layer.o: layer.h



.PHONY : clean
clean : 
		-rm $(obj1)					#remove obj files
ifneq ($(OS),Windows_NT)
		-rm ${app_name}				#remove linux exec
else
		-rm ${app_name}.exe			#remove windows exec
endif


