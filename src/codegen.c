#include "willani.h"

static void load(Type *type);
static void store(Type *type);
static void gen_if(Node *node);
static void gen_while(Node *node);
static void gen_addr(Node *node);
static void gen(Node *node);
static void gen_binary_operator(Node *node);
static void prologue(Function *func);
static void epilogue(void);

char arg_regs1[][4] = { "dil", "sil", "dl", "cl", "r8b", "r9b" };
char arg_regs4[][4] = { "edi", "esi", "edx", "ecx", "r8d", "r9d" };
char arg_regs8[][4] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

//======================================
// labels
// identify jump label of break, continue and case
typedef struct Stack Stack;
struct Stack {
  int labct;
  Stack *parent;
};
void push(Stack **stack,int labct) {
  Stack *s = calloc(1, sizeof(Stack));
  s->labct = labct;
  s->parent = *stack;
  *stack = s;
}
int pop(Stack **stack) {
  int labct = (*stack)->labct;
  *stack = (*stack)->parent;
  return labct;
}
Stack *continue_stack;
Stack *break_stack;
Stack *switch_stack;

// identify jump label of return
char *funcname = NULL;
int funcnamelen = 0;

int label_count = 1;




//======================================
// load and store
// change the stack top from addr to value
static void load(Type *type) {
  if (type->kind == TYPE_ARRAY)
    return;

  printf("  popq %%rax\n");          // load the stack top to rax
  switch (type->size) {
  case 1:
    printf("  movzbl (%%rax), %%rax\n");   // load the actual value of rax to rax
    break;
  case 4:
    printf("  movl (%%rax), %%eax\n");   // load the actual value of rax to rax
    break;
  case 8:
    printf("  movq (%%rax), %%rax\n");   // load the actual value of rax to rax
    break;
  default:
    error("failed to load a variable becase of unknown type size");
  }
  printf("  pushq %%rax\n");         // store rax to the stack top
}

// store value to the variable.
static void store(Type *type) {
  // stack
  // before : (top) value, (variable's address), ...
  // after  : (top) value, ...
  printf("  popq %%rdi\n");          // load the stack top to rdi
  printf("  popq %%rax\n");          // load the stack top to rax

  int size = type->kind == TYPE_ARRAY ? type_size_pointer : type->size;
  switch (size) {
  case 1:
    printf("  movb %%dil, (%%rax)\n");   // copy rdi's value to the address pointed by rax
    break;
  case 4:
    printf("  movl %%edi, (%%rax)\n");   // copy rdi's value to the address pointed by rax
    break;
  case 8:
    printf("  movq %%rdi, (%%rax)\n");   // copy rdi's value to the address pointed by rax
    break;
  default:
    error("failed to store a variable becase of unknown type size");
  }
  printf("  pushq %%rdi\n");         // store rdi to the stack top
}

// load the address of node's variable to the stack top
static void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_EXPR_VAR:
    if (node->var->is_global) {
      // load the address of the actual value of (rbp - offset)
      printf("  mov $%.*s, %%rax\n", node->var->namelen, node->var->name);
      printf("  pushq %%rax\n");         // pushq rbp - offset
      return;
    }
    // load the address of the actual value of (rbp - offset)
    printf("  lea -%d(%%rbp), %%rax\n", node->var->offset);
    printf("  pushq %%rax\n");         // pushq rbp - offset
    return;
  case ND_EXPR_DEREF:
    gen(node->left);
    return;
  case ND_EXPR_MEMBER:
    gen_addr(node->left);
    printf("  pop %%rax\n");
    printf("  add $%d, %%rax\n", node->member->offset);
    printf("  push %%rax\n");
    return;
  default:
    error_at(node->token->location, "Left side is expected a variable or *variable.");
  }
}

