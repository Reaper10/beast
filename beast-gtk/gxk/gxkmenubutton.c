/* GXK - Gtk+ Extension Kit
 * Copyright (C) 2003-2004 Tim Janik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "gxkmenubutton.h"
#include "gxkstock.h"
#include "gxkauxwidgets.h"
#include <gdk/gdkkeysyms.h>
#include <string.h>

enum {
  MENU_BUTTON_PROP_0,
  MENU_BUTTON_PROP_MENU,
  MENU_BUTTON_PROP_STOCK_SIZE,
  MENU_BUTTON_PROP_RELIEF,
  MENU_BUTTON_PROP_MODE
};

/* --- prototypes --- */
static void     menu_button_remove_contents             (GxkMenuButton  *self);
static void     menu_button_layout                      (GxkMenuButton  *self);


/* --- variables --- */
static guint menu_button_signal_changed = 0;


/* --- functions --- */
G_DEFINE_TYPE (GxkMenuButton, gxk_menu_button, GTK_TYPE_EVENT_BOX);

static void
menu_button_save_snapshot (GxkMenuButton *self)
{
  GtkWidget *widget;
  GdkPixmap *pixmap;
  GdkGCValues gc_values;
  GdkGC *gc;
  gint width, height;
  gdk_window_get_user_data (self->cslot->parent->window, (void*) &widget);
  gdk_drawable_get_size (widget->window, &width, &height);
  if (!self->bwindow)
    {
      GdkWindowAttr attributes;
      guint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
      attributes.window_type = GDK_WINDOW_CHILD;
      attributes.wclass = GDK_INPUT_OUTPUT;
      attributes.visual = gtk_widget_get_visual (widget);
      attributes.colormap = gtk_widget_get_colormap (widget);
      attributes.x = attributes.y = 0;
      attributes.width = width;
      attributes.height = height;
      attributes.event_mask = 0;
      self->bwindow = gdk_window_new (widget->window, &attributes, attributes_mask);
      gdk_window_set_user_data (self->bwindow, self);
    }
  else
    gdk_window_resize (self->bwindow, width, height);
  gdk_window_process_updates (widget->window, TRUE);
  pixmap = gdk_pixmap_new (widget->window, width, height, -1);
  gc_values.subwindow_mode = GDK_INCLUDE_INFERIORS;
  gc = gdk_gc_new_with_values (widget->window, &gc_values, GDK_GC_SUBWINDOW);
  gdk_draw_drawable (pixmap, gc, widget->window, 0, 0, 0, 0, width, height);
  g_object_unref (gc);
  gdk_window_set_back_pixmap (self->bwindow, pixmap, FALSE);
  g_object_unref (pixmap);
  gdk_window_show (self->bwindow);
  gdk_window_clear (self->bwindow);
}

static void
menu_button_destroy_backing (GxkMenuButton *self)
{
  if (self->bwindow)
    {
      gdk_window_set_user_data (self->bwindow, NULL);
      gdk_window_destroy (self->bwindow);
      self->bwindow = NULL;
    }
}

static void
menu_button_grab_focus (GxkMenuButton *self)
{
  if (self->mode != GXK_MENU_BUTTON_COMBO_MODE && self->button)
    gtk_widget_grab_focus (self->button);
  else
    gtk_widget_grab_focus (GTK_WIDGET (self));
}

static void
menu_button_popup (GxkMenuButton *self,
                   guint          button,
                   guint32        time)
{
  gboolean push_in = self->mode == GXK_MENU_BUTTON_OPTION_MODE;
  gboolean popup_right = self->mode == GXK_MENU_BUTTON_POPUP_MODE;
  gboolean popup_bottom = (self->mode == GXK_MENU_BUTTON_TOOL_MODE ||
                           self->mode == GXK_MENU_BUTTON_COMBO_MODE);
  GdkEvent *event = gtk_get_current_event ();
  GtkWidget *menu_item, *widget = GTK_WIDGET (self);
  gint x, y;
  menu_button_grab_focus (self);
  /* handle expose events and snapshot background */
  menu_button_destroy_backing (self);
  if (self->cslot && GTK_WIDGET_DRAWABLE (self))
    menu_button_save_snapshot (self);
  /* fixate sizes across removing child */
  if (self->islot)
    g_object_set (self->islot,
                  "width-request", self->islot->requisition.width,
                  "height-request", self->islot->requisition.height,
                  NULL);
  menu_button_remove_contents (self);
  gdk_window_get_origin (widget->window, &x, &y);
  if (popup_right)
    x += widget->allocation.width;
  if (popup_bottom)
    y += widget->allocation.height;
  gxk_menu_popup (self->menu, x, y, push_in, button, event ? gdk_event_get_time (event) : 0);
  menu_item = gtk_menu_get_active (self->menu);
  if (menu_item)
    gtk_menu_shell_select_item (GTK_MENU_SHELL (self->menu), menu_item);
}

