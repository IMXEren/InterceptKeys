#include "common.h"
#include "interception.h"
#include "keys.h"
#include "keymap.hpp"
#include "Service.hpp"
#include "Cli.hpp"
#include "utils.hpp"

#include <sstream>
#include <string>

int InterceptKeys() {
	InterceptionContext context = interception_create_context();
	InterceptionDevice device;

	interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

	// Used in detectKeysOnly mode
	bool detectKeysOnly = false;
	size_t noOfClicks = 0;
	if (gCli.detect_keys_only_flag) {
		detectKeysOnly = true;
	}

	KeyMapEntry::removeEmptyEntries(g_MapEntries);
	if (g_MapEntries.empty()) {
		INTERCEPT_LOGD_N_OUT(gLogger, "No key mappings found!");
		goto cleanup;
	}

	utils::process::raise_process_priority();

	while (g_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		InterceptionStroke stroke;
		INTERCEPT_LOG_DEBUG(gLogger, "Waiting on context...");
		device = interception_wait(context);

		if (interception_is_keyboard(device)) {
			INTERCEPT_LOG_DEBUG(gLogger, "Waiting for key stroke...");
			interception_receive(context, device, &stroke, 1);
			InterceptionKeyStroke* kstroke = (InterceptionKeyStroke*)&stroke;

			int code = kstroke->code;
			int state = kstroke->state;
			bool is_down = (state & INTERCEPTION_KEY_UP) == 0;
			bool e0 = (state & INTERCEPTION_KEY_E0) != 0; // Extended key flag

			if (detectKeysOnly) {
				std::stringstream _scanCode;
				std::string scanCode;
				int codeWithFlags = code;
				if (e0) {
					codeWithFlags |= (0xE0 << 8);
				}
				_scanCode << "0x" << std::hex << std::uppercase << codeWithFlags;
				_scanCode.flags(std::ios::fmtflags());
				scanCode = _scanCode.str();

				// Only on click as to not cause confusion
				if (is_down) {
					INTERCEPT_LOGD_N_OUT(gLogger, "{}. Pressed key: {} ({}), click: {}, state: {}", ++noOfClicks, code, scanCode, is_down, state);
				}
				interception_send(context, device, &stroke, 1);
				continue;
			}

			// Iterating entries to find the one that matches
			// If the entry is found, send the mapped key stroke
			bool sendMappedKeyStroke = false;
			for (auto& entry : g_MapEntries) {
				bool foundModifierKey = false;
				bool foundRegularKey = false;

				int fromKeyIndex = 0;
				for (int fromKey : entry.from) {
					int e0bits = fromKey & SC_E0;
					bool extended = e0bits == SC_E0;
					if (extended)
						fromKey &= ~SC_E0;
					INTERCEPT_LOGD_N_OUT(gLogger, "From key[{}]: {}, extended: {}, finding key: {}, e0: {}",
						fromKeyIndex, fromKey, extended, code, e0);

					bool isLast = fromKeyIndex == entry.from.size() - 1;

					// Regular key
					if (isLast) {
						foundRegularKey = fromKey == code && extended == e0;
						INTERCEPT_LOGD_N_OUT(gLogger, "Reached last key! found regular key? {}, ctr: {}, entry.from.size() - 1: {}",
							foundRegularKey, entry.ctr(), entry.from.size() - 1);
						if (foundRegularKey && entry.ctr() == entry.from.size() - 1) {
							// This key should be suppressed
							// Release the modifier keys
							// Send the mapped key combo
							sendMappedKeyStroke = true;
							INTERCEPT_LOGD_N_OUT(gLogger, "Pressed regular key: {}, click: {}, state: {}",
								code, is_down, state);
						}
					}
					// Modifier combo sequence can be random
					else if (fromKey == code && extended == e0) {
						foundModifierKey = true;
						if (is_down)
							entry.clickKey(code, fromKeyIndex);
						else
							entry.releaseKey(code, fromKeyIndex);
						INTERCEPT_LOGD_N_OUT(gLogger, "Pressed modifier key: {}, click: {}, state: {}",
							code, is_down, state);
					}

					if (foundModifierKey || foundRegularKey)
						break;
					fromKeyIndex++;
				}

				if (sendMappedKeyStroke) {
					// Release until regular key
					int fromKeyIndex = 0;
					InterceptionKeyStroke mkfstroke;
					for (int fromKey : entry.from) {
						if (fromKeyIndex == entry.from.size() - 1)
							break;

						int e0bits = fromKey & SC_E0;
						bool extended = e0bits == SC_E0;
						if (extended)
							fromKey &= ~SC_E0;

						mkfstroke.code = fromKey;
						mkfstroke.state = INTERCEPTION_KEY_UP;
						if (extended)
							mkfstroke.state |= INTERCEPTION_KEY_E0;

						interception_send(context, device, (InterceptionStroke*)&mkfstroke, 1);
						fromKeyIndex++;
					}

					// Send mapped combo
					InterceptionKeyStroke mktstroke;
					auto sendToKey = [&](int toKey) {
						int e0bits = toKey & SC_E0;
						bool extended = e0bits == SC_E0;
						if (extended)
							toKey &= ~SC_E0;

						mktstroke.code = toKey;
						if (is_down)
							mktstroke.state = INTERCEPTION_KEY_DOWN;
						else
							mktstroke.state = INTERCEPTION_KEY_UP;
						if (extended)
							mktstroke.state |= INTERCEPTION_KEY_E0;

						interception_send(context, device, (InterceptionStroke*)&mktstroke, 1);
						INTERCEPT_LOGD_N_OUT(gLogger, "Sending key: {}, click: {}, state: {}",
							mktstroke.code, is_down, mktstroke.state);
						};

					// Send down for all keys in exact order
					if (is_down) {
						// All keys are pressed once
						// Now only press the regular last key
						if (entry.allToKeysDown) {
							sendToKey(entry.to.back());
						}
						else {
							entry.allToKeysDown = true;
							for (auto toKey : entry.to) {
								sendToKey(toKey);
							}
						}
					}
					// Send up for all keys in reverse order
					else {
						entry.allToKeysDown = false;
						for (auto toKeyIt = entry.to.rbegin(); toKeyIt != entry.to.rend();
							++toKeyIt) {
							sendToKey(*toKeyIt);
						}
					}

					if (entry.to.empty()) {
						INTERCEPT_LOGD_N_OUT(gLogger, "Sending key: disabled");
					}

					break;
				};
			}

			if (sendMappedKeyStroke)
				continue;
		}

		// Default: pass through
		interception_send(context, device, &stroke, 1);
	}

cleanup:
	interception_destroy_context(context);
	return 0;
}