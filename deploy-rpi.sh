#!/bin/bash

# Photobooth Deployment Script for Raspberry Pi
# This script sets up flatpak, adds flathub, and installs photobooth along with dependencies

set -e

echo "=========================================="
echo "Photobooth Deployment for Raspberry Pi"
echo "=========================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[*]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   print_error "This script must be run as root"
   exit 1
fi

# Update package lists
print_status "Updating package lists..."
apt-get update

# Install flatpak if not available
print_status "Checking for flatpak installation..."
if ! command -v flatpak &> /dev/null; then
    print_status "Installing flatpak..."
    apt-get install -y flatpak
else
    print_status "Flatpak is already installed"
fi

# Add flathub remote if not present
print_status "Checking flathub remote..."
if ! flatpak remotes | grep -q "flathub"; then
    print_status "Adding flathub remote..."
    flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
else
    print_status "Flathub remote is already configured"
fi

# Update flatpak remotes
print_status "Updating flatpak remotes..."
flatpak update -y --appstream

# Install system dependencies
print_status "Installing system dependencies..."

# Install libgpiod and related tools
print_status "Installing libgpiod..."
apt-get install -y libgpiod2 libgpiod-dev gpiod

# Install additional dependencies that might be needed
print_status "Installing additional dependencies..."
apt-get install -y \
    libusb-1.0-0 \
    libusb-1.0-0-dev \
    libcamera0 \
    libcamera-dev

# Install photobooth from flathub
print_status "Installing Photobooth from flathub..."
flatpak install -y flathub io.github.saeugetier.qtbooth

# Update flatpak
print_status "Updating flatpak applications..."
flatpak update -y

print_status "Installation completed successfully!"
echo ""
echo "=========================================="
echo "Next steps:"
echo "1. Reboot the system: sudo reboot"
echo "2. Launch Photobooth from the applications menu or run:"
echo "   flatpak run io.github.saeugetier.qtbooth"
echo "=========================================="
