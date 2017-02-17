#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

#include "data.h"
#include "common.h"

unsigned char data3[128] = 
{
	0x8C, 0x0E, 0xAE, 0x9D, 0x39, 0x8C, 0xFE, 0x24, 0x65, 0x21, 0xAD, 0x2D, 0x65, 0xA9, 0x61, 0xDF, 
	0xD4, 0x4A, 0x13, 0x03, 0xB4, 0xCD, 0x32, 0x5F, 0xBD, 0xB0, 0xF4, 0xF9, 0x8A, 0x70, 0xBE, 0x1E, 
	0x39, 0x2C, 0x7D, 0xC0, 0xC1, 0xC6, 0x6B, 0x81, 0xAA, 0x3C, 0x06, 0x53, 0x94, 0x1B, 0xCE, 0xE5, 
	0x44, 0x16, 0xCF, 0xDB, 0xB1, 0xE3, 0x89, 0x7B, 0xA2, 0xD2, 0xE7, 0xD6, 0xC1, 0x26, 0x6B, 0x58, 
	0x8D, 0x2C, 0xE6, 0xC3, 0x15, 0x97, 0xD0, 0x29, 0xC9, 0x16, 0x81, 0xB6, 0xCC, 0x42, 0xEE, 0x0C, 
	0x28, 0x10, 0xEA, 0xF2, 0x6B, 0x6F, 0x90, 0x30, 0x05, 0xBE, 0x4A, 0x2F, 0x4A, 0xBC, 0xDC, 0xE5, 
	0x87, 0xCE, 0x19, 0xB9, 0x80, 0xDE, 0xB8, 0x32, 0xDD, 0xAD, 0x89, 0x67, 0xE0, 0x92, 0x78, 0x89, 
	0xE8, 0xDC, 0x45, 0x1D, 0x0C, 0xBE, 0x8B, 0x99, 0x4D, 0x50, 0xB3, 0xD6, 0x58, 0x96, 0x61, 0x75
};

#define WriteInteger(a, c) \
	x = a; \
	for (i = 0; i < c; i++) \
		fwrite(&x, 1, 4, out)

#define WriteChar(a, c) \
	for (i = 0; i < c; i++) \
		fputc(a, out)

#define WriteRandom(c) \
	for (i = 0; i < c; i++) \
	{ \
		x = lrand48(); \
		fwrite(&x, 1, 4, out); \
	}

