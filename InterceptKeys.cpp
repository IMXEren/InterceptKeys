#include <interception.h>
#include <windows.h>
#include <stdio.h>

#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) 
#endif

#define KEYS(...) std::vector<int>({__VA_ARGS__})
#define SC_LWIN   0x5B
#define SC_LSHIFT 0x2A
#define SC_F23    0x6E
#define SC_LCTRL  0x1D
#define SC_E0    (0xE0 << 8)


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

std::vector<KeyMapEntry> mapEntries = {
   KeyMapEntry(KEYS(SC_LWIN | SC_E0, SC_LSHIFT, SC_F23), KEYS(SC_LCTRL | SC_E0)),
};


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HANDLE hMutex = CreateMutexA(nullptr, FALSE, "Global\\InterceptKeysMutex");
	if (hMutex == nullptr) {
		//MessageBoxA(nullptr, "Failed to create mutex.", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		//MessageBoxA(nullptr, "Another instance is already running.", "Instance Detected", MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}

	int main();
	int status = main();
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return status;
}

int main() {
	InterceptionContext context = interception_create_context();
	InterceptionDevice device;

	interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

	while (1) {
		InterceptionStroke stroke;
		device = interception_wait(context);

		if (interception_is_keyboard(device)) {
			interception_receive(context, device, &stroke, 1);
			InterceptionKeyStroke* kstroke = (InterceptionKeyStroke*)&stroke;

			int code = kstroke->code;
			int state = kstroke->state;
			bool is_down = (state & INTERCEPTION_KEY_UP) == 0;
			bool e0 = (state & INTERCEPTION_KEY_E0) != 0; // Extended key flag
			//DEBUG_PRINT("Pressed key: %d, click: %d, state: %d\n", code, is_down, state);

			// Iterating entries to find the one that matches
			// If the entry is found, send the mapped key stroke
			int entryIndex = 0;
			bool sendMappedKeyStroke = false;
			for (auto& entry : mapEntries) {
				if (entry.from.empty()) mapEntries.erase(mapEntries.begin() + entryIndex);

				bool foundModifierKey = false;
				bool foundRegularKey = false;

				int fromKeyIndex = 0;
				for (int fromKey : entry.from) {
					int e0bits = fromKey & SC_E0;
					bool extended = e0bits == SC_E0;
					if (extended) fromKey -= SC_E0;
					//DEBUG_PRINT("From key[%d]: %d, extended: %d, finding key: %d, e0: %d\n", fromKeyIndex, fromKey, extended, code, e0);

					bool isLast = fromKeyIndex == entry.from.size() - 1;

					// Regular key
					if (isLast) {
						foundRegularKey = fromKey == code && extended == e0;
						if (foundRegularKey && entry.ctr == entry.from.size() - 1) {
							// This key should be suppressed
							// Release the modifier keys
							// Send the mapped key combo
							sendMappedKeyStroke = true;
							DEBUG_PRINT("Pressed regular key: %d, click: %d, state: %d\n", code, is_down, state);
						}
					}
					// Modifier combo sequence can be random
					else if (fromKey == code && extended == e0) {
						foundModifierKey = true;
						if (is_down) entry.ctr++;
						else entry.ctr--;
						DEBUG_PRINT("Pressed modifier key: %d, click: %d, state: %d\n", code, is_down, state);
					}

					if (foundModifierKey || foundRegularKey) break;
					fromKeyIndex++;
				}

				if (sendMappedKeyStroke) {
					// Release until regular key
					int fromKeyIndex = 0;
					InterceptionKeyStroke mkfstroke;
					for (int fromKey : entry.from) {
						if (fromKeyIndex == entry.from.size() - 1) break;

						int e0bits = fromKey & SC_E0;
						bool extended = e0bits == SC_E0;
						if (extended) fromKey -= SC_E0;

						mkfstroke.code = fromKey;
						mkfstroke.state = INTERCEPTION_KEY_UP;
						if (extended) mkfstroke.state |= INTERCEPTION_KEY_E0;

						interception_send(context, device, (InterceptionStroke*)&mkfstroke, 1);
						fromKeyIndex++;
					}

					// Send mapped combo
					InterceptionKeyStroke mktstroke;
					auto sendToKey = [&](int toKey) {
						int e0bits = toKey & SC_E0;
						bool extended = e0bits == SC_E0;
						if (extended) toKey -= SC_E0;

						mktstroke.code = toKey;
						if (is_down)
							mktstroke.state = INTERCEPTION_KEY_DOWN;
						else
							mktstroke.state = INTERCEPTION_KEY_UP;
						if (extended) mktstroke.state |= INTERCEPTION_KEY_E0;

						interception_send(context, device, (InterceptionStroke*)&mktstroke, 1);
						DEBUG_PRINT("Sending key: %d, click: %d, state: %d\n", mktstroke.code, is_down, mktstroke.state);
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
						for (auto toKeyIt = entry.to.rbegin(); toKeyIt != entry.to.rend(); ++toKeyIt) {
							sendToKey(*toKeyIt);
						}
					}

					if (entry.to.empty())
						DEBUG_PRINT("Sending key: disabled\n");

					break;
				};
				entryIndex++;
			}

			if (sendMappedKeyStroke)
				continue;

		}

		// Default: pass through
		interception_send(context, device, &stroke, 1);
	}

	interception_destroy_context(context);
	return 0;
}