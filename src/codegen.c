#include "willani.h"

static FILE *file;

static void load(Type *type);
static void store(Type *type);
static void gen_if(Node *node);
static void gen_while(Node *node);
static void gen_addr(Node *node);
static void gen_ternary(Node *node);
static void gen_log_or(Node *node);
static void gen_log_and(Node *node);
static void gen(Node *node);
static void gen_binary_operator(Node *node);
static void prologue(Function *func);
static void epilogue();

static char arg_regs1[][4] = { "dil", "sil", "dl", "cl", "r8b", "r9b" };
static char arg_regs4[][4] = { "edi", "esi", "edx", "ecx", "r8d", "r9d" };
static char arg_regs8[][4] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

//======================================
// labels
// identify jump label of break, continue and case
typedef struct Stack Stack;
struct Stack {
  int labct;
  Stack *parent;
};
static void push(Stack **stack,int labct) {
  Stack *s = calloc(1, sizeof(Stack));
  s->labct = labct;
  s->parent = *stack;
  *stack = s;
}
static int pop(Stack **stack) {
  int labct = (*stack)->labct;
  *stack = (*stack)->parent;
  return labct;
}
static Stack *continue_stack;
static Stack *break_stack;
static Stack *switch_stack;

// identify jump label of return
static char *funcname = NULL;
static int funcnamelen = 0;

static int label_count = 1;




//======================================
// load and store
// change the stack top from addr to value
static void load(Type *type) {
  if (type->kind == TYPE_ARRAY)
    return;

  fprintf(file, "  popq %%rsi\n");               // load the stack top to rsi
  char signchar = type->is_unsigned ? 'z'  : 's';
  switch (type->size) {
  case 1:
    fprintf(file, "  mov $0, %%rax\n");
    fprintf(file, "  mov (%%rsi), %%al\n");  // load the actual value of rsi to rax
    fprintf(file, "  mov%cx %%al, %%rax\n", signchar);
    break;
  case 4:
    fprintf(file, "  mov (%%rsi), %%eax\n");    // load the actual value of rsi to rax
    fprintf(file, "  mov%cx %%eax, %%rax\n", signchar);
    break;
  case 8:
    fprintf(file, "  mov (%%rsi), %%rax\n");    // load the actual value of rsi to rax
    break;
  default:
    error("failed to load a variable becase of unknown type size");
  }
  fprintf(file, "  pushq %%rax\n");              // store rax to the stack top
}

// store value to the variable.
static void store(Type *type) {
  // stack
  // before : (top) value, (variable's address), ...
  // after  : (top) value, ...
  fprintf(file, "  popq %%rdi\n");               // load the stack top to rdi
  fprintf(file, "  popq %%rax\n");               // load the stack top to rax

  int size = type->kind == TYPE_ARRAY ? type_size_pointer : type->size;
  switch (size) {
  case 1:
    fprintf(file, "  movb %%dil, (%%rax)\n");    // copy rdi's value to the address pointed by rax
    break;
  case 4:
    fprintf(file, "  movl %%edi, (%%rax)\n");    // copy rdi's value to the address pointed by rax
    break;
  case 8:
    fprintf(file, "  movq %%rdi, (%%rax)\n");    // copy rdi's value to the address pointed by rax
    break;
  default:
    error("failed to store a variable becase of unknown type size");
  }
  fprintf(file, "  pushq %%rdi\n");              // store rdi to the stack top
}

// load the address of node's variable to the stack top
static void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_EXPR_VAR:
    if (node->var->is_global) {
      // load the address of the actual value of (rbp - offset)
      fprintf(file, "  mov $%.*s, %%rax\n", node->var->namelen, node->var->name);
      fprintf(file, "  pushq %%rax\n");          // pushq rbp - offset
      return;
    }
    // load the address of the actual value of (rbp - offset)
    fprintf(file, "  lea -%d(%%rbp), %%rax\n", node->var->offset);
    fprintf(file, "  pushq %%rax\n");            // pushq rbp - offset
    return;
  case ND_EXPR_DEREF:
    gen(node->left);
    return;
  case ND_EXPR_MEMBER:
    gen_addr(node->left);
    fprintf(file, "  pop %%rax\n");
    fprintf(file, "  add $%d, %%rax\n", node->member->offset);
    fprintf(file, "  push %%rax\n");
    return;
  default:
    error_at(node->token, "Left side is expected a variable or *variable.");
  }
}

