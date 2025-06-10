#pragma once

#include <vector>
#include <intrin.h>

struct KeyMapEntry {
	std::vector<int> from;
	std::vector<int> to;
	int8_t fromModKeysDown = 0b0000;
	bool allToKeysDown = false;

	KeyMapEntry() {
		from = {};
		to = {};
	}

	KeyMapEntry(std::vector<int> ikeys, std::vector<int> okeys) {
		from = ikeys;
		to = okeys;
	}

	void clickKey(int keycode, int fromKeyIndex) {
		fromModKeysDown = fromModKeysDown | (1 << fromKeyIndex);
	}

	void releaseKey(int keycode, int fromKeyIndex) {
		fromModKeysDown = ~(~fromModKeysDown | 1 << fromKeyIndex);
	}

	int8_t ctr() const {
		return (int8_t)__popcnt16(fromModKeysDown);
	}

	static void removeEmptyEntries(std::vector<KeyMapEntry> mapEntries) {
		std::vector<int> emptyEntriesIndex;
		int entryIndex = 0;
		for (auto& entry : mapEntries) {
			if (entry.from.empty()) {
				emptyEntriesIndex.push_back(entryIndex);
			}
		}

		for (auto& index : emptyEntriesIndex) {
			mapEntries.erase(mapEntries.begin() + index);
		}
	}
};
