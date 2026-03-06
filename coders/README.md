*This project has been created as part of the 42 curriculum by ldauber.*

# Codexion

## Description

**Codexion** is a concurrency simulation where multiple coders work in a shared inclusive co_working hub. 
To compile their quantum code, they must acquire two limited USB dongles simultaneously, one for each hand. 
The project explores thread orchestration, deadlock prevention, and custom resource scheduling using POSIX threads and mutexes.

## Instruction

### Compilation

The project must be compiled using the provided Makefile. To build the `codexion` executable with the mandatory flags (`-Wall -Wextra -Werror -pthread`), run:

```c
make
```

### Execution

The program requires 8 mandatory arguments:

```c
./codexion <number of coders> <time_to_burnout> <time_to_compile> <time_to_debug> <time_to_refactor> <numbers_of_compiles_required> <dongle_cooldown> <scheduler>
```

You can also execute the program through an interactive menu using

```c
make run
```

### Arguments

all timing are in milliseconds (ms)

- `number_of_coders`:  Total number of coders and dongles.
- `time_to_burnout`: Max time (ms) between the start of two compilations.
- `time_to_compile`: Duration (ms) of the compilation phase.
- `time_to_debug`: Duration (ms) of the debugging phase.
- `time_to_refactor`: Duration (ms) of the refactoring phase.
- `number_of_compiles_required`: Simulation stops if everyone compiles this many times.
- `dongle_cooldown`: Delay (ms) before a released dongle becomes available again.
- `scheduler`: Arbitration policy: `fifo` (First In, First Out) or `edf` (Earliest Deadline First).

## Thread synchronization mechanisms

This implementation relies on the following POSIX primitives include with `pthread.h`:

- `pthread_t`: Each coder is represented by an independent thread (using `pthread_create`.
- `pthread_mutex_t`: Protects the state of each dongle, the shared logging output, and simulation-wide control variables.
- `pthread_cond_t`: Manages the waiting queues for dongles, allowing threads to sleep efficiently until resources are available or the simulation stops.

## Blocking cases handled

The following concurrency challenges were addressed to ensure simulation stability:

- **Deadlock Prevention**: Coders follow an asymmetric resource acquisition strategy (even/odd IDs pick their left or right dongles in different orders) to prevent circular wait.
- **Starvation & Liveness**: The EDF scheduler prioritizes coders closest to their burnout limit to maintain simulation liveness.
- **Precise Burnout Detection**: A separate monitor thread continuously checks for burnout conditions, ensuring the simulation stops and logs the event within 10ms of the actual time.
- **Log Serialization**: A dedicated mutex ensures that log messages from different threads do not interleave on the same line.

## Resources

- **POSIX Thdreads & Mutexes**: Official documentation for `pthread.h`
- **Allocation Algorithms**: EDF and FIFO scheduling theory
- **AI Usage**:
    - Assistance on creating an Advanced Makefile rules for colors implementation and interactive testing.
    - Assistance in comprehension of the subject and the different usage of `pthread.h`
    - Assistance to validate all logic through rough testing