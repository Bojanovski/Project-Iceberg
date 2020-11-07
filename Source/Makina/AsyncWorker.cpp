
#include "AsyncWorker.h"

using namespace std;
using namespace Makina;

AsyncWorker::AsyncWorker()
: mData(0),
mDataSize(0),
mDoWork(false),
mLoop(true)
{
	mWorkingProcThread = thread(&AsyncWorker::WorkingProc, this);
}

AsyncWorker::~AsyncWorker()
{
	{ // Set flag and signal the AsyncWorker::WorkingProc() that it should end the loop.
		unique_lock<mutex> locker(mLock);
		mLoop = false;
		mStartCheck.notify_one();
	}

	// Wait for it to actually end.
	if (mWorkingProcThread.joinable())
		mWorkingProcThread.join();

	// Delete allocated data.
	if (mData) delete mData;
}

void AsyncWorker::Assign(void *data, size_t dataSize)
{
	unique_lock<mutex> locker(mLock);

	// First wait for previous call to AsyncWorker::Work() to finish.
	while (mDoWork) mFinishCheck.wait(locker);

	// First copy the data, then set the flag and signal the AsyncWorker::WorkingProc()
	// that it can make one call to AsyncWorker::Work().
	mDataSize = dataSize;
	if (mData) delete mData;
	mData = new char[mDataSize];
	for (unsigned int i = 0; i < mDataSize; ++i) ((char *)mData)[i] = ((char *)data)[i];
	mDoWork = true;
	mStartCheck.notify_one();
}

void AsyncWorker::Join()
{
	unique_lock<mutex> locker(mLock);
	while (mDoWork) mFinishCheck.wait(locker);
}

void AsyncWorker::WorkingProc()
{
	while (true)
	{
		{ // Wait for AsyncWorker::Assign() to be called
			unique_lock<mutex> locker(mLock);
			while (!mDoWork)
			{
				if (mLoop)	mStartCheck.wait(locker);
				else		return; // Destructor is called. End the loop.
			}
		}

		// Do the actual work
		Work();

		{ // Set the flag and signal the AsyncWorker::Assign() that AsyncWorker::Work() has finished.
			unique_lock<mutex> locker(mLock);
			mDoWork = false;
			mFinishCheck.notify_one();
		}
	}
}