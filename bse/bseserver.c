/* BSE - Bedevilled Sound Engine
 * Copyright (C) 1997-1999, 2000-2002 Tim Janik
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */
#include "bseserver.h"


#include "bseproject.h"
#include "gslengine.h"
#include "gslcommon.h"
#include "bsemarshal.h"
#include "bsemidimodule.h"
#include "bsemain.h"		/* threads enter/leave */


/* --- PCM GslModule implementations ---*/
#include "bsepcmmodule.c"


/* --- parameters --- */
enum
{
  PARAM_0,
  PARAM_PCM_LATENCY,
};


/* --- prototypes --- */
static void	 bse_server_class_init		(BseServerClass	   *class);
static void	 bse_server_init		(BseServer	   *server);
static void	 bse_server_destroy		(BseObject	   *object);
static void	 bse_server_set_property	(BseServer	   *server,
						 guint              param_id,
						 GValue            *value,
						 GParamSpec        *pspec);
static void	 bse_server_get_property	(BseServer	   *server,
						 guint              param_id,
						 GValue            *value,
						 GParamSpec        *pspec);
static void	 engine_init			(BseServer	   *server,
						 gfloat		    mix_freq);
static void	 engine_shutdown		(BseServer	   *server);
static gboolean	 iowatch_remove			(BseServer	   *server,
						 BseIOWatch	    watch_func,
						 gpointer	    data);
static void	 iowatch_add			(BseServer	   *server,
						 gint		    fd,
						 GIOCondition	    events,
						 BseIOWatch	    watch_func,
						 gpointer	    data);


/* --- variables --- */
static GTypeClass *parent_class = NULL;
static guint       signal_script_status = 0;


/* --- functions --- */
BSE_BUILTIN_TYPE (BseServer)
{
  static const GTypeInfo server_info = {
    sizeof (BseServerClass),
    
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) bse_server_class_init,
    (GClassFinalizeFunc) NULL,
    NULL /* class_data */,
    
    sizeof (BseServer),
    0 /* n_preallocs */,
    (GInstanceInitFunc) bse_server_init,
  };
  
  return bse_type_register_static (BSE_TYPE_ITEM,
				   "BseServer",
				   "BSE Server type",
				   &server_info);
}

static void
bse_server_class_init (BseServerClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);
  BseObjectClass *object_class = BSE_OBJECT_CLASS (class);
  
  parent_class = g_type_class_peek_parent (class);
  
  gobject_class->set_property = (GObjectSetPropertyFunc) bse_server_set_property;
  gobject_class->get_property = (GObjectGetPropertyFunc) bse_server_get_property;
  object_class->destroy = bse_server_destroy;

  bse_object_class_add_param (object_class, "PCM Settings",
			      PARAM_PCM_LATENCY,
			      bse_param_spec_uint ("latency", "Latency [ms]", NULL,
						   1, 2000,
						   50, 5,
						   BSE_PARAM_DEFAULT | G_PARAM_CONSTRUCT));

  signal_script_status = bse_object_class_add_signal (object_class, "script-status",
						      bse_marshal_VOID__ENUM_STRING_FLOAT_ENUM, NULL,
						      G_TYPE_NONE, 4,
						      BSE_TYPE_SCRIPT_STATUS, G_TYPE_STRING,
						      G_TYPE_FLOAT, BSE_TYPE_ERROR_TYPE);
}

static void
bse_server_init (BseServer *server)
{
  server->engine_source = NULL;
  server->projects = NULL;
  server->dev_use_count = 0;
  server->pcm_device = NULL;
  server->pcm_imodule = NULL;
  server->pcm_omodule = NULL;
  server->pcm_ref_count = 0;
  server->midi_device = NULL;
  server->midi_decoder = NULL;
  server->midi_modules = NULL;
  server->midi_ref_count = 0;
  server->main_context = g_main_context_default ();
  g_main_context_ref (server->main_context);
  BSE_OBJECT_SET_FLAGS (server, BSE_ITEM_FLAG_SINGLETON);
}

