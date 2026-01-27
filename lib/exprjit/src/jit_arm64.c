/*
** This file has been pre-processed with DynASM.
** https://luajit.org/dynasm.html
** DynASM version 1.5.0, DynASM arm version 1.5.0
** DO NOT EDIT! The original file is in "jit_arm64.dasc".
*/

#include "dynasm/dasm_arm64.h"

//|.arch arm64
#if DASM_VERSION != 10500
#error "Version mismatch between DynASM and included encoding engine"
#endif
//|.actionlist ej_compile_actionlist
static const unsigned int ej_compile_actionlist[329] = {
0xfd4003e0,
0x1e614000,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e612800,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e613800,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e610800,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e611800,
0xfd0003e0,
0x00000000,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xf9400020,
0x9e670000,
0xfc1f0fe0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x9e670000,
0xfc1f0fe0,
0x00000000,
0xfc4107e0,
0xd65f03c0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x00000000,
0xf81f0ffe,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xf84107fe,
0xfc1f0fe0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x00000000,
0xfd4003e0,
0xf81f0ffe,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xf84107fe,
0x910003ff,
0xfd0003e0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x00000000,
0xfd400be0,
0xfd4003e1,
0xf81f0ffe,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xf84107fe,
0x910043ff,
0xfd0003e0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x00000000,
0xfd4013e0,
0xfd400be1,
0xfd4003e2,
0xf81f0ffe,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xf84107fe,
0x910083ff,
0xfd0003e0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x00000000,
0xfd401be0,
0xfd4013e1,
0xfd400be2,
0xfd4003e3,
0xf81f0ffe,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xf84107fe,
0x9100c3ff,
0xfd0003e0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x00000000,
0xfd4023e0,
0xfd401be1,
0xfd4013e2,
0xfd400be3,
0xfd4003e4,
0xf81f0ffe,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xf84107fe,
0x910103ff,
0xfd0003e0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x00000000,
0xfd402be0,
0xfd4023e1,
0xfd401be2,
0xfd4013e3,
0xfd400be4,
0xfd4003e5,
0xf81f0ffe,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xf84107fe,
0x910143ff,
0xfd0003e0,
0x00000000,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x00000000,
0xfd4033e0,
0xfd402be1,
0xfd4023e2,
0xfd401be3,
0xfd4013e4,
0xfd400be5,
0xfd4003e6,
0xf81f0ffe,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xf84107fe,
0x910183ff,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e612000,
0x1a9f57e0,
0x1e630000,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e612000,
0x1a9f87e0,
0x1e630000,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e612000,
0x1a9fd7e0,
0x1e630000,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e612000,
0x1a9fb7e0,
0x1e630000,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e612000,
0x1a9f17e0,
0x1e630000,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e612000,
0x1a9f07e0,
0x1e630000,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e602008,
0x1a9f07e0,
0x1e602028,
0x1a9f07e1,
0x0a010000,
0x1e630000,
0xfd0003e0,
0x00000000,
0xfc4107e1,
0xfd4003e0,
0x1e602008,
0x1a9f07e0,
0x1e602028,
0x1a9f07e1,
0x2a010000,
0x1e630000,
0xfd0003e0,
0x00000000,
0xfd4003e0,
0x1e602018,
0x1a9f17e0,
0x1e630000,
0xfd0003e0,
0x00000000
};

//|.section code
#define DASM_SECTION_CODE	0
#define DASM_MAXSECTION		1
//|.globals GLOB_
enum {
  GLOB__MAX
};

