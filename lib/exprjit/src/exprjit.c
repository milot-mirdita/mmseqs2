//
//  exprjit.c
//  Expr JIT
//
//  Created by Indi Kernick on 5/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "exprjit.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <sys/mman.h>
#endif

#if defined(_M_X64) || defined(__x86_64__)
#define EJ_X86_64 1
#endif
#if defined(_M_ARM64) || defined(__aarch64__) || defined(__arm64__)
#define EJ_ARM64 1
#endif

#if defined(__CYGWIN__) && defined(EJ_ARM64)
#error "Cygwin ARM64 is not supported by ExprJIT"
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
static void* ej_alloc_exec(const size_t size) {
  return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static bool ej_make_exec(void* ptr, const size_t size) {
  DWORD old_protect = 0;
  return VirtualProtect(ptr, size, PAGE_EXECUTE_READ, &old_protect) != 0;
}

static void ej_free_exec(void* ptr, const size_t size) {
  (void)size;
  if (ptr) {
    VirtualFree(ptr, 0, MEM_RELEASE);
  }
}
#else
static void* ej_alloc_exec(const size_t size) {
  void* ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == MAP_FAILED) {
    return NULL;
  }
  return ptr;
}

static bool ej_make_exec(void* ptr, const size_t size) {
  return mprotect(ptr, size, PROT_READ | PROT_EXEC) == 0;
}

static void ej_free_exec(void* ptr, const size_t size) {
  if (ptr) {
    munmap(ptr, size);
  }
}
#endif

enum {
  OP_pos = 0,
  OP_neg,
  OP_add,
  OP_sub,
  OP_mul,
  OP_div,
  OP_var,
  OP_con,
  
  OP_fun0 = 8,
  OP_fun1,
  OP_fun2,
  OP_fun3,
  OP_fun4,
  OP_fun5,
  OP_fun6,
  OP_fun7,
  
  OP_clo0 = 16,
  OP_clo1,
  OP_clo2,
  OP_clo3,
  OP_clo4,
  OP_clo5,
  OP_clo6,
  OP_clo7,

  OP_gt   = 24,
  OP_ge,
  OP_lt,
  OP_le,
  OP_eq,
  OP_neq,
  OP_and,
  OP_or,
  OP_not,

  OP_ret,
};

struct ej_bytecode {
  uint64_t *ops;
  size_t size;
  size_t capacity;
  double(*jit)();
  size_t jit_size;
};

typedef enum {
  OPT_VAL_CONST,
  OPT_VAL_VAR,
  OPT_VAL_EMITTED
} opt_val_kind;

typedef struct {
  opt_val_kind kind;
  double constant;
  uint64_t imm;
} opt_val;

typedef struct {
  uint64_t *ops;
  size_t size;
  size_t capacity;
} opt_bc;

static void opt_bc_init(opt_bc *out, size_t cap) {
  out->size = 0;
  out->capacity = cap ? cap : 16;
  out->ops = malloc(out->capacity * sizeof(uint64_t));
}

static void opt_bc_free(opt_bc *out) {
  if (out->ops) {
    free(out->ops);
    out->ops = NULL;
  }
  out->size = 0;
  out->capacity = 0;
}

static void opt_bc_push(opt_bc *out, uint64_t op) {
  if (out->size == out->capacity) {
    out->capacity *= 2;
    out->ops = realloc(out->ops, out->capacity * sizeof(uint64_t));
  }
  out->ops[out->size++] = op;
}

static uint64_t double_to_u64(double value) {
  uint64_t bits;
  memcpy(&bits, &value, sizeof(bits));
  return bits;
}

static double u64_to_double(uint64_t bits) {
  double value;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

static void opt_materialize_val(opt_bc *out, opt_val *val) {
  if (val->kind == OPT_VAL_CONST) {
    opt_bc_push(out, OP_con);
    opt_bc_push(out, double_to_u64(val->constant));
    val->kind = OPT_VAL_EMITTED;
  } else if (val->kind == OPT_VAL_VAR) {
    opt_bc_push(out, OP_var);
    opt_bc_push(out, val->imm);
    val->kind = OPT_VAL_EMITTED;
  }
}

static void opt_materialize_all(opt_bc *out, opt_val *stack, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    opt_materialize_val(out, &stack[i]);
  }
}

static bool opt_is_const(const opt_val *val, double expected) {
  return val->kind == OPT_VAL_CONST && val->constant == expected;
}

static ej_bytecode *bc_alloc(const size_t cap) {
  ej_bytecode *bc = malloc(sizeof(ej_bytecode));
  bc->ops = malloc(sizeof(uint64_t) * cap);
  bc->size = 0;
  bc->capacity = cap;
  bc->jit = NULL;
  bc->jit_size = 0;
  return bc;
}

static void bc_push_op(ej_bytecode *bc, const uint64_t op) {
  assert(bc);
  const size_t size = bc->size;
  const size_t capacity = bc->capacity;
  if (size == capacity) {
    const size_t newCapacity = capacity * 2;
    bc->capacity = newCapacity;
    bc->ops = realloc(bc->ops, newCapacity * sizeof(uint64_t));
  }
  const size_t newSize = size + 1;
  bc->ops[size] = op;
  bc->size = newSize;
}

static void bc_push_var(ej_bytecode *bc, const double *var) {
  bc_push_op(bc, OP_var);
  bc_push_op(bc, *(uint64_t*)(&var));
}

static void bc_push_con(ej_bytecode *bc, double con) {
  bc_push_op(bc, OP_con);
  bc_push_op(bc, *(uint64_t*)(&con));
}

