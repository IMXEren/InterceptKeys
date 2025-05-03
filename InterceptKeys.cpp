#include <interception.h>
#include <windows.h>
#include <stdio.h>

#define SC_LWIN   0x5B
#define SC_LSHIFT 0x2A
#define SC_F23    0x6E
#define SC_LCTRL  0x1D

int main();

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

	int status = main();
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return status;
}

int main() {
	InterceptionContext context = interception_create_context();
	InterceptionDevice device;

	interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

	bool lwin_down = false;
	bool lshift_down = false;

	while (1) {
		InterceptionStroke stroke;
		device = interception_wait(context);

		if (interception_is_keyboard(device)) {
			interception_receive(context, device, &stroke, 1);
			InterceptionKeyStroke* kstroke = (InterceptionKeyStroke*)&stroke;

			int code = kstroke->code;
			int state = kstroke->state;

			/*if (code == SC_LCTRL) {
				printf("Pressed LCtrl: %d\n", state);
				InterceptionKeyStroke rctrl_stroke = { SC_LCTRL, state | INTERCEPTION_KEY_E0 };
				interception_send(context, device, (InterceptionStroke*)&rctrl_stroke, 1);
				continue;
			}*/


			bool is_down = (state & INTERCEPTION_KEY_UP) == 0;
			bool e0 = (state & INTERCEPTION_KEY_E0) != 0; // Extended key flag

			if (code == SC_LWIN && e0) {
				//printf("Pressed LWin: %d\n", is_down);
				lwin_down = is_down;
			}
			else if (code == SC_LSHIFT && !e0) {
				//printf("Pressed LShift: %d\n", is_down);
				lshift_down = is_down;
			}
			else if (code == SC_F23 && !e0) {
				//printf("Pressed F23: %d\n", is_down);

				// Suppress F23
				if (lwin_down && lshift_down) {
					InterceptionKeyStroke rctrl_stroke;
					rctrl_stroke.code = SC_LCTRL;
					if (is_down) {
						rctrl_stroke.state = INTERCEPTION_KEY_DOWN;
					}
					else {
						rctrl_stroke.state = INTERCEPTION_KEY_UP;
					}
					rctrl_stroke.state |= INTERCEPTION_KEY_E0; // Set the extended key flag for RCTRL

					interception_send(context, device, (InterceptionStroke*)&rctrl_stroke, 1);
					//printf("Sending RCtrl: %d\n", is_down);
					continue;
				}

			}
			else {
				//printf("Pressed key: %d, state: %d\n", code, state);
				lwin_down = false;
				lshift_down = false;
			}
		}

		// Default: pass through
		interception_send(context, device, &stroke, 1);
	}

	interception_destroy_context(context);
	return 0;
}