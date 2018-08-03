// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#ifndef __BST_ITEM_VIEW_H__
#define __BST_ITEM_VIEW_H__

#include	"bstutils.hh"

/* --- Gtk+ type macros --- */
#define	BST_TYPE_ITEM_VIEW	      (bst_item_view_get_type ())
#define	BST_ITEM_VIEW(object)	      (GTK_CHECK_CAST ((object), BST_TYPE_ITEM_VIEW, BstItemView))
#define	BST_ITEM_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BST_TYPE_ITEM_VIEW, BstItemViewClass))
#define	BST_IS_ITEM_VIEW(object)      (GTK_CHECK_TYPE ((object), BST_TYPE_ITEM_VIEW))
#define	BST_IS_ITEM_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BST_TYPE_ITEM_VIEW))
#define BST_ITEM_VIEW_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), BST_TYPE_ITEM_VIEW, BstItemViewClass))

#define	BST_ITEM_VIEW_TREE_HEIGHT     (120)


/* --- structures & typedefs --- */
typedef	struct	_BstItemView		BstItemView;
typedef	struct	_BstItemViewClass	BstItemViewClass;
struct _BstItemView
{
  GtkAlignment	  parent_object;

  GtkTreeView    *tree;
  GxkListWrapper *wlist;

  GtkWidget	 *pview;

  Bse::ContainerS container;
  SfiProxy	 auto_select;
  GtkWidget    **op_widgets;
};
struct _BstItemViewClass
{
  GtkAlignmentClass parent_class;

  const gchar	   *item_type;

  void	      (*set_container)	(BstItemView	*self,
				 SfiProxy	 new_container);
  void	      (*listen_on)	(BstItemView	*self,
				 SfiProxy	 item);
  void	      (*unlisten_on)	(BstItemView	*self,
				 SfiProxy	 item);
};


/* --- prototypes --- */
GType		bst_item_view_get_type		(void);
void		bst_item_view_select		(BstItemView	*item_view,
						 SfiProxy	 item);
SfiProxy	bst_item_view_get_current	(BstItemView	*item_view);
Bse::PartH	bst_item_view_get_current_part	(BstItemView	*self);
SfiProxy	bst_item_view_get_proxy		(BstItemView	*item_view,
						 gint            row);
gint            bst_item_view_get_proxy_row     (BstItemView    *self,
                                                 SfiProxy        item);
void		bst_item_view_set_container	(BstItemView	*item_view,
						 SfiProxy	 new_container);
void		bst_item_view_set_tree  	(BstItemView	*item_view,
						 GtkTreeView    *tree);
void            bst_item_view_complete_tree     (BstItemView    *self,
						 GtkTreeView    *tree);
void            bst_item_view_build_param_view  (BstItemView    *self,
                                                 GtkContainer   *container);
void		bst_item_view_refresh   	(BstItemView    *self,
						 SfiProxy        item);
void		bst_item_view_name_edited	(BstItemView    *self,
						 const gchar    *strpath,
						 const gchar    *text);
void		bst_item_view_blurb_edited	(BstItemView    *self,
						 const gchar    *strpath,
						 const gchar    *text);
void		bst_item_view_enable_param_view	(BstItemView    *self,
                                                 gboolean        enabled);
GtkTreeModel* bst_item_view_adapt_list_wrapper	(BstItemView	*self,
						 GxkListWrapper *lwrapper);


#endif /* __BST_ITEM_VIEW_H__ */
