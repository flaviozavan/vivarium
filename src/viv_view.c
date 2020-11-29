#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "viv_view.h"

#include "viv_types.h"

void viv_view_bring_to_front(struct viv_view *view) {
    struct wl_list *link = &view->workspace_link;
    wl_list_remove(link);
    wl_list_insert(&view->workspace->views, link);
}

void viv_view_focus(struct viv_view *view, struct wlr_surface *surface) {
	if (view == NULL) {
		return;
	}
	struct viv_server *server = view->server;
	struct wlr_seat *seat = server->seat;
	struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
	if (prev_surface == surface) {
		/* Don't re-focus an already focused surface. */
		return;
	}
	if (prev_surface) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		struct wlr_xdg_surface *previous = wlr_xdg_surface_from_wlr_surface(
					seat->keyboard_state.focused_surface);
		wlr_xdg_toplevel_set_activated(previous, false);
	}
	struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

	/* Activate the new surface */
	wlr_xdg_toplevel_set_activated(view->xdg_surface, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	wlr_seat_keyboard_notify_enter(seat, view->xdg_surface->surface,
		keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);

    view->workspace->active_view = view;
}

void viv_view_ensure_floating(struct viv_view *view) {
    if (!view->is_floating) {
        // If the view is already floating, no additional layout is necessary
        view->workspace->needs_layout = true;
    }
    view->is_floating = true;
}

void viv_view_shift_to_workspace(struct viv_view *view, struct viv_workspace *workspace) {
    if (view->workspace == workspace) {
        wlr_log(WLR_DEBUG, "Asked to shift view to workspace that view was already in, doing nothing");
        return;
    }

    struct viv_workspace *cur_workspace = view->workspace;

    struct viv_view *next_view = NULL;
    if (wl_list_length(&cur_workspace->views) > 1) {
        struct wl_list *next_view_link = view->workspace_link.next;
        if (next_view_link == &cur_workspace->views) {
            next_view_link = next_view_link->next;
        }
        next_view = wl_container_of(next_view_link, next_view, workspace_link);
    }

    wl_list_remove(&view->workspace_link);
    wl_list_insert(&workspace->views, &view->workspace_link);

    if (next_view != NULL) {
        viv_view_focus(next_view, view->xdg_surface->surface);
    }

    cur_workspace->needs_layout = true;
    workspace->needs_layout = true;

    cur_workspace->active_view = next_view;
    if (workspace->active_view == NULL) {
        workspace->active_view = view;
    }
    view->workspace = workspace;
}
