#include "common.h"
#include "interception.h"
#include "keys.h"
#include "keymap.hpp"
#include "Service.hpp"
#include "utils.hpp"

#include <cstddef>

static void send_key(InterceptionContext context, InterceptionDevice device, const KeyStep& ks, bool is_down) {
	InterceptionKeyStroke mktstroke;
	mktstroke.code = ks.code;
	if (is_down)
		mktstroke.state = INTERCEPTION_KEY_DOWN;
	else
		mktstroke.state = INTERCEPTION_KEY_UP;
	if (ks.is_extended)
		mktstroke.state |= INTERCEPTION_KEY_E0;

	interception_send(context, device, (InterceptionStroke*)&mktstroke, 1);
	INTERCEPT_LOGD_N_OUT(g_Logger, "Sending key: {}, click: {}, state: {}",
		mktstroke.code, is_down, mktstroke.state);
}

static void send_mapped_combo(InterceptionContext context, InterceptionDevice device,
                              KeyMapEntry& entry, bool is_down) {
	// Release all modifier keys
	InterceptionKeyStroke mkfstroke;
	for (const auto& ks : entry.from_mods) {
		mkfstroke.code = ks.code;
		mkfstroke.state = INTERCEPTION_KEY_UP;
		if (ks.is_extended)
			mkfstroke.state |= INTERCEPTION_KEY_E0;

		interception_send(context, device, (InterceptionStroke*)&mkfstroke, 1);
		INTERCEPT_LOGD_N_OUT(g_Logger, "Releasing key: {}, state: {}, extended: {}",
			mkfstroke.code, mkfstroke.state, ks.is_extended);
	}

	// Send mapped combo
	// Send down for all keys in exact order
	if (is_down) {
		// All keys are pressed once
		// Now only press the regular last key
		if (entry.all_to_keys_down) {
			send_key(context, device, entry.to.back(), is_down);
		} else {
			entry.all_to_keys_down = true;
			for (const auto& ks : entry.to)
				send_key(context, device, ks, is_down);
		}
	}
	// Send up for all keys in reverse order
	else {
		entry.all_to_keys_down = false;
		for (auto it = entry.to.rbegin(); it != entry.to.rend(); ++it)
			send_key(context, device, *it, false);
	}

	if (entry.to.empty())
		INTERCEPT_LOGD_N_OUT(g_Logger, "Sending key: disabled");
}

static bool try_match_entry(KeyMapEntry& entry, InterceptionKeyStroke* kstroke) {
	int code = kstroke->code;
	int state = kstroke->state;
	bool is_down = (state & INTERCEPTION_KEY_UP) == 0;
	bool e0 = (state & INTERCEPTION_KEY_E0) != 0;

	// All modifiers matched, check if this key is the trigger key
	if (entry.modifiers_matched() && entry.trigger_key.code == code && entry.trigger_key.is_extended == e0) {
		return true;
	}

	// Process modifier keys
	for (size_t i = 0; i < entry.from_mods.size(); ++i) {
		auto& ks = entry.from_mods[i];
		if (ks.code == code && ks.is_extended == e0) {
			if (is_down)
				entry.click_key(i);
			else
				entry.release_key(i);
			return false;
		}
	}
	return false;
}

int DetectKeysOnly() {
	InterceptionContext context = interception_create_context();
	InterceptionDevice device;
	InterceptionStroke stroke;

	interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

	size_t clicks = 0;
	while (g_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		device = interception_wait(context);
		if (interception_is_keyboard(device)) {
			interception_receive(context, device, &stroke, 1);
			InterceptionKeyStroke* kstroke = (InterceptionKeyStroke*)&stroke;

			int code = kstroke->code;
			int state = kstroke->state;
			bool is_down = (state & INTERCEPTION_KEY_UP) == 0;
			bool e0 = (state & INTERCEPTION_KEY_E0) != 0;

			int code_ext = code;
			if (e0) {
				code_ext |= SC_E0;
			}
			// Only on click as to not cause confusion
			if (is_down) {
				INTERCEPT_LOGD_N_OUT(g_Logger, "{}. Pressed key: {} (0x{:X}), click: {}, state: {}", ++clicks, code, code_ext, is_down, state);
			}
			interception_send(context, device, &stroke, 1);
		}
	}

	return 0;
}

int InterceptKeys() {
	InterceptionContext context = interception_create_context();
	InterceptionDevice device;

	interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

	KeyMapEntry::remove_empty_entries(g_MapEntries);
	if (g_MapEntries.empty()) {
		INTERCEPT_LOGD_N_OUT(g_Logger, "No key mappings found!");
		goto cleanup;
	}

	utils::process::raise_process_priority();

	while (g_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		InterceptionStroke stroke;
		INTERCEPT_LOG_DEBUG(g_Logger, "Waiting for device...");
		device = interception_wait(context);

		if (!interception_is_keyboard(device)) {
			continue;
		}

		// Only process key strokes
		INTERCEPT_LOG_DEBUG(g_Logger, "Waiting for key stroke...");
		interception_receive(context, device, &stroke, 1);
		InterceptionKeyStroke* kstroke = (InterceptionKeyStroke*)&stroke;

		int code = kstroke->code;
		int state = kstroke->state;
		bool is_down = (state & INTERCEPTION_KEY_UP) == 0;
		bool e0 = (state & INTERCEPTION_KEY_E0) != 0; // Extended key flag

		// Iterating entries to find the one that matches
		// If the entry is found, send the mapped key stroke
		bool sent_mapped_key_stroke = false;

		for (auto& entry : g_MapEntries) {
			if (try_match_entry(entry, kstroke)) {
				INTERCEPT_LOGD_N_OUT(g_Logger, "Found regular key! {}, will modify it to press the 'to' keys", code);
				send_mapped_combo(context, device, entry, is_down);
				sent_mapped_key_stroke = true;
				break;
			}
		}

		if (sent_mapped_key_stroke)
			continue;

		// Default: pass through
		interception_send(context, device, &stroke, 1);
	}

cleanup:
	interception_destroy_context(context);
	return 0;
}