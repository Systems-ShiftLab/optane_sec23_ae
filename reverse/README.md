## Reverse Engineering microbenchmarks

This directory contains code which is used to carry out our Reverse Engineering experiments.

## Repository Directory Structure
```shell
.
├── bit_pc           # Bitmask pointer chasing used to calculate buffer associativity
├── read_flush       # Used to demonstrate the effect of cache-line flushes on RMW
├── read_write_cont  # Demonstates effect of concurrent writes on read performance 
├── replacement      # Demonstates buffer replacement policy
├── user_lens        # LENS (https://github.com/TheNetAdmin/LENS) in userspace
└── wear-level       # Reproduces wear-levelling behaviour
```
