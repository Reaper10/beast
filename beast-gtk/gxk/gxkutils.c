/* GXK - Gtk+ Extension Kit
 * Copyright (C) 1998-2002 Tim Janik
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
#include "gxkutils.h"


/* --- generated marshallers --- */
#include "gxkmarshal.c"


/* --- generated type IDs and enums --- */
#include "gxkgentypes.c"


/* --- functions --- */
void
_gxk_init_utils (void)
{
  gxk_type_register_generated (G_N_ELEMENTS (generated_type_entries), generated_type_entries);
}

/**
 * gxk_type_register_generated
 * @n_entries: number of generated types to register
 * @entries:   GxkTypeGenerated type descriptions
 *
 * Register each of the generated type entries with the
 * type system. Currently supported parent types are
 * %G_TYPE_ENUM and %G_TYPE_FLAGS in which cases the
 * @type_data member must point to a %NULL terminated
 * array of GEnumValue or GFlagValue structures. No
 * extra copying is performed, so the passed in
 * structures have to persist throughout runtime.
 */
void
gxk_type_register_generated (guint                   n_entries,
			     const GxkTypeGenerated *entries)
{
  guint i;

  for (i = 0; i < n_entries; i++)
    {
      GType type_id;
      switch (entries[i].parent)
	{
	case G_TYPE_ENUM:
	  type_id = g_enum_register_static (entries[i].type_name, entries[i].type_data);
	  break;
	case G_TYPE_FLAGS:
	  type_id = g_flags_register_static (entries[i].type_name, entries[i].type_data);
	  break;
	default:
	  g_warning ("%s: unsupported parent type `%s'", G_STRLOC, g_type_name (entries[i].parent));
	  type_id = 0;
	  break;
	}
      *entries[i].type_id = type_id;
    }
}

/**
 * g_object_set_long
 * @object: a valid GObject
 * @name:   name of the long value to set
 * @v_long: the actual value
 *
 * Convenience variant of g_object_set_data() to set
 * a long instead of a pointer.
 */
void
g_object_set_long (gpointer     object,
		   const gchar *name,
		   glong        v_long)
{
  g_return_if_fail (G_IS_OBJECT (object));

  g_object_set_data (object, name, (gpointer) v_long);
}

/**
 * g_object_get_long
 * @object:  a valid GObject
 * @name:    name of the long value to retrieve
 * @RETURNS: the actual value
 *
 * Convenience variant of g_object_get_data() to retrieve
 * a long instead of a pointer.
 */
glong
g_object_get_long (gpointer     object,
		   const gchar *name)
{
  g_return_val_if_fail (G_IS_OBJECT (object), 0);

  return (glong) g_object_get_data (object, name);
}

/**
 * gxk_widget_make_insensitive
 * @widget: a valid GtkWidget
 *
 * This function is euqivalent to gtk_widget_set_sensitive (@widget, #FALSE);
 * It exists as a convenient signal connection callback.
 */
void
gxk_widget_make_insensitive (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (GTK_WIDGET_IS_SENSITIVE (widget))
    gtk_widget_set_sensitive (widget, FALSE);
}

/**
 * gxk_widget_make_sensitive
 * @widget: a valid GtkWidget
 *
 * This function is euqivalent to gtk_widget_set_sensitive (@widget, #TRUE);
 * It exists as a convenient signal connection callback.
 */
void
gxk_widget_make_sensitive (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (!GTK_WIDGET_IS_SENSITIVE (widget))
    gtk_widget_set_sensitive (widget, TRUE);
}

static gint
idle_shower (GtkWidget **widget_p)
{
  GDK_THREADS_ENTER ();

  if (GTK_IS_WIDGET (*widget_p))
    {
      gtk_signal_disconnect_by_func (GTK_OBJECT (*widget_p),
				     GTK_SIGNAL_FUNC (gtk_widget_destroyed),
				     widget_p);
      gtk_widget_show (*widget_p);
    }

  g_free (widget_p);

  GDK_THREADS_LEAVE ();

  return FALSE;
}

/**
 * gxk_idle_show_widget
 * @widget: a valid widget
 *
 * Defer showing this widget until the next idle handler
 * is run. This is usefull if other things are pending
 * which need to be processed first, for instance
 * hiding other toplevels.
 */
