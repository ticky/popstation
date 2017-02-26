int getsize(FILE *f);
void ErrorExit(char *fmt, ...);
int deflateCompress(void *inbuf, int insize, void *outbuf, int outsize,
                    int level);
void SetSFOTitle(char *sfo, char *title);

#define N_GAME_CODES 12

char *gamecodes[N_GAME_CODES] = {"SCUS", "SLUS", "SLES", "SCES",
                                 "SCED", "SLPS", "SLPM", "SCPS",
                                 "SLED", "SLPS", "SIPS", "ESPM"};

#define BASE "BASE.PBP"

char buffer[1 * 1048576];
char buffer2[0x9300];

int pic0 = 0, pic1 = 0, icon0 = 0, icon1 = 0, snd = 0, prx = 0;
int sfo_size, pic0_size, pic1_size, icon0_size, icon1_size, snd_size, prx_size;
int start_dat = 0;
unsigned int psp_header[0x30 / 4];
unsigned int base_header[0x28 / 4];
unsigned int header[0x28 / 4];
unsigned int dummy[6];

#ifndef WIN32
typedef struct __attribute__((packed))
#else
#pragma pack(1)
typedef struct
#endif
{
  unsigned int offset;
  unsigned int length;
  unsigned int dummy[6];
} IsoIndex;
#ifdef WIN32
#pragma pack()
#endif
