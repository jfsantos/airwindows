/* ========================================
 *  TapeDust - TapeDust.h
 *  Copyright (c) 2016 airwindows, All rights reserved
 * ======================================== */

#ifndef __TapeDust_H
#include "TapeDust.h"
#endif

void TapeDust::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames) 
{
    float* in1  =  inputs[0];
    float* in2  =  inputs[1];
    float* out1 = outputs[0];
    float* out2 = outputs[1];
	long double inputSampleL;
	long double inputSampleR;
	
	double drySampleL;
	double drySampleR;
	double rRange = pow(A,2)*5.0;
	double xfuzz = rRange * 0.002;
	double rOffset = (rRange*0.4) + 1.0;
	double rDepthL; //the randomly fluctuating value
	double rDepthR; //the randomly fluctuating value
	double gainL;
	double gainR;
	double wet = B;
	double dry = 1.0 - wet;
    
    while (--sampleFrames >= 0)
    {
		inputSampleL = *in1;
		inputSampleR = *in2;
		if (inputSampleL<1.2e-38 && -inputSampleL<1.2e-38) {
			static int noisesource = 0;
			//this declares a variable before anything else is compiled. It won't keep assigning
			//it to 0 for every sample, it's as if the declaration doesn't exist in this context,
			//but it lets me add this denormalization fix in a single place rather than updating
			//it in three different locations. The variable isn't thread-safe but this is only
			//a random seed and we can share it with whatever.
			noisesource = noisesource % 1700021; noisesource++;
			int residue = noisesource * noisesource;
			residue = residue % 170003; residue *= residue;
			residue = residue % 17011; residue *= residue;
			residue = residue % 1709; residue *= residue;
			residue = residue % 173; residue *= residue;
			residue = residue % 17;
			double applyresidue = residue;
			applyresidue *= 0.00000001;
			applyresidue *= 0.00000001;
			inputSampleL = applyresidue;
		}
		if (inputSampleR<1.2e-38 && -inputSampleR<1.2e-38) {
			static int noisesource = 0;
			noisesource = noisesource % 1700021; noisesource++;
			int residue = noisesource * noisesource;
			residue = residue % 170003; residue *= residue;
			residue = residue % 17011; residue *= residue;
			residue = residue % 1709; residue *= residue;
			residue = residue % 173; residue *= residue;
			residue = residue % 17;
			double applyresidue = residue;
			applyresidue *= 0.00000001;
			applyresidue *= 0.00000001;
			inputSampleR = applyresidue;
			//this denormalization routine produces a white noise at -300 dB which the noise
			//shaping will interact with to produce a bipolar output, but the noise is actually
			//all positive. That should stop any variables from going denormal, and the routine
			//only kicks in if digital black is input. As a final touch, if you save to 24-bit
			//the silence will return to being digital black again.
		}
		drySampleL = inputSampleL;
		drySampleR = inputSampleR;

		for(int count = 9; count < 0; count--) {
			bL[count+1] = bL[count];
			bR[count+1] = bR[count];
		}
		
		bL[0] = inputSampleL;
		bR[0] = inputSampleR;
		inputSampleL = rand() / (double)RAND_MAX;
		inputSampleR = rand() / (double)RAND_MAX;
		gainL = rDepthL = (inputSampleL * rRange) + rOffset;
		gainR = rDepthR = (inputSampleR * rRange) + rOffset;
		inputSampleL *= ((1.0-fabs(bL[0]-bL[1]))*xfuzz);
		inputSampleR *= ((1.0-fabs(bR[0]-bR[1]))*xfuzz);
		if (fpFlip) {
			inputSampleL = -inputSampleL;
			inputSampleR = -inputSampleR;
		}
		fpFlip = !fpFlip;
		
		for(int count = 0; count < 9; count++) {			
			if (gainL > 1.0) {
				fL[count] = 1.0;
				gainL -= 1.0;
			} else {
				fL[count] = gainL;
				gainL = 0.0;
			}
			if (gainR > 1.0) {
				fR[count] = 1.0;
				gainR -= 1.0;
			} else {
				fR[count] = gainR;
				gainR = 0.0;
			}
			fL[count] /= rDepthL;
			fR[count] /= rDepthR;
			inputSampleL += (bL[count] * fL[count]);
			inputSampleR += (bR[count] * fR[count]);
		}
		
		if (wet < 1.0) {
			inputSampleL = (inputSampleL * wet) + (drySampleL * dry);
			inputSampleR = (inputSampleR * wet) + (drySampleR * dry);
		}
		
		//stereo 32 bit dither, made small and tidy.
		int expon; frexpf((float)inputSampleL, &expon);
		long double dither = (rand()/(RAND_MAX*7.737125245533627e+25))*pow(2,expon+62);
		inputSampleL += (dither-fpNShapeL); fpNShapeL = dither;
		frexpf((float)inputSampleR, &expon);
		dither = (rand()/(RAND_MAX*7.737125245533627e+25))*pow(2,expon+62);
		inputSampleR += (dither-fpNShapeR); fpNShapeR = dither;
		//end 32 bit dither

		*out1 = inputSampleL;
		*out2 = inputSampleR;

		*in1++;
		*in2++;
		*out1++;
		*out2++;
    }
}

