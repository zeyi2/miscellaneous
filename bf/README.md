# bf.c
bf.c is a brainfuck interpreter / compiler (using GCC as backend)

## Implementation details
- Cell Bounds: from 0 to 32767
- Tape Size: 16777216
- EOF leaves the cell values unchanged
- New line on I/O is 10 ('\n')

## Install
```shell
sudo make install
```

## Usage
- Help
```shell
bf -h

MiniBf 0.3
A simple Brainfuck interpreter / compiler.

Usage:
  bf                      Run the interpreter interactively.
  bf -h                   Display this help message.
  bf -f <filename>        Execute Brainfuck code from a file.
  bf -t <filename>        Convert Brainfuck code to C code.
  bf -c <input> <output>  Compile Brainfuck code to an executable.

Commands:
  +                       Increment the current cell
  -                       Decrement the current cell
  >                       Move the pointer to the right
  <                       Move the pointer to the left
  [                       Jump past the matching ] if the cell at the pointer is 0
  ]                       Jump back to the matching [ if the cell at the pointer is nonzero
  .                       Output the character at the pointer
  ,                       Input a character and store it in the cell at the pointer
  #                       Output the value of the current cell for debugging
  @                       Output the values of all cells used so far for debugging

Controls:
  Ctrl + D                Execute the entered code.
  Ctrl + C                Exit the interpreter.

```

- Interactive Interpreter
    
  Press Ctrl+D to evaluate programs.  
  Press Ctrl+C to Exit.
      
```shell
bf

    MiniBf 0.3

    Input bf -h for help

    This could be the start of a beautiful program.

>>>#++#>>+++++#@       


# DEBUG INFO (1):
cell #3: 0


# DEBUG INFO (2):
cell #3: 2


# DEBUG INFO (3):
cell #5: 5


@ DEBUG INFO (1):
#0: 0  #1: 0  #2: 0  #3: 2  #4: 0  
#5: 5  


```
- Read from files
```shell
cat foo.bf
>++++[>++++++<-]>-[[<+++++>>+<-]>-]<<[<]>>>>-
-.<<<-.>>>-.<.<.>---.<<+++.>>>++.<<---.[>]<<.

bf -f foo.bf
brainfuck

```
- Compile bf to executables (UNIX only / GCC required)  
**Currently no vaildation, you need to make sure the bf code is correct.**
```shell
bf -c foo.bf foo

Brainfuck code converted to C code in temp_output.c
Executable created: foo

./foo
brainfuck
```
- Other compilers  
You can convert the bf code into c code first.
```shell
bf -t foo.bf
Brainfuck code converted to C code in output.c

clang output.c
./a.out
brainfuck
```