void convert(int ndiscs, char **inputs, char *output, char *title, char **titles, char *code, char **codes, int *complevels)
{
	FILE *in, *out, *base, *t;
	int i, offset, isosize, isorealsize, x;
	int index_offset, p1_offset, p2_offset, m_offset, end_offset;
	IsoIndex *indexes;
	int iso_positions[5];
	int ciso;	
	
	base = fopen(BASE, "rb");
	if (!base)
	{
		ErrorExit("Cannot open %s\n", BASE);
	}

	out = fopen(output, "wb");
	if (!out)
	{
		ErrorExit("Cannot create %s\n", output);
	}

	printf("Writing PBP header...\n");

	fread(base_header, 1, 0x28, base);

	if (base_header[0] != 0x50425000)
	{
		ErrorExit("%s is not a PBP file.\n", BASE);
	}

	sfo_size = base_header[3] - base_header[2];
	
	t = fopen("ICON0.PNG", "rb");
	if (t)
	{
		icon0_size = getsize(t);
		icon0 = 1;
		fclose(t);
	}
	else
	{
		icon0_size = base_header[4] - base_header[3];
	}

	t = fopen("ICON1.PMF", "rb");
	if (t)
	{
		icon1_size = getsize(t);
		icon1 = 1;
		fclose(t);
	}
	else
	{
		icon1_size = 0;
	}

	t = fopen("PIC0.PNG", "rb");
	if (t)
	{
		pic0_size = getsize(t);
		pic0 = 1;
		fclose(t);
	}
	else
	{
		pic0_size = 0; //base_header[6] - base_header[5];
	}

	t = fopen("PIC1.PNG", "rb");
	if (t)
	{
		pic1_size = getsize(t);
		pic1 = 1;
		fclose(t);
	}
	else
	{
		pic1_size = 0; // base_header[7] - base_header[6];
	}

	t = fopen("SND0.AT3", "rb");
	if (t)
	{
		snd_size = getsize(t);
		snd = 1;
		fclose(t);
	}
	else
	{
		snd = 0;
	}
	t = fopen("DATA.PSP", "rb");
	if (t)
	{
		prx_size = getsize(t);
		prx = 1;
		fclose(t);
	}
	else
	{
		fseek(base, base_header[8], SEEK_SET);
		fread(psp_header, 1, 0x30, base);

		prx_size = psp_header[0x2C/4];
	}

	int curoffs = 0x28;

	header[0] = 0x50425000;
	header[1] = 0x10000;
	
	header[2] = curoffs;

	curoffs += sfo_size;
	header[3] = curoffs;

	curoffs += icon0_size;
	header[4] = curoffs;

	curoffs += icon1_size;
	header[5] = curoffs;

	curoffs += pic0_size;
	header[6] = curoffs;

	curoffs += pic1_size;
	header[7] = curoffs;

	curoffs += snd_size;
	header[8] = curoffs;

	x = header[8] + prx_size;

	if ((x % 0x10000) != 0)
	{
		x = x + (0x10000 - (x % 0x10000));
	}
	
	header[9] = x;

	fwrite(header, 1, 0x28, out);

	printf("Writing sfo...\n");

	fseek(base, base_header[2], SEEK_SET);
	fread(buffer, 1, sfo_size, base);
	SetSFOTitle(buffer, title);
	strcpy(buffer+0x108, code);
	fwrite(buffer, 1, sfo_size, out);

	printf("Writing icon0.png...\n");

	if (!icon0)
	{
		fseek(base, base_header[3], SEEK_SET);
		fread(buffer, 1, icon0_size, base);
		fwrite(buffer, 1, icon0_size, out);
	}
	else
	{
		t = fopen("ICON0.PNG", "rb");
		fread(buffer, 1, icon0_size, t);
		fwrite(buffer, 1, icon0_size, out);
		fclose(t);
	}

	if (icon1)
	{
		printf("Writing icon1.pmf...\n");
		
		t = fopen("ICON1.PMF", "rb");
		fread(buffer, 1, icon1_size, t);
		fwrite(buffer, 1, icon1_size, out);
		fclose(t);
	}

	if (!pic0)
	{
		//fseek(base, base_header[5], SEEK_SET);
		//fread(buffer, 1, pic0_size, base);
		//fwrite(buffer, 1, pic0_size, out);
	}
	else
	{
		printf("Writing pic0.png...\n");
		
		t = fopen("PIC0.PNG", "rb");
		fread(buffer, 1, pic0_size, t);
		fwrite(buffer, 1, pic0_size, out);
		fclose(t);
	}

	if (!pic1)
	{
		//fseek(base, base_header[6], SEEK_SET);
		//fread(buffer, 1, pic1_size, base);
		//fwrite(buffer, 1, pic1_size, out);		
	}
	else
	{
		printf("Writing pic1.png...\n");
		
		t = fopen("PIC1.PNG", "rb");
		fread(buffer, 1, pic1_size, t);
		fwrite(buffer, 1, pic1_size, out);
		fclose(t);
	}

	if (snd)
	{
		printf("Writing snd0.at3...\n");
		
		t = fopen("SND0.AT3", "rb");
		fread(buffer, 1, snd_size, t);
		fwrite(buffer, 1, snd_size, out);
		fclose(t);
	}

	printf("Writing DATA.PSP...\n");

	if (prx)
	{
		t = fopen("DATA.PSP", "rb");
		fread(buffer, 1, prx_size, t);
		fwrite(buffer, 1, prx_size, out);
		fclose(t);
	}
	else
	{
		fseek(base, base_header[8], SEEK_SET);
		fread(buffer, 1, prx_size, base);
		fwrite(buffer, 1, prx_size, out);
	}

	offset = ftell(out);
	
	for (i = 0; i < header[9]-offset; i++)
	{
		fputc(0, out);
	}

	printf("Writing PSTITLE header...\n");

	fwrite("PSTITLEIMG000000", 1, 16, out);

	// Save this offset position
	p1_offset = ftell(out);
	
	WriteInteger(0, 2);
	WriteInteger(0x2CC9C5BC, 1);
	WriteInteger(0x33B5A90F, 1);
	WriteInteger(0x06F6B4B3, 1);
	WriteInteger(0xB25945BA, 1);
	WriteInteger(0, 0x76);

	m_offset = ftell(out);

	memset(iso_positions, 0, sizeof(iso_positions));
	fwrite(iso_positions, 1, sizeof(iso_positions), out);

	WriteRandom(12);
	WriteInteger(0, 8);
	
	fputc('_', out);
	fwrite(code, 1, 4, out);
	fputc('_', out);
	fwrite(code+4, 1, 5, out);

	WriteChar(0, 0x15);
	
	p2_offset = ftell(out);
	WriteInteger(0, 2);
	
	fwrite(data3, 1, sizeof(data3), out);
	fwrite(title, 1, strlen(title), out);

	WriteChar(0, 0x80-strlen(title));
	WriteInteger(7, 1);
	WriteInteger(0, 0x1C);

	for (ciso = 0; ciso < ndiscs; ciso++)
	{
		in = fopen (inputs[ciso], "rb");
		if (!in)
		{
			ErrorExit("Cannot open %s\n", inputs[ciso]);
		}

		isosize = getsize(in);
		isorealsize = isosize;

		if ((isosize % 0x9300) != 0)
		{
			isosize = isosize + (0x9300 - (isosize%0x9300));
		}
		
		offset = ftell(out);

		if (offset % 0x8000)
		{
			x = 0x8000 - (offset % 0x8000);

			WriteChar(0, x);			
		}

		iso_positions[ciso] = ftell(out) - header[9];

		printf("Writing header (iso #%d)\n", ciso+1);

		fwrite("PSISOIMG0000", 1, 12, out);

		WriteInteger(0, 0xFD);

		memcpy(data1+1, codes[ciso], 4);
		memcpy(data1+6, codes[ciso]+4, 5);
		fwrite(data1, 1, sizeof(data1), out);

		WriteInteger(0, 1);
		
		strcpy(data2+8, titles[ciso]);
		fwrite(data2, 1, sizeof(data2), out);

		index_offset = ftell(out);

		printf("Writing indexes (iso #%d)...\n", ciso+1);

		memset(dummy, 0, sizeof(dummy));

		offset = 0;

		if (complevels[ciso] == 0)
		{	
			x = 0x9300;
		}
		else
		{
			x = 0;
		}

		for (i = 0; i < isosize / 0x9300; i++)
		{
			fwrite(&offset, 1, 4, out);
			fwrite(&x, 1, 4, out);
			fwrite(dummy, 1, sizeof(dummy), out);

			if (complevels[ciso] == 0)
				offset += 0x9300;
		}

		offset = ftell(out);

		for (i = 0; i < (iso_positions[ciso]+header[9]+0x100000)-offset; i++)
		{
			fputc(0, out);
		}

		printf("Writing iso #%d (%s)...\n", ciso+1, inputs[ciso]);

		if (complevels[ciso] == 0)
		{
			while ((x = fread(buffer, 1, 1048576, in)) > 0)
			{
				fwrite(buffer, 1, x, out);
			}

			for (i = 0; i < (isosize-isorealsize); i++)
			{
				fputc(0, out);
			}
		}
		else
		{
			indexes = (IsoIndex *)malloc(sizeof(IsoIndex) * (isosize/0x9300));

			if (!indexes)
			{
				fclose(in);
				fclose(out);
				fclose(base);

				ErrorExit("Cannot alloc memory for indexes!\n");
			}

			i = 0;
			offset = 0;

			while ((x = fread(buffer2, 1, 0x9300, in)) > 0)
			{
				if (x < 0x9300)
				{
					memset(buffer2+x, 0, 0x9300-x);
				}
			
				x = deflateCompress(buffer2, 0x9300, buffer, sizeof(buffer), complevels[ciso]);

				if (x < 0)
				{
					fclose(in);
					fclose(out);
					fclose(base);
					free(indexes);

					ErrorExit("Error in compression!\n");
				}

				memset(&indexes[i], 0, sizeof(IsoIndex));

				indexes[i].offset = offset;

				if (x >= 0x9300) /* Block didn't compress */
				{				
					indexes[i].length = 0x9300;
					fwrite(buffer2, 1, 0x9300, out);
					offset += 0x9300;
				}
				else
				{
					indexes[i].length = x;
					fwrite(buffer, 1, x, out);
					offset += x;
				}

				i++; 
			}

			if (i != (isosize/0x9300))
			{
				fclose(in);
				fclose(out);
				fclose(base);
				free(indexes);

				ErrorExit("Some error happened.\n");
			}
		}

		if (complevels[ciso] != 0)
		{
			offset = ftell(out);
			
			printf("Updating compressed indexes (iso #%d)...\n", ciso+1);

			fseek(out, index_offset, SEEK_SET);
			fwrite(indexes, 1, sizeof(IsoIndex) * (isosize/0x9300), out);

			fseek(out, offset, SEEK_SET);
		}
	}

	x = ftell(out);

	if ((x % 0x10) != 0)
	{
		end_offset = x + (0x10 - (x % 0x10));
			
		for (i = 0; i < (end_offset-x); i++)
		{
			fputc('0', out);
		}
	}
	else
	{
		end_offset = x;
	}

	end_offset -= header[9];

	printf("Writing special data...\n");
	
	fseek(base, base_header[9]+12, SEEK_SET);
	fread(&x, 1, 4, base);

	x += 0x50000;

	fseek(base, x, SEEK_SET);
	fread(buffer, 1, 8, base);
	
	if (memcmp(buffer, "STARTDAT", 8) != 0)
	{
		ErrorExit("Cannot find STARTDAT in %s.\n", 
			      "Not a valid PSX eboot.pbp\n", BASE);
	}

	fseek(base, x, SEEK_SET);

	while ((x = fread(buffer, 1, 1048576, base)) > 0)
	{
		fwrite(buffer, 1, x, out);
	}

	fseek(out, p1_offset, SEEK_SET);
	fwrite(&end_offset, 1, 4, out);

	end_offset += 0x2d31;
	fseek(out, p2_offset, SEEK_SET);
	fwrite(&end_offset, 1, 4, out);

	fseek(out, m_offset, SEEK_SET);
	fwrite(iso_positions, 1, sizeof(iso_positions), out);

	fclose(in);
	fclose(out);
	fclose(base);
}