static void gen_if(Node *node) {
  int labct = label_count ++;

  gen(node->cond);                        // calculate condition
  fprintf(file, "  popq %%rax\n");               // load result to the stach top
  fprintf(file, "  cmp $0, %%rax\n");            // evaluate result

  if (node->els) {
    fprintf(file, "  je  .L.else.%d\n", labct);  // jump if result is false
    gen(node->then);
    fprintf(file, "  jmp .L.end.%d\n", labct);   // end then stmt
    fprintf(file, ".L.else.%d:\n", labct);
    gen(node->els);
  } else {
    fprintf(file, "  je  .L.end.%d\n", labct);   // jump if result is false
    gen(node->then);
  }

  fprintf(file, ".L.end.%d:\n", labct);
}

static void gen_switch(Node *node) {
  int labct = label_count ++;
  push(&switch_stack, labct);
  push(&break_stack, labct);

  gen(node->cond);
  int case_num = 1;
  for (Node *case_expr = node->cases; case_expr; case_expr = case_expr->next) {
    gen(case_expr);
    fprintf(file, "  popq %%rax\n");               // load result of case_expr
    fprintf(file, "  cmp  (%%rsp), %%rax\n");
    fprintf(file, "  je  .L.case.%d.%d\n", labct, case_num++);
  }
  if (node->have_default)
    fprintf(file, "  jmp .L.default.%d\n", labct); // end then stmt
  else
    fprintf(file, "  jmp .L.end.%d\n", labct);     // end then stmt
  
  for (Node *stmt_node = node->body; stmt_node; stmt_node = stmt_node->next)
    gen(stmt_node);

  fprintf(file, ".L.end.%d:\n", labct);
  fprintf(file, "  add $8, %%rsp\n");              // drop cond

  pop(&switch_stack);
  pop(&break_stack);
}

static void gen_while(Node *node) {
  int labct = label_count ++;
  push(&continue_stack, labct);
  push(&break_stack, labct);


  fprintf(file, ".L.begin.%d:\n", labct);
  gen(node->cond);                      // calculate condition
  fprintf(file, "  popq %%rax\n");             // load result to the stach top
  fprintf(file, "  cmp $0, %%rax\n");          // evaluate result

  fprintf(file, "  je  .L.end.%d\n", labct);   // jump if result is false

  gen(node->then);
  fprintf(file, "  jmp .L.begin.%d\n", labct); // jump cond

  fprintf(file, ".L.end.%d:\n", labct);

  pop(&continue_stack);
  pop(&break_stack);
}

static void gen_for(Node *node) {
  int labct = label_count ++;
  push(&continue_stack, labct);
  push(&break_stack, labct);

  for (Node *n = node->init; n; n=n->next)
    gen(n);

  fprintf(file, "  jmp .L.first.%d\n", labct); // jump initialize
  fprintf(file, ".L.begin.%d:\n", labct);
  gen(node->increment);
  fprintf(file, ".L.first.%d:\n", labct);
  gen(node->cond);                      // calculate condition
  fprintf(file, "  popq %%rax\n");             // load result to the stach top
  fprintf(file, "  cmp $0, %%rax\n");          // evaluate result

  fprintf(file, "  je  .L.end.%d\n", labct);   // jump if result is false

  gen(node->then);
  fprintf(file, "  jmp .L.begin.%d\n", labct); // jump cond

  fprintf(file, ".L.end.%d:\n", labct);


  pop(&continue_stack);
  pop(&break_stack);
}

static void gen_func_call(Node *node) {
  int argc = 0;
  for (Node *arg_node = node->func_args; arg_node; arg_node = arg_node->next) {
    gen(arg_node);
    argc++;
  }
  if (argc > 6)
    error_at(node->token, "unsupport calling function with over 6 argoments");

  while (argc>0)
    fprintf(file, "  popq  %%%s\n", arg_regs8[--argc]);

  // align RSP to a 16 byte boundary
  int labct = label_count ++;
  fprintf(file, "  movq %%rsp, %%rax\n");
  fprintf(file, "  and $15, %%rax\n");
  fprintf(file, "  jne .L.needAlign.%d\n", labct);

  fprintf(file, "  mov $0, %%al\n"); // set zero to al for calling variadic arguments
  fprintf(file, "  call %.*s\n", node->func_namelen, node->func_name);
  fprintf(file, "  jmp .L.end.%d\n", labct);

  fprintf(file, ".L.needAlign.%d:\n", labct);
  fprintf(file, "  sub $8, %%rsp\n");
  fprintf(file, "  mov $0, %%al\n"); // set zero to al for calling variadic arguments
  fprintf(file, "  call %.*s\n", node->func_namelen, node->func_name);
  fprintf(file, "  add $8, %%rsp\n");

  fprintf(file, ".L.end.%d:\n", labct);
  fprintf(file, "  pushq %%rax\n");    // push returned value
}

