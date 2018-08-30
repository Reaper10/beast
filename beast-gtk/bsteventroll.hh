// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#ifndef __BST_EVENT_ROLL_H__
#define __BST_EVENT_ROLL_H__

#include	"bstsegment.hh"

/* --- type macros --- */
#define BST_TYPE_EVENT_ROLL              (bst_event_roll_get_type ())
#define BST_EVENT_ROLL(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), BST_TYPE_EVENT_ROLL, BstEventRoll))
#define BST_EVENT_ROLL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), BST_TYPE_EVENT_ROLL, BstEventRollClass))
#define BST_IS_EVENT_ROLL(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), BST_TYPE_EVENT_ROLL))
#define BST_IS_EVENT_ROLL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), BST_TYPE_EVENT_ROLL))
#define BST_EVENT_ROLL_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), BST_TYPE_EVENT_ROLL, BstEventRollClass))


/* --- typedefs & enums --- */
typedef struct _BstEventRoll        BstEventRoll;
typedef struct _BstEventRollClass   BstEventRollClass;


/* --- structures & typedefs --- */
struct BstEventRollDrag : GxkScrollCanvasDrag {
  int           tick_width = 0;
  uint	        start_tick = 0;
  float         start_value = 0;
  bool		start_valid = false;
  bool          current_valid = false;          // value out of range
  uint          current_tick = 0;
  float         current_value = 0;              // between -1 and +1 if valid
  float         current_value_raw = 0;
  // convenience:
  BstEventRoll *eroll = NULL;
};
struct _BstEventRoll
{
  GxkScrollCanvas parent_instance;

  Bse::PartS      part;
  Bse::MidiSignal control_type;
  GtkWidget      *child;

  /* horizontal layout */
  guint		 ppqn;		/* parts per quarter note */
  guint		 qnpt;		/* quarter notes per tact */
  guint		 max_ticks;	/* in ticks */
  gfloat	 hzoom;
  guint		 draw_qn_grid : 1;
  guint		 draw_qqn_grid : 1;

  /* drag data */
  guint		start_valid : 1;
  guint	        start_tick;
  gfloat        start_value;

  /* vpanel width sync */
  gint         (*fetch_vpanel_width) (gpointer data);
  gpointer       fetch_vpanel_width_data;

  /* line drawing */
  BstSegment     segment;

  /* selection rectangle */
  guint		 selection_tick;
  guint		 selection_duration;
  gint		 selection_min_note;
  gint		 selection_max_note;
};
struct _BstEventRollClass
{
  GxkScrollCanvasClass parent_class;

  void		(*canvas_drag)			(BstEventRoll	  *self,
						 BstEventRollDrag *drag);
  void		(*canvas_clicked)		(BstEventRoll	  *eroll,
						 guint		   button,
						 guint		   tick_position,
						 gfloat            value,
						 GdkEvent	  *event);
  void		(*vpanel_drag)			(BstEventRoll	  *self,
						 BstEventRollDrag *drag);
  void		(*vpanel_clicked)		(BstEventRoll	  *eroll,
						 guint		   button,
						 gfloat            value,
						 GdkEvent	  *event);
};


/* --- prototypes --- */
GType       bst_event_roll_get_type              (void);
void        bst_event_roll_set_part              (BstEventRoll *self, Bse::PartH part = Bse::PartH());
gfloat      bst_event_roll_set_hzoom             (BstEventRoll   *self,
                                                  gfloat          hzoom);
void        bst_event_roll_set_view_selection    (BstEventRoll   *self,
                                                  guint           tick,
                                                  guint           duration);
void        bst_event_roll_set_vpanel_width_hook (BstEventRoll   *self,
                                                  gint          (*fetch_vpanel_width) (gpointer data),
                                                  gpointer        data);
void        bst_event_roll_set_control_type      (BstEventRoll *self, Bse::MidiSignal control_type);
void        bst_event_roll_init_segment          (BstEventRoll   *self,
                                                  BstSegmentType  type);
void        bst_event_roll_segment_start         (BstEventRoll   *self,
                                                  guint           tick,
                                                  gfloat          value);
void        bst_event_roll_segment_move_to       (BstEventRoll   *self,
                                                  guint           tick,
                                                  gfloat          value);
void        bst_event_roll_segment_tick_range    (BstEventRoll   *self,
                                                  guint          *tick,
                                                  guint          *duration);
gdouble     bst_event_roll_segment_value         (BstEventRoll   *self,
                                                  guint           tick);
void        bst_event_roll_clear_segment         (BstEventRoll   *self);

#endif /* __BST_EVENT_ROLL_H__ */
