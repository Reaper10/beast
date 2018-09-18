// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include "bsttracksynthdialog.hh"
#include "bsttreestores.hh"
#include "bse/internal.hh"

/* --- prototypes --- */
static void     bst_track_synth_dialog_finalize     (GObject                    *object);
static void     bst_track_synth_dialog_activate     (BstTrackSynthDialog        *self);
static gboolean bst_track_synth_dialog_delete_event (GtkWidget                  *widget,
                                                     GdkEventAny                *event);
static void     bst_track_synth_dialog_setup        (BstTrackSynthDialog *self,
                                                     gpointer             parent_widget,
                                                     const gchar         *title,
                                                     SfiProxy             proxy);

/* --- functions --- */
G_DEFINE_TYPE (BstTrackSynthDialog, bst_track_synth_dialog, GXK_TYPE_DIALOG);

static void
bst_track_synth_dialog_class_init (BstTrackSynthDialogClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gobject_class->finalize = bst_track_synth_dialog_finalize;

  widget_class->delete_event = bst_track_synth_dialog_delete_event;
}

static void
bst_track_synth_dialog_init (BstTrackSynthDialog *self)
{
  GtkTreeSelection *tsel;
  GtkTreeModel *smodel;
  GtkWidget *main_box = GXK_DIALOG (self)->vbox;

  /* configure self */
  g_object_set (self,
                "flags", (GXK_DIALOG_HIDE_ON_DELETE |
                          GXK_DIALOG_PRESERVE_STATE |
                          GXK_DIALOG_POPUP_POS |
                          GXK_DIALOG_MODAL),
                NULL);
  gxk_dialog_set_sizes (GXK_DIALOG (self), 550, 300, 600, 450);

  /* notebook */
  self->notebook = (GtkNotebook*) g_object_new (GXK_TYPE_NOTEBOOK,
                                                "visible", TRUE,
                                                "homogeneous", TRUE,
                                                "show_border", TRUE,
                                                "show_tabs", TRUE,
                                                "scrollable", FALSE,
                                                "tab_border", 0,
                                                "enable_popup", TRUE,
                                                "tab_pos", GTK_POS_TOP,
                                                "border_width", 5,
                                                "parent", main_box,
                                                "enable_popup", FALSE,
                                                NULL);

  /* synth list */
  self->spage = (GtkWidget*) g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                                           "visible", TRUE,
                                           "hscrollbar_policy", GTK_POLICY_AUTOMATIC,
                                           "vscrollbar_policy", GTK_POLICY_ALWAYS,
                                           "border_width", 5,
                                           "shadow_type", GTK_SHADOW_IN,
                                           NULL);
  gxk_notebook_append (self->notebook, self->spage, "synth", TRUE);

  /* synth selection store and tree */
  self->pstore = bst_item_seq_store_new (TRUE);
  smodel = gtk_tree_model_sort_new_with_model (self->pstore);
  self->tview = (GtkTreeView*) g_object_new (GTK_TYPE_TREE_VIEW,
                                             "visible", TRUE,
                                             "can_focus", TRUE,
                                             "model", smodel,
                                             "rules_hint", TRUE,
                                             "parent", self->spage,
                                             "search_column", BST_PROXY_STORE_NAME,
                                             NULL);
  g_object_unref (smodel);
  tsel = gtk_tree_view_get_selection (self->tview);
  gtk_tree_selection_set_mode (tsel, GTK_SELECTION_BROWSE);
  gxk_tree_selection_force_browse (tsel, smodel);

  /* synth selection tree columns */
  if (BST_DVL_HINTS)
    gxk_tree_view_add_text_column (self->tview, BST_PROXY_STORE_SEQID, "S",
                                   0.0, "ID", NULL,
                                   NULL, NULL, G_CONNECT_SWAPPED);
  gxk_tree_view_add_text_column (self->tview, BST_PROXY_STORE_NAME, "S",
                                 0.0, "Name", NULL,
                                 NULL, NULL, G_CONNECT_SWAPPED);
  gxk_tree_view_add_text_column (self->tview, BST_PROXY_STORE_BLURB, "",
                                 0.0, "Comment", NULL,
                                 NULL, NULL, G_CONNECT_SWAPPED);
  if (BST_DVL_HINTS)
    gxk_tree_view_add_text_column (self->tview, BST_PROXY_STORE_TYPE, "",
                                   0.0, "Type", NULL,
                                   NULL, NULL, G_CONNECT_SWAPPED);

  /* wave repo view */
  self->wpage = (GtkWidget*) g_object_new (BST_TYPE_WAVE_VIEW, "visible", TRUE, NULL);
  gxk_notebook_append (self->notebook, self->wpage, "wave", TRUE);
  bst_wave_view_set_editable (BST_WAVE_VIEW (self->wpage), FALSE);
  /* sound font view */
  self->sfont_page = (GtkWidget *) g_object_new (BST_TYPE_SOUND_FONT_VIEW, "visible", TRUE, NULL);
  gxk_notebook_append (self->notebook, self->sfont_page, "sound_font", TRUE);

  /* provide buttons */
  self->ok = gxk_dialog_default_action_swapped (GXK_DIALOG (self), BST_STOCK_OK, (void*) bst_track_synth_dialog_activate, self);
  gxk_dialog_action (GXK_DIALOG (self), BST_STOCK_CANCEL, (void*) gxk_toplevel_delete, (GtkWidget*) self);

  /* make row connections */
  g_signal_connect_object (self->tview, "row_activated", G_CALLBACK (gtk_button_clicked), self->ok, G_CONNECT_SWAPPED);
  g_signal_connect_object (BST_ITEM_VIEW (self->wpage)->tree, "row_activated",
                           G_CALLBACK (gtk_button_clicked), self->ok, G_CONNECT_SWAPPED);
}

