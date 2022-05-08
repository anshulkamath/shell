#include "regex.h"
#include "regex-private.h"

#include "testing-logger.h"
#include <string.h>

#define BITS_LONG (8 * sizeof(long))
#define GET_IND(arr, i) ((arr)[(i) / BITS_LONG] >> ((i) % BITS_LONG)) & 1

void test_regex_compile_naive() {
    testing_logger_t *tester = create_tester();
    re_t *reg_list;
    char *regexp;
    
    regexp = "hello";
    reg_list = re_compile(regexp);
    for (int i = 0; i < 5; i++) {
        expect(tester, reg_list[i].type == CHAR);
        expect(tester, reg_list[i].class.c == regexp[i]);
        expect(tester, reg_list[i].nccl == 0);
    }
    re_free(reg_list);

    regexp = "^hello.*";
    reg_list = re_compile(regexp);
    expect(tester, reg_list[0].type == BEGIN);
    expect(tester, reg_list[0].class.c == '^');
    expect(tester, reg_list[0].nccl == 0);

    // check `hello` characters
    for (int i = 1; i < 6; i++) {
        expect(tester, reg_list[i].type == CHAR);
        expect(tester, reg_list[i].class.c == regexp[i]);
        expect(tester, reg_list[i].nccl == 0);
    }

    expect(tester, reg_list[6].type == DOT);
    expect(tester, reg_list[6].class.c == '.');
    expect(tester, reg_list[6].nccl == 0);

    expect(tester, reg_list[7].type == STAR);
    expect(tester, reg_list[7].class.c == '*');
    expect(tester, reg_list[7].nccl == 0);
    re_free(reg_list);

    regexp = "\\.*";
    reg_list = re_compile(regexp);
    expect(tester, reg_list[0].type == CHAR);
    expect(tester, reg_list[0].class.c == '.');
    expect(tester, reg_list[0].nccl == 0);

    expect(tester, reg_list[1].type == STAR);
    expect(tester, reg_list[1].class.c == '*');
    expect(tester, reg_list[1].nccl == 0);
    re_free(reg_list);

    regexp = "\\\\*";
    reg_list = re_compile(regexp);
    expect(tester, reg_list[0].type == CHAR);
    expect(tester, reg_list[0].class.c == '\\');
    expect(tester, reg_list[0].nccl == 0);

    expect(tester, reg_list[1].type == STAR);
    expect(tester, reg_list[1].class.c == '*');
    expect(tester, reg_list[1].nccl == 0);
    re_free(reg_list);

    log_tests(tester);
}

void test_naive_regex() {
    testing_logger_t *tester = create_tester();
    char *test_reg = "^Hello .orld.";
    
    expect(tester, re_is_match(test_reg, "Hello World!"));
    expect(tester, re_is_match(test_reg, "Hello world."));
    expect(tester, !re_is_match(test_reg, "world."));
    expect(tester, !re_is_match(test_reg, "random string."));

    test_reg = "^Hello .orld*!";

    // expect kleene star to accept arbitrary characters (0 or more)
    expect(tester, re_is_match(test_reg, "Hello worlddddddddd!"));
    expect(tester, re_is_match(test_reg, "Hello worl!"));

    // expect empty regex to match anything
    expect(tester, re_is_match("", "test"));

    // expect non-empty regex and empty string to not match
    expect(tester, !re_is_match("a", ""));

    // expect $ to work
    expect(tester, re_is_match("world$", "this is the world"));

    log_tests(tester);
}

void test_regex_escape() {
    testing_logger_t *tester = create_tester();

    // simple examples of escaping
    expect(tester, re_is_match("Hello world\\.", "Hello world."));
    expect(tester, !re_is_match("Hello world\\.", "Hello world"));

    expect(tester, re_is_match("Hello world\\$", "Hello world$"));
    expect(tester, !re_is_match("Hello world\\$", "Hello world"));

    expect(tester, re_is_match("Hello world\\*", "Hello world*"));
    expect(tester, !re_is_match("Hello world\\*", "Hello worldddd"));

    // escaping at beginning
    expect(tester, re_is_match("\\$Hello world", "$Hello world"));
    expect(tester, !re_is_match("\\$Hello world", "Hello world"));

    // kleene star on `\`
    expect(tester, re_is_match("Hello world\\\\*.", "Hello world\\\\\\."));
    expect(tester, re_is_match("Hello world\\\\*.", "Hello world."));

    log_tests(tester);
}

void test_regex_some_many() {
    testing_logger_t *tester = create_tester();

    expect(tester, re_is_match("Hello world!+", "Hello world!"));
    expect(tester, re_is_match("Hello world!+", "Hello world!!!!"));
    expect(tester, re_is_match("Hello world.+", "Hello worlddddd"));
    expect(tester, !re_is_match("Hello world.+", "Hello world"));
    expect(tester, re_is_match("Hello world!\\+", "Hello world!+"));

    expect(tester, re_is_match("Hello world!?", "Hello world!"));
    expect(tester, re_is_match("Hello world!?", "Hello world"));
    expect(tester, re_is_match("Hello world.?", "Hello world!!!!"));
    expect(tester, re_is_match("Hello world!\\?\\?", "Hello world!??"));

    log_tests(tester);
}

void test_regex_compile_ccl() {
    testing_logger_t *tester = create_tester();
    re_t *reg_list;
    char *regexp;
    
    regexp = "[abc]";
    reg_list = re_compile(regexp);
    expect(tester, reg_list[0].type == CHAR_CLASS);
    expect(tester, GET_IND(reg_list[0].class.mask, 'a'));
    expect(tester, GET_IND(reg_list[0].class.mask, 'b'));
    expect(tester, GET_IND(reg_list[0].class.mask, 'c'));
    expect(tester, reg_list[0].nccl == 0);
    re_free(reg_list);

    regexp = ".[abc]*";
    reg_list = re_compile(regexp);
    expect(tester, reg_list[0].type == DOT);
    expect(tester, reg_list[0].class.c == '.');
    expect(tester, reg_list[0].nccl == 0);
    
    expect(tester, reg_list[1].type == CHAR_CLASS);
    expect(tester, GET_IND(reg_list[1].class.mask, 'a'));
    expect(tester, GET_IND(reg_list[1].class.mask, 'b'));
    expect(tester, GET_IND(reg_list[1].class.mask, 'c'));
    expect(tester, reg_list[1].nccl == 0);

    expect(tester, reg_list[2].type == STAR);
    expect(tester, reg_list[2].class.c == '*');
    expect(tester, reg_list[2].nccl == 0);
    re_free(reg_list);

    log_tests(tester);
}

int main() {
    test_regex_compile_naive();
    test_naive_regex();
    test_regex_escape();
    test_regex_some_many();
    test_regex_compile_ccl();

    return 0;
}
