// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include "gxkpolygon.hh"
#include "sfi/private.hh"
#include <math.h>

#define	SQR(x)		((x) * (x))
#define	PI		3.1415926535897932384626433832795029


/* --- prototypes --- */
static void	gxk_polygon_class_init		(GxkPolygonClass *klass);
static void	gxk_polygon_init		(GxkPolygon	 *self);
static void	gxk_polygon_finalize		(GObject	 *object);
static void	gxk_polygon_size_request	(GtkWidget	 *widget,
						 GtkRequisition	 *requisition);
static gboolean gxk_polygon_expose		(GtkWidget       *widget,
						 GdkEventExpose  *event);


/* --- variables --- */
static gpointer parent_class = NULL;


/* --- functions --- */
GType
gxk_polygon_get_type (void)
{
  static GType type = 0;
  if (!type)
    {
      static const GTypeInfo type_info = {
	sizeof (GxkPolygonClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gxk_polygon_class_init,
	NULL,   /* class_finalize */
	NULL,   /* class_data */
	sizeof (GxkPolygon),
	0,      /* n_preallocs */
	(GInstanceInitFunc) gxk_polygon_init,
      };

      type = g_type_register_static (GTK_TYPE_WIDGET,
				     "GxkPolygon",
				     &type_info, GTypeFlags (0));
    }
  return type;
}

static void
gxk_polygon_class_init (GxkPolygonClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gxk_polygon_finalize;

  widget_class->size_request = gxk_polygon_size_request;
  widget_class->expose_event = gxk_polygon_expose;
}

static void
gxk_polygon_init (GxkPolygon *self)
{
  GTK_WIDGET_SET_FLAGS (self, GTK_NO_WINDOW);
  self->n_lines = 0;
  self->lines = NULL;
  self->n_arcs = 0;
  self->arcs = NULL;
  gtk_widget_show (GTK_WIDGET (self));
}

static void
gxk_polygon_finalize (GObject *object)
{
  GxkPolygon *self = GXK_POLYGON (object);

  g_free (self->lines);
  g_free (self->arcs);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * @param polygon_graph	set of lines and arcs
 *
 * Create a new polygon widget.
 */
gpointer
gxk_polygon_new (GxkPolygonGraph *polygon_graph)
{
  GxkPolygon *self = (GxkPolygon*) g_object_new (GXK_TYPE_POLYGON, NULL);
  gxk_polygon_set_graph (self, polygon_graph);
  return self;
}

/**
 * @param self	valid GxkPolygon
 * @param n_lines	number of lines
 * @param lines	array of lines
 *
 * Set the lines for this polygon. The direction
 * of a line determines it's shadow type.
 */
void
gxk_polygon_set_lines (GxkPolygon     *self,
		       guint           n_lines,
		       GxkPolygonLine *lines)
{
  assert_return (GXK_IS_POLYGON (self));
  if (n_lines)
    assert_return (lines != NULL);

  g_free (self->lines);
  self->n_lines = n_lines;
  self->lines = (GxkPolygonLine*) g_memdup (lines, sizeof (lines[0]) * n_lines);
  gtk_widget_queue_draw (GTK_WIDGET (self));
}

/**
 * @param self	valid GxkPolygon
 * @param n_arcs	number of arcs
 * @param arcs	array of arcs
 *
 * Set the arcs for this polygon. The direction
 * of an arc determines it's shadow type.
 */
void
gxk_polygon_set_arcs (GxkPolygon    *self,
		      guint          n_arcs,
		      GxkPolygonArc *arcs)
{
  assert_return (GXK_IS_POLYGON (self));
  if (n_arcs)
    assert_return (arcs != NULL);

  g_free (self->arcs);
  self->n_arcs = n_arcs;
  self->arcs = (GxkPolygonArc*) g_memdup (arcs, sizeof (arcs[0]) * n_arcs);
  gtk_widget_queue_draw (GTK_WIDGET (self));
}

/**
 * @param self	valid GxkPolygon
 * @param polygon_graph	set of lines and arcs
 *
 * Set the lines and arcs for this polygon, see
 * gxk_polygon_set_lines() and gxk_polygon_set_arcs().
 */
void
gxk_polygon_set_graph (GxkPolygon      *self,
		       GxkPolygonGraph *polygon_graph)
{
  assert_return (GXK_IS_POLYGON (self));
  if (polygon_graph)
    {
      gxk_polygon_set_lines (self, polygon_graph->n_lines, polygon_graph->lines);
      gxk_polygon_set_arcs (self, polygon_graph->n_arcs, polygon_graph->arcs);
      gxk_polygon_set_length (self, polygon_graph->length);
    }
  else
    {
      gxk_polygon_set_lines (self, 0, NULL);
      gxk_polygon_set_arcs (self, 0, NULL);
      /* gxk_polygon_set_length (self, 0); */
    }
}

/**
 * @param self	valid GxkPolygon
 * @param length	set of lines and arcs
 *
 * Set the desired width and height for this polygon
 * to @a length.
 */
void
gxk_polygon_set_length (GxkPolygon *self,
			guint       length)
{
  assert_return (GXK_IS_POLYGON (self));

  self->request_length = length;
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
gxk_polygon_size_request (GtkWidget      *widget,
			  GtkRequisition *requisition)
{
  GxkPolygon *self = GXK_POLYGON (widget);
  requisition->width = self->request_length;
  requisition->height = self->request_length;
}

/* draw line styles:
 * _ETCHED_IN:                   _ETCHED_OUT:
 * #######.  3) # = Dark         +++++++X  3) + = Light
 * #+++++X.  1) + = Light        +#####.X  1) # = Dark
 * #+    X.  2) X = Dark         +#    .X  2) . = Light
 * #XXXXXX.  4) . = Light        +......X  4) X = Dark
 * ........                      XXXXXXXX
 * _IN:                          _OUT:
 * +++++++X  3) + = Dark         XXXXXXX#  3) X = Light
 * +#####.X  1) # = Black        X.....+#  1) . = Background
 * +#    .X  2) . = Background   X.    +#  2) + = Dark
 * +......X  4) X = Light        X++++++#  4) # = Black
 * XXXXXXXX                      ########
 */
static void
init_gcs (GxkPolygon   *self,
	  GtkShadowType draw_type,
	  GdkGC       **gc1,
	  GdkGC       **gc2,
	  GdkGC       **gc3,
	  GdkGC       **gc4)
{
  GtkWidget *widget = GTK_WIDGET (self);
  GdkGC *black = widget->style->black_gc;
  GdkGC *dark = widget->style->dark_gc[widget->state];
  GdkGC *bg = widget->style->bg_gc[widget->state];
  GdkGC *light = widget->style->light_gc[widget->state];
  if (!GTK_WIDGET_IS_SENSITIVE (self))
    {
      *gc1 = bg;
      *gc2 = bg;
      *gc3 = dark;
      *gc4 = dark;
      return;
    }
  switch (draw_type)
    {
    default:
    case GTK_SHADOW_IN:
      *gc1 = black;
      *gc2 = bg;
      *gc3 = dark;
      *gc4 = light;
      break;
    case GTK_SHADOW_OUT:
      *gc1 = bg;
      *gc2 = dark;
      *gc3 = light;
      *gc4 = black;
      break;
    case GTK_SHADOW_ETCHED_IN:
      *gc1 = light;
      *gc2 = dark;
      *gc3 = dark;
      *gc4 = light;
      break;
    case GTK_SHADOW_ETCHED_OUT:
      *gc1 = dark;
      *gc2 = light;
      *gc3 = light;
      *gc4 = dark;
      break;
    }
}

static gboolean
gxk_polygon_expose (GtkWidget      *widget,
		    GdkEventExpose *event)
{
  GxkPolygon *self = GXK_POLYGON (widget);
  GdkRectangle area = event->area;
  GdkGC *gc1, *gc2, *gc3, *gc4, *bg_gc = widget->style->bg_gc[widget->state];
  gint width = CLAMP (widget->allocation.width, 2, widget->allocation.height);
  gint height = width;
  gint x = widget->allocation.x + (widget->allocation.width - width) / 2;
  gint y = widget->allocation.y + (widget->allocation.height - height) / 2;
  guint n, pass, draw_lines, draw_arcs;
  width -= 2;
  height -= 2;

  if (0)
    gdk_draw_rectangle (widget->window,
			widget->style->white_gc,
			TRUE,
			x, y, width, height);
  draw_lines = gdk_rectangle_intersect (&area, &widget->allocation, &area);
  draw_arcs = draw_lines;
  if (draw_lines)
    for (pass = 1; pass <= 4; pass++)
      for (n = 0; n < self->n_lines; n++)
	{
	  GxkPolygonLine p = self->lines[n];
	  gdouble angle = atan2 (p.y2 - p.y1, p.x2 - p.x1) / PI;
	  gint ix1, iy1, ix2, iy2, sx, sy, clockwise;
	  ix1 = 1 + x + p.x1 * width;
	  iy1 = 1 + y + height - p.y1 * height;
	  ix2 = 1 + x + p.x2 * width;
	  iy2 = 1 + y + height - p.y2 * height;
	  sx = ABS (p.y2 - p.y1) >= ABS (p.x2 - p.x1);
	  sy = ABS (p.y2 - p.y1) < ABS (p.x2 - p.x1);
	  clockwise = angle >= -0.25 && angle < 0.75;
	  init_gcs (self, p.draw_type, &gc1, &gc2, &gc3, &gc4);
	  switch (pass)
	    {
	    case 1:
	      if (clockwise)
		{
		  gdk_draw_line (widget->window, gc1,
				 ix1 + sx, iy1 + sy, ix2 + sx, iy2 + sy);
		  if (sx || sy)
		    {
		      gdk_draw_point (widget->window, bg_gc, ix1 + sx, iy1 + sy);
		      gdk_draw_point (widget->window, bg_gc, ix2 + sx, iy2 + sy);
		    }
		}
	      break;
	    case 2:
	      if (!clockwise)
		{
		  gdk_draw_line (widget->window, gc2,
				 ix1 - sx, iy1 - sy, ix2 - sx, iy2 - sy);
                  if (sx || sy)
		    {
		      gdk_draw_point (widget->window, bg_gc, ix1 - sx, iy1 - sy);
		      gdk_draw_point (widget->window, bg_gc, ix2 - sx, iy2 - sy);
		    }
		}
	      break;
	    case 3:
	      if (clockwise)
		gdk_draw_line (widget->window, gc3,
			       ix1, iy1, ix2, iy2);
	      break;
	    case 4:
	      if (!clockwise)
		gdk_draw_line (widget->window, gc4,
			       ix1, iy1, ix2, iy2);
	      break;
	    }
	}
  if (draw_arcs)
    for (pass = 2; pass <= 3; pass++)
      for (n = 0; n < self->n_arcs; n++)
	{
	  GdkGC *dark_gc = widget->style->dark_gc[widget->state];
	  GdkGC *light_gc = widget->style->light_gc[widget->state];
	  GdkGC *black_gc = widget->style->black_gc;
	  GdkGC *bg_gc = widget->style->bg_gc[widget->state];
	  GxkPolygonArc p = self->arcs[n];
	  gint ax = 1 + x + (p.xc - p.xr) * width;
          gint ay = 1 + y + (1.0 - p.yc - p.yr) * height;
	  gint aw = 1 + 2 * p.xr * width;
          gint ah = 1 + 2 * p.yr * height;
	  gdouble a1, a2, len, s = +1;
	  if (MAX (p.sa, p.ea) >= 360 && MIN (p.sa, p.ea) >= 0)
	    {
	      p.sa -= 360;
	      p.ea -= 360;
	    }
	  else if (MIN (p.sa, p.ea) <= -360 && MAX (p.sa, p.ea) <= 0)
	    {
	      p.sa += 360;
	      p.ea += 360;
	    }
	  p.sa = CLAMP (p.sa, -360, +360);
	  p.ea = CLAMP (p.ea, -360, +360);
	  gdk_draw_arc (widget->window, light_gc, FALSE,
			ax, ay, aw, ah,
			p.sa * 64, (p.ea - p.sa) * 64);
	  a1 = MIN (p.sa, p.ea);
	  a2 = MAX (p.sa, p.ea);
	  if (a1 < p.ea)
	    {
	      s = -1;
	      black_gc = light_gc;
	      dark_gc = bg_gc;
	      light_gc = widget->style->black_gc;
	      bg_gc = widget->style->dark_gc[widget->state];
	    }
	  if (!GTK_WIDGET_IS_SENSITIVE (self))
	    {
	      black_gc = widget->style->dark_gc[widget->state];
	      dark_gc = widget->style->bg_gc[widget->state];
	      light_gc = widget->style->dark_gc[widget->state];
	      bg_gc = widget->style->bg_gc[widget->state];
	    }
	  while (a1 < a2)
	    {
	      if (a1 >= -225 && a1 < -135)	/* left quarter */
		{
		  len = MIN (a2, -135) - a1;
		  if (pass < 3)
		    gdk_draw_arc (widget->window, dark_gc, FALSE,
				  ax - s, ay, aw, ah, .5+ a1 * 64, .5+ len * 64);
		  else
		    gdk_draw_arc (widget->window, black_gc, FALSE,
				  ax, ay, aw, ah, .5+ a1 * 64, .5+ len * 64);
		}
	      else if (a1 >= -135 && a1 < -45)	/* bottom quarter */
		{
		  len = MIN (a2, -45) - a1;
		  if (pass < 3)
		    gdk_draw_arc (widget->window, bg_gc, FALSE,
				  ax, ay + s, aw, ah, .5+ a1 * 64, .5+ len * 64);
		  else
		    gdk_draw_arc (widget->window, light_gc, FALSE,
				  ax, ay, aw, ah, .5+ a1 * 64, .5+ len * 64);
		}
	      else if (a1 >= -45 && a1 < +45)	/* right quarter */
		{
		  len = MIN (a2, +45) - a1;
                  if (pass < 3)
		    gdk_draw_arc (widget->window, bg_gc, FALSE,
				  ax + s, ay, aw, ah, .5+ a1 * 64, .5+ len * 64);
                  else
		    gdk_draw_arc (widget->window, light_gc, FALSE,
				  ax, ay, aw, ah, .5+ a1 * 64, .5+ len * 64);
		}
	      else if (a1 >= +45 && a1 < +135)	/* top quarter */
		{
		  len = MIN (a2, +135) - a1;
                  if (pass < 3)
		    gdk_draw_arc (widget->window, dark_gc, FALSE,
				  ax, ay - s, aw, ah, .5+ a1 * 64, .5+ len * 64);
                  else
		    gdk_draw_arc (widget->window, black_gc, FALSE,
				  ax, ay, aw, ah, .5+ a1 * 64, .5+ len * 64);
		}
	      else
		a2 += len = a1 > 0 ? -360 : 360;
	      a1 += len;
	    }
	}
  return FALSE;
}

#if 0 /* test polygons */
/* right turn */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 0.0, 0.9 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 0.0, 1.0 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 0.1, 1.0 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 0.9, 1.0 }; /* / */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 1.0, 1.0 }; /* / */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 1.0, 0.9 }; /* / */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 1.0, 0.1 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 1.0, 0.0 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 0.9, 0.0 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 0.1, 0.0 }; /* / */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 0.0, 0.0 }; /* / */
lines[i++] = (GxkPolygonLine) { 0.5, 0.5, 0.0, 0.1 }; /* / */
/* left turn */
lines[i++] = (GxkPolygonLine) { 0.0, 0.9, 0.5, 0.5 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.0, 1.0, 0.5, 0.5 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.1, 1.0, 0.5, 0.5 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.9, 1.0, 0.5, 0.5 }; /* / */
lines[i++] = (GxkPolygonLine) { 1.0, 1.0, 0.5, 0.5 }; /* / */
lines[i++] = (GxkPolygonLine) { 1.0, 0.9, 0.5, 0.5 }; /* / */
lines[i++] = (GxkPolygonLine) { 1.0, 0.1, 0.5, 0.5 }; /* \ */
lines[i++] = (GxkPolygonLine) { 1.0, 0.0, 0.5, 0.5 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.9, 0.0, 0.5, 0.5 }; /* \ */
lines[i++] = (GxkPolygonLine) { 0.1, 0.0, 0.5, 0.5 }; /* / */
lines[i++] = (GxkPolygonLine) { 0.0, 0.0, 0.5, 0.5 }; /* / */
lines[i++] = (GxkPolygonLine) { 0.0, 0.1, 0.5, 0.5 }; /* / */
#endif

