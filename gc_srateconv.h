/*
Copyright (C) 2023 BrerDawg

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

//gc_srateconv.h
//v1.08

#ifndef gc_srateconv_h
#define gc_srateconv_h

#include <stdio.h>
#include <string.h>
#include <string>
#include <wchar.h>
#include <math.h>
#include <complex>


#include "globals.h"
#include "GCProfile.h"
#include "audio_formats.h"
#include "filter_code.h"

using namespace std;


namespace gc_srateconv_code
	{


double qdss_resample( double x, double *buf, unsigned int bufsz, double fmax, double fsr, int wnwdth );
float qdss_resample_float( float x, float *buf, unsigned int bufsz, double fmax, double fsr, int wnwdth );
float qdss_resample_float_complete( float x, float *buf, unsigned int bufsz, float fmax, float fsrate, int wnwdth );
double qdss_resample_double_continuous( double xin, double *buf, unsigned int bufsz, double fmax, double fsrate, int wnwdth, double *head_bf );
float qdss_resample_float_continuous( float xin, float *buf, unsigned int bufsz, float fmax, float fsrate, int wnwdth, float *head_bf );

bool test_qdss_resampl();

float farrow_resample_float( float sampleIndexOutput, float *bf, unsigned int bfsize, bool &end_reached );
double farrow_resample_double( double sampleIndexOutput, double *bf, unsigned int bfsize, bool &end_reached );


bool make_srate_conv_tables( float src_srate, float fractional_step, float frq_cutoff, uint16_t tbl_cnt, uint16_t wnd_sz, float *farray );
float srconv_tbl_interp_fir_double( uint64_t pos, int tbl_idx, float *sinc_tbl, int sinc_sz, double *buf, unsigned int bufsz );
float srconv_tbl_interp_fir_float( uint64_t pos, int tbl_idx, float *sinc_tbl, int sinc_sz, float *buf, unsigned int bufsz );

void test_srconv_tbl();


bool qdss_resample_float_vector( int output_count, unsigned int srate_in, float fmax, int wnwdth, vector<float>vin, vector<float>&vout );
bool farrow_resample_float_vector( int output_count, vector<float>vin, vector<float>&vout );
bool farrow_resample_double_vector( int output_count, vector<double>vin, vector<double>&vout );


	}	//namespace gc_srateconv_code











#endif