static void
bst_track_synth_dialog_finalize (GObject *object)
{
  BstTrackSynthDialog *self = BST_TRACK_SYNTH_DIALOG (object);

  bst_track_synth_dialog_setup (self, NULL, NULL, 0);

  /* chain parent class' handler */
  G_OBJECT_CLASS (bst_track_synth_dialog_parent_class)->finalize (object);
}

static gboolean
bst_track_synth_dialog_delete_event (GtkWidget   *widget,
                                     GdkEventAny *event)
{
  BstTrackSynthDialog *self = BST_TRACK_SYNTH_DIALOG (widget);
  GxkFreeFunc selected_cleanup = self->selected_cleanup;
  self->selected_callback = NULL;
  self->selected_cleanup = NULL;
  if (selected_cleanup)
    selected_cleanup (self->selected_data);
  if (self->pstore)
    bst_item_seq_store_set (self->pstore, NULL);
  /* chain parent class' handler */
  return GTK_WIDGET_CLASS (bst_track_synth_dialog_parent_class)->delete_event (widget, event);
}

static void
parent_window_destroyed (BstTrackSynthDialog *self)
{
  GxkFreeFunc selected_cleanup = self->selected_cleanup;
  self->selected_callback = NULL;
  self->selected_cleanup = NULL;
  if (selected_cleanup)
    selected_cleanup (self->selected_data);
  gtk_widget_hide (GTK_WIDGET (self));
  bst_track_synth_dialog_setup (self, NULL, NULL, 0);
  gxk_toplevel_delete (GTK_WIDGET (self));
}

static void
bst_track_synth_dialog_setup (BstTrackSynthDialog *self,
                              gpointer             parent_widget,
                              const gchar         *title,
                              SfiProxy             proxy)
{
  GtkWindow *window = GTK_WINDOW (self);

  assert_return (BST_IS_TRACK_SYNTH_DIALOG (self));

  self->selected_callback = NULL;
  GxkFreeFunc selected_cleanup = self->selected_cleanup;
  self->selected_callback = NULL;
  self->selected_cleanup = NULL;
  if (selected_cleanup)
    selected_cleanup (self->selected_data);

  gtk_widget_hide (GTK_WIDGET (self));

  /* reset proxy handling */
  bst_window_sync_title_to_proxy (self, proxy, title);

  /* cleanup connections to old parent_window */
  if (self->parent_window)
    g_signal_handlers_disconnect_by_func (self->parent_window, (void*) parent_window_destroyed, self);
  if (window->group)
    gtk_window_group_remove_window (window->group, window);
  gtk_window_set_transient_for (window, NULL);

  self->parent_window = parent_widget ? (GtkWindow*) gtk_widget_get_ancestor ((GtkWidget*) parent_widget, GTK_TYPE_WINDOW) : NULL;

  /* setup connections to new parent_window */
  if (self->parent_window)
    {
      gtk_window_set_transient_for (window, self->parent_window);
      if (self->parent_window->group)
        gtk_window_group_add_window (self->parent_window->group, window);
      g_signal_connect_object (self->parent_window, "destroy",
                               G_CALLBACK (parent_window_destroyed),
                               self, G_CONNECT_SWAPPED);
    }

  /* allow activation */
  self->ignore_activate = FALSE;
  gxk_notebook_set_current_page_widget (self->notebook, self->spage);
}

static BstTrackSynthDialog*
bst_track_synth_dialog_singleton (void)
{
  static BstTrackSynthDialog *ts_singleton = NULL;
  if (!ts_singleton)
    ts_singleton = (BstTrackSynthDialog*) g_object_new (BST_TYPE_TRACK_SYNTH_DIALOG, NULL);
  return ts_singleton;
}

