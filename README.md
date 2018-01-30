# postfix

C postfix notation library.

## Usage

### API description

The `postix` library allows to evaluate expressions according to custom rules.
The calculations process contains of 2 steps:

 * transform expression from infix to postfix notation
 * calculate expression in postfix notation

The function

```C
int px_parse(px_token_t* infix, px_token_t* postfix, px_prio_t prio);
```

is used to transform expression from infix notation to postfix notation. The
expected infix notation is composed from tokens represented by the type

```C
struct px_token
{
    px_value_t value;
    int        type;
};
```

The field `type` indicates the type of the token. There are 4 basic types:
 1. `PX_TOKEN_TERM` -- indicates the end of sequence of tokens
 2. `PX_TOKEN_VAR` -- indicates the variable or number
 3. `PX_TOKEN_LBRC` -- indicates the left bracket `(`
 4. `PX_TOKEN_RBRC` -- indicates the right bracket `)`

User can define new token types but they should not overlap the basic types.

The field `value` contains a token value represented by the type:

```C
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
```

User can define macro PX_TOKEN_VALUE to extend available types.

The parameter `infix` should be terminated by the token of type
`PX_TOKEN_TERM`. User is responsible to build the correct array of tokens
representing the expression.

The parameter `postfix` is an empty array which size is enough to store all the
tokens of `infix` except the brackets. User can use the macro `PX_LEN` to get
the correct size of `postfix` if `infix` is an array located in stack. After
the function `px_parse` exists the `postfix` array will contain tokens of
expression in the postfix notation terminated by the token of type
`PX_TOKEN_TERM`.

The parameter `prio` is a pointer to a function with the following signature:

```C
int prio(px_token_t);
```

it returns priority of a token. The library assumes this function always
returns correct values.

The funciton `px_parse` returns the following error codes:

 * `PX_SUCCESS` -- if postfix notation was successfully built
 * `PX_E_UNMATCHED_BRACKET` -- if infix notation contains unmatched brackets
 * `PX_E_STAK_OVERFLOW` -- if infix notation exceeds `PX_STACK_SIZE`

If a token represents an arithmetic operation or function its value should be
of type `px_func_t` which is a function with the following signature:

```C
int px_func(px_value_t* bp, px_value_t** sp, void* ctx);
```

The parameter `bp` points to the beginning of a stack that is used to evaluate
expression in a postfix notation.

The parameter `sp` points to the head of the stack.

The parameter `ctx` can be used to share additional information across
operations (for example it can be a table of variables values). In a simple
case it can be `NULL`.

The library assumes the following behavior from the functions of type
`px_func_t`:

 1. take from stack one or more operands
 2. evaluate calculation result using the operands
 2. push result back to the stack

The condition

```C
bp == *sp
```

indicates the stack is empty. If a function of type `ps_func_t` needs to get an
operand from the stack but the stack is empty an error `PX_E_MISSING_ARGUMENT`
should be returned. Functions of type `px_func_t` are used to evaluate an
expression in postfix notation. If some such function returns non zero value
which indicate an error, the evaluation will be terminated and the error will
be returned.

After an expression is transformed from infix notation to postfix notation the
function

```C
int px_eval(px_token_t* postfix, void* ctx, px_value_t* res);
```

should be called to evaluate the expression.

Here the parameter `postfix` is an array terminated by the token of type
`PX_TOKEN_TERM` that represents the expression in the postfix notation.

The parameter `ctx` will be passed to values of tokens representing arithmetic
operations or functions (values of type `px_func_t` which is described above).

The function `px_eval` returns the following error codes:

 * `PX_SUCCESS` -- if evaluation succeeded
 * `PX_E_UNEXPECTED_TOKEN` -- if postfix contains token that is not variable
   or function
 * `PX_E_STACK_CORRUPTED` -- if stack size is not 1 at the end of
   calculation (which can be cause by incorrect implementation of `px_func_t`).
 * A user defined error code which is the result of execution a function of
   type`px_funct_t`.

### Example

Let's consider how to use the `postfix` library to calculate expressions of
integers with the following operations: `+` `-` `*` `/`. (In the code pieces
below it is assumed the header `postfix.h` is included)

