// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#include "gxkled.hh"
#include "sfi/private.hh"
#include <math.h>

#define	FUDGE	3


/* --- prototypes --- */
static void	gxk_led_class_init		(GxkLedClass	*klass);
static void	gxk_led_init			(GxkLed		*self);
static void	gxk_led_finalize		(GObject	*object);
static void	gxk_led_size_request		(GtkWidget	*widget,
						 GtkRequisition	*requisition);
static void     gxk_led_size_allocate		(GtkWidget	*widget,
						 GtkAllocation	*allocation);
static void	gxk_led_state_changed		(GtkWidget	*widget,
						 GtkStateType	 previous_state);
static void	gxk_led_style_set		(GtkWidget	*widget,
						 GtkStyle	*previous_style);
static void	gxk_led_resize			(GxkLed		*self);
static void	gxk_led_rerender		(GxkLed		*self);
static gboolean gxk_led_expose			(GtkWidget      *widget,
						 GdkEventExpose *event);


/* --- variables --- */
static gpointer parent_class = NULL;


/* --- functions --- */
GType
gxk_led_get_type (void)
{
  static GType type = 0;
  if (!type)
    {
      static const GTypeInfo type_info = {
	sizeof (GxkLedClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gxk_led_class_init,
	NULL,   /* class_finalize */
	NULL,   /* class_data */
	sizeof (GxkLed),
	0,      /* n_preallocs */
	(GInstanceInitFunc) gxk_led_init,
      };

      type = g_type_register_static (GTK_TYPE_WIDGET,
				     "GxkLed",
				     &type_info, GTypeFlags (0));
    }
  return type;
}

static void
gxk_led_class_init (GxkLedClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gxk_led_finalize;

  widget_class->size_request = gxk_led_size_request;
  widget_class->size_allocate = gxk_led_size_allocate;
  widget_class->state_changed = gxk_led_state_changed;
  widget_class->style_set = gxk_led_style_set;
  widget_class->expose_event = gxk_led_expose;
}

static void
gxk_led_init (GxkLed *self)
{
  GTK_WIDGET_SET_FLAGS (self, GTK_NO_WINDOW);
  self->color = GXK_LED_OFF;
  self->radius = 1;
  gtk_widget_show (GTK_WIDGET (self));
  gxk_led_resize (self);
}

static void
gxk_led_finalize (GObject *object)
{
  GxkLed *self = GXK_LED (object);

  if (self->pixbuf)
    g_object_unref (self->pixbuf);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * @param color	rgb color
 *
 * Create a new led widget. It's active color can be specified
 * as 24bit RGB @a color value.
 */
gpointer
gxk_led_new (guint color)
{
  GxkLed *self = (GxkLed*) g_object_new (GXK_TYPE_LED, NULL);
  gxk_led_set_color (self, color);
  return self;
}

/**
 * @param self	valid GxkLed
 * @param color	color as RGB byte triple
 *
 * Set the current led color as 24bit RGB values.
 */
void
gxk_led_set_color (GxkLed *self,
		   guint   color)
{
  assert_return (GXK_IS_LED (self));

  if (!color)	/* lazy bum! */
    color = GXK_LED_OFF;
  self->color = color;
  if (self->pixbuf)
    gxk_led_rerender (self);
}

void
gxk_led_set_border_width (GxkLed *self,
			  guint   border_width)
{
  assert_return (GXK_IS_LED (self));

  self->border_width = border_width;
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
gxk_led_state_changed (GtkWidget   *widget,
		       GtkStateType previous_state)
{
  GxkLed *self = GXK_LED (widget);
  if (GTK_WIDGET_CLASS (parent_class)->state_changed)
    GTK_WIDGET_CLASS (parent_class)->state_changed (widget, previous_state);
  gxk_led_rerender (self);
}

static void
gxk_led_style_set (GtkWidget *widget,
		   GtkStyle  *previous_style)
{
  GxkLed *self = GXK_LED (widget);
  if (GTK_WIDGET_CLASS (parent_class)->style_set)
    GTK_WIDGET_CLASS (parent_class)->style_set (widget, previous_style);
  gxk_led_rerender (self);
}

static void
gxk_led_size_request (GtkWidget      *widget,
		      GtkRequisition *requisition)
{
  GxkLed *self = GXK_LED (widget);
  GdkPixbuf *pixbuf = self->pixbuf;

  requisition->width = 0;
  requisition->height = 0;
  if (pixbuf)
    {
      requisition->width += gdk_pixbuf_get_width (pixbuf);
      requisition->height += gdk_pixbuf_get_height (pixbuf);
    }
}

static void
gxk_led_resize (GxkLed *self)
{
  /* GtkWidget *widget = GTK_WIDGET (self); */

  if (self->pixbuf)
    g_object_unref (self->pixbuf);
  self->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
				 2 * self->radius,
				 2 * self->radius);
  gxk_led_rerender (self);
}

static void
gxk_led_size_allocate (GtkWidget     *widget,
		       GtkAllocation *allocation)
{
  GxkLed *self = GXK_LED (widget);
  GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);
  self->radius = MIN (widget->allocation.width, widget->allocation.height) / 2;
  self->radius = MAX (self->radius, 3 + self->border_width) - 2 - self->border_width;
  gxk_led_resize (self);
}

static void
gxk_led_rerender (GxkLed *self)
{
  GdkPixbuf *pixbuf = self->pixbuf;
  gint width = gdk_pixbuf_get_width (pixbuf);
  gint rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  gint height = gdk_pixbuf_get_height (pixbuf);
  guint8 *pixels = gdk_pixbuf_get_pixels (pixbuf);
  guint8 r = self->color >> 16;
  guint8 g = self->color >> 8;
  guint8 b = self->color;
  gint x, y;
  gdouble xc, yc, radius = self->radius;
  gdouble lx, ly, lr;

  /* center */
  xc = width / 2;
  yc = height / 2;
  /* light source */
  ly = lx = xc - 0.1 * radius;
  lr = 10.5 * radius;
  /* fill buffer */
  for (x = 0; x < width; x++)
    for (y = 0; y < height; y++)
      {
	gdouble dist = sqrt ((x - xc) * (x - xc) + (y - yc) * (y - yc));
	gdouble ldist = sqrt ((x - lx) * (x - lx) + (y - ly) * (y - ly));
	gdouble la, a = dist < radius ? 1 : 0;
	la = 1 - ldist / lr;
	pixels[y * rowstride + x * 4 + 0] = la * r;
	pixels[y * rowstride + x * 4 + 1] = la * g;
	pixels[y * rowstride + x * 4 + 2] = la * b;
	pixels[y * rowstride + x * 4 + 3] = a * 0xff;
      }

  if (!GTK_WIDGET_IS_SENSITIVE (self))
    gdk_pixbuf_saturate_and_pixelate (pixbuf, pixbuf, 0.8, TRUE);

  gtk_widget_queue_draw (GTK_WIDGET (self));
}

static gboolean
gxk_led_expose (GtkWidget      *widget,
		GdkEventExpose *event)
{
  GxkLed *self = GXK_LED (widget);
  GdkRectangle rect, area = event->area;
  GdkPixbuf *pixbuf = self->pixbuf;

  if (0)
    gdk_draw_rectangle (widget->window,
			widget->style->black_gc,
			TRUE,
			widget->allocation.x,
			widget->allocation.y,
			widget->allocation.width,
			widget->allocation.height);

  if (pixbuf && gdk_rectangle_intersect (&area, &widget->allocation, &area))
    {
      gdouble radius = self->radius;
      gint x, y, xc, yc;
      rect.width = gdk_pixbuf_get_width (pixbuf);
      rect.height = gdk_pixbuf_get_height (pixbuf);
      x = widget->allocation.x + (widget->allocation.width - rect.width) / 2;
      y = widget->allocation.y + (widget->allocation.height - rect.height) / 2;
      rect.x = x;
      rect.y = y;
      xc = rect.x + rect.width / 2;
      yc = rect.y + rect.height / 2;
      /* paint led */
      if (gdk_rectangle_intersect (&rect, &area, &rect))
	gdk_pixbuf_render_to_drawable (pixbuf,
				       widget->window,
				       widget->style->black_gc,
				       rect.x - x, rect.y - y,
				       rect.x, rect.y,
				       rect.width, rect.height,
				       GDK_RGB_DITHER_MAX, 0, 0);
      /* draw dark shade on the left */
      gdk_draw_arc (widget->window,
		    widget->style->dark_gc[widget->state],
		    FALSE,
		    xc - radius, yc - radius - 1,
		    2 * radius, 2 * radius,
		    45. * 64, 90. * 64);
      gdk_draw_arc (widget->window,
		    widget->style->dark_gc[widget->state],
		    FALSE,
		    xc - radius - 1, yc - radius,
		    2 * radius, 2 * radius,
		    135. * 64, 90. * 64);
      gdk_draw_arc (widget->window,
		    widget->style->black_gc,
		    FALSE,
		    xc - radius, yc - radius,
		    2 * radius, 2 * radius,
		    45. * 64, 180. * 64);
      /* draw light shade on the right */
      gdk_draw_arc (widget->window,
		    widget->style->bg_gc[widget->state],
		    FALSE,
		    xc - radius, yc - radius,
		    2 * radius, 2 * radius,
		    45. * 64, -180. * 64);
      gdk_draw_arc (widget->window,
		    widget->style->light_gc[widget->state],
		    FALSE,
		    xc - radius + 1, yc - radius,
		    2 * radius, 2 * radius,
		    45. * 64, -90. * 64);
      gdk_draw_arc (widget->window,
		    widget->style->light_gc[widget->state],
		    FALSE,
		    xc - radius, yc - radius + 1,
		    2 * radius, 2 * radius,
		    -45. * 64, -90. * 64);
    }
  return FALSE;
}