#define	STOCK_SIZE	16

/* power */
static GxkPolygonLine power_lines[] = {
  { 0.44, 1.0, 0.44, 0.5 }, /* | */
  { 0.55, 0.5, 0.55, 1.0 }, /* | */
};
static GxkPolygonArc power_arcs[] = {
  { 0.5, 0.5, 0.45, 0.45, 45 + 90 - 360, 45 },     /* U */
};
GxkPolygonGraph gxk_polygon_power = { G_N_ELEMENTS (power_lines), power_lines,
				      G_N_ELEMENTS (power_arcs), power_arcs, STOCK_SIZE };
/* stop */
static GxkPolygonLine stop_lines[] = {
  { 0.25, 0.25, 0.25, 0.75, }, /* up */
  { 0.25, 0.75, 0.75, 0.75, }, /* right */
  { 0.75, 0.75, 0.75, 0.25, }, /* down */
  { 0.75, 0.25, 0.25, 0.25, }, /* left */
};
GxkPolygonGraph gxk_polygon_stop = { G_N_ELEMENTS (stop_lines), stop_lines,
				     0, NULL, STOCK_SIZE };
/* pause */
static GxkPolygonLine pause_lines[] = {
  { 0.2, 0.15, 0.2, 0.85 }, /* up */
  { 0.2, 0.85, 0.4, 0.85 }, /* right */
  { 0.4, 0.85, 0.4, 0.15 }, /* down */
  { 0.4, 0.15, 0.2, 0.15 }, /* left */
  { 0.6, 0.15, 0.6, 0.85 }, /* up */
  { 0.6, 0.85, 0.8, 0.85 }, /* right */
  { 0.8, 0.85, 0.8, 0.15 }, /* down */
  { 0.8, 0.15, 0.6, 0.15 }, /* left */
};
GxkPolygonGraph gxk_polygon_pause = { G_N_ELEMENTS (pause_lines), pause_lines,
				      0, NULL, STOCK_SIZE };