GtkWidget*
bst_track_synth_dialog_popup (gpointer     parent_widget,
                              SfiProxy     track,
                              const gchar *candidate_label,
                              const gchar *candidate_tooltip,
                              BseIt3mSeq  *candidates,
                              const gchar *wrepo_label,
                              const gchar *wrepo_tooltip,
                              SfiProxy     wrepo,
                              const gchar *sfrepo_label,
                              const gchar *sfrepo_tooltip,
                              SfiProxy     sfrepo,
                              BstTrackSynthDialogSelected  selected_callback,
                              gpointer                     selected_data,
                              GxkFreeFunc                  selected_cleanup)
{
  BstTrackSynthDialog *self = bst_track_synth_dialog_singleton ();
  GtkWidget *widget = GTK_WIDGET (self);
  if (!candidate_label)
    candidate_label = "";
  if (!wrepo_label)
    wrepo_label = "";
  if (!sfrepo_label)
    sfrepo_label = "";

  bst_track_synth_dialog_setup (self, NULL, NULL, 0);

  g_object_set (gtk_notebook_get_tab_label (self->notebook, self->spage), "label", candidate_label, NULL);
  gxk_widget_set_tooltip (self->tview, candidate_tooltip);
  g_object_set (gtk_notebook_get_tab_label (self->notebook, self->wpage), "label", wrepo_label, NULL);
  gxk_widget_set_tooltip (BST_ITEM_VIEW (self->wpage)->tree, wrepo_tooltip);
  g_object_set (gtk_notebook_get_tab_label (self->notebook, self->sfont_page), "label", sfrepo_label, NULL);
  gxk_widget_set_tooltip (BST_ITEM_VIEW (self->sfont_page)->tree, sfrepo_tooltip);

  bst_track_synth_dialog_set (self, candidates, wrepo, sfrepo);
  bst_track_synth_dialog_setup (self, parent_widget,
                                /* TRANSLATORS: this is a dialog title and %s is replaced by an object name */
                                _("Synthesizer Selection: %s"),
                                track);

  self->selected_callback = selected_callback;
  self->selected_data = selected_data;
  self->selected_cleanup = selected_cleanup;
  gxk_widget_showraise (widget);

  return widget;
}

void
bst_track_synth_dialog_set (BstTrackSynthDialog *self,
                            BseIt3mSeq          *iseq,
                            SfiProxy             wrepo,
			    SfiProxy		 sfrepo)
{
  assert_return (BST_IS_TRACK_SYNTH_DIALOG (self));

  bst_item_view_set_container (BST_ITEM_VIEW (self->wpage), wrepo);
  bst_item_view_set_container (BST_ITEM_VIEW (self->sfont_page), sfrepo);
  bst_item_seq_store_set (self->pstore, iseq);
  g_object_set (self->spage, "visible", iseq != NULL, NULL);
  g_object_set (self->wpage, "visible", wrepo != 0, NULL);
  g_object_set (self->sfont_page, "visible", sfrepo != 0, NULL);
}

static void
bst_track_synth_dialog_activate (BstTrackSynthDialog *self)
{
  SfiProxy proxy = 0;

  if (self->ignore_activate)
    return;

  if (self->tview && gxk_widget_viewable (GTK_WIDGET (self->tview)))
    {
      GtkTreeIter siter;
      GtkTreeModel *smodel;
      if (gtk_tree_selection_get_selected (gtk_tree_view_get_selection (self->tview), &smodel, &siter))
        {
          GtkTreeIter piter;
          if (GTK_IS_TREE_MODEL_SORT (smodel))
            gtk_tree_model_sort_convert_iter_to_child_iter (GTK_TREE_MODEL_SORT (smodel), &piter, &siter);
          else
            piter = siter;
          proxy = bst_item_seq_store_get_from_iter (self->pstore, &piter);
        }
    }
  else if (self->wpage && gxk_widget_viewable (GTK_WIDGET (self->wpage)))
    {
      proxy = bst_item_view_get_current (BST_ITEM_VIEW (self->wpage));
    }
  else if (self->sfont_page && gxk_widget_viewable (GTK_WIDGET (self->sfont_page)))
    {
      proxy = bst_sound_font_view_get_preset (BST_SOUND_FONT_VIEW (self->sfont_page));
    }

  /* ignore_activate guards against multiple clicks */
  self->ignore_activate = TRUE;
  /* notify and done */
  BstTrackSynthDialogSelected selected_callback = self->selected_callback;
  GxkFreeFunc selected_cleanup = self->selected_cleanup;
  gpointer selected_data = self->selected_data;
  self->selected_callback = NULL;
  self->selected_cleanup = NULL;
  if (selected_callback)
    selected_callback (selected_data, proxy, self);
  if (selected_cleanup)
    selected_cleanup (selected_data);
  gxk_toplevel_delete (GTK_WIDGET (self));
}
