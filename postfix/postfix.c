/**
 * Postfix notation library.
 * 
 * Copyright (C) Vladimir Bogretsov <bogrecov@gmail.com>
 */
#include "postfix.h"

#include <stdlib.h>

#define ISTERM(token) ((token).type == PX_TOKEN_TERM)
#define ISVAR(token)  ((token).type == PX_TOKEN_VAR)
#define ISFUNC(token) ((token).type > PX_TOKEN_RBRC)
#define ISLBRC(token) ((token).type == PX_TOKEN_LBRC)
#define ISRBRC(token) ((token).type == PX_TOKEN_RBRC)

static const px_token_t PX_TERM = (px_token_t)
{
    .value.i = 0,
    .type = PX_TOKEN_TERM
};

int px_parse(px_token_t* infix, px_token_t* postfix, px_prio_t prio)
{
    px_token_t stack[PX_STACK_SIZE];
    px_token_t* sp = stack;

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
                PX_STACK_PUSH(postfix, token);
                break;
            case PX_TOKEN_LBRC:
                PX_STACK_PUSH(sp, token);
                break;
            case PX_TOKEN_RBRC:
                while (true)
                {
                    if (sp == stack)
                    {
                        return PX_E_UNMATCHED_BRACKET;
                    }

                    px_token_t t = PX_STACK_POP(sp);
                    if (ISLBRC(t))
                    {
                        break;
                    }

                    PX_STACK_PUSH(postfix, t);
                }
                break;
            default:
                while (sp != stack && prio(token) <= prio(PX_STACK_TOP(sp)))
                {
                    PX_STACK_PUSH(postfix, PX_STACK_POP(sp));
                }
                PX_STACK_PUSH(sp, token);
                break;
        }
    }

    while (sp > stack)
    {
        px_token_t t = PX_STACK_POP(sp);
        if (ISLBRC(t))
        {
            return PX_E_UNMATCHED_BRACKET;
        }
        PX_STACK_PUSH(postfix, t);
    }

    *postfix = PX_TERM;
    return PX_SUCCESS;
}

int px_eval(px_token_t* postfix, void* ctx, px_value_t* res)
{
    px_value_t stack[PX_STACK_SIZE];
    px_value_t* sp = stack;

    for (int i = 0; !ISTERM(postfix[i]); ++i)
    {
        px_token_t token = postfix[i];
        if (ISVAR(token))
        {
            PX_STACK_PUSH(sp, token.value);
        }
        else if (ISFUNC(token))
        {
            int err = token.value.op(stack, &sp, ctx);
            if (err != PX_SUCCESS)
            {
                return err;
            }
        }
        else
        {
            return PX_E_UNEXPECTED_TOKEN;
        }
    }

    if (sp - stack != 1)
    {
        return PX_E_STACK_CORRUPTED;
    }

    *res = PX_STACK_POP(sp);
    return PX_SUCCESS;
}
