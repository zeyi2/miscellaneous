/*
 * swordle.c
 * 
 * Description: A simple command-line Wordle-style game implemented in C. 
 *              Players have a limited number of guesses to find a hidden word.
 *              The game provides feedback on the correctness of each guess
 *              with color-coded outputs.
 * 
 * Author: Mitchell <mitchell@sdf.org>
 * Date: July 2024
 * 
 * Compilation: gcc -o swordle swordle.c
 * 
 * This program reads a list of words from "words.txt", selects a random word,
 * and allows the player to guess the word within a set number of tries. The game
 * provides feedback on each guess using color codes to indicate whether letters
 * are correct and in the correct position.
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLOR_GREEN "\x1B[32m"
#define COLOR_YELLOW "\x1B[33m"
#define COLOR_WHITE "\x1B[37m"
#define COLOR_DEFAULT_TEXT COLOR_WHITE

#define LETTER_SEPERATOR_STR "_ "
#define NULL_CHAR '\0'
#define NEWLINE_STR "\n"
#define NEWLINE_CHAR (char) '\n'

#define WORD_LENGTH 5
#define MAX_GUESSES 6
#define LAST_CHARACTER WORD_LENGTH
#define GUESS_ARRAY_SIZE (MAX_GUESSES * WORD_LENGTH)

#define NOT_IN_WORD 0
#define IN_WORD_WRONG_INDEX 1
#define IN_WORD_CORRECT_INDEX 2

#define PLAYER_WON true
#define PLAYER_LOST false

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#define clear_screen() system("clear")
#endif

#if defined(_WIN32) || defined(_WIN64)
#define clear_screen() system("cls")
#endif

#define assert_print(condition, string)         \
  if (!(condition)) {                           \
    printf("%s", string);                       \
    terminate(1);                               \
  }

#define enter_game_loop() get_input()

void get_input();
void terminate(uint8_t exit_code);

FILE* file_ptr;

size_t words_length = 0;
size_t current_line = 0;

char user_input[WORD_LENGTH + 1];
char current_word[WORD_LENGTH + 1];
char guess_list[GUESS_ARRAY_SIZE];

bool game_over = false;

/**
 * Check if a letter is in the current word and return its status.
 *
 * @param letter The letter to check.
 * @param guess_list_index The index in the guess list.
 * @return Status code indicating if the letter is in the word and its position.
 */
uint8_t check_letter_in_word(char letter, size_t guess_list_index) {
  if (current_word[guess_list_index] == letter)
    return IN_WORD_CORRECT_INDEX;

  for (uint8_t i = 0; i < WORD_LENGTH; ++i) 
    if (current_word[i] == letter)
      return IN_WORD_WRONG_INDEX;

  return NOT_IN_WORD;
}

/**
 * Determine the color to print the letter based on its status.
 *
 * @param letter The letter to determine the color for.
 * @param guess_list_index The index in the guess list.
 * @return The color code for the letter.
 */
const char* determine_letter_color(char letter, size_t guess_list_index) {
  switch (check_letter_in_word(letter, guess_list_index)) {
  case IN_WORD_CORRECT_INDEX:
    return COLOR_GREEN;
  case IN_WORD_WRONG_INDEX:
    return COLOR_YELLOW;
  default:
    return COLOR_DEFAULT_TEXT;  
  }
}

/**
 * Print a letter with the given color.
 *
 * @param letter The letter to print.
 * @param color The color code to use.
 */
void print_letter(char letter, const char* color) {
  printf("%s%c%s ", color, letter, COLOR_DEFAULT_TEXT);
}

/**
 * Assign a random word from the "words.txt" file to the current word.
 */
void assign_random_word() {
 start_label:;
  char ch;

  if (!file_ptr) {
    file_ptr = fopen("words.txt", "r");
    assert_print(file_ptr, "Unable to find or open words.txt");

    fseek(file_ptr, 0, SEEK_END);
    words_length = ftell(file_ptr) - WORD_LENGTH - 1;
    fseek(file_ptr, 0, SEEK_SET);
  }

  fseek(file_ptr, rand() % words_length, SEEK_SET);

  while ((ch = fgetc(file_ptr)) != EOF) {
    if (ch == NEWLINE_CHAR) {
      for (size_t i = 0; i < WORD_LENGTH; ++i)
        current_word[i] = toupper(fgetc(file_ptr));
      return;
    }
  }

  goto start_label;
}

