	

    // TP1_timer.cpp : periodic timer with time measurement
    //
     
    #include "stdafx.h"
     
    // some differences between WinCE and WinNT
    #ifdef UNDER_CE
    #pragma comment(lib,"mmtimer.lib")      // CE timer library
    #else
    #include "windows.h"
    #pragma comment(lib,"winmm.lib")        // NT timer library
    #endif
     
    /* =================================================
            Constants
       ================================================= */
    #define PERIOD          10    // milliseconds
    #define RESOLUTION      0               // best timer quality
     
     
    /* =================================================
            Globals
       ================================================= */
    LARGE_INTEGER T1,T2, Freq;  // time measurement variables
     
    double pourcentage_total = 0;
    char *star = NULL;
    double counter = 0;
    double MIN = -1.00000;
    double MAX = -1.00000;
    /* =================================================
            Structure
       ================================================= */
     
    struct my_time
    {
             double value;
             int cpt;
    };
    my_time *mt = NULL;
     
    void init_my_time(LPVOID t)
    {
            my_time *mt1 = (my_time*) t;
            double value = -60.00000;
           
            star = (char *) malloc(sizeof(char) * 100);
           
            for (int i =0 ; i < 14; i++)
            {
                    mt1[i].value= value;
                    mt1[i].cpt=0;
                    value=value+ 10.00000;
                   
            }
     
    }
    char* transform_int_to_star(int value)
    {
            if (value > 0)
            {
                   
                    int i =0;
                    for ( ; i < value ; i++)
                            star[i] = '*';
                    star[i]= '\0';
                    return star;
            }
            else
                    return "";
    }
    void show_struct()
    {
           
            double affiche = 0;
     
            if (MAX == -1.00000)
                    MAX = counter;
            else if (counter >= MAX)
                    MAX = counter;
     
            if (MIN == -1.00000)
                    MIN = counter;
            else if (counter < MIN)
                    MIN = counter;
     
           
     
            printf("***************************************\n");
            printf("Timer period     : %.2lf\n", counter/1000);
            printf("Period MAX in ms : %.2lf\n", MAX/1000);
            printf("Period MIN in ms : %.2lf\n", MIN/1000);
            printf("***************************************\n");
            printf("Total timer cycles = %d\n", (int)pourcentage_total);
            printf("Total timer loops  = %d\n", (int)pourcentage_total);
            printf("---------------------------------------\n");
            printf("ms              | hits  | pourcentage\n");
            printf("---------------------------------------\n");
            double pourcentage = 0;
            for (int i =0 ; i < 14; i++)
            {
                    affiche = mt[i].value * 0.001 ;
                    pourcentage = (double)(mt[i].cpt / pourcentage_total);
                    pourcentage *= 100;
                    memset(star,0x0,sizeof(char*));
                    printf("%.2lf           %d      %s \n",affiche,mt[i].cpt, transform_int_to_star((int)pourcentage));
                   
            }
    }
    /* -------------------------------------------------
            timer function
       ------------------------------------------------- */
    void CALLBACK TimerCallback(UINT tid, UINT b, DWORD_PTR usr, DWORD_PTR p2, DWORD_PTR p3)
    {
            // measure time and display delay in microseconds
            QueryPerformanceCounter(&T2);
            counter = ((((T2.QuadPart - T1.QuadPart)*1000000) / Freq.QuadPart));
           
            counter = counter - (PERIOD * 1000);
           
            printf("counter %lf \n:", counter);
            for (int i =0 ; i < 14; i++)
            {
                    if (counter <= mt[0].value)
                    {
                            mt[0].cpt+=1;
                            break;
                    }
                    else if (counter >= mt[13].value)
                    {
                            mt[13].cpt+=1;
                            break;
                    }
                    else if (counter >= mt[i].value && counter < mt[i + 1].value )
                    {
                            mt[i].cpt +=1;
                            break;
                    }      
            }
            pourcentage_total += 1;
            show_struct();
            T1 = T2;
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
            main thread
       ------------------------------------------------- */
    int _tmain(int argc, _TCHAR* argv[])
    {
    DWORD Ti;
    HANDLE hThread;
    HANDLE hEvtStop;
    UINT TimerID;
     
     
           
           
            mt = (my_time *) malloc (sizeof(my_time) * 14);
            init_my_time(mt);
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
                                            1,                                                                                      // timer number
                                            TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
           
            // wait for signal from stop thread
            WaitForSingleObject(hEvtStop,INFINITE);
     
            // clear timer
            timeKillEvent(TimerID);
            timeEndPeriod(1);
            free(star);
            free(mt);
            printf("Program will stop in 2 s\n");
            Sleep(2000);
            printf("bye...");
            return 0;
    }

