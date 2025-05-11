#include "keymap.hpp"
#include "keys.h"

std::vector<KeyMapEntry> mapEntries = {
	KeyMapEntry(KEYS(SC_LWIN | SC_E0, SC_LSHIFT, SC_F23), KEYS(SC_LCTRL | SC_E0)),
};
