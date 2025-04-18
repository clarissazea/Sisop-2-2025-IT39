<div align=center>

  # Sisop-2-2025-IT39
  
</div>

# SOAL 1
Dikerjakan oleh Clarissa Aydin Rahmazea (5027241014)   

Soal ini diminta membuat sebuah program C bernama `action.c` yang berfungsi untuk mengelola file-file petunjuk (clue) dari sebuah arsip `Clues.zip`. Program ini harus mampu mengunduh file zip dari link, mengekstraknya, kemudian memfilter file-file teks yang hanya memiliki satu karakter alfanumerik sebagai nama (diikuti dengan `.txt`) dan memindahkannya ke folder `Filtered/`. Setelah itu, program harus menggabungkan isi file-file tersebut secara bergantian antara file yang bernama angka dan huruf ke dalam satu file baru bernama `Combined.txt`, lalu melakukan dekripsi terhadap isi file tersebut menggunakan metode `ROT13` dan menyimpannya ke dalam file `Decoded.txt`. 

Program `action.c` terdiri atas beberapa perintah utama:

- `./action` → Mengunduh dan mengekstrak `Clues.zip` ke dalam folder Clues.
- `./action -m Filter` → Memfilter file valid ke dalam folder Filtered.
- `./action -m Combine` → Menggabungkan isi file ke dalam `Combined.txt`.
- `./action -m Decode` → Mendekode isi `Combined.txt` dengan ROT13 ke `Decoded.txt`.

Sementara file Clues.zip berisi beberapa folder (ClueA–ClueD) yang masing-masing berisi banyak file clue.

## Cara Pengerjaan  

### a. Downloading the Clues
Mengunduh file `Clues.zip` dari URL, lalu mengekstraknya menjadi folder `Clues/`. Jika folder `Clues/` sudah ada, proses download dilewati.

Fungsi terkait:
- run_command()
- download_and_unzip()

```bash
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
```
Penjelasan:
- Membuat proses baru (fork())
- Anak proses menjalankan perintah (execvp()), misalnya wget atau unzip
- Proses induk menunggu anak selesai dan memeriksa apakah berhasil.
  
```bash
void download_and_unzip() {
    struct stat st = {0};

    if (stat("Clues", &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("Folder Clues sudah ada. Lewati download.\n");
        return;
    }

    printf("Mengunduh Clues.zip...\n");
    char *wget_args[] = {"wget", "-q", "-O", "Clues.zip", "https://drive.google.com/uc?export=download&id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK", NULL};
    if (!run_command("wget", wget_args)) {
        fprintf(stderr, "Gagal mengunduh Clues.zip\n");
        return;
    }

    printf("Ekstrak Clues.zip...\n");
    char *unzip_args[] = {"unzip", "-q", "Clues.zip", NULL};
    if (!run_command("unzip", unzip_args)) {
        fprintf(stderr, "Gagal mengekstrak Clues.zip\n");
        return;
    }

    remove("Clues.zip");
    printf("Download dan ekstrak selesai.\n");
}
```
Penjelasan:
- Mengecek apakah folder `Clues/` sudah ada.
- Jika belum ada, file `Clues.zip` diunduh menggunakan wget.
- File `Clues.zip` diekstrak menggunakan unzip.
- Setelah ekstraksi, file zip dihapus.

### b. Filtering the Files
Memfilter file-file dalam folder `Clues/` agar hanya file dengan nama satu karakter alfanumerik (a.txt, 5.txt, dll) yang dipindahkan ke folder `Filtered/`, sedangkan file lain dihapus.

Fungsi terkait:
- is_valid_file()
- filter_files()

```bash
int is_valid_file(const char *filename) {
    return strlen(filename) == 5 && isalnum(filename[0]) && strcmp(filename + 1, ".txt") == 0;
}
```
Penjelasan:
- Cek panjang nama file (1 karakter + .txt = 5).
- Karakter pertama harus alfanumerik (huruf/angka).
- Ekstensi harus .txt.

```bash
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
```
Penjelasan:
- Membuat folder Filtered/.
- MembUka setiap subfolder (ClueA, ClueB, ClueC, ClueD).
- File yang valid dipindahkan ke `Filtered/`.
- Sedankan file yang tidak valid dihapus.

### c. Combine the File Content
Menggabungkan isi file di folder Filtered/ ke dalam satu file Combined.txt dengan urutan angka → huruf → angka → huruf secara bergantian.

Fungsi terkait:
- cmp()
- combine_files()

