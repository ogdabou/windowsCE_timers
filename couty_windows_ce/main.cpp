// TP1_timer.cpp : periodic timer with time measurement
//

#include "stdafx.h"

// some differences between WinCE and WinNT
#ifdef UNDER_CE
#pragma comment(lib,"mmtimer.lib")	// CE timer library
#else
#include "windows.h"
#include <math.h>
#pragma comment(lib,"winmm.lib")	// NT timer library
#endif

/* =================================================
	Constants
   ================================================= */
#define PERIOD		10    // milliseconds
#define RESOLUTION	0		// best timer quality 


/* =================================================
	Globals
   ================================================= */
LARGE_INTEGER T1,T2, Freq;  // time measurement variables
unsigned int SECOND_IN_MICRO;
LONGLONG MAX_PERIOD, MIN_PERIOD;

int CAT_MIN, CAT_MAX, TOTAL_HITS, TOTAL_CYCLES;


bool DEBUG = true;
void sortSimpleResultInBenchResults(LONGLONG);

/* -------------------------------------------------
	Bench Container
   ------------------------------------------------- */
struct BenchResults
{
	int category;
	int hits;
	int percent;
};

float round(float value) {
     return floor(value + 0.5);
} 

BenchResults benchRes[14];

void printResults()
{
	printf("**************************\n");
	printf("Timer period     : %d\n", PERIOD);
	printf("Period MAX in ms : %d\n", MAX_PERIOD / 1000);
	printf("Period MIN in ms : %d\n", MIN_PERIOD / 1000);
	printf("**************************\n");
	printf("Total timer cycle = %d\n", TOTAL_CYCLES);
	printf("--------------------------\n");
	printf(" ms  | hits| percentage   \n");
	printf("--------------------------\n");
	for (int i = 0; i < 14; i ++)
	{	
		float categoryAsFloat = (float)benchRes[i].category / 1000;
		if ( categoryAsFloat >= 0)
		{
			printf(" ");	
		}
		printf("%.2f     %d|", categoryAsFloat, benchRes[i].hits);
		int j = 0;
		while (j < benchRes[i].percent)
		{
			printf("*");
			j = j + 1;
		}
		printf("\n");
	}
}

void computePercentages()
{
	for (int i = 0; i < 14; i ++)
	{
		if (benchRes[i].hits != 0)
		{
			benchRes[i].percent = round(((float)benchRes[i].hits / (float)TOTAL_HITS) * 100);
		}
	}
}

void fillBenchResultsCategories()
{	
	int category = CAT_MIN;
	int increment = 10;

	for (int i = 0; i < 14; i++)
	{
		benchRes[i].category = category;
		benchRes[i].hits = 0;
		benchRes[i].percent = 0;
		category = category + increment;
	}
}

DWORD WINAPI sortSimpleResultInBenchResults(LPVOID param)
{
	LONGLONG result = (LONGLONG) param;
	// printf("Diff : %d us\n", result);
	for (int i = 0; i < 14; i ++)
	{
		// TODO : changer cat_min et cat_max, il est possible quej e rate des valeurs
		if (result > CAT_MIN && result < CAT_MAX && benchRes[i].category > result)
		{
			//if (DEBUG) printf("Diff : %d > %d\n", benchRes[i].category, result);
			if (result >= 0)
			{
				i = i - 1;
			}
			benchRes[i].hits = benchRes[i].hits + 1;
			TOTAL_HITS = TOTAL_HITS + 1;
			computePercentages();
			printResults();
			return 0;
		}
	}
	return 0;
}



/* -------------------------------------------------
	stopper thread
   ------------------------------------------------- */
DWORD WINAPI stopThread(LPVOID param)
{
	HANDLE hEstop;

    hEstop= (HANDLE)param;
	printf("Press <RET> key to stop\n\n");

	// wait for user input
	getchar();
	SetEvent(hEstop);
	return 0;
}

/* -------------------------------------------------
	timer function
   ------------------------------------------------- */
void CALLBACK TimerCallback(UINT tid, UINT b, DWORD_PTR usr, DWORD_PTR p2, DWORD_PTR p3)
{
	// measure time and display delay in microseconds
	QueryPerformanceCounter(&T2);
	TOTAL_CYCLES = TOTAL_CYCLES + 1;
	LONGLONG result = ((T2.QuadPart - T1.QuadPart)*1000000) / Freq.QuadPart;
	// printf("Timer %d - %d us\n", usr, result);
	LONGLONG diff = result - SECOND_IN_MICRO;
	if (MAX_PERIOD < result)
	{
		MAX_PERIOD = round(result);
	}

	if (MIN_PERIOD > result || MIN_PERIOD == 0)
	{
		MIN_PERIOD = round(result);
	}
  
	DWORD threadId;
	HANDLE computingThread = CreateThread(NULL, 0, sortSimpleResultInBenchResults, (LPVOID)diff, 0, &threadId);
	
	
	//sortSimpleResultInBenchResults(diff);

	T1 = T2;
}

/* -------------------------------------------------
	main thread
   ------------------------------------------------- */
int _tmain(int argc, _TCHAR* argv[])
{
	DWORD Ti;
	HANDLE hThread;
	HANDLE hEvtStop;
	UINT TimerID;

	
	SECOND_IN_MICRO = 10000;
	CAT_MAX = 70;
	CAT_MIN = -60;
	TOTAL_HITS = 0;
	MAX_PERIOD = 0;
	MIN_PERIOD = 0;
	TOTAL_CYCLES = 0;
	fillBenchResultsCategories();
	printResults();

	// Create utilities
	hEvtStop = CreateEvent(NULL,TRUE,FALSE,NULL);
	hThread = CreateThread(NULL,0,stopThread,hEvtStop,0,&Ti);
	// prepare time measurement
	QueryPerformanceFrequency(&Freq);
	QueryPerformanceCounter(&T1);

	// set best timer resolution
	timeBeginPeriod(1);
	
	// start timer
	TimerID = timeSetEvent(
					PERIOD,
					RESOLUTION,
					(LPTIMECALLBACK)TimerCallback,
					1,											// timer number
					TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
	
	// wait for signal from stop thread
	WaitForSingleObject(hEvtStop, INFINITE);

	// clear timer
	timeKillEvent(TimerID);
	timeEndPeriod(1);

	printf("Program will stop on keydown");
	getchar();
	printf("bye...");
	return 0;
}