/* exm-window.c
 *
 * Copyright 2022 Matthew Jakeman
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "exm-config.h"
#include "exm-window.h"

#include "model/exm-manager.h"
#include "model/exm-extension.h"

#include <adwaita.h>

struct _ExmWindow
{
    GtkApplicationWindow  parent_instance;

    ExmManager *manager;

    /* Template widgets */
    AdwHeaderBar        *header_bar;
    GtkListBox          *list_box;
};

G_DEFINE_TYPE (ExmWindow, exm_window, GTK_TYPE_APPLICATION_WINDOW)

static void
exm_window_class_init (ExmWindowClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    gtk_widget_class_set_template_from_resource (widget_class, "/com/mattjakeman/ExtensionManager/exm-window.ui");
    gtk_widget_class_bind_template_child (widget_class, ExmWindow, header_bar);
    gtk_widget_class_bind_template_child (widget_class, ExmWindow, list_box);
}

static gboolean
extension_state_set (GtkSwitch    *toggle,
                     gboolean      state,
                     ExmExtension *extension)
{
    GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (toggle));
    ExmWindow *self = EXM_WINDOW (root);

    if (state)
        exm_manager_enable_extension (self->manager, extension);
    else
        exm_manager_disable_extension (self->manager, extension);

    return FALSE;
}

static GtkWidget *
widget_factory (ExmExtension* extension)
{
    GtkWidget *row;
    GtkWidget *label;
    GtkWidget *toggle;

    gchar *name, *uuid, *description;
    gboolean enabled;
    g_object_get (extension,
                  "display-name", &name,
                  "uuid", &uuid,
                  "description", &description,
                  "enabled", &enabled,
                  NULL);

    name = g_markup_escape_text (name, -1);

    row = adw_expander_row_new ();

    adw_preferences_row_set_title (ADW_PREFERENCES_ROW (row), name);
    adw_expander_row_set_subtitle (ADW_EXPANDER_ROW (row), uuid);

    toggle = gtk_switch_new ();
    gtk_switch_set_state (GTK_SWITCH (toggle), enabled);
    gtk_widget_set_valign (toggle, GTK_ALIGN_CENTER);
    gtk_widget_set_halign (toggle, GTK_ALIGN_CENTER);
    adw_expander_row_add_action (ADW_EXPANDER_ROW (row), toggle);
    g_signal_connect (toggle, "state-set", G_CALLBACK (extension_state_set), extension);

    label = gtk_label_new (description);
    gtk_label_set_xalign (GTK_LABEL (label), 0);
    gtk_label_set_wrap (GTK_LABEL (label), GTK_WRAP_WORD);
    gtk_widget_add_css_class (label, "description-label");
    adw_expander_row_add_row (ADW_EXPANDER_ROW (row), label);

    return row;
}

static void
exm_window_init (ExmWindow *self)
{
    GListModel *model;

    gtk_widget_init_template (GTK_WIDGET (self));

    self->manager = exm_manager_new ();
    g_object_get (self->manager, "list-model", &model, NULL);

    // TODO: Recreate/Update model whenever extensions are changed
    gtk_list_box_bind_model (self->list_box, model,
                             (GtkListBoxCreateWidgetFunc)widget_factory,
                             NULL, NULL);
}