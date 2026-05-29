Welcome to the **Projects** workbench. This directory serves as a centralized development space for integrated, specialized, and highly dependent applications built on top of our core system frameworks. 

Unlike standalone, general-purpose utilities (such as low-level `camera-control`, core `controls`, or isolated `daemon-testing`), the projects housed here represent complete, mission-specific implementations that orchestrate multiple dependencies to fulfill advanced operational requirements.

## Current Projects

### 🦎 Gecko
* **Directory:** `./gecko`
* **Description:** The foundational application within this workbench. `gecko` manages high-level application orchestration, configuration parsing, validation, and real-time report generation based on system events. 
* **Core Characteristics:**
  * Implements `ConfigManager` to securely load, initialize, and validate hardware and environment sub-parameters.
  * Handles interactive user input during system initialization sequences.
  * Dynamically organizes automated report structures mapped to execution timestamps.

---

## Directory Philosophy & Architecture

This repository operates as a **workbench** rather than a collection of utilities. Future projects added to this directory should adhere to the following architectural guidelines:

1. **High Dependency Integration:** Projects here should leverage and tie together lower-level microservices, hardware abstraction layers, and daemons.
2. **Specific Scope:** Every project must solve a concrete, high-level operational task rather than providing a generalized toolset.
3. **Encapsulation:** While projects may share core sub-modules, each project directory must remain a self-contained executable or deployable unit with its own initialization logic.

## Adding a New Project

When introducing a new workbench project, please ensure the following structure is maintained:
