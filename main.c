#include <openssl/sha.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define RANDN_PATH "/dev/urandom"
#define NEWS_PATH "./newspapers"
#define HASH_SIZE SHA256_DIGEST_LENGTH
#define NUM_TO_TICK 6

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
        printf("%02x", hash[i]);
    }

    printf("\n");
}


void xor_bytes(const unsigned char* a, const unsigned char* b, unsigned char* result, const size_t len) {
    for(size_t i = 0; i < len; i++) {
        result[i] = a[i] ^ b[i];
    }
}

void generate_grid(const unsigned char* hash, char* grid, const size_t grid_size) {
    size_t pos = strlen(grid);
    int numbers[NUM_TO_TICK];

    for(size_t i=0; i<NUM_TO_TICK; i++) {
        int num;
        int duplicate;

        do {
            num = (hash[rand()%HASH_SIZE] % 49) + 1;
            duplicate = 0;

            for(size_t j=0; j<i; j++) {
                if(numbers[j] == num) {
                    duplicate = 1;
                    num = (hash[rand()%HASH_SIZE] % 49) + 1;
                    break;
                }
            }
        } while(duplicate);

        numbers[i] = num;
    }

    for(size_t i=0; i<NUM_TO_TICK-1; i++) {
        for(size_t j=i+1; j<NUM_TO_TICK; j++) {
            if(numbers[i] > numbers[j]) {
                const int tmp = numbers[i];
                numbers[i] = numbers[j];
                numbers[j] = tmp;
            }
        }

    }

    for(size_t i=0; i<NUM_TO_TICK; i++) {
        pos += snprintf(grid + pos, grid_size - pos, "%d ", numbers[i]);
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


    SHA256(secret, HASH_SIZE, secret_hash);
    printf("Releasing secret hash to the public !\n");
    print_hash(secret_hash, HASH_SIZE);

    SHA256(newspaper, HASH_SIZE, news_hash);
    printf("\nL'article de journal sélectionné est:\n%s\n", newspaper);

    xor_bytes(secret, news_hash, xor_res, HASH_SIZE);
    printf("\nXOR result:\n");
    print_hash(xor_res, HASH_SIZE);

    SHA256(xor_res, HASH_SIZE, xor_hash);
    printf("\nXOR hash:\n");
    print_hash(xor_hash, HASH_SIZE);

    generate_grid(xor_hash, grid, 200);
    printf("\nResulting grid: %s\n", grid);

    free(newspaper);

    return 0;
}