static void bc_push_fun(ej_bytecode *bc, const void *addr, size_t arity) {
  assert(arity < 8);
  bc_push_op(bc, OP_fun0 + arity);
  bc_push_op(bc, *(uint64_t*)(&addr));
}

static void bc_push_clo(ej_bytecode *bc, const void *addr, size_t arity, void *ctx) {
  assert(arity < 8);
  bc_push_op(bc, OP_clo0 + arity);
  bc_push_op(bc, *(uint64_t*)(&ctx));
  bc_push_op(bc, *(uint64_t*)(&addr));
}

enum {
  OPER_inflix,
  OPER_prefix,
  OPER_paren
};

enum {
  ASSOC_left,
  ASSOC_right
};

typedef struct oper {
  const char *name;
  uint8_t prec;
  uint8_t assoc;
  uint8_t type;
  uint8_t args;
  const void *addr;
  void *ctx;
} oper;

// use same precedence as C
// https://en.cppreference.com/w/c/language/operator_precedence
static oper prec_table[] = {
  {"+",  1,  ASSOC_right, OPER_prefix},
  {"-",  1,  ASSOC_right, OPER_prefix},
  {"!",  1,  ASSOC_right, OPER_prefix},
  {"^",  2,  ASSOC_right, OPER_inflix},
  {"*",  3,  ASSOC_left,  OPER_inflix},
  {"/",  3,  ASSOC_left,  OPER_inflix},
  {"%",  3,  ASSOC_left,  OPER_inflix},
  {"+",  4,  ASSOC_left,  OPER_inflix},
  {"-",  4,  ASSOC_left,  OPER_inflix},
  {"<=", 6,  ASSOC_left,  OPER_inflix},
  {"<",  6,  ASSOC_left,  OPER_inflix},
  {">=", 6,  ASSOC_left,  OPER_inflix},
  {">",  6,  ASSOC_left,  OPER_inflix},
  {"==", 7,  ASSOC_left,  OPER_inflix},
  {"!=", 7,  ASSOC_left,  OPER_inflix},
  {"&&", 11, ASSOC_left,  OPER_inflix},
  {"||", 12, ASSOC_left,  OPER_inflix},
};

const char* valid_op_chars = "+-!^*/%<>=&|()_,.$";

static const double constant_e = M_E;
static const double constant_pi = M_PI;

static ej_variable builtins[] = {
  {"abs", fabs,        EJ_FUNCTION1, 0},
  {"acos", acos,       EJ_FUNCTION1, 0},
  {"asin", asin,       EJ_FUNCTION1, 0},
  {"atan", atan,       EJ_FUNCTION1, 0},
  {"atan2", atan2,     EJ_FUNCTION2, 0},
  {"ceil", ceil,       EJ_FUNCTION1, 0},
  {"cos", cos,         EJ_FUNCTION1, 0},
  {"cosh", cosh,       EJ_FUNCTION1, 0},
  {"e", &constant_e,   EJ_VARIABLE, 0},
  {"exp", exp,         EJ_FUNCTION1, 0},
  {"floor", floor,     EJ_FUNCTION1, 0},
  {"ln", log,          EJ_FUNCTION1, 0},
  {"log", log,         EJ_FUNCTION1, 0},
  {"log10", log10,     EJ_FUNCTION1, 0},
  {"pi", &constant_pi, EJ_VARIABLE, 0},
  {"pow", &pow,        EJ_FUNCTION2, 0},
  {"sin", sin,         EJ_FUNCTION1, 0},
  {"sinh", sinh,       EJ_FUNCTION1, 0},
  {"sqrt", sqrt,       EJ_FUNCTION1, 0},
  {"tan", tan,         EJ_FUNCTION1, 0},
  {"tanh", tanh,       EJ_FUNCTION1, 0},
};

typedef struct oper_stack {
  oper *data;
  size_t size;
  size_t capacity;
} oper_stack;

static oper_stack os_alloc(const size_t cap) {
  oper_stack stack = {malloc(cap * sizeof(oper)), 0, cap};
  return stack;
}

static void os_free(oper_stack *stack) {
  assert(stack);
  free(stack->data);
}

static oper *os_top(oper_stack *stack) {
  assert(stack);
  assert(stack->size);
  return stack->data + (stack->size - 1);
}

static void os_pop(oper_stack *stack) {
  assert(stack);
  assert(stack->size);
  stack->size--;
}

static void os_push(oper_stack *stack, oper op) {
  assert(stack);
  const size_t size = stack->size;
  const size_t capacity = stack->capacity;
  if (size == capacity) {
    const size_t newCapacity = capacity * 2;
    stack->capacity = newCapacity;
    stack->data = realloc(stack->data, newCapacity * sizeof(oper));
  }
  const size_t newSize = size + 1;
  stack->data[size] = op;
  stack->size = newSize;
}

static ej_variable *findVar(ej_variable *vars, size_t len, const char *ident, size_t identSize) {
  for (; len != 0; --len, ++vars) {
    if (strncmp(vars->name, ident, identSize) == 0) {
      return vars;
    }
  }
  return NULL;
}

static ej_variable *findBuiltin(const char *ident, size_t identSize) {
  return findVar(builtins, sizeof(builtins) / sizeof(builtins[0]), ident, identSize);
}

static oper *findOper(const char* op, int type) {
  oper *row = prec_table;
  size_t size = sizeof(prec_table) / sizeof(prec_table[0]);
  for (; size != 0; --size, ++row) {
    if (strncmp(op, row->name, strlen(row->name)) == 0 && row->type == type) {
      return row;
    }
  }
  return NULL;
}

