# Format Samples

```toml
############################################################################################################
#                                               EXAMPLES                                                   #
############################################################################################################

[[mappings]]
## (LWin + LShift + F23) Copilot Key -> RCtrl
priority = -1
from = [0xE05B, 0x2A, 0x6E]
to = [0xE01D]


############################################################################################################
#                                            FORMAT SAMPLES                                                #
############################################################################################################

## Note: Ignore the made-up key scancodes and they are just placeholders.

## Priorities are used to sort the different mappings.
## With every mapped click, by iterating over mappings, 
## 'from' keys are step up by 1.
## So, Lower the priority, faster the chance of it being triggered.

[[mappings]]
## If no priority,
## then priority = last priority + 1
## (default: 1000, incase of no last priority)
## hence, priority = 1000
from = [0xE0]
to = [0xE0]

[[mappings]]
## priority = 1000
from = [0xE0]
to = [0xE0]

[[mappings]]
## priority = 1000
from = [0xE0]
to = [0xE0]


[[mappings]]
priority = 0
from = [20]
to = [0xE01D, 20]

[[mappings]]
## priority = 0 + 1
from = [0xE0]
to = [0xE0]

[[mappings]]
## priority = 1 + 1
from = [0xE0]
to = [0xE0]
## This mapping will be filtered due to having same
## 'from' keys but having a lower priority.
## The one with highest priority, in case of similar
## 'from' keys, will be the only one to be included.

[[mappings]]
## You can also use decimal (base 10) instead of
## hexadecimal (base 16).
priority = 0
from = [20]         ## [T]
to = [57373, 20]    ## [RCtrl, T]
```
