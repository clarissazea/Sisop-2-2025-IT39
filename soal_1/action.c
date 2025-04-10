#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define ZIP_FILE "Clues.zip"
#define ZIP_URL "https://drive.google.com/uc?export=download&id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK"  // Ganti ini

int is_valid_file(const char *filename) {
    return strlen(filename) == 5 && isalnum(filename[0]) && strcmp(filename + 1, ".txt") == 0;
}

int run_command(const char *cmd, char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmd, argv);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    } else {
        perror("fork failed");
        return 0;
    }
}

void download_and_unzip() {
    struct stat st = {0};

    if (stat("Clues", &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("Folder Clues sudah ada. Lewati download.\n");
        return;
    }

    printf("Mengunduh Clues.zip...\n");
//    char *wget_args[] = {"wget", "-q", ZIP_URL, NULL};
    char *wget_args[] = {"wget", "-q", "-O", ZIP_FILE, ZIP_URL, NULL};  // Unduh dengan nama clues.zip
    if (!run_command("wget", wget_args)) {
        fprintf(stderr, "Gagal mengunduh Clues.zip\n");
        return;
    }

    printf("Ekstrak Clues.zip...\n");
    char *unzip_args[] = {"unzip", "-q", ZIP_FILE, NULL};
    if (!run_command("unzip", unzip_args)) {
        fprintf(stderr, "Gagal mengekstrak Clues.zip\n");
        return;
    }

    remove(ZIP_FILE);
    printf("Download dan ekstrak selesai.\n");
}

void filter_files() {
    DIR *dir;
    struct dirent *entry;

    mkdir("Filtered", 0755);

    const char *subdirs[] = {"Clues/ClueA", "Clues/ClueB", "Clues/ClueC", "Clues/ClueD"};

    for (int i = 0; i < 4; ++i) {
        dir = opendir(subdirs[i]);
        if (!dir) continue;

        char path[256];
        while ((entry = readdir(dir))) {
            if (is_valid_file(entry->d_name)) {
                snprintf(path, sizeof(path), "%s/%s", subdirs[i], entry->d_name);
                char dest[256];
                snprintf(dest, sizeof(dest), "Filtered/%s", entry->d_name);
                rename(path, dest);
            } else if (entry->d_type == DT_REG) {
                snprintf(path, sizeof(path), "%s/%s", subdirs[i], entry->d_name);
                remove(path);
            }
        }
        closedir(dir);
    }

    printf("Filtering selesai. File disimpan di folder Filtered.\n");
}

int cmp(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void combine_files() {
    DIR *dir = opendir("Filtered");
    struct dirent *entry;
    char *numbers[100], *letters[100];
    int n_count = 0, l_count = 0;

    if (!dir) {
        fprintf(stderr, "Folder Filtered tidak ditemukan.\n");
        return;
    }

    while ((entry = readdir(dir))) {
        if (is_valid_file(entry->d_name)) {
            if (isdigit(entry->d_name[0]))
                numbers[n_count++] = strdup(entry->d_name);
            else if (isalpha(entry->d_name[0]))
                letters[l_count++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    qsort(numbers, n_count, sizeof(char *), cmp);
    qsort(letters, l_count, sizeof(char *), cmp);

    FILE *out = fopen("Combined.txt", "w");
    if (!out) {
        perror("Gagal membuat Combined.txt");
        return;
    }

    int ni = 0, li = 0;
    while (ni < n_count || li < l_count) {
        if (ni < n_count) {
            char path[256];
            snprintf(path, sizeof(path), "Filtered/%s", numbers[ni++]);
            FILE *f = fopen(path, "r");
            if (f) {
                int c;
                while ((c = fgetc(f)) != EOF) fputc(c, out);
                fclose(f);
            }
            remove(path);
        }
        if (li < l_count) {
            char path[256];
            snprintf(path, sizeof(path), "Filtered/%s", letters[li++]);
            FILE *f = fopen(path, "r");
            if (f) {
                int c;
                while ((c = fgetc(f)) != EOF) fputc(c, out);
                fclose(f);
            }
            remove(path);
        }
    }

    fclose(out);
    printf("Isi file telah digabung ke Combined.txt\n");

    for (int i = 0; i < n_count; i++) free(numbers[i]);
    for (int i = 0; i < l_count; i++) free(letters[i]);
}

void rot13_decode() {
    FILE *in = fopen("Combined.txt", "r");
    FILE *out = fopen("Decoded.txt", "w");
    if (!in || !out) {
        perror("Gagal membuka Combined.txt atau membuat Decoded.txt");
        return;
    }

    int c;
    while ((c = fgetc(in)) != EOF) {
        if (isalpha(c)) {
            if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
                c += 13;
            else
                c -= 13;
        }
        fputc(c, out);
    }

    fclose(in);
    fclose(out);
    printf("Decode ROT13 selesai. Output disimpan di Decoded.txt\n");
}

void print_usage() {
    printf("Penggunaan:\n");
    printf("  ./action            # Download & extract Clues.zip\n");
    printf("  ./action -m Filter  # Filter file valid ke folder Filtered\n");
    printf("  ./action -m Combine # Gabungkan isi file ke Combined.txt\n");
    printf("  ./action -m Decode  # Decode Combined.txt ke Decoded.txt\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        download_and_unzip();
    } else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
        if (strcmp(argv[2], "Filter") == 0) {
            filter_files();
        } else if (strcmp(argv[2], "Combine") == 0) {
            combine_files();
        } else if (strcmp(argv[2], "Decode") == 0) {
            rot13_decode();
        } else {
            print_usage();
        }
    } else {
        print_usage();
    }

    return 0;
}
