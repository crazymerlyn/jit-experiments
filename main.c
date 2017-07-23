#include <stdio.h>
#include <jit/jit.h>

int gcd(jit_function_t function) {
    jit_value_t x = jit_value_get_param(function, 0);
    jit_value_t y = jit_value_get_param(function, 1);

    jit_value_t const0 = jit_value_create_nint_constant(function, jit_type_int, 0);

    jit_value_t y_is_0 = jit_insn_eq(function, y, const0);

    jit_label_t recursion = jit_label_undefined;
    jit_insn_branch_if_not(function, y_is_0, &recursion);
    jit_insn_return(function, x);
    jit_insn_label(function, &recursion);

    jit_value_t temp_args[2];
    temp_args[0] = y;
    temp_args[1] = jit_insn_rem(function, x, y);
    jit_insn_call(function, "gcd", function, 0, temp_args, 2, JIT_CALL_TAIL); 

    return 1;
}

int gcd_iter(jit_function_t function) {
    jit_value_t x = jit_value_get_param(function, 0);
    jit_value_t y = jit_value_get_param(function, 1);

    jit_value_t const0 = jit_value_create_nint_constant(function, jit_type_int, 0);

    jit_label_t after_while = jit_label_undefined;
    jit_label_t before_while = jit_label_undefined;

    jit_insn_label(function, &before_while);

    jit_value_t y_is_0 = jit_insn_eq(function, y, const0);

    jit_insn_branch_if(function, y_is_0, &after_while);
    
    jit_value_t t = jit_value_create(function, jit_type_int);
    jit_insn_store(function, t, x);
    jit_insn_store(function, x, y);
    jit_value_t rem = jit_insn_rem(function, t, y);
    jit_insn_store(function, y, rem);

    jit_insn_branch(function, &before_while);
    jit_insn_label(function, &after_while);

    jit_label_t label_positive = jit_label_undefined;
    jit_value_t is_positive = jit_insn_ge(function, x, const0);
    jit_insn_branch_if(function, is_positive, &label_positive);
    jit_value_t minus_x = jit_insn_neg(function, x);
    jit_insn_return(function, x);
    jit_insn_label(function, &label_positive);
    jit_insn_return(function, x);

    return 1;
}

int main(int argc, char **argv) {
    jit_type_t params[2];
    jit_type_t signature;
    jit_value_t x, y;

	jit_context_t context = jit_context_create();

	jit_context_build_start(context);

    params[0] = jit_type_int;
    params[1] = jit_type_int;

    signature = jit_type_create_signature(jit_abi_cdecl, jit_type_int, params, 2, 1);

    jit_function_t function = jit_function_create(context, signature);
    jit_type_free(signature);

    jit_function_set_on_demand_compiler(function, gcd_iter);

    jit_context_build_end(context);

    void *args[3];
    jit_int arg1, arg2;

    arg1 = 15238942;
    arg2 = 302342346;

    args[0] = &arg1;
    args[1] = &arg2;

    jit_int result;
    jit_function_apply(function, args, &result);

    printf("gcd(%d, %d) = %d\n", arg1, arg2, (int)result);

    jit_context_destroy(context);
    return 0;
}
