// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#ifndef __BSE_LADSPA_H__
#define __BSE_LADSPA_H__

#include <bse/bseutils.hh>
#include <gmodule.h>

/* --- object type macros --- */
#define BSE_TYPE_LADSPA_PLUGIN              (BSE_TYPE_ID (BseLadspaPlugin))
#define BSE_LADSPA_PLUGIN(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), BSE_TYPE_LADSPA_PLUGIN, BseLadspaPlugin))
#define BSE_LADSPA_PLUGIN_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST ((class), BSE_TYPE_LADSPA_PLUGIN, BseLadspaPluginClass))
#define BSE_IS_LADSPA_PLUGIN(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), BSE_TYPE_LADSPA_PLUGIN))
#define BSE_IS_LADSPA_PLUGIN_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE ((class), BSE_TYPE_LADSPA_PLUGIN))
#define BSE_LADSPA_PLUGIN_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), BSE_TYPE_LADSPA_PLUGIN, BseLadspaPluginClass))


/* --- BseLadspaPlugin --- */
struct BseLadspaInfo;
typedef struct {
  GType          type;
  BseLadspaInfo *info;
} BseLadspaTypeInfo;
struct BseLadspaPlugin : GObject {
  gchar             *fname;
  GModule           *gmodule;
  guint	             use_count;
  guint              n_types;
  BseLadspaTypeInfo *types;
};
struct BseLadspaPluginClass : GObjectClass
{};
struct BseLadspaPort {
  gchar        *ident;
  const gchar  *name;
  gfloat	minimum;
  gfloat	default_value;
  gfloat	maximum;
  guint		port_index;
  guint		audio_channel : 1;
  guint		input : 1;
  guint		output : 1;
  guint		boolean : 1;
  guint		integer_stepping : 1;
  guint		rate_relative : 1;	/* sample rate relative values */
  guint		frequency : 1;		/* provide logarithmic frequency slider */
  guint		logarithmic : 1;
  guint		concert_a : 1;		/* default to 440Hz concert A */
};
struct BseLadspaInfo {
  gchar         *file_path;		/* fully qualified file path and name */
  gchar	        *ident;			/* unique identifier */
  guint          plugin_id;		/* unique plugin type ID */
  const gchar   *name;			/* descriptive name */
  const gchar   *author;
  const gchar   *copyright;
  guint	         broken : 1;
  guint	         interactive : 1;	/* low-latency request */
  guint	         rt_capable : 1;	/* hard realtime capability */
  guint	         n_cports;
  BseLadspaPort *cports;
  guint	         n_aports;
  BseLadspaPort *aports;
  gconstpointer  descdata;
  gpointer     (*instantiate)	(gconstpointer	descdata,
				 gulong		sample_rate);
  void	       (*connect_port)	(gpointer	instance,
				 gulong		port_index,
				 gfloat	       *location);
  void	       (*activate)	(gpointer	instance);
  void	       (*run)		(gpointer	instance,
				 gulong		n_samples);
  void	       (*deactivate)	(gpointer	instance);
  void	       (*cleanup)	(gpointer	instance);
};

BseLadspaInfo*	bse_ladspa_info_assemble	  (const gchar		*file_path,
						   gconstpointer	 ladspa_descriptor);
void		bse_ladspa_info_free		  (BseLadspaInfo	*bli);
const gchar*    bse_ladspa_plugin_check_load      (const gchar		*file_name);
gchar*		bse_ladspa_info_port_2str	  (BseLadspaPort	*port);

Bse::StringVector bse_ladspa_plugin_path_list_files (void);

#endif /* __BSE_LADSPA_H__ */
