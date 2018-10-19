// https://developer.x-plane.com/sdk/plugin-sdk-sample-code/
// https://developer.x-plane.com/code-sample/simdata/
// https://developer.x-plane.com/code-sample/timedprocessing/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "XPLMMenus.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"

void MyMenuHandlerCallback(void* inMenuRef, void* inItemRef);
float MyFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop,
							int inCounter, void* inRefcon);

static XPLMMenuID myMenu;
static int index_start;
static int index_end;
static FILE* OutputFile;
static char XplanePath[255];

static XPLMDataRef PlaneLat;
static XPLMDataRef PlaneLon;
static XPLMDataRef PlaneEle;
static XPLMDataRef PlanePsi;
static XPLMDataRef PlaneTheta;
static XPLMDataRef PlanePhi;

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	strcpy(outName, "DataRecord");
	strcpy(outSig , "xplanesdk.utility.datarecord");
	strcpy(outDesc, "A plugin that records sim data.");

	XPLMGetSystemPath(XplanePath);

	myMenu = XPLMCreateMenu("Data", NULL, 0, MyMenuHandlerCallback, NULL); //Put the menu in the menu bar.
	index_start = XPLMAppendMenuItem(myMenu, "Start", (void*)0, 1); //Append item.
	index_end = XPLMAppendMenuItem(myMenu, "End", (void*)1, 1); //Append item.
	XPLMEnableMenuItem(myMenu, index_end, 0);

	PlaneLat = XPLMFindDataRef("sim/flightmodel/position/latitude");
	PlaneLon = XPLMFindDataRef("sim/flightmodel/position/longitude");
	PlaneEle = XPLMFindDataRef("sim/flightmodel/position/elevation");
	PlanePsi = XPLMFindDataRef("sim/flightmodel/position/true_psi");
	PlaneTheta = XPLMFindDataRef("sim/flightmodel/position/true_theta");
	PlanePhi = XPLMFindDataRef("sim/flightmodel/position/true_phi");

	return 1;
}

PLUGIN_API void	XPluginStop(void)
{
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage, void* inParam)
{
}

// Get current time
// https://blog.csdn.net/qq_22122811/article/details/52741483

void MyMenuHandlerCallback(void* inMenuRef, void* inItemRef)
{
	time_t now;
	struct tm *p;
	char OutputPath[300], timestr[30];
	
	if (inItemRef == 0)
	{
		time(&now);
		p = localtime(&now);
		sprintf(timestr, "%d-%02d-%02d_%02d-%02d-%02d", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
		strcpy(OutputPath, XplanePath);
		strcat(OutputPath, "Data/Data_");
		strcat(OutputPath, timestr);
		strcat(OutputPath, ".txt");
		OutputFile = fopen(OutputPath, "w");

		XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, 0.1f, NULL); //interval = 0.1

		XPLMEnableMenuItem(myMenu, index_end, 1);
		XPLMEnableMenuItem(myMenu, index_start, 0);
	}
	else
	{
		XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL); //Unregister the callback
		
		fclose(OutputFile);
		
		XPLMEnableMenuItem(myMenu, index_end, 0);
		XPLMEnableMenuItem(myMenu, index_start, 1);
	}
}

float MyFlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop,
							int inCounter, void* inRefcon)
{
	float  elapsed = XPLMGetElapsedTime();

	double lat = XPLMGetDatad(PlaneLat);
	double lon = XPLMGetDatad(PlaneLon);
	double ele = XPLMGetDatad(PlaneEle);
	float psi = XPLMGetDataf(PlanePsi);
	float theta = XPLMGetDataf(PlaneTheta);
	float phi = XPLMGetDataf(PlanePhi);

	/* Write the data to a file. */
	fprintf(OutputFile, "%14.6f%20.12f%20.12f%14.6f%14.6f%14.6f%14.6f\r\n", elapsed, lat, lon, ele, psi, theta, phi);

	/* Return 0.1 to indicate that we want to be called again in 0.1 second. */
	return 0.1f;
}
