#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#define LOGFILE "debugmon.log"
#define PIDFILE "/tmp/debugmon_pid.txt"

// Fungsi mencatat ke file log
void log_entry(const char* process_name, const char* status) {
    FILE* log = fopen(LOGFILE, "a");
    if (log == NULL) {
        perror("Error opening log file");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log, "[%02d:%02d:%04d]-%02d:%02d:%02d_%s_%s\n",
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            process_name, status);
    fclose(log);
}

// Menampilkan daftar proses user
void list_processes(const char* user) {
    char command[1024];
    snprintf(command, sizeof(command),
        "ps -u %s -o pid=,comm=,%%cpu=,%%mem=,rss= --sort=-%%cpu | "
        "awk 'BEGIN { "
        "printf \"%%-10s %%20s %%10s %%10s %%12s\\n\", \"PID\", \"COMMAND\", \"%%CPU\", \"%%MEM\", \"MEM(KB)\" "
        "} "
        "{ "
        "printf \"%%-10s %%20s %%10.2f %%10.2f %%12d\\n\", $1, $2, $3, $4, $5 "
        "}'", user);
    system(command);
}

// Mode daemon
void daemon_process(const char* user) {
    pid_t pid = fork();
    if (pid == 0) {
        FILE* pidfile = fopen(PIDFILE, "w");
        if (pidfile != NULL) {
            fprintf(pidfile, "%d", getpid());
            fclose(pidfile);
        }

        while (1) {
            char command[256];
            snprintf(command, sizeof(command), "ps -u %s -o comm=", user);
            FILE* fp = popen(command, "r");
            if (fp == NULL) {
                perror("Error executing ps");
                exit(1);
            }

            char cmd[256];
            while (fgets(cmd, sizeof(cmd), fp)) {
                cmd[strcspn(cmd, "\n")] = 0;
                if (strlen(cmd) > 0) {
                    log_entry(cmd, "RUNNING");
                }
            }

            pclose(fp);
            sleep(5);
        }
    } else if (pid > 0) {
        printf("Debugmon is running in daemon mode for user '%s'.\n", user);
    } else {
        perror("Fork failed");
    }
}

// Menghentikan daemon
void stop_process(const char* user) {
    FILE* pidfile = fopen(PIDFILE, "r");
    if (pidfile == NULL) {
        printf("No running Debugmon found for user '%s'.\n", user);
        return;
    }

    int pid;
    fscanf(pidfile, "%d", &pid);
    fclose(pidfile);

    if (kill(pid, SIGTERM) == 0) {
        log_entry("debugmon_daemon", "RUNNING");
        remove(PIDFILE);
        printf("Debugmon daemon stopped for user '%s'.\n", user);
    } else {
        perror("Failed to stop Debugmon");
    }
}

// Gagalkan semua proses dan blok user
void fail_process(const char* user) {
    FILE* pidfile = fopen(PIDFILE, "r");
    if (pidfile != NULL) {
        int pid;
        fscanf(pidfile, "%d", &pid);
        fclose(pidfile);
        kill(pid, SIGTERM);
        remove(PIDFILE);
    }

    char command[256];
    snprintf(command, sizeof(command), "ps -u %s -o comm=", user);
    FILE* fp = popen(command, "r");
    if (fp) {
        char proc[256];
        while (fgets(proc, sizeof(proc), fp)) {
            proc[strcspn(proc, "\n")] = 0;
            if (strlen(proc) > 0) {
                log_entry(proc, "FAILED");
            }
        }
        pclose(fp);
    }

    char lock[256];
    snprintf(lock, sizeof(lock), "sudo usermod -s /usr/sbin/nologin %s", user);
    system(lock);

    printf("User '%s' is now blocked from running processes.\n", user);
}

// Kembalikan akses user
void revert_process(const char* user) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "sudo usermod -s /bin/bash %s", user);
    system(cmd);
    log_entry("user_revert", "RUNNING");
    printf("User '%s' can now run processes again.\n", user);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s {list|daemon|stop|fail|revert} <user>\n", argv[0]);
        return 1;
    }

    const char* cmd = argv[1];
    const char* user = argv[2];

    if (strcmp(cmd, "list") == 0) {
        list_processes(user);
    } else if (strcmp(cmd, "daemon") == 0) {
        daemon_process(user);
    } else if (strcmp(cmd, "stop") == 0) {
        stop_process(user);
    } else if (strcmp(cmd, "fail") == 0) {
        fail_process(user);
    } else if (strcmp(cmd, "revert") == 0) {
        revert_process(user);
    } else {
        printf("Unknown command: %s\n", cmd);
        return 1;
    }

    return 0;
}
