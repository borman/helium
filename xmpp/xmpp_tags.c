#include "xmpp_tags.h"

typedef struct
{
  const char *tag;
  XMPP_TAG code;
} XMPP_TAG_ENTRY;

static const XMPP_TAG_ENTRY xmpp_tags[] =
{
  {"message", XMPP_MESSAGE},
  {"iq", XMPP_IQ},
  {"presence", XMPP_PRESENCE},

  {"stream:stream", XMPP_STREAM},
  {"stream:error", XMPP_STREAM_ERROR},
  {"stream:features", XMPP_STREAM_FEATURES}
};
static const unsigned int n_tags =  sizeof(xmpp_tags)/sizeof(XMPP_TAG_ENTRY);

XMPP_TAG xmpp_gettag(const XMLNode *node)
{
  if (node->name==NULL)
    return XMPP_UNKNOWN;

  for (int i=0; i<n_tags; i++)
   if (!strcmp(node->name->d, xmpp_tags[i].tag))
     return xmpp_tags[i].code;

  return XMPP_UNKNOWN;
}

