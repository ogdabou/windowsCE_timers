// TP1_timer.cpp : periodic timer with time measurement
//

#include "stdafx.h"

// some differences between WinCE and WinNT
#ifdef UNDER_CE
#pragma comment(lib,"mmtimer.lib")	// CE timer library
#else
#include "windows.h"
#pragma comment(lib,"winmm.lib")	// NT timer library
#endif

/* =================================================
	Constants
   ================================================= */
#define PERIOD		1000    // milliseconds
#define RESOLUTION	0		// best timer quality 


/* =================================================
	Globals
   ================================================= */
LARGE_INTEGER T1,T2, Freq;  // time measurement variables
unsigned int SECOND_IN_MICRO;

int CAT_MIN, CAT_MAX, TOTAL_HITS;


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

BenchResults benchRes[14];

void printResults()
{
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
			
			benchRes[i].percent = ((float)benchRes[i].hits / (float)TOTAL_HITS) * 100;
			if (DEBUG) printf("%d \n",benchRes[i].percent);
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
		if (result > CAT_MIN && result < CAT_MAX && benchRes[i].category > result)
		{
			if (DEBUG) printf("Diff : %d > %d\n", benchRes[i].category, result);
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
	LONGLONG result = ((T2.QuadPart - T1.QuadPart)*1000000) / Freq.QuadPart;
	// printf("Timer %d - %d us\n", usr, result);
	LONGLONG diff = result - SECOND_IN_MICRO;
  
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

	
	SECOND_IN_MICRO = 1000000;
	CAT_MAX = 70;
	CAT_MIN = -60;
	TOTAL_HITS = 0;
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

	printf("Program will stop in 2 s\n");
	Sleep(2000);
	printf("bye...");
	return 0;
}