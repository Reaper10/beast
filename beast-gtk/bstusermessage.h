/* BEAST - Bedevilled Audio System
 * Copyright (C) 2002 Tim Janik
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
#ifndef __BST_USER_MESSAGE_H__
#define __BST_USER_MESSAGE_H__

#include	"bstutils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* --- prototypes --- */
void	   bst_user_messages_listen	(void);
void	   bst_user_messages_kill	(void);
GtkWidget* bst_user_message_dialog_new	(BswProxy	script_control);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif	/* __BST_USER_MESSAGE_H__ */
