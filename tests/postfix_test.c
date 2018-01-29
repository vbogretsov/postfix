#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <check.h>
#include <postfix.h>

#include <stdio.h>

#define TEST_LOG "postfix_test.log"

#define _PX_TOKEN(v, vt, tt) (px_token_t){.value.vt = (v), .type = (tt)}
#define _PX_VAR(value, type) _PX_TOKEN(value, type, PX_TOKEN_VAR)

#define _TEST_PX_PARSE_SUCCESS(infix, expected)                               \
do                                                                            \
{                                                                             \
    px_token_t* postfix = (px_token_t[PX_LEN(infix)]){};                      \
    int err = px_parse(infix, postfix, _px_prio);                             \
    ck_assert(err == PX_SUCCESS);                                             \
    px_assert_stack_eq(expected, postfix);                                    \
} while(0)

#define _TEST_PX_PARSE_FAILED(infix, code)                                    \
do                                                                            \
{                                                                             \
    px_token_t* postfix = (px_token_t[PX_LEN(infix)]){};                      \
    int err = px_parse(infix, postfix, _px_prio);                             \
    ck_assert(err == (code));                                                 \
} while(0)

#define _TEST_PX_EVAL_SUCCESS(postfix, expected)                              \
do                                                                            \
{                                                                             \
    px_value_t res;                                                           \
    int err = px_eval((postfix), NULL, &res);                                 \
    ck_assert(err == PX_SUCCESS);                                             \
    ck_assert(res.i64 == (expected));                                         \
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
static const px_token_t _PX_ADD  = _PX_TOKEN(_px_add, p, _PX_TEST_TOKEN_ADD);
static const px_token_t _PX_SUB  = _PX_TOKEN(_px_sub, p, _PX_TEST_TOKEN_SUB);
static const px_token_t _PX_MUL  = _PX_TOKEN(_px_mul, p, _PX_TEST_TOKEN_MUL);
static const px_token_t _PX_DIV  = _PX_TOKEN(_px_div, p, _PX_TEST_TOKEN_DIV);

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
    if (t.type < 0 || t.type > PX_LEN(_PX_TEST_PRIO_MAP))
    {
        return -1;
    }
    return _PX_TEST_PRIO_MAP[t.type];
}

static void px_assert_token_eq(px_token_t exp, px_token_t act)
{
    ck_assert(exp.type == act.type);
    ck_assert(exp.value.i64 == act.value.i64);
}

static void px_assert_stack_eq(px_token_t* sp_exp, px_token_t* sp_act)
{
    while (sp_exp->type != PX_TOKEN_TERM && sp_act->type != PX_TOKEN_TERM)
    {
        px_token_t exp = *sp_exp++;
        px_token_t act = *sp_act++;
        px_assert_token_eq(exp, act);
    }

    if (sp_exp->type != PX_TOKEN_TERM && sp_act->type == PX_TOKEN_TERM)
    {
        ck_abort_msg("len(expected) > len(actual)");
    }

    if (sp_exp->type == PX_TOKEN_TERM && sp_act->type != PX_TOKEN_TERM)
    {
        ck_abort_msg("len(expected) < len(actual)");
    }
}

START_TEST(test_parse_empty_infix)
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
END_TEST

START_TEST(test_parse_single_var)
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
END_TEST

START_TEST(test_parse_binary_op)
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
END_TEST

START_TEST(test_parse_different_prio)
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
END_TEST

START_TEST(test_parse_brackets_simple)
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
END_TEST

START_TEST(test_parse_brackets_complex)
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
END_TEST

START_TEST(test_parse_brackets_nested)
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
END_TEST

START_TEST(test_parse_brackets_empty)
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
END_TEST

START_TEST(test_parse_unmatched_bracket_single)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_TERM,
    };

    _TEST_PX_PARSE_FAILED(infix, PX_E_UNMATCHED_BRACKET);
}
END_TEST

START_TEST(test_parse_unmatched_bracket_simple)
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
END_TEST

START_TEST(test_parse_unmatched_bracket_complex)
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
END_TEST

START_TEST(test_parse_unmatched_bracket_nested)
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
END_TEST

START_TEST(test_eval_binary_op)
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
END_TEST

START_TEST(test_eval_different_prio)
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
END_TEST

START_TEST(test_eval_brackets_simple)
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

    _TEST_PX_EVAL_SUCCESS(postfix, 20);
}
END_TEST

START_TEST(test_eval_brackets_complex)
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
END_TEST


static TCase* create_parse_tcase()
{
    TCase* tcase = tcase_create("prefix_parse");
    tcase_add_test(tcase, test_parse_empty_infix);
    tcase_add_test(tcase, test_parse_single_var);
    tcase_add_test(tcase, test_parse_binary_op);
    tcase_add_test(tcase, test_parse_different_prio);
    tcase_add_test(tcase, test_parse_brackets_simple);
    tcase_add_test(tcase, test_parse_brackets_complex);
    tcase_add_test(tcase, test_parse_brackets_nested);
    tcase_add_test(tcase, test_parse_brackets_empty);
    tcase_add_test(tcase, test_parse_unmatched_bracket_single);
    tcase_add_test(tcase, test_parse_unmatched_bracket_simple);
    tcase_add_test(tcase, test_parse_unmatched_bracket_complex);
    tcase_add_test(tcase, test_parse_unmatched_bracket_nested);
    return tcase;
}

static TCase* create_eval_tcase()
{
    TCase* tcase = tcase_create("prefix_eval");
    tcase_add_test(tcase, test_eval_binary_op);
    tcase_add_test(tcase, test_eval_different_prio);
    tcase_add_test(tcase, test_eval_brackets_simple);
    tcase_add_test(tcase, test_eval_brackets_complex);
    return tcase;
}

static Suite* create_suite()
{
    Suite* suite = suite_create("postfix");
    suite_add_tcase(suite, create_parse_tcase());
    suite_add_tcase(suite, create_eval_tcase());
    return suite;
}

int main(void)
{
    srand(time(0));

    Suite* suite = create_suite();

    SRunner* runner = srunner_create(suite);
    srunner_set_xml(runner, TEST_LOG);
    srunner_set_fork_status(runner, CK_NOFORK);
    srunner_run_all(runner, CK_VERBOSE);

    int number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
