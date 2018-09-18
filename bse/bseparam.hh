// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#ifndef __BSE_PARAM_H__
#define __BSE_PARAM_H__

#include <bse/bsetype.hh>
#include <bse/bseutils.hh>



/* --- object param specs --- */
#define	    BSE_TYPE_PARAM_OBJECT		(G_TYPE_PARAM_OBJECT)
#define	    BSE_IS_PARAM_SPEC_OBJECT(pspec)	(G_TYPE_CHECK_INSTANCE_TYPE ((pspec), BSE_TYPE_PARAM_OBJECT))
#define	    BSE_PARAM_SPEC_OBJECT(pspec)	(G_TYPE_CHECK_INSTANCE_CAST ((pspec), BSE_TYPE_PARAM_OBJECT, BseParamSpecObject))
typedef	    GParamSpecObject			 BseParamSpecObject;
GParamSpec* bse_param_spec_object		(const gchar	*name,
						 const gchar	*nick,
						 const gchar	*blurb,
						 GType		 object_type,
						 const gchar	*hints);

#define	    BSE_VALUE_HOLDS_OBJECT(value)	(G_TYPE_CHECK_VALUE_TYPE ((value), BSE_TYPE_OBJECT))
#define	    bse_value_get_object		 g_value_get_object
#define	    bse_value_set_object		 g_value_set_object
#define	    bse_value_take_object		 g_value_take_object
GValue*	    bse_value_object			(gpointer	 vobject);


/* --- boxed parameters --- */
typedef GParamSpecBoxed			 BseParamSpecBoxed;
#define BSE_TYPE_PARAM_BOXED		(G_TYPE_PARAM_BOXED)
#define BSE_IS_PARAM_SPEC_BOXED(pspec)	(G_TYPE_CHECK_INSTANCE_TYPE ((pspec), BSE_TYPE_PARAM_BOXED))
#define BSE_PARAM_SPEC_BOXED(pspec)	(G_TYPE_CHECK_INSTANCE_CAST ((pspec), BSE_TYPE_PARAM_BOXED, BseParamSpecBoxed))
#define BSE_VALUE_HOLDS_BOXED(value)	(G_TYPE_CHECK_VALUE_TYPE ((value), G_TYPE_BOXED))
GParamSpec* bse_param_spec_boxed	(const gchar  *name,
					 const gchar  *nick,
					 const gchar  *blurb,
					 GType	       boxed_type,
					 const gchar  *hints);
#define     bse_value_get_boxed          g_value_get_boxed
#define     bse_value_set_boxed          g_value_set_boxed
#define     bse_value_dup_boxed          g_value_dup_boxed
#define     bse_value_take_boxed         g_value_take_boxed


/* --- convenience pspec constructors --- */
GParamSpec* bse_param_spec_freq         (const gchar  *name,
					 const gchar  *nick,
					 const gchar  *blurb,
					 SfiReal       default_freq,
                                         SfiReal       min_freq,
                                         SfiReal       max_freq,
					 const gchar  *hints);


#endif /* __BSE_PARAM_H__ */