static void emit(Dst_DECL, uint64_t* op) {
  //| .macro movq, Xn, imm
  //|   movz    Xn, #(imm)&0xffff
  //|   movk    Xn, #(imm>>16)&0xffff, lsl #16
  //|   movk    Xn, #(imm>>32)&0xffff, lsl #32
  //|   movk    Xn, #(imm>>48)&0xffff, lsl #48
  //| .endmacro
  //| .macro callop_push
  //|   str     x30, [sp, #-16]!
  //|   movq    x1, *op
  //|   blr     x1
  //|   ldr     x30, [sp], #16
  //|   str     d0, [sp, #-16]!
  //| .endmacro
  //| .macro callop_store, adj
  //|   str     x30, [sp, #-16]!
  //|   movq    x1, *op
  //|   blr     x1
  //|   ldr     x30, [sp], #16
  //|   add     sp, sp, #(16 * adj)
  //|   str     d0, [sp]
  //| .endmacro
  //| .macro loadargs1
  //|   ldr     d0, [sp]
  //| .endmacro
  //| .macro loadargs2
  //|   ldr     d0, [sp, #(16 * 1)]
  //|   ldr     d1, [sp, #(16 * 0)]
  //| .endmacro
  //| .macro loadargs3
  //|   ldr     d0, [sp, #(16 * 2)]
  //|   ldr     d1, [sp, #(16 * 1)]
  //|   ldr     d2, [sp, #(16 * 0)]
  //| .endmacro
  //| .macro loadargs4
  //|   ldr     d0, [sp, #(16 * 3)]
  //|   ldr     d1, [sp, #(16 * 2)]
  //|   ldr     d2, [sp, #(16 * 1)]
  //|   ldr     d3, [sp, #(16 * 0)]
  //| .endmacro
  //| .macro loadargs5
  //|   ldr     d0, [sp, #(16 * 4)]
  //|   ldr     d1, [sp, #(16 * 3)]
  //|   ldr     d2, [sp, #(16 * 2)]
  //|   ldr     d3, [sp, #(16 * 1)]
  //|   ldr     d4, [sp, #(16 * 0)]
  //| .endmacro
  //| .macro loadargs6
  //|   ldr     d0, [sp, #(16 * 5)]
  //|   ldr     d1, [sp, #(16 * 4)]
  //|   ldr     d2, [sp, #(16 * 3)]
  //|   ldr     d3, [sp, #(16 * 2)]
  //|   ldr     d4, [sp, #(16 * 1)]
  //|   ldr     d5, [sp, #(16 * 0)]
  //| .endmacro
  //| .macro loadargs7
  //|   ldr     d0, [sp, #(16 * 6)]
  //|   ldr     d1, [sp, #(16 * 5)]
  //|   ldr     d2, [sp, #(16 * 4)]
  //|   ldr     d3, [sp, #(16 * 3)]
  //|   ldr     d4, [sp, #(16 * 2)]
  //|   ldr     d5, [sp, #(16 * 1)]
  //|   ldr     d6, [sp, #(16 * 0)]
  //| .endmacro
  //| .macro logic, op
  //|   ldr     d1, [sp], #16
  //|   ldr     d0, [sp]
  //|   fcmp    d0, d1
  //|   cset    w0, op
  //|   ucvtf   d0, w0
  //|   str     d0, [sp]
  //| .endmacro
  //| .macro arithm, op
  //|   ldr     d1, [sp], #16
  //|   ldr     d0, [sp]
  //|   op      d0, d0, d1
  //|   str     d0, [sp]
  //| .endmacro
  while (1) {
    switch (*op) {
      case OP_pos:
        //| .nop
        break;
      case OP_neg:
        //| ldr     d0, [sp]
        //| fneg    d0, d0
        //| str     d0, [sp]
        dasm_put(Dst, 0);
        break;
      case OP_add:
        //| arithm  fadd
        dasm_put(Dst, 4);
        break;
      case OP_sub:
        //| arithm  fsub
        dasm_put(Dst, 9);
        break;
      case OP_mul:
        //| arithm  fmul
        dasm_put(Dst, 14);
        break;
      case OP_div:
        //| arithm  fdiv
        dasm_put(Dst, 19);
        break;
      case OP_var:
        op++;
        //| movq    x1, *op
        //| ldr     x0, [x1]
        //| fmov    d0, x0
        //| str     d0, [sp, #-16]!
        dasm_put(Dst, 24, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_con:
        op++;
        //| movq    x0, *op
        //| fmov    d0, x0
        //| str     d0, [sp, #-16]!
        dasm_put(Dst, 36, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_ret:
        //| ldr     d0, [sp], #16
        //| ret
        dasm_put(Dst, 47);
        return;
      case OP_clo0:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 50, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun0:
        op++;
        //| callop_push
        dasm_put(Dst, 59, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo1:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 72, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun1:
        op++;
        //| loadargs1
        //| callop_store 0
        dasm_put(Dst, 81, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo2:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 96, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun2:
        op++;
        //| loadargs2
        //| callop_store 1
        dasm_put(Dst, 105, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo3:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 121, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun3:
        op++;
        //| loadargs3
        //| callop_store 2
        dasm_put(Dst, 130, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo4:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 147, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun4:
        op++;
        //| loadargs4
        //| callop_store 3
        dasm_put(Dst, 156, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo5:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 174, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun5:
        op++;
        //| loadargs5
        //| callop_store 4
        dasm_put(Dst, 183, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo6:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 202, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun6:
        op++;
        //| loadargs6
        //| callop_store 5
        dasm_put(Dst, 211, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo7:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 231, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun7:
        op++;
        //| loadargs7
        //| callop_store 6
        dasm_put(Dst, 240, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;

      case OP_lt:
        //| logic   mi
        dasm_put(Dst, 261);
        break;
      case OP_le:
        //| logic   ls
        dasm_put(Dst, 268);
        break;
      case OP_gt:
        //| logic   gt
        dasm_put(Dst, 275);
        break;
      case OP_ge:
        //| logic   ge
        dasm_put(Dst, 282);
        break;
      case OP_eq:
        //| logic   eq
        dasm_put(Dst, 289);
        break;
      case OP_neq:
        //| logic   ne
        dasm_put(Dst, 296);
        break;
      case OP_and:
        //| ldr     d1, [sp], #16
        //| ldr     d0, [sp]
        //| fcmp    d0, #0.0
        //| cset    w0, ne
        //| fcmp    d1, #0.0
        //| cset    w1, ne
        //| and     w0, w0, w1
        //| ucvtf   d0, w0
        //| str     d0, [sp]
        dasm_put(Dst, 303);
        break;
      case OP_or:
        //| ldr     d1, [sp], #16
        //| ldr     d0, [sp]
        //| fcmp    d0, #0.0
        //| cset    w0, ne
        //| fcmp    d1, #0.0
        //| cset    w1, ne
        //| orr     w0, w0, w1
        //| ucvtf   d0, w0
        //| str     d0, [sp]
        dasm_put(Dst, 313);
        break;
      case OP_not:
        //| ldr     d0, [sp]
        //| fcmpe   d0, #0.0
        //| cset    w0, eq
        //| ucvtf   d0, w0
        //| str     d0, [sp]
        dasm_put(Dst, 323);
        break;

      default:
        assert(0);
        __builtin_unreachable();
    }
    
    ++op;
  }
}
