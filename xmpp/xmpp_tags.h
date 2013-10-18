#ifndef XMPP_TAGS_H
#define XMPP_TAGS_H

#include "xml_common.h"

typedef enum
{
  XMPP_UNKNOWN,
  
  /* stream-level */
  XMPP_STREAM,
  XMPP_STREAM_ERROR,
  XMPP_STREAM_FEATURES,

  /* stanzas */
  XMPP_MESSAGE,
  XMPP_IQ,
  XMPP_PRESENCE
} XMPP_TAG;

XMPP_TAG xmpp_gettag(const XMLNode *node);

#endif