static gboolean
menu_button_button_press (GtkWidget      *widget,
                          GdkEventButton *event)
{
  GxkMenuButton *self = GXK_MENU_BUTTON (widget);
  menu_button_grab_focus (self);
  if (self->menu && event->type == GDK_BUTTON_PRESS && event->button == 1)
    menu_button_popup (self, event->button, event->time);
  return TRUE;
}

static gboolean
menu_button_key_press (GtkWidget   *widget,
                       GdkEventKey *event)
{
  GxkMenuButton *self = GXK_MENU_BUTTON (widget);
  switch (event->keyval)
    {
    case GDK_KP_Enter: case GDK_Return:
    case GDK_KP_Space: case GDK_space:
      menu_button_popup (self, 0, event->time);
      return TRUE;
    }
  return FALSE;
}

static gboolean
menu_button_mnemonic_activate (GtkWidget *widget,
                               gboolean   group_cycling)
{
  GxkMenuButton *self = GXK_MENU_BUTTON (widget);
  menu_button_grab_focus (self);
  return TRUE;
}

static void
menu_button_proxy_state (GxkMenuButton *self)
{
  if (self->child)
    gtk_widget_set_sensitive (self->child, GTK_WIDGET_IS_SENSITIVE (self->menu_item));
  if (self->image)
    gtk_widget_set_sensitive (self->image, GTK_WIDGET_IS_SENSITIVE (self->menu_item));
}

static void
menu_button_max_size (GxkMenuButton *self)
{
  if (self->child)
    {
      GList *list, *children = GTK_MENU_SHELL (self->menu)->children;
      GtkRequisition child_requisition = { 0, };
      guint width = 0, height = 0;
      for (list = children; list; list = list->next)
        {
          GtkWidget *mitem = list->data;
          if (GTK_WIDGET_VISIBLE (mitem))
            {
              GtkWidget *child = GTK_BIN (mitem)->child;
              if (child && GTK_WIDGET_VISIBLE (child))
                {
                  gtk_widget_size_request (child, &child_requisition);
                  width = MAX (width, child_requisition.width);
                  height = MAX (height, child_requisition.height);
                }
            }
        }
      gtk_widget_size_request (self->child, &child_requisition);
      width = MAX (width, child_requisition.width);
      height = MAX (height, child_requisition.height);
      g_object_set (self->cslot,
                    "width-request", width,
                    "height-request", height,
                    NULL);
    }
}

static void
menu_button_remove_contents (GxkMenuButton *self)
{
  if (self->menu_item)
    {
      if (self->child)
        {
          gtk_widget_set_sensitive (self->child, TRUE);
          gtk_widget_reparent (self->child, self->menu_item);
          self->child = NULL;
        }
      if (self->image)
        {
          g_object_ref (self->image);
          g_object_set (self->image, "sensitive", TRUE, "icon-size", self->old_icon_size, NULL);
          gtk_container_remove (GTK_CONTAINER (self->image->parent), self->image);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (self->menu_item), self->image);
          g_object_unref (self->image);
        }
      self->image = NULL;
      g_signal_handlers_disconnect_by_func (self->menu_item, menu_button_proxy_state, self);
      g_object_unref (self->menu_item);
      self->menu_item = NULL;
      gtk_tooltips_set_tip (GXK_TOOLTIPS, GTK_WIDGET (self), NULL, NULL);
    }
}

