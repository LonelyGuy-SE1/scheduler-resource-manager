# CampusCompute

Command-line prototype demonstrating operating-system concepts for a shared
university computing lab: job scheduling, concurrent logging, IPC, deadlock
avoidance, worker failure detection, and resource limits.

Written in C for Linux / Ubuntu (or WSL Ubuntu).

## Project layout

```
.
├── LICENSE
├── Makefile
├── README.md
├── include/
│   └── campus.h          # shared types and demo entry points
├── src/
│   ├── main.c            # interactive menu (FR8)
│   ├── schedule.c        # job model + FCFS / RR / Priority (FR1–FR2)
│   ├── logger.c          # bounded producer–consumer logger (FR3)
│   ├── ipc.c             # POSIX shared memory + named semaphore (FR4)
│   ├── banker.c          # Banker's safety algorithm (FR5)
│   └── workers.c         # fork/waitpid + setrlimit (FR6–FR7)
└── build/                # created by make (objects + binary)
```

## Requirements

- `gcc`
- POSIX threads (`-pthread`)
- POSIX shared memory / semaphores (`-lrt` on some systems)

## Build / run

```bash
make
./build/campuscompute
```

Or:

```bash
make run
```

```bash
make clean    # remove build/ and campuscompute.log
```

## Menu → requirements

| Option | Feature |
|--------|---------|
| 1 | FR1 / FR2 — jobs + FCFS, Round Robin, Priority |
| 2 | FR3 — bounded producer–consumer logger |
| 3 | FR4 — POSIX shared memory + named semaphore |
| 4 | FR5 — Banker's safety algorithm |
| 5 | FR6 — worker success / simulated failure |
| 6 | FR7 — `setrlimit(RLIMIT_CPU)` demo |
| 0 | Exit |

Invalid menu input prints an error. Job fields are validated (`id>0`,
`arrival>=0`, `burst>0`, `priority>=0`).

## Scheduling rules

- **Round Robin** time quantum = **2**
- **Priority**: higher number = higher priority (non-preemptive)
- **Tie-break**: earlier arrival, then smaller job id

## Resource limit (option 6)

The child sets `RLIMIT_CPU` soft/hard to 1 second, then busy-loops. The kernel
stops it (typically `SIGXCPU` / `SIGKILL`), showing the CPU-time limit in effect.

## Cleanup

Each demo frees what it creates (threads joined; shm/sem unlinked; `waitpid` on
children). Build artifacts stay under `build/` and are removed by `make clean`.

## License

MIT — see [LICENSE](LICENSE).
