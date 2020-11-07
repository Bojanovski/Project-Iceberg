
#include <random>

namespace Makina
{
	class PCGObject
	{
	public:
		PCGObject(int seed)
			:mSeed(seed)
		{
			mMT.seed(seed);
		}

	protected:
		// A Mersenne Twister pseudo-random generator of 32-bit numbers with a state size of 19937 bits.
		std::mt19937 mMT;
		// Seed used to initialize Mersenne Twister pseudo-random generator.
		const int mSeed;
	};
}