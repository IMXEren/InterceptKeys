# InterceptKeys

Create your own key/shortcuts by remapping them using necessary scan codes. It's basically **PowerToys' Keyboard Manager** but at the global level i.e. mapped key/shortcuts are what received by all apps (games included).

## Installation

1. Install [interception driver](https://github.com/oblitum/Interception) as specified and restart.
2. Go to [Releases](https://github.com/IMXEren/InterceptKeys/releases/latest), download `InterceptKeys_release_x64_service_nologging.7z` and extract to a folder.
3. Configure any [_mappings_](#mapping) you want.
4. Open Command Prompt (`cmd.exe`) as admin in the folder and run command: `InterceptKeys.exe install`. Follow the instructions and it'll be done.
5. It'll install **InterceptKeysService** to ensure it auto starts and run in background all the time.

## Uninstallation

1. Open Command Prompt (`cmd.exe`) as admin in the folder and run command: `InterceptKeys.exe uninstall`. Follow the instructions and it'll be done. This would remove the **InterceptKeysService**.
2. To uninstall [interception driver](https://github.com/oblitum/Interception), follow the steps mentioned in the repo's docs.
3. Restart your computer.

## Mapping

1. Make sure you know the scan codes of the required keys. If not, [read this...](#finding-scan-codes)
2. Open [`mapping.toml`](./mapping.toml) and add mappings.
3. `[[mappings]]` is an array, consisting of `from`, `to` and `priority`.
4. The keys format should be `[_MODIFIER_KEY1_, ..., _MODIFIER_KEY4_, _REGULAR_KEY_]`, i.e. max of 5 key strokes.
5. The entries are set on a priority basis. Mappings with least priority are first to be triggered.
6. Here are some [samples...](./docs/format_samples.md)
7. After you've done mapping, simply run: `InterceptKeys.exe` to test if everything's working fine.

## Finding Scan Codes

1. In Command Prompt (`cmd.exe`), run this command: `InterceptKeys.exe --detect-keys-only`
2. Now, the program would detect all your key/shortcut clicks. This would help you find the desired scan code (inside parenthesis).

## Caution

This tool alters kernel-level keyboard input globally — incorrect mappings can disable essential keys.
Use carefully and test thoroughly to avoid losing control of your system.

## CLI

Checkout the [docs](./docs/cli.md).
