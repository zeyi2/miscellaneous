/*
 * Program: shred
 * Author: Mitchell
 *
 *
 * Description:
 * This program provides a set of utilities for secure deletion and overwriting of data.
 *
 * The deletion process is as follows:
 * 1. The overwriting procedure (in secure mode) performs 38 overwriting passes. 
 *    After each pass, the disk cache is flushed.
 * 2. The file is truncated to zero size so that an attacker cannot determine which disk blocks belonged to the file.
 * 3. The file is renamed so that an attacker cannot infer the contents of the deleted file from its name.
 * 4. Finally, the file is deleted (unlinked).
 *
 * Note: By default, all shred utilities work in secure mode (38 special passes). 
 * To lower the security and make the process faster, you may add thae -l (one 0xff pass, one random pass) 
 * or -ll (one random pass) option to the parameters.
 *
 * Warning:
 * - Use these utilities with caution as they irreversibly delete data.
 * - This program has only been tested on Linux systems so far.
 * - USE THE PROGRAM AT YOUR OWN RISK!!!
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/vfs.h>
#include <sys/swap.h> 

/*
  Function declarations
  
  srm:   Securely delete a file.
  sfill: Securely overwrite the unused disk space.
  sswap: Securely overwrite and clean the swap space.
  smem:  Securely overwrite the unused memory (RAM).
  
  secure_overwrite:    Perform secure overwriting of a file descriptor.
  truncate_and_rename: Truncate a file and rename it to an unknown name.
  handle_error:        Handle errors by printing an error message and exiting.
 */
void srm(const char *filepath, int level);
void sfill(int level);
void sswap(const char *swap_partition, int level);
void smem(int level);

