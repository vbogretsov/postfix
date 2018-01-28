#ifndef POSTFIX_H
#define POSTFIX_H

#define PX_STACK_SIZE 1000

enum {
    PX_TOKEN_TERM,
    PX_TOKEN_VAR,
    PX_TOKEN_LBRC,
    PX_TOKEN_RBRC,
};

enum {
    PX_SUCCESS,
    PX_E_UNMATCHED_BRACKET,
    PX_E_STAK_OVERFLOW,
    PX_E_EVAL,
};

#define PX_LEN(array) sizeof((array))/sizeof((*array))
#define PX_VAL(value) ((void*)value)

typedef struct
{
    void* value;
    int   type;
} px_token_t;

typedef int (*px_func_t)(px_token_t*, void*);
typedef int (*px_prio_t)(px_token_t);

int px_parse(px_token_t* infix, px_token_t** postfix, px_prio_t prio);

int px_eval(px_token_t* postfix, px_func_t* funcs, void* ctx);

#endif // !POSTFIX_H