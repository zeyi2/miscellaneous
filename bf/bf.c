#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#define VERSION "0.3"
#define TAPESIZE 16777216

int stack[TAPESIZE], stack_ptr;
int loops[TAPESIZE], source_ptr, source_length, user_input;
short int tape[TAPESIZE], cell, max_cell_used = 0;
char source[TAPESIZE];
int debug_counter = 1, memory_counter = 1;

#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RED     "\x1b[31m"

void InitLoops();
void ExecuteSource();
void handle_sigint(int sig);
void print_help();
void read_file(const char *filename);
void bf_to_c(const char *input_filename, const char *output_filename);
void compile_c_to_executable(const char *c_filename, const char *executable_filename);

int main(int argc, char **argv) {
  signal(SIGINT, handle_sigint);

  if (argc > 1) {
    if (strcmp(argv[1], "-h") == 0) {
      print_help();
      return 0;
    } else if (strcmp(argv[1], "-f") == 0) {
      if (argc < 3) {
        fprintf(stderr, COLOR_RED "Error: No file specified.\n" COLOR_RESET);
        return 1;
      }
      read_file(argv[2]);
      return 0;
    } else if (strcmp(argv[1], "-t") == 0) {
      if (argc < 3) {
        fprintf(stderr, COLOR_RED "Error: No file specified.\n" COLOR_RESET);
        return 1;
      }
      bf_to_c(argv[2], "output.c");
      return 0;
    } else if (strcmp(argv[1], "-c") == 0) {
      if (argc < 4) {
        fprintf(stderr, COLOR_RED "Error: No file specified.\n" COLOR_RESET);
        return 1;
      }
      bf_to_c(argv[2], "temp_output.c");
      compile_c_to_executable("temp_output.c", argv[3]);
      remove("temp_output.c");
      return 0;
    }
  }

  printf("\n    MiniBf %s\n", VERSION);
  printf("\n    TAPE SIZE: %d", TAPESIZE);
  printf("\n    CELL SIZE: 0-32767\n");
  printf("\n    Input 'bf -h' for help\n\n");

  char c;
  while (1) {
    source_length = 0;
    memset(source, 0, sizeof(source));
    while (source_length < TAPESIZE - 1 && (c = getchar()) != EOF) {
      source[source_length++] = c;
    }

    if (c == EOF) {
      if (feof(stdin)) {
        clearerr(stdin);
        ExecuteSource();
        memset(tape, 0, sizeof(tape));
        cell = 0;
        max_cell_used = 0;
        debug_counter = 1;
        memory_counter = 1;
      } else {
        break;
      }
    }
  }

  return 0;
}

void read_file(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    fprintf(stderr, COLOR_RED "Error: Could not open file %s\n" COLOR_RESET, filename);
    exit(1);
  }

  source_length = fread(source, 1, TAPESIZE, file);
  fclose(file);

  ExecuteSource();
}

void InitLoops() {
  stack_ptr = 0;
  for (source_ptr = 0; source_ptr < source_length; source_ptr++) {
    if (source[source_ptr] == '[') stack[stack_ptr++] = source_ptr;
    if (source[source_ptr] == ']') {
      if (!stack_ptr) {
        fprintf(stderr, COLOR_RED "\n\nError: couldn't find matching '[' for ']' at byte %d\n" COLOR_RESET, source_ptr);
        fprintf(stderr, "%s\n", source);
        for (int i = 0; i < source_ptr; i++) fprintf(stderr, " ");
        fprintf(stderr, COLOR_RED "^ missing '['\n" COLOR_RESET);
      } else {
        --stack_ptr;
        loops[source_ptr] = stack[stack_ptr];
        loops[stack[stack_ptr]] = source_ptr;
      }
    }
  }

  if (stack_ptr > 0) {
    fprintf(stderr, COLOR_RED "\n\nError: couldn't find matching ']' for '[' at byte %d\n" COLOR_RESET, stack[--stack_ptr]);
    fprintf(stderr, "%s\n", source);
    for (int i = 0; i < stack[stack_ptr]; i++) fprintf(stderr, " ");
    fprintf(stderr, COLOR_RED "^ missing ']'\n" COLOR_RESET);
  }
}