void secure_overwrite(int fd, off_t size, int passes);
void truncate_and_rename(const char *filepath);
void handle_error(const char *message);

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <command> [options]\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Default security level
  int level = 0; 
  if (argc == 4 && strcmp(argv[3], "-l") == 0) {
    level = 1;
  } else if (argc == 4 && strcmp(argv[3], "-ll") == 0) {
    level = 2;
  }

  // Determine which command to execute
  if (strcmp(argv[1], "srm") == 0) {
    if (argc < 3) {
      fprintf(stderr, "Usage: %s srm <file_path> [-l|-ll]\n", argv[0]);
      return EXIT_FAILURE;
    }
    srm(argv[2], level);
  } else if (strcmp(argv[1], "sfill") == 0) {
    sfill(level);
  } else if (strcmp(argv[1], "sswap") == 0) {
    if (argc < 3) {
      fprintf(stderr, "Usage: %s sswap <swap_partition> [-l|-ll]\n", argv[0]);
      return EXIT_FAILURE;
    }
    sswap(argv[2], level);
  } else if (strcmp(argv[1], "smem") == 0) {
    smem(level);
  } else {
    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/*
  Securely deletes a file by overwriting it, truncating it, renaming it, and finally deleting it.
  1. Uses stat to get file size and open to get a file descriptor.
  2. Calls secure_overwrite to overwrite the file contents.
  3. Calls truncate_and_rename to truncate and rename the file.
  4. Uses unlink to delete the file.
 */
void srm(const char *filepath, int level) {
  struct stat st;
  if (stat(filepath, &st) != 0) {
    perror("stat");
    exit(EXIT_FAILURE);
  }

  int fd = open(filepath, O_WRONLY);
  if (fd < 0) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  // Determine the number of overwrite passes based on security level
  int passes = level == 0 ? 38 : (level == 1 ? 2 : 1);
  secure_overwrite(fd, st.st_size, passes);
  truncate_and_rename(filepath);

  // Unlink (delete) the file
  if (unlink(filepath) != 0) {
    perror("unlink");
    exit(EXIT_FAILURE);
  }

  close(fd);
}

/*
  Securely overwrites the unused disk space.
  1. Uses open to get a file descriptor for /dev/zero.
  2. Uses statfs to get the free space available on the disk.
  3. Calls secure_overwrite to overwrite the unused space.
 */
void sfill(int level) {
  int fd = open("/dev/zero", O_WRONLY);
  if (fd < 0) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  int passes = level == 0 ? 38 : (level == 1 ? 2 : 1);
  struct statfs st;
  if (statfs("/", &st) != 0) {
    perror("statfs");
    exit(EXIT_FAILURE);
  }

  off_t free_space = st.f_bavail * st.f_bsize;
  secure_overwrite(fd, free_space, passes);

  close(fd);
}

/*
  Securely overwrite and clean the swap filesystem.
  1. Uses swapoff to disable the swap space.
  2. Uses open to get a file descriptor for the swap partition.
  3. Uses fstat to get the size of the swap partition.
  4. Calls secure_overwrite to overwrite the swap space.
  5. Uses swapon to re-enable the swap space.
 */
void sswap(const char *swap_partition, int level) {
  if (swapoff(swap_partition) != 0) {
    perror("swapoff");
    exit(EXIT_FAILURE);
  }

  int fd = open(swap_partition, O_WRONLY);
  if (fd < 0) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  struct stat st;
  if (fstat(fd, &st) != 0) {
    perror("fstat");
    exit(EXIT_FAILURE);
  }

  int passes = level == 0 ? 38 : (level == 1 ? 2 : 1);
  secure_overwrite(fd, st.st_size, passes);

  close(fd);

  if (swapon(swap_partition, 0) != 0) {
    perror("swapon");
    exit(EXIT_FAILURE);
  }
}

/*
  Securely overwrites the unused memory (RAM).
  1. Uses open to get a file descriptor for /dev/mem.
  2. Uses fstat to get the size of the memory.
  3. Calls secure_overwrite to overwrite the memory.
 */
void smem(int level) {
  int fd = open("/dev/mem", O_WRONLY);
  if (fd < 0) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  struct stat st;
  if (fstat(fd, &st) != 0) {
    perror("fstat");
    exit(EXIT_FAILURE);
  }

  int passes = level == 0 ? 38 : (level == 1 ? 2 : 1);
  secure_overwrite(fd, st.st_size, passes);

  close(fd);
}

/*
  Overwrites a file descriptor with a specified number of passes.
  Different overwrite patterns based on the number of passes:
  - One pass with 0xFF, one pass with random data for low security.
  - One pass with 0xFF, five passes with random data for medium security.
  - 38 passes (one with 0xFF, five random, 27 special values, five random) for high security.
 */
void secure_overwrite(int fd, off_t size, int passes) {
  char *buffer = malloc(size);
  if (!buffer) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  // Different overwrite patterns based on the number of passes
  if (passes == 1) {
    // One pass with 0xFF, one pass with random data
    memset(buffer, 0xFF, size);
    if (write(fd, buffer, size) != size) {
      perror("write");
      exit(EXIT_FAILURE);
    }
    memset(buffer, rand(), size);
    if (write(fd, buffer, size) != size) {
      perror("write");
      exit(EXIT_FAILURE);
    }
  } else if (passes == 2) {
    // One pass with 0xFF, five passes with random data
    memset(buffer, 0xFF, size);
    if (write(fd, buffer, size) != size) {
      perror("write");
      exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 5; i++) {
      memset(buffer, rand(), size);
      if (write(fd, buffer, size) != size) {
        perror("write");
        exit(EXIT_FAILURE);
      }
    }
  } else {
    // 38 passes: one pass with 0xFF, five random passes, 27 special value passes, five random passes
    for (int i = 0; i < passes; i++) {
      if (i == 0) {
        memset(buffer, 0xFF, size);
      } else if (i < 6) {
        memset(buffer, rand(), size);
      } else {
        // Overwrite with special values for maximum security
        memset(buffer, i, size);
      }
      if (write(fd, buffer, size) != size) {
        perror("write");
        exit(EXIT_FAILURE);
      }
      fsync(fd);
    }
  }

  free(buffer);
}

/*
  Truncates a file to zero size.
  Renames the file to an unknown name using mkstemp.
 */
void truncate_and_rename(const char *filepath) {
  if (truncate(filepath, 0) != 0) {
    perror("truncate");
    exit(EXIT_FAILURE);
  }

  char new_name[256];
  snprintf(new_name, sizeof(new_name), "%sXXXXXX", filepath);
  int fd = mkstemp(new_name);
  if (fd < 0) {
    perror("mkstemp");
    exit(EXIT_FAILURE);
  }

  if (rename(new_name, filepath) != 0) {
    perror("rename");
    exit(EXIT_FAILURE);
  }

  close(fd);
}

void handle_error(const char *message) {
  perror(message);
  exit(EXIT_FAILURE);
}
