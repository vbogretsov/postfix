#include <stdlib.h>
#include <time.h>

#include <check.h>
#include <postfix.h>

// #include <stdio.h>

#define TEST_LOG "postfix_test.log"

#define _PX_TOKEN_EQ(exp, act)                                                \
do                                                                            \
{                                                                             \
    ck_assert((exp).type == (act).type);                                      \
    ck_assert((exp).value == (act).value);                                    \
} while (0)

#define _PX_TOKEN(v, t) (px_token_t){.value = (v), .type = (t)}
#define _PX_VAR(value) _PX_TOKEN(PX_VAL(value), PX_TOKEN_VAR)

// ****************************************************************************
// #define ISTERM(token) ((token).type == PX_TOKEN_TERM)

// #define STACK_PUSH(sp, value) (*(sp)++ = value)
// #define STACK_POP(sp)         (*--(sp))
// #define STACK_TOP(sp)         (*((sp) - 1))

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
//     px_token_t t;
//     while (!ISTERM(t = STACK_POP(sp)))
//     {
//         print_token(t);
//     }
// }
// ****************************************************************************

#define _TEST_PX_PARSE_SUCCESS(infix, expected)                               \
do                                                                            \
{                                                                             \
    px_token_t* postfix = (px_token_t[PX_LEN(infix)]){};                      \
    int err = px_parse(infix, &postfix, px_test_prio);                        \
    ck_assert(err == PX_SUCCESS);                                             \
    px_assert_stack_eq(expected + (PX_LEN(expected) - 1), postfix);           \
} while(0)

#define _TEST_PX_PARSE_FAILED(infix, code)                                    \
do                                                                            \
{                                                                             \
    px_token_t* postfix = (px_token_t[PX_LEN(infix)]){};                      \
    int err = px_parse(infix, &postfix, px_test_prio);                        \
    ck_assert(err == (code));                                                 \
} while(0)

enum
{
    _PX_TEST_TOKEN_ADD = PX_TOKEN_RBRC + 1,
    _PX_TEST_TOKEN_SUB,
    _PX_TEST_TOKEN_MUL,
    _PX_TEST_TOKEN_DIV,
};

static const px_token_t _PX_TERM = _PX_TOKEN(NULL, PX_TOKEN_TERM);
static const px_token_t _PX_LBRC = _PX_TOKEN(PX_VAL('('), PX_TOKEN_LBRC);
static const px_token_t _PX_RBRC = _PX_TOKEN(PX_VAL(')'), PX_TOKEN_RBRC);
static const px_token_t _PX_ADD  = _PX_TOKEN(PX_VAL('+'), _PX_TEST_TOKEN_ADD);
static const px_token_t _PX_SUB  = _PX_TOKEN(PX_VAL('-'), _PX_TEST_TOKEN_SUB);
static const px_token_t _PX_MUL  = _PX_TOKEN(PX_VAL('*'), _PX_TEST_TOKEN_MUL);
static const px_token_t _PX_DIV  = _PX_TOKEN(PX_VAL('/'), _PX_TEST_TOKEN_DIV);

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

static int px_test_prio(px_token_t t)
{
    if (t.type < 0 || t.type > PX_LEN(_PX_TEST_PRIO_MAP))
    {
        return -1;
    }
    return _PX_TEST_PRIO_MAP[t.type];
}

static void px_assert_stack_eq(px_token_t* sp_exp, px_token_t* sp_act)
{
    while ((*sp_exp).type != PX_TOKEN_TERM && (*sp_act).type != PX_TOKEN_TERM)
    {
        _PX_TOKEN_EQ(*sp_exp, *sp_act);
        --sp_exp;
        --sp_act;
    }

    if ((*sp_exp).type != PX_TOKEN_TERM && (*sp_act).type == PX_TOKEN_TERM)
    {
        ck_abort_msg("len(expected) > len(actual)");
    }

    if ((*sp_exp).type == PX_TOKEN_TERM && (*sp_act).type != PX_TOKEN_TERM)
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
        _PX_VAR(1),
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_TERM,
        _PX_VAR(1),
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}
END_TEST

