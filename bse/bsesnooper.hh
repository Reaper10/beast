// Licensed GNU LGPL v2.1 or later: http://www.gnu.org/licenses/lgpl.html
#ifndef __BSE_SNOOPER_H__
#define __BSE_SNOOPER_H__

#include <bse/bsesource.hh>

/* --- object type macros --- */
#define BSE_TYPE_SNOOPER              (BSE_TYPE_ID (BseSnooper))
#define BSE_SNOOPER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), BSE_TYPE_SNOOPER, BseSnooper))
#define BSE_SNOOPER_CLASS(class)      (G_TYPE_CHECK_CLASS_CAST ((class), BSE_TYPE_SNOOPER, BseSnooperClass))
#define BSE_IS_SNOOPER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), BSE_TYPE_SNOOPER))
#define BSE_IS_SNOOPER_CLASS(class)   (G_TYPE_CHECK_CLASS_TYPE ((class), BSE_TYPE_SNOOPER))
#define BSE_SNOOPER_GET_CLASS(object) (G_TYPE_INSTANCE_GET_CLASS ((object), BSE_TYPE_SNOOPER, BseSnooperClass))
/* --- BseSnooper source --- */

struct BseSnooper : BseSource {
  BseModule *module;
};

struct BseSnooperClass : BseSourceClass
{};

enum
{
  BSE_SNOOPER_JCHANNEL_MONO,
  BSE_SNOOPER_N_JCHANNELS
};

#endif /* __BSE_SNOOPER_H__ */