static bool funOrClo(const int type) {
  return (type & EJ_FUNCTION) == EJ_FUNCTION || (type & EJ_CLOSURE) == EJ_CLOSURE;
}

static bool shouldPop(oper *top, oper *other) {
  if (top->type == OPER_paren) return false;
  if ((top->type & EJ_FUNCTION) == EJ_FUNCTION) return true;
  if ((top->type & EJ_CLOSURE) == EJ_CLOSURE) return true;
  if (top->prec <= other->prec) return true;
  if (top->prec == other->prec && top->assoc == ASSOC_left) return true;
  return false;
}

#define ARITY(TYPE) ((TYPE) & 7)

static void pushToOutput(ej_bytecode *bc, oper *op) {
  if (op->type == OPER_inflix) {
    if (strncmp(op->name, "+", 1) == 0) {
      bc_push_op(bc, OP_add);
    } else if (strncmp(op->name, "-", 1) == 0) {
      bc_push_op(bc, OP_sub);
    } else if (strncmp(op->name, "*", 1) == 0) {
      bc_push_op(bc, OP_mul);
    } else if (strncmp(op->name, "/", 1) == 0) {
      bc_push_op(bc, OP_div);
    } else if (strncmp(op->name, "%", 1) == 0) {
      bc_push_fun(bc, fmod, 2);
    } else if (strncmp(op->name, "^", 1) == 0) {
      bc_push_fun(bc, pow, 2);
    } else if (strncmp(op->name, "<=", 2) == 0) {
      bc_push_op(bc, OP_le);
    } else if (strncmp(op->name, ">=", 2) == 0) {
      bc_push_op(bc, OP_ge);
    } else if (strncmp(op->name, "<", 1) == 0) {
      bc_push_op(bc, OP_lt);
    } else if (strncmp(op->name, ">", 1) == 0) {
      bc_push_op(bc, OP_gt);
    } else if (strncmp(op->name, "==", 2) == 0) {
      bc_push_op(bc, OP_eq);
    } else if (strncmp(op->name, "!=", 2) == 0) {
      bc_push_op(bc, OP_neq);
    } else if (strncmp(op->name, "&&", 2) == 0) {
      bc_push_op(bc, OP_and);
    } else if (strncmp(op->name, "||", 2) == 0) {
      bc_push_op(bc, OP_or);
    } else {
      assert(false);
    }
  } else if (op->type == OPER_prefix) {
    if (op->name[0] == '+') {
      // bc_push_op(bc, OP_pos);
    } else if (op->name[0] == '-') {
      bc_push_op(bc, OP_neg);
    } else if (op->name[0] == '!') {
      bc_push_op(bc, OP_not);
    } else {
      assert(false);
    }
  } else if ((op->type & EJ_FUNCTION) == EJ_FUNCTION) {
    bc_push_fun(bc, op->addr, ARITY(op->type));
  } else if ((op->type & EJ_CLOSURE) == EJ_CLOSURE) {
    bc_push_clo(bc, op->addr, ARITY(op->type), op->ctx);
  } else {
    assert(false);
  }
}

static bool pushOutputUntilParen(ej_bytecode *bc, oper_stack *stack) {
  oper *top = os_top(stack);
  while (top->type != OPER_paren) {
    pushToOutput(bc, top);
    os_pop(stack);
    if (stack->size == 0) {
      // Unmatching parentheses
      return false;
    }
    top = os_top(stack);
  }
  return true;
}

static bool pushToOper(ej_bytecode *bc, oper_stack *stack, oper *op) {
  if (stack->size) {
    oper *top = os_top(stack);
    while (shouldPop(top, op)) {
      pushToOutput(bc, top);
      os_pop(stack);
      if (stack->size == 0) {
        break;
      }
      top = os_top(stack);
    }
  }
  os_push(stack, *op);
  return true;
}

static oper *incrementArgs(oper_stack *stack) {
  assert(stack->size);
  oper *top = stack->data + (stack->size - 1);
  assert(top->type == OPER_paren);
  if (stack->size == 1) return NULL;
  --top;
  if (funOrClo(top->type)) {
    ++top->args;
    return top;
  } else {
    return NULL;
  }
}

static char prevChar(const char *str) {
  --str;
  while (isspace(*str)) --str;
  return *str;
}

bool ej_validate(ej_bytecode *bc) {
  if (!bc || !(bc->ops)) {
    return false;
  }

  size_t stack_size = 0;
  uint64_t *op = bc->ops;
  while (1) {
    switch (*op) {
      case OP_pos:
      case OP_neg:
      case OP_not:
        if (stack_size < 1) {
          return false;
        }
        break;
      case OP_add:
      case OP_sub:
      case OP_mul:
      case OP_div:
      case OP_gt:
      case OP_ge:
      case OP_lt:
      case OP_le:
      case OP_eq:
      case OP_neq:
      case OP_and:
      case OP_or:
        if (stack_size < 2) {
          return false;
        }
        stack_size--;
        break;

      case OP_var:
        ++op;
        stack_size++;
        break;
      case OP_con:
        ++op;
        stack_size++;
        break;
      case OP_ret:
        if (stack_size != 1) {
          return false;
        }
        return true;

      case OP_clo0: case OP_clo1: case OP_clo2: case OP_clo3:
      case OP_clo4: case OP_clo5: case OP_clo6: case OP_clo7: {
        const uint64_t arity = ARITY(*op);
        ++op;
        ++op;
        if (stack_size < arity) {
          return false;
        }
        stack_size -= (arity - 1);
        break;
      }

      case OP_fun0: case OP_fun1: case OP_fun2: case OP_fun3:
      case OP_fun4: case OP_fun5: case OP_fun6: case OP_fun7: {
        const uint64_t arity = ARITY(*op);
        ++op;
        if (stack_size < arity) {
          return false;
        }
        stack_size -= (arity - 1);
        break;
      }

      default:
        return false;
    }
    ++op;
  }
  return false;
}
  