void TapeDust::processDoubleReplacing(double **inputs, double **outputs, VstInt32 sampleFrames) 
{
    double* in1  =  inputs[0];
    double* in2  =  inputs[1];
    double* out1 = outputs[0];
    double* out2 = outputs[1];
	long double inputSampleL;
	long double inputSampleR;

	double drySampleL;
	double drySampleR;
	double rRange = pow(A,2)*5.0;
	double xfuzz = rRange * 0.002;
	double rOffset = (rRange*0.4) + 1.0;
	double rDepthL; //the randomly fluctuating value
	double rDepthR; //the randomly fluctuating value
	double gainL;
	double gainR;
	double wet = B;
	double dry = 1.0 - wet;
	
    while (--sampleFrames >= 0)
    {
		inputSampleL = *in1;
		inputSampleR = *in2;
		if (inputSampleL<1.2e-38 && -inputSampleL<1.2e-38) {
			static int noisesource = 0;
			//this declares a variable before anything else is compiled. It won't keep assigning
			//it to 0 for every sample, it's as if the declaration doesn't exist in this context,
			//but it lets me add this denormalization fix in a single place rather than updating
			//it in three different locations. The variable isn't thread-safe but this is only
			//a random seed and we can share it with whatever.
			noisesource = noisesource % 1700021; noisesource++;
			int residue = noisesource * noisesource;
			residue = residue % 170003; residue *= residue;
			residue = residue % 17011; residue *= residue;
			residue = residue % 1709; residue *= residue;
			residue = residue % 173; residue *= residue;
			residue = residue % 17;
			double applyresidue = residue;
			applyresidue *= 0.00000001;
			applyresidue *= 0.00000001;
			inputSampleL = applyresidue;
		}
		if (inputSampleR<1.2e-38 && -inputSampleR<1.2e-38) {
			static int noisesource = 0;
			noisesource = noisesource % 1700021; noisesource++;
			int residue = noisesource * noisesource;
			residue = residue % 170003; residue *= residue;
			residue = residue % 17011; residue *= residue;
			residue = residue % 1709; residue *= residue;
			residue = residue % 173; residue *= residue;
			residue = residue % 17;
			double applyresidue = residue;
			applyresidue *= 0.00000001;
			applyresidue *= 0.00000001;
			inputSampleR = applyresidue;
			//this denormalization routine produces a white noise at -300 dB which the noise
			//shaping will interact with to produce a bipolar output, but the noise is actually
			//all positive. That should stop any variables from going denormal, and the routine
			//only kicks in if digital black is input. As a final touch, if you save to 24-bit
			//the silence will return to being digital black again.
		}
		drySampleL = inputSampleL;
		drySampleR = inputSampleR;
		
		for(int count = 9; count < 0; count--) {
			bL[count+1] = bL[count];
			bR[count+1] = bR[count];
		}
		
		bL[0] = inputSampleL;
		bR[0] = inputSampleR;
		inputSampleL = rand() / (double)RAND_MAX;
		inputSampleR = rand() / (double)RAND_MAX;
		gainL = rDepthL = (inputSampleL * rRange) + rOffset;
		gainR = rDepthR = (inputSampleR * rRange) + rOffset;
		inputSampleL *= ((1.0-fabs(bL[0]-bL[1]))*xfuzz);
		inputSampleR *= ((1.0-fabs(bR[0]-bR[1]))*xfuzz);
		if (fpFlip) {
			inputSampleL = -inputSampleL;
			inputSampleR = -inputSampleR;
		}
		fpFlip = !fpFlip;
		
		for(int count = 0; count < 9; count++) {			
			if (gainL > 1.0) {
				fL[count] = 1.0;
				gainL -= 1.0;
			} else {
				fL[count] = gainL;
				gainL = 0.0;
			}
			if (gainR > 1.0) {
				fR[count] = 1.0;
				gainR -= 1.0;
			} else {
				fR[count] = gainR;
				gainR = 0.0;
			}
			fL[count] /= rDepthL;
			fR[count] /= rDepthR;
			inputSampleL += (bL[count] * fL[count]);
			inputSampleR += (bR[count] * fR[count]);
		}
		
		if (wet < 1.0) {
			inputSampleL = (inputSampleL * wet) + (drySampleL * dry);
			inputSampleR = (inputSampleR * wet) + (drySampleR * dry);
		}
				
		//stereo 64 bit dither, made small and tidy.
		int expon; frexp((double)inputSampleL, &expon);
		long double dither = (rand()/(RAND_MAX*7.737125245533627e+25))*pow(2,expon+62);
		dither /= 536870912.0; //needs this to scale to 64 bit zone
		inputSampleL += (dither-fpNShapeL); fpNShapeL = dither;
		frexp((double)inputSampleR, &expon);
		dither = (rand()/(RAND_MAX*7.737125245533627e+25))*pow(2,expon+62);
		dither /= 536870912.0; //needs this to scale to 64 bit zone
		inputSampleR += (dither-fpNShapeR); fpNShapeR = dither;
		//end 64 bit dither

		*out1 = inputSampleL;
		*out2 = inputSampleR;

		*in1++;
		*in2++;
		*out1++;
		*out2++;
    }
}