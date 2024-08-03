#ifndef __ZFW_INPUT_H__
#define __ZFW_INPUT_H__

#include <zfw_common.h>

typedef enum
{
    ZFW_KEY_CODE__SPACE,

    ZFW_KEY_CODE__0,
    ZFW_KEY_CODE__1,
    ZFW_KEY_CODE__2,
    ZFW_KEY_CODE__3,
    ZFW_KEY_CODE__4,
    ZFW_KEY_CODE__5,
    ZFW_KEY_CODE__6,
    ZFW_KEY_CODE__7,
    ZFW_KEY_CODE__8,
    ZFW_KEY_CODE__9,

    ZFW_KEY_CODE__A,
    ZFW_KEY_CODE__B,
    ZFW_KEY_CODE__C,
    ZFW_KEY_CODE__D,
    ZFW_KEY_CODE__E,
    ZFW_KEY_CODE__F,
    ZFW_KEY_CODE__G,
    ZFW_KEY_CODE__H,
    ZFW_KEY_CODE__I,
    ZFW_KEY_CODE__J,
    ZFW_KEY_CODE__K,
    ZFW_KEY_CODE__L,
    ZFW_KEY_CODE__M,
    ZFW_KEY_CODE__N,
    ZFW_KEY_CODE__O,
    ZFW_KEY_CODE__P,
    ZFW_KEY_CODE__Q,
    ZFW_KEY_CODE__R,
    ZFW_KEY_CODE__S,
    ZFW_KEY_CODE__T,
    ZFW_KEY_CODE__U,
    ZFW_KEY_CODE__V,
    ZFW_KEY_CODE__W,
    ZFW_KEY_CODE__X,
    ZFW_KEY_CODE__Y,
    ZFW_KEY_CODE__Z,

    ZFW_KEY_CODE__ESCAPE,
    ZFW_KEY_CODE__ENTER,
    ZFW_KEY_CODE__TAB,

    ZFW_KEY_CODE__RIGHT,
    ZFW_KEY_CODE__LEFT,
    ZFW_KEY_CODE__DOWN,
    ZFW_KEY_CODE__UP,

    ZFW_KEY_CODE__F1,
    ZFW_KEY_CODE__F2,
    ZFW_KEY_CODE__F3,
    ZFW_KEY_CODE__F4,
    ZFW_KEY_CODE__F5,
    ZFW_KEY_CODE__F6,
    ZFW_KEY_CODE__F7,
    ZFW_KEY_CODE__F8,
    ZFW_KEY_CODE__F9,
    ZFW_KEY_CODE__F10,
    ZFW_KEY_CODE__F11,
    ZFW_KEY_CODE__F12,

    ZFW_KEY_CODE__LEFT_SHIFT,
    ZFW_KEY_CODE__LEFT_CONTROL,
    ZFW_KEY_CODE__LEFT_ALT,

    ZFW_KEY_CODE_COUNT
} zfw_key_code_t;

typedef enum
{
    ZFW_MOUSE_BUTTON_CODE__LEFT,
    ZFW_MOUSE_BUTTON_CODE__RIGHT,
    ZFW_MOUSE_BUTTON_CODE__MIDDLE,

    ZFW_MOUSE_BUTTON_CODE_COUNT
} zfw_mouse_button_code_t;

typedef enum
{
    ZFW_GAMEPAD_BUTTON_CODE__A,
    ZFW_GAMEPAD_BUTTON_CODE__B,
    ZFW_GAMEPAD_BUTTON_CODE__X,
    ZFW_GAMEPAD_BUTTON_CODE__Y,

    ZFW_GAMEPAD_BUTTON_CODE__LEFT_BUMPER,
    ZFW_GAMEPAD_BUTTON_CODE__RIGHT_BUMPER,

    ZFW_GAMEPAD_BUTTON_CODE__BACK,
    ZFW_GAMEPAD_BUTTON_CODE__START,
    ZFW_GAMEPAD_BUTTON_CODE__GUIDE,

    ZFW_GAMEPAD_BUTTON_CODE__LEFT_THUMB,
    ZFW_GAMEPAD_BUTTON_CODE__RIGHT_THUMB,

    ZFW_GAMEPAD_BUTTON_CODE__DPAD_UP,
    ZFW_GAMEPAD_BUTTON_CODE__DPAD_RIGHT,
    ZFW_GAMEPAD_BUTTON_CODE__DPAD_DOWN,
    ZFW_GAMEPAD_BUTTON_CODE__DPAD_LEFT,

    ZFW_GAMEPAD_BUTTON_CODE_COUNT
} zfw_gamepad_button_code_t;