ej_bytecode *ej_compile(const char *str, ej_variable *vars, size_t len, int *error) {
  assert(str);
  assert(vars ? len != 0 : len == 0);

  ej_bytecode *bc = bc_alloc(64);
  oper_stack stack = os_alloc(16);
  bool prefixContext = true;

  if (error) {
    *error = 0;
  }

  const char* start = str;
  bool pushed;

  while (*str) {
    while (isspace(*str)) ++str;

    if (*str == '\0') { continue; }

    if (isalnum(*str) == false && strchr(valid_op_chars, *str) == NULL) {
        // invalid character
        goto error;
    }
    
    if (isalpha(*str) || *str == '_' || *str == '$') {
      const char *begin = str++;
      while (isalnum(*str) || *str == '_' || *str == '$') ++str;
      ej_variable *var = findVar(vars, len, begin, str - begin);
      if (!var) {
        var = findBuiltin(begin, str - begin);
      } else {
        var->bound = true;
      }
      if (!var) {
        // "Failed to lookup identifier"
        goto error;
      }
      while (isspace(*str)) ++str;
      if (*str == '(') {
        if (!funOrClo(var->type)) {
          // "Calling a variable"
          goto error;
        }
        oper op;
        op.name = var->name;
        op.prec = 255;
        op.assoc = 0;
        op.type = var->type;
        op.args = 0;
        op.addr = var->address;
        op.ctx = var->context;
        os_push(&stack, op);
      } else {
        if (var->type != EJ_VARIABLE) {
          // "Taking the value of a function"
          goto error;
        }
        bc_push_var(bc, var->address);
        prefixContext = false;
      }
      continue;
    }
    
    if (*str == '(') {
      ++str;
      oper op;
      op.name = "(";
      op.prec = 255;
      op.assoc = 0;
      op.type = OPER_paren;
      os_push(&stack, op);
      prefixContext = true;
      continue;
    }
    
    if (*str == ',') {
      ++str;
      if (stack.size == 0) {
        goto error;
      }
      pushed = pushOutputUntilParen(bc, &stack);
      if (pushed == false) {
        goto error;
      }
      incrementArgs(&stack);
      prefixContext = true;
      continue;
    }
    
    if (*str == ')') {
      const char prev = prevChar(str);
      ++str;
      if (stack.size == 0) {
        goto error;
      }
      pushed = pushOutputUntilParen(bc, &stack);
      if (pushed == false) {
        goto error;
      }
      if (prev != '(') {
        oper *fun = incrementArgs(&stack);
        if (fun && fun->args != ARITY(fun->type)) {
          // "Wrong number of arguments
          goto error;
        }
      }
      os_pop(&stack);
      prefixContext = false;
      continue;
    }
    
    if (prefixContext) {
      oper *op = findOper(str, OPER_prefix);
      if (op) {
        str += strlen(op->name);
        pushed = pushToOper(bc, &stack, op);
        if (pushed == false) {
          goto error;
        }
        prefixContext = true;
        continue;
      }
    }
    
    oper *op = findOper(str, OPER_inflix);
    if (op) {
      str += strlen(op->name);
      pushed = pushToOper(bc, &stack, op);
      if (pushed == false) {
        goto error;
      }
      prefixContext = true;
      continue;
    }
    
    char *end;
    double number = strtod(str, &end);
    if (end != str) {
      str = end;
      bc_push_con(bc, number);
      prefixContext = false;
      continue;
    }
    
    goto error;
  }
  
  while (stack.size) {
    oper *top = os_top(&stack);
    if (top->type == OPER_paren) {
      goto error;
    }
    pushToOutput(bc, top);
    os_pop(&stack);
  }

  if (bc->size == 0) {
    goto error;
  }
  
  bc_push_op(bc, OP_ret);

  if (ej_validate(bc) == false) {
    goto error;
  }

  os_free(&stack);
  return bc;

error:
  // if (bc->size != 0) {
  //   bc_push_op(bc, OP_ret);
  //   ej_print(bc);
  // }
  if (error) {
    *error = (str - start);
    if (*error == 0) *error = 1;
  }
  os_free(&stack);
  ej_free(bc);

  return NULL;
}

#define CAST_FUN(...) (*(double(**)(__VA_ARGS__))(++op))
#define CAST_CLO(...) (*(double(**)(void *, __VA_ARGS__))(++op))
#define CAST_CLO0()   (*(double(**)(void *))(++op))
#define CAST_CTX()    (*(void**)(++op))

#define PUSH(VAL)                                                               \
  ++top;                                                                        \
  assert(top < stack + EJ_STACK_SIZE && "Stack overflow");                      \
  *top = VAL
#define POP() *(top--)

