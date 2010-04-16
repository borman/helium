/*
 * XML unit tester
 *
 * Reads xml input from file and prints its tree
 */

#include "xml/xml_parser.h"
#include "xml/xml_memory.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>


static void dump_xmltree(XMLNode *node, int level)
{
  char indent[40];
  for (int i=0; i<level*2; i++)
    indent[i] = ' ';
  indent[level] = '\x00';

  if (node==NULL)
    return;

  char *name = node->name->d;
  if (name==NULL)
    name = "<NULL>";

  printf("%s%s {\n", indent, name);

  XMLAttr *attr = node->attr;
  while (attr)
  {
    printf("%s \"%s\" = \"%s\"\n", indent, attr->name->d, attr->value->d);
    attr = attr->next;
  }

  if (node->text)
    printf("%s[[%s]]\n", indent, node->text->d);

  dump_xmltree(node->first_child, level+1);

  printf("%s}\n", indent);

  dump_xmltree(node->next_sibling, level);
}

static void stanza_callback(XMLNode *stanza)
{
  printf("--> Stanza:\n");
  dump_xmltree(stanza, 2);
  XML_DestroyTree(stanza);
}

static void stream_begin_callback(XMLNode *stream)
{
  printf("--> stream begin:\n");
  dump_xmltree(stream, 2);
}

static void stream_end_callback(XMLNode *stream)
{
  printf("--> stream end:\n");
  dump_xmltree(stream, 2);
  XML_DestroyTree(stream);
}

#define BUFSIZE 8192

char netbuf[BUFSIZE];

#define SERVER_PORT 5222

int main(int argc, char **argv)
{
  if (argc!=2)
  {
    printf("Usage: %s <hostname>\n", argv[0]);
    return 1;
  }

  struct hostent *hp;
  const char *hostname = argv[1];
  printf("Connecting to %s\n", hostname);
  /* go find out about the desired host machine */
  if ((hp = gethostbyname(hostname)) == 0) 
  {
    perror("gethostbyname failed");
    return 1;
  }

  struct sockaddr_in pin;
  /* fill in the socket structure with host information */
  memset(&pin, 0, sizeof (pin));
  pin.sin_family = AF_INET;
  pin.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
  pin.sin_port = htons(SERVER_PORT);

  int sock; 

  /* grab an Internet domain socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
  {
    perror("socket() failed ");
    return 1;
  }

  /* connect to PORT on HOST */
  if (connect(sock, (struct sockaddr *) &pin, sizeof (pin)) == -1) 
  {
    perror("connect() failed");
    return 1;
  }

  /* Prepare socket set for select() */
  fd_set fds, tmp_set;
  FD_ZERO(&tmp_set);
  FD_SET(sock, &tmp_set);

  XML_CONTEXT *ctx = XML_CreateContext();
  ctx->onStanza = stanza_callback;
  ctx->onStreamBegin = stream_begin_callback;
  ctx->onStreamEnd = stream_end_callback;
  
  const char *greeting="<stream:stream "
    "to='%s' "
    "xmlns='jabber:client' "
    "xmlns:stream='http://etherx.jabber.org/streams' "
    "version='1.0'></stream:stream>";
  char outbuf[256];
  sprintf(outbuf, greeting, hostname);
  send(sock, outbuf, strlen(outbuf), 0);

  while (1)
  {
    memcpy(&fds, &tmp_set, sizeof(fd_set));
    int result = select(sock+1, &fds, NULL, NULL, NULL);
    if (result==0)
      printf("select() timeout\n");
    else if (result<0 && errno!=EINTR)
    {
      printf("Error in select(): %s\n", strerror(errno));
      break;
    }
    else if (result>0)
    {
      ssize_t n_read = recv(sock, netbuf, BUFSIZE, 0);
      XML_Decode(ctx, netbuf, n_read);
    }
  }
  
  XML_DestroyContext(ctx);

  close(sock);

  return 0;
}