```bash
int cmp(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}
```
Penjelasan:
- Fungsi pembanding untuk mengurutkan nama file secara alfabetik

```bash
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
```
Penjelasan:
- File dengan nama angka dan huruf dikumpulkan dan diurutkan.
- Menggabungkan isi file angka lalu huruf secara bergantian ke `Combined.txt`.
- Setelah isi file digabungkan, file aslinya dihapus.




# Soal_2 
Dikerjakan oleh Ahmad Wildan Fawwaz (5027241001)  
Pengerjaan soal ini menggunakan 1 file script starterkit.c dan 1 file starter_kit.zip  
`starterkit.c` terdiri atas 5 perintah utama:  
1. `./starterkit --decrypt` untuk mengaktifkan program decrypt mendekripsi file dalam ``folder starter_kit``
2. `./starterkit --quarantine` untuk memindahkan file ke ``quarantine``
3. `./starterkit --return` untuk meindahkan file dari ``quarantine`` ke ``starter_kit``
4. `./starterkit --eradicate` untuk menghapus file yang ada di ``quarantine``
5. `./starterkit --shutdown` untuk mematikan program decrypt

Sementara file starter_kit.zip tersebut terdiri dari 27 file termasuk virus

## Cara Pengerjaan  
a. Download & Unzip Starter Kit
## c
```bash
void download_zip() {
    ...
    curl_easy_setopt(curl, CURLOPT_URL, "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download");
    ...
}

void unzip_file(const char *zip_path, const char *dest_dir) {
    ...
}


int main() [
if (strcmp(argv[1], "--download") == 0) {
    mkdir(STARTER_KIT_PATH, 0755);
    download_zip();
    unzip_file(ZIP_FILE, STARTER_KIT_PATH);
    remove(ZIP_FILE);
}
]
```
b. Decrypt Nama File Base64 ke Asli (Daemon Process)  
## C
```bash
void run_daemon() {
    pid_t pid = fork();
    ...
    while (1) {
        decrypt_files();
        sleep(10);
    }
}
void decrypt_files() {
    ...
    char *decoded = decode_base64(entry->d_name);
    ...
}

int main() [
else if (strcmp(argv[1], "--decrypt") == 0) {
    mkdir(QUARANTINE_PATH, 0755);
    run_daemon();
}
}
```

c. Fitur Karantina dan Return File
```bash
void move_to_quarantine() {
    ...
    rename(src, dst);
}

void return_from_quarantine() {
    ...
    rename(src, dst);
}

int main(){
else if (strcmp(argv[1], "--quarantine") == 0) {
    move_to_quarantine();
} else if (strcmp(argv[1], "--return") == 0) {
    return_from_quarantine();
}
}
```
d. Hapus File dalam Karantina (Eradicate)  
```bash
void erase_quarantine() {
    ...
    remove(path);
}

int main(){
else if (strcmp(argv[1], "--eradicate") == 0) {
    erase_quarantine();
}
}
```
e. Shutdown Daemon berdasarkan PID
```bash
void shutdown_daemon() {
    FILE *fp = fopen("decryption.pid", "r");
    ...
    kill(pid, SIGTERM);
}

int main(){
else if (strcmp(argv[1], "--shutdown") == 0) {
    shutdown_daemon();
}
}
```
f. Error Handling Sederhana  
```bash
if (argc != 2) {
    printf("Usage:\n  ./starterkit --download\n ...\n");
    return 1;
}
```
g. Logging ke activity.log  
```bash
void write_log(const char *msg) {
    ...
    fprintf(f, "[%02d-%02d-%04d][%02d:%02d:%02d] - %s\n", ...);
}
```
# Soal_3
Dikerjakan oleh Muhammad Rafi' Adly (5027241082)  


## Cara Pengerjaan  

Membuat fungsi agar file bekerja secara daemon
```bash
void daemon() {
  pid_t pid, sid;        // Variabel untuk menyimpan PID
  pid = fork();     // Menyimpan PID dari Child Process

  /* Keluar saat fork gagal
  * (nilai variabel pid < 0) */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Keluar saat fork berhasil
  * (nilai variabel pid adalah PID dari child process) */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  if ((chdir("/")) < 0) {
    exit(EXIT_FAILURE);
  }

  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}
```
## a. Menyamarkan proses menjadi /init
```bash
strncpy(argv[0], "/init", strlen(argv[0]));
```

## Revisi


# Soal_4
Dikerjakan oleh Ahmad Wildan Fawwaz (5027241001)  

