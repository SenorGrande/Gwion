#include "gwion_util.h"
#include "gwion_ast.h"
#include "gwion_env.h"
#include "vm.h"
#include "object.h"
#include "instr.h"
#include "gwion.h"
#include "operator.h"
#include "import.h"
#include "gwi.h"

SFUN(coverage_int)     { *(m_uint*)RETURN    = 0; }
SFUN(coverage_float)   { *(m_float*)RETURN   = 0; }

GWION_IMPORT(coverage) {
  GWI_OB(gwi_class_ini(gwi, "Coverage", NULL))
  GWI_BB(gwi_func_ini(gwi, "int", "i"))
  GWI_BB(gwi_func_end(gwi, coverage_int, ae_flag_static))
  GWI_BB(gwi_func_ini(gwi, "float", "f"))
  GWI_BB(gwi_func_end(gwi, coverage_float, ae_flag_static))

  ALLOC_PTR(gwi->gwion->mp, i, m_uint, 5);
  GWI_BB(gwi_item_ini(gwi,"int", "s_i"))
  GWI_BB(gwi_item_end(gwi, ae_flag_static, i))

  ALLOC_PTR(gwi->gwion->mp, f, m_float, 2.1);
  GWI_BB(gwi_item_ini(gwi,"int", "s_f"))
  GWI_BB(gwi_item_end(gwi, ae_flag_static, (void*)f))

  ALLOC_PTR(gwi->gwion->mp, ci, m_uint, 5);
  GWI_BB(gwi_item_ini(gwi,"int", "sc_i"))
  GWI_BB(gwi_item_end(gwi, ae_flag_static | ae_flag_const, ci))

  ALLOC_PTR(gwi->gwion->mp, cf, m_float, 2.1);
  GWI_BB(gwi_item_ini(gwi,"float", "sc_f"))
  GWI_BB(gwi_item_end(gwi, ae_flag_static | ae_flag_const, (void*)cf))

  GWI_BB(gwi_item_ini(gwi,"int[][]", "test_array"))
  GWI_BB(gwi_item_end(gwi, ae_flag_none, NULL))

  GWI_BB(gwi_class_end(gwi))
  return GW_OK;
}