double ej_eval_switch(ej_bytecode *bc) {
  if (!bc) return NAN;

  if (bc->jit != NULL) {
    return (bc->jit)();
  }

  assert(bc->ops);
  uint64_t *op = bc->ops;
  double stack[EJ_STACK_SIZE];
  double *top = stack;
  
  double x, y;
  void *ctx;
  
  while (1) {
    switch (*op) {
      case OP_pos:
        *top = +(*top);
        break;
      case OP_neg:
        *top = -(*top);
        break;
      case OP_add:
        y = POP();
        x = *top;
        *top = x + y;
        break;
      case OP_sub:
        y = POP();
        x = *top;
        *top = x - y;
        break;
      case OP_mul:
        y = POP();
        x = *top;
        *top = x * y;
        break;
      case OP_div:
        y = POP();
        x = *top;
        *top = x / y;
        break;
      case OP_var:
        ++op;
        PUSH(**(double**)op);
        break;
      case OP_con:
        ++op;
        PUSH(*(double*)op);
        break;
      case OP_ret:
        return *top;
      
      case OP_fun0:
        PUSH(CAST_FUN(void)());
        break;
      case OP_fun1:
        x = CAST_FUN(double)(*top);
        --top;
        PUSH(x);
        break;
      case OP_fun2:
        x = CAST_FUN(double, double)(top[-1], top[0]);
        top -= 2;
        PUSH(x);
        break;
      case OP_fun3:
        x = CAST_FUN(double, double, double)(top[-2], top[-1], top[0]);
        top -= 3;
        PUSH(x);
        break;
      case OP_fun4:
        x = CAST_FUN(double, double, double, double)(top[-3], top[-2], top[-1], top[0]);
        top -= 4;
        PUSH(x);
        break;
      case OP_fun5:
        x = CAST_FUN(double, double, double, double, double)(top[-4], top[-3], top[-2], top[-1], top[0]);
        top -= 5;
        PUSH(x);
        break;
      case OP_fun6:
        x = CAST_FUN(double, double, double, double, double, double)(top[-5], top[-4], top[-3], top[-2], top[-1], top[0]);
        top -= 6;
        PUSH(x);
        break;
      case OP_fun7:
        x = CAST_FUN(double, double, double, double, double, double, double)(top[-6], top[-5], top[-4], top[-3], top[-2], top[-1], top[0]);
        top -= 7;
        PUSH(x);
        break;
      
      case OP_clo0:
        ctx = CAST_CTX();
        PUSH(CAST_CLO0()(ctx));
        break;
      case OP_clo1:
        ctx = CAST_CTX();
        x = CAST_CLO(double)(ctx, *top);
        --top;
        PUSH(x);
        break;
      case OP_clo2:
        ctx = CAST_CTX();
        x = CAST_CLO(double, double)(ctx, top[-1], top[0]);
        top -= 2;
        PUSH(x);
        break;
      case OP_clo3:
        ctx = CAST_CTX();
        x = CAST_CLO(double, double, double)(ctx, top[-2], top[-1], top[0]);
        top -= 3;
        PUSH(x);
        break;
      case OP_clo4:
        ctx = CAST_CTX();
        x = CAST_CLO(double, double, double, double)(ctx, top[-3], top[-2], top[-1], top[0]);
        top -= 4;
        PUSH(x);
        break;
      case OP_clo5:
        ctx = CAST_CTX();
        x = CAST_CLO(double, double, double, double, double)(ctx, top[-4], top[-3], top[-2], top[-1], top[0]);
        top -= 5;
        PUSH(x);
        break;
      case OP_clo6:
        ctx = CAST_CTX();
        x = CAST_CLO(double, double, double, double, double, double)(ctx, top[-5], top[-4], top[-3], top[-2], top[-1], top[0]);
        top -= 6;
        PUSH(x);
        break;
      case OP_clo7:
        ctx = CAST_CTX();
        x = CAST_CLO(double, double, double, double, double, double, double)(ctx, top[-6], top[-5], top[-4], top[-3], top[-2], top[-1], top[0]);
        top -= 7;
        PUSH(x);
        break;

      case OP_lt:
        y = POP();
        x = *top;
        *top = (double)(x < y);
        break;
      case OP_le:
        y = POP();
        x = *top;
        *top = (double)(x <= y);
        break;
      case OP_gt:
        y = POP();
        x = *top;
        *top = (double)(x > y);
        break;
      case OP_ge:
        y = POP();
        x = *top;
        *top = (double)(x >= y);
        break;
      case OP_eq:
        y = POP();
        x = *top;
        *top = (double)(x == y);
        break;
      case OP_neq:
        y = POP();
        x = *top;
        *top = (double)(x != y);
        break;
      case OP_and:
        y = POP();
        x = *top;
        *top = (double)(x && y);
        break;
      case OP_or:
        y = POP();
        x = *top;
        *top = (double)(x || y);
        break;
      case OP_not:
        *top = (double)(!(*top));
        break;

      default:
        assert(0);
        __builtin_unreachable();
    }
    
    ++op;
  }
}

