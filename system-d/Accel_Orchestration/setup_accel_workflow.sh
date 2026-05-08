#!/bin/bash
# =============================================================================
# setup_accel_workflow.sh
# Install/Update script for the Automated Power Outlet and Data Archival Workflow
# Safe to re-run at any time — always deploys the latest files from this folder.
# Run as root or with sudo: sudo bash setup_accel_workflow.sh
# =============================================================================

set -e

# --- Color output helpers ---
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

info()    { echo -e "${GREEN}[INFO]${NC}  $1"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

# --- Must be run as root ---
if [[ "$EUID" -ne 0 ]]; then
    error "Please run this script as root or with sudo."
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
info "Deploying files from: $SCRIPT_DIR"

# =============================================================================
# 1. Validate all source files exist before touching anything
# =============================================================================
info "Validating source files..."

SH_SOURCE="$SCRIPT_DIR/accel_orchestration.sh"
SERVICE_SOURCE="$SCRIPT_DIR/periodic_accel_grab.service"
TIMER_SOURCE="$SCRIPT_DIR/periodic_accel_grab.timer"

[[ -f "$SH_SOURCE" ]]      || error "Missing: accel_orchestration.sh — aborting before any changes were made."
[[ -f "$SERVICE_SOURCE" ]] || error "Missing: periodic_accel_grab.service — aborting before any changes were made."
[[ -f "$TIMER_SOURCE" ]]   || error "Missing: periodic_accel_grab.timer — aborting before any changes were made."

info "All source files found."

# =============================================================================
# 2. Stop the timer and service if currently running (safe for updates)
# =============================================================================
info "Stopping existing timer and service (if running)..."

if systemctl is-active --quiet periodic_accel_grab.timer; then
    systemctl stop periodic_accel_grab.timer
    info "Timer stopped."
else
    info "Timer was not running — skipping stop."
fi

if systemctl is-active --quiet periodic_accel_grab.service; then
    systemctl stop periodic_accel_grab.service
    info "Service stopped."
else
    info "Service was not running — skipping stop."
fi

# =============================================================================
# 3. Deploy the shell script
# =============================================================================
info "Deploying accel_orchestration.sh -> /usr/local/bin/..."
cp "$SH_SOURCE" /usr/local/bin/accel_orchestration.sh
chmod +x /usr/local/bin/accel_orchestration.sh
info "Shell script deployed and marked executable."

# =============================================================================
# 4. Deploy the systemd service file
# =============================================================================
info "Deploying periodic_accel_grab.service -> /etc/systemd/system/..."
cp "$SERVICE_SOURCE" /etc/systemd/system/periodic_accel_grab.service
chmod 644 /etc/systemd/system/periodic_accel_grab.service
info "Service file deployed."

# =============================================================================
# 5. Deploy the systemd timer file
# =============================================================================
info "Deploying periodic_accel_grab.timer -> /etc/systemd/system/..."
cp "$TIMER_SOURCE" /etc/systemd/system/periodic_accel_grab.timer
chmod 644 /etc/systemd/system/periodic_accel_grab.timer
info "Timer file deployed."

# =============================================================================
# 6. Ensure the archive base directory exists
# =============================================================================
info "Ensuring /archive directory exists..."
mkdir -p /archive
info "Archive directory ready."

# =============================================================================
# 7. Reload systemd to pick up any file changes
# =============================================================================
info "Reloading systemd daemon..."
systemctl daemon-reload
info "Daemon reloaded."

# =============================================================================
# 8. Enable and start the timer
# =============================================================================
info "Enabling periodic_accel_grab.timer to start on boot..."
systemctl enable periodic_accel_grab.timer

info "Starting periodic_accel_grab.timer..."
systemctl start periodic_accel_grab.timer

# =============================================================================
# 9. Verify
# =============================================================================
info "Verifying timer status..."
systemctl is-active --quiet periodic_accel_grab.timer \
    && info "Timer is ACTIVE and running." \
    || warn "Timer does not appear to be active. Check: systemctl status periodic_accel_grab.timer"

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Deploy complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Useful commands:"
echo "  Check timer status:   systemctl status periodic_accel_grab.timer"
echo "  List all timers:      systemctl list-timers --all | grep accel"
echo "  Run service manually: systemctl start periodic_accel_grab.service"
echo "  Watch live logs:      journalctl -fu periodic_accel_grab.service"
echo ""