/**
 * Postfix notation library.
 * 
 * Copyright (C) Vladimir Bogretsov <bogrecov@gmail.com>
 */
#ifndef POSTFIX_H
#define POSTFIX_H

#include <stdbool.h>
#include <stdint.h>

#define PX_STACK_SIZE 1000

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
};

#define PX_LEN(array) sizeof((array))/sizeof((*array))

#ifndef PX_TOKEN_VALUE
#define PX_TOKEN_VALUE
#endif

typedef union
{
    bool    b;
    char    c;
    int     i;
    float   f;
    double  d;
    void*   p;
    int8_t  i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    PX_TOKEN_VALUE
} px_value_t;

typedef struct
{
    px_value_t value;
    int        type;
} px_token_t;

typedef int (*px_func_t)(px_value_t*, px_value_t*, void*);
typedef int (*px_prio_t)(px_token_t);

/*
 * Make postfix notation from the infix notation.
 *
 * @param infix   Infix notation, should be terminated with token of type
 *                PX_TOKEN_TERM.
 * @param postfix Pointer to array of tokens in postifx notation, should have
 *                enough size to store all the tokens (use PX_LEN(infix)),
 *                after function returns *postifx will point to the head of
 *                postfix notation, postfix notation will be terminated with
 *                token of type PX_TOKEN_TERM.
 * @param prio    Should return non negative priority of a token, if token type
 *                is unexpected it should return -1.
 * @return        PX_SUCCESS if postfix notation created,
 *                PX_E_UNMATCHED_BRACKET if infix expression contains unmatched
 *                pair of brackets,
 *                PX_E_STAK_OVERFLOW if infix expression is too big.
 */
int px_parse(px_token_t* infix, px_token_t* postfix, px_prio_t prio);

/*
 * Evaluate expression in the postfix notation.
 */
int px_eval(px_token_t* postfix, void* ctx, px_value_t* res);

#endif // !POSTFIX_H