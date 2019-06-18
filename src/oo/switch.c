#include <stdlib.h>
#include "gwion_util.h"
#include "gwion_ast.h"
#include "oo.h"
#include "env.h"
#include "vm.h"
#include "env.h"
#include "gwion.h"
#include "operator.h"
#include "value.h"
#include "type.h"
#include "context.h"
#include "nspc.h"
#include "traverse.h"
#include "parse.h"
#include "switch.h"

static inline void _scope_add(Scope s, Switch sw) { vector_add((Vector)(void*)s, (vtype)sw); }
static inline vtype _scope_pop(Scope s) { return vector_pop((Vector)(void*)s); }
static inline vtype _scope_back(Scope s) { return vector_back((Vector)(void*)s); }
static inline void _scope_clear(Scope s) { vector_clear((Vector)(void*)s); }

static Switch new_switch(MemPool p) {
  Switch sw = mp_calloc(p, Switch);
  sw->cases = new_map(p); // could be struct ?
  vector_init(&sw->exp);
  sw->vec = new_vector(p);
  return sw;
}

ANN static void free_switch(MemPool p, const Switch sw) {
  if(!sw->ok)
    free_map(p, sw->cases);
  free_vector(p, sw->vec); // only for dynamic
  vector_release(&sw->exp);
  mp_free(p, Switch, sw);
}

struct SwInfo_ {
  Stmt_Switch s;
  Type t;
  Func f;
};

ANN static Switch new_swinfo(const Env env, const Stmt_Switch stmt) {
  struct SwInfo_ *info = mp_calloc(env->gwion->mp, SwInfo);
  info->s = stmt;
  info->t = env->class_def;
  info->f = env->func;
  const Switch sw = new_switch(env->gwion->mp);
  map_set(&env->scope->swi->map, (vtype)info, (vtype)sw);
  sw->depth = env->scope->depth + 2;
  return sw;
}

ANN static inline m_bool swinfo_cmp(const struct SwInfo_ *i1, const struct SwInfo_ *i2) {
  return i1->s == i2->s && i1->t == i2->t && i1->f == i2->f;
}

ANN Switch swinfo_get(const Env env, const struct SwInfo_ *info) {
  for(m_uint i = 0; i < VLEN(&env->scope->swi->map); ++i) {
    const struct SwInfo_ *key = (const struct SwInfo_*)VKEY(&env->scope->swi->map, i);
    if(swinfo_cmp(key, info))
      return (Switch)VVAL(&env->scope->swi->map, i);
  }
  return NULL;
}

ANN m_bool switch_add(const Env env, const Stmt_Switch stmt) {
  const struct SwInfo_ info = { stmt, env->class_def, env->func };
  Switch sw = (Switch)swinfo_get(env, &info) ?: new_swinfo(env, stmt);
  _scope_add(env->scope->swi, sw);
  return GW_OK;
}

ANN m_bool switch_decl(const Env env, const loc_t pos) {
  const Switch sw = (Switch)(VLEN(&env->scope->swi->map) ?
    VVAL(&env->scope->swi->map, VLEN(&env->scope->swi->map) - 1): 0);
  if(sw && sw->depth == env->scope->depth)
    ERR_B(pos, _("Declaration in switch is prohibited."))
  return GW_OK;
}

ANN void switch_get(const Env env, const Stmt_Switch stmt) {
  const struct SwInfo_ info = { stmt, env->class_def, env->func };
  const Switch sw = swinfo_get(env, &info);
  _scope_add(env->scope->swi, sw);
}

void switch_reset(const Env env) {
  for(m_uint i = VLEN(&env->scope->swi->map) + 1; --i;) {
    struct SwInfo_ *info = (struct SwInfo_ *)VKEY(&env->scope->swi->map, i - 1);
    mp_free(env->gwion->mp, SwInfo, info);
    Switch sw = (Switch)VVAL(&env->scope->swi->map, i - 1);
    free_switch(env->gwion->mp, sw);
  }
  _scope_clear(env->scope->swi);
  map_clear(&env->scope->swi->map);
}

ANN void switch_release(const Scope sw) {
  vector_release((Vector)sw);
  map_release(&sw->map);
}

ANN void switch_expset(const Env env, const Exp e) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  vector_add(&sw->exp, (vtype)e);
}

ANN Exp switch_expget(const Env env) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  return (Exp)vector_at(&sw->exp, sw->iter++);
}

ANN m_bool switch_inside(const Env env, const loc_t pos) {
  if(!VLEN(env->scope->swi))
    ERR_B(pos, _("case found outside switch statement."))
  return GW_OK;
}
ANN m_bool switch_dup(const Env env, const m_int value, const loc_t pos) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  if(map_get(sw->cases, (vtype)value))
    ERR_B(pos, _("duplicated cases value %i"), value)
  sw->ok = 1;
  return GW_OK;
}

ANN void switch_pc(const Env env, const m_uint pc) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  vector_add(sw->vec, pc);
}

ANN void switch_dynpc(const Env env, const m_int val, const m_uint pc) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  map_set(sw->cases, val, pc);
}

ANN m_bool switch_dyn(const Env env) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  return vector_size(&sw->exp);
}

ANN m_bool switch_default(const Env env, const m_uint pc, const loc_t pos) {
  if(!VLEN(env->scope->swi))
    ERR_B(pos, _("'default'case found outside switch statement."))
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  if(sw->default_case_index)
    ERR_B(pos, _("default case already defined"))
  sw->default_case_index = pc;
  return GW_OK;
}

ANN Map switch_map(const Env env) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  return sw->cases;
}

ANN Vector switch_vec(const Env env) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  return vector_copy(env->gwion->mp, sw->vec); // new_vector(); // dyn only
}

ANN m_uint switch_idx(const Env env) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  return sw->default_case_index;
}

ANN m_bool switch_pop(const Env env) {
  const Switch sw = (Switch)_scope_back(env->scope->swi);
  sw->ok = 1;
  _scope_pop(env->scope->swi);
  return GW_OK;
}

ANN m_bool switch_end(const Env env, const loc_t pos) {
  const Switch sw = (Switch)_scope_pop(env->scope->swi);
  const vtype index = VKEY(&env->scope->swi->map, VLEN(&env->scope->swi->map) - 1);
//  sw->ok = 1;
  if(!VLEN(sw->cases) && !VLEN(&sw->exp))
    ERR_B(pos, _("switch statement with no cases."))
  map_remove(&env->scope->swi->map, index);
  free_switch(env->gwion->mp, sw);
  return GW_OK;
}
