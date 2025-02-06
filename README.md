# DiscordUpdaterKit
Copyright (C) 2025 半透明狐人間 (半狐,TranslucentFoxHuman,TlFoxHuman)

## What is this?
This project was created to simplify and automate the installation and updating of Discord on Debian-based GNU/Linux operating systems.  
This command checks for Discord updates, downloads the packages, and performs the upgrade all in one go.
  
By using this, you can reduce the process of installing/updating Discord to just a single command, eliminating the need to open a browser, download packages, and perform installation tasks from a package installation app or the command line.  
  
Additionally, this project includes a shell script that uses Zenity to display a dialog asking if you want to update when an update is available.   
By registering this as a startup program in your desktop environment, you can check for updates upon login. If an update is available, a dialog will appear, and by answering "yes",  the main program of discordupdater will launch to perform the update.

## Files
- discordupdater.cxx : Main program
- updater.sh         : Update check dialog
- COPYING            : License file
- gpl-3.0.txt        : GNU GENERAL PUBLIC LICENSE Version 3
- README.md          : This file

## How to build
You can compile using g++ or clang++. Additionally, the libcurl development package is required. On many Debian-based GNU/Linux systems, you can install it with the following command:
```
sudo apt install libcurl4-openssl-dev
```
If you prefer the GnuTLS flavor, you can also install libcurl4-gnutls-dev instead.  
  
There's no need to think about anything complicated. You can compile with just the following command:
```
g++ discordupdater.cxx -o discordupdater -lcurl
```
Or:
```
clang++ discordupdater.cxx -o discordupdater -lcurl
```

## License
See the COPYING file