#ifdef HAVE_COMPUTED_GOTO
double ej_eval_goto(ej_bytecode *bc) {
  if (!bc) return NAN;

  if (bc->jit != NULL) {
    return (bc->jit)();
  }

  assert(bc->ops);
  uint64_t *op = bc->ops;
  double stack[EJ_STACK_SIZE];
  double *top = stack;

  double x, y;
  void *ctx;

  static void* dispatch_table[] = {
    &&do_OP_pos,
    &&do_OP_neg,
    &&do_OP_add, &&do_OP_sub, &&do_OP_mul, &&do_OP_div,
    &&do_OP_var, &&do_OP_con,
    &&do_OP_fun0, &&do_OP_fun1, &&do_OP_fun2, &&do_OP_fun3, &&do_OP_fun4, &&do_OP_fun5, &&do_OP_fun6, &&do_OP_fun7,
    &&do_OP_clo0, &&do_OP_clo1, &&do_OP_clo2, &&do_OP_clo3, &&do_OP_clo4, &&do_OP_clo5, &&do_OP_clo6, &&do_OP_clo7,
    &&do_OP_gt, &&do_OP_ge, &&do_OP_lt, &&do_OP_le, &&do_OP_eq, &&do_OP_neq, &&do_OP_and, &&do_OP_or, &&do_OP_not,
    &&do_OP_ret,
  };

  #define DISPATCH() goto *dispatch_table[*(++op)]
  goto *dispatch_table[*op];
  while (1) {
    do_OP_pos:
      *top = +(*top);
      DISPATCH();
    do_OP_neg:
      *top = -(*top);
      DISPATCH();
    do_OP_add:
      y = POP();
      // x = *top;
      // *top = x + y;
      *top += y;
      DISPATCH();
    do_OP_sub:
      y = POP();
      // x = *top;
      // *top = x - y;
      *top -= y;
      DISPATCH();
    do_OP_mul:
      y = POP();
      // x = *top;
      // *top = x * y;
      *top *= y;
      DISPATCH();
    do_OP_div:
      y = POP();
      // x = *top;
      // *top = x / y;
      *top /= y;
      DISPATCH();
    do_OP_var:
      ++op;
      PUSH(**(double**)op);
      DISPATCH();
    do_OP_con:
      ++op;
      PUSH(*(double*)op);
      DISPATCH();

    do_OP_fun0:
      PUSH(CAST_FUN(void)());
      DISPATCH();
    do_OP_fun1:
      x = CAST_FUN(double)(*top);
      --top;
      PUSH(x);
      DISPATCH();
    do_OP_fun2:
      x = CAST_FUN(double, double)(top[-1], top[0]);
      top -= 2;
      PUSH(x);
      DISPATCH();
    do_OP_fun3:
      x = CAST_FUN(double, double, double)(top[-2], top[-1], top[0]);
      top -= 3;
      PUSH(x);
      DISPATCH();
    do_OP_fun4:
      x = CAST_FUN(double, double, double, double)(top[-3], top[-2], top[-1], top[0]);
      top -= 4;
      PUSH(x);
      DISPATCH();
    do_OP_fun5:
      x = CAST_FUN(double, double, double, double, double)(top[-4], top[-3], top[-2], top[-1], top[0]);
      top -= 5;
      PUSH(x);
      DISPATCH();
    do_OP_fun6:
      x = CAST_FUN(double, double, double, double, double, double)(top[-5], top[-4], top[-3], top[-2], top[-1], top[0]);
      top -= 6;
      PUSH(x);
      DISPATCH();
    do_OP_fun7:
      x = CAST_FUN(double, double, double, double, double, double, double)(top[-6], top[-5], top[-4], top[-3], top[-2], top[-1], top[0]);
      top -= 7;
      PUSH(x);
      DISPATCH();

    do_OP_clo0:
      ctx = CAST_CTX();
      PUSH(CAST_CLO0()(ctx));
      DISPATCH();
    do_OP_clo1:
      ctx = CAST_CTX();
      x = CAST_CLO(double)(ctx, *top);
      --top;
      PUSH(x);
      DISPATCH();
    do_OP_clo2:
      ctx = CAST_CTX();
      x = CAST_CLO(double, double)(ctx, top[-1], top[0]);
      top -= 2;
      PUSH(x);
      DISPATCH();
    do_OP_clo3:
      ctx = CAST_CTX();
      x = CAST_CLO(double, double, double)(ctx, top[-2], top[-1], top[0]);
      top -= 3;
      PUSH(x);
      DISPATCH();
    do_OP_clo4:
      ctx = CAST_CTX();
      x = CAST_CLO(double, double, double, double)(ctx, top[-3], top[-2], top[-1], top[0]);
      top -= 4;
      PUSH(x);
      DISPATCH();
    do_OP_clo5:
      ctx = CAST_CTX();
      x = CAST_CLO(double, double, double, double, double)(ctx, top[-4], top[-3], top[-2], top[-1], top[0]);
      top -= 5;
      PUSH(x);
      DISPATCH();
    do_OP_clo6:
      ctx = CAST_CTX();
      x = CAST_CLO(double, double, double, double, double, double)(ctx, top[-5], top[-4], top[-3], top[-2], top[-1], top[0]);
      top -= 6;
      PUSH(x);
      DISPATCH();
    do_OP_clo7:
      ctx = CAST_CTX();
      x = CAST_CLO(double, double, double, double, double, double, double)(ctx, top[-6], top[-5], top[-4], top[-3], top[-2], top[-1], top[0]);
      top -= 7;
      PUSH(x);
      DISPATCH();

    do_OP_gt:
      y = POP();
      x = *top;
      *top = (double)(x > y);
      DISPATCH();
    do_OP_ge:
      y = POP();
      x = *top;
      *top = (double)(x >= y);
      DISPATCH();
    do_OP_lt:
      y = POP();
      x = *top;
      *top = (double)(x < y);
      DISPATCH();
    do_OP_le:
      y = POP();
      x = *top;
      *top = (double)(x <= y);
      DISPATCH();
    do_OP_eq:
      y = POP();
      x = *top;
      *top = (double)(x == y);
      DISPATCH();
    do_OP_neq:
      y = POP();
      x = *top;
      *top = (double)(x != y);
      DISPATCH();
    do_OP_and:
      y = POP();
      x = *top;
      *top = (double)(x && y);
      DISPATCH();
    do_OP_or:
      y = POP();
      x = *top;
      *top = (double)(x || y);
      DISPATCH();
    do_OP_not:
      *top = (double)(!(*top));
      DISPATCH();

    do_OP_ret:
      return *top;
  }
}
#endif

