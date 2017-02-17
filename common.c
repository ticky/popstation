#include <stdio.h>
#include <string.h>
#include <zlib.h>

int getsize(FILE *f)
{
  int size;

  fseek(f, 0, SEEK_END);
  size = ftell(f);

  fseek(f, 0, SEEK_SET);
  return size;
}


void ErrorExit(char *fmt, ...)
{
  va_list list;
  char msg[256];  

  va_start(list, fmt);
  vsprintf(msg, fmt, list);
  va_end(list);

  printf(msg);
  exit(-1);
}

z_stream z;

int deflateCompress(void *inbuf, int insize, void *outbuf, int outsize, int level)
{
  int res;
  
  z.zalloc = Z_NULL;
  z.zfree  = Z_NULL;
  z.opaque = Z_NULL;

  if (deflateInit2(&z, level , Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
    return -1;

  z.next_out  = outbuf;
  z.avail_out = outsize;
  z.next_in   = inbuf;
  z.avail_in  = insize;

  if (deflate(&z, Z_FINISH) != Z_STREAM_END)
  {
    return -1;
  }

  res = outsize - z.avail_out;

  if (deflateEnd(&z) != Z_OK)
    return -1;

  return res;
}

#ifndef WIN32
typedef struct __attribute__((packed))
#else
#pragma pack(1)
typedef struct
#endif
{
  unsigned int signature;
  unsigned int version;
  unsigned int fields_table_offs;
  unsigned int values_table_offs;
  int nitems;
} SFOHeader;
#ifdef WIN32
#pragma pack()
#endif

#ifndef WIN32
typedef struct  __attribute__((packed))
#else
#pragma pack(1)
typedef struct
#endif
{
  unsigned short field_offs;
  unsigned char  unk;
  unsigned char  type; // 0x2 -> string, 0x4 -> number
  unsigned int length;
  unsigned int size;
  unsigned short val_offs;
  unsigned short unk4;
} SFODir;
#ifdef WIN32
#pragma pack()
#endif

void SetSFOTitle(char *sfo, char *title)
{
  SFOHeader *header = (SFOHeader *)sfo;
  SFODir *entries = (SFODir *)(sfo+0x14);
  int i;
  
  for (i = 0; i < header->nitems; i++)
  {
    if (strcmp(sfo+header->fields_table_offs+entries[i].field_offs, "TITLE") == 0)
    {
      strncpy(sfo+header->values_table_offs+entries[i].val_offs, title, entries[i].size);
      
      if (strlen(title)+1 > entries[i].size)
      {
        entries[i].length = entries[i].size;
      }
      else
      {
        entries[i].length = strlen(title)+1;
      }
    }
  }
}
