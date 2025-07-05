#pragma once

#include "common.h"
#include "logger.hpp"

#include "fmt/base.h"
#include "fmt/ostream.h"
#include "quill/HelperMacros.h"

#define TOML_ENABLE_FORMATTERS 0
#define TOML_EXCEPTIONS 0
#include "toml++/toml.hpp"
#include "keymap.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

template <>
struct fmt::formatter<toml::node_type> : fmt::ostream_formatter {};
template <>
struct fmt::formatter<toml::parse_error> : fmt::ostream_formatter {};
template <>
struct fmt::formatter<toml::source_region> : fmt::ostream_formatter {};

QUILL_LOGGABLE_DEFERRED_FORMAT(toml::node_type);
QUILL_LOGGABLE_DEFERRED_FORMAT(toml::parse_error);
QUILL_LOGGABLE_DEFERRED_FORMAT(toml::source_region);

class MapConfig {
private:
	bool found_priority = false;
	int32_t last_priority = 0;
	std::vector<std::pair<int32_t, KeyMapEntry>> mapConfigEntries;
public:
	int32_t default_priority = 1000;
	std::vector<std::string> config_path_list;
	MapConfig(std::vector<std::string> configPathList) : config_path_list(configPathList) {}

	void prefix_path_output(std::string_view config_path) const {
		INTERCEPT_LOGE_N_ERR(gLogger, "[path: {}] ", config_path);
	}

	void prefix_source_output(toml::source_region source) const {
		INTERCEPT_LOGE_N_ERR(gLogger, "[source: At {}]", source);
	}

	void move_map_config_entries_to_mapped_entries() {
		for (auto& map : mapConfigEntries) {
			g_MapEntries.emplace_back(std::move(map.second));
		}
	}

	void load_config() {
		assert(!config_path_list.empty() && "Config path list cannot be empty");
		for (const auto& config_path : config_path_list) {
			toml::parse_result result = toml::parse_file(config_path);
			if (!result) {
				INTERCEPT_LOGE_N_ERR(gLogger, "Parsing failed for file: {}\nError: {}", config_path, result.error());
				continue;
			}
			INTERCEPT_LOGD_N_OUT(gLogger, "Parsed file: {}", config_path);
			toml::array* mappings = result["mappings"].as_array();
			if (!mappings) {
				prefix_path_output(config_path);
				INTERCEPT_LOGE_N_ERR(gLogger, "No 'mappings' array found in file");
				continue;
			}
			int mapping_index = -1;
			for (auto& mapping : *mappings) {
				++mapping_index;
				toml::table* map_table = mapping.as_table();
				if (!map_table) {
					prefix_path_output(config_path);
					INTERCEPT_LOGE_N_ERR(gLogger, "Mapping[{}] is not a table. Type: {}", mapping_index, mapping.type());
					prefix_source_output(mapping.source());
					continue;
				}
				toml::array* from_arr = (*map_table)["from"].as_array();
				toml::array* to_arr = (*map_table)["to"].as_array();
				if (!from_arr || !to_arr) {
					prefix_path_output(config_path);
					INTERCEPT_LOGE_N_ERR(gLogger, "Mapping[{}] must contain both 'from' and 'to' arrays. "
						"Incase of disabling the keystrokes, put an empty 'to' array. This helps remove ambiguity if 'to' is actually disabled.", mapping_index);
					prefix_source_output(map_table->source());
					continue;
				}
				if (from_arr->empty()) {
					prefix_path_output(config_path);
					INTERCEPT_LOGE_N_ERR(gLogger, "Mapping[{}] 'from' array is empty.", mapping_index);
					prefix_source_output(from_arr->source());
					continue;
				}
				if (!(from_arr->size() <= 5 && from_arr->is_homogeneous<int64_t>()) ||
					!(to_arr->size() <= 5 && to_arr->is_homogeneous<int64_t>()))
				{
					prefix_path_output(config_path);
					INTERCEPT_LOGE_N_ERR(gLogger, "Invalid 'from' or 'to' arrays in mapping[{}]. "
						"Expected max 5 keys and all integer values...", mapping_index);
					prefix_source_output(from_arr->source());
					prefix_source_output(to_arr->source());
					continue;
				}

				int32_t priority = default_priority;
				if ((*map_table)["priority"].is_integer()) {
					found_priority = true;
					last_priority = priority = (*map_table)["priority"].as_integer()->get();
				}
				else if (found_priority) {
					last_priority = priority = last_priority + 1;
				}

				std::vector<int> from_entries = toml_array_to_vector<int>(from_arr);
				std::vector<int> to_entries = toml_array_to_vector<int>(to_arr);
				KeyMapEntry entry(from_entries, to_entries);
				auto entry_loaded_template = fmt::format("{}. {} -> {}", mapping_index, entry.from, entry.to);
				INTERCEPT_LOGD_N_OUT(gLogger, "{}", entry_loaded_template);
				mapConfigEntries.emplace_back(std::move(std::pair(priority, entry)));
			}

		}

		// Sort based on priority
		std::sort(mapConfigEntries.begin(), mapConfigEntries.end(),
			[](std::pair<int32_t, KeyMapEntry>& e1,
				std::pair<int32_t, KeyMapEntry>& e2) {
					return e1.first < e2.first;
			});

		deduplicate();
		move_map_config_entries_to_mapped_entries();
	}

	void deduplicate() {
		// Deduplicate if 'from' keys are same
		// Keep the only one with highest priority
		// Iterate from the end, check for similarities, erase and shift
		// Also, sorting is preserved
		for (auto it = mapConfigEntries.rbegin(),
			rend_it = mapConfigEntries.rend(); it != rend_it; ++it) {
			auto it2 = std::next(it);
			while (it2 != rend_it && KeyMapEntry::are_from_keys_same(it->second, it2->second)) {
				it2 = std::next(it2);
				it = std::next(it);
				it = decltype(it)(mapConfigEntries.erase(it.base()));
				// 'it' still points to where it was pointing before and valid
			}
			if (it2 == rend_it) break;
		}
	}

	template<typename T>
	std::vector<T> toml_array_to_vector(const toml::array* arr) {
		std::vector<T> vec;
		if (!arr) return vec;

		for (const auto& node : *arr) {
			if (auto val = node.value<T>())
				vec.push_back(*val);
		}

		return vec;
	}
};