void ej_free(ej_bytecode *bc) {
  if (bc) {
    if (bc->jit) {
      ej_free_exec(bc->jit, bc->jit_size);
    }
    free(bc->ops);
    free(bc);
  }
}

void ej_optimize(ej_bytecode *bc) {
  if (!bc || !bc->ops) {
    return;
  }
  if (bc->jit) {
    ej_free_exec(bc->jit, bc->jit_size);
    bc->jit = NULL;
    bc->jit_size = 0;
  }

  opt_bc out;
  opt_bc_init(&out, bc->size);

  opt_val stack[EJ_STACK_SIZE];
  size_t stack_size = 0;
  bool ok = true;

  uint64_t *op = bc->ops;
  while (1) {
    switch (*op) {
      case OP_pos:
        break;
      case OP_neg: {
        if (stack_size < 1) {
          ok = false;
          goto finalize;
        }
        opt_val *top = &stack[stack_size - 1];
        if (top->kind == OPT_VAL_CONST) {
          top->constant = -top->constant;
        } else {
          opt_materialize_all(&out, stack, stack_size);
          opt_bc_push(&out, OP_neg);
          top->kind = OPT_VAL_EMITTED;
        }
        break;
      }
      case OP_not: {
        if (stack_size < 1) {
          ok = false;
          goto finalize;
        }
        opt_val *top = &stack[stack_size - 1];
        if (top->kind == OPT_VAL_CONST) {
          top->constant = (top->constant == 0.0) ? 1.0 : 0.0;
        } else {
          opt_materialize_all(&out, stack, stack_size);
          opt_bc_push(&out, OP_not);
          top->kind = OPT_VAL_EMITTED;
        }
        break;
      }
      case OP_add:
      case OP_sub:
      case OP_mul:
      case OP_div:
      case OP_gt:
      case OP_ge:
      case OP_lt:
      case OP_le:
      case OP_eq:
      case OP_neq:
      case OP_and:
      case OP_or: {
        if (stack_size < 2) {
          ok = false;
          goto finalize;
        }
        opt_val *rhs = &stack[stack_size - 1];
        opt_val *lhs = &stack[stack_size - 2];
        const bool lhs_emitted = lhs->kind == OPT_VAL_EMITTED;
        const bool rhs_emitted = rhs->kind == OPT_VAL_EMITTED;

        if (!lhs_emitted && !rhs_emitted && lhs->kind == OPT_VAL_CONST && rhs->kind == OPT_VAL_CONST) {
          double result = 0.0;
          const double x = lhs->constant;
          const double y = rhs->constant;
          switch (*op) {
            case OP_add: result = x + y; break;
            case OP_sub: result = x - y; break;
            case OP_mul: result = x * y; break;
            case OP_div: result = x / y; break;
            case OP_gt: result = (double)(x > y); break;
            case OP_ge: result = (double)(x >= y); break;
            case OP_lt: result = (double)(x < y); break;
            case OP_le: result = (double)(x <= y); break;
            case OP_eq: result = (double)(x == y); break;
            case OP_neq: result = (double)(x != y); break;
            case OP_and: result = (double)((x != 0.0) && (y != 0.0)); break;
            case OP_or: result = (double)((x != 0.0) || (y != 0.0)); break;
            default: break;
          }
          stack_size -= 2;
          stack[stack_size++] = (opt_val){OPT_VAL_CONST, result, 0};
          break;
        }

        if (!lhs_emitted && !rhs_emitted) {
          if (*op == OP_add) {
            if (opt_is_const(rhs, 0.0)) {
              stack_size--;
              break;
            }
            if (opt_is_const(lhs, 0.0)) {
              stack[stack_size - 2] = *rhs;
              stack_size--;
              break;
            }
          } else if (*op == OP_sub) {
            if (opt_is_const(rhs, 0.0)) {
              stack_size--;
              break;
            }
          } else if (*op == OP_mul) {
            if (opt_is_const(rhs, 1.0)) {
              stack_size--;
              break;
            }
            if (opt_is_const(lhs, 1.0)) {
              stack[stack_size - 2] = *rhs;
              stack_size--;
              break;
            }
          } else if (*op == OP_div) {
            if (opt_is_const(rhs, 1.0)) {
              stack_size--;
              break;
            }
          } else if (*op == OP_and) {
            if (opt_is_const(lhs, 0.0) || opt_is_const(rhs, 0.0)) {
              stack_size -= 2;
              stack[stack_size++] = (opt_val){OPT_VAL_CONST, 0.0, 0};
              break;
            }
          } else if (*op == OP_or) {
            if ((lhs->kind == OPT_VAL_CONST && lhs->constant != 0.0) ||
                (rhs->kind == OPT_VAL_CONST && rhs->constant != 0.0)) {
              stack_size -= 2;
              stack[stack_size++] = (opt_val){OPT_VAL_CONST, 1.0, 0};
              break;
            }
          }
        }

        opt_materialize_all(&out, stack, stack_size);
        opt_bc_push(&out, *op);
        stack_size -= 2;
        stack[stack_size++] = (opt_val){OPT_VAL_EMITTED, 0.0, 0};
        break;
      }

      case OP_var:
        op++;
        if (stack_size >= EJ_STACK_SIZE) {
          ok = false;
          goto finalize;
        }
        stack[stack_size++] = (opt_val){OPT_VAL_VAR, 0.0, *op};
        break;
      case OP_con:
        op++;
        if (stack_size >= EJ_STACK_SIZE) {
          ok = false;
          goto finalize;
        }
        stack[stack_size++] = (opt_val){OPT_VAL_CONST, u64_to_double(*op), 0};
        break;
      case OP_fun0: case OP_fun1: case OP_fun2: case OP_fun3:
      case OP_fun4: case OP_fun5: case OP_fun6: case OP_fun7: {
        const size_t arity = ARITY(*op);
        if (stack_size < arity) {
          ok = false;
          goto finalize;
        }
        opt_materialize_all(&out, stack, stack_size);
        opt_bc_push(&out, *op);
        op++;
        opt_bc_push(&out, *op);
        stack_size -= arity;
        stack[stack_size++] = (opt_val){OPT_VAL_EMITTED, 0.0, 0};
        break;
      }
      case OP_clo0: case OP_clo1: case OP_clo2: case OP_clo3:
      case OP_clo4: case OP_clo5: case OP_clo6: case OP_clo7: {
        const size_t arity = ARITY(*op);
        if (stack_size < arity) {
          ok = false;
          goto finalize;
        }
        opt_materialize_all(&out, stack, stack_size);
        opt_bc_push(&out, *op);
        op++;
        opt_bc_push(&out, *op);
        op++;
        opt_bc_push(&out, *op);
        stack_size -= arity;
        stack[stack_size++] = (opt_val){OPT_VAL_EMITTED, 0.0, 0};
        break;
      }
      case OP_ret:
        opt_materialize_all(&out, stack, stack_size);
        opt_bc_push(&out, OP_ret);
        goto finalize;
      default:
        ok = false;
        goto finalize;
    }
    ++op;
  }

finalize:
  if (ok && out.size != 0) {
    free(bc->ops);
    bc->ops = out.ops;
    bc->size = out.size;
    bc->capacity = out.capacity;
  } else {
    opt_bc_free(&out);
  }
}