static void
bse_server_destroy (BseObject *object)
{
  // BseServer *server = BSE_SERVER (object);

  g_error ("BseServer got unreferenced, though persistent-singleton");
  
  /* chain parent class' destroy handler */
  BSE_OBJECT_CLASS (parent_class)->destroy (object);
}


static void
bse_server_set_property (BseServer  *server,
			 guint       param_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
  switch (param_id)
    {
      BsePcmHandle *handle;
    case PARAM_PCM_LATENCY:
      server->pcm_latency = g_value_get_uint (value);
      handle = server->pcm_device ? bse_pcm_device_get_handle (server->pcm_device) : NULL;
      if (handle)
	bse_pcm_handle_set_watermark (handle, server->pcm_latency);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (server, param_id, pspec);
      break;
    }
}

static void
bse_server_get_property (BseServer  *server,
			 guint       param_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
  switch (param_id)
    {
    case PARAM_PCM_LATENCY:
      g_value_set_uint (value, server->pcm_latency);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (server, param_id, pspec);
      break;
    }
}

/**
 * bse_server_get
 * @Returns: Global BSE Server
 *
 * Retrive the global BSE server object.
 **/
BseServer*
bse_server_get (void)
{
  static BseServer *server = NULL;

  if (!server)
    {
      server = bse_object_new (BSE_TYPE_SERVER, NULL);
      g_object_ref (server);
    }

  return server;
}

static void
destroy_project (BseProject *project,
		 BseServer  *server)
{
  server->projects = g_list_remove (server->projects, project);
}

BseProject*
bse_server_create_project (BseServer   *server,
			   const gchar *name)
{
  BseProject *project;

  g_return_val_if_fail (BSE_IS_SERVER (server), NULL);
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (bse_server_find_project (server, name) == NULL, NULL);

  project = g_object_new (BSE_TYPE_PROJECT,
			  "uname", name,
			  NULL);
  server->projects = g_list_prepend (server->projects, project);
  g_object_connect (project,
		    "signal::destroy", destroy_project, server,
		    NULL);

  return project;
}

