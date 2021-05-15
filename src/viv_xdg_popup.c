
#include "viv_damage.h"
#include "viv_output.h"
#include "viv_types.h"

#include "viv_xdg_popup.h"
#include "viv_wlr_surface_tree.h"

/// Add to x and y the global (i.e. output-layout) coords of the input popup, calculated
/// by walking up the popup tree and adding the geometry of each parent.
static void add_popup_global_coords(void *popup_pointer, int *x, int *y) {
    struct viv_xdg_popup *popup = popup_pointer;

    int px = 0;
    int py = 0;

    struct viv_xdg_popup *cur_popup = popup;
    while (true) {
        px += cur_popup->wlr_popup->geometry.x;
        py += cur_popup->wlr_popup->geometry.y;

        if (cur_popup->parent_popup != NULL) {
            cur_popup = cur_popup->parent_popup;
        } else {
            break;
        }
    }

    px += *popup->lx;
    py += *popup->ly;

    *x += px;
    *y += py;
}

static void handle_popup_surface_unmap(struct wl_listener *listener, void *data) {
    UNUSED(data);
    struct viv_xdg_popup *popup = wl_container_of(listener, popup, surface_unmap);

    int px = 0;
    int py = 0;
    add_popup_global_coords(popup, &px, &py);

    struct wlr_box geo_box = {
        .x = px,
        .y = py,
        .width = popup->wlr_popup->geometry.width,
        .height = popup->wlr_popup->geometry.height,
    };

    struct viv_output *output;
    wl_list_for_each(output, &popup->server->outputs, link) {
        viv_output_damage_layout_coords_box(output, &geo_box);
    }
}

static void handle_popup_surface_destroy(struct wl_listener *listener, void *data) {
    UNUSED(data);
    struct viv_xdg_popup *popup = wl_container_of(listener, popup, destroy);
    wlr_log(WLR_INFO, "Popup being destroyed");
    free(popup);
}

static void handle_new_popup(struct wl_listener *listener, void *data) {
    struct viv_xdg_popup *popup = wl_container_of(listener, popup, new_popup);
	struct wlr_xdg_popup *wlr_popup = data;

    struct viv_xdg_popup *new_popup = calloc(1, sizeof(struct viv_xdg_popup));
    new_popup->server = popup->server;
    new_popup->lx = popup->lx;
    new_popup->ly = popup->ly;
    new_popup->parent_popup = popup;
    viv_xdg_popup_init(new_popup, wlr_popup);
}

void viv_xdg_popup_init(struct viv_xdg_popup *popup, struct wlr_xdg_popup *wlr_popup) {
    popup->wlr_popup = wlr_popup;

    wlr_log(WLR_INFO, "New popup %p with parent %p", popup, popup->parent_popup);

    viv_surface_tree_root_create(popup->server, wlr_popup->base->surface, &add_popup_global_coords, popup);

    popup->surface_unmap.notify = handle_popup_surface_unmap;
    wl_signal_add(&wlr_popup->base->events.unmap, &popup->surface_unmap);

    popup->destroy.notify = handle_popup_surface_destroy;
    wl_signal_add(&wlr_popup->base->surface->events.destroy, &popup->destroy);

    popup->new_popup.notify = handle_new_popup;
    wl_signal_add(&wlr_popup->base->events.new_popup, &popup->new_popup);

    wlr_log(WLR_INFO, "New wlr_popup surface at %p", wlr_popup->base->surface);
}