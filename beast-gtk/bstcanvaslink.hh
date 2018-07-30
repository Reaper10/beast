// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#ifndef __BST_CANVAS_LINK_H__
#define __BST_CANVAS_LINK_H__

#include	"bstutils.hh"
#include	"bstcanvassource.hh"



/* --- Gtk+ type macros --- */
#define BST_TYPE_CANVAS_LINK            (bst_canvas_link_get_type ())
#define BST_CANVAS_LINK(object)         (GTK_CHECK_CAST ((object), BST_TYPE_CANVAS_LINK, BstCanvasLink))
#define BST_CANVAS_LINK_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BST_TYPE_CANVAS_LINK, BstCanvasLinkClass))
#define BST_IS_CANVAS_LINK(object)      (GTK_CHECK_TYPE ((object), BST_TYPE_CANVAS_LINK))
#define BST_IS_CANVAS_LINK_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BST_TYPE_CANVAS_LINK))
#define BST_CANVAS_LINK_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), BST_TYPE_CANVAS_LINK, BstCanvasLinkClass))


/* --- structures & typedefs --- */
typedef	struct	_BstCanvasLink		BstCanvasLink;
typedef	struct	_BstCanvasLinkClass	BstCanvasLinkClass;
struct _BstCanvasLink
{
  GnomeCanvasGroup parent_object;

  GnomeCanvasItem *line;
  GnomeCanvasItem *arrow;
  GnomeCanvasItem *tag_start, *tag_end;

  BstCanvasSource *ocsource;
  guint            ochannel;
  gulong           oc_handler;
  Bse::SourceH     oc_bsesource;
  BstCanvasSource *icsource;
  guint            ichannel;
  gulong           ic_handler;
  Bse::SourceH     ic_bsesource;

  guint		   in_move : 1;
  gdouble          start_move_dx, start_move_dy;
  gdouble          end_move_dx, end_move_dy;

  GtkWidget	  *link_view;
};
struct _BstCanvasLinkClass
{
  GnomeCanvasGroupClass parent_class;
};


/* --- prototypes --- */
GtkType		 bst_canvas_link_get_type		(void);
GnomeCanvasItem* bst_canvas_link_new			(GnomeCanvasGroup *group);
void		 bst_canvas_link_set_ocsource		(BstCanvasLink    *clink,
							 BstCanvasSource  *ocsource,
							 guint             ochannel);
void		 bst_canvas_link_set_icsource		(BstCanvasLink    *clink,
							 BstCanvasSource  *icsource,
							 guint             ichannel);
void             bst_canvas_link_popup_view		(BstCanvasLink	  *clink);
void             bst_canvas_link_toggle_view		(BstCanvasLink	  *clink);
BstCanvasLink*	 bst_canvas_link_at			(GnomeCanvas      *canvas,
							 gdouble           world_x,
							 gdouble           world_y);
BstCanvasSource* bst_canvas_link_csource_at		(BstCanvasLink	  *clink,
							 gdouble           world_x,
							 gdouble           world_y);




#endif /* __BST_CANVAS_LINK_H__ */