void
gxk_idle_show_widget (GtkWidget *widget)
{
  GtkWidget **widget_p;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  widget_p = g_new (GtkWidget*, 1);

  *widget_p = widget;
  gtk_signal_connect (GTK_OBJECT (widget),
		      "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed),
		      widget_p);
  gtk_idle_add_priority (GTK_PRIORITY_RESIZE - 1, (GtkFunction) idle_shower, widget_p);
}

/**
 * gxk_widget_showraise
 * @widget: a valid widget
 *
 * Show the widget. If the widget is a toplevel,
 * also raise its window to top.
 */
void
gxk_widget_showraise (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_widget_show (widget);
  if (GTK_WIDGET_REALIZED (widget) && !widget->parent)
    gdk_window_raise (widget->window);
}

/**
 * gxk_toplevel_delete
 * @widget: a widget having a toplevel
 *
 * This function is usefull to produce the exact same effect
 * as if the user caused window manager triggered window
 * deletion on the toplevel of @widget.
 */
void
gxk_toplevel_delete (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  widget = gtk_widget_get_toplevel (widget);
  if (GTK_IS_WINDOW (widget) && GTK_WIDGET_DRAWABLE (widget))
    {
      GdkEvent event = { 0, };

      event.any.type = GDK_DELETE;
      event.any.window = widget->window;
      event.any.send_event = TRUE;
      gdk_event_put (&event);
    }
}

/**
 * gxk_toplevel_activate_default
 * @widget: a widget having a toplevel
 *
 * Activate the default widget of the toplevel of @widget.
 */
void
gxk_toplevel_activate_default (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  widget = gtk_widget_get_toplevel (widget);
  if (GTK_IS_WINDOW (widget))
    gtk_window_activate_default (GTK_WINDOW (widget));
}

/**
 * gxk_toplevel_hide
 * @widget: a widget having a toplevel
 *
 * Hide the toplevel of @widget.
 */
void
gxk_toplevel_hide (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  widget = gtk_widget_get_toplevel (widget);
  gtk_widget_hide (widget);
}

static void
style_modify_fg_as_sensitive (GtkWidget *widget)
{
  GtkRcStyle *rc_style = gtk_rc_style_new ();

  rc_style->color_flags[GTK_STATE_INSENSITIVE] = GTK_RC_FG;
  rc_style->fg[GTK_STATE_INSENSITIVE].red = widget->style->fg[GTK_STATE_NORMAL].red;
  rc_style->fg[GTK_STATE_INSENSITIVE].green = widget->style->fg[GTK_STATE_NORMAL].green;
  rc_style->fg[GTK_STATE_INSENSITIVE].blue = widget->style->fg[GTK_STATE_NORMAL].blue;
  gtk_widget_modify_style (widget, rc_style);
}

/**
 * gxk_widget_modify_as_title
 * @widget: a valid GtkWidget
 *
 * Modify the widget and it's style, so that it is insensitive,
 * but doesn't quite look that way. Usefull for inactive title
 * menu items in menus.
 */
void
gxk_widget_modify_as_title (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_widget_set_sensitive (widget, FALSE);
  if (GTK_WIDGET_REALIZED (widget))
    style_modify_fg_as_sensitive (widget);
  if (!gxk_signal_handler_pending (widget, "realize", G_CALLBACK (style_modify_fg_as_sensitive), NULL))
    g_signal_connect_after (widget, "realize", G_CALLBACK (style_modify_fg_as_sensitive), NULL);
  if (!gxk_signal_handler_pending (widget, "realize", G_CALLBACK (gxk_widget_make_insensitive), NULL))
    g_signal_connect_after (widget, "realize", G_CALLBACK (gxk_widget_make_insensitive), NULL);
}

static void
style_modify_bg_as_base (GtkWidget *widget)
{
  GtkRcStyle *rc_style = gtk_widget_get_modifier_style (widget);

  rc_style->color_flags[GTK_STATE_NORMAL] = GTK_RC_BG;
  rc_style->bg[GTK_STATE_NORMAL].red = widget->style->base[GTK_STATE_NORMAL].red;
  rc_style->bg[GTK_STATE_NORMAL].green = widget->style->base[GTK_STATE_NORMAL].green;
  rc_style->bg[GTK_STATE_NORMAL].blue = widget->style->base[GTK_STATE_NORMAL].blue;
  gtk_widget_modify_style (widget, rc_style);
}

/**
 * gxk_widget_modify_bg_as_base
 * @widget: a valid GtkWidget
 *
 * Modify the widget's background to look like the background
 * of a text or list widget (usually white). This is usefull
 * if a hbox or similar widget is used to "simulate" a list,
 * text, or similar widget.
 */
