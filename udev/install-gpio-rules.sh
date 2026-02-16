#!/bin/bash
# install-gpio-rules.sh — Install udev rules for GPIO access
# Usage: sudo ./install-gpio-rules.sh
#
# Detects the board type from /proc/device-tree/model and installs the
# appropriate udev rules file to /etc/udev/rules.d/.
# Creates a "gpio" group if it doesn't exist and adds the current user to it.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RULES_DIR="/etc/udev/rules.d"
TARGET_RULE="99-gpio-photobooth.rules"

if [ "$EUID" -ne 0 ]; then
    echo "Error: This script must be run as root (use sudo)."
    exit 1
fi

# Detect board type
BOARD=""
if [ -f /proc/device-tree/model ]; then
    MODEL=$(tr -d '\0' < /proc/device-tree/model)
    echo "Detected board: ${MODEL}"

    if echo "${MODEL}" | grep -qi "raspberry pi"; then
        BOARD="raspberrypi"
    elif echo "${MODEL}" | grep -qi "orange pi"; then
        BOARD="orangepi3b"
    elif echo "${MODEL}" | grep -qi "rk3566\|rock\|radxa\|quartz64\|pine64"; then
        BOARD="orangepi3b"  # Use RK3566-compatible rules
    fi
fi

if [ -z "${BOARD}" ]; then
    echo ""
    echo "Could not auto-detect board type."
    echo "Please select your board:"
    echo "  1) Raspberry Pi (any model)"
    echo "  2) Orange Pi 3B / RK3566-based SBC"
    echo ""
    read -rp "Selection [1-2]: " choice
    case "$choice" in
        1) BOARD="raspberrypi" ;;
        2) BOARD="orangepi3b" ;;
        *) echo "Invalid selection. Aborting."; exit 1 ;;
    esac
fi

SOURCE_RULE="${SCRIPT_DIR}/99-gpio-${BOARD}.rules"

if [ ! -f "${SOURCE_RULE}" ]; then
    echo "Error: Rules file not found: ${SOURCE_RULE}"
    exit 1
fi

echo "Installing ${SOURCE_RULE} → ${RULES_DIR}/${TARGET_RULE}"
cp "${SOURCE_RULE}" "${RULES_DIR}/${TARGET_RULE}"
chmod 644 "${RULES_DIR}/${TARGET_RULE}"

# Create gpio group if it doesn't exist
if ! getent group gpio > /dev/null 2>&1; then
    echo "Creating 'gpio' group..."
    groupadd gpio
fi

# Add the invoking user to the gpio group (handle sudo)
REAL_USER="${SUDO_USER:-${USER}}"
if [ -n "${REAL_USER}" ] && [ "${REAL_USER}" != "root" ]; then
    if ! id -nG "${REAL_USER}" | grep -qw gpio; then
        echo "Adding user '${REAL_USER}' to 'gpio' group..."
        usermod -aG gpio "${REAL_USER}"
    else
        echo "User '${REAL_USER}' is already in 'gpio' group."
    fi
fi

# Reload udev rules
echo "Reloading udev rules..."
udevadm control --reload-rules
udevadm trigger

echo ""
echo "Done! GPIO udev rules installed for ${BOARD}."
echo ""
echo "IMPORTANT: You must log out and log back in (or reboot) for the"
echo "group membership change to take effect."
echo ""
echo "After reboot, verify with:  ls -la /dev/gpiochip*"
