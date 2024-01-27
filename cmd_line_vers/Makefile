# A Makefile for both Linux and Windows, 30-dec-2015


#define all executables here
app_name= ti_lpc_cmd


all: ${app_name}

#define compiler options	
CC=g++

ifneq ($(OS),Windows_NT)			#linux?
#	CFLAGS=-g -O0 -fpermissive -fno-inline -Dbuild_date="\"`date +%Y\ %b\ %d`\"" #-Dbuild_date="\"2016 Mar 23\""
#	LIBS=-L/usr/X11/lib -L/usr/local/lib -lfltk_images /usr/local/lib/libfltk.a -lfltk_png -lfltk_z -ljpeg -lrt -lm -lXcursor -lXfixes -lXext -lXft -lfontconfig -lXinerama -lpthread -ldl -lX11 -lfftw3 #-ljack
	INCLUDE= -I/usr/local/include
	CFLAGS=-g -O0  -Wfatal-errors -Wfatal-errors -fpermissive -fno-inline -Dbuild_date="\"`date +%Y-%b-%d`\"" #-Dbuild_date="\"2016-Mar-23\""			#64 bit
	LIBS=-lrt -lm -ldl 													#64 bit
else								#windows?
	CFLAGS=-g -O0 -fno-inline -DWIN32 -mms-bitfields -Dcompile_for_windows -Dbuild_date="\"`date +%Y\ %b\ %d`\""
LIBS= -L/usr/local/lib -static -lm
	INCLUDE= -I/usr/local/include
endif


#define object files for each executable, see dependancy list at bottom
obj1= ti_lpc_cmd.o GCProfile.o audio_formats.o      #gc_srateconv.o filter_code.o
 #obj2= remez.o
#obj3= remez1.o remez1.tab.o



#linker definition
ti_lpc_cmd: $(obj1)
	$(CC) $(CFLAGS) -o $@ $(obj1) $(LIBS)


#linker definition
#remez: $(obj2)
#	$(CC) $(CFLAGS) -o $@ $(obj2) $(LIBS)


#linker definition
#remez1: $(obj3)
#	$(CC) $(CFLAGS) -o $@ $(obj3) $(LIBS)


#compile definition for all cpp files to be complied into .o files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $<

%.o: %.cxx
	$(CC) $(CFLAGS) $(INCLUDE) -c $<


#dependancy list per each .o file
ti_lpc_cmd.o: ti_lpc_cmd.h GCProfile.h                    #audio_formats.h gc_srateconv.h filter_code.h
GCProfile.o: GCProfile.h
audio_formats.o: audio_formats.h GCProfile.h
#gc_srateconv.o: gc_srateconv.h GCProfile.h
#filter_code.o: filter_code.h
#mgraph.o:  mgraph.h GCProfile.h




.PHONY : clean
clean : 
		-rm ti_lpc_cmd $(obj1)
#		-rm remez.exe $(obj2)


