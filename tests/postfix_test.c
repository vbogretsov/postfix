#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmockery/cmockery.h>

#include <postfix/postfix.h>

#define _PX_TOKEN(v, vt, tt) (px_token_t){.value.vt = (v), .type = (tt)}
#define _PX_VAR(value, type) _PX_TOKEN(value, type, PX_TOKEN_VAR)

#define _TEST_PX_PARSE_SUCCESS(infix, expected)                               \
do                                                                            \
{                                                                             \
    px_token_t* postfix = (px_token_t[PX_LEN(infix)]){};                      \
    int err = px_parse(infix, postfix, _px_prio);                             \
    assert_true(err == PX_SUCCESS);                                           \
    px_assert_stack_eq(expected, postfix);                                    \
} while(0)

#define _TEST_PX_PARSE_FAILED(infix, code)                                    \
do                                                                            \
{                                                                             \
    px_token_t* postfix = (px_token_t[PX_LEN(infix)]){};                      \
    int err = px_parse(infix, postfix, _px_prio);                             \
    assert_true(err == (code));                                               \
} while(0)

#define _TEST_PX_EVAL_SUCCESS(postfix, expected)                              \
do                                                                            \
{                                                                             \
    px_value_t res;                                                           \
    int err = px_eval((postfix), NULL, &res);                                 \
    assert_true(err == PX_SUCCESS);                                           \
    assert_true(res.i64 == (expected));                                       \
} while (0)

#define _PX_BINARY_OP(op)                                                     \
do                                                                            \
{                                                                             \
    if (*sp == bp)                                                            \
    {                                                                         \
        return PX_E_MISSING_ARGUMENT;                                         \
    }                                                                         \
    int64_t a = PX_STACK_POP(*sp).i64;                                        \
    if (*sp == bp)                                                            \
    {                                                                         \
        return PX_E_MISSING_ARGUMENT;                                         \
    }                                                                         \
    int64_t b = PX_STACK_POP(*sp).i64;                                        \
    PX_STACK_PUSH(*sp, (px_value_t){.i64 = b op a});                          \
    return PX_SUCCESS;                                                        \
} while (0)

enum
{
    _PX_TEST_TOKEN_ADD = PX_TOKEN_RBRC + 1,
    _PX_TEST_TOKEN_SUB,
    _PX_TEST_TOKEN_MUL,
    _PX_TEST_TOKEN_DIV,
};

static int _px_add(px_value_t* bp, px_value_t** sp, void* ctx)
{
    _PX_BINARY_OP(+);
}

static int _px_sub(px_value_t* bp, px_value_t** sp, void* ctx)
{
    _PX_BINARY_OP(-);
}

static int _px_mul(px_value_t* bp, px_value_t** sp, void* ctx)
{
    _PX_BINARY_OP(*);
}

static int _px_div(px_value_t* bp, px_value_t** sp, void* ctx)
{
    _PX_BINARY_OP(/);
}

static const px_token_t _PX_TERM = _PX_TOKEN(0, i, PX_TOKEN_TERM);
static const px_token_t _PX_LBRC = _PX_TOKEN('(', c, PX_TOKEN_LBRC);
static const px_token_t _PX_RBRC = _PX_TOKEN(')', c, PX_TOKEN_RBRC);
static const px_token_t _PX_ADD  = _PX_TOKEN(_px_add, op, _PX_TEST_TOKEN_ADD);
static const px_token_t _PX_SUB  = _PX_TOKEN(_px_sub, op, _PX_TEST_TOKEN_SUB);
static const px_token_t _PX_MUL  = _PX_TOKEN(_px_mul, op, _PX_TEST_TOKEN_MUL);
static const px_token_t _PX_DIV  = _PX_TOKEN(_px_div, op, _PX_TEST_TOKEN_DIV);

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

static void px_assert_token_eq(px_token_t exp, px_token_t act)
{
    assert_true(exp.type == act.type);
    assert_true(exp.value.i64 == act.value.i64);
}

static void px_assert_stack_eq(px_token_t* sp_exp, px_token_t* sp_act)
{
    while (sp_exp->type != PX_TOKEN_TERM && sp_act->type != PX_TOKEN_TERM)
    {
        px_token_t exp = *sp_exp++;
        px_token_t act = *sp_act++;
        px_assert_token_eq(exp, act);
    }

    assert_int_equal(sp_exp->type, PX_TOKEN_TERM);
    assert_int_equal(sp_act->type, PX_TOKEN_TERM);
}

