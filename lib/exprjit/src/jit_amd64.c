/*
** This file has been pre-processed with DynASM.
** https://luajit.org/dynasm.html
** DynASM version 1.5.0, DynASM x64 version 1.5.0
** DO NOT EDIT! The original file is in "jit_amd64.dasc".
*/

#include "dynasm/dasm_x86.h"

//|.arch x64
#if DASM_VERSION != 10500
#error "Version mismatch between DynASM and included encoding engine"
#endif
//|.actionlist ej_compile_actionlist
static const unsigned char ej_compile_actionlist[913] = {
  85,87,86,83,65,84,65,85,65,86,65,87,72,137,229,255,88,102,72,15,110,200,15,
  87,192,252,242,15,92,193,102,72,15,126,192,80,255,88,102,72,15,110,200,88,
  102,72,15,110,192,252,242,15,88,193,102,72,15,126,192,80,255,88,102,72,15,
  110,200,88,102,72,15,110,192,252,242,15,92,193,102,72,15,126,192,80,255,88,
  102,72,15,110,200,88,102,72,15,110,192,252,242,15,89,193,102,72,15,126,192,
  80,255,88,102,72,15,110,200,88,102,72,15,110,192,252,242,15,94,193,102,72,
  15,126,192,80,255,72,184,237,237,72,139,0,80,255,72,184,237,237,80,255,88,
  102,72,15,110,192,72,137,252,236,65,95,65,94,65,93,65,92,91,94,95,93,195,
  255,72,191,237,237,255,73,137,228,72,131,228,252,240,72,190,237,237,252,255,
  214,76,137,228,102,72,15,126,192,80,255,88,102,72,15,110,192,73,137,228,72,
  131,228,252,240,72,190,237,237,252,255,214,76,137,228,102,72,15,126,192,80,
  255,88,102,72,15,110,200,88,102,72,15,110,192,73,137,228,72,131,228,252,240,
  72,190,237,237,252,255,214,76,137,228,102,72,15,126,192,80,255,88,102,72,
  15,110,208,88,102,72,15,110,200,88,102,72,15,110,192,73,137,228,72,131,228,
  252,240,72,190,237,237,252,255,214,76,137,228,102,72,15,126,192,80,255,88,
  102,72,15,110,216,88,102,72,15,110,208,88,102,72,15,110,200,88,102,72,15,
  110,192,73,137,228,72,131,228,252,240,72,190,237,237,252,255,214,76,137,228,
  102,72,15,126,192,80,255,88,102,72,15,110,224,88,102,72,15,110,216,88,102,
  72,15,110,208,88,102,72,15,110,200,88,102,72,15,110,192,73,137,228,72,131,
  228,252,240,72,190,237,237,252,255,214,76,137,228,102,72,15,126,192,80,255,
  88,102,72,15,110,232,88,102,72,15,110,224,88,102,72,15,110,216,88,102,72,
  15,110,208,88,102,72,15,110,200,88,102,72,15,110,192,73,137,228,72,131,228,
  252,240,72,190,237,237,252,255,214,76,137,228,102,72,15,126,192,80,255,88,
  102,72,15,110,252,240,88,102,72,15,110,232,88,102,72,15,110,224,88,102,72,
  15,110,216,88,102,72,15,110,208,88,102,72,15,110,200,88,102,72,15,110,192,
  73,137,228,72,131,228,252,240,72,190,237,237,252,255,214,76,137,228,102,72,
  15,126,192,80,255,88,102,72,15,110,192,88,102,72,15,110,200,252,242,15,194,
  200,1,72,184,237,237,102,72,15,110,192,102,15,84,193,102,72,15,126,192,80,
  255,88,102,72,15,110,192,88,102,72,15,110,200,252,242,15,194,200,2,72,184,
  237,237,102,72,15,110,192,102,15,84,193,102,72,15,126,192,80,255,88,102,72,
  15,110,192,88,102,72,15,110,200,252,242,15,194,193,1,72,184,237,237,102,72,
  15,110,200,102,15,84,193,102,72,15,126,192,80,255,88,102,72,15,110,192,88,
  102,72,15,110,200,252,242,15,194,193,2,72,184,237,237,102,72,15,110,200,102,
  15,84,193,102,72,15,126,192,80,255,88,102,72,15,110,192,88,102,72,15,110,
  200,252,242,15,194,193,0,72,184,237,237,102,72,15,110,200,102,15,84,193,102,
  72,15,126,192,80,255,88,102,72,15,110,192,88,102,72,15,110,200,252,242,15,
  194,193,4,72,184,237,237,102,72,15,110,200,102,15,84,193,102,72,15,126,192,
  80,255,88,102,72,15,110,192,88,102,72,15,110,200,102,15,87,210,252,242,15,
  194,194,4,252,242,15,194,202,4,102,15,84,193,72,184,237,237,102,72,15,110,
  200,102,15,84,193,102,72,15,126,192,80,255,88,102,72,15,110,192,88,102,72,
  15,110,200,102,15,87,210,252,242,15,194,194,4,252,242,15,194,202,4,102,15,
  86,193,72,184,237,237,102,72,15,110,200,102,15,84,193,102,72,15,126,192,80,
  255,88,102,72,15,110,192,15,87,201,252,242,15,194,193,0,72,184,237,237,102,
  72,15,110,200,102,15,84,193,102,72,15,126,192,80,255
};

