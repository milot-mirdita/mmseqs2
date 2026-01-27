#include <iostream>
#include <cassert>
#include <cmath>
#include <chrono>

#include "ExpressionParser.h"

const char* binary_name = "test_tinyexpr";

static void assert_ok(ExpressionParser &expression, const char *expr) {
    if (!expression.isOk()) {
        std::cerr << "Failed to parse expression: " << expr << std::endl;
        assert(false);
    }
}

static void assert_eval(ExpressionParser &expression, const char *expr, double expected, double eps = 1e-12) {
    assert_ok(expression, expr);
    const double result = expression.evaluate();
    if (std::fabs(result - expected) > eps) {
        std::cerr << "Unexpected result for '" << expr << "': " << result << " (expected " << expected << ")\n";
        assert(false);
    }
}

static void assert_parse_fail(const char *expr) {
    ExpressionParser expression(expr);
    if (expression.isOk()) {
        std::cerr << "Expected parse failure for expression: " << expr << std::endl;
        assert(false);
    }
}

static double plus_ctx(void *ctx, double x) {
    const double *bias = static_cast<const double *>(ctx);
    return *bias + x;
}

static double time_eval(te_expr *expr, int iterations) {
    const auto start = std::chrono::high_resolution_clock::now();
    double sink = 0.0;
    for (int i = 0; i < iterations; ++i) {
        sink += te_eval(expr);
    }
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::micro> elapsed = end - start;
    if (sink == 123456789.0) {
        std::cout << "sink=" << sink << std::endl;
    }
    return elapsed.count();
}

static void assert_same_result(const char *expr, te_variable *vars, size_t var_count) {
    int err = 0;
    te_expr *compiled = te_compile(expr, vars, var_count, &err);
    assert(err == 0 && compiled != NULL);
    const double before = te_eval(compiled);
    ej_optimize(compiled);
    const double after = te_eval(compiled);
    if (std::isnan(before) && std::isnan(after)) {
        // ok
    } else if (std::fabs(before - after) > 1e-12) {
        std::cerr << "Optimize mismatch for '" << expr << "': " << before << " vs " << after << std::endl;
        assert(false);
    }
    te_free(compiled);
}