void
gxk_menu_button_update (GxkMenuButton *self)
{
  GtkWidget *old_menu_item = self->menu_item;
  menu_button_destroy_backing (self);
  if (self->menu)
    {
      menu_button_remove_contents (self);
      self->menu_item = gtk_menu_get_active (self->menu);
      if (self->menu_item)
        {
          GtkTooltipsData *tipdata;
          g_object_ref (self->menu_item);
          if (self->cslot)
            {
              self->child = GTK_BIN (self->menu_item)->child;
              if (self->child)
                gtk_widget_reparent (self->child, self->cslot);
              if (GTK_IS_IMAGE_MENU_ITEM (self->menu_item))
                self->image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (self->menu_item));
              if (self->image)
                {
                  g_object_get (self->image, "icon-size", &self->old_icon_size, NULL);
                  gtk_widget_reparent (self->image, self->islot);
                  if (self->icon_size)
                    g_object_set (self->image, "icon-size", self->icon_size, NULL);
                }
              g_object_set (self->islot,        /* make room for cslot */
                            "visible", GTK_BIN (self->islot)->child != NULL,
                            NULL);
              g_object_set (self->cslot,        /* make room for islot */
                            "visible", GTK_BIN (self->cslot)->child != NULL,
                            NULL);
              g_object_connect (self->menu_item, "swapped_signal::state_changed", menu_button_proxy_state, self, NULL);
              menu_button_proxy_state (self);
              tipdata = gtk_tooltips_data_get (self->menu_item);
              if (tipdata && tipdata->tip_text)
                gtk_tooltips_set_tip (GXK_TOOLTIPS, GTK_WIDGET (self), tipdata->tip_text, tipdata->tip_private);
              else
                gtk_tooltips_set_tip (GXK_TOOLTIPS, GTK_WIDGET (self),
                                      gxk_widget_get_latent_tooltip (self->menu_item), NULL);
              gtk_widget_queue_resize (GTK_WIDGET (self));
              /* restore slot sizes */
              g_object_set (self->islot,
                            "width-request", -1,
                            "height-request", -1,
                            NULL);
              menu_button_max_size (self);
            }
        }
    }
  if (old_menu_item != self->menu_item)
    g_signal_emit (self, menu_button_signal_changed, 0);
}

static void
menu_button_detacher (GtkWidget *widget,
                      GtkMenu   *menu)
{
  GxkMenuButton *self = GXK_MENU_BUTTON (widget);
  menu_button_destroy_backing (self);
  menu_button_remove_contents (self);
  g_signal_handlers_disconnect_by_func (self->menu, gxk_menu_button_update, self);
  g_signal_handlers_disconnect_by_func (self->menu, menu_button_max_size, self);
  self->menu = NULL;
  g_object_notify (self, "menu");
}

static void
gxk_menu_button_set_property (GObject      *object,
                              guint         param_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  GxkMenuButton *self = GXK_MENU_BUTTON (object);
  switch (param_id)
    {
      const gchar *cstr;
      guint mode;
    case MENU_BUTTON_PROP_MODE:
      mode = g_value_get_enum (value);
      if (self->mode != mode)
        {
          self->mode = mode;
          menu_button_layout (self);
        }
      break;
    case MENU_BUTTON_PROP_RELIEF:
      self->relief = g_value_get_enum (value);
      if (self->button)
        gtk_button_set_relief (GTK_BUTTON (self->button), self->relief);
      break;
    case MENU_BUTTON_PROP_MENU:
      if (self->menu)
        gtk_menu_detach (self->menu);
      self->menu = g_value_get_object (value);
      if (self->menu)
        {
          gxk_menu_attach_as_popup_with_func (self->menu, GTK_WIDGET (self), menu_button_detacher);
          g_object_connect (self->menu,
                            "swapped_signal_after::selection_done", gxk_menu_button_update, self,
                            "swapped_signal_after::size-request", menu_button_max_size, self,
                            NULL);
          gtk_widget_queue_resize (GTK_WIDGET (self));
          gxk_menu_button_update (self);
        }
      break;
    case MENU_BUTTON_PROP_STOCK_SIZE:
      cstr = g_value_get_string (value);
      if (cstr)
        self->icon_size = gtk_icon_size_from_name (cstr);
      else
        self->icon_size = 0;
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, param_id, pspec);
      break;
    }
}

