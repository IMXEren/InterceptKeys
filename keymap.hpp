#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

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

	static void removeEmptyEntries(std::vector<KeyMapEntry>& mapEntries) {
		mapEntries.erase(
			std::remove_if(mapEntries.begin(), mapEntries.end(),
				[](const KeyMapEntry& entry) {
					return entry.from.empty();
				}),
			mapEntries.end());
	}

	static bool are_from_keys_same(KeyMapEntry& self, KeyMapEntry& other) {
		bool size_and_regular_key_same = self.from.size() == other.from.size() &&
			self.from.back() == other.from.back();
		if (size_and_regular_key_same) return size_and_regular_key_same;
		std::unordered_set<int> self_(self.from.begin(), self.from.end());
		std::unordered_set<int> other_(other.from.begin(), other.from.end());
		return self_ == other_;
	}
};

inline std::vector<KeyMapEntry> g_MapEntries = {};
