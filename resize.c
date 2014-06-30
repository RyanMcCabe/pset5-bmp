/**
 * copy.c
 *
 * Computer Science 50
 * Problem Set 5
 *
 * Copies a BMP piece by piece, just because.
 */
       
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "bmp.h"


int main(int argc, char* argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        printf("Usage: ./resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    int n = atoi(argv[1]);
    char* infile = argv[2];
    char* outfile = argv[3];
    
    //checks the value of n is allowed
    if (n < 1 || n > 100)
    {
        printf("value of n must be positve and less than 100\n");
        return 5;
    }

    // open input file 
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE* outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // storage of variable from orginal bmp for fseek function later
    int oldPadding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    LONG oldWidth = bi.biWidth; 
    DWORD oldSizeImg = bi.biSizeImage;
    LONG oldHeight = bi.biHeight;    
    DWORD oldSize = bf.bfSize;
    
    // new size of of image for header info.
    bi.biHeight *= n;
    bi.biWidth *= n;
    
    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;    
    
    // use new information to determine headerinfo
    bi.biSizeImage = ((abs(bi.biWidth) * sizeof(RGBTRIPLE) + padding) * abs(bi.biHeight));
    bf.bfSize = oldSize - oldSizeImg + bi.biSizeImage;
    
    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }

    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);


    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(oldHeight); i < biHeight; i++)
    {
        
        // iterate additional scanlines to extend vertically
        for (int j = 0; j < n; j++)
        {
            // iterate over pixels in scanline
            for (int k = 0; k < oldWidth; k++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            
            
                // write RGB triple to outfile n times
                for (int l = 0; l < n; l++)
                {
                     fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }    
                
                // add padding at end of scanline
            for (int m = 0; m < padding; m++)
            {
                fputc(0x00, outptr);
            }
            // reset to the beginning of scanline to read again for vertical extension
            fseek(inptr, (sizeof(RGBTRIPLE) * -oldWidth), SEEK_CUR);
        }    
        
        // skip forward to the next line
        fseek(inptr, oldPadding + (sizeof(RGBTRIPLE) * oldWidth), SEEK_CUR);
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // that's all folks
    return 0;
}
