#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>

#define BUF_SIZE (4096*4096)
#define MEM_SIZE 30000

#define must(ok, msg) if (!(ok)) { perror(msg); return 1; }
#define eprintf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

typedef struct {
  uint8_t* code;
  int len; /* bytecode can have \0 inside it so strlen doesn't work */
} BC;

int run(BC bytecode);
BC compile(char* code);

int main(int argc, char* argv[]) {
  if (argc < 2) {
    eprintf("Usage: %s <file.bf>", argv[0]);
    return 1;
  }
  char* filename = argv[1];

  char* code = calloc(BUF_SIZE, sizeof(char));
  FILE* file = fopen(filename, "r");
  must(file, "fopen");
  size_t nread = fread(code, sizeof(char), BUF_SIZE, file);
  // eprintf("read %zu bytes of %d\n", nread, BUF_SIZE);
  must(nread <= BUF_SIZE, "fread");
  must(!fclose(file), "fclose");

  // eprintf("code len: %lu\n", strlen(code));
  BC bytecode = compile(code);
  int ret = run(bytecode);
  free(code);
  return ret;
}

BC compile(char* code) {
  int len = strlen(code);
  uint8_t* bytecode = calloc(BUF_SIZE, sizeof(uint8_t));
  int j = 0;

  uint16_t stack[256] = {};
  int stack_ptr = 0;

  for (int i = 0; i < len; i++) {
    char op = code[i];
    switch (op) {
      case '>':
      case '<':
      case '+':
      case '-':
      case '.':
      case ',':
        bytecode[j++] = op;
        break;
      case '[':
        bytecode[j++] = 'j';  // jump
        bytecode[j++] = true; // if zero
        j++; j++; // leave room for jump to ] when it becomes available
        stack[stack_ptr++] = j;
        break;
      case ']': {
        uint16_t pos = stack[--stack_ptr]; // pop pos of [ off stack
        bytecode[j++] = 'j';   // jump
        bytecode[j++] = false; // if nonzero
        bytecode[j++] =  pos & 0x00ff;
        bytecode[j++] = (pos & 0xff00) >> 8;

        bytecode[pos-2] =  j & 0x00ff;
        bytecode[pos-1] = (j & 0xff00) >> 8;

        break;
       }
    }
  }
  // eprintf("j: %d len: %d\n", j, len);


  BC bc = {.code = bytecode, .len = j};
  return bc;
}

// Run compiled brainfuck bytecode
int run(BC bytecode) {
  int len = bytecode.len;
  uint8_t* code = bytecode.code;
  int pc = 0;
  int ptr = 0;
  char* mem = calloc(MEM_SIZE, sizeof(char));

  uint64_t count = 0;
  while (pc < len) {
    count++;
    assert(pc >= 0);
    assert(ptr >= 0);

    char op = code[pc++];

    switch (op) {
      case '>': ptr++; break;
      case '<': ptr--; break;
      case '+': mem[ptr]++; break;
      case '-': mem[ptr]--; break;
      case '.': putchar(mem[ptr]); fflush(NULL); break;
      case ',': mem[ptr] = getchar(); break;
      case 'j': {
          bool want = code[pc++];
          bool should_jump = (mem[ptr] == 0) == want;
          uint8_t a = code[pc++];
          uint8_t b = code[pc++];
          uint16_t dest = a | (b << 8);
          /* printf("dest: %u from a %d b %d\n", dest, a, b); */
          if (should_jump) pc = dest;
          break;
      }
    }
  }

  eprintf("\nDone, ran %lu instructions\n", count);
  return 0;
}
