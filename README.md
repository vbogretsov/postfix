# postfix

C postfix notation library.

## Usage

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

The parameter `infix` should be terminated by the token with type
`PX_TOKEN_TERM`. User is responsible to build the correct array of tokens
representing the expression.

The parameter `postfix` is an empty array which size is enough to store all the
tokens of `infix` except the brackets. User can use the macro `PX_LEN` to get
the correct size of `postfix` if `infix` is an array located in stack. After
the function `px_parse` exists the `postfix` array will contain tokens of
expression in the postfix notation terminated by the token with type
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

TODO

Let's consider how to use it to calculate expressions of integers with the
following operations: `+` `-` `*` `/`.

## Licence

See the [LICENCE](https://github.com/vbogretsov/postfix/blob/master/LICENSE) file.
