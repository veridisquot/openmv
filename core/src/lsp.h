#pragma once

#include "common.h"

struct lsp_state;
struct lsp_chunk;

enum {
	lsp_val_nil = 0,
	lsp_val_num,
	lsp_val_bool,
	lsp_val_obj
};

enum {
	lsp_obj_str = 0,
	lsp_obj_fun,
	lsp_obj_nat
};

typedef struct lsp_val (*lsp_nat_fun_t)(struct lsp_state*, u32, struct lsp_val*);

struct lsp_obj {
	u32 ref;
	u8 type;
	bool recyclable;

	union {
		struct { char* chars; u32 len; }              str;
		struct { struct lsp_chunk* chunk; u32 argc; } fun;
	} as;
};

struct lsp_val {
	u8 type;
	union {
		double num;
		bool boolean;
		struct lsp_obj* obj;
	} as;
};

#define lsp_make_nil()           ((struct lsp_val) { .type = lsp_val_nil })
#define lsp_make_num(n_)         ((struct lsp_val) { .type = lsp_val_num,  .as.num = n_ })
#define lsp_make_bool(b_)        ((struct lsp_val) { .type = lsp_val_bool, .as.boolean = b_ })

struct lsp_val lsp_make_str(struct lsp_state* ctx, const char* start, u32 len);
struct lsp_val lsp_make_fun(struct lsp_state* ctx, struct lsp_chunk* chunk, u32 argc);

API bool lsp_vals_eq(struct lsp_state* ctx, struct lsp_val a, struct lsp_val b);

API struct lsp_state* new_lsp_state(void* error, void* info);
API void free_lsp_state(struct lsp_state* state);

API void lsp_set_simple_errors(struct lsp_state* state, bool simple);

API void           lsp_push(struct lsp_state* ctx, struct lsp_val val);
API struct lsp_val lsp_pop(struct lsp_state* ctx);
API struct lsp_val lsp_peek(struct lsp_state* ctx);

API struct lsp_val lsp_do_string(struct lsp_state* ctx, const char* str);
API struct lsp_val lsp_do_file(struct lsp_state* ctx, const char* file_path);

API void lsp_register(struct lsp_state* ctx, const char* name, u32 argc, lsp_nat_fun_t fun);
API void lsp_register_std(struct lsp_state* ctx);
