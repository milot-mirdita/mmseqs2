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
static const unsigned int ej_compile_actionlist[357] = {
0xfd400be0,
0x1e614000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e602820,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e603820,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e600820,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e601820,
0xfd000be0,
0x00000000,
0xd10043ff,
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
0xfd000be0,
0x00000000,
0xd10043ff,
0xd2800000,
0x000a0205,
0xf2a00000,
0x000a0205,
0xf2c00000,
0x000a0205,
0xf2e00000,
0x000a0205,
0x9e670000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
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
0xa9be7bfd,
0x910003fd,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xa8c27bfd,
0xd10043ff,
0xfd000be0,
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
0xa9be7bfd,
0x910003fd,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xa8c27bfd,
0x00000000,
0xfd000be0,
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
0xa9be7bfd,
0x910003fd,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xa8c27bfd,
0x910043ff,
0xfd000be0,
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
0xa9be7bfd,
0x910003fd,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xa8c27bfd,
0x910083ff,
0xfd000be0,
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
0xa9be7bfd,
0x910003fd,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xa8c27bfd,
0x9100c3ff,
0xfd000be0,
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
0xa9be7bfd,
0x910003fd,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xa8c27bfd,
0x910103ff,
0xfd000be0,
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
0xa9be7bfd,
0x910003fd,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xa8c27bfd,
0x910143ff,
0xfd000be0,
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
0xfd403be0,
0xfd4033e1,
0xfd402be2,
0xfd4023e3,
0xfd401be4,
0xfd4013e5,
0xfd400be6,
0xa9be7bfd,
0x910003fd,
0xd2800001,
0x000a0205,
0xf2a00001,
0x000a0205,
0xf2c00001,
0x000a0205,
0xf2e00001,
0x000a0205,
0xd63f0020,
0xa8c27bfd,
0x910183ff,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e602030,
0x1a9f57e0,
0x1e630000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e602030,
0x1a9f87e0,
0x1e630000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e602030,
0x1a9fd7e0,
0x1e630000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e602030,
0x1a9fb7e0,
0x1e630000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e602030,
0x1a9f17e0,
0x1e630000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0xfd400be1,
0x1e602030,
0x1a9f07e0,
0x1e630000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0x52800000,
0x1e602008,
0x54000000,
0x00050814,
0xfd400be0,
0x1e602008,
0x1a9f07e0,
0x00060014,
0x1e630000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x910043ff,
0x52800020,
0x1e602008,
0x54000001,
0x00050815,
0xfd400be0,
0x1e602008,
0x1a9f07e0,
0x00060015,
0x1e630000,
0xfd000be0,
0x00000000,
0xfd400be0,
0x1e602018,
0x1a9f17e0,
0x1e630000,
0xfd000be0,
0x00000000
};

//|.section code
#define DASM_SECTION_CODE	0
#define DASM_MAXSECTION		1
//|.globals GLOB_
enum {
  GLOB_endand,
  GLOB_endor,
  GLOB__MAX
};

