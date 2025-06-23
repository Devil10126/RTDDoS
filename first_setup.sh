#!/bin/bash
echo -e "\033[1;31m[+] Installing GhostStrike-X dependencies...\033[0m"

sudo apt update && sudo apt install -y \
    build-essential \
    net-tools \
    curl \
    gcc \
    libpcap-dev \
    libnet1-dev \
    macchanger \
    cron \
    htop \
    dnsutils \
    nmap

echo -e "\033[1;32m[✔] Dependencies installed.\033[0m"
echo -e "\033[1;34m[+] Compiling ghoststrike_x_v4.c...\033[0m"

gcc ghoststrike_x.c -o ghoststrike_x -lpthread
chmod +x ghoststrike_x

echo -e "\033[1;32m[✔] Build complete. Run with: ./ghoststrike_x\033[0m"
