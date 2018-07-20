// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include "bsefreeverb.hh"

#include <bse/bseengine.hh>
#include <bse/bsecxxplugin.hh>


/* --- properties --- */
enum
{
  PROP_0,
  PROP_ROOM_SIZE,
  PROP_DAMPING,
  PROP_WET_LEVEL,
  PROP_DRY_LEVEL,
  PROP_WIDTH
};


/* --- prototypes --- */
static void	bse_free_verb_init		(BseFreeVerb		*self);
static void	bse_free_verb_class_init	(BseFreeVerbClass	*klass);
static void	bse_free_verb_set_property	(GObject		*object,
						 guint			 param_id,
						 const GValue		*value,
						 GParamSpec		*pspec);
static void	bse_free_verb_get_property	(GObject		*object,
						 guint			 param_id,
						 GValue			*value,
						 GParamSpec		*pspec);
static void	bse_free_verb_context_create	(BseSource		*source,
						 guint			 context_handle,
						 BseTrans		*trans);
static void	bse_free_verb_update_modules	(BseFreeVerb		*self);

// == Type Registration ==
#include "../icons/reverb.c"
BSE_RESIDENT_SOURCE_DEF (BseFreeVerb, bse_free_verb, N_("Filters/Free Verb"),
                         "BseFreeVerb - Free, studio-quality reverb (SOURCE CODE in the public domain) "
                         "Written by Jezar at Dreampoint - http://www.dreampoint.co.uk",
                         reverb_icon);

/* --- variables --- */
static gpointer        parent_class = NULL;


/* --- functions --- */
static void
bse_free_verb_class_init (BseFreeVerbClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  BseObjectClass *object_class = BSE_OBJECT_CLASS (klass);
  BseSourceClass *source_class = BSE_SOURCE_CLASS (klass);
  BseFreeVerbConstants *constants = &klass->constants;
  BseFreeVerbConfig defaults;
  guint channel;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->set_property = bse_free_verb_set_property;
  gobject_class->get_property = bse_free_verb_get_property;

  source_class->context_create = bse_free_verb_context_create;

  bse_free_verb_cpp_defaults (&defaults, constants);

  bse_object_class_add_param (object_class, "Reverb Style",
			      PROP_ROOM_SIZE,
			      sfi_pspec_real ("room_size", "Room Size", NULL,
					      constants->room_offset + constants->room_scale * defaults.room_size,
					      constants->room_offset,
					      constants->room_offset + constants->room_scale * 1.0,
					      0.1 * constants->room_scale,
					      SFI_PARAM_STANDARD ":dial"));
  bse_object_class_add_param (object_class, "Reverb Style",
			      PROP_DAMPING,
			      sfi_pspec_real ("damping", "Damping [%]", NULL,
					      constants->damp_scale * defaults.damp,
					      0, constants->damp_scale * 1.0,
					      0.1 * constants->damp_scale,
					      SFI_PARAM_STANDARD ":dial"));
  bse_object_class_add_param (object_class, "Reverb Style",
			      PROP_WET_LEVEL,
			      sfi_pspec_real ("wet_level", "Wet Level [dB]", NULL,
					      constants->wet_scale * defaults.wet,
					      0, constants->wet_scale * 1.0,
					      0.1 * constants->wet_scale,
					      SFI_PARAM_STANDARD ":dial"));
  bse_object_class_add_param (object_class, "Reverb Style",
			      PROP_DRY_LEVEL,
			      sfi_pspec_real ("dry_level", "Dry Level [dB]", NULL,
					      constants->dry_scale * defaults.dry,
					      0, constants->dry_scale * 1.0,
					      0.1 * constants->dry_scale,
					      SFI_PARAM_STANDARD ":dial"));
  bse_object_class_add_param (object_class, "Reverb Style",
			      PROP_WIDTH,
			      sfi_pspec_real ("width", "Width [%]", NULL,
					      constants->width_scale * defaults.width,
					      0, constants->width_scale * 1.0,
					      0.1 * constants->width_scale,
					      SFI_PARAM_STANDARD ":dial"));
  channel = bse_source_class_add_ichannel (source_class, "left-audio-in", _("Left Audio In"), _("Left Input"));
  channel = bse_source_class_add_ichannel (source_class, "right-audio-in", _("Right Audio In"), _("Right Input"));
  channel = bse_source_class_add_ochannel (source_class, "left-audio-out", _("Left Audio Out"), _("Left Output"));
  channel = bse_source_class_add_ochannel (source_class, "right-audio-out", _("Right Audio Out"), _("Right Output"));
  (void) channel;
}

static void
bse_free_verb_init (BseFreeVerb *self)
{
  bse_free_verb_cpp_defaults (&self->config, NULL);
}

