/**
 * Postfix notation library.
 * 
 * Copyright (C) Vladimir Bogretsov <bogrecov@gmail.com>
 */
#ifndef POSTFIX_H
#define POSTFIX_H

#include <stdbool.h>
#include <stdint.h>

#define PX_STACK_SIZE 1 << 10

#define PX_STACK_PUSH(sp, value) (*(sp)++ = (value))
#define PX_STACK_POP(sp)         (*--(sp))
#define PX_STACK_TOP(sp)         (*((sp) - 1))

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
    PX_E_UNEXPECTED_TOKEN,
    PX_E_MISSING_ARGUMENT,
    PX_E_STACK_CORRUPTED,
};

#define PX_LEN(array) sizeof((array))/sizeof((*array))

#ifndef PX_TOKEN_VALUE
#define PX_TOKEN_VALUE
#endif

typedef union  px_value px_value_t;
typedef struct px_token px_token_t;

typedef int (*px_func_t)(px_value_t*, px_value_t**, void*);
typedef int (*px_prio_t)(px_token_t);

union px_value
{
    bool      b;
    char      c;
    int       i;
    float     f;
    double    d;
    void*     p;
    int8_t    i8;
    int16_t   i16;
    int32_t   i32;
    int64_t   i64;
    px_func_t op;
    PX_TOKEN_VALUE
};

struct px_token
{
    px_value_t value;
    int        type;
};

/*
 * Create postfix notation for expression provided in the infix notation.
 *
 * @param infix   Infix notaion terminated with PX_TERM.
 * @param postfix Empty array for postfix notation, should have enough space
 *                (use PX_LEN(infix)), will be terminated with PX_TOKEN_TERM.
 * @param prio    Function to calculate priority of a token.
 * @return        PX_SUCCESS - if postfix notation was successfully built,
 *                PX_E_UNMATCHED_BRACKET - if infix notation contains unmatched
 *                brackets,
 *                PX_E_STAK_OVERFLOW - if infix notation exceeds PX_STACK_SIZE.
 */
int px_parse(px_token_t* infix, px_token_t* postfix, px_prio_t prio);

/*
 * Evaluate expression provided in the postfix notation.
 *
 * @param postfix Expression in postfix notation terminated with PX_TOKEN_TERM.
 * @param ctx     Closure context for operators represented ad px_func_t.
 * @param res     Calculation result.
 * @return        PX_SUCCESS - if evaluation succeeded,
 *                PX_E_UNEXPECTED_TOKEN - if postfix contains token that is not
 *                variable or function,
 *                PX_E_STACK_CORRUPTED - if stack size is not 1 at the end of
 *                calculation (which can be cause by incorrect implementation
 *                of px_func_t).
 */
int px_eval(px_token_t* postfix, void* ctx, px_value_t* res);

#endif // !POSTFIX_H