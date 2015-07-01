//
//    Copyright (c) 2014-2015 Juan Garcia-Prieto Cuesta
//
//    This file is part of Fast Functional Connectivity (FastFC)
//
//    FastFC is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    FastFC is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with FastFC.  If not, see <http://www.gnu.org/licenses/>.
//
//    ------------------------------------------ 
//    Contact:
//    Juan Garcia-Prieto <juangpc@gmail.com>
//    ------------------------------------------
//
//    Please consider helping by citing our research.
// 
//    J. Garcia-Prieto, E. Pereda
//
#include <mex.h>
#include <fftw3.h>


void mexFunction(int nlhs,mxArray *plhs[],int nrhs,const mxArray *prhs[])
{
    
    unsigned int i,aux_i;
    float *b,*x;
    double *x_d,*b_d,*y;
    fftwf_complex *fft_b,*fft_x;
    fftwf_plan p_r2c,p_c2r;
    
    const unsigned int l_b=(unsigned int)mxGetN(prhs[0]);
    const unsigned int n_samples=(unsigned int)mxGetM(prhs[1]);
    const unsigned int n_sensors=(unsigned int)mxGetN(prhs[1]);
    const short int mode=(short int)mxGetScalar(prhs[2]);
            
    const unsigned int l_p=n_samples+2*(l_b-1);
    const int is_even=(l_p%2)?0:1;

    b_d=(double*)mxGetPr(prhs[0]);    
    x_d=(double*)mxGetPr(prhs[1]);
    
    b=(float*)malloc(l_p*sizeof(float));
    x=(float*)malloc(l_p*sizeof(float));
    fft_x=(fftwf_complex*)fftwf_malloc(l_p*sizeof(fftwf_complex));
    fft_b=(fftwf_complex*)fftwf_malloc(l_p*sizeof(fftwf_complex));
    
    if(mode==2)
    {
        p_r2c=fftwf_plan_dft_r2c_1d(l_p,b,fft_b,FFTW_EXHAUSTIVE);
        p_c2r=fftwf_plan_dft_c2r_1d(l_p,fft_b,b,FFTW_EXHAUSTIVE);
    }
    else
    {
        p_r2c=fftwf_plan_dft_r2c_1d(l_p,b,fft_b,FFTW_ESTIMATE);
        p_c2r=fftwf_plan_dft_c2r_1d(l_p,fft_b,b,FFTW_ESTIMATE);
    }                                           

    //copy b with normal padding
    for(i=0;i<l_b;i++)
        b[i]=(float)b_d[i];
    for(;i<l_p;i++)
        b[i]=(float)0.;
    //execute fft of b
    fftwf_execute_dft_r2c(p_r2c,b,fft_b);
    
    //calculate filter mask and store in b (as it is Real)
    for(i=0;i<l_p/2+1;i++)
        b[i]=fft_b[i][0]*fft_b[i][0]+fft_b[i][1]*fft_b[i][1];
    
    //copy sensor with circular padding
    for(i=0;i<l_b-2;i++)
        x[i]=(2.*(float)x_d[0])-(float)x_d[l_b-2-i];
    for(i=0;i<n_samples;i++)
        x[l_b-2+i]=(float)x_d[i];
    aux_i=1;
    for(;i<l_p;i++)
        x[l_b-2+i]=(2.*(float)x_d[n_samples-1])-(float)x_d[n_samples-1-aux_i++];
    //calc fft of sensor
    fftwf_execute_dft_r2c(p_r2c,x,fft_x);
    
    //filter in freq domain
    for(i=0;i<l_p/2+1;i++)
    {
        fft_x[i][0]*=b[i];
        fft_x[i][1]*=b[i];
    }
    
    fftwf_execute_dft_c2r(p_c2r,fft_x,x);
    plhs[0]=mxCreateDoubleMatrix(n_samples,n_sensors,mxREAL);
    y=(double*)mxGetPr(plhs[0]);
    
    aux_i=0;
    for(i=l_b-2;i<l_p-l_b;i++)
        y[aux_i++]=(double)(x[i]/l_p);

    fftwf_destroy_plan(p_c2r);
    fftwf_destroy_plan(p_r2c);
    fftwf_free(fft_b);    
    fftwf_free(fft_x);
    free(b);
    free(x);

}