static void gen_if(Node *node) {
  int labct = label_count ++;

  gen(node->cond);               // calculate condition
  printf("  popq %%rax\n");         // load result to the stach top
  printf("  cmp $0, %%rax\n");      // evaluate result

  if (node->els) {
    printf("  je  .L.else.%d\n", labct); // jump if result is false
    gen(node->then);
    printf("  jmp .L.end.%d\n", labct);  // end then stmt
    printf(".L.else.%d:\n", labct);
    gen(node->els);
  } else {
    printf("  je  .L.end.%d\n", labct);  // jump if result is false
    gen(node->then);
  }

  printf(".L.end.%d:\n", labct);
}

static void gen_switch(Node *node) {
  int labct = label_count ++;
  push(&switch_stack, labct);
  push(&break_stack, labct);

  gen(node->cond);
  int case_num = 1;
  for (Node *case_expr = node->cases; case_expr; case_expr = case_expr->next) {
    gen(case_expr);
    printf("  popq %%rax\n"); // load result of case_expr
    printf("  cmp  (%%rsp), %%rax\n");
    printf("  je  .L.case.%d.%d\n", labct, case_num++);
  }
  // TODO: Support default : case
  if (node->have_default)
    printf("  jmp .L.default.%d\n", labct);  // end then stmt
  else
    printf("  jmp .L.end.%d\n", labct);  // end then stmt
  
  for (Node *stmt_node = node->body; stmt_node; stmt_node = stmt_node->next)
    gen(stmt_node);

  printf(".L.end.%d:\n", labct);
  printf("  add $8, %%rsp\n"); // drop cond

  pop(&switch_stack);
  pop(&break_stack);
}

static void gen_while(Node *node) {
  int labct = label_count ++;
  push(&continue_stack, labct);
  push(&break_stack, labct);


  printf(".L.begin.%d:\n", labct);
  gen(node->cond);               // calculate condition
  printf("  popq %%rax\n");         // load result to the stach top
  printf("  cmp $0, %%rax\n");       // evaluate result

  printf("  je  .L.end.%d\n", labct); // jump if result is false

  gen(node->then);
  printf("  jmp .L.begin.%d\n", labct); // jump cond

  printf(".L.end.%d:\n", labct);

  pop(&continue_stack);
  pop(&break_stack);
}

static void gen_for(Node *node) {
  int labct = label_count ++;
  push(&continue_stack, labct);
  push(&break_stack, labct);

  gen(node->init);

  printf("  jmp .L.first.%d\n", labct); // jump initialize
  printf(".L.begin.%d:\n", labct);
  gen(node->increment);
  printf(".L.first.%d:\n", labct);
  gen(node->cond);               // calculate condition
  printf("  popq %%rax\n");         // load result to the stach top
  printf("  cmp $0, %%rax\n");       // evaluate result

  printf("  je  .L.end.%d\n", labct); // jump if result is false

  gen(node->then);
  printf("  jmp .L.begin.%d\n", labct); // jump cond

  printf(".L.end.%d:\n", labct);


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
    error_at(node->func_name, "unsupport calling function with over 6 argoments");

  while (argc>0)
    printf("  popq  %%%s\n", arg_regs8[--argc]);

  // align RSP to a 16 byte boundary
  int labct = label_count ++;
  printf("  movq %%rsp, %%rax\n");
  printf("  and $15, %%rax\n");
  printf("  jne .L.needAlign.%d\n", labct);

  printf("  mov $0, %%al\n"); // set zero to al for calling variadic arguments
  printf("  call %.*s\n", node->func_namelen, node->func_name);
  printf("  jmp .L.end.%d\n", labct);

  printf(".L.needAlign.%d:\n", labct);
  printf("  sub $8, %%rsp\n");
  printf("  mov $0, %%al\n"); // set zero to al for calling variadic arguments
  printf("  call %.*s\n", node->func_namelen, node->func_name);
  printf("  add $8, %%rsp\n");

  printf(".L.end.%d:\n", labct);
  printf("  pushq %%rax\n");    // push returned value
}

