#include "regex.h"

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

    log_tests(tester);
}

int main() {
    test_naive_regex();

    return 0;
}
