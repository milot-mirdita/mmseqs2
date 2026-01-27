//
//  exprjit.h
//  Expr JIT
//
//  Created by Indi Kernick on 5/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef expr_jit_h
#define expr_jit_h

#include <stddef.h>

#ifndef EJ_STACK_SIZE
#define EJ_STACK_SIZE 32
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum {
  EJ_VARIABLE,
  EJ_FUNCTION = 8,
  EJ_FUNCTION0 = EJ_FUNCTION, EJ_FUNCTION1, EJ_FUNCTION2, EJ_FUNCTION3, EJ_FUNCTION4, EJ_FUNCTION5, EJ_FUNCTION6, EJ_FUNCTION7,
  EJ_CLOSURE = 16,
  EJ_CLOSURE0 = EJ_CLOSURE, EJ_CLOSURE1, EJ_CLOSURE2, EJ_CLOSURE3, EJ_CLOSURE4, EJ_CLOSURE5, EJ_CLOSURE6, EJ_CLOSURE7
};

typedef struct ej_variable {
  const char *name;
  const void *address;
  int type;
  void *context;
  int bound;
} ej_variable;

typedef struct ej_bytecode ej_bytecode;

ej_bytecode *ej_compile(const char *, ej_variable *, size_t, int *);
double ej_eval_switch(ej_bytecode *);
#ifdef HAVE_COMPUTED_GOTO
double ej_eval_goto(ej_bytecode *);
#define ej_eval(x) ej_eval_goto((x))
#else
#define ej_eval(x) ej_eval_switch((x))
#endif
void ej_print(ej_bytecode *);
void ej_free(ej_bytecode *);
double ej_interp(const char *, int *);
void ej_jit(ej_bytecode* bc);
void ej_optimize(ej_bytecode* bc);

#ifdef __cplusplus
}
#endif

#endif