void
gxk_widget_modify_bg_as_base (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (GTK_WIDGET_REALIZED (widget))
    style_modify_bg_as_base (widget);
  if (!gxk_signal_handler_pending (widget, "realize", G_CALLBACK (style_modify_bg_as_base), NULL))
    g_signal_connect_after (widget, "realize", G_CALLBACK (style_modify_bg_as_base), NULL);
}

static void
style_modify_base_as_bg (GtkWidget *widget)
{
  GtkRcStyle *rc_style = gtk_rc_style_new ();

  rc_style->color_flags[GTK_STATE_NORMAL] = GTK_RC_BASE;
  rc_style->base[GTK_STATE_NORMAL].red = widget->style->bg[GTK_STATE_NORMAL].red;
  rc_style->base[GTK_STATE_NORMAL].green = widget->style->bg[GTK_STATE_NORMAL].green;
  rc_style->base[GTK_STATE_NORMAL].blue = widget->style->bg[GTK_STATE_NORMAL].blue;
  gtk_widget_modify_style (widget, rc_style);
}

/**
 * gxk_widget_modify_base_as_bg
 * @widget: a valid GtkWidget
 *
 * Modify the widget's base background (used by list and
 * text widgets) to look like an ordinary widget background.
 * This is usefull if a list, text or similar widget shouldn't
 * stand out as such, e.g. when the GtkTextView widget displaying
 * a long non-editable text should look similar to a GtkLabel.
 */
void
gxk_widget_modify_base_as_bg (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (GTK_WIDGET_REALIZED (widget))
    style_modify_base_as_bg (widget);
  if (!gxk_signal_handler_pending (widget, "realize", G_CALLBACK (style_modify_base_as_bg), NULL))
    g_signal_connect_after (widget, "realize", G_CALLBACK (style_modify_base_as_bg), NULL);
}

static gboolean
expose_bg_clear (GtkWidget      *widget,
		 GdkEventExpose *event)
{
  gtk_paint_flat_box (widget->style, widget->window, GTK_STATE_NORMAL,
		      GTK_SHADOW_NONE, &event->area, widget, "base", 0, 0, -1, -1);

  return FALSE;
}

/**
 * gxk_widget_force_bg_clear
 * @widget: a valid GtkWidget
 *
 * Enforce drawing of a widget's background.
 * Some widgets do not explicitely draw their background,
 * but simply draw themsleves on top of their parent's
 * background. This function forces the widget into
 * drawing its background according to its style settings.
 */
void
gxk_widget_force_bg_clear (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  gtk_widget_set_redraw_on_allocate (widget, TRUE);
  if (!gxk_signal_handler_pending (widget, "expose_event", G_CALLBACK (expose_bg_clear), NULL))
    g_signal_connect (widget, "expose_event", G_CALLBACK (expose_bg_clear), NULL);
}

/**
 * gxk_size_group
 * @sgmode: size group mode, one of %GTK_SIZE_GROUP_NONE,
 *          %GTK_SIZE_GROUP_HORIZONTAL, %GTK_SIZE_GROUP_VERTICAL or
 *	    %GTK_SIZE_GROUP_BOTH
 * @...:    %NULL terminated list of widgets to group together
 *
 * Group horizontal and/or vertical resizing behaviour of
 * widgets. See gtk_size_group_set_mode() on the effect of
 * the various grouping modes.
 */
void
gxk_size_group (GtkSizeGroupMode sgmode,
		GtkWidget       *first_widget,
		...)
{
  if (first_widget)
    {
      GtkWidget *widget = first_widget;
      GtkSizeGroup *sgroup = gtk_size_group_new (sgmode);
      va_list args;

      va_start (args, first_widget);
      while (widget)
	{
	  gtk_size_group_add_widget (sgroup, widget);
	  widget = va_arg (args, GtkWidget*);
	}
      va_end (args);
      g_object_unref (sgroup);
    }
}

/**
 * gxk_tree_selection_select_spath
 * @selection: GtkTreeSelection to modify
 * @str_path:  a stringified GtkTreePath
 *
 * Select the row denoted by @str_path.
 */
