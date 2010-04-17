#include "dict.h"

/** Serialized Compressed TRIE format:
 * node =
 * {
 *   u8 value;
 *   u8 n_links;
 *   u8 links[n_links];
 *   {u16_le next_node; asciiz link} links[n_links];
 * }
 * next_node -- offset of the next node relative to the beginning of TRIE
 * links[i] -- offset of the link relative to links[i]
 */

unsigned int dict_match(const char *dict, const char *word)
{
  const char *node = dict;

  while (*word!=0)
  {
    const unsigned int n_links = node[1];
    const char *link_offset = node+2;
    unsigned int next_node_offset = 0;

    /**
     * FIXME: Should we use binary search?
     * Average number of links is very small.
     */
    for (unsigned int i=0; next_node_offset==0 && i<n_links; i++, link_offset++)
    {
      const char *link = link_offset + *link_offset+2;
      const unsigned char *offset_p = (unsigned char *)link-2;

      // Find common prefix length
      unsigned int p = 0;
      while ((link[p] & word[p])!=0 && link[p]==word[p])
        p++;

      if (link[p]==0) // link-string is a prefix, follow the link 
      {
        next_node_offset = offset_p[0] + (offset_p[1]<<8);
        word += p;
      }
    }

    if (next_node_offset==0) // no valid links found
      return 0;
    node = dict + next_node_offset;
  }
  return node[0]; // Node's value
}

