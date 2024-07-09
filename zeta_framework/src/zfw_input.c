#include <zfw.h>

#include <string.h>

void zfw_reset_gamepad_state(zfw_input_state_t *const input_state)
{
    input_state->gamepad_glfw_joystick_index = -1;

    input_state->gamepad_buttons_down_bits = 0;
    memset(&input_state->gamepad_axis_values, 0, sizeof(input_state->gamepad_axis_values));

    zfw_log("Gamepad state reset.");
}

zfw_bool_t zfw_is_key_down(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state)
{
    return (input_state->keys_down_bits & ((zfw_keys_down_bits_t)1 << key_code)) != 0;
}

zfw_bool_t zfw_is_key_pressed(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_key_down(key_code, input_state) && !zfw_is_key_down(key_code, input_state_last);
}

zfw_bool_t zfw_is_key_released(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_key_down(key_code, input_state) && zfw_is_key_down(key_code, input_state_last);
}

zfw_bool_t zfw_is_mouse_button_down(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state)
{
    return (input_state->mouse_buttons_down_bits & ((zfw_mouse_buttons_down_bits_t)1 << button_code)) != 0;
}

zfw_bool_t zfw_is_mouse_button_pressed(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_mouse_button_down(button_code, input_state) && !zfw_is_mouse_button_down(button_code, input_state_last);
}

zfw_bool_t zfw_is_mouse_button_released(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_mouse_button_down(button_code, input_state) && zfw_is_mouse_button_down(button_code, input_state_last);
}

zfw_bool_t zfw_is_gamepad_button_down(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state)
{
    return (input_state->gamepad_buttons_down_bits & ((zfw_gamepad_buttons_down_bits_t)1 << button_code)) != 0;
}

zfw_bool_t zfw_is_gamepad_button_pressed(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_gamepad_button_down(button_code, input_state) && !zfw_is_gamepad_button_down(button_code, input_state_last);
}

zfw_bool_t zfw_is_gamepad_button_released(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_gamepad_button_down(button_code, input_state) && zfw_is_gamepad_button_down(button_code, input_state_last);
}
