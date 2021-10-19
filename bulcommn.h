/*
 * bulcommn.h   define exported routines from bulcommn.c
 */

#ifndef BULL_BULCOMMN_H
#define BULL_BULCOMMN_H

// RNC compression magic identifier
#define RNC_SIGNATURE_STR "RNC\001"

// Routines

long file_length (char *path);
long file_length_opened (FILE *fp);
char *filename_from_path(char *pathname);

long read_long_le_file (FILE *fp);
long read_long_le_buf (unsigned char *buff);
unsigned short read_short_le_file (FILE *fp);
unsigned short read_short_le_buf (unsigned char *buff);

void write_short_le_file (FILE *fp, unsigned short x);
void write_short_le_buf (unsigned char *buff, unsigned short x);
void write_long_le_file (FILE *fp, unsigned long x);
void write_long_le_buf (unsigned char *buff, unsigned long x);

float read_float_le_buf(unsigned char *buff);

long read_long_be_buf (unsigned char *buff);

void write_short_be_buf (unsigned char *buff, unsigned short x);
void write_long_be_buf (unsigned char *buff, unsigned long x);

int nth_bit( unsigned char c, int n );
int nth_bit_fourbytes( unsigned char c[4], int n );

void write_bmp_idx (char *fname, int width, int height, unsigned char *pal, 
		char *data, int red, int green, int blue, int mult);
void write_bmp_24b (char *fname, int width, int height, char *data);

int read_palette_rgb(unsigned char *palette, char *fname, unsigned int nColors);

int rnc_compressed_buf (unsigned char *buff);
int rnc_compressed_file (FILE *fp);

#endif