static void gen_ternary(Node *node) {
  int labct = label_count++;
  gen(node->cond);
  fprintf(file, "  popq %%rax\n");
  fprintf(file, "  cmp $0, %%rax\n");
  fprintf(file, "  je  .L.else.%d\n", labct);
  gen(node->left);
  fprintf(file, "  jmp  .L.end.%d\n", labct);
  fprintf(file, ".L.else.%d:\n", labct);
  gen(node->right);
  fprintf(file, ".L.end.%d:\n", labct);
}

static void gen_log_or(Node *node) {
  int labct = label_count++;
  gen(node->left);
  fprintf(file, "  popq %%rax\n");
  fprintf(file, "  cmp $0, %%rax\n");
  fprintf(file, "  jne  .L.true.%d\n", labct);
  gen(node->right);
  fprintf(file, "  popq %%rax\n");
  fprintf(file, "  cmp $0, %%rax\n");
  fprintf(file, "  jne  .L.true.%d\n", labct);
  fprintf(file, "  push $0\n");
  fprintf(file, "  jmp  .L.end.%d\n", labct);
  fprintf(file, ".L.true.%d:\n", labct);
  fprintf(file, "  push $1\n");
  fprintf(file, ".L.end.%d:\n", labct);
}
static void gen_log_and(Node *node) {
  int labct = label_count++;
  gen(node->left);
  fprintf(file, "  popq %%rax\n");
  fprintf(file, "  cmp $0, %%rax\n");
  fprintf(file, "  je  .L.false.%d\n", labct);
  gen(node->right);
  fprintf(file, "  popq %%rax\n");
  fprintf(file, "  cmp $0, %%rax\n");
  fprintf(file, "  je  .L.false.%d\n", labct);
  fprintf(file, "  push $1\n");
  fprintf(file, "  jmp  .L.end.%d\n", labct);
  fprintf(file, ".L.false.%d:\n", labct);
  fprintf(file, "  push $0\n");
  fprintf(file, ".L.end.%d:\n", labct);
}

static void gen_stmt_var_init(Node *node) {
  if (node->var->is_global)
    error_at(node->token, "unsupported initialize global variable");
  for (int i =0; i < node->var_inits_size; i++)
    fprintf(file, "  movb $%d, -%d(%%rbp)\n", (node->var_inits[i])&0b00000000000000000000000011111111, node->var->offset - i); 
}

static void cast(Type *type) {
  fprintf(file, "  popq %%rax\n");
  if (type->kind == TYPE_BOOL) {
    fprintf(file, "  cmp $0, %%rax\n");
    fprintf(file, "  setne %%al\n");
  }
  char signchar = type->is_unsigned ? 'z'  : 's';
  switch (type->size) {
  case 1:
    fprintf(file, "  mov%cx %%al, %%rax\n", signchar);
    break;
  case 4:
    fprintf(file, "  mov%cx %%eax, %%rax\n", signchar);
    break;
  case 8:
    break;
  default:
    error("faild to cast type to unknown type size");
  }
  fprintf(file, "  pushq  %%rax\n");
}

