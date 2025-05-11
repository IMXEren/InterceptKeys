#pragma once

#include <vector>

struct KeyMapEntry {
	std::vector<int> from;
	std::vector<int> to;
	int ctr = 0;
	bool allToKeysDown = false;

	KeyMapEntry() {
		from = {};
		to = {};
	}

	KeyMapEntry(std::vector<int> ikeys, std::vector<int> okeys) {
		from = ikeys;
		to = okeys;
	}
};