static void
gxk_menu_button_get_property (GObject    *object,
                              guint       param_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  GxkMenuButton *self = GXK_MENU_BUTTON (object);
  switch (param_id)
    {
    case MENU_BUTTON_PROP_MODE:
      g_value_set_enum (value, self->mode);
      break;
    case MENU_BUTTON_PROP_RELIEF:
      g_value_set_enum (value, self->relief);
      break;
    case MENU_BUTTON_PROP_MENU:
      g_value_set_object (value, self->menu);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, param_id, pspec);
      break;
    }
}

static void
menu_button_notify (GObject        *object,
                    GParamSpec     *pspec)
{
  GxkMenuButton *self = GXK_MENU_BUTTON (object);
  if (strcmp (pspec->name, "can-focus") == 0)
    {
      gboolean can_focus = GTK_WIDGET_CAN_FOCUS (self) && self->mode != GXK_MENU_BUTTON_COMBO_MODE;
      if (self->button && can_focus != GTK_WIDGET_CAN_FOCUS (self->button))
        g_object_set (self->button, "can-focus", can_focus, NULL);
    }
  if (G_OBJECT_CLASS (gxk_menu_button_parent_class)->notify)
    G_OBJECT_CLASS (gxk_menu_button_parent_class)->notify (object, pspec);
}

static void
menu_button_dispose (GObject *object)
{
  GxkMenuButton *self = GXK_MENU_BUTTON (object);
  if (self->menu)
    gtk_menu_detach (self->menu);
  G_OBJECT_CLASS (gxk_menu_button_parent_class)->dispose (object);
}

#define FOCUS_SHADOW  (GTK_SHADOW_IN /* anything != NONE */)

static void
menu_button_focus_changed (GxkMenuButton *self)
{
  if (self->fframe)
    gtk_frame_set_shadow_type (GTK_FRAME (self->fframe),
                               GTK_WIDGET_HAS_FOCUS (self) ? FOCUS_SHADOW : GTK_SHADOW_NONE);
}

static GtkWidget*
menu_button_create_button (GxkMenuButton *self,
                           gpointer       child)
{
  g_return_val_if_fail (self->button == NULL, NULL);
  self->button = g_object_new (GTK_TYPE_BUTTON,
                               "can-focus", GTK_WIDGET_CAN_FOCUS (self) && self->mode != GXK_MENU_BUTTON_COMBO_MODE,
                               "relief", self->relief,
                               "child", child ? gtk_widget_get_toplevel (child) : NULL,
                               NULL);
  gxk_nullify_in_object (self, &self->button);
  g_signal_connect_swapped (self->button, "button-press-event", G_CALLBACK (menu_button_button_press), self);
  g_signal_connect_swapped (self->button, "key-press-event", G_CALLBACK (menu_button_key_press), self);
  return self->button;
}

