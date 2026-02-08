#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TAPE_SIZE 256

// 定義 Context 來封裝狀態，避免全域變數
typedef struct {
    uint8_t *tape;
    size_t ptr;
    size_t tape_size;
} BFContext;

// 統一函數簽名
typedef void (*opcode_handler)(BFContext *ctx, int32_t *pc, const int32_t *jump);

static void op_inc_cell(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    (void)pc; (void)jump;
    ctx->tape[ctx->ptr]++; 
}

static void op_dec_cell(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    (void)pc; (void)jump;
    ctx->tape[ctx->ptr]--; 
}

static void op_move_right(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    (void)pc; (void)jump;
    if (ctx->ptr < ctx->tape_size - 1) ctx->ptr++; 
}

static void op_move_left(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    (void)pc; (void)jump;
    if (ctx->ptr > 0) ctx->ptr--; 
}

static void op_output(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    (void)pc; (void)jump;
    putchar(ctx->tape[ctx->ptr]); 
}

static void op_input(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    (void)pc; (void)jump;
    int c = getchar();
    if (c == EOF) {
        ctx->tape[ctx->ptr] = 0;
    } else {
        ctx->tape[ctx->ptr] = (uint8_t)c;
    }
}

static void op_loop_enter(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    if (ctx->tape[ctx->ptr] == 0) {
        *pc = jump[*pc];
    }
}

static void op_loop_exit(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    if (ctx->tape[ctx->ptr] != 0) {
        *pc = jump[*pc];
    }
}

static void op_noop(BFContext *ctx, int32_t *pc, const int32_t *jump) { 
    (void)ctx; (void)pc; (void)jump; 
}

static opcode_handler dispatch_table[256];

static void init_dispatch_table(void) {
    for (int i = 0; i < 256; i++) dispatch_table[i] = op_noop;
    dispatch_table['>'] = op_move_right;
    dispatch_table['<'] = op_move_left;
    dispatch_table['+'] = op_inc_cell;
    dispatch_table['-'] = op_dec_cell;
    dispatch_table['.'] = op_output;
    dispatch_table[','] = op_input;
    dispatch_table['['] = op_loop_enter;
    dispatch_table[']'] = op_loop_exit;
}

void brainfuck_interpreter(const char *code) {
    size_t code_len = strlen(code);
    int32_t *jump = malloc(sizeof(int32_t) * code_len);
    int32_t *stack = malloc(sizeof(int32_t) * code_len);
    
    // 初始化 Context
    BFContext ctx;
    ctx.tape = calloc(TAPE_SIZE, sizeof(uint8_t));
    ctx.ptr = 0;
    ctx.tape_size = TAPE_SIZE;

    if (!jump || !stack || !ctx.tape) {
        perror("Memory allocation failed");
        // 記得 free 其他已分配的記憶體
        free(jump); free(stack); free(ctx.tape);
        return;
    }

    // ... (Jump table 建置邏輯保持不變，但記得檢查錯誤) ...
    int32_t top = 0;
    for (int32_t i = 0; i < (int32_t)code_len; i++) {
        if (code[i] == '[') {
            stack[top++] = i;
        } else if (code[i] == ']') {
            if (top == 0) { /* Error handling */ break; }
            int32_t j = stack[--top];
            jump[i] = j;
            jump[j] = i;
        }
    }
    free(stack);

    init_dispatch_table(); // 注意：如果是多線程這裡只能初始化一次

    // Execution
    for (int32_t pc = 0; pc < (int32_t)code_len; pc++) {
        uint8_t op = (uint8_t)code[pc];
        dispatch_table[op](&ctx, &pc, jump);
    }

    free(jump);
    free(ctx.tape);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
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