static void emit(Dst_DECL, uint64_t* op) {
  //| .macro movq, Xn, imm
  //|   movz    Xn, #(imm)&0xffff
  //|   movk    Xn, #(imm>>16)&0xffff, lsl #16
  //|   movk    Xn, #(imm>>32)&0xffff, lsl #32
  //|   movk    Xn, #(imm>>48)&0xffff, lsl #48
  //| .endmacro
  //| .macro logic, op
  //|   ldr     d0, [sp, #16]
  //|   add     sp, sp, #16
  //|   ldr     d1, [sp, #16]
  //|   fcmpe   d1, d0
  //|   cset    w0, op
  //|   ucvtf   d0, w0
  //|   str     d0, [sp, #16]
  //| .endmacro
  while (1) {
    switch (*op) {
      case OP_pos:
        //| .nop
        break;
      case OP_neg:
        //| ldr     d0, [sp, #16]
        //| fneg    d0, d0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 0);
        break;
      case OP_add:
        //| ldr     d0, [sp, #16]
        //| add     sp, sp, #16
        //| ldr     d1, [sp, #16]
        //| fadd    d0, d1, d0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 4);
        break;
      case OP_sub:
        //| ldr     d0, [sp, #16]
        //| add     sp, sp, #16
        //| ldr     d1, [sp, #16]
        //| fsub    d0, d1, d0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 10);
        break;
      case OP_mul:
        //| ldr     d0, [sp, #16]
        //| add     sp, sp, #16
        //| ldr     d1, [sp, #16]
        //| fmul    d0, d1, d0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 16);
        break;
      case OP_div:
        //| ldr     d0, [sp, #16]
        //| add     sp, sp, #16
        //| ldr     d1, [sp, #16]
        //| fdiv    d0, d1, d0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 22);
        break;
      case OP_var:
        op++;
        //| sub     sp, sp, #16
        //| movq    x1, *op
        //| ldr     x0, [x1]
        //| fmov    d0, x0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 28, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_con:
        op++;
        //| sub     sp, sp, #16
        //| movq    x0, *op
        //| fmov    d0, x0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 41, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_ret:
        //| ldr     d0, [sp, #16]
        //| add     sp, sp, #16
        //| ret
        dasm_put(Dst, 53);
        return;
      case OP_clo0:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 57, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun0:
        op++;
        //| stp     x29, x30, [sp, #-32]!
        //| mov     x29, sp
        //| movq    x1, *op
        //| blr     x1
        //| ldp     x29, x30, [sp], #32
        //| sub     sp, sp, #16
        //| str     d0, [sp, #16]
        dasm_put(Dst, 66, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo1:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 81, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun1:
        op++;
        //| ldr     d0, [sp, #(16 * 1)]
        //| stp     x29, x30, [sp, #-32]!
        //| mov     x29, sp
        //| movq    x1, *op
        //| blr     x1
        //| ldp     x29, x30, [sp], #32
        dasm_put(Dst, 90, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        //| add      sp, sp, #(16 * 0)
        //| str     d0, [sp, #16]
        dasm_put(Dst, 104);
        break;
      case OP_clo2:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 106, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun2:
        op++;
        //| ldr     d0, [sp, #(16 * 2)]
        //| ldr     d1, [sp, #(16 * 1)]
        //| stp     x29, x30, [sp, #-32]!
        //| mov     x29, sp
        //| movq    x1, *op
        //| blr     x1
        //| ldp     x29, x30, [sp], #32
        //| add     sp, sp, #(16 * 1)
        //| str     d0, [sp, #16]
        dasm_put(Dst, 115, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo3:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 132, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun3:
        op++;
        //| ldr     d0, [sp, #(16 * 3)]
        //| ldr     d1, [sp, #(16 * 2)]
        //| ldr     d2, [sp, #(16 * 1)]
        //| stp     x29, x30, [sp, #-32]!
        //| mov     x29, sp
        //| movq    x1, *op
        //| blr     x1
        //| ldp     x29, x30, [sp], #32
        //| add     sp, sp, #(16 * 2)
        //| str     d0, [sp, #16]
        dasm_put(Dst, 141, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo4:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 159, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun4:
        op++;
        //| ldr     d0, [sp, #(16 * 4)]
        //| ldr     d1, [sp, #(16 * 3)]
        //| ldr     d2, [sp, #(16 * 2)]
        //| ldr     d3, [sp, #(16 * 1)]
        //| stp     x29, x30, [sp, #-32]!
        //| mov     x29, sp
        //| movq    x1, *op
        //| blr     x1
        //| ldp     x29, x30, [sp], #32
        //| add     sp, sp, #(16 * 3)
        //| str     d0, [sp, #16]
        dasm_put(Dst, 168, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo5:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 187, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun5:
        op++;
        //| ldr     d0, [sp, #(16 * 5)]
        //| ldr     d1, [sp, #(16 * 4)]
        //| ldr     d2, [sp, #(16 * 3)]
        //| ldr     d3, [sp, #(16 * 2)]
        //| ldr     d4, [sp, #(16 * 1)]
        //| stp     x29, x30, [sp, #-32]!
        //| mov     x29, sp
        //| movq    x1, *op
        //| blr     x1
        //| ldp     x29, x30, [sp], #32
        //| add     sp, sp, #(16 * 4)
        //| str     d0, [sp, #16]
        dasm_put(Dst, 196, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo6:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 216, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun6:
        op++;
        //| ldr     d0, [sp, #(16 * 6)]
        //| ldr     d1, [sp, #(16 * 5)]
        //| ldr     d2, [sp, #(16 * 4)]
        //| ldr     d3, [sp, #(16 * 3)]
        //| ldr     d4, [sp, #(16 * 2)]
        //| ldr     d5, [sp, #(16 * 1)]
        //| stp     x29, x30, [sp, #-32]!
        //| mov     x29, sp
        //| movq    x1, *op
        //| blr     x1
        //| ldp     x29, x30, [sp], #32
        //| add     sp, sp, #(16 * 5)
        //| str     d0, [sp, #16]
        dasm_put(Dst, 225, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;
      case OP_clo7:
        op++;
        //| movq    x0, *op
        dasm_put(Dst, 246, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
      case OP_fun7:
        op++;
        //| ldr     d0, [sp, #(16 * 7)]
        //| ldr     d1, [sp, #(16 * 6)]
        //| ldr     d2, [sp, #(16 * 5)]
        //| ldr     d3, [sp, #(16 * 4)]
        //| ldr     d4, [sp, #(16 * 3)]
        //| ldr     d5, [sp, #(16 * 2)]
        //| ldr     d6, [sp, #(16 * 1)]
        //| stp     x29, x30, [sp, #-32]!
        //| mov     x29, sp
        //| movq    x1, *op
        //| blr     x1
        //| ldp     x29, x30, [sp], #32
        //| add     sp, sp, #(16 * 6)
        //| str     d0, [sp, #16]
        dasm_put(Dst, 255, (*op)&0xffff, (*op>>16)&0xffff, (*op>>32)&0xffff, (*op>>48)&0xffff);
        break;

      case OP_lt:
        //| logic   mi
        dasm_put(Dst, 277);
        break;
      case OP_le:
        //| logic   ls
        dasm_put(Dst, 285);
        break;
      case OP_gt:
        //| logic   gt
        dasm_put(Dst, 293);
        break;
      case OP_ge:
        //| logic   ge
        dasm_put(Dst, 301);
        break;
      case OP_eq:
        //| logic   eq
        dasm_put(Dst, 309);
        break;
      case OP_neq:
        //| logic   ne
        dasm_put(Dst, 317);
        break;
      case OP_and:
        //| ldr     d0, [sp, #16]
        //| add     sp, sp, #16
        //| mov     w0, #0
        //| fcmp    d0, #0.0
        //| beq     ->endand
        //| ldr     d0, [sp, #16]
        //| fcmp    d0, #0.0
        //| cset    w0, ne
        //|->endand:
        //| ucvtf   d0, w0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 325);
        break;
      case OP_or:
        //| ldr     d0, [sp, #16]
        //| add     sp, sp, #16
        //| mov     w0, #1
        //| fcmp    d0, #0.0
        //| bne     ->endor
        //| ldr     d0, [sp, #16]
        //| fcmp    d0, #0.0
        //| cset    w0, ne
        //|->endor:
        //| ucvtf   d0, w0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 338);
        break;
      case OP_not:
        //| ldr     d0, [sp, #16]
        //| fcmpe   d0, #0.0
        //| cset    w0, eq
        //| ucvtf   d0, w0
        //| str     d0, [sp, #16]
        dasm_put(Dst, 351);
        break;

      default:
        assert(0);
        __builtin_unreachable();
    }
    
    ++op;
  }
}
