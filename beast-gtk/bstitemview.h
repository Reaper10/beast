/* BEAST - Bedevilled Audio System
 * Copyright (C) 1998-2003 Tim Janik
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __BST_ITEM_VIEW_H__
#define __BST_ITEM_VIEW_H__

#include	"bstutils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* --- Gtk+ type macros --- */
#define	BST_TYPE_ITEM_VIEW	      (bst_item_view_get_type ())
#define	BST_ITEM_VIEW(object)	      (GTK_CHECK_CAST ((object), BST_TYPE_ITEM_VIEW, BstItemView))
#define	BST_ITEM_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BST_TYPE_ITEM_VIEW, BstItemViewClass))
#define	BST_IS_ITEM_VIEW(object)      (GTK_CHECK_TYPE ((object), BST_TYPE_ITEM_VIEW))
#define	BST_IS_ITEM_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BST_TYPE_ITEM_VIEW))
#define BST_ITEM_VIEW_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), BST_TYPE_ITEM_VIEW, BstItemViewClass))


/* --- structures & typedefs --- */
typedef	struct	_BstItemView		BstItemView;
typedef	struct	_BstItemViewClass	BstItemViewClass;
typedef	struct	_BstItemViewOp		BstItemViewOp;
struct _BstItemView
{
  GtkAlignment	  parent_object;

  GxkListWrapper *wlist;
  GtkTreeView    *tree;
  GtkWidget	 *pview;

  const gchar	*item_type;
  SfiProxy	 container;
  gchar		*id_format;

  GtkWidget	**op_widgets;
};
struct _BstItemViewClass
{
  GtkAlignmentClass parent_class;

  guint		    n_ops;
  BstItemViewOp	   *ops;

  guint             default_param_view_height;

  void          (*operate)       (BstItemView	*item_view,
				  BstOps	 op);
  gboolean      (*can_operate)   (BstItemView	*item_view,
				  BstOps         op);
};
struct _BstItemViewOp
{
  gchar	      *op_name;
  guint	       op;
  const gchar *stock_icon;
};


/* --- prototypes --- */
GtkType		bst_item_view_get_type		(void);
void		bst_item_view_select		(BstItemView	*item_view,
						 SfiProxy	 item);
SfiProxy	bst_item_view_get_current	(BstItemView	*item_view);
void		bst_item_view_set_container	(BstItemView	*item_view,
						 SfiProxy	 new_container);
void		bst_item_view_operate		(BstItemView	*item_view,
						 BstOps		 op);
gboolean	bst_item_view_can_operate	(BstItemView	*item_view,
						 BstOps		 op);
void		bst_item_view_set_id_format	(BstItemView	*item_view,
						 const gchar	*id_format);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BST_ITEM_VIEW_H__ */