BseProject*
bse_server_find_project (BseServer   *server,
			 const gchar *name)
{
  GList *node;

  g_return_val_if_fail (BSE_IS_SERVER (server), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  for (node = server->projects; node; node = node->next)
    {
      BseProject *project = node->data;
      gchar *uname = BSE_OBJECT_UNAME (project);

      if (uname && strcmp (name, uname) == 0)
	return project;
    }
  return NULL;
}

void
bse_server_pick_default_devices (BseServer *server)
{
  GType *children, choice = 0;
  guint n, i, rating;

  g_return_if_fail (BSE_IS_SERVER (server));
  g_return_if_fail (server->pcm_device == NULL);
  g_return_if_fail (server->midi_device == NULL);

  /* pcm device driver implementations all derive from BsePcmDevice */
  children = g_type_children (BSE_TYPE_PCM_DEVICE, &n);
  /* pick class with highest rating */
  rating = 0;
  for (i = 0; i < n; i++)
    {
      BsePcmDeviceClass *class = g_type_class_ref (children[i]);

      if (class->driver_rating > rating)
	{
	  rating = class->driver_rating;
	  choice = children[i];
	}
      g_type_class_unref (class);
    }
  g_free (children);
  if (rating)
    server->pcm_device = g_object_new (choice, NULL);

  /* midi device driver implementations all derive from BseMidiDevice */
  children = g_type_children (BSE_TYPE_MIDI_DEVICE, &n);
  /* pick class with highest rating */
  rating = 0;
  for (i = 0; i < n; i++)
    {
      BseMidiDeviceClass *class = g_type_class_ref (children[i]);

      if (class->driver_rating > rating)
	{
	  rating = class->driver_rating;
	  choice = children[i];
	}
      g_type_class_unref (class);
    }
  g_free (children);
  if (rating)
    server->midi_device = g_object_new (choice, NULL);
}

BseErrorType
bse_server_activate_devices (BseServer *server)
{
  BseErrorType error = BSE_ERROR_NONE;

  g_return_val_if_fail (BSE_IS_SERVER (server), BSE_ERROR_INTERNAL);
  g_return_val_if_fail (server->midi_decoder == NULL, BSE_ERROR_INTERNAL);

  if (!server->pcm_device || !server->midi_device)
    bse_server_pick_default_devices (server);
  if (!server->pcm_device || !server->midi_device)
    return BSE_ERROR_INTERNAL;	/* shouldn't happen */
  
  if (!error)
    error = bse_pcm_device_open (server->pcm_device);
  if (!error)
    {
      server->midi_decoder = bse_midi_decoder_new ();
      error = bse_midi_device_open (server->midi_device, server->midi_decoder);
      if (error)
	{
	  bse_pcm_device_suspend (server->pcm_device);
	  bse_midi_decoder_destroy (server->midi_decoder);
	  server->midi_decoder = NULL;
	}
    }
  if (!error)
    {
      GslTrans *trans;

      bse_pcm_handle_set_watermark (bse_pcm_device_get_handle (server->pcm_device),
				    server->pcm_latency);
      engine_init (server, bse_pcm_device_get_handle (server->pcm_device)->mix_freq);

      trans = gsl_trans_open ();
      server->pcm_imodule = bse_pcm_imodule_insert (bse_pcm_device_get_handle (server->pcm_device), trans);
      server->pcm_omodule = bse_pcm_omodule_insert (bse_pcm_device_get_handle (server->pcm_device), trans);
      gsl_trans_commit (trans);
    }
  
  return error;
}

void
bse_server_suspend_devices (BseServer *server)
{
  GslTrans *trans;
  GSList *slist;

  g_return_if_fail (BSE_IS_SERVER (server));
  g_return_if_fail (server->pcm_ref_count == 0);
  g_return_if_fail (server->midi_ref_count == 0);

  trans = gsl_trans_open ();
  if (server->pcm_omodule)
    {
      bse_pcm_imodule_remove (server->pcm_imodule, trans);
      server->pcm_imodule = NULL;
      bse_pcm_omodule_remove (server->pcm_omodule, trans);
      server->pcm_omodule = NULL;
    }
  for (slist = server->midi_modules; slist; slist = slist->next)
    bse_midi_module_remove (slist->data, trans);
  g_slist_free (server->midi_modules);
  server->midi_modules = NULL;
  gsl_trans_commit (trans);
  
  /* wait until transaction has been processed */
  gsl_engine_wait_on_trans ();
  
  bse_pcm_device_suspend (server->pcm_device);
  bse_midi_device_suspend (server->midi_device);
  bse_midi_decoder_destroy (server->midi_decoder);
  server->midi_decoder = NULL;

  engine_shutdown (server);
}

GslModule*
bse_server_retrive_pcm_output_module (BseServer   *server,
				      BseSource   *source,
				      const gchar *uplink_name)
{
  g_return_val_if_fail (BSE_IS_SERVER (server), NULL);
  g_return_val_if_fail (BSE_IS_SOURCE (source), NULL);
  g_return_val_if_fail (uplink_name != NULL, NULL);
  g_return_val_if_fail (server->pcm_omodule != NULL, NULL); // FIXME server->pcm_devices_open

  server->pcm_ref_count += 1;

  return server->pcm_omodule;
}

void
bse_server_discard_pcm_output_module (BseServer *server,
				      GslModule *module)
{
  g_return_if_fail (BSE_IS_SERVER (server));
  g_return_if_fail (module != NULL);
  g_return_if_fail (server->pcm_ref_count > 0);

  g_return_if_fail (server->pcm_omodule == module); // FIXME

  server->pcm_ref_count -= 1;
}

GslModule*
bse_server_retrive_pcm_input_module (BseServer   *server,
				     BseSource   *source,
				     const gchar *uplink_name)
{
  g_return_val_if_fail (BSE_IS_SERVER (server), NULL);
  g_return_val_if_fail (BSE_IS_SOURCE (source), NULL);
  g_return_val_if_fail (uplink_name != NULL, NULL);
  g_return_val_if_fail (server->pcm_imodule != NULL, NULL); // FIXME server->pcm_devices_open

  server->pcm_ref_count += 1;

  return server->pcm_imodule;
}

void
bse_server_discard_pcm_input_module (BseServer *server,
				     GslModule *module)
{
  g_return_if_fail (BSE_IS_SERVER (server));
  g_return_if_fail (module != NULL);
  g_return_if_fail (server->pcm_ref_count > 0);

  g_return_if_fail (server->pcm_imodule == module); // FIXME

  server->pcm_ref_count -= 1;
}

GslModule*
bse_server_retrive_midi_input_module (BseServer   *server,
				      const gchar *downlink_name,
				      guint        midi_channel_id,
				      guint        nth_note,
				      guint        signals[4])
{
  GslTrans *trans;
  GslModule *module;
  GSList *slist;

  g_return_val_if_fail (BSE_IS_SERVER (server), NULL);
  g_return_val_if_fail (downlink_name != NULL, NULL);
  g_return_val_if_fail (midi_channel_id - 1 < BSE_MIDI_MAX_CHANNELS, NULL);
  g_return_val_if_fail (signals != NULL, NULL);
  g_assert (4 == BSE_MIDI_MODULE_N_CHANNELS);
  
  server->midi_ref_count += 1;
  for (slist = server->midi_modules; slist; slist = slist->next)
    if (bse_midi_module_matches (slist->data, midi_channel_id, nth_note, signals))
      return slist->data;

  trans = gsl_trans_open ();
  module = bse_midi_module_insert (server->midi_decoder, midi_channel_id, nth_note, signals, trans);
  gsl_trans_commit (trans);
  server->midi_modules = g_slist_prepend (server->midi_modules, module);

  return module;
}

void
bse_server_discard_midi_input_module (BseServer   *server,
				      GslModule *module)
{
  g_return_if_fail (BSE_IS_SERVER (server));
  g_return_if_fail (module != NULL);
  g_return_if_fail (server->midi_ref_count > 0);
  g_return_if_fail (g_slist_find (server->midi_modules, module));	/* paranoid */

  server->midi_ref_count -= 1;
}

gboolean
bse_server_script_status (BseServer      *server,
			  BseScriptStatus status,
			  const gchar    *script_name,
			  gfloat          progress,
			  BseErrorType    error)
{
  gboolean abort = FALSE;

  g_return_val_if_fail (BSE_IS_SERVER (server), TRUE);
  g_return_val_if_fail (script_name != NULL, TRUE);

  g_signal_emit (server, signal_script_status, 0,
		 status, script_name, progress, error, &abort);
  return abort;
}

void
bse_server_add_io_watch (BseServer      *server,
			 gint            fd,
			 GIOCondition    events,
			 BseIOWatch      watch_func,
			 gpointer        data)
{
  g_return_if_fail (BSE_IS_SERVER (server));
  g_return_if_fail (watch_func != NULL);
  g_return_if_fail (fd >= 0);

  iowatch_add (server, fd, events, watch_func, data);
}

void
bse_server_remove_io_watch (BseServer *server,
			    BseIOWatch watch_func,
			    gpointer   data)
{
  g_return_if_fail (BSE_IS_SERVER (server));
  g_return_if_fail (watch_func != NULL);

  if (!iowatch_remove (server, watch_func, data))
    g_warning (G_STRLOC ": no such io watch installed %p(%p)", watch_func, data);
}


/* --- G_IO_* <-> GSL_POLL* --- */
static guint gio2gslpoll = 2; /* three state: 0:equalvalues, 1:translatevalues, 2:queryvalues */

static inline guint
gio_from_gslpoll (guint gslpoll)
{
  do
    {
      if (gio2gslpoll == 0)
	return gslpoll;
      else if (gio2gslpoll == 1)
	{
	  guint gio = (gslpoll & GSL_POLLIN) ? G_IO_IN : 0;
	  gio |= (gslpoll & GSL_POLLPRI) ? G_IO_PRI : 0;
	  gio |= (gslpoll & GSL_POLLOUT) ? G_IO_OUT : 0;
	  gio |= (gslpoll & GSL_POLLERR) ? G_IO_ERR : 0;
	  gio |= (gslpoll & GSL_POLLHUP) ? G_IO_HUP : 0;
	  gio |= (gslpoll & GSL_POLLNVAL) ? G_IO_NVAL : 0;
	  return gio;
	}
      else /* compare */
	gio2gslpoll = (GSL_POLLIN == G_IO_IN && GSL_POLLPRI == G_IO_PRI &&
		       GSL_POLLOUT == G_IO_OUT && GSL_POLLERR == G_IO_ERR &&
		       GSL_POLLHUP == G_IO_HUP && GSL_POLLNVAL == G_IO_NVAL);
    }
  while (TRUE);
}

static inline guint
gslpoll_from_gio (guint gio)
{
  do
    {
      if (gio2gslpoll == 0)
	return gio;
      else if (gio2gslpoll == 1)
	{
	  guint gslpoll = (gio & G_IO_IN) ? GSL_POLLIN : 0;
	  gslpoll |= (gio & G_IO_PRI) ? GSL_POLLPRI : 0;
	  gslpoll |= (gio & G_IO_OUT) ? GSL_POLLOUT : 0;
	  gslpoll |= (gio & G_IO_ERR) ? GSL_POLLERR : 0;
	  gslpoll |= (gio & G_IO_HUP) ? GSL_POLLHUP : 0;
	  gslpoll |= (gio & G_IO_NVAL) ? GSL_POLLNVAL : 0;
	  return gslpoll;
	}
      else /* compare */
	gio2gslpoll = (GSL_POLLIN == G_IO_IN && GSL_POLLPRI == G_IO_PRI &&
		       GSL_POLLOUT == G_IO_OUT && GSL_POLLERR == G_IO_ERR &&
		       GSL_POLLHUP == G_IO_HUP && GSL_POLLNVAL == G_IO_NVAL);
    }
  while (TRUE);
}


/* --- GPollFD IO watch source --- */
typedef struct {
  GSource    source;
  GPollFD    pfd;
  BseIOWatch watch_func;
  gpointer   data;
} WSource;

static gboolean
iowatch_prepare (GSource *source,
		 gint    *timeout_p)
{
  /* WSource *wsource = (WSource*) source; */
  gboolean need_dispatch;
  
  /* BSE_THREADS_ENTER (); */
  need_dispatch = FALSE;
  /* BSE_THREADS_LEAVE (); */

  return need_dispatch;
}

static gboolean
iowatch_check (GSource *source)
{
  WSource *wsource = (WSource*) source;
  guint need_dispatch;

  /* BSE_THREADS_ENTER (); */
  need_dispatch = wsource->pfd.events & wsource->pfd.revents;
  /* BSE_THREADS_LEAVE (); */

  return need_dispatch > 0;
}

static gboolean
iowatch_dispatch (GSource    *source,
		  GSourceFunc callback,
		  gpointer    user_data)
{
  WSource *wsource = (WSource*) source;

  BSE_THREADS_ENTER ();
  wsource->watch_func (wsource->data, &wsource->pfd);
  BSE_THREADS_LEAVE ();

  return TRUE;
}

static void
iowatch_add (BseServer   *server,
	     gint         fd,
	     GIOCondition events,
	     BseIOWatch   watch_func,
	     gpointer     data)
{
  static GSourceFuncs iowatch_gsource_funcs = {
    iowatch_prepare,
    iowatch_check,
    iowatch_dispatch,
    NULL
  };
  GSource *source = g_source_new (&iowatch_gsource_funcs, sizeof (WSource));
  WSource *wsource = (WSource*) source;

  server->watch_list = g_slist_prepend (server->watch_list, wsource);
  wsource->pfd.fd = fd;
  wsource->pfd.events = events;
  wsource->watch_func = watch_func;
  wsource->data = data;
  g_source_set_priority (source, G_PRIORITY_HIGH);
  g_source_add_poll (source, &wsource->pfd);
  g_source_attach (source, g_main_context_default ());
}

static gboolean
iowatch_remove (BseServer *server,
		BseIOWatch watch_func,
		gpointer   data)
{
  GSList *slist;

  for (slist = server->watch_list; slist; slist = slist->next)
    {
      WSource *wsource = slist->data;

      if (wsource->watch_func == watch_func && wsource->data == data)
	{
	  g_source_destroy (&wsource->source);
	  server->watch_list = g_slist_remove (server->watch_list, wsource);
	  return TRUE;
	}
    }
  return FALSE;
}


/* --- GSL engine main loop --- */
typedef struct {
  GSource       source;
  guint         n_fds;
  GPollFD       fds[GSL_ENGINE_MAX_POLLFDS];
  GslEngineLoop loop;
} PSource;

static gboolean
engine_prepare (GSource *source,
		gint    *timeout_p)
{
  PSource *psource = (PSource*) source;
  gboolean need_dispatch;
  
  BSE_THREADS_ENTER ();
  need_dispatch = gsl_engine_prepare (&psource->loop);
  if (psource->loop.fds_changed)
    {
      guint i;

      for (i = 0; i < psource->n_fds; i++)
	g_source_remove_poll (source, psource->fds + i);
      psource->n_fds = psource->loop.n_fds;
      for (i = 0; i < psource->n_fds; i++)
	{
	  GPollFD *pfd = psource->fds + i;

	  pfd->fd = psource->loop.fds[i].fd;
	  pfd->events = gio_from_gslpoll (psource->loop.fds[i].events);
	  g_source_add_poll (source, pfd);
	}
    }
  *timeout_p = psource->loop.timeout;
  BSE_THREADS_LEAVE ();

  return need_dispatch;
}

static gboolean
engine_check (GSource *source)
{
  PSource *psource = (PSource*) source;
  gboolean need_dispatch;
  guint i;

  BSE_THREADS_ENTER ();
  for (i = 0; i < psource->n_fds; i++)
    psource->loop.fds[i].revents = gslpoll_from_gio (psource->fds[i].revents);
  psource->loop.revents_filled = TRUE;
  need_dispatch = gsl_engine_check (&psource->loop);
  BSE_THREADS_LEAVE ();

  return need_dispatch;
}

static gboolean
engine_dispatch (GSource    *source,
		 GSourceFunc callback,
		 gpointer    user_data)
{
  BSE_THREADS_ENTER ();
  gsl_engine_dispatch ();
  BSE_THREADS_LEAVE ();

  return TRUE;
}

static void
engine_init (BseServer *server,
	     gfloat	mix_freq)
{
  static GSourceFuncs engine_gsource_funcs = {
    engine_prepare,
    engine_check,
    engine_dispatch,
    NULL
  };
  static gboolean engine_is_initialized = FALSE;

  g_return_if_fail (server->engine_source == NULL);
  g_assert (sizeof (GslPollFD) == sizeof (GPollFD));

  bse_globals_lock ();		// FIXME: globals mix_freq
  server->engine_source = g_source_new (&engine_gsource_funcs, sizeof (PSource));
  g_source_set_priority (server->engine_source, G_PRIORITY_DEFAULT); 	// FIXME: prio settings

  if (!engine_is_initialized)	// FIXME: hack because we can't deinitialize the engine
    {
      engine_is_initialized = TRUE;
      gsl_engine_init (1, BSE_BLOCK_N_VALUES, mix_freq, 63);
    }
  else
    g_assert (mix_freq == gsl_engine_sample_freq () && BSE_BLOCK_N_VALUES == gsl_engine_block_size ());

  g_source_attach (server->engine_source, g_main_context_default ());
}

static void
engine_shutdown (BseServer *server)
{
  g_return_if_fail (server->engine_source != NULL);

  // FIXME: need to be able to completely unintialize engine here
  g_source_destroy (server->engine_source);
  server->engine_source = NULL;
  bse_globals_unlock ();
}