void
gxk_tree_selection_select_spath (GtkTreeSelection *selection,
				 const gchar      *str_path)
{
  GtkTreePath *path;

  g_return_if_fail (GTK_IS_TREE_SELECTION (selection));
  g_return_if_fail (str_path != NULL);

  path = gtk_tree_path_new_from_string (str_path);
  gtk_tree_selection_select_path (selection, path);
  gtk_tree_path_free (path);
}

/**
 * gxk_tree_selection_unselect_spath
 * @selection: GtkTreeSelection to modify
 * @str_path:  a stringified GtkTreePath
 *
 * Unselect the row denoted by @str_path.
 */
void
gxk_tree_selection_unselect_spath (GtkTreeSelection *selection,
				   const gchar      *str_path)
{
  GtkTreePath *path;

  g_return_if_fail (GTK_IS_TREE_SELECTION (selection));
  g_return_if_fail (str_path != NULL);

  path = gtk_tree_path_new_from_string (str_path);
  gtk_tree_selection_unselect_path (selection, path);
  gtk_tree_path_free (path);
}

/**
 * gxk_tree_selection_select_ipath
 * @selection: GtkTreeSelection to modify
 * @...:       GtkTreePath indices
 *
 * Select the row denoted by the path to be constructed from the
 * -1 terminated indices.
 */
void
gxk_tree_selection_select_ipath (GtkTreeSelection *selection,
				 gint              first_index,
				 ...)
{
  GtkTreePath *path;
  va_list args;
  gint i;

  g_return_if_fail (GTK_IS_TREE_SELECTION (selection));

  path = gtk_tree_path_new ();
  i = first_index;
  va_start (args, first_index);
  while (i != -1)
    {
      gtk_tree_path_append_index (path, i);
      i = va_arg (args, gint);
    }
  va_end (args);
  gtk_tree_selection_select_path (selection, path);
  gtk_tree_path_free (path);
}

/**
 * gxk_tree_selection_unselect_ipath
 * @selection: GtkTreeSelection to modify
 * @...:       GtkTreePath indices
 *
 * Select the row denoted by the path to be constructed from the
 * -1 terminated indices.
 */
void
gxk_tree_selection_unselect_ipath (GtkTreeSelection *selection,
				   gint              first_index,
				   ...)
{
  GtkTreePath *path;
  va_list args;
  gint i;

  g_return_if_fail (GTK_IS_TREE_SELECTION (selection));

  path = gtk_tree_path_new ();
  i = first_index;
  va_start (args, first_index);
  while (i != -1)
    {
      gtk_tree_path_append_index (path, i);
      i = va_arg (args, gint);
    }
  va_end (args);
  gtk_tree_selection_unselect_path (selection, path);
  gtk_tree_path_free (path);
}

static GSList           *browse_selection_queue = NULL;
static guint             browse_selection_handler_id = 0;
static GtkTreeSelection *browse_selection_ignore = NULL;

static void
browse_selection_weak_notify (gpointer data,
			      GObject *object)
{
  browse_selection_queue = g_slist_remove (browse_selection_queue, object);
}

static gboolean
browse_selection_handler (gpointer data)
{
  GtkTreeSelection *selection;
  GDK_THREADS_ENTER ();
  selection = g_slist_pop_head (&browse_selection_queue);
  while (selection)
    {
      GtkTreeIter iter;
      gboolean needs_sel;
      g_object_weak_unref (G_OBJECT (selection), browse_selection_weak_notify, selection);
      needs_sel = (gtk_tree_selection_get_mode (selection) == GTK_SELECTION_BROWSE &&
		   !gtk_tree_selection_get_selected (selection, NULL, &iter));
      if (needs_sel)
	{
	  GtkTreePath *path = g_object_get_data (selection, "GxkTreeSelection-last");
	  g_object_ref (selection);
	  browse_selection_ignore = selection;
	  if (path)
	    {
	      gtk_tree_selection_select_path (selection, path);
	      path = g_object_get_data (selection, "GxkTreeSelection-last");
	      needs_sel = !gtk_tree_selection_get_selected (selection, NULL, NULL);
	      if (needs_sel && path && gxk_tree_path_prev (path))
		{
		  gtk_tree_selection_select_path (selection, path);
		  path = g_object_get_data (selection, "GxkTreeSelection-last");
		  needs_sel = !gtk_tree_selection_get_selected (selection, NULL, NULL);
		}
	      if (needs_sel && path && gtk_tree_path_up (path))
		{
		  gtk_tree_selection_select_path (selection, path);
		  path = g_object_get_data (selection, "GxkTreeSelection-last");
		  needs_sel = !gtk_tree_selection_get_selected (selection, NULL, NULL);
		}
	    }
	  if (needs_sel)
	    gxk_tree_selection_select_ipath (selection, 0, -1);
	  browse_selection_ignore = NULL;
	  g_object_unref (selection);
	}
      selection = g_slist_pop_head (&browse_selection_queue);
    }
  browse_selection_handler_id = 0;
  GDK_THREADS_LEAVE ();
  return FALSE;
}

