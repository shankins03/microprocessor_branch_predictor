# Branch Predictor Simulator

A comprehensive branch predictor simulator implementing **Bimodal** and **Gshare** branch prediction algorithms. This project simulates branch prediction behavior on instruction traces and provides detailed statistics about prediction accuracy.

## Overview

This simulator reads branch instruction traces and predicts whether each branch will be taken or not taken using configurable branch prediction algorithms. It tracks prediction accuracy and outputs detailed statistics including misprediction rates and final predictor state.

## Features

- **Bimodal Branch Predictor**: Uses a simple 2-bit saturating counter table indexed by branch address
- **Gshare Branch Predictor**: Combines branch address with global branch history register (BHR) using XOR for improved prediction accuracy
- **2-bit Saturating Counters**: Each predictor entry uses a 2-bit counter (0-3) for state transitions
- **Detailed Statistics**: Reports total predictions, mispredictions, and misprediction rate
- **Final State Output**: Displays the complete predictor table state after simulation

## Project Structure

```
microprocessor_branch_predictor/
├── main.cpp              # Main simulator implementation
├── sim_bp.h              # Branch predictor parameter definitions
├── Makefile              # Build configuration
├── CMakeLists.txt        # CMake build configuration (alternative)
├── *.txt                 # Trace files and validation outputs
└── README.md             # This file
```

## Building

Build using Make:

```bash
make
```

This will compile `main.cpp` and create the `sim` executable.

To clean build artifacts:
```bash
make clean    # Removes .o files and sim executable
make clobber  # Removes only .o files (keeps sim executable)
```

Alternatively, you can use CMake:
```bash
mkdir build && cd build
cmake ..
make
```

## Usage

### Bimodal Predictor

The bimodal predictor uses M2 bits to index into a prediction table of size 2^M2.

```bash
./sim bimodal <M2> <trace_file>
```

**Example:**
```bash
./sim bimodal 6 gcc_trace.txt
```

- `M2`: Number of bits used for indexing (table size = 2^M2)
- `trace_file`: Path to the branch trace file

### Gshare Predictor

The gshare predictor uses M1 bits for indexing and N bits for the branch history register (BHR).

```bash
./sim gshare <M1> <N> <trace_file>
```

**Example:**
```bash
./sim gshare 9 3 gcc_trace.txt
```

- `M1`: Number of bits used for indexing (table size = 2^M1)
- `N`: Number of bits in the branch history register
- `trace_file`: Path to the branch trace file

## Trace File Format

Trace files contain branch instructions in the following format:
```
<hex_address> <outcome>
```

Where:
- `<hex_address>`: Branch instruction address in hexadecimal (e.g., `302d28`)
- `<outcome>`: Branch outcome - `t` for taken, `n` for not taken

**Example:**
```
302d28 n
302d30 n
305b0c t
30093c t
```

## Output Format

The simulator outputs:
1. **COMMAND**: Echo of the command line used
2. **OUTPUT**: Statistics section containing:
   - Number of predictions
   - Number of mispredictions
   - Misprediction rate (percentage)
3. **FINAL [PREDICTOR] CONTENTS**: Complete predictor table state
   - Each line shows: `<index> <counter_value>`
   - Counter values range from 0-3 (2-bit saturating counter)

**Example Output (Bimodal):**
```
COMMAND
./sim bimodal 6 gcc_trace.txt
OUTPUT
number of predictions:    2000000
number of mispredictions: 623425
misprediction rate:       31.17%
FINAL BIMODAL CONTENTS
0	2
1	1
2	3
...
```

**Example Output (Gshare):**
```
COMMAND
./sim gshare 9 3 gcc_trace.txt
OUTPUT
number of predictions:    2000000
number of mispredictions: 433345
misprediction rate:       21.67%
FINAL GSHARE CONTENTS
0	3
1	3
2	0
...
```

## Algorithm Details

### Bimodal Predictor
- Uses lower M2 bits of the branch address (shifted right by 2) to index into prediction table
- Each entry is a 2-bit saturating counter:
  - States 0-1: Predict "not taken"
  - States 2-3: Predict "taken"
- Counter increments on taken branches, decrements on not-taken branches
- Counters saturate at 0 and 3

### Gshare Predictor
- Combines branch address with global branch history using XOR
- Index calculation: `(address >> 2) XOR (BHR << (M1 - N))`
- Branch History Register (BHR) is updated after each prediction:
  - Taken: BHR = (BHR >> 1) | (1 << (N-1))
  - Not taken: BHR = BHR >> 1
- Uses same 2-bit saturating counter mechanism as bimodal

## Testing

The project includes validation trace files:
- `gcc_trace.txt` - GCC compiler trace
- `jpeg_trace.txt` - JPEG processing trace
- `perl_trace.txt` - Perl interpreter trace

Validation outputs are provided for comparison:
- `val_bimodal_*.txt` - Bimodal predictor validation outputs
- `val_gshare_*.txt` - Gshare predictor validation outputs

To verify your implementation matches expected outputs:
```bash
./sim bimodal 6 gcc_trace.txt > output.txt
diff output.txt val_bimodal_1.txt
```

## Requirements

- C++ compiler with C++11 support (g++ recommended)
- Make build system (or CMake)
- Standard C++ libraries (`<vector>`, `<cmath>`, `<stdio.h>`, `<stdlib.h>`, `<string.h>`)

## Implementation Notes

- Predictor table entries are initialized to state 2 (weakly taken)
- Branch addresses are right-shifted by 2 bits to ignore byte offset
- The simulator processes traces sequentially and updates predictor state after each branch
- Misprediction is counted when prediction doesn't match actual outcome
- For gshare predictor, BHR is only updated when N > 0

## Performance

Typical misprediction rates on provided traces:
- **Bimodal (M2=6)**: ~31% on gcc_trace.txt
- **Gshare (M1=9, N=3)**: ~22% on gcc_trace.txt

## License

This project was developed as part of ECE 463 coursework.