static void
menu_button_layout (GxkMenuButton *self)
{
  GtkWidget *arrow = NULL;
  GxkMenuButtonMode mode = self->mode;
  GtkTable *table = g_object_new (GTK_TYPE_TABLE, NULL);
  /* remove old contents */
  menu_button_destroy_backing (self);
  menu_button_remove_contents (self);
  gtk_container_foreach (GTK_CONTAINER (self), (GtkCallback) gtk_widget_destroy, NULL);
  /* setup button & focus frame */
  if (mode != GXK_MENU_BUTTON_COMBO_MODE)
    menu_button_create_button (self, table);
  /* setup table widget */
  gtk_container_add (GTK_CONTAINER (self), gtk_widget_get_toplevel (GTK_WIDGET (table)));
  /* slot to capture image */
  switch (mode)
    {
    case GXK_MENU_BUTTON_TOOL_MODE:     /* expand image in tool mode */
      self->islot = gtk_alignment_new (0.5, 0.5, 1, 1);
      break;
    case GXK_MENU_BUTTON_POPUP_MODE:    /* not capturing image */
      break;
    default:
      self->islot = gtk_alignment_new (0.5, 0.5, 0, 0);
      break;
    }
  if (self->islot)
    gxk_nullify_in_object (self, &self->islot);
  /* slot to capture child */
  switch (mode)
    {
    case GXK_MENU_BUTTON_COMBO_MODE:    /* left align child in combo mode */
      self->cslot = gtk_alignment_new (0, 0.5, 0, 0);
      break;
    case GXK_MENU_BUTTON_POPUP_MODE:    /* not capturing child */
      break;
    default:
      self->cslot = gtk_alignment_new (0.5, 0.5, 0, 0);
      break;
    }
  if (self->cslot)
    gxk_nullify_in_object (self, &self->cslot);
  /* arrow creation */
  switch (mode)
    {
    case GXK_MENU_BUTTON_TOOL_MODE:
      arrow = g_object_new (GTK_TYPE_ARROW,
                            "arrow-type", GTK_ARROW_DOWN,
                            "yalign", 0.5,
                            NULL);
      break;
    case GXK_MENU_BUTTON_OPTION_MODE:
      arrow = g_object_new (GTK_TYPE_VBOX,
                            "child", g_object_new (GTK_TYPE_ARROW,
                                                   "arrow-type", GTK_ARROW_UP,
                                                   "yalign", 1.0,
                                                   NULL),
                            "child", g_object_new (GTK_TYPE_ARROW,
                                                   "arrow-type", GTK_ARROW_DOWN,
                                                   "yalign", 0.0,
                                                   NULL),
                            NULL);
      break;
    case GXK_MENU_BUTTON_COMBO_MODE:
      arrow = g_object_new (GTK_TYPE_ARROW,
                            "arrow-type", GTK_ARROW_DOWN,
                            "yalign", 0.5,
                            NULL);
      arrow = menu_button_create_button (self, arrow);
      break;
    case GXK_MENU_BUTTON_POPUP_MODE:
      arrow = g_object_new (GTK_TYPE_ARROW,
                            "arrow-type", GTK_ARROW_RIGHT,
                            "xalign", 0.5,
                            "yalign", 0.5,
                            NULL);
      break;
    }
  /* setup slots */
  switch (mode)
    {
      GtkWidget *hbox, *ebox, *frame;
    case GXK_MENU_BUTTON_TOOL_MODE:             /* ---image--- [label] | double arrow */
      gtk_table_attach (table, self->islot,
                        0, 1,                   /* left, right */
                        0, 1,                   /* top, bottom */
                        GTK_FILL | GTK_EXPAND,  /* horizontal */
                        GTK_FILL | GTK_EXPAND,  /* vertical */
                        0, 0);
      gtk_table_attach (table, self->cslot,
                        0, 1,                   /* left, right */
                        1, 2,                   /* top, bottom */
                        GTK_FILL | GTK_EXPAND,  /* horizontal */
                        GTK_FILL,               /* vertical */
                        0, 0);
      gtk_table_attach (table, arrow,
                        1, 2,                   /* left, right */
                        0, 2,                   /* top, bottom */
                        GTK_FILL,               /* horizontal */
                        GTK_FILL | GTK_EXPAND,  /* vertical */
                        0, 0);
      break;
    case GXK_MENU_BUTTON_COMBO_MODE:            /* [image] label---    | down arrow */
      /* pack islot and cslot */
      hbox = g_object_new (GTK_TYPE_HBOX, NULL);
      gtk_box_pack_start (GTK_BOX (hbox), self->islot, FALSE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (hbox), gtk_widget_get_toplevel (self->cslot), TRUE, TRUE, 0);
      /* setup focus frame */
      self->fframe = g_object_new (GXK_TYPE_FOCUS_FRAME, "child", gtk_widget_get_toplevel (hbox), NULL);
      gxk_nullify_in_object (self, &self->fframe);
      /* setup slots */
      ebox = g_object_new (GTK_TYPE_EVENT_BOX, "child", gtk_widget_get_toplevel (hbox), NULL);
      gxk_widget_modify_bg_as_base (ebox);
      hbox = g_object_new (GTK_TYPE_HBOX, NULL);
      gtk_box_pack_start (GTK_BOX (hbox), ebox, TRUE, TRUE, 0);
      gtk_box_pack_end (GTK_BOX (hbox), arrow, FALSE, TRUE, 0);
      frame = g_object_new (GTK_TYPE_FRAME, "child", hbox, "shadow-type", GTK_SHADOW_IN, NULL);
      gtk_table_attach (table, frame,
                        0, 2,                   /* left, right */
                        0, 1,                   /* top, bottom */
                        GTK_FILL | GTK_EXPAND,  /* horizontal */
                        GTK_FILL | GTK_EXPAND,  /* vertical */
                        0, 0);
      break;
    case GXK_MENU_BUTTON_OPTION_MODE:           /* ---(image,label)--- | double arrow (push-in) */
      hbox = g_object_new (GTK_TYPE_HBOX, NULL);
      gtk_box_pack_start (GTK_BOX (hbox), self->islot, FALSE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (hbox), self->cslot, FALSE, TRUE, 0);
      gtk_table_attach (table, hbox,
                        0, 1,                   /* left, right */
                        0, 1,                   /* top, bottom */
                        GTK_EXPAND,             /* horizontal */
                        GTK_FILL | GTK_EXPAND,  /* vertical */
                        0, 0);
      gtk_table_attach (table, arrow,
                        1, 2,                   /* left, right */
                        0, 1,                   /* top, bottom */
                        GTK_FILL,               /* horizontal */
                        GTK_FILL | GTK_EXPAND,  /* vertical */
                        0, 0);
      break;
    case GXK_MENU_BUTTON_POPUP_MODE:            /* right arrow */
      gtk_table_attach (table, arrow,
                        0, 1,                   /* left, right */
                        0, 1,                   /* top, bottom */
                        GTK_FILL | GTK_EXPAND,  /* horizontal */
                        GTK_FILL | GTK_EXPAND,  /* vertical */
                        0, 0);
      break;
    }
  gtk_widget_show_all (GTK_WIDGET (GTK_BIN (self)->child));
  menu_button_focus_changed (self);
}

