#include "willani.h"

static void gen_num(Node *node);
static void load(void);
static void store(void);
static void gen_if(Node *node);
static void gen_while(Node *node);
static void gen_addr(Node *node);
static void gen(Node *node);
static void gen_binary_operator(Node *node);
static void prologue(Function *func);
static void epilogue(void);

char arg_registers[][4] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

static void print_node_with_comment_begin(Node *node) {
  printf ("  # >>> "); print_node(stdout, node);
}
static void print_node_with_comment_end(Node *node) {
  printf ("  # <<< "); print_node(stdout, node);
}

static void gen_num(Node *node) {
  print_node_with_comment_begin(node);
  printf("  push %ld\n", node->value); // push constant
  print_node_with_comment_end(node);
}

char *funcname = NULL;
int funcnamelen = 0;

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
  if (node->kind == ND_VAR) {
    printf("  lea rax, [rbp-%d]\n", node->lvar->offset); // load the address of the actual value of (rbp - offset)
    printf("  push rax\n");         // push rbp - offset
    return;
  }
  if (node->kind == ND_DEREF) {
    gen(node->left);
    return;
  }
  error("Left side is expected a variable or *variable.");
}

int label_count = 1;

static void gen_if(Node *node) {
  int labct = label_count ++;
  if (node->kind != ND_IF) {
    error("expected node->kind is ND_IF");
  }
  print_node_with_comment_begin(node);
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
  print_node_with_comment_end(node);
}

static void gen_while(Node *node) {
  print_node_with_comment_begin(node);
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

  print_node_with_comment_end(node);
}

static void gen_for(Node *node) {
  print_node_with_comment_begin(node);
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
  print_node_with_comment_end(node);
}

static void gen_block(Node *node) {
  print_node_with_comment_begin(node);
  if (node->kind != ND_BLOCK) {
    error("expected { ... }");
  }

  for(Node *n = node->body; n; n = n->next) {
    gen(n);
  }
  print_node_with_comment_end(node);
}

static void gen_func_call(Node *node) {
  print_node_with_comment_begin(node);
//  printf("  pop rax\n");      // save rax
  if (node->kind != ND_FUNC_CALL) {
    error("expected function call");
  }

  int i = 0;
  for (Node *cur = node->fncl->args; cur; cur = cur->next) {
    gen(cur);
    printf("  pop  %s\n", arg_registers[i++]);
  }

  // align RSP to a 16 bite boundary
  int labct = label_count ++;
  printf("  mov rax, rsp\n");
  printf("  and rax, 15\n");
  printf("  jne .L.needAlign.%d\n", labct);

  printf("  call %.*s\n", node->fncl->length, node->fncl->name);
  printf("  jmp .L.end.%d\n", labct);

  printf(".L.needAlign.%d:\n", labct);
  printf("  sub rsp, 8\n");
  printf("  call %.*s\n", node->fncl->length, node->fncl->name);
  printf("  add rsp, 8\n");

  printf(".L.end.%d:\n", labct);
  printf("  push rax\n");    // restore saved rax
}

static void gen_var(Node *node) {
  print_node_with_comment_begin(node);
  gen_addr(node);
  load();
  print_node_with_comment_end(node);
}

static void gen_assign(Node *node) {
  if (node->kind != ND_ASSIGN) {
    error("expected node->kind is ND_ASSIGN");
  }
  print_node_with_comment_begin(node);
  gen_addr(node->left);
  gen(node->right);
  store();
  print_node_with_comment_end(node);
}

static void gen_return(Node *node) {
  if (node->kind != ND_RETURN) {
    error("expected node->kind is ND_RETURN");
  }
  print_node_with_comment_begin(node);
  gen(node->left);
  printf("  pop rax\n");
  printf("  jmp .L.return.%.*s\n", funcnamelen, funcname);
  print_node_with_comment_end(node);
}

static void gen_expr_stmt(Node *node) {
  if (node->kind != ND_EXPR_STMT) {
    error("expected node->kind is ND_EXPR_STMT");
  }
  print_node_with_comment_begin(node);
  gen(node->left);
  printf("  add rsp, 8\n"); // stmt is not leave any values in the stack
  print_node_with_comment_end(node);
}

static void gen(Node *node) {
  switch (node->kind) {
  // statements
  case ND_RETURN:
    gen_return(node);
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
  case ND_EXPR_STMT:
    gen_expr_stmt(node);
   return;
  case ND_DECLARE_VAR:
    // skip
    return;

  // expression
  case ND_NUM:
    gen_num(node);
    return;
  case ND_VAR:
    gen_var(node);
    return;
  case ND_ASSIGN:
    gen_assign(node);
    return;
  case ND_FUNC_CALL:
    gen_func_call(node);
    return;
  case ND_ADDR:
    print_node_with_comment_begin(node);
    gen_addr(node->left);
    print_node_with_comment_end(node);
    return;
  case ND_DEREF:
    print_node_with_comment_begin(node);
    gen(node->left);
    load();
    print_node_with_comment_end(node);
    return;
  }

  // expression
  // expect binary operator node
  gen_binary_operator(node);
}


static void gen_binary_operator(Node *node) {
  print_node_with_comment_begin(node);
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
  print_node_with_comment_end(node);
}

static int argc(LVar *args) {
  int i = 0;
  for (LVar *current = args; current; current = current->next) {
    i ++;
  }
  return i;
}

static void prologue(Function *func) {
  int offset = func->lvar ? (func->lvar->offset) : 0;

  printf("  push rbp\n");         // record caller's rbp
  printf("  mov rbp, rsp\n");     // set current stack top to rbp
  printf("  sub rsp, %d\n", offset);     // allocate memory for a-z variables

  int i = 0;
  int arglength = argc(func->args);
  for (LVar *current = func->args; current; current = current->next) {
    i ++;
    printf("  mov [rbp-%d], %s\n", current->offset, arg_registers[arglength-i]);
  }
}

static void epilogue(void) {
  printf(".L.return.%.*s:\n", funcnamelen, funcname);
  printf("  mov rsp, rbp\n");   // ignore the remanig data in the stack
  printf("  pop rbp\n");        // set caller's rbp to rsp
  printf("  ret\n");
}

void gen_function(Function *func) {
  funcname = func->name;
  funcnamelen = func->namelen;
  printf("%.*s:\n", func->namelen, func->name);
  prologue(func);

  gen(func->node);

  epilogue();
}

void gen_func_names(Function *head) {
  bool isHead = true;
  for (Function *current = head; current; current = current->next) {
    if (isHead) {
      isHead = false;
    } else {
      printf(", ");
    }
    printf("%.*s", current->namelen, current->name);
  }
}

void code_generate(Function *func) {
  // assembly code header
  printf(".intel_syntax noprefix\n");
  printf(".global "); gen_func_names(func); printf("\n");

  for (Function *current = func; current; current = current->next) {
    gen_function(current);
  }
}
