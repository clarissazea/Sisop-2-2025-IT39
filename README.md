<div align=center>

  # Sisop-2-2025-IT39
  
</div>

# SOAL 1
Dikerjakan oleh Clarissa Aydin Rahmazea (5027241014)   


## Cara Pengerjaan  





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

