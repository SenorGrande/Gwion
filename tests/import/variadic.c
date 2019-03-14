#include "gwion_util.h"
#include "gwion_ast.h"
#include "oo.h"
#include "vm.h"
#include "env.h"
#include "type.h"
#include "instr.h"
#include "object.h"
#include "import.h"
#include "vararg.h"

static MFUN(m_test) {
  printf("%p\n", *(M_Object*)MEM(0));
}

static MFUN(m_variadic) {
  M_Object str_obj = *(M_Object*)MEM(SZ_INT);
  if(!str_obj)return;
  m_str str = STRING(str_obj);
  struct Vararg_* arg = *(struct Vararg_**)MEM(SZ_INT*2);

  while(arg->i < arg->s) {
    if(*str == 'i') {
      printf("%" INT_F "\n", *(m_int*)(arg->d + arg->o));
      arg->o += SZ_INT;
    } else if(*str == 'f') {
      printf("%f\n", *(m_float*)(arg->d + arg->o));
      arg->o += SZ_FLOAT;
    } else if(*str == 'o') {
      printf("%p\n", (void*)*(M_Object*)(arg->d + arg->o));
      arg->o += SZ_INT;
    }
    arg->i++;
    str++;
  }
  free_vararg(arg);
}

GWION_IMPORT(variadic test) {
  const Type t_variadic = gwi_mk_type(gwi, "Variadic", SZ_INT, t_object);
  CHECK_BB(gwi_class_ini(gwi, t_variadic, NULL, NULL))
  CHECK_BB(gwi_func_ini(gwi, "void", "member", m_variadic))
  CHECK_BB(gwi_func_arg(gwi, "string", "format"))
  CHECK_BB(gwi_func_end(gwi, ae_flag_variadic))
  CHECK_BB(gwi_func_ini(gwi, "void", "test", m_test))
  CHECK_BB(gwi_func_end(gwi, 0))
  CHECK_BB(gwi_class_end(gwi))
  return GW_OK;
}