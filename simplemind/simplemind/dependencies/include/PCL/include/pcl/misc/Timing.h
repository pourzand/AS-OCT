#ifndef PCL_TIMING
#define PCL_TIMING

#include <time.h>
#include <stdio.h>
#include <ctime>

namespace pcl
{

	class Timing {
	public:
		static void Tic()
		{
			getStaticStart() = clock(); 
		}
		static double Toc()
		{
			clock_t end = clock();
			return (end-getStaticStart())/double(CLOCKS_PER_SEC);
		}

		Timing() {}

		void tic() 
		{ 
			m_Start = clock(); 
		}
		Timing& toc() 
		{ 
			m_End = clock(); 
			return *this;
		}

		clock_t getClock() { return m_End-m_Start; }
		double getClockInSeconds() { return (m_End-m_Start)/double(CLOCKS_PER_SEC); }

	protected:
		clock_t m_Start, m_End;
		static clock_t& getStaticStart() {
			static clock_t static_start;
			return static_start;
		}
	};

}

#endif