//|.section code
#define DASM_SECTION_CODE	0
#define DASM_MAXSECTION		1
//|.globals GLOB_
enum {
  GLOB__MAX
};

static void emit(Dst_DECL, uint64_t* op) {
  //| .macro logic, imm, first, second
  //|   pop     rax
  //|   movd    xmm0, rax
  //|   pop     rax
  //|   movd    xmm1, rax
  //|   cmpsd   first, second, imm
  //|   mov64   rax, 0x3ff0000000000000
  //|   movd    second, rax
  //|   andpd   xmm0, xmm1
  //|   movd    rax, xmm0
  //|   push    rax
  //| .endmacro
  //| .macro arith, ins
  //|   pop     rax
  //|   movd    xmm1, rax
  //|   pop     rax
  //|   movd    xmm0, rax
  //|   ins   xmm0, xmm1
  //|   movd    rax, xmm0
  //|   push    rax
  //| .endmacro
  //| .macro saveregs
  //|   push rbp; push rdi; push rsi; push rbx; push r12; push r13; push r14; push r15
  //|   mov      rbp, rsp
  //| .endmacro
  //| .macro restoreregs
  //|   mov      rsp, rbp
  //|   pop r15; pop r14; pop r13; pop r12; pop rbx; pop rsi; pop rdi; pop rbp
  //| .endmacro
  //| .macro loadargs1
  //|   pop      rax
  //|   movd     xmm0, rax
  //| .endmacro
  //| .macro loadargs2
  //|   pop      rax
  //|   movd     xmm1, rax
  //|   pop      rax
  //|   movd     xmm0, rax
  //| .endmacro
  //| .macro loadargs3
  //|   pop      rax
  //|   movd     xmm2, rax
  //|   pop      rax
  //|   movd     xmm1, rax
  //|   pop      rax
  //|   movd     xmm0, rax
  //| .endmacro
  //| .macro loadargs4
  //|   pop      rax
  //|   movd     xmm3, rax
  //|   pop      rax
  //|   movd     xmm2, rax
  //|   pop      rax
  //|   movd     xmm1, rax
  //|   pop      rax
  //|   movd     xmm0, rax
  //| .endmacro
  //| .macro loadargs5
  //|   pop      rax
  //|   movd     xmm4, rax
  //|   pop      rax
  //|   movd     xmm3, rax
  //|   pop      rax
  //|   movd     xmm2, rax
  //|   pop      rax
  //|   movd     xmm1, rax
  //|   pop      rax
  //|   movd     xmm0, rax
  //| .endmacro
  //| .macro loadargs6
  //|   pop      rax
  //|   movd     xmm5, rax
  //|   pop      rax
  //|   movd     xmm4, rax
  //|   pop      rax
  //|   movd     xmm3, rax
  //|   pop      rax
  //|   movd     xmm2, rax
  //|   pop      rax
  //|   movd     xmm1, rax
  //|   pop      rax
  //|   movd     xmm0, rax
  //| .endmacro
  //| .macro loadargs7
  //|   pop      rax
  //|   movd     xmm6, rax
  //|   pop      rax
  //|   movd     xmm5, rax
  //|   pop      rax
  //|   movd     xmm4, rax
  //|   pop      rax
  //|   movd     xmm3, rax
  //|   pop      rax
  //|   movd     xmm2, rax
  //|   pop      rax
  //|   movd     xmm1, rax
  //|   pop      rax
  //|   movd     xmm0, rax
  //| .endmacro
  //|
  //|.if WIN64
  //| .macro loadctx
  //|   mov64    rcx, *op
  //| .endmacro
  //| .macro callop_win0
  //|   mov      r12, rsp
  //|   sub      rsp, 40
  //|   mov64    rax, *op
  //|   call     rax
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callop_win1
  //|   mov      r12, rsp
  //|   sub      rsp, 32
  //|   mov64    rax, *op
  //|   call     rax
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callop_win2
  //|   mov      r12, rsp
  //|   sub      rsp, 40
  //|   mov64    rax, *op
  //|   call     rax
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callop_win3
  //|   mov      r12, rsp
  //|   sub      rsp, 32
  //|   mov64    rax, *op
  //|   call     rax
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callop_win4
  //|   mov      r12, rsp
  //|   sub      rsp, 40
  //|   mov64    rax, *op
  //|   call     rax
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callop_win5
  //|   mov      r12, rsp
  //|   sub      rsp, 48
  //|   movsd    [rsp+32], xmm4
  //|   mov64    rax, *op
  //|   call     rax
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callop_win6
  //|   mov      r12, rsp
  //|   sub      rsp, 56
  //|   movsd    [rsp+32], xmm4
  //|   movsd    [rsp+40], xmm5
  //|   mov64    rax, *op
  //|   call     rax
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callop_win7
  //|   mov      r12, rsp
  //|   sub      rsp, 64
  //|   movsd    [rsp+32], xmm4
  //|   movsd    [rsp+40], xmm5
  //|   movsd    [rsp+48], xmm6
  //|   mov64    rax, *op
  //|   call     rax
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callfun0
  //|   callop_win0
  //| .endmacro
  //| .macro callfun1
  //|   loadargs1
  //|   callop_win1
  //| .endmacro
  //| .macro callfun2
  //|   loadargs2
  //|   callop_win2
  //| .endmacro
  //| .macro callfun3
  //|   loadargs3
  //|   callop_win3
  //| .endmacro
  //| .macro callfun4
  //|   loadargs4
  //|   callop_win4
  //| .endmacro
  //| .macro callfun5
  //|   loadargs5
  //|   callop_win5
  //| .endmacro
  //| .macro callfun6
  //|   loadargs6
  //|   callop_win6
  //| .endmacro
  //| .macro callfun7
  //|   loadargs7
  //|   callop_win7
  //| .endmacro
  //|.else
  //| .macro loadctx
  //|   mov64    rdi, *op
  //| .endmacro
  //| .macro callop
  //|   mov      r12, rsp
  //|   and      rsp, -16
  //|   mov64    rsi, *op
  //|   call     rsi
  //|   mov      rsp, r12
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
  //| .macro callfun0
  //|   callop
  //| .endmacro
  //| .macro callfun1
  //|   loadargs1
  //|   callop
  //| .endmacro
  //| .macro callfun2
  //|   loadargs2
  //|   callop
  //| .endmacro
  //| .macro callfun3
  //|   loadargs3
  //|   callop
  //| .endmacro
  //| .macro callfun4
  //|   loadargs4
  //|   callop
  //| .endmacro
  //| .macro callfun5
  //|   loadargs5
  //|   callop
  //| .endmacro
  //| .macro callfun6
  //|   loadargs6
  //|   callop
  //| .endmacro
  //| .macro callfun7
  //|   loadargs7
  //|   callop
  //| .endmacro
  //|.endif
  //| saveregs
  dasm_put(Dst, 0);
  while (1) {
    switch (*op) {
      case OP_pos:
        //| .nop
        break;
      case OP_neg:
        //| pop     rax
        //| movd    xmm1, rax
        //| xorps   xmm0, xmm0
        //| subsd   xmm0, xmm1
        //| movd    rax, xmm0
        //| push    rax
        dasm_put(Dst, 16);
        break;
      case OP_add:
        //| arith   addsd
        dasm_put(Dst, 37);
        break;
      case OP_sub:
        //| arith   subsd
        dasm_put(Dst, 61);
        break;
      case OP_mul:
        //| arith   mulsd
        dasm_put(Dst, 85);
        break;
      case OP_div:
        //| arith   divsd
        dasm_put(Dst, 109);
        break;
      case OP_var:
        op++;
        //| mov64   rax, *op
        //| mov     rax, [rax]
        //| push    rax
        dasm_put(Dst, 133, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_con:
        op++;
        //| mov64    rax, *op
        //| push     rax
        dasm_put(Dst, 142, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_ret:
        //| pop      rax
        //| movd     xmm0, rax
        //| restoreregs
        //| ret
        dasm_put(Dst, 148);
        return;

      case OP_clo0:
        op++;
        //| loadctx
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun0:
        op++;
        //| callfun0
        dasm_put(Dst, 177, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo1:
        op++;
        //| loadctx
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun1:
        op++;
        //| callfun1
        dasm_put(Dst, 202, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo2:
        op++;
        //| loadctx
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun2:
        op++;
        //| callfun2
        dasm_put(Dst, 233, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo3:
        op++;
        //| loadctx
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun3:
        op++;
        //| callfun3
        dasm_put(Dst, 270, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo4:
        op++;
        //| loadctx
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun4:
        op++;
        //| callfun4
        dasm_put(Dst, 313, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo5:
        op++;
        //| loadctx
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun5:
        op++;
        //| callfun5
        dasm_put(Dst, 362, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo6:
        op++;
        //| loadctx
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun6:
        op++;
        //| callfun6
        dasm_put(Dst, 417, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo7:
        op++;
        //| loadctx
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun7:
        op++;
        //| callfun7
        dasm_put(Dst, 478, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;

      case OP_lt:
        //| logic    1, xmm1, xmm0
        dasm_put(Dst, 546, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_le:
        //| logic    2, xmm1, xmm0
        dasm_put(Dst, 584, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_gt:
        //| logic    1, xmm0, xmm1
        dasm_put(Dst, 622, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_ge:
        //| logic    2, xmm0, xmm1
        dasm_put(Dst, 660, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_eq:
        //| logic    0, xmm0, xmm1
        dasm_put(Dst, 698, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_neq:
        //| logic    4, xmm0, xmm1
        dasm_put(Dst, 736, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_and:
        //| pop      rax
        //| movd     xmm0, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| xorpd    xmm2, xmm2
        //| cmpsd    xmm0, xmm2, 4
        //| cmpsd    xmm1, xmm2, 4
        //| andpd    xmm0, xmm1
        //| mov64    rax, 0x3ff0000000000000
        //| movd     xmm1, rax
        //| andpd    xmm0, xmm1
        //| movd     rax, xmm0
        //| push     rax
        dasm_put(Dst, 774, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_or:
        //| pop      rax
        //| movd     xmm0, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| xorpd    xmm2, xmm2
        //| cmpsd    xmm0, xmm2, 4
        //| cmpsd    xmm1, xmm2, 4
        //| orpd     xmm0, xmm1
        //| mov64    rax, 0x3ff0000000000000
        //| movd     xmm1, rax
        //| andpd    xmm0, xmm1
        //| movd     rax, xmm0
        //| push     rax
        dasm_put(Dst, 826, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_not:
        //| pop      rax
        //| movd     xmm0, rax
        //| xorps    xmm1, xmm1
        //| cmpsd    xmm0, xmm1, 0
        //| mov64    rax, 0x3ff0000000000000
        //| movd     xmm1, rax
        //| andpd    xmm0, xmm1
        //| movd     rax, xmm0
        //| push     rax
        dasm_put(Dst, 878, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;

      default:
        assert(0);
        __builtin_unreachable();
    }

    ++op;
  }
}
