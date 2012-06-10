#include "timer.h"
#include <Windows.h>

double Timer::GetCurrentTimeMilli()
{
	LARGE_INTEGER frequency;
	LARGE_INTEGER t;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t);
	return t.QuadPart * 1000.0 / frequency.QuadPart;
}

