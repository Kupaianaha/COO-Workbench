# COO-Workbench

COO-Workbench is a **personal development sandbox** for COO-related work, containing **testing applications, scripts, daemon prototypes, and experimental code** that do not fold into final production systems.

This repository serves as a **holding area and reference workspace** for exploratory development, validation efforts, and one-off tooling created during COO projects.

## Purpose

The goals of this repository include:
- Experimenting with communication protocols and daemon architectures
- Testing interactions with hardware-controlling services
- Prototyping scripts and utilities for validation and diagnostics
- Exploring usability, capabilities, and integration patterns
- Retaining non-production code that may be useful for future reference

## Repository Structure

Subdirectories are generally organized by focus area. Examples include:

- **`mKTL-testing/`**  
  Protocol- and daemon-level testing specific to mKTL, including communication checks, usability exploration, and capability validation.

- **`daemon-testing/`**  
  Broader daemon testing covering multiple architectures and communication protocols, often involving low-level hardware control via vendor or system libraries.

Additional directories may appear over time as new testing efforts or exploratory work are added.

## Disclaimer

- Code in this repository is **not production-ready**  
- Scripts and daemons may be **hardware-specific**, environment-dependent, or require low-level libraries  
- Interfaces, protocols, and implementations **may change without notice**  
- This repository is intended for **internal testing, development, and reference only**  
- Users are responsible for ensuring their environment and hardware compatibility before running any scripts or daemons  

## Status

Active on an as-needed basis.  
Artifacts are retained even if no longer actively used.
