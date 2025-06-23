#!/bin/bash
echo -e "\033[1;36m[~] Refreshing network interface and MAC...\033[0m"

sudo ifconfig eth0 down
sudo macchanger -r eth0
sudo ifconfig eth0 up
sudo dhclient eth0

echo -e "\033[1;32m[✓] MAC & IP refreshed.\033[0m"
echo -e "\033[1;34m[~] Checking kill switch status...\033[0m"

curl -s https://pastebin.com/raw/ghostkill | tr -d '\n' | xxd -r -p | strings | grep -q "KILL"
if [ $? -eq 0 ]; then
    echo -e "\033[1;31m[!] Remote kill signal detected. Deleting binary...\033[0m"
    rm -f ghoststrike_x
else
    echo -e "\033[1;32m[+] No kill signal found. Ready.\033[0m"
fi