/* first */
static GxkPolygonLine first_lines[] = {
  { 1.000, 0.85, 1.000, 0.15 }, /* | */
  { 1.000, 0.15, 0.651, 0.50 }, /* \ */
  { 0.651, 0.50, 1.000, 0.85 }, /* / */
  { 0.550, 0.85, 0.550, 0.15 }, /* | */
  { 0.550, 0.15, 0.201, 0.50 }, /* \ */
  { 0.201, 0.50, 0.550, 0.85 }, /* / */
  { 0.000, 0.15, 0.000, 0.85 }, /* up */
  { 0.000, 0.85, 0.100, 0.85 }, /* right */
  { 0.100, 0.85, 0.100, 0.15 }, /* down */
  { 0.100, 0.15, 0.000, 0.15 }, /* left */
};
GxkPolygonGraph gxk_polygon_first = { G_N_ELEMENTS (first_lines), first_lines,
				      0, NULL, STOCK_SIZE };
/* previous */
static GxkPolygonLine previous_lines[] = {
  { 0.300, 0.15, 0.300, 0.85 }, /* up */
  { 0.300, 0.85, 0.400, 0.85 }, /* right */
  { 0.400, 0.85, 0.400, 0.15 }, /* down */
  { 0.400, 0.15, 0.300, 0.15 }, /* left */
  { 0.800, 0.85, 0.800, 0.15 }, /* | */
  { 0.800, 0.15, 0.451, 0.50 }, /* \ */
  { 0.451, 0.50, 0.800, 0.85 }, /* / */
};
GxkPolygonGraph gxk_polygon_previous = { G_N_ELEMENTS (previous_lines), previous_lines,
					 0, NULL, STOCK_SIZE };
