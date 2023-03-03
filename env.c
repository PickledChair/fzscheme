#include "fzscheme.h"

DEFINE_DOUBLY_LINKED_LIST_FUNCS(EnvNode, Env, true)
static EnvNode *marked_env_list_head = &(EnvNode){};

void mark_env_node(EnvNode *node) {
  DOUBLY_LINKED_LIST_REMOVE_FUNC_NAME(EnvNode)(node);
  DOUBLY_LINKED_LIST_PUSH_FUNC_NAME(EnvNode)(node, &marked_env_list_head);
}

void env_list_gc(void) {
  DOUBLY_LINKED_LIST_CLEAR_FUNC_NAME(EnvNode)();
  if (marked_env_list_head->next != NULL) {
    DOUBLY_LINKED_LIST_HEAD_NAME(EnvNode)->next = marked_env_list_head->next;
    marked_env_list_head->next->prev = DOUBLY_LINKED_LIST_HEAD_NAME(EnvNode);

    marked_env_list_head->next = NULL;
  }
}

Env *new_env(void) {
  Env *env = calloc(1, sizeof(Env));
  env->vars = NIL;
  env->next = NULL;
  return env;
}

void env_collect_roots(Env *env) {
  for (Env *cur = env; cur != NULL; cur = cur->next) {
    NODE_TYPE_NEW_FUNC_NAME(RootNode)(&cur->vars);
  }
}

static int position_var(Object *symbol, Object *vars, int *pos) {
  switch (vars->tag) {
    case OBJ_CELL: {
      int i = 0;
      while (vars != NIL) {
        if (vars->tag != OBJ_CELL) {
          if (vars == symbol) {
            *pos = -(i + 1);
            return 0;
          } else {
            return -1;
          }
        }
        if (CAR(vars) == symbol) {
          *pos = i;
          return 0;
        }
        vars = CDR(vars);
        i++;
      }
      return -1;
    }
    case OBJ_SYMBOL:
      if (vars == symbol) {
        *pos = -1;
        return 0;
      } else {
        return -1;
      }
    default:
      return -1;
  }
}

static int location_helper(Env *env, Object *symbol, int cur_i, int *i, int *j) {
  int cur_j;
  if (position_var(symbol, env->vars, &cur_j) == 0) {
    *i = cur_i;
    *j = cur_j;
    return 0;
  } else {
    if (env->next) {
      return location_helper(env->next, symbol, cur_i + 1, i, j);
    } else {
      return -1;
    }
  }
}

int location(Env *env, Object *symbol, int *i, int *j) {
  return location_helper(env, symbol, 0, i, j);
}

static Object *get_helper(Env *env, int tmp_i, int target_i, int j) {
  if (tmp_i == target_i) {
    if (env->vars->tag == OBJ_CELL) {
      int pos = (j < 0) ? -(j + 1) : j;
      Object *obj = env->vars;
      if (obj == NIL && j != -1)
        return NULL;
      for (int k = 0; k < pos; k++)
        obj = CDR(obj);
      if (j < 0)
        return obj;
      else
        return CAR(obj);
    } else {
      if (j == -1) {
        return env->vars;
      } else {
        return NULL;
      }
    }
  } else {
    if (env->next) {
      return get_helper(env->next, tmp_i + 1, target_i, j);
    } else {
      return NULL;
    }
  }
}

Object *get_lvar(Env *env, int i, int j) {
  return get_helper(env, 0, i, j);
}

static int set_helper(Env *env, int tmp_i, int target_i, int j, Object *val) {
  if (tmp_i == target_i) {
    if (j >= 0) {
      Object *var = env->vars;
      for (int k = 0; k <= j; k++)
        var = CDR(var);
      CAR(var) = val;
      return 0;
    } else {
      if (j == -1) {
        env->vars = val;
        return 0;
      } else {
        Object *tail = env->vars;
        for (int k = 0; k < -j; k++)
          tail = CDR(tail);
        CDR(tail) = val;
        return 0;
      }
    }
  } else {
    if (env->next)
      return set_helper(env->next, tmp_i + 1, target_i, j, val);
    else
      return -1;
  }
}

int set_lvar(Env *env, int i, int j, Object *val) {
  return set_helper(env, 0, i, j, val);
}