static void
browse_selection_changed (GtkTreeSelection *selection)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
      g_object_set_data_full (selection, "GxkTreeSelection-last", path, gtk_tree_path_free);
    }
  if (browse_selection_ignore != selection &&
      !g_slist_find (browse_selection_queue, selection))
    {
      g_object_weak_ref (G_OBJECT (selection), browse_selection_weak_notify, selection);
      browse_selection_queue = g_slist_prepend (browse_selection_queue, selection);
      if (browse_selection_handler_id == 0)
	browse_selection_handler_id = g_idle_add_full (G_PRIORITY_DEFAULT, browse_selection_handler, NULL, NULL);
    }
  g_print ("browse_selection_changed\n");
}

/**
 * gxk_tree_selection_force_browse
 * @selection: GtkTreeSelection to watch
 * @model:     tree model used with @selection
 *
 * Watch deletion and insertions into empty trees to
 * ensure valid selections across these events.
 */
void
gxk_tree_selection_force_browse (GtkTreeSelection *selection,
				 GtkTreeModel     *model)
{
  g_return_if_fail (GTK_IS_TREE_SELECTION (selection));
  if (model)
    g_return_if_fail (GTK_IS_TREE_MODEL (model));

  if (!gxk_signal_handler_pending (selection, "changed", G_CALLBACK (browse_selection_changed), selection))
    g_signal_connect_data (selection, "changed", G_CALLBACK (browse_selection_changed), selection, NULL, 0);
  if (model && !gxk_signal_handler_pending (model, "row-inserted", G_CALLBACK (browse_selection_changed), selection))
    g_signal_connect_object (model, "row-inserted", G_CALLBACK (browse_selection_changed), selection, G_CONNECT_SWAPPED);
  browse_selection_changed (selection);
}

/**
 * gxk_tree_path_prev
 * @path:
 *
 * Workaround for gtk_tree_path_prev() which corrupts memory
 * if called on empty paths (up to version 2.2 at least).
 */
gboolean
gxk_tree_path_prev (GtkTreePath *path)
{
  if (path && gtk_tree_path_get_depth (path) < 1)
    return FALSE;
  return gtk_tree_path_prev (path);
}

/**
 * gxk_notebook_append
 * @notebook: a valid notebook
 * @child:    a valid parent-less widget
 * @label:    notebook page name
 *
 * Add a new page containing @child to @notebook,
 * naming the page @label.
 */
void
gxk_notebook_append (GtkNotebook *notebook,
		     GtkWidget   *child,
		     const gchar *label)
{
  g_return_if_fail (GTK_IS_NOTEBOOK (notebook));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (label != NULL);

  gtk_notebook_append_page (notebook, child, g_object_new (GTK_TYPE_LABEL,
							   "visible", TRUE,
							   "label", label,
							   NULL));
}

/**
 * gxk_signal_handler_pending
 * @instance:        object instance with signals
 * @detailed_signal: signal name
 * @callback:        custom callback function
 * @data:            callback data
 * @RETURNS:         whether callback is connected
 *
 * Find out whether a specific @callback is pending for a
 * specific signal on an instance. %TRUE is returned if
 * the @callback is found, %FALSE otherwise.
 */
gboolean
gxk_signal_handler_pending (gpointer     instance,
			    const gchar *detailed_signal,
			    GCallback    callback,
			    gpointer     data)
{
  guint signal_id;
  GQuark detail = 0;

  g_return_val_if_fail (G_TYPE_CHECK_INSTANCE (instance), FALSE);
  g_return_val_if_fail (detailed_signal != NULL, FALSE);
  g_return_val_if_fail (callback != NULL, FALSE);

  if (g_signal_parse_name (detailed_signal, G_TYPE_FROM_INSTANCE (instance),
			   &signal_id, &detail, FALSE))
    {
      if (g_signal_handler_find (instance, (G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_DATA |
					    (detail ? G_SIGNAL_MATCH_DETAIL : 0)),
				 signal_id, detail, NULL, callback, data) != 0)
	return TRUE;
    }
  else
    g_warning ("%s: signal name \"%s\" is invalid for instance `%p'", G_STRLOC, detailed_signal, instance);
  return FALSE;
}