void test_parse_empty_infix(void** state)
{
    px_token_t infix[] =
    {
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_TERM,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}

void test_parse_single_var(void** state)
{
    px_token_t infix[] =
    {
        _PX_VAR(1, i64),
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_VAR(1, i64),
        _PX_TERM,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}

void test_parse_binary_op(void** state)
{
    px_token_t infix[] =
    {
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(2, i64),
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_VAR(2, i64),
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_TERM,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}

void test_parse_different_prio(void** state)
{
    px_token_t infix[] =
    {
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(3, i64),
        _PX_MUL,
        _PX_VAR(4, i64),
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_VAR(2, i64),
        _PX_VAR(3, i64),
        _PX_VAR(4, i64),
        _PX_MUL,
        _PX_ADD,
        _PX_TERM,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}

void test_parse_brackets_simple(void** state)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(3, i64),
        _PX_RBRC,
        _PX_MUL,
        _PX_VAR(4, i64),
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_VAR(2, i64),
        _PX_VAR(3, i64),
        _PX_ADD,
        _PX_VAR(4, i64),
        _PX_MUL,
        _PX_TERM,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}

void test_parse_brackets_complex(void** state)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_LBRC,
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(3, i64),
        _PX_RBRC,
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_LBRC,
        _PX_VAR(3, i64),
        _PX_ADD,
        _PX_VAR(4, i64),
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_RBRC,
        _PX_DIV,
        _PX_LBRC,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_VAR(1, i64),
        _PX_RBRC,
        _PX_RBRC,
        _PX_ADD,
        _PX_VAR(1, i64),
        _PX_SUB,
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(5, i64),
        _PX_MUL,
        _PX_VAR(6, i64),
        _PX_SUB,
        _PX_LBRC,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_VAR(3, i64),
        _PX_MUL,
        _PX_VAR(4, i64),
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_RBRC,
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_VAR(2, i64),
        _PX_VAR(3, i64),
        _PX_ADD,
        _PX_VAR(2, i64),
        _PX_DIV,
        _PX_VAR(3, i64),
        _PX_VAR(4, i64),
        _PX_VAR(2, i64),
        _PX_DIV,
        _PX_ADD,
        _PX_MUL,
        _PX_VAR(1, i64),
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_DIV,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_VAR(2, i64),
        _PX_SUB,
        _PX_VAR(5, i64),
        _PX_VAR(6, i64),
        _PX_MUL,
        _PX_ADD,
        _PX_VAR(1, i64),
        _PX_VAR(2, i64),
        _PX_VAR(3, i64),
        _PX_MUL,
        _PX_VAR(4, i64),
        _PX_MUL,
        _PX_VAR(2, i64),
        _PX_DIV,
        _PX_ADD,
        _PX_SUB,
        _PX_TERM,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}

void test_parse_brackets_nested(void** state)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_LBRC,
        _PX_LBRC,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_LBRC,
        _PX_VAR(2, i64),
        _PX_RBRC,
        _PX_RBRC,
        _PX_RBRC,
        _PX_MUL,
        _PX_LBRC,
        _PX_LBRC,
        _PX_VAR(3, i64),
        _PX_SUB,
        _PX_VAR(1, i64),
        _PX_RBRC,
        _PX_RBRC,
        _PX_RBRC,
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_VAR(1, i64),
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(3, i64),
        _PX_VAR(1, i64),
        _PX_SUB,
        _PX_MUL,
        _PX_TERM,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}

void test_parse_brackets_empty(void** state)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_RBRC,
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_TERM,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}

void test_parse_unmatched_bracket_single(void** state)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_TERM,
    };

    _TEST_PX_PARSE_FAILED(infix, PX_E_UNMATCHED_BRACKET);
}

void test_parse_unmatched_bracket_simple(void** state)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_LBRC,
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(3, i64),
        _PX_RBRC,
        _PX_TERM,
    };

    _TEST_PX_PARSE_FAILED(infix, PX_E_UNMATCHED_BRACKET);
}