/* rewind */
static GxkPolygonLine rewind_lines[] = {
  { 0.450, 0.85, 0.450, 0.15 }, /* | */
  { 0.450, 0.15, 0.101, 0.50 }, /* \ */
  { 0.101, 0.50, 0.450, 0.85 }, /* / */
  { 0.900, 0.85, 0.900, 0.15 }, /* | */
  { 0.900, 0.15, 0.551, 0.50 }, /* \ */
  { 0.551, 0.50, 0.900, 0.85 }, /* / */
};
GxkPolygonGraph gxk_polygon_rewind = { G_N_ELEMENTS (rewind_lines), rewind_lines,
				       0, NULL, STOCK_SIZE };
/* play */
static GxkPolygonLine play_lines[] = {
  { 0.2, 0.15, 0.2, 0.85 }, /* | */
  { 0.2, 0.85, 0.8, 0.50 }, /* \ */
  { 0.8, 0.50, 0.2, 0.15 }, /* / */
};
GxkPolygonGraph gxk_polygon_play = { G_N_ELEMENTS (play_lines), play_lines,
				     0, NULL, STOCK_SIZE };
/* forward */
static GxkPolygonLine forward_lines[] = {
  { 0.550, 0.15, 0.550, 0.85 }, /* | */
  { 0.550, 0.85, 0.901, 0.50 }, /* \ */
  { 0.901, 0.50, 0.550, 0.15 }, /* / */
  { 0.100, 0.15, 0.100, 0.85 }, /* | */
  { 0.100, 0.85, 0.451, 0.50 }, /* \ */
  { 0.451, 0.50, 0.100, 0.15 }, /* / */
};
GxkPolygonGraph gxk_polygon_forward = { G_N_ELEMENTS (forward_lines), forward_lines,
					0, NULL, STOCK_SIZE };