void usage(char *prog)
{
	ErrorExit("Usage: %s main_title <titles> main_gamecode <gamecodes> <compressionlevels> <files>\n", prog);
}

int main(int argc, char *argv[])
{
	int i, j;
	int ndiscs;
	int complevels[5];
	time_t t;

	time(&t);
	srand48(t);
	
	if (argc != 11 && argc != 15 && argc != 19 && argc != 23)
	{
		printf("Invalid number of arguments.\n");
		usage(argv[0]);
	}

	ndiscs = ((argc - 11) / 4) + 2;	

	for (i = 0; i < (ndiscs+1); i++)
	{
		if (strlen(argv[ndiscs+i+2]) != 9)
		{
			printf("Invalid game code %s\n", argv[ndiscs+i+2]);
			usage(argv[0]);
		}
		
		for (j = 0; j < N_GAME_CODES; j++)
		{
			if (strncmp(argv[ndiscs+i+2], gamecodes[j], 4) == 0)
				break;
		}

		if (j == N_GAME_CODES)
		{
			printf("Invalid game code %s\n", argv[ndiscs+i+2]);
			usage(argv[0]);
		}

		for (j = 4; j < 9; j++)
		{
			if (argv[ndiscs+i+2][j] < '0' || argv[ndiscs+i+2][j] > '9')
			{
				printf("Invalid game code %s\n", argv[ndiscs+i+2]);
				usage(argv[0]);
			}
		}
	}

	for (i = 0; i < ndiscs; i++)
	{
		if (strlen(argv[3+(ndiscs*2)+i]) != 1)
		{
			printf("Invalid compression level %s\n", argv[3+(ndiscs*2)+i]);
			usage(argv[0]);
		}

		if (argv[3+(ndiscs*2)+i][0] < '1' || argv[3+(ndiscs*2)+i][0] > '9')
		{
			printf("Invalid compression level: %s (must be 1-9)\n", argv[3+(ndiscs*2)+i]);
			usage(argv[0]);
		}

		complevels[i] = argv[3+(ndiscs*2)+i][0] - '0';
	}

	convert(ndiscs, argv+(3+(ndiscs*3)), "EBOOT.PBP", argv[1], argv+2, argv[ndiscs+2], argv+ndiscs+3, complevels);

	printf("Done.\n");
	return 0;
}