static void gen(Node *node) {
  if (!node)
    return;

  // for debug
  printf ("  # >>> "); print_node(stdout, node);

  switch (node->kind) {
  // statements
  case ND_STMT_RETURN:
    gen(node->left);
    printf("  popq %%rax\n");
    printf("  jmp .L.return.%.*s\n", funcnamelen, funcname);
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
    printf("  add $8, %%rsp\n"); // stmt is not leave any values in the stack
    break;
  case ND_STMT_CONTINUE:
    if (!continue_stack)
      error_at(node->token->location, "cannot jump the begin of loop (continue statement)");
    printf("  jmp .L.begin.%d\n", continue_stack->labct);
    break;
  case ND_STMT_BREAK:
    if (!break_stack)
      error_at(node->token->location, "cannot jump the end of loop (break statement)");
    printf("  jmp .L.end.%d\n", break_stack->labct);
    break;
  case ND_STMT_SWITCH:
    gen_switch(node);
    break;
  case ND_LABEL_CASE:
    if (!switch_stack)
      error_at(node->token->location, "case label must be in switch statement");
    printf(".L.case.%d.%d:\n", switch_stack->labct , node->case_num);
    break;
  case ND_LABEL_DEFAULT:
    if (!switch_stack)
      error_at(node->token->location, "default label must be in switch statement");
    printf(".L.default.%d:\n", switch_stack->labct);
    break;

  // expression
  case ND_EXPR_NUM:
    printf("  pushq $%ld\n", node->value); // pushq constant
    break;
  case ND_EXPR_STRING:
    printf("  pushq $.LC%d\n", node->string->id);
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
      printf("  sub $8, %%rsp\n");
    else
      // return void
      // TODO: Add void type
      printf("  push $0\n");
    break;
  case ND_EXPR_COMMA:
    gen(node->left);
    gen(node->right);
    break;
  case ND_EXPR_NOT:
    gen(node->left);
    printf("  pop %%rax\n");
    printf("  cmp $0, %%rax\n");
    printf("  sete %%al\n");
    printf("  movzbl %%al, %%rax\n");
    printf("  pushq %%rax\n");
    break;
  case ND_EXPR_BIT_NOT:
    gen(node->left);
    printf("  pop %%rax\n");
    printf("  not %%rax\n");
    printf("  push %%rax\n");
    break;
  default:
    // expect binary operator node
    gen_binary_operator(node);
  }

  // for debug
  printf ("  # <<< "); print_node(stdout, node);
}

