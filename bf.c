#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// #define DEBUG_PARSE
// #define DEBUG_EXECUTE

#define TAPE_SIZE 256

unsigned char Tape[TAPE_SIZE] = {0};
uint8_t Ptr = 0;

static inline void inc_cell(void) { Tape[Ptr]++; }
static inline void dec_cell(void) { Tape[Ptr]--; }
static inline void move_right(void) { Ptr++; }
static inline void move_left(void) { Ptr--; }
static inline void output_byte(void) { putchar(Tape[Ptr]); }
static inline void read_byte(void) {
    int c = getchar();
    if (c == EOF) Tape[Ptr] = 0;
    else Tape[Ptr] = (unsigned char)c;
}

static inline void loop_enter(int32_t *pc, const int32_t *jump) {
    if (Tape[Ptr] == 0) {
        *pc = jump[*pc];
    }
}

static inline void loop_exit(int32_t *pc, const int32_t *jump) {
    if (Tape[Ptr] != 0) {
        *pc = jump[*pc];
    }
}


void brainfuck_interpreter(const char *code) {
    size_t code_len = 0;
    while (code[code_len] != '\0') code_len++;

    int32_t *jump = malloc(sizeof(int32_t) * code_len);
    if (!jump) {
        perror("malloc jump table");
        return;
    }

    int32_t *stack = malloc(sizeof(int32_t) * code_len);
    if (!stack) {
        perror("malloc stack");
        free(jump);
        return;
    }

    int32_t top = 0;

    // Preprocess: build jump table
    for (int32_t i = 0; i < (int32_t)code_len; i++) {
        if (code[i] == '[') {
            stack[top++] = i;
        } else if (code[i] == ']') {
            if (top == 0) {
                fprintf(stderr, "Unmatched ] at %d\n", i);
                free(stack);
                free(jump);
                return;
            }
            int32_t j = stack[--top];
            jump[i] = j;
            jump[j] = i;
        }
    }

    if (top != 0) {
        fprintf(stderr, "Unmatched [ in program\n");
        free(stack);
        free(jump);
        return;
    }

    free(stack);

    // Execution
    for (int32_t pc = 0; pc < (int32_t)code_len; pc++) {
        switch (code[pc]) {
            case '>': move_right(); break;
            case '<': move_left(); break;
            case '+': inc_cell(); break;
            case '-': dec_cell(); break;
            case '.': output_byte(); break;
            case ',': read_byte(); break;
            case '[': loop_enter(&pc, jump); break;
            case ']': loop_exit(&pc, jump); break;
            default: break;
        }
#ifdef DEBUG_EXECUTE
        printf("pc=%d ptr=%u cell=%u\n", pc, Ptr, Tape[Ptr]);
#endif
    }

    free(jump);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *code = malloc(size + 1);
    if (!code) {
        perror("malloc code");
        fclose(file);
        return 1;
    }

    fread(code, 1, size, file);
    code[size] = '\0';

    fclose(file);

    brainfuck_interpreter(code);

    free(code);
    return 0;
}