static void gen(Node *node) {
  if (!node)
    return;

  // for debug
  if (is_printing_asm_debug) {
    fprintf(file, "  # >>> ");
    print_node(file, node);
  }

  switch (node->kind) {
  // statements
  case ND_STMT_RETURN:
    gen(node->left);
    fprintf(file, "  popq %%rax\n");
    fprintf(file, "  jmp .L.return.%.*s\n", funcnamelen, funcname);
    break;
  case ND_STMT_IF:
    gen_if(node);
    break;
  case ND_STMT_WHILE:
    gen_while(node);
    break;
  case ND_STMT_FOR:
    gen_for(node);
    break;
  case ND_STMT_BLOCK:
    for(Node *n = node->body; n; n = n->next)
      gen(n);
    break;
  case ND_STMT_WITH_EXPR:
    gen(node->left);
    fprintf(file, "  add $8, %%rsp\n"); // stmt is not leave any values in the stack
    break;
  case ND_STMT_VAR_INIT:
    gen_stmt_var_init(node);
    break;
  case ND_STMT_CONTINUE:
    if (!continue_stack)
      error_at(node->token, "cannot jump the begin of loop (continue statement)");
    fprintf(file, "  jmp .L.begin.%d\n", continue_stack->labct);
    break;
  case ND_STMT_BREAK:
    if (!break_stack)
      error_at(node->token, "cannot jump the end of loop (break statement)");
    fprintf(file, "  jmp .L.end.%d\n", break_stack->labct);
    break;
  case ND_STMT_SWITCH:
    gen_switch(node);
    break;
  case ND_LABEL_CASE:
    if (!switch_stack)
      error_at(node->token, "case label must be in switch statement");
    fprintf(file, ".L.case.%d.%d:\n", switch_stack->labct , node->case_num);
    break;
  case ND_LABEL_DEFAULT:
    if (!switch_stack)
      error_at(node->token, "default label must be in switch statement");
    fprintf(file, ".L.default.%d:\n", switch_stack->labct);
    break;

  // expression
  case ND_EXPR_NUM:
    if (node->type->kind == TYPE_LONG)
      fprintf(file, "  pushq $%ld\n", node->value); // pushq constant
    else
      fprintf(file, "  pushq $%d\n", (int) node->value); // pushq constant
    break;
  case ND_EXPR_STRING:
    fprintf(file, "  pushq $.LC%d\n", node->string->id);
    break;
  case ND_EXPR_VAR:
  case ND_EXPR_MEMBER:
    gen_addr(node);
    load(node->type);
    break;
  case ND_EXPR_ASSIGN:
    gen_addr(node->left);
    gen(node->right);
    store(node->type);
    break;
  case ND_EXPR_ASSIGN_ADD:
  case ND_EXPR_ASSIGN_PTR_ADD:
  case ND_EXPR_ASSIGN_SUB:
  case ND_EXPR_ASSIGN_PTR_SUB:
  case ND_EXPR_ASSIGN_MUL:
  case ND_EXPR_ASSIGN_DIV:
  case ND_EXPR_ASSIGN_MOD:
  case ND_EXPR_ASSIGN_SHIFT_LEFT:
  case ND_EXPR_ASSIGN_SHIFT_RIGHT:
    gen_addr(node->left);
    fprintf(file, "  pushq (%%rsp)\n");
    load(node->left->type);
    gen(node->right);
    gen_binary_operator(node);
    store(node->type);
    break;
  case ND_EXPR_FUNC_CALL:
    gen_func_call(node);
    break;
  case ND_EXPR_ADDR:
    gen_addr(node->left);
    break;
  case ND_EXPR_DEREF:
    gen(node->left);
    load(node->type);
    break;
  case ND_EXPR_WITH_STMTS:
    for (Node *stmt_node = node->body; stmt_node; stmt_node = stmt_node->next)
      gen(stmt_node);
    if (node->type)  // type is not NULL if tail stmt is expr_stmt
      fprintf(file, "  sub $8, %%rsp\n");
    else
      // return void
      // TODO: Add void type
      fprintf(file, "  push $0\n");
    break;
  case ND_EXPR_TERNARY:
    gen_ternary(node);
    break;
  case ND_EXPR_COMMA:
    gen(node->left);
    gen(node->right);
    break;
  case ND_EXPR_LOG_OR:
    gen_log_or(node);
    break;
  case ND_EXPR_LOG_AND:
    gen_log_and(node);
    break;
  case ND_EXPR_NOT:
    gen(node->left);
    fprintf(file, "  pop %%rax\n");
    fprintf(file, "  cmp $0, %%rax\n");
    fprintf(file, "  sete %%al\n");
    fprintf(file, "  movzbl %%al, %%rax\n");
    fprintf(file, "  pushq %%rax\n");
    break;
  case ND_EXPR_BIT_NOT:
    gen(node->left);
    fprintf(file, "  pop %%rax\n");
    fprintf(file, "  not %%rax\n");
    fprintf(file, "  push %%rax\n");
    break;
  case ND_EXPR_PRE_INC:
    gen_addr(node->left);
    fprintf(file, "  push (%%rsp)\n");
    load(node->type);
    fprintf(file, "  add $%d, (%%rsp)\n", node->type->base ? node->type->base->size : 1);
    store(node->type);
    break;
  case ND_EXPR_PRE_DEC:
    gen_addr(node->left);
    fprintf(file, "  push (%%rsp)\n");
    load(node->type);
    fprintf(file, "  sub $%d, (%%rsp)\n", node->type->base ? node->type->base->size : 1);
    store(node->type);
    break;
  case ND_EXPR_POST_INC:
    gen_addr(node->left);
    fprintf(file, "  push (%%rsp)\n");
    load(node->type);
    fprintf(file, "  add $%d, (%%rsp)\n", node->type->base ? node->type->base->size : 1);
    store(node->type);
    fprintf(file, "  sub $%d, (%%rsp)\n", node->type->base ? node->type->base->size : 1);
    break;
  case ND_EXPR_POST_DEC:
    gen_addr(node->left);
    fprintf(file, "  push (%%rsp)\n");
    load(node->type);
    fprintf(file, "  sub $%d, (%%rsp)\n", node->type->base ? node->type->base->size : 1);
    store(node->type);
    fprintf(file, "  add $%d, (%%rsp)\n", node->type->base ? node->type->base->size : 1);
    break;
  case ND_EXPR_TYPE_CAST:
    gen(node->left);
    cast(node->type);
    break;
  default:
    // expect binary operator node
    gen(node->left);
    gen(node->right);
    gen_binary_operator(node);
  }

  // for debug
  if (is_printing_asm_debug) {
    fprintf(file, "  # <<< ");
    print_node(file, node);
  }
}