/**
 * Convert a string to uppercase.
 *
 * @param str The string to convert.
 */
void string_toupper(char* str) {
  while (*str)
    *(str++) = toupper(*str);
}

/**
 * Validate if the input string meets the requirements for a valid guess.
 *
 * @param str The input string to validate.
 * @return true if the input is valid; false otherwise.
 */
bool is_input_valid(const char* str) {
  for (int i = 0; i < WORD_LENGTH; ++i)
    if (!str[i] || !isalpha(str[i]))
      return false;

  return !str[LAST_CHARACTER];
}

/**
 * Render the current state of the game including guesses and feedback.
 */
void render() {
  clear_screen();
    
  for (size_t i = 0; i < GUESS_ARRAY_SIZE; ++i) {
    size_t letter_guess_array_index = i % WORD_LENGTH;

    if (letter_guess_array_index == 0 && i != 0)
      printf(NEWLINE_STR);

    if (guess_list[i])
      print_letter(guess_list[i], determine_letter_color(guess_list[i], letter_guess_array_index));
    else
      printf(LETTER_SEPERATOR_STR);
  }

  printf(NEWLINE_STR);
}

/**
 * End the game and display the outcome (win or lose).
 *
 * @param victory true if the player won; false if the player lost.
 */
void end_game(bool victory) {
  game_over = true;

  if (victory)
    printf("\nYou win!\n\n");
  else
    printf("\nYou lose, the word was: %s\n\n", current_word);

  printf("PLAY | EXIT\n");
}

/**
 * Check if the player has guessed the word correctly or if the game has ended.
 */
void check_victory() {
 start_label:;
  bool guessed_correct_word = strncmp(user_input, current_word, WORD_LENGTH) == 0;

  render();

  if (guessed_correct_word)
    end_game(PLAYER_WON);
  if (current_line == MAX_GUESSES && !guessed_correct_word)
    end_game(PLAYER_LOST);

  get_input();
  goto start_label;
}

/**
 * Clear the guess list array.
 */
void clear_guess_list() {
  for (size_t i = 0; i < GUESS_ARRAY_SIZE; ++i)
    guess_list[i] = NULL_CHAR;
}

/**
 * Start a new game by initializing the game state and selecting a new word.
 */
void start_game() {
  current_line = 0;
  game_over = false;

  clear_screen();
  clear_guess_list();
  assign_random_word();
  check_victory();
}

/**
 * Get user input and handle different commands and guesses.
 */
void get_input() {
 start_label:;
  char guess[7];

  if (fgets(guess, WORD_LENGTH + 2, stdin)) {
    guess[strcspn(guess, NEWLINE_STR)] = NULL_CHAR;
    string_toupper(guess);
  }

  if (strlen(guess) > WORD_LENGTH) 
    goto start_label;

  if (strcmp(guess, "PLAY") == 0) start_game();
  if (strcmp(guess, "EXIT") == 0) terminate(0);

  if (is_input_valid(guess) && !game_over) {
    for (int i = 0; i < WORD_LENGTH; ++i) {
      user_input[i] = guess[i];
      guess_list[(WORD_LENGTH * current_line) + i] = guess[i];
    }
    ++current_line;
  }
}

/**
 * Main function to initialize the game and enter the game loop.
 *
 * @return Exit status code.
 */
int main() {
  srand(time(NULL));

  printf("%s", COLOR_DEFAULT_TEXT);
  printf("SWORDLE - A Wordle Game written in C\n");
  printf("------------------------------------\n");
  printf("PLAY | EXIT\n");

  enter_game_loop();

  terminate(0);
}

/**
 * Terminate the program, closing any open files and exiting with the given code.
 *
 * @param exit_code The exit code to return.
 */
void terminate(uint8_t exit_code) {
  if (file_ptr)
    fclose(file_ptr);

  exit(exit_code);
}