int main (int, const char**) {
    {
        ExpressionParser expression("sqrt($11^2+$2^2)");
        assert_ok(expression, "sqrt($11^2+$2^2)");
        std::vector<int> indices = expression.findBindableIndices();
        assert(indices.size() == 2);
        assert(indices[0] == 2);
        assert(indices[1] == 11);
        expression.bind(11, 3);
        expression.bind(2, 4);
        const double result = expression.evaluate();
        std::cout << result << std::endl;
        assert(std::fabs(result - 5.0) < 1e-12);
    }

    {
        ExpressionParser expression("$1 + $2 * 3");
        assert_ok(expression, "$1 + $2 * 3");
        expression.bind(1, 2);
        expression.bind(2, 4);
        assert_eval(expression, "$1 + $2 * 3", 14.0);
    }

    {
        ExpressionParser expression("2^3 + 1");
        assert_eval(expression, "2^3 + 1", 9.0);
    }

    {
        ExpressionParser expression("1 < 2");
        assert_eval(expression, "1 < 2", 1.0);
        ExpressionParser expression2("2 <= 2");
        assert_eval(expression2, "2 <= 2", 1.0);
        ExpressionParser expression3("2 == 3");
        assert_eval(expression3, "2 == 3", 0.0);
        ExpressionParser expression4("2 != 3");
        assert_eval(expression4, "2 != 3", 1.0);
    }

    {
        ExpressionParser expression("1 && 0");
        assert_eval(expression, "1 && 0", 0.0);
        ExpressionParser expression2("1 || 0");
        assert_eval(expression2, "1 || 0", 1.0);
        ExpressionParser expression3("!0");
        assert_eval(expression3, "!0", 1.0);
    }

    {
        ExpressionParser expression("sin(0) + cos(0) + log(1)");
        assert_eval(expression, "sin(0) + cos(0) + log(1)", 1.0);
    }

    {
        te_variable var;
        var.name = "x";
        double x = 7.0;
        var.address = &x;
        var.type = TE_VARIABLE;
        var.context = NULL;
        var.bound = 0;
        std::vector<te_variable> vars = {var};
        ExpressionParser expression("x + 1", vars);
        assert_eval(expression, "x + 1", 8.0);
    }

    {
        te_variable clo;
        clo.name = "bias";
        double bias = 2.5;
        clo.address = reinterpret_cast<const void*>(&plus_ctx);
        clo.type = TE_CLOSURE1;
        clo.context = &bias;
        clo.bound = 0;
        std::vector<te_variable> vars = {clo};
        ExpressionParser expression("bias(1.5)", vars);
        assert_eval(expression, "bias(1.5)", 4.0);
    }

    {
        ExpressionParser expression("$2 + $2");
        assert_ok(expression, "$2 + $2");
        std::vector<int> indices = expression.findBindableIndices();
        assert(indices.size() == 1);
        assert(indices[0] == 2);
    }

    {
        ExpressionParser nan_expr("0/0");
        assert_ok(nan_expr, "0/0");
        assert(std::isnan(nan_expr.evaluate()));

        ExpressionParser and_true("(0/0) && 1");
        assert_eval(and_true, "(0/0) && 1", 1.0);
        ExpressionParser and_false("(0/0) && 0");
        assert_eval(and_false, "(0/0) && 0", 0.0);
        ExpressionParser or_true("(0/0) || 0");
        assert_eval(or_true, "(0/0) || 0", 1.0);
        ExpressionParser not_nan("!(0/0)");
        assert_eval(not_nan, "!(0/0)", 0.0);

        ExpressionParser eq_nan("(0/0) == (0/0)");
        assert_eval(eq_nan, "(0/0) == (0/0)", 0.0);
        ExpressionParser neq_nan("(0/0) != (0/0)");
        assert_eval(neq_nan, "(0/0) != (0/0)", 1.0);
    }

    {
        int err = 0;
        te_expr *expr = te_compile("2+3*4", NULL, 0, &err);
        assert(err == 0 && expr != NULL);
        ej_optimize(expr);
        const double result = te_eval(expr);
        assert(std::fabs(result - 14.0) < 1e-12);
        te_free(expr);
    }

    {
        int err = 0;
        double value = 5.0;
        te_variable var;
        var.name = "$1";
        var.address = &value;
        var.type = TE_VARIABLE;
        var.context = NULL;
        var.bound = 0;
        te_expr *expr = te_compile("$1 + 0", &var, 1, &err);
        assert(err == 0 && expr != NULL);
        ej_optimize(expr);
        const double result = te_eval(expr);
        assert(std::fabs(result - 5.0) < 1e-12);
        te_free(expr);
    }

    {
        int err = 0;
        double v1 = 1.1;
        double v2 = 2.2;
        te_variable vars[] = {
            {"$1", &v1, TE_VARIABLE, NULL, 0},
            {"$2", &v2, TE_VARIABLE, NULL, 0}
        };
        te_expr *expr = te_compile("($1+2)*($2-3)+($1*$2)+42", vars, 2, &err);
        assert(err == 0 && expr != NULL);
        const int iterations = 200000;
        const double before_us = time_eval(expr, iterations);
        ej_optimize(expr);
        const double after_us = time_eval(expr, iterations);
        std::cout << "optimizer timing (us): before=" << before_us << " after=" << after_us << std::endl;
        te_free(expr);
    }

    {
        int err = 0;
        te_expr *expr = te_compile("sin(0)+cos(0)+log(1)+sqrt(16)+2^3", NULL, 0, &err);
        assert(err == 0 && expr != NULL);
        const int iterations = 200000;
        const double before_us = time_eval(expr, iterations);
        ej_optimize(expr);
        const double after_us = time_eval(expr, iterations);
        std::cout << "optimizer const timing (us): before=" << before_us << " after=" << after_us << std::endl;
        te_free(expr);
    }

    {
        assert_same_result("2+3*4", NULL, 0);
        assert_same_result("(0/0) && 1", NULL, 0);
        assert_same_result("(0/0) != (0/0)", NULL, 0);

        double a = 3.5;
        double b = -2.0;
        te_variable vars[] = {
            {"$1", &a, TE_VARIABLE, NULL, 0},
            {"$2", &b, TE_VARIABLE, NULL, 0}
        };
        assert_same_result("$1 + $2 * 3 - 4", vars, 2);
        assert_same_result("($1*$1)+($2*$2)", vars, 2);
    }

    assert_parse_fail("1 +");
    assert_parse_fail("((1+2)");
    assert_parse_fail("1 @ 2");
    assert_parse_fail("unknown_var");
    assert_parse_fail("sin");
    assert_parse_fail("x(1)");
    assert_parse_fail("$129 + 1");
}