/* --- derivation convenience --- */
typedef struct {
  gpointer *parent_class_p;
  /* GObject */
  gpointer finalize, dispose;
  /* GtkObject */
  gpointer destroy;
} TypeMethods;

static void
gxk_generic_type_class_init (gpointer class,
			     gpointer class_data)
{
  TypeMethods *tm = class_data;
  if (tm)
    *tm->parent_class_p = g_type_class_peek_parent (class);
  if (tm && G_IS_OBJECT_CLASS (class))
    {
      GObjectClass *oclass = G_OBJECT_CLASS (class);
      if (tm->finalize)
	oclass->finalize = tm->finalize;
      if (tm->dispose)
	oclass->dispose = tm->dispose;
    }
  if (tm && GTK_IS_OBJECT_CLASS (class))
    {
      GtkObjectClass *oclass = GTK_OBJECT_CLASS (class);
      if (tm->destroy)
	oclass->destroy = tm->destroy;
    }
}

/**
 * gxk_object_derive
 *
 * Derive a new object type, giving a list of
 * methods which are implemented for this new object type.
 */
GType
gxk_object_derive (GType          parent_type,
		   const gchar   *name,
		   gpointer      *parent_class_p,
		   guint          instance_size,
		   guint          class_size,
		   GxkMethodType  mtype,
		   ...)
{
  GType type = g_type_from_name (name);
  if (!type)
    {
      GTypeInfo type_info = {
	0,	/* class_size */
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gxk_generic_type_class_init,
	NULL,   /* class_finalize */
	NULL,   /* class_data */
	0,	/* instance_size */
	0,      /* n_preallocs */
	NULL,	/* instance_init */
      };
      TypeMethods tm = { NULL, };
      gpointer initfunc = NULL;
      gboolean need_cd = FALSE;
      va_list args;
      g_return_val_if_fail (G_TYPE_IS_OBJECT (parent_type), 0);
      type_info.class_size = class_size;
      type_info.instance_size = instance_size;
      va_start (args, mtype);
      while (mtype)
	{
	  gpointer func = va_arg (args, gpointer);
	  switch (mtype)
	    {
	    case GXK_METHOD_INIT:		initfunc = func;	break;
	    case GXK_METHOD_FINALIZE:		tm.finalize = func;	break;
	    case GXK_METHOD_DISPOSE:		tm.dispose = func;	break;
	    case GXK_METHOD_DESTROY:		tm.destroy = func;	break;
	    default:	g_error ("invalid method type: %d", mtype);
	    }
	  need_cd |= mtype != GXK_METHOD_INIT;
	  mtype = va_arg (args, GxkMethodType);
	}
      va_end (args);
      type_info.instance_init = initfunc;
      if (need_cd || parent_class_p)
	{
	  g_return_val_if_fail (parent_class_p != NULL, 0);
	  tm.parent_class_p = parent_class_p;
	  type_info.class_data = g_memdup (&tm, sizeof (TypeMethods));
	}
      type = g_type_register_static (parent_type, name, &type_info, 0);
    }
  return type;
}


/* --- Gtk bug fixes --- */
static gchar*
path_fix_uline (const gchar *str)
{
  gchar *path = g_strdup (str);
  gchar *p = path, *q = path;
  if (!p)
    return NULL;
  while (*p)
    {
      if (*p == '_')
	{
	  if (p[1] == '_')
	    {
	      p++;
	      *q++ = '_';
	    }
	}
      else
	*q++ = *p;
      p++;
    }
  *q = 0;
  return path;
}

#undef gtk_item_factory_get_item
GtkWidget*
gxk_item_factory_get_item (GtkItemFactory *ifactory,
			   const gchar    *path)
{
  gchar *p = path_fix_uline (path);
  GtkWidget *widget = gtk_item_factory_get_item (ifactory, p);
  g_free (p);
  return widget;
}

#undef gtk_item_factory_get_widget
GtkWidget*
gxk_item_factory_get_widget (GtkItemFactory *ifactory,
			     const gchar    *path)
{
  gchar *p = path_fix_uline (path);
  GtkWidget *widget = gtk_item_factory_get_widget (ifactory, p);
  g_free (p);
  return widget;
}
