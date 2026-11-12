#include "Random.h"
#include <stdlib.h> 
#include <time.h> 

namespace Random {

	void Init() {
		srand(static_cast<unsigned int>(time(nullptr)));
	}

	int Range(int min, int max) {
		return min + rand() % (max - min + 1);
	}

	float RangeF(float min, float max) {
		return min + (max - min) * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
	}

	bool Chance(int percent) {
		return rand() % 100 < percent;
	}

} // namespace Random