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
static const unsigned char ej_compile_actionlist[1153] = {
  85,87,86,83,65,84,65,85,65,86,65,87,72,137,229,255,88,102,72,15,110,200,15,
  87,192,252,242,15,92,193,102,72,15,126,192,80,255,88,102,72,15,110,200,88,
  102,72,15,110,192,252,242,15,88,193,102,72,15,126,192,80,255,88,102,72,15,
  110,200,88,102,72,15,110,192,252,242,15,92,193,102,72,15,126,192,80,255,88,
  102,72,15,110,200,88,102,72,15,110,192,252,242,15,89,193,102,72,15,126,192,
  80,255,88,102,72,15,110,200,88,102,72,15,110,192,252,242,15,94,193,102,72,
  15,126,192,80,255,72,184,237,237,72,139,0,80,255,72,184,237,237,80,255,88,
  102,72,15,110,192,72,137,252,236,65,95,65,94,65,93,65,92,91,94,95,93,195,
  255,72,191,237,237,255,85,87,86,83,65,84,65,85,65,86,65,87,72,137,229,72,
  49,192,72,131,228,252,240,72,190,237,237,252,255,214,72,137,252,236,65,95,
  65,94,65,93,65,92,91,94,95,93,102,72,15,126,192,80,255,88,102,72,15,110,192,
  85,87,86,83,65,84,65,85,65,86,65,87,72,137,229,72,49,192,72,131,228,252,240,
  72,190,237,237,252,255,214,72,137,252,236,65,95,65,94,65,93,65,92,91,94,95,
  93,102,72,15,126,192,80,255,88,102,72,15,110,200,88,102,72,15,110,192,85,
  87,86,83,65,84,65,85,65,86,65,87,72,137,229,72,49,192,72,131,228,252,240,
  72,190,237,237,252,255,214,72,137,252,236,65,95,65,94,65,93,65,92,91,94,95,
  93,102,72,15,126,192,80,255,88,102,72,15,110,208,88,102,72,15,110,200,88,
  102,72,15,110,192,85,87,86,83,65,84,65,85,65,86,65,87,72,137,229,72,49,192,
  72,131,228,252,240,72,190,237,237,252,255,214,72,137,252,236,65,95,65,94,
  65,93,65,92,91,94,95,93,102,72,15,126,192,80,255,88,102,72,15,110,216,88,
  102,72,15,110,208,88,102,72,15,110,200,88,102,72,15,110,192,85,87,86,83,65,
  84,65,85,65,86,65,87,72,137,229,72,49,192,72,131,228,252,240,72,190,237,237,
  252,255,214,72,137,252,236,65,95,65,94,65,93,65,92,91,94,95,93,102,72,15,
  126,192,80,255,88,102,72,15,110,224,88,102,72,15,110,216,88,102,72,15,110,
  208,88,102,72,15,110,200,88,102,72,15,110,192,85,87,86,83,65,84,65,85,65,
  86,65,87,72,137,229,72,49,192,72,131,228,252,240,72,190,237,237,252,255,214,
  72,137,252,236,65,95,65,94,65,93,65,92,91,94,95,93,102,72,15,126,192,80,255,
  88,102,72,15,110,232,88,102,72,15,110,224,88,102,72,15,110,216,88,102,72,
  15,110,208,88,102,72,15,110,200,88,102,72,15,110,192,85,87,86,83,65,84,65,
  85,65,86,65,87,72,137,229,72,49,192,72,131,228,252,240,72,190,237,237,252,
  255,214,72,137,252,236,65,95,65,94,65,93,65,92,91,94,95,93,102,72,15,126,
  192,80,255,88,102,72,15,110,252,240,88,102,72,15,110,232,88,102,72,15,110,
  224,88,102,72,15,110,216,88,102,72,15,110,208,88,102,72,15,110,200,88,102,
  72,15,110,192,85,87,86,83,65,84,65,85,65,86,65,87,72,137,229,72,49,192,72,
  131,228,252,240,72,190,237,237,252,255,214,72,137,252,236,65,95,65,94,65,
  93,65,92,91,94,95,93,102,72,15,126,192,80,255,88,102,72,15,110,192,88,102,
  72,15,110,200,252,242,15,194,200,1,72,184,237,237,102,72,15,110,192,102,15,
  84,193,102,72,15,126,192,80,255,88,102,72,15,110,192,88,102,72,15,110,200,
  252,242,15,194,200,2,72,184,237,237,102,72,15,110,192,102,15,84,193,102,72,
  15,126,192,80,255,88,102,72,15,110,192,88,102,72,15,110,200,252,242,15,194,
  193,1,72,184,237,237,102,72,15,110,200,102,15,84,193,102,72,15,126,192,80,
  255,88,102,72,15,110,192,88,102,72,15,110,200,252,242,15,194,193,2,72,184,
  237,237,102,72,15,110,200,102,15,84,193,102,72,15,126,192,80,255,88,102,72,
  15,110,192,88,102,72,15,110,200,252,242,15,194,193,0,72,184,237,237,102,72,
  15,110,200,102,15,84,193,102,72,15,126,192,80,255,88,102,72,15,110,192,88,
  102,72,15,110,200,252,242,15,194,193,4,72,184,237,237,102,72,15,110,200,102,
  15,84,193,102,72,15,126,192,80,255,88,102,72,15,110,192,88,102,72,15,110,
  200,102,15,87,210,102,15,46,194,15,154,208,15,149,209,8,193,102,15,46,202,
  15,154,208,15,149,210,8,194,32,202,15,182,194,15,87,192,252,242,15,42,192,
  102,72,15,126,192,80,255,88,102,72,15,110,192,88,102,72,15,110,200,102,15,
  87,210,102,15,46,194,15,154,208,15,149,209,8,193,102,15,46,202,15,154,208,
  15,149,210,8,194,8,202,15,182,194,15,87,192,252,242,15,42,192,102,72,15,126,
  192,80,255,88,102,72,15,110,192,15,87,201,252,242,15,194,193,0,72,184,237,
  237,102,72,15,110,200,102,15,84,193,102,72,15,126,192,80,255
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
  //| .macro callop
  //|   saveregs
  //|   xor      rax, rax
  //|   and      rsp, -16
  //|   mov64    rsi, *op
  //|   call     rsi
  //|   restoreregs
  //|   movd     rax, xmm0
  //|   push     rax
  //| .endmacro
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
        //| mov64    rdi, *op
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun0:
        op++;
        //| callop
        dasm_put(Dst, 177, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo1:
        op++;
        //| mov64    rdi, *op
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun1:
        op++;
        //| pop      rax
        //| movd     xmm0, rax
        //| callop
        dasm_put(Dst, 230, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo2:
        op++;
        //| mov64    rdi, *op
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun2:
        op++;
        //| pop      rax
        //| movd     xmm1, rax
        //| pop      rax
        //| movd     xmm0, rax
        //| callop
        dasm_put(Dst, 289, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo3:
        op++;
        //| mov64    rdi, *op
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun3:
        op++;
        //| pop      rax
        //| movd     xmm2, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| pop      rax
        //| movd     xmm0, rax
        //| callop
        dasm_put(Dst, 354, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo4:
        op++;
        //| mov64    rdi, *op
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun4:
        op++;
        //| pop      rax
        //| movd     xmm3, rax
        //| pop      rax
        //| movd     xmm2, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| pop      rax
        //| movd     xmm0, rax
        //| callop
        dasm_put(Dst, 425, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo5:
        op++;
        //| mov64    rdi, *op
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun5:
        op++;
        //| pop      rax
        //| movd     xmm4, rax
        //| pop      rax
        //| movd     xmm3, rax
        //| pop      rax
        //| movd     xmm2, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| pop      rax
        //| movd     xmm0, rax
        //| callop
        dasm_put(Dst, 502, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo6:
        op++;
        //| mov64    rdi, *op
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun6:
        op++;
        //| pop      rax
        //| movd     xmm5, rax
        //| pop      rax
        //| movd     xmm4, rax
        //| pop      rax
        //| movd     xmm3, rax
        //| pop      rax
        //| movd     xmm2, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| pop      rax
        //| movd     xmm0, rax
        //| callop
        dasm_put(Dst, 585, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;
      case OP_clo7:
        op++;
        //| mov64    rdi, *op
        dasm_put(Dst, 172, (unsigned int)(*op), (unsigned int)((*op)>>32));
      case OP_fun7:
        op++;
        //| pop      rax
        //| movd     xmm6, rax
        //| pop      rax
        //| movd     xmm5, rax
        //| pop      rax
        //| movd     xmm4, rax
        //| pop      rax
        //| movd     xmm3, rax
        //| pop      rax
        //| movd     xmm2, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| pop      rax
        //| movd     xmm0, rax
        //| callop
        dasm_put(Dst, 674, (unsigned int)(*op), (unsigned int)((*op)>>32));
        break;

      case OP_lt:
        //| logic    1, xmm1, xmm0
        dasm_put(Dst, 770, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_le:
        //| logic    2, xmm1, xmm0
        dasm_put(Dst, 808, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_gt:
        //| logic    1, xmm0, xmm1
        dasm_put(Dst, 846, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_ge:
        //| logic    2, xmm0, xmm1
        dasm_put(Dst, 884, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_eq:
        //| logic    0, xmm0, xmm1
        dasm_put(Dst, 922, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_neq:
        //| logic    4, xmm0, xmm1
        dasm_put(Dst, 960, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;
      case OP_and:
        //| pop      rax
        //| movd     xmm0, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| xorpd    xmm2, xmm2
        //| ucomisd  xmm0, xmm2
        //| setp     al
        //| setne    cl
        //| or       cl, al
        //| ucomisd  xmm1, xmm2
        //| setp     al
        //| setne    dl
        //| or       dl, al
        //| and      dl, cl
        //| movzx    eax, dl
        //| xorps    xmm0, xmm0
        //| cvtsi2sd xmm0, eax
        //| movd     rax, xmm0
        //| push     rax
        dasm_put(Dst, 998);
        break;
      case OP_or:
        //| pop      rax
        //| movd     xmm0, rax
        //| pop      rax
        //| movd     xmm1, rax
        //| xorpd    xmm2, xmm2
        //| ucomisd  xmm0, xmm2
        //| setp     al
        //| setne    cl
        //| or       cl, al
        //| ucomisd  xmm1, xmm2
        //| setp     al
        //| setne    dl
        //| or       dl, al
        //| or       dl, cl
        //| movzx    eax, dl
        //| xorps    xmm0, xmm0
        //| cvtsi2sd xmm0, eax
        //| movd     rax, xmm0
        //| push     rax
        dasm_put(Dst, 1058);
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
        dasm_put(Dst, 1118, (unsigned int)(0x3ff0000000000000), (unsigned int)((0x3ff0000000000000)>>32));
        break;

      default:
        assert(0);
        __builtin_unreachable();
    }

    ++op;
  }
}
