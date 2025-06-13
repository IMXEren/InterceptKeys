#include "common.h"
#include "interception.h"
#include "keys.h"
#include "keymap.hpp"
#include "Service.hpp"

#include <sstream>
#include <string>

extern std::vector<KeyMapEntry> mapEntries;

int InterceptMain(int argc, char* argv[]) {
	InterceptionContext context = interception_create_context();
	InterceptionDevice device;

	interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

#ifdef _DEBUG
	bool detectKeysOnly = false;
	size_t noOfClicks = 0;

	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			std::string arg = argv[i];
			if (arg == "-h" || arg == "--help") {
				printf("Usage: InterceptKeys.exe [options]\n");
				printf("Options:\n");
				printf("  -h, --help\tShow this help message\n");
				printf("  --detect-keys-only\tHelper to detect key clicks to find the necessary scancode.\n");
				return 0;
			}

			if (arg == "--detect-keys-only") {
				detectKeysOnly = true;
			}
		}
	}
#endif // _DEBUG

	KeyMapEntry::removeEmptyEntries(mapEntries);
	if (mapEntries.empty()) {
		DEBUG_PRINT("No key mappings found!\n");
		LOG_DEBUG(gLogger, "No key mappings found!");
		goto cleanup;
	}

	while (g_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		InterceptionStroke stroke;
		LOG_DEBUG(gLogger, "Waiting on context...");
		device = interception_wait(context);

		if (interception_is_keyboard(device)) {
			LOG_DEBUG(gLogger, "Waiting for key stroke...");
			interception_receive(context, device, &stroke, 1);
			InterceptionKeyStroke* kstroke = (InterceptionKeyStroke*)&stroke;

			int code = kstroke->code;
			int state = kstroke->state;
			bool is_down = (state & INTERCEPTION_KEY_UP) == 0;
			bool e0 = (state & INTERCEPTION_KEY_E0) != 0; // Extended key flag

#ifdef _DEBUG
			if (detectKeysOnly) {
				std::stringstream _scanCode;
				std::string scanCode;
				_scanCode << std::showbase << std::hex << std::uppercase << code;
				_scanCode.flags(std::ios::fmtflags());
				if (e0) {
					_scanCode << " | SC_E0";
				}
				scanCode = _scanCode.str();

				// Only on click as to not cause confusion
				if (is_down) {
					DEBUG_PRINT("%zd. Pressed key: %d (%s), click: %d, state: %d\n", ++noOfClicks, code, scanCode.c_str(), is_down, state);
					LOG_DEBUG(gLogger, "{}. Pressed key: {} ({}), click: {}, state: {}", ++noOfClicks, code, scanCode, is_down, state);
				}
				interception_send(context, device, &stroke, 1);
				continue;
			}
#endif // _DEBUG

			// Iterating entries to find the one that matches
			// If the entry is found, send the mapped key stroke
			bool sendMappedKeyStroke = false;
			for (auto& entry : mapEntries) {
				bool foundModifierKey = false;
				bool foundRegularKey = false;

				int fromKeyIndex = 0;
				for (int fromKey : entry.from) {
					int e0bits = fromKey & SC_E0;
					bool extended = e0bits == SC_E0;
					if (extended)
						fromKey -= SC_E0;
					DEBUG_PRINT("From key[%d]: %d, extended: %d, finding key: %d, e0: %d\n",
						fromKeyIndex, fromKey, extended, code, e0);
					LOG_DEBUG(gLogger,"From key[{}]: {}, extended: {}, finding key: {}, e0: {}",
						fromKeyIndex, fromKey, extended, code, e0);

					bool isLast = fromKeyIndex == entry.from.size() - 1;

					// Regular key
					if (isLast) {
						foundRegularKey = fromKey == code && extended == e0;
						DEBUG_PRINT("Reached last key! found regular key: %d, ctr: %d, entry.from.size() - 1: %zd\n",
							foundRegularKey, entry.ctr(), entry.from.size() - 1);
						LOG_DEBUG(gLogger,"Reached last key! found regular key: {}, ctr: {}, entry.from.size() - 1: {}",
							foundRegularKey, entry.ctr(), entry.from.size() - 1);
						if (foundRegularKey && entry.ctr() == entry.from.size() - 1) {
							// This key should be suppressed
							// Release the modifier keys
							// Send the mapped key combo
							sendMappedKeyStroke = true;
							DEBUG_PRINT("Pressed regular key: %d, click: %d, state: %d\n",
								code, is_down, state);
							LOG_DEBUG(gLogger,"Pressed regular key: {}, click: {}, state: {}",
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
						DEBUG_PRINT("Pressed modifier key: %d, click: %d, state: %d\n",
							code, is_down, state);
						LOG_DEBUG(gLogger,"Pressed modifier key: {}, click: {}, state: {}",
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
							fromKey -= SC_E0;

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
							toKey -= SC_E0;

						mktstroke.code = toKey;
						if (is_down)
							mktstroke.state = INTERCEPTION_KEY_DOWN;
						else
							mktstroke.state = INTERCEPTION_KEY_UP;
						if (extended)
							mktstroke.state |= INTERCEPTION_KEY_E0;

						interception_send(context, device, (InterceptionStroke*)&mktstroke, 1);
						DEBUG_PRINT("Sending key: %d, click: %d, state: %d\n",
							mktstroke.code, is_down, mktstroke.state);
						LOG_DEBUG(gLogger,"Sending key: {}, click: {}, state: {}",
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
						DEBUG_PRINT("Sending key: disabled\n");
						LOG_DEBUG(gLogger, "Sending key: disabled");
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