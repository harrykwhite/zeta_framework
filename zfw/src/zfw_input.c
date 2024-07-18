#include <zfw.h>

#include <string.h>

void zfw_reset_gamepad_state(zfw_input_state_t *const input_state)
{
    input_state->gamepad_glfw_joystick_index = -1;

    input_state->gamepad_buttons_down_bits = 0;
    memset(&input_state->gamepad_axis_values, 0, sizeof(input_state->gamepad_axis_values));

    zfw_log("Gamepad state reset.");
}
