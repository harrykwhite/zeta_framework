#include <zfw_input.h>

#include <string.h>
#include <GLFW/glfw3.h>
#include <zfw_common_debug.h>

void zfw_update_gamepad_state(zfw_input_state_t *const input_state)
{
    if (input_state->gamepad_glfw_joystick_index != -1)
    {
        GLFWgamepadstate glfw_gamepad_state;

        if (glfwGetGamepadState(input_state->gamepad_glfw_joystick_index, &glfw_gamepad_state))
        {
            input_state->gamepad_buttons_down_bits = 0;

            for (int i = 0; i < ZFW_GAMEPAD_BUTTON_CODE_COUNT; i++)
            {
                input_state->gamepad_buttons_down_bits |= (zfw_gamepad_buttons_down_bits_t)(glfw_gamepad_state.buttons[i] == GLFW_PRESS) << i;
            }

            for (int i = 0; i < ZFW_GAMEPAD_AXIS_CODE_COUNT; i++)
            {
                input_state->gamepad_axis_values[i] = glfw_gamepad_state.axes[i];
            }
        }
        else
        {
            zfw_log_error("Failed to retrieve the state of gamepad with GLFW joystick index %d.", input_state->gamepad_glfw_joystick_index);
            zfw_reset_gamepad_state(&input_state);
        }
    }
}

void zfw_reset_gamepad_state(zfw_input_state_t *const input_state)
{
    input_state->gamepad_glfw_joystick_index = -1;

    input_state->gamepad_buttons_down_bits = 0;
    memset(&input_state->gamepad_axis_values, 0, sizeof(input_state->gamepad_axis_values));

    zfw_log("Gamepad state reset.");
}

zfw_key_code_t zfw_get_glfw_to_zfw_key_code(const int glfw_key_code)
{
    switch (glfw_key_code)
    {
        case GLFW_KEY_SPACE: return ZFW_KEY_CODE__SPACE;

        case GLFW_KEY_0: return ZFW_KEY_CODE__0;
        case GLFW_KEY_1: return ZFW_KEY_CODE__1;
        case GLFW_KEY_2: return ZFW_KEY_CODE__2;
        case GLFW_KEY_3: return ZFW_KEY_CODE__3;
        case GLFW_KEY_4: return ZFW_KEY_CODE__4;
        case GLFW_KEY_5: return ZFW_KEY_CODE__5;
        case GLFW_KEY_6: return ZFW_KEY_CODE__6;
        case GLFW_KEY_7: return ZFW_KEY_CODE__7;
        case GLFW_KEY_8: return ZFW_KEY_CODE__8;
        case GLFW_KEY_9: return ZFW_KEY_CODE__9;

        case GLFW_KEY_A: return ZFW_KEY_CODE__A;
        case GLFW_KEY_B: return ZFW_KEY_CODE__B;
        case GLFW_KEY_C: return ZFW_KEY_CODE__C;
        case GLFW_KEY_D: return ZFW_KEY_CODE__D;
        case GLFW_KEY_E: return ZFW_KEY_CODE__E;
        case GLFW_KEY_F: return ZFW_KEY_CODE__F;
        case GLFW_KEY_G: return ZFW_KEY_CODE__G;
        case GLFW_KEY_H: return ZFW_KEY_CODE__H;
        case GLFW_KEY_I: return ZFW_KEY_CODE__I;
        case GLFW_KEY_J: return ZFW_KEY_CODE__J;
        case GLFW_KEY_K: return ZFW_KEY_CODE__K;
        case GLFW_KEY_L: return ZFW_KEY_CODE__L;
        case GLFW_KEY_M: return ZFW_KEY_CODE__M;
        case GLFW_KEY_N: return ZFW_KEY_CODE__N;
        case GLFW_KEY_O: return ZFW_KEY_CODE__O;
        case GLFW_KEY_P: return ZFW_KEY_CODE__P;
        case GLFW_KEY_Q: return ZFW_KEY_CODE__Q;
        case GLFW_KEY_R: return ZFW_KEY_CODE__R;
        case GLFW_KEY_S: return ZFW_KEY_CODE__S;
        case GLFW_KEY_T: return ZFW_KEY_CODE__T;
        case GLFW_KEY_U: return ZFW_KEY_CODE__U;
        case GLFW_KEY_V: return ZFW_KEY_CODE__V;
        case GLFW_KEY_W: return ZFW_KEY_CODE__W;
        case GLFW_KEY_X: return ZFW_KEY_CODE__X;
        case GLFW_KEY_Y: return ZFW_KEY_CODE__Y;
        case GLFW_KEY_Z: return ZFW_KEY_CODE__Z;

        case GLFW_KEY_ESCAPE: return ZFW_KEY_CODE__ESCAPE;
        case GLFW_KEY_ENTER: return ZFW_KEY_CODE__ENTER;
        case GLFW_KEY_TAB: return ZFW_KEY_CODE__TAB;

        case GLFW_KEY_RIGHT: return ZFW_KEY_CODE__RIGHT;
        case GLFW_KEY_LEFT: return ZFW_KEY_CODE__LEFT;
        case GLFW_KEY_DOWN: return ZFW_KEY_CODE__DOWN;
        case GLFW_KEY_UP: return ZFW_KEY_CODE__UP;

        case GLFW_KEY_F1: return ZFW_KEY_CODE__F1;
        case GLFW_KEY_F2: return ZFW_KEY_CODE__F2;
        case GLFW_KEY_F3: return ZFW_KEY_CODE__F3;
        case GLFW_KEY_F4: return ZFW_KEY_CODE__F4;
        case GLFW_KEY_F5: return ZFW_KEY_CODE__F5;
        case GLFW_KEY_F6: return ZFW_KEY_CODE__F6;
        case GLFW_KEY_F7: return ZFW_KEY_CODE__F7;
        case GLFW_KEY_F8: return ZFW_KEY_CODE__F8;
        case GLFW_KEY_F9: return ZFW_KEY_CODE__F9;
        case GLFW_KEY_F10: return ZFW_KEY_CODE__F10;
        case GLFW_KEY_F11: return ZFW_KEY_CODE__F11;
        case GLFW_KEY_F12: return ZFW_KEY_CODE__F12;

        case GLFW_KEY_LEFT_SHIFT: return ZFW_KEY_CODE__LEFT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return ZFW_KEY_CODE__LEFT_CONTROL;
        case GLFW_KEY_LEFT_ALT: return ZFW_KEY_CODE__LEFT_ALT;
    }

    return -1;
}
