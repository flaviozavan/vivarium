#ifndef VIV_CONFIG_H
#define VIV_CONFIG_H

#include <stdlib.h>

#include "wlr/types/wlr_keyboard.h"
#include <wlr/util/log.h>

#include "viv_types.h"
#include "viv_layout.h"
#include "viv_workspace.h"
#include "viv_mappable_functions.h"

#include "viv_config_support.h"

#define CONFIG_WIN_MOVE_BUTTON VIV_LEFT_BUTTON
#define CONFIG_WIN_RESIZE_BUTTON VIV_RIGHT_BUTTON

#define CONFIG_SPACER_INCREMENT 0.05

#define CONFIG_TERMINAL "alacritty"

#define CONFIG_LAYOUT_PARAMETER_DEFAULT 0.666
#define CONFIG_LAYOUT_COUNTER_DEFAULT 1

#define CONFIG_BORDER_WIDTH_DEFAULT 2
#define CONFIG_BORDER_COLOUR_ACTIVE_DEFAULT {1, 0, 0.7, 1}
#define CONFIG_BORDER_COLOUR_INACTIVE_DEFAULT {0.6, 0.6, 0.9, 1}

#define CONFIG_CLEAR_COLOUR_DEFAULT {0.3, 0.3, 0.3, 1.0}

struct viv_keybind {
    xkb_keysym_t key;
    void (*binding)(struct viv_workspace *workspace, union viv_mappable_payload payload);
    union viv_mappable_payload payload;
};

/// Example to demonstrate how a user-defined function can be passed as a keybinding
void example_user_function(struct viv_workspace *workspace) {
    wlr_log(WLR_INFO, "User-provided function called with workspace at address %p", workspace);
}

struct viv_keybind the_keybinds[] = {
    KEYBIND_MAPPABLE(q, terminate),
    KEYBIND_MAPPABLE(Return, do_exec, .executable = CONFIG_TERMINAL),
    /* KEYBIND_MAPPABLE(w, do_exec, .executable = "weston-terminal"), */
    KEYBIND_MAPPABLE(l, increment_divide, .increment = CONFIG_SPACER_INCREMENT),
    KEYBIND_MAPPABLE(h, increment_divide, .increment = -CONFIG_SPACER_INCREMENT),
    KEYBIND_MAPPABLE(j, next_window),
    KEYBIND_MAPPABLE(k, prev_window),
    KEYBIND_MAPPABLE(J, shift_active_window_down),
    KEYBIND_MAPPABLE(K, shift_active_window_up),
    KEYBIND_MAPPABLE(s, swap_out),
    KEYBIND_MAPPABLE(t, tile_window),
    KEYBIND_MAPPABLE(e, right_output),
    KEYBIND_MAPPABLE(w, left_output),
    KEYBIND_MAPPABLE(o, do_shell, .command = "okular"),
    KEYBIND_MAPPABLE(space, next_layout),
    KEYBIND_USER_FUNCTION(F, &example_user_function),
    KEYBIND_MAPPABLE(exclam, shift_active_window_to_workspace, .workspace_name = "1"),
    KEYBIND_MAPPABLE(quotedbl, shift_active_window_to_workspace, .workspace_name = "2"),
    KEYBIND_MAPPABLE(E, shift_active_window_to_right_output),
    KEYBIND_MAPPABLE(W, shift_active_window_to_left_output),
    TERMINATE_KEYBINDS_LIST()
};

struct viv_layout the_layouts[] = {
    {
        .name = "Tall",
        .layout_function = &viv_layout_do_split,
        .parameter = CONFIG_LAYOUT_PARAMETER_DEFAULT,
        .counter = CONFIG_LAYOUT_COUNTER_DEFAULT,
    },
    {
        .name = "Fullscreen",
        .layout_function = &viv_layout_do_fullscreen,
        .parameter = CONFIG_LAYOUT_PARAMETER_DEFAULT,
        .counter = CONFIG_LAYOUT_COUNTER_DEFAULT,
    },
    TERMINATE_LAYOUTS_LIST()
};

static struct viv_config the_config = {
    .focus_follows_mouse = true,
    .global_meta_key = WLR_MODIFIER_ALT,

    .win_move_cursor_button = VIV_LEFT_BUTTON,
    .win_resize_cursor_button = VIV_RIGHT_BUTTON,

    .keybinds = the_keybinds,

    .layouts = the_layouts,

    .workspaces = { "1", "2", "3" },

    .border_width = CONFIG_BORDER_WIDTH_DEFAULT,
    .active_border_colour = CONFIG_BORDER_COLOUR_ACTIVE_DEFAULT,
    .inactive_border_colour = CONFIG_BORDER_COLOUR_INACTIVE_DEFAULT,
    .clear_colour = CONFIG_CLEAR_COLOUR_DEFAULT,

	.xkb_rules = {
        .model = "pc104",
        .layout = "widecolemak",
        .variant = "widecolemak",
        .options = "ctrl:nocaps",
    },
};


#endif
