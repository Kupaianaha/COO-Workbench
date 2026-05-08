============================================================
Systemd Utilities & Automation Scripts
============================================================

.. image:: https://img.shields.io/badge/Systemd-v240+-blue.svg
   :target: https://www.freedesktop.org/wiki/Software/systemd/
   :alt: Systemd Version

A collection of ``systemd`` unit files, automation scripts, and process management configurations designed for Linux systems. This repository serves as a centralized hub for managing persistent background jobs, scheduled timers, and monitoring scripts.

Overview
========

This repository contains various components used to leverage the power of the ``systemd`` init system. From simple one-shot scripts to complex multi-dependency services, these configurations help automate server maintenance and application uptime.

Project Structure
=================

* ``services/``: Custom ``.service`` files for managing persistent background processes.
* ``timers/``: Systemd ``.timer`` files for scheduling tasks (replaces traditional cron jobs).
* ``scripts/``: Bash, Python, or Go scripts executed by the systemd units.
* ``templates/``: Parameterized unit files (e.g., ``worker@.service``) for scaling processes.
* ``overrides/``: Drop-in configuration files (``override.conf``) for existing system services.

Getting Started
===============

Prerequisites
-------------

* A Linux distribution utilizing ``systemd`` (Debian, Ubuntu, CentOS, Arch, etc.).
* Root or ``sudo`` privileges to install units to ``/etc/systemd/system/``.

Installation
------------

1.  **Clone the repository:**

    .. code-block:: bash

       git clone https://github.com/yourusername/systemd-jobs.git
       cd systemd-jobs

2.  **Copy the unit file:**

    Choose a service from the ``services/`` directory and copy it to the system directory:

    .. code-block:: bash

       sudo cp services/example.service /etc/systemd/system/

3.  **Reload the daemon:**

    Inform systemd of the new or modified files:

    .. code-block:: bash

       sudo systemctl daemon-reload

Usage
=====

To enable a service so it starts on boot:

.. code-block:: bash

   sudo systemctl enable example.service

To start the service immediately:

.. code-block:: bash

   sudo systemctl start example.service

To check the logs/output of a specific job:

.. code-block:: bash

   journalctl -u example.service -f

Common Patterns
===============

One-Shot Jobs
-------------
Used for scripts that run once and exit (e.g., system updates or cleanup).

.. code-block:: ini

   [Service]
   Type=oneshot
   ExecStart=/usr/local/bin/cleanup.sh

Scheduled Tasks (Timers)
------------------------
Used to run a script every 5 minutes or at a specific time daily.

.. code-block:: ini

   [Timer]
   OnCalendar=*-*-* 04:00:00
   Persistent=true

Best Practices
==============

- **Security:** Always run services with the least privilege necessary. Use ``User=`` and ``Group=`` directives.
- **Sandboxing:** Utilize ``ProtectSystem=full`` and ``PrivateTmp=true`` for enhanced security.