typedef enum
{
    ZFW_GAMEPAD_AXIS_CODE__LEFT_X,
    ZFW_GAMEPAD_AXIS_CODE__LEFT_Y,

    ZFW_GAMEPAD_AXIS_CODE__RIGHT_X,
    ZFW_GAMEPAD_AXIS_CODE__RIGHT_Y,

    ZFW_GAMEPAD_AXIS_CODE__LEFT_TRIGGER,
    ZFW_GAMEPAD_AXIS_CODE__RIGHT_TRIGGER,

    ZFW_GAMEPAD_AXIS_CODE_COUNT
} zfw_gamepad_axis_code_t;

typedef unsigned long long zfw_keys_down_bits_t;
typedef unsigned char zfw_mouse_buttons_down_bits_t;
typedef unsigned short zfw_gamepad_buttons_down_bits_t;

typedef struct
{
    zfw_keys_down_bits_t keys_down_bits;
    zfw_mouse_buttons_down_bits_t mouse_buttons_down_bits;
    zfw_gamepad_buttons_down_bits_t gamepad_buttons_down_bits;

    zfw_vec_2d_t mouse_pos;
    int mouse_scroll;

    int gamepad_glfw_joystick_index;
    float gamepad_axis_values[ZFW_GAMEPAD_AXIS_CODE_COUNT];
} zfw_input_state_t;

void zfw_reset_gamepad_state(zfw_input_state_t *const input_state);
zfw_key_code_t zfw_get_glfw_to_zfw_key_code(const int glfw_key_code);

inline zfw_bool_t zfw_is_key_down(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state)
{
    return (input_state->keys_down_bits & ((zfw_keys_down_bits_t)1 << key_code)) != 0;
}

inline zfw_bool_t zfw_is_key_pressed(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_key_down(key_code, input_state) && !zfw_is_key_down(key_code, input_state_last);
}

inline zfw_bool_t zfw_is_key_released(const zfw_key_code_t key_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_key_down(key_code, input_state) && zfw_is_key_down(key_code, input_state_last);
}

inline zfw_bool_t zfw_is_mouse_button_down(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state)
{
    return (input_state->mouse_buttons_down_bits & ((zfw_mouse_buttons_down_bits_t)1 << button_code)) != 0;
}

inline zfw_bool_t zfw_is_mouse_button_pressed(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_mouse_button_down(button_code, input_state) && !zfw_is_mouse_button_down(button_code, input_state_last);
}

inline zfw_bool_t zfw_is_mouse_button_released(const zfw_mouse_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_mouse_button_down(button_code, input_state) && zfw_is_mouse_button_down(button_code, input_state_last);
}

inline zfw_bool_t zfw_is_gamepad_button_down(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state)
{
    return (input_state->gamepad_buttons_down_bits & ((zfw_gamepad_buttons_down_bits_t)1 << button_code)) != 0;
}

inline zfw_bool_t zfw_is_gamepad_button_pressed(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return zfw_is_gamepad_button_down(button_code, input_state) && !zfw_is_gamepad_button_down(button_code, input_state_last);
}

inline zfw_bool_t zfw_is_gamepad_button_released(const zfw_gamepad_button_code_t button_code, const zfw_input_state_t *const input_state, const zfw_input_state_t *const input_state_last)
{
    return !zfw_is_gamepad_button_down(button_code, input_state) && zfw_is_gamepad_button_down(button_code, input_state_last);
}

#endif

