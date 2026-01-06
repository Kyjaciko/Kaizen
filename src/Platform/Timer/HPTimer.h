////////////////////////////////////////////////////////////////////////////////
// Filename: HPTimer.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _HPTIMER_H_
#define _HPTIMER_H_

///////////////////////
// LINKING LIBRARIES //
///////////////////////

#pragma comment(lib, "winmm.lib")

//////////////
// INCLUDES //
//////////////

#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: HPTimer (aka The Time-Turner)
////////////////////////////////////////////////////////////////////////////////

class HPTimer
{
public:
	HPTimer() 
	{ 
		LARGE_INTEGER t; 
		QueryPerformanceFrequency(&t);
		frequency = t.QuadPart;

		Reset();
	}

	void Update()
	{
		lastCallToUpdate = currentCallToUpdate;
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		currentCallToUpdate = t.QuadPart;

		// FPS limiter -- START
		timeBeginPeriod(1); // Best to call this only once during the whole live of the APP, heavy function
		int TARGET_FPS = 144;
		double elapsedTime = GetDeltaTime();
		while (elapsedTime < 1.0 / (TARGET_FPS + 0.5))
		{
			Sleep(1);

			QueryPerformanceCounter(&t);
			currentCallToUpdate = t.QuadPart;
			elapsedTime = GetDeltaTime();
		}
		timeEndPeriod(1);
		// FPS limiter -- END
	}

	void Reset()
	{
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		startTime = t.QuadPart;
		currentCallToUpdate = t.QuadPart;
		lastCallToUpdate = t.QuadPart;
	}

	// Time from Reset(), floats are possible for performance increase.
	double GetTotalTime()
	{
		double d = currentCallToUpdate - startTime;
		return d / frequency;
	}

	// Time between Last Frame, in seconds.
	double GetDeltaTime()
	{
		double d = currentCallToUpdate - lastCallToUpdate;
		return d / frequency;
	}

private:
	long long	startTime;
	long long	lastCallToUpdate;
	long long	currentCallToUpdate;
	long long	frequency;
};

#endif // !_HPTIMER_H_

