/**
 * Postfix notation library.
 * 
 * Copyright (C) Vladimir Bogretsov <bogrecov@gmail.com>
 */
#include "postfix.h"

#include <stdlib.h>

// #include <stdio.h>

#define ISTERM(token) ((token).type == PX_TOKEN_TERM)
#define ISVAR(token)  ((token).type == PX_TOKEN_VAR)
#define ISFUNC(token) ((token).type > PX_TOKEN_RBRC)
#define ISLBRC(token) ((token).type == PX_TOKEN_LBRC)
#define ISRBRC(token) ((token).type == PX_TOKEN_RBRC)

// #define STACK_PUSH(sp, value) (*(sp)++ = value)
// #define STACK_POP(sp)         (*--(sp))
// #define STACK_TOP(sp)         (*((sp) - 1))

static const px_token_t PX_TERM = (px_token_t)
{
    .value.i = 0,
    .type = PX_TOKEN_TERM
};

// ****************************************************************************
// static void print_token(px_token_t t)
// {
//     if (t.type == PX_TOKEN_VAR)
//     {
//         printf(" %d", (int)t.value);
//     }
//     else if (t.type == PX_TOKEN_TERM)
//     {
//         printf("%s", "");
//     }
//     else
//     {
//         printf(" %c", (char)t.value);
//     }
// }

// static void print_stack(px_token_t* sp)
// {
//     printf("%s\n", "print_stack");
//     px_token_t t;
//     while (!ISTERM(t = STACK_POP(sp)))
//     {
//         print_token(t);
//     }
// }
// ****************************************************************************

int px_parse(px_token_t* infix, px_token_t** postfix, px_prio_t prio)
{
    PX_STACK_PUSH(*postfix, PX_TERM);

    px_token_t stack[PX_STACK_SIZE];
    px_token_t* sp = stack;
    PX_STACK_PUSH(sp, PX_TERM);

    for (int i = 0; !ISTERM(infix[i]); ++i)
    {
        if (i == PX_STACK_SIZE)
        {
            return PX_E_STAK_OVERFLOW;
        }

        px_token_t token = infix[i];
        switch (token.type)
        {
            case PX_TOKEN_VAR:
                PX_STACK_PUSH(*postfix, token);
                break;
            case PX_TOKEN_LBRC:
                PX_STACK_PUSH(sp, token);
                break;
            case PX_TOKEN_RBRC:
                {
                    px_token_t t;
                    while (!ISLBRC(t = PX_STACK_POP(sp)))
                    {
                        if (ISTERM(t))
                        {
                            return PX_E_UNMATCHED_BRACKET;
                        }
                        PX_STACK_PUSH(*postfix, t);
                    }
                }
                break;
            default:
                while (prio(token) <= prio(PX_STACK_TOP(sp)))
                {
                    PX_STACK_PUSH(*postfix, PX_STACK_POP(sp));
                }
                PX_STACK_PUSH(sp, token);
                break;
        }
    }

    px_token_t t;
    while (!ISTERM(t = PX_STACK_POP(sp)))
    {
        if (ISLBRC(t))
        {
            return PX_E_UNMATCHED_BRACKET;
        }
        PX_STACK_PUSH(*postfix, t);
    }

    --*postfix;
    return PX_SUCCESS;
}

int px_eval(px_token_t* postfix, void* ctx, px_value_t* res)
{
    px_value_t stack[PX_STACK_SIZE];
    px_value_t* sp = stack;

    px_token_t token;
    while (!ISTERM(token = PX_STACK_POP(postfix)))
    {
        if (ISVAR(token))
        {
            PX_STACK_PUSH(sp, token.value);
        }
        else if (ISFUNC(token))
        {
            px_func_t func = (px_func_t)token.value.p;

            int err = func(stack, sp, ctx);
            if (err != 0)
            {
                return err;
            }
        }
        else
        {
            return PX_E_UNEXPECTED_TOKEN;
        }
    }

    *res = token.value;
    return PX_SUCCESS;
}