static void
bse_free_verb_set_property (GObject      *object,
			    guint         param_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
  BseFreeVerb *self = BSE_FREE_VERB (object);
  BseFreeVerbConstants *constants = &BSE_FREE_VERB_GET_CLASS (self)->constants;

  switch (param_id)
    {
    case PROP_ROOM_SIZE:
      self->config.room_size = (sfi_value_get_real (value) - constants->room_offset) / constants->room_scale;
      bse_free_verb_update_modules (self);
      break;
    case PROP_DAMPING:
      self->config.damp = sfi_value_get_real (value) / constants->damp_scale;
      bse_free_verb_update_modules (self);
      break;
    case PROP_WET_LEVEL:
      self->config.wet = sfi_value_get_real (value) / constants->wet_scale;
      bse_free_verb_update_modules (self);
      break;
    case PROP_DRY_LEVEL:
      self->config.dry = sfi_value_get_real (value) / constants->dry_scale;
      bse_free_verb_update_modules (self);
      break;
    case PROP_WIDTH:
      self->config.width = sfi_value_get_real (value) / constants->width_scale;
      bse_free_verb_update_modules (self);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, param_id, pspec);
      break;
    }
}

static void
bse_free_verb_get_property (GObject    *object,
			    guint       param_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  BseFreeVerb *self = BSE_FREE_VERB (object);
  BseFreeVerbConstants *constants = &BSE_FREE_VERB_GET_CLASS (self)->constants;

  switch (param_id)
    {
    case PROP_ROOM_SIZE:
      sfi_value_set_real (value, self->config.room_size * constants->room_scale + constants->room_offset);
      break;
    case PROP_DAMPING:
      sfi_value_set_real (value, self->config.damp * constants->damp_scale);
      break;
    case PROP_WET_LEVEL:
      sfi_value_set_real (value, self->config.wet * constants->wet_scale);
      break;
    case PROP_DRY_LEVEL:
      sfi_value_set_real (value, self->config.dry * constants->dry_scale);
      break;
    case PROP_WIDTH:
      sfi_value_set_real (value, self->config.width * constants->width_scale);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (self, param_id, pspec);
      break;
    }
}

static void
free_verb_access (BseModule *module,
		  gpointer   data)
{
  BseFreeVerbCpp *cpp = (BseFreeVerbCpp*) module->user_data;
  BseFreeVerbConfig *config = (BseFreeVerbConfig*) data;

  /* this runs in the Gsl Engine threads */
  bse_free_verb_cpp_configure (cpp, config);

  /* save config for _reset() */
  bse_free_verb_cpp_save_config (cpp, config);
}

static void
bse_free_verb_update_modules (BseFreeVerb *self)
{
  if (BSE_SOURCE_PREPARED (self))
    bse_source_access_modules (BSE_SOURCE (self),
			       free_verb_access,
			       g_memdup (&self->config, sizeof (self->config)),
			       g_free,
			       NULL);
}

static void
free_verb_process (BseModule *module,
		   guint      n_values)
{
  BseFreeVerbCpp *cpp = (BseFreeVerbCpp*) module->user_data;
  const gfloat *ileft = BSE_MODULE_IBUFFER (module, BSE_FREE_VERB_ICHANNEL_LEFT);
  const gfloat *iright = BSE_MODULE_IBUFFER (module, BSE_FREE_VERB_ICHANNEL_RIGHT);
  gfloat *oleft = BSE_MODULE_OBUFFER (module, BSE_FREE_VERB_OCHANNEL_LEFT);
  gfloat *oright = BSE_MODULE_OBUFFER (module, BSE_FREE_VERB_OCHANNEL_RIGHT);

  bse_free_verb_cpp_process (cpp, n_values, ileft, iright, oleft, oright);
}

static void
free_verb_reset (BseModule *module)
{
  BseFreeVerbCpp *cpp = (BseFreeVerbCpp*) module->user_data;
  BseFreeVerbConfig config;

  bse_free_verb_cpp_restore_config (cpp, &config);
  bse_free_verb_cpp_destroy (cpp);
  bse_free_verb_cpp_create (cpp);
  bse_free_verb_cpp_configure (cpp, &config);
  bse_free_verb_cpp_save_config (cpp, &config);
}

static void
free_verb_destroy (gpointer        data,
		   const BseModuleClass *klass)
{
  BseFreeVerbCpp *cpp = (BseFreeVerbCpp*) data;

  bse_free_verb_cpp_destroy (cpp);
  g_free (cpp);
}

static void
bse_free_verb_context_create (BseSource *source,
			      guint      context_handle,
			      BseTrans  *trans)
{
  static const BseModuleClass free_verb_class = {
    BSE_FREE_VERB_N_ICHANNELS,	/* n_istreams */
    0,				/* n_jstreams */
    BSE_FREE_VERB_N_OCHANNELS,	/* n_ostreams */
    free_verb_process,		/* process */
    NULL,			/* process_defer */
    free_verb_reset,		/* reset */
    free_verb_destroy,		/* free */
    Bse::ModuleFlag::EXPENSIVE,		/* cost */
  };
  BseFreeVerb *self = BSE_FREE_VERB (source);
  BseFreeVerbCpp *cpp = g_new0 (BseFreeVerbCpp, 1);
  BseModule *module;

  /* initialize module data */
  bse_free_verb_cpp_create (cpp);
  bse_free_verb_cpp_configure (cpp, &self->config);
  bse_free_verb_cpp_save_config (cpp, &self->config);
  module = bse_module_new (&free_verb_class, cpp);

  /* commit module to engine */
  bse_trans_add (trans, bse_job_integrate (module));

  /* setup module i/o streams with BseSource i/o channels */
  bse_source_set_context_module (source, context_handle, module, trans);

  /* chain parent class' handler */
  BSE_SOURCE_CLASS (parent_class)->context_create (source, context_handle, trans);
}
