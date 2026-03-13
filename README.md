# Digital Clock System on Raspberry Pi

> University project for the Digital Systems course — UPM (2022)  
> Authors: Sandra & Sofia

A real-time digital clock implemented in C for Raspberry Pi, built around a **Finite State Machine (FSM)** architecture. The system tracks time and date, handles user input from a matrix keypad, and displays output on either the terminal or an LCD screen depending on the version.

---

## Overview

The project was developed incrementally across four versions, each adding hardware and functionality:

| Version | Description |
|---------|-------------|
| v1 | Clock FSM only, terminal output |
| v2 | Adds keyboard input via PC terminal thread |
| v3 | Raspberry Pi GPIO matrix keypad (TL04), no display |
| v4 | Full system: GPIO keypad + LCD display |

---

## Features

- Real-time clock with seconds, minutes, hours, day, month, and year
- Automatic date rollover with leap year detection
- 12h and 24h time formats
- Set time interactively via keypad (digit-by-digit input with validation)
- Reset clock to default values
- Thread-safe shared state using mutex locks (`piLock`/`piUnlock`)
- Timer-driven updates via POSIX real-time signals
- Hardware abstraction: same codebase runs in emulated or lab environment

---

## Architecture

The system uses a table-driven FSM pattern (`fsm.c`/`fsm.h`) where each FSM is defined as a transition table: `{origin_state, guard_function, next_state, action_function}`.

Three FSMs run concurrently in the main loop:

```
fsmReloj            — updates the clock every second (timer interrupt driven)
fsmCoreWatch        — manages system states: START, STAND_BY, SET_TIME
deteccionComandosFSM — processes keypad input and routes commands
tecladoFSM          — handles matrix keypad column scanning and debounce
```

Each FSM fires on every cycle (every `CLK_MS = 10ms`), checking its guard conditions and executing actions only when transitions are valid.

---

## File Structure

```
Digital-systems-project/
├── fsm.c / fsm.h          # Generic FSM engine (new, init, fire, destroy)
├── reloj.c / reloj.h      # Clock logic: time/date update, leap year, set time
├── coreWatch.c / coreWatch.h  # Main system FSM, input processing, main loop
├── teclado_TL04.c / .h    # Matrix keypad driver (GPIO, debounce, ISRs)
├── tmr.c / tmr.h          # POSIX timer wrapper (periodic/one-shot)
├── kbhit.c / kbhit.h      # Non-blocking keyboard read (terminal, v2)
├── systemConfig.h         # Version selection, flags, global includes
├── ent2004cfConfig.h      # GPIO pin mapping and environment config
└── util.h                 # MIN/MAX macros
```

---

## Hardware (v3/v4)

- Raspberry Pi (BCM GPIO)
- 4x4 matrix keypad (TL04) connected via GPIO rows and columns
- 16x2 LCD display (v4 only), connected in 8-bit mode via WiringPi `lcdInit`
- WiringPi library for GPIO control, LCD, and threading

---

## Building and Running

**Dependencies:** WiringPi, Eclipse IDE for Embedded C/C++ (project includes `.cproject`)

```bash
# Open with Eclipse or compile manually:
gcc -o coreWatch coreWatch.c reloj.c fsm.c tmr.c teclado_TL04.c kbhit.c \
    -lwiringPi -lpthread -lrt -Wall

# Run (requires root for GPIO access on Raspberry Pi):
sudo ./coreWatch
```

To switch versions, change `#define VERSION` in `systemConfig.h` (values: 1, 2, 3, 4).

---

## Key Concepts Demonstrated

- **FSM-based embedded design**: decoupled event detection from action execution
- **Concurrent threads**: keyboard polling runs in a separate `piThread`
- **Interrupt-driven timers**: POSIX `timer_create` with signal notification for 1-second clock ticks
- **Hardware debouncing**: configurable `DEBOUNCE_TIME_MS` per keypad row ISR
- **Thread-safe shared state**: flag-based communication between ISRs and FSMs using mutex locks
- **Incremental versioning**: same codebase scales from pure software simulation to full hardware deployment

---

## License

Academic project. Not licensed for commercial use.
