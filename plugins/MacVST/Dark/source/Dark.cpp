/* ========================================
 *  Dark - Dark.h
 *  Copyright (c) 2016 airwindows, All rights reserved
 * ======================================== */

#ifndef __Dark_H
#include "Dark.h"
#endif

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {return new Dark(audioMaster);}

Dark::Dark(audioMasterCallback audioMaster) :
    AudioEffectX(audioMaster, kNumPrograms, kNumParameters)
{
	A = 1.0;
	for(int count = 0; count < 99; count++) {
		lastSampleL[count] = 0;
		lastSampleR[count] = 0;
	}
	fpd = 17;
	//this is reset: values being initialized only once. Startup values, whatever they are.
	
    _canDo.insert("plugAsChannelInsert"); // plug-in can be used as a channel insert effect.
    _canDo.insert("plugAsSend"); // plug-in can be used as a send effect.
    _canDo.insert("x2in2out"); 
    setNumInputs(kNumInputs);
    setNumOutputs(kNumOutputs);
    setUniqueID(kUniqueId);
    canProcessReplacing();     // supports output replacing
    canDoubleReplacing();      // supports double precision processing
	programsAreChunks(true);
    vst_strncpy (_programName, "Default", kVstMaxProgNameLen); // default program name
}

Dark::~Dark() {}
VstInt32 Dark::getVendorVersion () {return 1000;}
void Dark::setProgramName(char *name) {vst_strncpy (_programName, name, kVstMaxProgNameLen);}
void Dark::getProgramName(char *name) {vst_strncpy (name, _programName, kVstMaxProgNameLen);}
//airwindows likes to ignore this stuff. Make your own programs, and make a different plugin rather than
//trying to do versioning and preventing people from using older versions. Maybe they like the old one!

static float pinParameter(float data)
{
	if (data < 0.0f) return 0.0f;
	if (data > 1.0f) return 1.0f;
	return data;
}

VstInt32 Dark::getChunk (void** data, bool isPreset)
{
	float *chunkData = (float *)calloc(kNumParameters, sizeof(float));
	chunkData[0] = A;
	/* Note: The way this is set up, it will break if you manage to save settings on an Intel
	 machine and load them on a PPC Mac. However, it's fine if you stick to the machine you 
	 started with. */
	
	*data = chunkData;
	return kNumParameters * sizeof(float);
}

VstInt32 Dark::setChunk (void* data, VstInt32 byteSize, bool isPreset)
{	
	float *chunkData = (float *)data;
	A = pinParameter(chunkData[0]);
	/* We're ignoring byteSize as we found it to be a filthy liar */
	
	/* calculate any other fields you need here - you could copy in 
	 code from setParameter() here. */
	return 0;
}

void Dark::setParameter(VstInt32 index, float value) {
    switch (index) {
        case kParamA: A = value; break;
		default: throw; // unknown parameter, shouldn't happen!
    }
}

float Dark::getParameter(VstInt32 index) {
    switch (index) {
        case kParamA: return A; break;
        default: break; // unknown parameter, shouldn't happen!
    } return 0.0; //we only need to update the relevant name, this is simple to manage
}

void Dark::getParameterName(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "Quant", kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
    } //this is our labels for displaying in the VST host
}

void Dark::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: switch((VstInt32)( A * 1.999 )) //0 to almost edge of # of params
		{	case 0: vst_strncpy (text, "CD 16", kVstMaxParamStrLen); break;
			case 1: vst_strncpy (text, "HD 24", kVstMaxParamStrLen); break;
			default: break; // unknown parameter, shouldn't happen!
		} break; //completed consoletype 'popup' parameter, exit
        default: break; // unknown parameter, shouldn't happen!
	} //this displays the values and handles 'popups' where it's discrete choices
}

void Dark::getParameterLabel(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "", kVstMaxParamStrLen); break;
		default: break; // unknown parameter, shouldn't happen!
    }
}

VstInt32 Dark::canDo(char *text) 
{ return (_canDo.find(text) == _canDo.end()) ? -1: 1; } // 1 = yes, -1 = no, 0 = don't know

bool Dark::getEffectName(char* name) {
    vst_strncpy(name, "Dark", kVstMaxProductStrLen); return true;
}

VstPlugCategory Dark::getPlugCategory() {return kPlugCategEffect;}

bool Dark::getProductString(char* text) {
  	vst_strncpy (text, "airwindows Dark", kVstMaxProductStrLen); return true;
}

bool Dark::getVendorString(char* text) {
  	vst_strncpy (text, "airwindows", kVstMaxVendorStrLen); return true;
}
