#pragma once

#include <vector>
#include <unordered_set>

struct KeyMapEntry {
	std::vector<int> from;
	std::vector<int> to;
	std::unordered_set<int> fromKeysDown;
	bool allToKeysDown = false;

	KeyMapEntry() {
		from = {};
		to = {};
	}

	KeyMapEntry(std::vector<int> ikeys, std::vector<int> okeys) {
		from = ikeys;
		to = okeys;
	}

	void clickKey(int keycode) {
		fromKeysDown.insert(keycode);
	}

	void releaseKey(int keycode) {
		fromKeysDown.erase(keycode);
	}

	size_t ctr() const {
		return fromKeysDown.size();
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
