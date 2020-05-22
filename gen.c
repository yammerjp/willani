#include "willani.h"

void gen(Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %ld\n", node->value);
    return;
  }

  // expect op2
  gen(node->left);
  gen(node->right);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");
}

void code_generate(Node *node) {

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  for(Node *n = node; n; n = n->next) {
    gen(n);
    printf("  pop rax\n");
  }
  printf("  ret\n");
}