static void gen_binary_operator(Node *node) {
  gen(node->left);
  gen(node->right);

  printf("  popq %%rdi\n");           // load the stack top to rdi to calculate
  printf("  popq %%rax\n");           // load the stack top to rax to calculate

  switch (node->kind) {
  case ND_EXPR_PTR_ADD:
    printf("  imul $%d, %%rdi\n", node->type->base->size);
  case ND_EXPR_ADD:
    printf("  add %%rdi, %%rax\n");   // rax += rdi
    break;
  case ND_EXPR_PTR_SUB:
    printf("  imul $%d, %%rdi\n", node->type->base->size);
  case ND_EXPR_SUB:
    printf("  sub %%rdi, %%rax\n");   // rax -= rdi
    break;
  case ND_EXPR_MUL:
    printf("  imul %%rdi, %%rax\n");  // rax *= rdi
    break;
  case ND_EXPR_PTR_DIFF:
    printf("  sub %%rdi, %%rax\n");                 // rax -= rdi
    printf("  cqo\n");                              // [rdx rax](128bit) = rax (64bit)
    printf("  mov $%d, %%rdi\n", node->left->type->base->size); // rax = [rdx rax] / size
  case ND_EXPR_DIV:
    printf("  cqo\n");                // [rdx rax](128bit) = rax (64bit)
    printf("  idiv %%rdi\n");         // rax = [rdx rax] / rdi
    break;
  case ND_EXPR_MOD:
    printf("  cqo\n");                // [rdx rax](128bit) = rax (64bit)
    printf("  idiv %%rdi\n");         // rdx = [rdx rax] % rdi
    printf("  movq %%rdx, %%rax\n");
    break;
  case ND_EXPR_EQ:
    printf("  cmp %%rdi, %%rax\n");   // set flag register with comparing rax and rdi
    printf("  sete %%al\n");          // al = ( flag register means rax == rdi ) ? 1 : 0
    printf("  movzb %%al, %%rax\n");  // rax(64bit) = al(8bit)
    break;
  case ND_EXPR_NE:
    printf("  cmp %%rdi, %%rax\n");
    printf("  setne %%al\n");         // al = ( flag register means rax != rdi ) ? 1 : 0
    printf("  movzb %%al, %%rax\n");
    break;
  case ND_EXPR_LESS_THAN:
    printf("  cmp %%rdi, %%rax\n");
    printf("  setl %%al\n");          // al = ( flag register means rax < rdi ) ? 1 : 0
    printf("  movzb %%al, %%rax\n");
    break;
  case ND_EXPR_LESS_EQ:
    printf("  cmp %%rdi, %%rax\n");
    printf("  setle %%al\n");         // al = ( flag register means rax <= rdi ) ? 1 : 0
    printf("  movzb %%al, %%rax\n");
    break;
  defalt:
    error_at(node->token->location, "unknown binary operator");
  }
  printf("  pushq %%rax\n");         // store result to stack top
}

static void prologue(Function *func) {
  int offset = func->var_byte;
  // align RSP to a 8 byte boundary
  while (offset%8)
    offset++;

  printf("  pushq %%rbp\n");         // record caller's rbp
  printf("  movq %%rsp, %%rbp\n");   // set current stack top to rbp
  printf("  sub $%d, %%rsp\n", offset);  // allocate memory for local variables

  int i = func->argc;
  for (Var *arg = func->args; arg; arg = arg->next) {
    switch (arg->type->size) {
    case 1:
      printf("  movb %%%s, -%d(%%rbp)\n",  arg_regs1[--i], arg->offset);
      break;
    case 4:
      printf("  movl %%%s, -%d(%%rbp)\n",  arg_regs4[--i], arg->offset);
      break;
    case 8:
      printf("  movq %%%s, -%d(%%rbp)\n",  arg_regs8[--i], arg->offset);
      break;
    default:
      error_at(arg->name, "failed to load a argument becase of unknown type size");
    }
  }
}

static void epilogue(void) {
  printf(".L.return.%.*s:\n", funcnamelen, funcname);
  printf("  movq %%rbp, %%rsp\n");   // ignore the remanig data in the stack
  printf("  popq %%rbp\n");        // set caller's rbp to rsp
  printf("  ret\n");
}

static void gen_function(Function *func) {
  funcname = func->name;
  funcnamelen = func->namelen;
  printf("%.*s:\n", func->namelen, func->name);

  prologue(func);

  gen(func->node);

  epilogue();
}

void code_generate() {
  // data sectioon
  printf(".data\n");
  for (String *str = strings; str; str = str->next) {
    printf(".LC%d:\n", str->id);
    for (int i=0; i < str->length; i++)
      printf("  .byte 0x%x\n", (str->p)[i]);
  }
  for (Var *var = gvars; var; var = var->next) {
    if (var->is_typedef)
      continue;
    printf("%.*s:\n", var->namelen, var->name);
    printf("  .zero %d\n", var->type->size);
  }

  // text section
  printf(".text\n");
  for (Function *func = functions; func; func = func->next) {
    if (func->definition || func->type->is_static)
      continue;
    printf(".globl %.*s\n", func->namelen, func->name);
  }

  for (Function *func = functions; func; func = func->next) {
    if (func->definition)
      continue;
    gen_function(func);
  }
}
