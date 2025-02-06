#!/bin/bash

# updater.sh - check Discord updates and install updates with dialog.
# This tool requires zenity. Please install it before using this.

# This script is public domain.

if [[ `apt show zenity 2>/dev/null` != *"APT-Manual-Installed"* ]];then
    echo "This tool requires zenity. Please install it."
    exit
fi

CHKOUT=`discordupdater -c`

if [[ "$CHKOUT" != *"Update available!"* ]];then
    exit
fi

zenity --question --title "Discord Updater" --text "A newer version of Discord is available! Do you want to install it?"

if [[ "$?" != "0" ]];then
    exit
fi

x-terminal-emulator -e "discordupdater -y -u"
