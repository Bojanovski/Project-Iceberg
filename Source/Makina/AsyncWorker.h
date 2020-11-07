
#ifndef ASYNCWORKER_H
#define ASYNCWORKER_H

#include <mutex>
#include <thread>
#include <condition_variable>

namespace Makina
{
	class AsyncWorker
	{
	public:
		__declspec(dllexport) AsyncWorker();
		__declspec(dllexport) virtual ~AsyncWorker();

		// Assign a job that will be done asynchronously.
		__declspec(dllexport) void Assign(void *data, size_t dataSize);

		// Wait for worker to finish it's assignment.
		__declspec(dllexport) void Join();

	protected:
		void *mData;
		size_t mDataSize;

	private:
		__declspec(dllexport) void WorkingProc();
		virtual void Work() = 0;

		std::mutex mLock;
		std::condition_variable mStartCheck;
		std::condition_variable mFinishCheck;
		std::thread mWorkingProcThread;
		bool mDoWork;
		bool mLoop;
	};
}

#endif