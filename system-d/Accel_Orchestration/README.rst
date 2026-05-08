============================================
Automated Power & Data Acquisition Workflow
============================================

This project utilizes **systemd** to orchestrate a hardware power cycle and a multi-stage data acquisition sequence. The system ensures that hardware is powered on, stabilized, and monitored before an archival process begins, with a full cleanup and power-down sequence upon completion.

Workflow Logic
==============

The execution follows a strict sequential timeline:

1. **Power On**: A Python subprocess command triggers the smart outlet.
2. **Stabilization**: The system waits 120 seconds for hardware initialization.
3. **Background Accel SHM posting**: ``KPIC_AccelReadout_ShmWriter`` is launched in the background.
4. **Pre-flight Delay**: The system waits 60 seconds to ensure the background process is stable.
5. **Data Archival**: ``KPIC_AccelReadout_ShmReader`` executes with a dynamic UTC timestamped directory (MM-DD-YY-HH-MM).
6. **Teardown**: Once the archival process finishes, ``KPIC_AccelReadout_ShmWriter`` is terminated via its PID.
7. **Power Off**: The Python subprocess command triggers the smart outlet to power down.

System Components
=================

1. accel_orchestration.sh
-------------------------
The orchestration script located in ``~/HISPEC/``. It handles the logic, timing (sleep), and process management (PID tracking) to ensure the binaries run in the correct order.

.. code-block:: bash

    #!/bin/bash
    set -e

    # 1. Turn outlet ON
    ~/HISPEC/python3 -c "import subprocess; subprocess.run(['echo', 'Turning Outlet ON'])"

    # 2. Wait 2 minutes
    sleep 120

    # 3. Start background binary
    ~/HISPEC/KPIC_AccelReadout_ShmWriter &
    BIN_TWO_PID=$!

    # 4. Wait 1 minute
    sleep 60

    # 5. Run archive binary with UTC timestamp
    CURRENT_TIME=$(date -u +"%m-%d-%y-%H-%M")
    ~/HISPEC/KPIC_AccelReadout_ShmReader --csv 500000 "/archive/$CURRENT_TIME"

    # 6. Kill background binary
    kill $BIN_TWO_PID

    # 7. Turn outlet OFF
    ~/HISPEC/python3 -c "import subprocess; subprocess.run(['echo', 'Turning Outlet OFF'])"

2. periodic_accel_grab.service
-------------------
A systemd **oneshot** service unit. It wraps the bash script, ensuring it runs with the correct permissions and provides a safety ``ExecStopPost`` hook to ensure the outlet is turned off even if the script fails.

.. code-block:: ini

    [Unit]
    Description=Automated Power Outlet and Data Archival Workflow
    After=network.target

    [Service]
    Type=oneshot
    User=root
    ExecStartPre=~/HISPEC/mkdir -p /archive
    ExecStart=/usr/local/bin/accel_orchestration.sh
    ExecStopPost=~/HISPEC/python3 -c "import subprocess; subprocess.run(['echo', 'Safety shutdown: Turning Outlet OFF'])"

3. time_keeper.timer
-----------------
The scheduler that triggers the workflow every 6 hours (UTC).

* **Schedule**: 00:00, 06:00, 12:00, 18:00 UTC.
* **Persistence**: Includes a ``Persistent=true`` flag, meaning if the system is offline during a scheduled window, the task will run immediately upon next boot.

.. code-block:: ini

    [Unit]
    Description=Run the Outlet and Archive workflow every 6 hours UTC

    [Timer]
    OnCalendar=*-*-* 00/4:00:00 UTC
    Persistent=true
    Unit=periodic_accel_grab.service

    [Install]
    WantedBy=timers.target

Final Deployment Steps
======================

1. **Move Files**: Place the ``.service`` and ``.timer`` files in ``/etc/systemd/system/`` and the ``.sh`` script in ``/usr/local/bin/``. Ensure the script is executable:

   .. code-block:: bash

       sudo chmod +x /usr/local/bin/accel_orchestration.sh

2. **Reload systemd**:

   .. code-block:: bash

       sudo systemctl daemon-reload

3. **Enable and Start the Timer**:

   .. code-block:: bash

       sudo systemctl enable --now time_keeper.timer

4. **Verify Schedule**:

   .. code-block:: bash

       systemctl list-timers --all | grep workflow

5. **Check Logs**:

   .. code-block:: bash

       journalctl -u periodic_accel_grab.service -f --utc

Maintenance Note
================

If ``KPIC_AccelReadout_ShmWriter`` is sensitive to being "killed" (e.g., it needs to save data before closing), change the kill command in the script to:

.. code-block:: bash

    kill -SIGINT $BIN_TWO_PID

This sends a signal similar to ``Ctrl+C``, allowing for a more graceful shutdown.



Current File Locations
======================

+------------------+--------------------------------------------------+
| File             | Path                                             |
+==================+==================================================+
| Shell script     | ``/usr/local/bin/accel_orchestration.sh``        |
+------------------+--------------------------------------------------+
| Systemd service  | ``/etc/systemd/system/periodic_accel_grab.service`` |
+------------------+--------------------------------------------------+
| Systemd timer    | ``/etc/systemd/system/periodic_accel_grab.timer``|
+------------------+--------------------------------------------------+

.. note::

   To deploy or update the workflow, give Keck the ``setup_accel_workflow.sh`` script and have them run:

   .. code-block:: bash

       cd /home/nfiudev/HISPEC/Accel_System/periodic_grab_src
       sudo bash setup_accel_workflow.sh