void ej_print(ej_bytecode *bc) {
  if (!bc) {
    return;
  }
  assert(bc);
  assert(bc->ops);
  
  uint64_t *op = bc->ops;
  while (1) {
    switch (*op) {
      case OP_pos:
        // puts("pos");
        break;
      case OP_neg:
        puts("neg");
        break;
      case OP_add:
        puts("add");
        break;
      case OP_sub:
        puts("sub");
        break;
      case OP_mul:
        puts("mul");
        break;
      case OP_div:
        puts("div");
        break;

      case OP_var:
        ++op;
        printf("var %p\n", *(void**)op);
        break;
      case OP_con:
        ++op;
        printf("con %g\n", *(double*)op);
        break;
      case OP_ret:
        puts("ret");
        return;
      
      case OP_fun0: case OP_fun1: case OP_fun2: case OP_fun3:
      case OP_fun4: case OP_fun5: case OP_fun6: case OP_fun7: {
        const uint64_t arity = ARITY(*op);
        void *fun = *(void**)(++op);
        printf("fun %" PRIu64 " %p\n", arity, fun);
        break;
      }
      
      case OP_clo0: case OP_clo1: case OP_clo2: case OP_clo3:
      case OP_clo4: case OP_clo5: case OP_clo6: case OP_clo7: {
        const uint64_t arity = ARITY(*op);
        void *ctx = *(void**)(++op);
        void *fun = *(void**)(++op);
        printf("clo %" PRIu64 " %p ctx %p\n", arity, fun, ctx);
        break;
      }

      case OP_gt:
        puts("gt");
        break;
      case OP_ge:
        puts("ge");
        break;
      case OP_lt:
        puts("lt");
        break;
      case OP_le:
        puts("le");
        break;
      case OP_eq:
        puts("eq");
        break;
      case OP_neq:
        puts("neq");
        break;
      case OP_and:
        puts("and");
        break;
      case OP_or:
        puts("or");
        break;
      case OP_not:
        puts("not");
        break;
      
      default:
        puts("(garbage)");
        return;
    }
    ++op;
  }
}

double ej_interp(const char *str, int* error) {
  ej_bytecode *bc = ej_compile(str, NULL, 0, error);
  if (!bc) {
    return NAN;
  }
  const double result = ej_eval(bc);
  ej_free(bc);
  return result;
}

// JIT is supported on x86_64 and arm64. Cygwin ARM64 is unsupported (guarded above).
#if defined(EJ_X86_64) || defined(EJ_ARM64)
// #define DASM_CHECKS 1
#include "dynasm/dasm_proto.h"
#if defined(EJ_ARM64)
#include "jit_arm64.c"
#elif defined(EJ_X86_64)
#include "jit_amd64.c"
#endif
void ej_jit(ej_bytecode* bc) {
  if (!bc) {
    return;
  }
  assert(bc->ops);

  dasm_State* state;
  dasm_init(&state, DASM_MAXSECTION);
  void* global_labels[GLOB__MAX];
  dasm_setupglobal(&state, global_labels, GLOB__MAX);
  dasm_setup(&state, ej_compile_actionlist);

  emit(&state, bc->ops);

  int status;
  status = dasm_link(&state, &(bc->jit_size));
  if (status != DASM_S_OK) {
    goto free_state;
  }
  bc->jit = ej_alloc_exec(bc->jit_size);
  if (bc->jit == NULL) {
    goto free_state;
  }
  status = dasm_encode(&state, bc->jit);
  if (status != DASM_S_OK) {
    goto unmap;
  }
  if (ej_make_exec(bc->jit, bc->jit_size)) {
    goto free_state;
  }
unmap:
  ej_free_exec(bc->jit, bc->jit_size);
  bc->jit = NULL;
  bc->jit_size = 0;
free_state:
  dasm_free(&state);
  return;
}
#else
void ej_jit(ej_bytecode* bc) {
  return;
}
#endif
