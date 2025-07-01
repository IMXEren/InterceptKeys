# CLI

The cli offers various options like starting the service, getting its current state etc.

```cmd
> InterceptKeys -h
Intercept key strokes to send modified key strokes using interception 
kernel-mode driver.


InterceptKeys [OPTIONS] [SUBCOMMAND]


OPTIONS:
  -h,     --help              Print this help message and exit
  -v,     --version           Display program version information and exit
          --status            Fetch the running status of InterceptKeysService.
          --detect-keys-only  Helper to detect key clicks to find the necessary scancode.
          --service           Run the interception as a service. Recommended after you've setup
                              the map config.
          --logging [DIR] [default: logs]
                              Enable logging
          --map-config [TOML]... [default: mapping.toml]
                              List of mapping config file to use while Interception.

SUBCOMMANDS:
  install                     Install the InterceptKeysService.
  start                       Start the InterceptKeysService.
  stop                        Stop the InterceptKeysService.
  restart                     Restart the InterceptKeysService.
  uninstall                   Uninstall the InterceptKeysService.
```

## `install`

Installs the given service with given arguments (if any).

```cmd
> InterceptKeys install -h
Install the InterceptKeysService.                                                                                                                           


InterceptKeys install [OPTIONS]


OPTIONS:
  -h,     --help              Print this help message and exit
          --map-config [TOML]... [default: mapping.toml]
                              List of mapping config file to use while Interception.
          --logging [DIR] [default: logs]
                              Enable logging
```
