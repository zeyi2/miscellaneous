/*
 * passgen.c
 * 
 * Description: A password generator that combines Rule 30 cellular automaton
 *              and random bytes to produce secure passwords.
 * 
 * Author: Mitchell <mitchell@sdf.org>
 * Date: August 2024
 * 
 * Compilation: gcc -o passgen passgen.c -lcrypto
 * Usage: passgen <length>
 * 
 * <length> - The length of the generated password.
 * 
 * This program generates a secure password of the specified length using a
 * combination of Rule 30 cellular automaton and cryptographic random bytes.
 * The generated password will include uppercase and lowercase letters, digits,
 * and special characters.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <openssl/rand.h>
 
#define CHARSET "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+"

int is_valid_integer(const char* str) {
  if (*str == '\0') return 0;
  while (*str) {
    if (!isdigit(*str)) return 0;
    str++;
  }
  return 1;
}

void rule30(uint8_t *buffer, int length) {
  uint8_t *state = (uint8_t*)malloc(length * sizeof(uint8_t));
  memset(state, 0, length * sizeof(uint8_t));
  state[length / 2] = 1;

  for (int i = 0; i < length; i++) {
    buffer[i] = state[i];
    uint8_t *next_state = (uint8_t*)malloc(length * sizeof(uint8_t));
    memset(next_state, 0, length * sizeof(uint8_t));

    for (int j = 0; j < length; j++) {
      uint8_t left = (j == 0) ? 0 : state[j - 1];
      uint8_t center = state[j];
      uint8_t right = (j == length - 1) ? 0 : state[j + 1];
      next_state[j] = !(left ^ (center || right));
    }

    memcpy(state, next_state, length * sizeof(uint8_t));
    free(next_state);
  }

  free(state);
}

void complex_mix(uint8_t *buffer1, uint8_t *buffer2, uint8_t *result, int length) {
  for (int i = 0; i < length; ++i) {
    result[i] = (buffer1[i] ^ buffer2[i]) + (buffer1[(i + 1) % length] * buffer2[(i + 2) % length]);
    result[i] = (result[i] << 3) | (result[i] >> 5);
  }
}

void generate_combined_password(char* password, int length) {
  uint8_t *rule30_buffer = (uint8_t*)malloc(length * sizeof(uint8_t));
  rule30(rule30_buffer, length);

  unsigned char rand_buffer[length];
  if (RAND_bytes(rand_buffer, length) != 1) {
    fprintf(stderr, "Failed to generate random bytes.\n");
    exit(1);
  }

  uint8_t *combined_buffer = (uint8_t*)malloc(length * sizeof(uint8_t));
  complex_mix(rule30_buffer, rand_buffer, combined_buffer, length);

  int charset_size = strlen(CHARSET);
  for (int i = 0; i < length; ++i) {
    int combined_index = combined_buffer[i] % charset_size;
    password[i] = CHARSET[combined_index];
  }
  password[length] = '\0';

  free(rule30_buffer);
  free(combined_buffer);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: passgen <length>\n");
    return 1;
  }

  if (!is_valid_integer(argv[1])) {
    fprintf(stderr, "Invalid length format. It should be a positive integer.\n");
    return 1;
  }

  int password_len = atoi(argv[1]);
  if (password_len <= 0) {
    fprintf(stderr, "Password length must be a positive integer.\n");
    return 1;
  }

  char *password = (char*)malloc((password_len + 1) * sizeof(char));
  if (password == NULL) {
    perror("Failed to allocate memory for password");
    return 1;
  }

  generate_combined_password(password, password_len);
  printf("Generated Password: %s\n", password);

  free(password);
  return 0;
}
