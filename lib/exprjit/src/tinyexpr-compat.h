#ifndef tinyexpr_compat_h
#define tinyexpr_compat_h

#include "exprjit.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TE_NAT_LOG 1

enum {
  TE_VARIABLE = EJ_VARIABLE,
  TE_FUNCTION = EJ_FUNCTION,
  TE_FUNCTION0 = TE_FUNCTION, TE_FUNCTION1, TE_FUNCTION2, TE_FUNCTION3, TE_FUNCTION4, TE_FUNCTION5, TE_FUNCTION6, TE_FUNCTION7,
  TE_CLOSURE = EJ_CLOSURE,
  TE_CLOSURE0 = TE_CLOSURE, TE_CLOSURE1, TE_CLOSURE2, TE_CLOSURE3, TE_CLOSURE4, TE_CLOSURE5, TE_CLOSURE6, TE_CLOSURE7
};
typedef struct ej_variable te_variable;
typedef struct ej_bytecode te_expr;
#define te_compile(a,b,c,d) ej_compile((a),(b),(c),(d))
#define te_eval(a) ej_eval((a))
#define te_print(a) ej_print((a))
#define te_free(a) ej_free((a))
#define te_interp(a,b) ej_interp((a),(b))
#define te_jit(a) ej_jit((a))

#ifdef __cplusplus
}
#endif

#endif
