#pragma once

#include "keys.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

struct KeyStep {
    int code;
    bool is_extended;

	KeyStep() : code(-1), is_extended(false) {}

    KeyStep(int raw_key) {
        is_extended = (raw_key & SC_E0) == SC_E0;
        code = is_extended ? (raw_key & ~SC_E0) : raw_key;
    }

	bool operator==(const KeyStep& other) const {
		return code == other.code && is_extended == other.is_extended;
	}

	int into_raw_key() const {
		int raw_key = is_extended ? (code | SC_E0) : code;
		return raw_key;
	}
};

struct KeyMapEntry {
	std::vector<KeyStep> from_mods;
	KeyStep trigger_key;
	std::vector<KeyStep> to;

	uint8_t from_mod_keys_down = 0b0000;
	uint8_t target_mod_mask    = 0b0000;
	bool all_to_keys_down      = false;

	KeyMapEntry() = default;

    KeyMapEntry(KeyMapEntry&&) noexcept = default;
    KeyMapEntry& operator=(KeyMapEntry&&) noexcept = default;

    KeyMapEntry(const KeyMapEntry&) = default;
    KeyMapEntry& operator=(const KeyMapEntry&) = default;

	KeyMapEntry(std::vector<int> ikeys, std::vector<int> okeys) {
		if (!ikeys.empty()) {
			trigger_key = ikeys.back();
		}
		for (size_t i = 0; i < ikeys.size() - 1; ++i) {
			from_mods.emplace_back(ikeys[i]);
		}
		for (size_t i = 0; i < okeys.size(); ++i) {
			to.emplace_back(okeys[i]);
		}
		target_mod_mask = (1 << from_mods.size()) - 1;
	}

	void click_key(int key_index) {
		from_mod_keys_down |= (1 << key_index);
	}

	void release_key(int key_index) {
		from_mod_keys_down &= ~(1 << key_index);
	}

	bool modifiers_matched() const {
        return from_mod_keys_down == target_mod_mask;
    }

	bool is_from_empty() const {
		return trigger_key.code == -1;
	}

	size_t from_size() const {
		return from_mods.size() + 1;
	}

	size_t to_size() const {
		return to.size();
	}

	std::vector<int> raw_from_keys() const {
		std::vector<int> keys;
		keys.reserve(from_size());
		for (auto& ks : from_mods) {
			keys.push_back(ks.into_raw_key());
		}
		keys.push_back(trigger_key.into_raw_key());
		return keys;
	}

	std::vector<int> raw_to_keys() const {
		std::vector<int> keys;
		keys.reserve(to_size());
		for (auto& ks : to) {
			keys.push_back(ks.into_raw_key());
		}
		return keys;
	}

	static void remove_empty_entries(std::vector<KeyMapEntry>& map_entries) {
		map_entries.erase(
			std::remove_if(map_entries.begin(), map_entries.end(),
				[](const KeyMapEntry& entry) {
					return entry.is_from_empty();
				}),
			map_entries.end());
	}

	static bool are_from_keys_same(KeyMapEntry& self, KeyMapEntry& other) {
		if (self.from_size() != other.from_size() || !(self.trigger_key == other.trigger_key)) {
            return false;
        }
		// Nested check is cheap because modifiers max len is 4
		for (const auto& self_mod : self.from_mods) {
            bool found = std::any_of(other.from_mods.begin(), other.from_mods.end(),
                [&self_mod](const KeyStep& other_mod) {
                    return self_mod == other_mod;
                });
            if (!found) return false;
        }
		return true;
	}
};

inline std::vector<KeyMapEntry> g_MapEntries = {};
