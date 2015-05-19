#ifndef PERFORMANCETIMER_H
#define PERFORMANCETIMER_H

#include "windows.h"

// Таймер для измерения производительности выполнения операций
// Пример использования:
// CTimer timer;
// timer.Start();
// ...
// <Выполнение операций>
// ...
// quint64 ticks = timer.Ticks_after_Start();
// double time = timer.Time();

class CTimer {
protected:
    LARGE_INTEGER F, T1;
public:
    CTimer() {QueryPerformanceFrequency(&F); T1.QuadPart=0;}

    //__fastcall
    inline quint64 GetFrequency(void) {return F.QuadPart;}

    //__fastcall
    inline quint64 GetCounter(void) {
        register LARGE_INTEGER T2;
        QueryPerformanceCounter(&T2);
        return T2.QuadPart;
    }

    //__fastcall
    inline void Start(void) {QueryPerformanceCounter(&T1);}

    //__fastcall
    inline quint64 Ticks_after_Start(void) {
        register LARGE_INTEGER T2;
        QueryPerformanceCounter(&T2);
        return T2.QuadPart-T1.QuadPart;
    }

    // __fastcall
    inline double Time(void) {
        LARGE_INTEGER T2;
        QueryPerformanceCounter(&T2);
        //QueryPerformanceFrequency(&F);
        return (double)(T2.QuadPart-T1.QuadPart)/F.QuadPart;
    }
};

#endif // PERFORMANCETIMER_H
