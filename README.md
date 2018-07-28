# dcpu16-as
DCPU 16 Assembler written in C

## Current features:
  * All standard instructions according to [specification 1.7](https://gist.github.com/metaphox/3888117)
  * Initialized data with `DAT`
  * Uninitialized data with `RES`
  * Local labels (starting with `.`)
  * Expressions for `DAT` values and `RES` size, including `$` for current byte
  * Special instruction `DBP a` (debug print) op `0x00` b `0x1F`
  * Special instruction `DBH` (debug halt) `0x00`
## Usage

```
Usage: dcpu16-as [OPTION...] DASM_FILES...

  -o, --output=FILE          Output to FILE instead of out.bin
  -v, --verbose              Verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version
```

## Building
Dependencies:
  * GCC (may work with other compilers)
  * flex
  * bison
  * GNU make

Build with `make`
