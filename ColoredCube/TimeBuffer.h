#pragma once

#include <time.h>

class TimeBuffer {
	private:
		unsigned long begTime;
		//bool isFiring;
	public:
		
		void resetClock() {
			begTime = clock();
		}

		unsigned long elapsedTime() {
			return ((unsigned long) clock() - begTime) / CLOCKS_PER_SEC;
		}
		
		/*bool canFire(unsigned long seconds) {

			return seconds <= elapsedTime();
		}*/

		
};