static void
gxk_menu_button_init (GxkMenuButton *self)
{
  g_object_set (self, "can-focus", 1, NULL);
  g_signal_connect (self, "focus-in-event", G_CALLBACK (menu_button_focus_changed), NULL);
  g_signal_connect (self, "focus-out-event", G_CALLBACK (menu_button_focus_changed), NULL);
}

static void
gxk_menu_button_class_init (GxkMenuButtonClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
  gobject_class->set_property = gxk_menu_button_set_property;
  gobject_class->get_property = gxk_menu_button_get_property;
  gobject_class->notify = menu_button_notify;
  gobject_class->dispose = menu_button_dispose;
  g_object_class_install_property (gobject_class, MENU_BUTTON_PROP_MODE,
                                   g_param_spec_enum ("mode", NULL, NULL, GXK_TYPE_MENU_BUTTON_MODE,
                                                      GXK_MENU_BUTTON_TOOL_MODE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (gobject_class, MENU_BUTTON_PROP_RELIEF,
                                   g_param_spec_enum ("relief", NULL, NULL, GTK_TYPE_RELIEF_STYLE,
                                                      GTK_RELIEF_NORMAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (gobject_class, MENU_BUTTON_PROP_MENU,
                                   g_param_spec_object ("menu", NULL, NULL, GTK_TYPE_MENU, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, MENU_BUTTON_PROP_STOCK_SIZE,
                                   g_param_spec_string ("stock-size", NULL, NULL, NULL, G_PARAM_WRITABLE));
  widget_class->button_press_event = menu_button_button_press;
  widget_class->key_press_event = menu_button_key_press;
  widget_class->mnemonic_activate = menu_button_mnemonic_activate;
  menu_button_signal_changed = g_signal_new ("changed", G_OBJECT_CLASS_TYPE (class),
                                             G_SIGNAL_RUN_FIRST, 0, NULL, NULL,
                                             gtk_signal_default_marshaller, G_TYPE_NONE, 0);
}