START_TEST(test_parse_binary_op)
{
    px_token_t infix[] =
    {
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(2),
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_TERM,
        _PX_VAR(2),
        _PX_VAR(2),
        _PX_ADD,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}
END_TEST

START_TEST(test_parse_different_prio)
{
    px_token_t infix[] =
    {
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(3),
        _PX_MUL,
        _PX_VAR(4),
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_TERM,
        _PX_VAR(2),
        _PX_VAR(3),
        _PX_VAR(4),
        _PX_MUL,
        _PX_ADD,
    };

    _TEST_PX_PARSE_SUCCESS(infix, expected);
}
END_TEST

START_TEST(test_parse_brackets_simple)
{
    px_token_t infix[] =
    {
        _PX_LBRC,
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(3),
        _PX_RBRC,
        _PX_MUL,
        _PX_VAR(4),
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_TERM,
        _PX_VAR(2),
        _PX_VAR(3),
        _PX_ADD,
        _PX_VAR(4),
        _PX_MUL,
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
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(3),
        _PX_RBRC,
        _PX_DIV,
        _PX_VAR(2),
        _PX_MUL,
        _PX_LBRC,
        _PX_VAR(3),
        _PX_ADD,
        _PX_VAR(4),
        _PX_DIV,
        _PX_VAR(2),
        _PX_RBRC,
        _PX_DIV,
        _PX_LBRC,
        _PX_VAR(1),
        _PX_ADD,
        _PX_VAR(1),
        _PX_RBRC,
        _PX_RBRC,
        _PX_ADD,
        _PX_VAR(1),
        _PX_SUB,
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(5),
        _PX_MUL,
        _PX_VAR(6),
        _PX_SUB,
        _PX_LBRC,
        _PX_VAR(1),
        _PX_ADD,
        _PX_VAR(2),
        _PX_MUL,
        _PX_VAR(3),
        _PX_MUL,
        _PX_VAR(4),
        _PX_DIV,
        _PX_VAR(2),
        _PX_RBRC,
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_TERM,
        _PX_VAR(2),
        _PX_VAR(3),
        _PX_ADD,
        _PX_VAR(2),
        _PX_DIV,
        _PX_VAR(3),
        _PX_VAR(4),
        _PX_VAR(2),
        _PX_DIV,
        _PX_ADD,
        _PX_MUL,
        _PX_VAR(1),
        _PX_VAR(1),
        _PX_ADD,
        _PX_DIV,
        _PX_VAR(1),
        _PX_ADD,
        _PX_VAR(2),
        _PX_SUB,
        _PX_VAR(5),
        _PX_VAR(6),
        _PX_MUL,
        _PX_ADD,
        _PX_VAR(1),
        _PX_VAR(2),
        _PX_VAR(3),
        _PX_MUL,
        _PX_VAR(4),
        _PX_MUL,
        _PX_VAR(2),
        _PX_DIV,
        _PX_ADD,
        _PX_SUB,
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
        _PX_VAR(1),
        _PX_ADD,
        _PX_LBRC,
        _PX_VAR(2),
        _PX_RBRC,
        _PX_RBRC,
        _PX_RBRC,
        _PX_MUL,
        _PX_LBRC,
        _PX_LBRC,
        _PX_VAR(3),
        _PX_SUB,
        _PX_VAR(1),
        _PX_RBRC,
        _PX_RBRC,
        _PX_RBRC,
        _PX_TERM,
    };

    px_token_t expected[] =
    {
        _PX_TERM,
        _PX_VAR(1),
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(3),
        _PX_VAR(1),
        _PX_SUB,
        _PX_MUL,
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
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(3),
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
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(3),
        _PX_RBRC,
        _PX_DIV,
        _PX_VAR(2),
        _PX_MUL,
        _PX_LBRC,
        _PX_VAR(3),
        _PX_ADD,
        _PX_VAR(4),
        _PX_DIV,
        _PX_VAR(2),
        _PX_RBRC,
        _PX_DIV,
        _PX_LBRC,
        _PX_VAR(1),
        _PX_ADD,
        _PX_VAR(1),
        _PX_RBRC,
        _PX_RBRC,
        _PX_ADD,
        _PX_VAR(1),
        _PX_SUB,
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(5),
        _PX_MUL,
        _PX_VAR(6),
        _PX_SUB,
        _PX_LBRC,
        _PX_VAR(1),
        _PX_ADD,
        _PX_VAR(2),
        _PX_MUL,
        _PX_VAR(3),
        _PX_MUL,
        _PX_VAR(4),
        _PX_DIV,
        _PX_VAR(2),
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
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(3),
        _PX_RBRC,
        _PX_DIV,
        _PX_VAR(2),
        _PX_MUL,
        _PX_LBRC,
        _PX_VAR(3),
        _PX_ADD,
        _PX_VAR(4),
        _PX_DIV,
        _PX_VAR(2),
        _PX_RBRC,
        _PX_DIV,
        _PX_LBRC,
        _PX_VAR(1),
        _PX_ADD,
        _PX_VAR(1),
        _PX_RBRC,
        _PX_RBRC,
        _PX_ADD,
        _PX_VAR(1),
        _PX_SUB,
        _PX_VAR(2),
        _PX_ADD,
        _PX_VAR(5),
        _PX_MUL,
        _PX_VAR(6),
        _PX_SUB,
        _PX_LBRC,
        _PX_VAR(1),
        _PX_ADD,
        _PX_VAR(2),
        _PX_MUL,
        _PX_VAR(3),
        _PX_MUL,
        _PX_VAR(4),
        _PX_DIV,
        _PX_VAR(2),
        _PX_RBRC,
        _PX_RBRC,
        _PX_RBRC,
        _PX_TERM,
    };

    _TEST_PX_PARSE_FAILED(infix, PX_E_UNMATCHED_BRACKET);
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

static Suite* create_suite()
{
    Suite* suite = suite_create("postfix");
    suite_add_tcase(suite, create_parse_tcase());
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
