# InterceptKeys

Create your own key/shortcuts by remapping them using necessary scan codes. It's basically **PowerToys' Keyboard Manager** but at the global level i.e. mapped key/shortcuts are what received by all apps (games included).

## Installation

1. Install [interception driver](https://github.com/oblitum/Interception) as specified and restart.
2. Open project in Visual Studio using solution file ([`InterceptKeys.sln`](./InterceptKeys.sln)).
3. Configure any [_mappings_](#mapping) you want and build the **Release** config.
4. Now, the program needs to be run in background to be able to intercept and send mapped keys/shortcuts. For this, we will Task Scheduler in Windows. It'll schedule the program to auto start, when your computer starts.
5. In Powershell (`powershell.exe`), run this command: `./add_to_task_scheduler.ps1`. It'll ask for admin permissions to work.

## Mapping

1. Make sure you know the scan codes of the required keys. If not, [read this...](#finding-scan-codes)
2. Open [`mapped_entries.cpp`](./mapped_entries.cpp) and add a new `KeyMapEntry(_INPUT_KEYS_, _OUTPUT_KEYS_)` using `KEYS()`.
3. The keys format should be `KEYS(_MODIFIER_KEY1_, ..., _MODIFIER_KEY4_, _REGULAR_KEY_)`.
4. The entries are set on a priority basis with uppermost having the highest priority.

## Finding Scan Codes

1. Open project in Visual Studio and build the **Debug** config.
2. In Command Prompt (`cmd.exe`), run this command: `x64/Debug/InterceptKeys.exe --detect-keys-only`
3. Now, the program would detect all your key/shortcut clicks. This would help you find the desired scan code (inside parenthesis).

## Caution

This tool alters low-level keyboard input globally — incorrect mappings can disable essential keys.
Use carefully and test thoroughly to avoid losing control of your system.
