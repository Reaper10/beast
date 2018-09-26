// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include <gxk/gxk.hh>
#include <string.h>

#define PRGNAME "tsmview"

/* --- functions --- */
static GtkWidget*
textget_handler (gpointer              user_data,
                 const gchar          *element_name,
                 const gchar         **attribute_names,
                 const gchar         **attribute_values)
{
  return (GtkWidget*) g_object_new (GTK_TYPE_LABEL,
                                    "visible", TRUE,
                                    "label", element_name,
                                    NULL);
}

int
main (int   argc,
      char *argv[])
{
  GtkWidget *sctext, *dialog;
  gchar *str, *title = NULL;
  uint i;
  GxkScrollTextFlags flags = GxkScrollTextFlags (0);
  /* initialize modules
   */
  gtk_init (&argc, &argv);
  gxk_init ();

  gxk_text_register_textget_handler ("textget-label", textget_handler, NULL);

  for (i = 1; i < uint (argc); i++)
    if (!flags && strcmp (argv[i], "--edit") == 0)
      {
	flags = GXK_SCROLL_TEXT_EDITABLE;
	argv[i] = NULL;
	if (title)
	  break;
      }
    else if (!title)
      {
	title = g_strdup (argv[i]);
	argv[i] = NULL;
	if (flags)
	  break;
      }
  if (!title)
    title = g_strdup (".");
  gxk_text_add_tsm_path (".");
  gxk_text_add_tsm_path (Bse::runpath (Bse::RPath::DOCDIR).c_str());
  gxk_text_add_tsm_path (Bse::installpath (Bse::INSTALLPATH_DATADIR_IMAGES).c_str());
  sctext = gxk_scroll_text_create (GXK_SCROLL_TEXT_NAVIGATABLE | flags, NULL);
  gxk_scroll_text_enter (sctext, title);
  for (i = 1; i < uint (argc); i++)
    if (argv[i])
      {
	gxk_scroll_text_enter (sctext, argv[i]);
	str = title;
	title = g_strconcat (title, " ", argv[i], NULL);
	g_free (str);
      }

  str = title;
  title = g_strdup ("tsmview");	// g_strconcat (title, " - tsmview", NULL);
  g_free (str);

  dialog = (GtkWidget*) gxk_dialog_new (NULL, NULL, GXK_DIALOG_DELETE_BUTTON, title, NULL);
  g_free (title);

  g_object_connect (dialog, "signal::destroy", gtk_main_quit, NULL, NULL);

  gxk_dialog_set_child (GXK_DIALOG (dialog), sctext);

  g_object_set (dialog,
		"default_width", 560,
		"default_height", 640,
		"visible", TRUE,
		NULL);
  gtk_main ();

  return 0;
}
