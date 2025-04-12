#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <curl/curl.h>
#include <zip.h>

#define STARTER_KIT_PATH "starter_kit"
#define QUARANTINE_PATH "quarantine"
#define LOG_PATH "activity.log"
#define ZIP_FILE "starter_kit.zip"

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

void download_zip() {
    CURL *curl = curl_easy_init();
    if (curl) {
        FILE *fp = fopen(ZIP_FILE, "wb");
        if (!fp) {
            printf("failed to create a file %s\n", ZIP_FILE);
            return;
        }

        curl_easy_setopt(curl, CURLOPT_URL, "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("Download failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Download success!\n");
        }

        fclose(fp);
        curl_easy_cleanup(curl);
    } else {
        printf("Failed recognized curll\n");
    }
}

void unzip_file(const char *zip_path, const char *dest_dir) {
    int err = 0;
    zip_t *zip = zip_open(zip_path, 0, &err);
    if (!zip) {
        printf("Failed to open zip: %s\n", zip_path);
        return;
    }

    zip_int64_t num_entries = zip_get_num_entries(zip, 0);
    for (zip_uint64_t i = 0; i < num_entries; i++) {
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat_index(zip, i, 0, &st);

        const char *filename = st.name;
        char path[512];
        snprintf(path, sizeof(path), "%s/%s", dest_dir, filename);

        zip_file_t *zf = zip_fopen_index(zip, i, 0);
        if (!zf) continue;

        FILE *out = fopen(path, "wb");
        if (!out) {
            zip_fclose(zf);
            continue;
        }

        char buf[1024];
        zip_int64_t n;
        while ((n = zip_fread(zf, buf, sizeof(buf))) > 0) {
            fwrite(buf, 1, n, out);
        }

        fclose(out);
        zip_fclose(zf);
        printf("Extract: %s\n", filename);
    }

    zip_close(zip);
}

char *decode_base64(const char *input) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int len = strlen(input);
    int pad = 0;

    if (len % 4 != 0) return NULL;
    if (input[len - 1] == '=') pad++;
    if (input[len - 2] == '=') pad++;

    int out_len = (len * 3) / 4 - pad;
    char *decoded = malloc(out_len + 1);
    if (!decoded) return NULL;

    int val = 0, valb = -8, index = 0;
    for (int i = 0; i < len; i++) {
        char *p = strchr(table, input[i]);
        if (p) {
            val = (val << 6) + (p - table);
            valb += 6;
            if (valb >= 0) {
                decoded[index++] = (val >> valb) & 0xFF;
                valb -= 8;
            }
        }
    }

    decoded[out_len] = '\0';
    return decoded;
}

void write_log(const char *msg) {
    FILE *f = fopen(LOG_PATH, "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%02d-%02d-%04d][%02d:%02d:%02d] - %s\n",
            t->tm_mday, t->tm_mon+1, t->tm_year+1900,
            t->tm_hour, t->tm_min, t->tm_sec, msg);

    fclose(f);
}

void decrypt_files() {
    DIR *dir = opendir(STARTER_KIT_PATH);
    if (!dir) {
        perror("starter_kit failed to open");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char old_path[512], new_path[512];
        snprintf(old_path, sizeof(old_path), "%s/%s", STARTER_KIT_PATH, entry->d_name);

        char *decoded = decode_base64(entry->d_name);
        if (!decoded) continue;

        snprintf(new_path, sizeof(new_path), "%s/%s", STARTER_KIT_PATH, decoded);
        if (rename(old_path, new_path) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "Successfully decrypted: %s", decoded);
            write_log(logmsg);
        }

        free(decoded);
    }

    closedir(dir);
}

void move_to_quarantine() {
    DIR *dir = opendir(STARTER_KIT_PATH);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", STARTER_KIT_PATH, entry->d_name);
        snprintf(dst, sizeof(dst), "%s/%s", QUARANTINE_PATH, entry->d_name);

        if (rename(src, dst) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully moved to quarantine directory.", entry->d_name);
            write_log(logmsg);
        }
    }
    closedir(dir);
}

void return_from_quarantine() {
    DIR *dir = opendir(QUARANTINE_PATH);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", QUARANTINE_PATH, entry->d_name);
        snprintf(dst, sizeof(dst), "%s/%s", STARTER_KIT_PATH, entry->d_name);

        if (rename(src, dst) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully returned to starter kit directory.", entry->d_name);
            write_log(logmsg);
        }
    }
    closedir(dir);
}

void erase_quarantine() {
    DIR *dir = opendir(QUARANTINE_PATH);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", QUARANTINE_PATH, entry->d_name);

        if (remove(path) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully deleted.", entry->d_name);
            write_log(logmsg);
        }
    }
    closedir(dir);
}

void shutdown_daemon() {
    FILE *fp = fopen("decryption.pid", "r");
    if (!fp) {
        printf("PID file not found.\n");
        return;
    }

    int pid;
    fscanf(fp, "%d", &pid);
    fclose(fp);

    if (kill(pid, SIGTERM) == 0) {
        char logmsg[256];
        snprintf(logmsg, sizeof(logmsg), "Successfully shut off decryption process with PID %d.", pid);
        write_log(logmsg);
        remove("decryption.pid");
    } else {
        perror("Failed to kill the process");
    }
}

void run_daemon() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) {
        FILE *fp = fopen("decryption.pid", "w");
        if (fp) {
            fprintf(fp, "%d", pid);
            fclose(fp);
        }
        char logmsg[256];
        snprintf(logmsg, sizeof(logmsg), "Successfully started decryption process with PID %d.", pid);
        write_log(logmsg);
        exit(EXIT_SUCCESS);
    }

    umask(0);
    setsid();
    chdir(".");
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    while (1) {
        decrypt_files();
        sleep(10);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage:\n  ./starterkit --download\n  ./starterkit --decrypt\n  ./starterkit --quarantine\n  ./starterkit --return\n  ./starterkit --eradicate\n  ./starterkit --shutdown\n");
        return 1;
    }

    if (strcmp(argv[1], "--download") == 0) {
        mkdir(STARTER_KIT_PATH, 0755);
        download_zip();
        unzip_file(ZIP_FILE, STARTER_KIT_PATH);
        remove(ZIP_FILE);
    } else if (strcmp(argv[1], "--decrypt") == 0) {
        mkdir(QUARANTINE_PATH, 0755);
        run_daemon();
    } else if (strcmp(argv[1], "--quarantine") == 0) {
        move_to_quarantine();
    } else if (strcmp(argv[1], "--return") == 0) {
        return_from_quarantine();
    } else if (strcmp(argv[1], "--eradicate") == 0) {
        erase_quarantine();
    } else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown_daemon();
    } else {
        printf("Error: Option not recognized: %s\n", argv[1]);
        printf("Choose one:\n");
        printf("--download, --decrypt, --quarantine, --return, --eradicate, --shutdown\n");
    }
    return 0;
}
