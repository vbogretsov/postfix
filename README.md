# postfix

C postfix notation library.

## Usage

The `postix` library allows to evaluate expressions according to custom rules.
Let's consider how to use it to calculate expressions of integers with the
following operations: `+` `-` `*` `/`.

The calculations process contains of 2 steps:

 * transform expression from infix to postfix notation
 * calculate expression in postfix notation

The function

```C
int px_parse(px_token_t* infix, px_token_t* postfix, px_prio_t prio);
```

is used to transform expression from infix notation to postfix notation. The
expected infix notation contains from tokens represented by the type

```C
struct px_token
{
    px_value_t value;
    int        type;
};
```

## Licence

See the [LICENCE](https://github.com/vbogretsov/postfix/blob/master/LICENSE) file.
