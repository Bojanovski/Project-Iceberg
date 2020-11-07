
#ifndef HOSTED_LIST
#define HOSTED_LIST

#include <cassert>
#include <vector>

namespace Makina
{
	static const int HostedList_End = -1;

	template <class DataType>
	class HostedList
	{
	private:
		struct HostedListElement
		{
			int mNextUsed, mPrevUsed;
			int mNextFree;
			DataType mData;
		};

	public:
		HostedList()
			: mFirstUsed(HostedList_End),
			mLastUsed(HostedList_End),
			mFirstFree(0)
		{
			AddEmpty();
		}

		inline int GetFirstIndex() { return mFirstUsed; }
		inline int GetLastIndex() { return mLastUsed; }
		inline int GetNextIndex(int index) { return mElements[index].mNextUsed; }
		inline int GetPreviousIndex(int index) { return mElements[index].mPrevUsed; }
		inline DataType &operator[](int index) { return mElements[index].mData; }

		int Add(const DataType &data)
		{
			if (mFirstFree == HostedList_End) mFirstFree = AddEmpty();

			int index = mFirstFree;
			int nextIndex = mFirstUsed;
			mElements[index].mNextUsed = nextIndex;
			mElements[index].mPrevUsed = HostedList_End;
			mFirstUsed = index;
			mFirstFree = mElements[index].mNextFree;
			mElements[index].mData = data;

			if (nextIndex != HostedList_End) // connect backwards
				mElements[nextIndex].mPrevUsed = index;

			if (mLastUsed == HostedList_End) // in case that this is the only entry
				mLastUsed = index;

			return index;
		}

		int Remove(int index)
		{
			assert(mElements[index].mNextUsed != -2 && mElements[index].mPrevUsed != -2);
			mElements[index].mNextFree = mFirstFree;
			mFirstFree = index;

			int nextIndex = mElements[index].mNextUsed;
			int prevIndex = mElements[index].mPrevUsed;
			mElements[index].mNextUsed = mElements[index].mPrevUsed = -2;

			if (prevIndex == HostedList_End) // it is first in list
				mFirstUsed = nextIndex;
			else
				mElements[prevIndex].mNextUsed = nextIndex;

			if (nextIndex == HostedList_End) // it is last in list
				mLastUsed = prevIndex;
			else
				mElements[nextIndex].mPrevUsed = prevIndex;

			return nextIndex;
		}

	private:
		int AddEmpty()
		{
			HostedListElement ele;
			ele.mNextUsed = HostedList_End;
			ele.mPrevUsed = HostedList_End;
			ele.mNextFree = HostedList_End; // there is no next free space index
			mElements.push_back(ele);
			return mElements.size() - 1;
		}

		int mFirstUsed, mLastUsed;
		int mFirstFree;
		std::vector<HostedListElement> mElements;
	};
}

#endif
