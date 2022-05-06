#include "regex.h"
#include "regex-private.h"

#include "testing-logger.h"

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

int main() {
    test_naive_regex();
    test_regex_escape();
    test_regex_some_many();

    return 0;
}
