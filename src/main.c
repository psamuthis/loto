#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define GREEN "\x1B[32m"
#define ORANGE "\x1B[38;5;208m"
#define GREY "\x1B[90m"
#define RESET "\x1B[0m"
#define RED "\x1B[31m"

#define RANDN_PATH "/dev/urandom"
#define NEWS_PATH "./newspapers"
#define RES_PATH "./output-data/results.txt"
#define HASH_SIZE SHA256_DIGEST_LENGTH
#define NUM_TO_TICK 6
#define MAX_NEWS_LEN 333

ssize_t init_seed() {
    FILE* randn_gen_file = fopen(RANDN_PATH, "r");

    if(randn_gen_file) {
        ssize_t seed;
        fread(&seed, sizeof(seed), 1, randn_gen_file);
        fclose(randn_gen_file);
        return seed;
    }

    return -1;
}

void print_hash(const unsigned char* hash, const size_t len) {
    for(size_t i=0; i<len; i++) {
        printf("%s%02x%s", GREY, hash[i], RESET);
    }

    printf("\n");
}


void xor_bytes(const unsigned char* a, const unsigned char* b, unsigned char* result, const size_t len) {
    for(size_t i = 0; i < len; i++) {
        result[i] = a[i] ^ b[i];
    }
}

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    const long file_size = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    const size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);

    return buffer;
}

ssize_t write_grid(const unsigned char* grid) {
    FILE* output = fopen(RES_PATH, "a");
    if(output != NULL) {
        fprintf(output, "%s\n", grid);
        fclose(output);
        return 1;
    } else {
        printf("%s[Error]%s Could not open output file to write grid.\n", RED, RESET);
    }
}

void generate_grid(const unsigned char* hash, char* grid, const size_t grid_size) {
    int numbers[NUM_TO_TICK];
    int count = 0;
    int byte_idx = 0;

    while (count < NUM_TO_TICK && byte_idx < HASH_SIZE) {
        int num = (hash[byte_idx] % 49) + 1;

        int duplicate = 0;
        for (int i = 0; i < count; i++) {
            if (numbers[i] == num) {
                duplicate = 1;
                break;
            }
        }

        if (!duplicate) {
            numbers[count++] = num;
        }
        byte_idx++;
    }

    for (int i = 0; i < NUM_TO_TICK - 1; i++) {
        for (int j = i + 1; j < NUM_TO_TICK; j++) {
            if (numbers[i] > numbers[j]) {
                int tmp = numbers[i];
                numbers[i] = numbers[j];
                numbers[j] = tmp;
            }
        }
    }

    int pos = 0;
    for (int i = 0; i < NUM_TO_TICK; i++) {
        pos += snprintf(grid + pos, grid_size - pos, "%d ", numbers[i]);
    }
    write_grid(grid);
}

int main() {
    const ssize_t seed = init_seed();
    if(seed == -1) {
        printf("Could not get the seed from %s.\n", RANDN_PATH);
        return 1;
    }
    srand(seed);

    unsigned char secret[HASH_SIZE];
    FILE* urandom = fopen(RANDN_PATH, "r");
    fread(secret, 1, HASH_SIZE, urandom);
    fclose(urandom);


    unsigned char* newspaper = (unsigned char*)read_file("./newspapers/lemonde-160226.txt");
    unsigned char secret_hash[HASH_SIZE];
    unsigned char news_hash[HASH_SIZE];
    unsigned char xor_res[HASH_SIZE];
    unsigned char xor_hash[HASH_SIZE];
    char grid[100];

    printf("%s***VEILLE DU TIRAGE***\n%s", ORANGE, RESET);

    printf("\n*Sélection de l'article de journal...");
    unsigned char trimmed_newspaper[MAX_NEWS_LEN];
    strncpy(trimmed_newspaper, newspaper, MAX_NEWS_LEN);
    printf("\n*L'article de journal sélectionné est:\n\"%s%s...%s\"\n", GREY, trimmed_newspaper, RESET);

    printf("\n*Calcul du hash du secret D...\n");
    SHA256(secret, HASH_SIZE, secret_hash);
    printf("*Publication du hash du secret au public !\n");
    printf("R=h(D)=");
    print_hash(secret_hash, HASH_SIZE);

    printf("\n%s***JOUR DU TIRAGE***%s", ORANGE, RESET);
    printf("\n*Calcul du hash du journal...\n");
    SHA256(newspaper, HASH_SIZE, news_hash);
    printf("S=h(J)=");
    print_hash(news_hash, HASH_SIZE);

    printf("\n*Calcul du xor entre le secret et le hash du journal...");
    xor_bytes(secret, news_hash, xor_res, HASH_SIZE);
    printf("\n*XOR result:\n");
    printf("D ⊕ S=");
    print_hash(xor_res, HASH_SIZE);

    printf("\n*Calcul du hash du xor obtenu précédemment...");
    SHA256(xor_res, HASH_SIZE, xor_hash);
    printf("\n*XOR hash:\n");
    printf("h(D ⊕ S)=");
    print_hash(xor_hash, HASH_SIZE);

    printf("\n*Génération de la grille...\n");
    generate_grid(xor_hash, grid, 200);

    printf("\n%s***LENDEMAIN DU TIRAGE***%s", ORANGE, RESET);
    printf("\n*Grille générée: %s%s%s", RED, grid, RESET);
    printf("\n*Secret initialement choisi:\n");
    print_hash(secret, HASH_SIZE);

    free(newspaper);

    return 0;
}