static void gen_binary_operator(Node *node) {
  fprintf(file, "  popq %%rdi\n");           // load the stack top to rdi to calculate
  fprintf(file, "  popq %%rax\n");           // load the stack top to rax to calculate

  switch (node->kind) {
  case ND_EXPR_PTR_ADD:
  case ND_EXPR_ASSIGN_PTR_ADD:
    fprintf(file, "  imul $%d, %%rdi\n", node->type->base->size);
    // continue
  case ND_EXPR_ADD:
  case ND_EXPR_ASSIGN_ADD:
    fprintf(file, "  add %%rdi, %%rax\n");   // rax += rdi
    break;
  case ND_EXPR_PTR_SUB:
  case ND_EXPR_ASSIGN_PTR_SUB:
    fprintf(file, "  imul $%d, %%rdi\n", node->type->base->size);
    // continue
  case ND_EXPR_ASSIGN_SUB:
  case ND_EXPR_SUB:
    fprintf(file, "  sub %%rdi, %%rax\n");   // rax -= rdi
    break;
  case ND_EXPR_ASSIGN_MUL:
  case ND_EXPR_MUL:
    fprintf(file, "  imul %%rdi, %%rax\n");  // rax *= rdi
    break;
  case ND_EXPR_PTR_DIFF:
    fprintf(file, "  sub %%rdi, %%rax\n");                             // rax -= rdi
    fprintf(file, "  mov $%d, %%rdi\n", node->left->type->base->size); // rax = [rdx rax] / size
    // continue
  case ND_EXPR_ASSIGN_DIV:
  case ND_EXPR_DIV:
    fprintf(file, "  cqo\n");                // [rdx rax](128bit) = rax (64bit)
    fprintf(file, "  idiv %%rdi\n");         // rax = [rdx rax] / rdi
    break;
  case ND_EXPR_ASSIGN_MOD:
  case ND_EXPR_MOD:
    fprintf(file, "  cqo\n");                // [rdx rax](128bit) = rax (64bit)
    fprintf(file, "  idiv %%rdi\n");         // rdx = [rdx rax] % rdi
    fprintf(file, "  movq %%rdx, %%rax\n");
    break;
  case ND_EXPR_EQ:
    fprintf(file, "  cmp %%rdi, %%rax\n");   // set flag register with comparing rax and rdi
    fprintf(file, "  sete %%al\n");          // al = ( flag register means rax == rdi ) ? 1 : 0
    fprintf(file, "  movzb %%al, %%rax\n");  // rax(64bit) = al(8bit)
    break;
  case ND_EXPR_NE:
    fprintf(file, "  cmp %%rdi, %%rax\n");
    fprintf(file, "  setne %%al\n");         // al = ( flag register means rax != rdi ) ? 1 : 0
    fprintf(file, "  movzb %%al, %%rax\n");
    break;
  case ND_EXPR_LESS_THAN:
    fprintf(file, "  cmp %%rdi, %%rax\n");
    fprintf(file, "  setl %%al\n");          // al = ( flag register means rax < rdi ) ? 1 : 0
    fprintf(file, "  movzb %%al, %%rax\n");
    break;
  case ND_EXPR_LESS_EQ:
    fprintf(file, "  cmp %%rdi, %%rax\n");
    fprintf(file, "  setle %%al\n");         // al = ( flag register means rax <= rdi ) ? 1 : 0
    fprintf(file, "  movzb %%al, %%rax\n");
    break;
  case ND_EXPR_BIT_OR:
    fprintf(file, "  or %%rdi, %%rax\n");
    break;
  case ND_EXPR_BIT_XOR:
    fprintf(file, "  xor %%rdi, %%rax\n");
    break;
  case ND_EXPR_BIT_AND:
    fprintf(file, "  and %%rdi, %%rax\n");
    break;
  case ND_EXPR_SHIFT_LEFT:
  case ND_EXPR_ASSIGN_SHIFT_LEFT:
    fprintf(file, "  mov %%rdi, %%rcx\n");
    fprintf(file, "  shl %%cl, %%rax\n");
    break;
  case ND_EXPR_SHIFT_RIGHT:
  case ND_EXPR_ASSIGN_SHIFT_RIGHT:
    fprintf(file, "  mov %%rdi, %%rcx\n");
    fprintf(file, "  sar %%cl, %%rax\n");
    break;
  default:
    error_at(node->token, "unknown binary operator");
  }
  fprintf(file, "  pushq %%rax\n");          // store result to stack top
}