void test_parse_unmatched_bracket_complex(void** state)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_LBRC,
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(3, i64),
        _PX_RBRC,
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_LBRC,
        _PX_VAR(3, i64),
        _PX_ADD,
        _PX_VAR(4, i64),
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_RBRC,
        _PX_DIV,
        _PX_LBRC,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_VAR(1, i64),
        _PX_RBRC,
        _PX_RBRC,
        _PX_ADD,
        _PX_VAR(1, i64),
        _PX_SUB,
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(5, i64),
        _PX_MUL,
        _PX_VAR(6, i64),
        _PX_SUB,
        _PX_LBRC,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_VAR(3, i64),
        _PX_MUL,
        _PX_VAR(4, i64),
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_RBRC,
        _PX_RBRC,
        _PX_TERM,
    };

    _TEST_PX_PARSE_FAILED(infix, PX_E_UNMATCHED_BRACKET);
}

void test_parse_unmatched_bracket_nested(void** state)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_LBRC,
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(3, i64),
        _PX_RBRC,
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_LBRC,
        _PX_VAR(3, i64),
        _PX_ADD,
        _PX_VAR(4, i64),
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_RBRC,
        _PX_DIV,
        _PX_LBRC,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_VAR(1, i64),
        _PX_RBRC,
        _PX_RBRC,
        _PX_ADD,
        _PX_VAR(1, i64),
        _PX_SUB,
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_VAR(5, i64),
        _PX_MUL,
        _PX_VAR(6, i64),
        _PX_SUB,
        _PX_LBRC,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_VAR(3, i64),
        _PX_MUL,
        _PX_VAR(4, i64),
        _PX_DIV,
        _PX_VAR(2, i64),
        _PX_RBRC,
        _PX_RBRC,
        _PX_RBRC,
        _PX_TERM,
    };

    _TEST_PX_PARSE_FAILED(infix, PX_E_UNMATCHED_BRACKET);
}

void test_eval_binary_op(void** state)
{
    px_token_t postfix[] =
    {
        _PX_VAR(2, i64),
        _PX_VAR(2, i64),
        _PX_ADD,
        _PX_TERM,
    };

    _TEST_PX_EVAL_SUCCESS(postfix, 4);
}

void test_eval_different_prio(void** state)
{
    px_token_t postfix[] =
    {
        _PX_VAR(2, i64),
        _PX_VAR(3, i64),
        _PX_VAR(4, i64),
        _PX_MUL,
        _PX_ADD,
        _PX_TERM,
    };

    _TEST_PX_EVAL_SUCCESS(postfix, 14);
}

void test_eval_brackets_simple(void** state)
{
    px_token_t postfix[] =
    {
        _PX_VAR(2, i64),
        _PX_VAR(3, i64),
        _PX_ADD,
        _PX_VAR(4, i64),
        _PX_MUL,
        _PX_TERM,
    };

    _TEST_PX_EVAL_SUCCESS(postfix, 21);
}

void test_eval_brackets_complex(void** state)
{
    px_token_t postfix[] =
    {
        _PX_VAR(2, i64),
        _PX_VAR(3, i64),
        _PX_ADD,
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_VAR(3, i64),
        _PX_VAR(4, i64),
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_ADD,
        _PX_MUL,
        _PX_VAR(1, i64),
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_MUL,
        _PX_VAR(1, i64),
        _PX_ADD,
        _PX_VAR(2, i64),
        _PX_SUB,
        _PX_VAR(5, i64),
        _PX_VAR(6, i64),
        _PX_MUL,
        _PX_ADD,
        _PX_VAR(1, i64),
        _PX_VAR(2, i64),
        _PX_VAR(3, i64),
        _PX_MUL,
        _PX_VAR(4, i64),
        _PX_MUL,
        _PX_VAR(2, i64),
        _PX_MUL,
        _PX_ADD,
        _PX_SUB,
        _PX_TERM,
    };

    _TEST_PX_EVAL_SUCCESS(postfix, 200);
}

int main(void)
{
    const UnitTest tests[] = {
        unit_test(test_parse_empty_infix),
        unit_test(test_parse_single_var),
        unit_test(test_parse_binary_op),
        unit_test(test_parse_different_prio),
        unit_test(test_parse_brackets_simple),
        unit_test(test_parse_brackets_complex),
        unit_test(test_parse_brackets_nested),
        unit_test(test_parse_brackets_empty),
        unit_test(test_parse_unmatched_bracket_single),
        unit_test(test_parse_unmatched_bracket_simple),
        unit_test(test_parse_unmatched_bracket_complex),
        unit_test(test_parse_unmatched_bracket_nested),
        unit_test(test_eval_binary_op),
        unit_test(test_eval_different_prio),
        unit_test(test_eval_brackets_simple),
        unit_test(test_eval_brackets_complex),
    };
    return run_tests(tests);
}
