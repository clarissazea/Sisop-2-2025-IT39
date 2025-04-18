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
1. run_command()
2. download_and_unzip()

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
1. is_valid_file()
2. filter_files()

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

### Struktur directory setelah Filter:
![image](https://github.com/user-attachments/assets/66dc1578-980a-4637-8cbf-17059aee3224)


### c. Combine the File Content
Menggabungkan isi file di folder Filtered/ ke dalam satu file Combined.txt dengan urutan angka → huruf → angka → huruf secara bergantian.

Fungsi terkait:
1. cmp()
2. combine_files()

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

### Struktur directory setelah Combine:
![image](https://github.com/user-attachments/assets/9e33e7d6-14f5-4b5a-bbc0-790fd92d8547)

### d. Decode the file
Mendekripsi isi Combined.txt menggunakan metode ROT13 dan menyimpan hasilnya di `Decoded.txt`.

Fungsi terakit:
rot13_decode()

```bash
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
```
Penjelasan:
- Membuka Combined.txt
- Membaca satu karakter per satu karakter
- Jika huruf, digeser 13 huruf di alfabet (a-z, A-Z)
- Output hasil dekripsi ditulis ke `Decoded.txt`

### Struktur directory setelah Decoded:
![image](https://github.com/user-attachments/assets/572d4552-a50c-4b4f-914d-44662e6890a9)


### Main Function
Mengatur jalannya program berdasarkan argumen yang diberikan saat eksekusi.

Fungsi terkait:
1. main()
2. print_usage()

```bash
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
```

```bash
void print_usage() {
    printf("Penggunaan:\n");
    printf("  ./action            # Download & extract Clues.zip\n");
    printf("  ./action -m Filter  # Filter file valid ke folder Filtered\n");
    printf("  ./action -m Combine # Gabungkan isi file ke Combined.txt\n");
    printf("  ./action -m Decode  # Decode Combined.txt ke Decoded.txt\n");
}
```
Penjelasan:

- Tanpa argumen → download dan ekstraksi.
- Dengan -m + mode:
- Filter → filtering file,
- Combine → gabung file,
- Decode → dekripsi file.
Selain itu, tampilkan panduan penggunaan.

### e. Password Check
- Output dari proses Decode = `BewareOfAmpy`
![image](https://github.com/user-attachments/assets/f5717fc7-01f2-4578-988f-d62929203df9)


- Kemudian masukkan ke web checker:

![Screenshot 2025-04-11 001813](https://github.com/user-attachments/assets/ff6830b4-3950-4301-9b33-4a0afb6b2265)


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
### a. Download & Unzip Starter Kit
## C
```bash
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
        char mark[512];
        snprintf(mark, sizeof(mark), "%s/%s", dest_dir, filename);

        zip_file_t *zf = zip_fopen_index(zip, i, 0);
        if (!zf) continue;

        FILE *out = fopen(mark, "wb");
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

int main() [
if (argc == 1) {
        mkdir(STARTER_KIT_MARK, 0755);
        download_zip();
        unzip_file(ZIP_FILE, STARTER_KIT_MARK);
        remove(ZIP_FILE);mkdir(STARTER_KIT_MARK, 0755);
        download_zip();
        unzip_file(ZIP_FILE, STARTER_KIT_MARK);
        remove(ZIP_FILE);
}
]
```
Pada kode tersebut intinya adalah,  
1. File starter_kit.zip diunduh dari Google Drive.  
2. File di-unzip ke dalam folder starter_kit.  
3. File ZIP asli dihapus setelah unzip.  

### b. Decrypt Nama File Base64 ke Asli (Daemon Process)  
## C
```bash
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
void decrypt_files() {
    DIR *dir = opendir(STARTER_KIT_MARK);
    if (!dir) {
        perror("starter_kit failed to open");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char old_mark[512], new_mark[512];
        snprintf(old_mark, sizeof(old_mark), "%s/%s", STARTER_KIT_MARK, entry->d_name);

        char *decoded = decode_base64(entry->d_name);
        if (!decoded) continue;

        snprintf(new_mark, sizeof(new_mark), "%s/%s", STARTER_KIT_MARK, decoded);
        if (rename(old_mark, new_mark) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "Successfully decrypted: %s", decoded);
            write_log(logmsg);
        }

        free(decoded);
    }

    closedir(dir);
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


int main() [
 else if (strcmp(argv[1], "--decrypt") == 0) {
        decrypt_files();
        run_daemon();
} else if (strcmp(argv[1], "--quarantine") == 0) {
        decrypt_files();
        run_daemon();
        move_to_quarantine();
    } else if (strcmp(argv[1], "--return") == 0) {
        return_from_quarantine();
    }
}
```  
Pada kode ini,  
1. Proses dijalankan sebagai daemon dan setiap 10 detik mendecrypt nama file dalam folder starter_kit.  
2. Dekripsi menggunakan fungsi decode_base64().  
3. PID disimpan ke decryption.pid.  

### c. karantine & return file  
## C
```bash
void move_to_quarantine() {
    DIR *dir = opendir(STARTER_KIT_MARK);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", STARTER_KIT_MARK, entry->d_name);
        snprintf(dst, sizeof(dst), "%s/%s", QUARANTINE_MARK, entry->d_name);

        if (rename(src, dst) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully moved to quarantine directory.", entry->d_name);
            write_log(logmsg);
        }
    }
    closedir(dir);
}

void return_from_quarantine() {
    DIR *dir = opendir(QUARANTINE_MARK);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", QUARANTINE_MARK, entry->d_name);
        snprintf(dst, sizeof(dst), "%s/%s", STARTER_KIT_MARK, entry->d_name);

        if (rename(src, dst) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully returned to starter kit directory.", entry->d_name);
            write_log(logmsg);
        }
    }
    closedir(dir);
}

int main(){
else if (strcmp(argv[1], "--quarantine") == 0) {
        decrypt_files();
        run_daemon();
        move_to_quarantine();
    } else if (strcmp(argv[1], "--return") == 0) {
        return_from_quarantine();
    }
}
```
Fungsi move_to_quarantine() memindahkan semua file dari starter_kit ke folder quarantine.  
Fungsi return_from_quarantine() melakukan hal sebaliknya.  
Di main(), keduanya dipanggil dengan argumen:  
--quarantine  
--return

### d. Hapus File dalam Karantina (Eradicate)  
## C
```bash
void erase_quarantine() {
    DIR *dir = opendir(QUARANTINE_MARK);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char mark[512];
        snprintf(mark, sizeof(mark), "%s/%s", QUARANTINE_MARK, entry->d_name);

        if (remove(mark) == 0) {
            char logmsg[512];
            snprintf(logmsg, sizeof(logmsg), "%s - Successfully deleted.", entry->d_name);
            write_log(logmsg);
        }
    }
    closedir(dir);
}

int main(){
 else if (strcmp(argv[1], "--eradicate") == 0) {
        erase_quarantine(); 
}
```  
Kode ini berisi Fungsi rename() digunakan untuk memindahkan file antar folder.  
Log ditulis ke activity.log.

### e. Shutdown Daemon berdasarkan PID
## C
```bash
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
int main(){
else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown_daemon();
}
```  
PID dibaca dari file.  
Proses dihentikan dengan kill().  
Log ditulis sesuai format.  

### f. Error Handling Sederhana  
## C
```bash
int main(int argc, char *argv[]) {
if (argc == 1) {
        mkdir(STARTER_KIT_MARK, 0755);
        download_zip();
        unzip_file(ZIP_FILE, STARTER_KIT_MARK);
        remove(ZIP_FILE);mkdir(STARTER_KIT_MARK, 0755);
        download_zip();
        unzip_file(ZIP_FILE, STARTER_KIT_MARK);
        remove(ZIP_FILE);
    } else if (strcmp(argv[1], "--info") == 0) {
        printf("Daftar argumen yang dapat digunakan:\n");
        printf("  --info        : Menampilkan informasi argumen\n");
        printf("  --decrypt     : Mendekripsi semua file terenkripsi\n");
        printf("  --quarantine  : Mendekripsi dan memindahkan file mencurigakan ke karantina\n");
        printf("  --return      : Mengembalikan file dari folder karantina ke asalnya\n");
        printf("  --eradicate   : Menghapus semua file di folder karantina\n");
        printf("  --shutdown    : Mematikan daemon\n");
} else {
        printf("Argumen tidak dikenali, melakukan download...\n");
        mkdir(STARTER_KIT_MARK, 0755);
        download_zip();
        unzip_file(ZIP_FILE, STARTER_KIT_MARK);
        remove(ZIP_FILE);
    }
}
```  
Pesan kesalahan ditampilkan jika pengguna salah mengetikkan argumen.  

### g. Logging ke activity.log  
## C
```bash
void write_log(const char *msg) {
    FILE *f = fopen(LOG_MARK, "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%02d-%02d-%04d][%02d:%02d:%02d] - %s\n",
            t->tm_mday, t->tm_mon+1, t->tm_year+1900,
            t->tm_hour, t->tm_min, t->tm_sec, msg);

    fclose(f);
}
```  
Code ini difokuskan untuk mencatat log.  
Format log sesuai dengan instruksi soal.  

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