First of all we need to declare types of new operations:

```C
enum
{
    _PX_TEST_TOKEN_ADD = PX_TOKEN_RBRC + 1,
    _PX_TEST_TOKEN_SUB,
    _PX_TEST_TOKEN_MUL,
    _PX_TEST_TOKEN_DIV,
};
```

We also need to specify to the library priority of each operation:

```C
static int _PX_TEST_PRIO_MAP[] =
{
    0, // PX_TOKEN_TERM
    0, // PX_TOKEN_VAR
    0, // PX_TOKEN_LBRC
    0, // PX_TOKEN_RBRC
    1, // _PX_TEST_TOKEN_ADD
    1, // _PX_TEST_TOKEN_SUB
    2, // _PX_TEST_TOKEN_MUL
    2, // _PX_TEST_TOKEN_DIV
};

static int _px_prio(px_token_t t)
{
    return _PX_TEST_PRIO_MAP[t.type];
}
```

And define functions representing arithmetic operations:

```C
static const int _PX_E_DIVISION_BY_ZERO = PX_E_STACK_CORRUPTED + 1;

static int _px_add(px_value_t* bp, px_value_t** sp, void* ctx)
{
    if (*sp == bp)
    {
        return PX_E_MISSING_ARGUMENT;
    }
    int64_t a = PX_STACK_POP(*sp).i64;

    if (*sp == bp)
    {
        return PX_E_MISSING_ARGUMENT;
    }
    int64_t b = PX_STACK_POP(*sp).i64;

    PX_STACK_PUSH(*sp, (px_value_t){.i64 = b + a});
    return PX_SUCCESS;
}

static int _px_sub(px_value_t* bp, px_value_t** sp, void* ctx)
{
    if (*sp == bp)
    {
        return PX_E_MISSING_ARGUMENT;
    }
    int64_t a = PX_STACK_POP(*sp).i64;

    if (*sp == bp)
    {
        return PX_E_MISSING_ARGUMENT;
    }
    int64_t b = PX_STACK_POP(*sp).i64;

    PX_STACK_PUSH(*sp, (px_value_t){.i64 = b - a});
    return PX_SUCCESS;
}

static int _px_mul(px_value_t* bp, px_value_t** sp, void* ctx)
{
    if (*sp == bp)
    {
        return PX_E_MISSING_ARGUMENT;
    }
    int64_t a = PX_STACK_POP(*sp).i64;

    if (*sp == bp)
    {
        return PX_E_MISSING_ARGUMENT;
    }
    int64_t b = PX_STACK_POP(*sp).i64;

    PX_STACK_PUSH(*sp, (px_value_t){.i64 = b * a});
    return PX_SUCCESS;
}

static int _px_div(px_value_t* bp, px_value_t** sp, void* ctx)
{
    if (*sp == bp)
    {
        return PX_E_MISSING_ARGUMENT;
    }
    int64_t a = PX_STACK_POP(*sp).i64;

    if (a == 0)
    {

    }

    if (*sp == bp)
    {
        return PX_E_MISSING_ARGUMENT;
    }
    int64_t b = PX_STACK_POP(*sp).i64;

    PX_STACK_PUSH(*sp, (px_value_t){.i64 = b / a});
    return PX_SUCCESS;
}
```

Now we can calculate expression `(2 + 3) * 4` represented by the following
array of tokens:

```C
px_token_t infix[] =
    {
        (px_token_t){.type = PX_TOKEN_LBRK},
        (px_token_t){.value.i64 = 2, .type = PX_TOKEN_VAR},
        (px_token_t){.value.op = _px_add, .type = _PX_TOKEN_ADD},
        (px_token_t){.value.i64 = 3, .type = PX_TOKEN_VAR},
        (px_token_t){.type = PX_TOKEN_RBRK},
        (px_token_t){.value.op = _px_mul, .type = _PX_TOKEN_MUL},
        (px_token_t){.value.i64 = 4, .type = PX_TOKEN_VAR},
        (px_token_t){.type = PX_TOKEN_TERM},
    };
```

using the functions `px_parse` and `px_eval`.

## Licence

See the [LICENCE](https://github.com/vbogretsov/postfix/blob/master/LICENSE) file.
