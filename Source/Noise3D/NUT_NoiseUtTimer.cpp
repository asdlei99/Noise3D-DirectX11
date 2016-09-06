
/***********************************************************************

							�ࣺNOISE TIMER

	�������߾��ȼ�ʱ������Ҫ��WINAPI��queryPerformanceCount��ʵ��    

***********************************************************************/
#include "Noise3D.h"

using namespace Noise3D;
using namespace Noise3D::Ut;

ITimer::ITimer(NOISE_TIMER_TIMEUNIT timeUnit = NOISE_TIMER_TIMEUNIT_MILLISECOND)
{
	//Ĭ���ú�����
	mTimeUnit				= timeUnit;
	mMilliSecondsPerCount	= 0.0;
	mDeltaTime				= 0.0;
	mTotalTime				= 0.0;
	mIsPaused				= FALSE;

	//ÿ����������ٴ�
	INT64 countsPerSecond;
	//��ȡ���������ʱ����Ƶ��
	NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	mMilliSecondsPerCount = (1000.0) /(double)countsPerSecond;//ÿһcount���ٺ���
	NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);

}

void ITimer::SelfDestruction()
{
};

void ITimer::NextTick()
{
	if(mIsPaused)
	{
		mDeltaTime = 0.0;
	}
	//���û����ͣ
	else
	{
		//����count
		mPrevCount = mCurrentCount;
		NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);

		//�����ʡ��ģʽ�£����л����������ܻᵼ��countsҲ�Ǹ���
		mDeltaCount = mCurrentCount - mPrevCount;
		BOOL isDeltaTimePositive = ((mDeltaCount) > 0);
		if(isDeltaTimePositive)
		{
			mDeltaTime =(double)(mDeltaCount * mMilliSecondsPerCount);
		}
		else
		{
			mDeltaTime = 0.0;
		};

		//û��ͣ�͸�����ʱ�� ��λ��ms
		mTotalTime += mDeltaTime;

		//...to compute FPS
		++mCurrentSecondTickCount;

		//compute FPS(check if we had fallen into next second)

		if (mCurrentSecondInteger != UINT(mTotalTime/1000.0))
		{
			mFPS = mCurrentSecondTickCount;
			mCurrentSecondTickCount = 0;//reset
			mCurrentSecondInteger = UINT(mTotalTime / 1000.0);
		};
		
	};
}

UINT ITimer::GetFPS() const
{
	return mFPS;
};


double ITimer::GetTotalTime()const
{
	switch(mTimeUnit)
	{
	case NOISE_TIMER_TIMEUNIT_MILLISECOND:
		return mTotalTime; 
		break;
	case NOISE_TIMER_TIMEUNIT_SECOND:
		return (mTotalTime/1000.0); 
		break;
	};
	return 0;
};

double ITimer::GetInterval()const
{
	switch(mTimeUnit)
	{
	case NOISE_TIMER_TIMEUNIT_MILLISECOND:
		return mDeltaTime; 
		break;
	case NOISE_TIMER_TIMEUNIT_SECOND:
		return (mDeltaTime/1000); 
		break;
	};
	return 0;
};

void ITimer::SetTimeUnit(NOISE_TIMER_TIMEUNIT timeUnit)
{
	if (timeUnit ==NOISE_TIMER_TIMEUNIT_SECOND||timeUnit==NOISE_TIMER_TIMEUNIT_MILLISECOND)
	{mTimeUnit = timeUnit;};
};

void ITimer::Pause()
{
	mIsPaused = TRUE;
};

void ITimer::Continue()
{
	mIsPaused = FALSE;
};

void ITimer::ResetAll()
{
	mTotalTime	= 0.0;
	mDeltaTime	= 0.0;
	mIsPaused	= FALSE;
};

void ITimer::ResetTotalTime()
{
	mTotalTime = 0;
};