/* next */
static GxkPolygonLine next_lines[] = {
  { 0.200, 0.15, 0.200, 0.85 }, /* | */
  { 0.200, 0.85, 0.551, 0.50 }, /* \ */
  { 0.551, 0.50, 0.200, 0.15 }, /* / */
  { 0.600, 0.15, 0.600, 0.85 }, /* up */
  { 0.600, 0.85, 0.700, 0.85 }, /* right */
  { 0.700, 0.85, 0.700, 0.15 }, /* down */
  { 0.700, 0.15, 0.600, 0.15 }, /* left */
};
GxkPolygonGraph gxk_polygon_next = { G_N_ELEMENTS (next_lines), next_lines,
				     0, NULL, STOCK_SIZE };
/* last */
static GxkPolygonLine last_lines[] = {
  { 0.000, 0.15, 0.000, 0.85 }, /* | */
  { 0.000, 0.85, 0.351, 0.50 }, /* \ */
  { 0.351, 0.50, 0.000, 0.15 }, /* / */
  { 0.450, 0.15, 0.450, 0.85 }, /* | */
  { 0.450, 0.85, 0.801, 0.50 }, /* \ */
  { 0.801, 0.50, 0.450, 0.15 }, /* / */
  { 0.900, 0.15, 0.900, 0.85 }, /* up */
  { 0.900, 0.85, 1.000, 0.85 }, /* right */
  { 1.000, 0.85, 1.000, 0.15 }, /* down */
  { 1.000, 0.15, 0.900, 0.15 }, /* left */
};
GxkPolygonGraph gxk_polygon_last = { G_N_ELEMENTS (last_lines), last_lines,
				     0, NULL, STOCK_SIZE };
