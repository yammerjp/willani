#include "willani.h"

static void gen(Node *node);

static void gen_num(Node *node) {
  printf("  # gen_num\n");
  printf("  push %ld\n", node->value); // push constant
  }

static void gen_binary_operator(Node *node) {
  printf("  # gen_op2 left\n");
  gen(node->left);
  printf("  # gen_op2 right\n");
  gen(node->right);
  printf("  # gen_op2 operator\n");

  printf("  pop rdi\n");          // load the stack top to rdi to calculate
  printf("  pop rax\n");          // load the stack top to rax to calculate

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");   // rax += rdi
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");   // rax -= rdi
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");  // rax *= rdi
    break;
  case ND_DIV:
    printf("  cqo\n");            // [rdx rax](128bit) = rax (64bit)
    printf("  idiv rdi\n");       // rax = [rdx rax] / rdi
                                  // rdx = [rdx rax] % rdi
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");   // set flag register with comparing rax and rdi
    printf("  sete al\n");        // al = ( flag register means rax == rdi ) ? 1 : 0
    printf("  movzb rax, al\n");  // rax(64bit) = al(8bit)
                                  // al refer to the lower 8 bits of the rax
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");       // al = ( flag register means rax != rdi ) ? 1 : 0
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");        // al = ( flag register means rax < rdi ) ? 1 : 0
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");       // al = ( flag register means rax <= rdi ) ? 1 : 0
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");         // store result to stack top
}

static void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    gen_num(node);
    return;
  }

  // expect binary operator node
  gen_binary_operator(node);
}

static void prologue(void) {
  printf("  # prologue\n");
  printf("  push rbp\n");         // record caller's rbp
  printf("  mov rbp, rsp\n");     // set current stack top to rbp
  printf("  sub rsp, 208\n");     // allocate memory for a-z variables
}

static void epilogue(void) {
  printf("  # epilogue\n");       // rax is already set, and will be exit code
  printf("  mov rsp, rbp\n");   // ignore the remanig data in the stack
  printf("  pop rbp\n");        // set caller's rbp to rsp
  printf("  ret\n");
}

void code_generate(Node *node) {

  // assembly code header
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  prologue();

  for(Node *n = node; n; n = n->next) {
    gen(n);
    printf("  pop rax\n"); // load result(stack top) to rax
  }

  epilogue();
}