void ExecuteSource() {
  InitLoops();
  for (source_ptr = 0; source_ptr < source_length; source_ptr++) {
    switch (source[source_ptr]) {
    case '+': tape[cell]++; break;
    case '-': if (tape[cell] > 0) tape[cell]--; break;
    case '<': cell--; break;
    case '>': cell++; if (cell > max_cell_used) max_cell_used = cell; break;
    case ',':
      user_input = getchar();
      if (user_input == EOF) {
        //DO NOTHING
      } else {
        tape[cell] = user_input;
        if (tape[cell] == '\n') {
          tape[cell] = 10;
        }
      }
      break;
    case '.': putchar(tape[cell]); break;
    case '[': if (!tape[cell]) source_ptr = loops[source_ptr]; break;
    case ']': if (tape[cell]) source_ptr = loops[source_ptr]; break;
    case '#':
      printf(COLOR_YELLOW "\n\n# DEBUG INFO (%d):\n" COLOR_RESET, debug_counter++);
      printf("cell #%d: %d\n", cell, tape[cell]);
      break;
    case '@':
      printf(COLOR_GREEN "\n\n@ DEBUG INFO (%d):\n" COLOR_RESET, memory_counter++);
      for (int i = 0; i <= max_cell_used; i++) {
        printf("#%d: %d  ", i, tape[i]);
        if (i % 5 == 4) printf("\n");
      }
      printf("\n");
      break;
    }
  }
  printf("\n");
}

void bf_to_c(const char *input_filename, const char *output_filename) {
  FILE *in = fopen(input_filename, "r");
  if (!in) {
    fprintf(stderr, COLOR_RED "Error: Could not open file %s\n" COLOR_RESET, input_filename);
    exit(1);
  }

  FILE *out = fopen(output_filename, "w");
  if (!out) {
    fprintf(stderr, COLOR_RED "Error: Could not open output file %s.\n" COLOR_RESET, output_filename);
    exit(1);
  }

  int c;
  int cellsize = TAPESIZE;

  fprintf(out,
          "#include <stdio.h>\n"
          "#include <stdlib.h>\n\n"
          "int main(int argc, char **argv)\n{\n"
          "\tunsigned char *cell = calloc(%d, 1);\n"
          "\tunsigned char *cells = cell;\n"
          "\tif (!cell) {\n"
          "\t\tfprintf(stderr, \"Error allocating memory.\\n\");\n"
          "\t\treturn 1;\n"
          "\t}\n\n", cellsize
          );
  while ((c = getc(in)) != EOF) {
    switch (c) {
    case '>': fprintf(out, "\t\t++cell;\n"); break;
    case '<': fprintf(out, "\t\t--cell;\n"); break;
    case '+': fprintf(out, "\t\t++*cell;\n"); break;
    case '-': fprintf(out, "\t\t--*cell;\n"); break;
    case '.': fprintf(out, "\t\tputchar(*cell);\n"); break;
    case ',': fprintf(out, "\t\t*cell = getchar();\n"); break;
    case '[': fprintf(out, "\twhile (*cell) {\n"); break;
    case ']': fprintf(out, "\t}\n"); break;
    default: break;
    }
  }
	
  fprintf(out, "\n\tfree(cells);\n\treturn 0;\n}\n\n");
  fclose(in);
  fclose(out);
  printf("Brainfuck code converted to C code in %s\n", output_filename);
}

void compile_c_to_executable(const char *c_filename, const char *executable_filename) {
  char command[256];
  snprintf(command, sizeof(command), "gcc %s -o %s", c_filename, executable_filename);
  int result = system(command);
  if (result == -1) {
    fprintf(stderr, "Error: Compilation failed.\n");
  } else {
    printf("Executable created: %s\n", executable_filename);
  }
}

void handle_sigint(int sig) {
  (void)sig; 
  printf("\nProcess Terminated\n");
  exit(0);
}

void print_help() {
  printf("\nMiniBf %s\n", VERSION);
  printf("A simple Brainfuck interpreter / compiler.\n");
  printf("\nUsage:\n");
  printf("  bf                      Run the interpreter interactively.\n");
  printf("  bf -h                   Display this help message.\n");
  printf("  bf -f <filename>        Execute Brainfuck code from a file.\n");
  printf("  bf -t <filename>        Convert Brainfuck code to C code.\n");
  printf("  bf -c <input> <output>  Compile Brainfuck code to an executable.\n");
  printf("\nCommands:\n");
  printf("  +                       Increment the current cell\n");
  printf("  -                       Decrement the current cell\n");
  printf("  >                       Move the pointer to the right\n");
  printf("  <                       Move the pointer to the left\n");
  printf("  [                       Jump past the matching ] if the cell at the pointer is 0\n");
  printf("  ]                       Jump back to the matching [ if the cell at the pointer is nonzero\n");
  printf("  .                       Output the character at the pointer\n");
  printf("  ,                       Input a character and store it in the cell at the pointer\n");
  printf("  #                       Output the value of the current cell for debugging\n");
  printf("  @                       Output the values of all cells used so far for debugging\n");
  printf("\nControls:\n");
  printf("  Ctrl + D                Execute the entered code.\n");
  printf("  Ctrl + C                Exit the interpreter.\n\n");
}
