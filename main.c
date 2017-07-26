#include <stdio.h>
#include <jit/jit.h>
#include <jit/jit-dump.h>

int native_mult(int x, int y) {
    return x * y;
}

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

jit_function_t get_adder(jit_context_t context) {
    jit_context_build_start(context);

    jit_type_t params[] = {jit_type_int, jit_type_int};
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_int, params, 2, 1);

    jit_function_t function = jit_function_create(context, signature);
    jit_type_free(signature);

    jit_value_t x = jit_value_get_param(function, 0);
    jit_value_t y = jit_value_get_param(function, 1);
    jit_value_t sum = jit_insn_add(function, x, y);

    jit_insn_return(function, sum);

    jit_context_build_end(context);

    return function;
}

jit_function_t get_mix(jit_context_t context, jit_function_t jit_adder) {
    jit_context_build_start(context);

    jit_type_t params[] = {jit_type_int, jit_type_int, jit_type_void_ptr};
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, 3, 1);
    jit_function_t function = jit_function_create(context, signature);
    jit_type_free(signature);

    jit_value_t x = jit_value_get_param(function, 0);
    jit_value_t y = jit_value_get_param(function, 1);
    jit_value_t result = jit_value_get_param(function, 2);

    jit_value_t adder_args[] = {x, y};
    jit_value_t adder_result = jit_insn_call(function, "jit_adder", jit_adder, 0, adder_args, 2, 0);

    jit_type_t mult_params[] = {jit_type_int, jit_type_int};
    jit_type_t mult_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_int, mult_params, 2, 1);

    jit_value_t mult_args[] = {adder_result, y};
    jit_value_t res = jit_insn_call_native(function, "native_mult", native_mult, mult_signature, mult_args, 2, JIT_CALL_NOTHROW);

    jit_insn_store_relative(function, result, 0, res);

    jit_context_build_end(context);

    return function;
}

int main(int argc, char **argv) {
	jit_context_t context = jit_context_create();

    jit_function_t function = get_adder(context);
    jit_function_compile(function);

    void *args[2];
    jit_int arg1, arg2;

    arg1 = 1523;
    arg2 = 3023;

    args[0] = &arg1;
    args[1] = &arg2;

    jit_int result;
    jit_function_apply(function, args, &result);

    printf("gcd(%d, %d) = %d\n", arg1, arg2, (int)result);

    jit_function_t mix = get_mix(context, function);
    jit_int mix_result, *mix_result_ptr = &mix_result;
    void *mix_args[3];
    jit_int a = 50;
    jit_int b = 6;

    mix_args[0] = &a;
    mix_args[1] = &b;
    mix_args[2] = &mix_result_ptr;

    jit_dump_function(stdout, mix, "mix");
    jit_function_compile(mix);

    jit_function_apply(mix, mix_args, 0);
    printf("mix(%d, %d) = %d\n", a, b, (int)mix_result);

    jit_context_destroy(context);
    return 0;
}