static void prologue(Function *func) {
  int offset = func->var_byte;
  // align RSP to a 8 byte boundary
  while (offset%8)
    offset++;

  fprintf(file, "  pushq %%rbp\n");              // record caller's rbp
  fprintf(file, "  movq %%rsp, %%rbp\n");        // set current stack top to rbp
  fprintf(file, "  sub $%d, %%rsp\n", offset);   // allocate memory for local variables

  int i = func->argc;
  for (Var *arg = func->args; arg; arg = arg->next) {
    switch (arg->type->size) {
    case 1:
      fprintf(file, "  movb %%%s, -%d(%%rbp)\n",  arg_regs1[--i], arg->offset);
      break;
    case 4:
      fprintf(file, "  movl %%%s, -%d(%%rbp)\n",  arg_regs4[--i], arg->offset);
      break;
    case 8:
      fprintf(file, "  movq %%%s, -%d(%%rbp)\n",  arg_regs8[--i], arg->offset);
      break;
    default:
      error_at(func->node->token, "failed to load a argument becase of unknown type size");
    }
  }
}

static void epilogue() {
  fprintf(file, "  movq $0, %%rax\n");
  fprintf(file, ".L.return.%.*s:\n", funcnamelen, funcname);
  fprintf(file, "  movq %%rbp, %%rsp\n");    // ignore the remanig data in the stack
  fprintf(file, "  popq %%rbp\n");           // set caller's rbp to rsp
  fprintf(file, "  ret\n");
}

static void gen_function(Function *func) {
  funcname = func->name;
  funcnamelen = func->namelen;
  fprintf(file, "%.*s:\n", func->namelen, func->name);

  prologue(func);

  gen(func->node);

  epilogue();
}

static void gen_gvars() {
  for (Var *var = gvars; var; var = var->next) {
    if (var->type->is_extern)
      continue;

    fprintf(file, "%.*s:\n", var->namelen, var->name);

    if (!var->init_values) {
      fprintf(file, "  .zero %d\n", var->type->size);
      continue;
    }

    for (int i =0; i< var->init_size; i++)
      fprintf(file, "  .byte 0x%x\n", (var->init_values[i])&0b00000000000000000000000011111111);
  }
}

void code_generate(char *filename) {
  file = fopen(filename,"w");
  if (!file)
    error("fail to open input file");
 
  // data sectioon
  fprintf(file, ".data\n");
  for (String *str = strings; str; str = str->next) {
    fprintf(file, ".LC%d:\n", str->id);
    for (int i=0; i < str->length; i++)
      fprintf(file, "  .byte 0x%x\n", (str->p)[i]);
  }
  gen_gvars();

  // text section
  fprintf(file, ".text\n");
  for (Var *var = gvars; var; var = var->next) {
    if (var->type->is_static)
      continue;
    fprintf(file, ".globl %.*s\n", var->namelen, var->name);
  }
  for (Function *func = functions; func; func = func->next) {
    if (func->definition || func->type->is_static)
      continue;
    fprintf(file, ".globl %.*s\n", func->namelen, func->name);
  }

  for (Function *func = functions; func; func = func->next) {
    if (func->definition)
      continue;
    gen_function(func);
  }

  fclose(file);
}
