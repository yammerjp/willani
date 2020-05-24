#include "willani.h"

static void gen_num(Node *node);
static void load(void);
static void store(void);
static void gen_if(Node *node);
static void gen_while(Node *node);
static void gen_addr(Node *node);
static void gen(Node *node);
static void gen_binary_operator(Node *node);
static void prologue(int offset);
static void epilogue(void);


static void gen_num(Node *node) {
  printf("  push %ld\n", node->value); // push constant
}


// change the stack top from addr to value
static void load(void) {
  printf("  pop rax\n");          // load the stack top to rax
  printf("  mov rax, [rax]\n");   // load the actual value of rax to rax
  printf("  push rax\n");         // store rax to the stack top
}

// store value to the variable.
static void store(void) {
  // stack
  // before : (top) value, (variable's address), ...
  // after  : (top) value, ...
  printf("  pop rdi\n");          // load the stack top to rdi
  printf("  pop rax\n");          // load the stack top to rax
  printf("  mov [rax], rdi\n");   // copy rdi's value to the address pointed by rax
  printf("  push rdi\n");         // store rdi to the stack top
}

// load the address of node's variable to the stack top
static void gen_addr(Node *node) {
  if (node->kind != ND_VAR) {
    error("Left side is expected a variable.");
  }
  printf("  lea rax, [rbp-%d]\n", node->lvar->offset); // load the address of the actual value of (rbp - offset)
  printf("  push rax\n");         // push rbp - offset
}

int label_count = 1;

static void gen_if(Node *node) {
  int labct = label_count ++;
  if (node->kind != ND_IF) {
    error("expected node->kind is ND_IF");
  }
  gen(node->cond);               // calculate condition
  printf("  pop rax\n");         // load result to the stach top
  printf("  cmp rax, 0\n");       // evaluate result

  if (node->els) 
    printf("  je  .L.else.%d\n", labct); // jump if result is false
  else 
    printf("  je  .L.end.%d\n", labct); // jump if result is false

  gen(node->then);
  printf("  jmp .L.end.%d\n", labct); // end then stmt

  if (node->els) {
    printf(".L.else.%d:\n", labct); // label
    gen(node->els);
  }

  printf(".L.end.%d:\n", labct);   // label
}

static void gen_while(Node *node) {
  int labct = label_count ++;
  if (node->kind != ND_WHILE) {
    error("expected node->kind is ND_WHILE");
  }
  printf(".L.begin.%d:\n", labct); // label
  gen(node->cond);               // calculate condition
  printf("  pop rax\n");         // load result to the stach top
  printf("  cmp rax, 0\n");       // evaluate result

  printf("  je  .L.end.%d\n", labct); // jump if result is false

  gen(node->then);
  printf("  jmp .L.begin.%d\n", labct); // jump cond

  printf(".L.end.%d:\n", labct);   // label
}

static void gen_for(Node *node) {
  int labct = label_count ++;
  if (node->kind != ND_FOR) {
    error("expected node->kind is ND_FOR");
  }
  gen(node->init);

  printf(".L.begin.%d:\n", labct); // label
  gen(node->cond);               // calculate condition
  printf("  pop rax\n");         // load result to the stach top
  printf("  cmp rax, 0\n");       // evaluate result

  printf("  je  .L.end.%d\n", labct); // jump if result is false

  gen(node->then);
  gen(node->increment);
  printf("  jmp .L.begin.%d\n", labct); // jump cond

  printf(".L.end.%d:\n", labct);   // label
}

static void gen_block(Node *node) {
  if (node->kind != ND_BLOCK) {
    error("expected { ... }");
  }

  for(Node *n = node->body; n; n = n->next) {
    gen(n);
    printf("  pop rax\n"); // load result(stack top) to rax
  }
}

static void gen_func_call(Node *node) {
  if (node->kind != ND_FUNC_CALL) {
    error("expected function call");
  }
  printf("  call %.*s\n", node->funcnamelen, node->funcname);
  printf("  push rax\n");
}


static void gen(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    gen_num(node);
    return;
  case ND_VAR:
    gen_addr(node);
    load();
    return;
  case ND_ASSIGN:
    gen_addr(node->left);
    gen(node->right);
    store();
    return;
  case ND_RETURN:
    gen(node->left);
    printf("  pop rax\n");
    printf("  jmp .L.return\n");
    return;
  case ND_IF:
    gen_if(node);
    return;
  case ND_WHILE:
    gen_while(node);
    return;
  case ND_FOR:
    gen_for(node);
    return;
  case ND_BLOCK:
    gen_block(node);
    return;
  case ND_FUNC_CALL:
    gen_func_call(node);
    return;
  }

  // expect binary operator node
  gen_binary_operator(node);
}


static void gen_binary_operator(Node *node) {
  gen(node->left);
  gen(node->right);

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


static void prologue(int offset) {
  printf("  push rbp\n");         // record caller's rbp
  printf("  mov rbp, rsp\n");     // set current stack top to rbp
  printf("  sub rsp, %d\n", offset);     // allocate memory for a-z variables
}

static void epilogue(void) {
  printf(".L.return:\n");
  printf("  mov rsp, rbp\n");   // ignore the remanig data in the stack
  printf("  pop rbp\n");        // set caller's rbp to rsp
  printf("  ret\n");
}

void code_generate(Function *func) {

  // assembly code header
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  int offset = func->lvar ? (func->lvar->offset) : 0;
  prologue(offset);

  for(Node *n = func->node; n; n = n->next) {
    gen(n);
    printf("  pop rax\n"); // load result(stack top) to rax
  }

  epilogue();
}
