/**
 * Postfix notation library.
 * 
 * Copyright (C) Vladimir Bogretsov <bogrecov@gmail.com>
 */
#ifndef POSTFIX_H
#define POSTFIX_H

// Maximum supported count of tokens in postfix notation.
#define PX_STACK_SIZE 1000

// Basic token types.
enum {
    // Termination token.
    PX_TOKEN_TERM,
    // Variable.
    PX_TOKEN_VAR,
    // Left bracket '('.
    PX_TOKEN_LBRC,
    // Right bracket ')'.
    PX_TOKEN_RBRC,
};

// Library error codes.
enum {
    // Indicates success result.
    PX_SUCCESS,
    // Indicates unmatched brackets.
    PX_E_UNMATCHED_BRACKET,
    // Indicates exceeded PX_STACK_SIZE.
    PX_E_STAK_OVERFLOW,
    // Indicates evaluation error.
    PX_E_EVAL,
};

// Get length of array allocated in stack.
#define PX_LEN(array) sizeof((array))/sizeof((*array))
// Cast value to type of token.value.
#define PX_VAL(value) ((void*)value)

// Expression token.
typedef struct
{
    void* value;
    int   type;
} px_token_t;

typedef int (*px_func_t)(px_token_t*, void*);
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
int px_parse(px_token_t* infix, px_token_t** postfix, px_prio_t prio);

int px_eval(px_token_t* postfix, px_func_t* funcs, void* ctx);

#endif // !POSTFIX_H