#ifndef __TIMER_H__
#define __TIMER_H__

class Timer
{
private:

	Timer() {}
	DISALLOW_COPY_AND_ASSIGN(Timer);

public:

	static double GetCurrentTimeMilli();

};

#endif // __TIMER_H__