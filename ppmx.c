/******************************************************************************************
 *                                                                                        *
 *                   Mid Year Programming Contest: Image Data Processing                  *
 *                 Kyocera Document Solutions Development Philippines, Inc.               *
 *                                                                                        *
 ******************************************************************************************/

/*
 *=========================================================================================
 *                                     Description
 *-----------------------------------------------------------------------------------------
 *  This program performs the basic image processing techniques but can only read a     
 *  P6 PPM image format. This can do rescale with respect to aspect     
 *  ratio, image rotation, P6 PPM to P5 PGM (grayscale) and P6 PPM to P4 PBM (bilevel)  
 *  conversion, and image flip (vertically and horizontally).                           
 *                                                                                      
 *  For PPM information: http://netpbm.sourceforge.net/doc/ppm.html
 *-----------------------------------------------------------------------------------------
 *                                   Revision History
 *----------+---------------------------+--------------------------------------------------
 * 08/08/18 |   Philogene Kyle Dimpas   |   Updated the documentation style and added
 *          |                           |   comment #Philogene Kyle Dimpas in the header
 *----------+---------------------------+--------------------------------------------------
 * 07/27/18 |   Philogene Kyle Dimpas   |   Finished the Program 
 *=========================================================================================
*/

//=========================================================================================
//                                     Definitions                       
//=========================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PXL unsigned char
#define PGM unsigned char
#define PBM unsigned char
#define M_PI 3.14159265358979323846

#define EXIT(...)                                        \
{   fprintf(stderr, ""__VA_ARGS__);                      \
    if(fpIn != NULL) fclose(fpIn);                       \
    if(outImg.format.ppm != NULL)free(outImg.format.ppm);\
    if(srcImg.format.ppm != NULL)free(srcImg.format.ppm);\
    return EXIT_FAILURE;                                 \
}

typedef struct{
    PXL  R;
    PXL  G;
    PXL  B;
}PPM;

typedef union{
    PPM *ppm;
    PGM *pgm;
    PBM *pbm;
}fileFormat;

typedef struct{
    fileFormat format;
    unsigned int size;
}fileType;

//=========================================================================================
//                                     Global Variables                  
//=========================================================================================
int  headerInfo[3]; 
char fType[2];

//=========================================================================================
//                                   Function Prototypes                     
//=========================================================================================

int    allocMem(fileType *);
int    writeFile(fileType, char []);
int    parseOptions(char [], int *);
int    sortOptions(int, int*, char *[]);
int    rescaleWidth(fileType *, fileFormat *, int, int, int );
int    rotateImage(fileType *, fileFormat *, int, int, int );
void   flipHorizontal(fileType *, fileFormat *, int ,int );
void   flipVertical(fileType *, fileFormat *,  int ,int );
void   toGrayScale(fileFormat *, fileFormat *, int ,int );
void   dithering(fileFormat *, fileFormat *, int , int );
void   rotate90(fileType *, fileFormat *, int , int );
void   readHeader(FILE *);
void   options();
double round (double );

//=========================================================================================
//                                     Main Function                 
//=========================================================================================
int main(int argc, char *argv[])
{   
    fileType     outImg;
    fileType     srcImg;
    FILE        *fpIn     = NULL;
    char        *filename = NULL;
    int          optionIdx[10];
    int          i;
    int          param;

    outImg.format.ppm = NULL;
    srcImg.format.ppm = NULL;
    outImg.size = srcImg.size = 0;

    if(argc < 3){
        options();
        exit(1);
    }
    
    if((i = sortOptions(argc, optionIdx, argv)) == 0){
        EXIT();
    }else if(i == -1){
        options();
        EXIT("ERROR: -options invalid");
        
    }
    filename = argv[argc-1];
    fpIn = fopen(filename,"rb");

    if(fpIn == NULL){
        EXIT("ERROR: File not found");
    }

    fType[0] = fgetc(fpIn);
    fType[1] = fgetc(fpIn);
    if('P' != fType[0] || '6' != fType[1]){
        EXIT("ERROR: File not PPM P6 format");
    }

    readHeader(fpIn);
    if(!allocMem(&srcImg)){
        EXIT("ERROR: Source image cannot allocate memory");
    }
    
    if(!fread(srcImg.format.ppm,1,sizeof(PPM)*headerInfo[0]*headerInfo[1],fpIn)){
        EXIT("ERROR: fread cannot read source image");
    }
    if(!allocMem(&outImg)){
        EXIT("ERROR: Cannot create new file");
    }

    //planning to revise and remove the switch
    for(i=0 ; i < argc-2 ; i++){
        switch (parseOptions(argv[optionIdx[i]], &param)){
            case 1:
                flipVertical(&outImg, &srcImg.format, headerInfo[0], headerInfo[1]); break;
            case 2:
                flipHorizontal(&outImg, &srcImg.format, headerInfo[0], headerInfo[1]); break;
            case 3:
                if(!rescaleWidth(&outImg, &srcImg.format, param, headerInfo[0], headerInfo[1])){
                    EXIT("ERROR: maximum dimension ( 9999 x 9999 )");
                }
                break;
            case 4:
                if(!rotateImage(&outImg, &srcImg.format, param, headerInfo[0], headerInfo[1])){
                    EXIT("ERROR: failed to allocate memory for rotate image");
                }
                break;
            case 5:
                fType[1] = '4';
                allocMem(&outImg);
                toGrayScale(&srcImg.format, &srcImg.format, headerInfo[0], headerInfo[1]);
                dithering(&outImg.format, &srcImg.format,headerInfo[0], headerInfo[1]);
                break;
            case 6: 
                fType[1] = '5';
                allocMem(&outImg);
                toGrayScale(&outImg.format, &srcImg.format, headerInfo[0], headerInfo[1]);
                break;
            default: options(); EXIT(); break;
        }

        if(fType[1] == '6'){
            free(srcImg.format.ppm);
        }
        
        if(!allocMem(&srcImg)){
            EXIT("ERROR: Failed to allocate memory for output copy");
        }

        memcpy(srcImg.format.ppm, outImg.format.ppm, outImg.size);
        printf("%s ", argv[optionIdx[i]]);
    }
    if(!writeFile(outImg,filename)){
        EXIT("\nERROR: failed to write the file\n");
    }

    EXIT("done!");
    return 0;
}



//=========================================================================================
//                                      Functions                
//=========================================================================================


/*
 *=================================================================================
 *
 *  int parseOptions(char [], int* )
 * 
 *  Description:
 *   Parses the given string option.
 *  Return: 
 *   This will return an integer based on its corresponding type; otherwise its 0.                    
 *
 *=================================================================================
 */
int parseOptions(char option[], int *param)
{
    int i;
    int j;
    int type = -1;
    int newWidth = 0;
    char buff[10];
    
    for(i = 1 ; option[i] != '\0' && type ; i++){

        switch(option[i]){
            //-fv is type 1 and -fh is 2        
            case 'f':

                if((option[++i] == 'v' || option[i] == 'h') && option[i+1] == '\0'){
                    type = (option[i] == 'v')? 1: 2;
                }
                break;
            //-w is type 3 and -r is 4
            case 'w': 
            case 'r':
                //limits number to 12 digits only
                for(j=0, ++i ; j < 12 && option[i] != '\0' && isdigit(option[i]) ; j++, i++){
                    buff[j] = option[i];
                }

                if(option[i] == '\0' && j != 0){
                    *param = atoi(buff);
                    type = (option[1] == 'w')?3:4;
                }else{
                    printf("ERROR: invalid input");
                    type = 0;
                }
                //check if width is greater than 0
                
                type = (type == 3 && *param < 1 )? 0: type;
                type = (type == 4 && *param > 359)? 0: type;

                i = strlen(option) - 1;
                break;
            //-mono is type 5 and -gray is 6
            case 'm': 
            case 'g': 
            
                if(strcmp(option,"-mono") == 0){
                    type = 5;
                }else if(strcmp(option,"-gray") == 0){
                    type = 6;
                }
            
                i = strlen(option)-1;
                break;
            
            default : type = 0;  break;
        }
    }

    return type;

}
/*
 *=================================================================================
 *
 *  int sortOptions(int , int *, char *[])
 * 
 *  Description:
 *    Sorts the options according to it's heirarchy.
 *    heirarchy of -options: -w (-r -f) -gray -mono. (-r & -f are in same heirarchy)          
 *  Return:
 *    returns 1 if successful; else 0.                    
 *
 *=================================================================================
 */
int sortOptions(int size, int *options, char *argv[])
{
    
    unsigned char   optionSet = 0;  //128 = w, 64 = r, 32 = f, 16 = g, 8 = m, 1 = flag swap f and r
    char            str[100];
    int             i;
    int             idx;
    int             cnt;
    int             mask = 128;
    int             invalidFlag = 0;
    char            prioOptions[6] = {'w', 'r', 'f', 'g', 'm'};
    
    for(cnt = i = 0 ; i < 6 && cnt < size - 2 ; i ++, mask >>= 1){

        for(idx = 1 ; idx < size - 1; idx++){
            if(*argv[idx] != '-'){
                printf("ERROR: -options invalid");
                return 0;
            }
            if(*(argv[idx] + 1) == prioOptions[i]){
                //check if the -option is not yet in the set
                if((optionSet & mask) == mask){
                    printf("ERROR: duplicate options");
                    return 0;
                }
                if(i == 4 && (optionSet & 16) == 16){
                    printf("ERROR: conflict options (mono and gray)");
                    return 0;
                }

                options[cnt++] = idx;
                optionSet |= mask;
                //ideally rotate and flip can be exchanged but rotate is always be first
                //this condition will swap flip over rotate if flip was inputted first by the user
                if( (optionSet & 96) == 96 && (optionSet & 1) != 1){
                    if(idx < options[cnt - 2]){
                        options[cnt-2] = options[cnt-2] + options[cnt-1];
                        options[cnt-1] = options[cnt-2] - options[cnt-1];
                        options[cnt-2] = options[cnt-2] - options[cnt-1];
                    }
                    //check only once
                    optionSet |= 1;
                }
            }
        }

    }
    
    return (cnt > 0 && cnt == size-2)? 1 : -1;  
}

/*
 *=================================================================================
 *
 * int writeFile(fileType, char [])
 * 
 * Description:
 *   Creates the image based on the final content of the memory 
 * Return:
 *  returns 1 if successful; else 0                               
 *
 *=================================================================================
 */
int writeFile(fileType out, char srcName[])
{
    FILE    *fp;
    char    filename[50];

    //copy filename without .extension
    strncpy(filename, srcName, strlen(srcName)-4);
    filename[strlen(srcName)-4] = '\0';

    switch(fType[1]){
        case '6': strcat(filename,".ppm.out"); 
                  fp = fopen(filename,"wb");
                  break;
        case '5': strcat(filename,".pgm.out"); 
                  fp = fopen(filename,"wb");
                  break;
        case '4': strcat(filename,".pbm.out"); 
                  fp = fopen(filename,"wb");
                  break;
        default : fp = NULL;
                  break;
    }

    if(fp == NULL){
        printf("ERROR: cannot create new file");
        return 0;
    }
    //writes the header of the file
    if(!fprintf(fp,"P%c\n#Philogene Kyle Dimpas\n"
        "%d %d\n",fType[1],headerInfo[0],headerInfo[1])){
        printf("ERROR: Unable to write into the file");
        fclose(fp);
        return 0;
    }
    //writes maximum color value
    if(fType[1] != '4'){
        fprintf(fp,"%d\n", headerInfo[2]);
    }
    fwrite(out.format.ppm,1,out.size,fp);
    
    fclose(fp);
    return 1;
}

/*
 *=================================================================================
 *
 * int allocMem(fileType *)
 * 
 * Description:
 *   Allocates a chunk of memory based on the file format.  
 * Return:
 *   returns 1 if successful; else 0.                   
 *            
 *=================================================================================
 */
int allocMem(fileType *out)
{
    if(headerInfo[0] >= 10000 || headerInfo[1] >= 10000){
        return 0;
    }

    //allocates memory accoring to its file type
    switch(fType[1]){
        case '6':
            out->size = sizeof(PPM)*headerInfo[0]*headerInfo[1]; 
            out->format.ppm = (PPM*)malloc(out->size);
            break;
        case '5':
            out->size = sizeof(PGM)*headerInfo[0]*headerInfo[1];
            out->format.pgm = (PGM*)realloc(out->format.ppm, out->size);
            break;
        case '4': 
            out->size = sizeof(PBM)*(headerInfo[1]*headerInfo[0]);// + (sizeof(PBM)*headerInfo[0]*headerInfo[1]) % 8 != 0);
            out->format.pbm = (PBM*)realloc(out->format.ppm, out->size);
            break;

        default : 
            printf("ERROR: wrong file format");
            return 0;
    }

    return (out->format.ppm == NULL) ? 0: 1;
}

/*
 *=================================================================================
 *
 * void readHeader(FILE *)
 * 
 * Description:
 *   reads the header of the file and parse the content of the header                                                               
 *
 *=================================================================================
 */
void readHeader(FILE *fp)
{
    char    ch = 0;
    int     i = 0;
    int     cnt = 0;
    int     isComment = 0;
    int     isValue = 0;
    char    buff[10];
    
    //to get the 3 data (width, height, maximum size)   
    while(cnt <3){
        ch = fgetc(fp);
        //check the #comment strings                                
        if(ch != '#' && isComment != 1){
            //find the first character of the data
            if(!isspace(ch)){
                buff[i++] = ch;
            //last character of the data and then add '\0'  
            }else if(i > 0){
                buff[i] = '\0';
                //stores integer data in headerInfo
                headerInfo[cnt++] = atoi(buff);
                //this indicates that no data is found yet
                i = 0;
            }
        }else{
            //flags the start and the end of the comment
            isComment = (ch == 10)? 0: 1;
        }   
    }
}

/*
 *=================================================================================
 *
 * int rescaleWidth(fileType *, fileFormat *, int , int, int )
 * 
 * Description:
 *    resize the image given new width and P6 image using bilinear interpolation.
 * Return:
 *  returns 1 if successful; else 0.    
 *
 *=================================================================================
 */
int rescaleWidth(fileType *out, fileFormat *src, int newWidth, int width, int height)
{
    PPM     pxl[4];
    PPM     temp;
    int     x;
    int     y;
    int     xDiff;
    int     yDiff;
    int     index;
    int     i;
    int     j;
    int     offset = 0;
    float   xRatio;
    float   yRatio;
    
    //free the pre allocated memory to reallocate bigger memory
    free(out->format.ppm);
    out->format.ppm = NULL;
    
    headerInfo[0] = newWidth;
    headerInfo[1] = (int) ceil((float)height / width * newWidth); 

    if(!allocMem(out)){
        return 0;
    }

    xRatio = ( (float) width - 1) / headerInfo[0];
    yRatio = ( (float) height - 1) / headerInfo[1];
    
    for(i = 0; i < headerInfo[1]; i++ ){
        for(j=0; j < headerInfo[0]; j++){

            x = xRatio * j;
            y = yRatio * i;
            xDiff = (xRatio * j) - x;
            yDiff = (yRatio * i) - y;

            index = (y * width + x);
            
            pxl[0] = *(src->ppm + index);
            pxl[1] = *(src->ppm + index + 1);
            pxl[2] = *(src->ppm + index + width);
            pxl[3] = *(src->ppm + index + width + 1);

            //using bilinear interpolation algorithm for R G B
            
            temp.R = (PXL) (pxl[0].R * (1 - xDiff) * (1 - yDiff) + pxl[1].R * xDiff * (1 - yDiff) +
                    pxl[2].R * yDiff * (1 - xDiff) + pxl[3].R * (xDiff * yDiff));

            temp.G = (PXL) (pxl[0].G * (1 - xDiff) * (1 - yDiff) + pxl[1].G * xDiff * (1 - yDiff) +
                    pxl[2].G * yDiff * (1 - xDiff) + pxl[3].G * (xDiff * yDiff));

            temp.B = (PXL) (pxl[0].B * (1 - xDiff) * (1 - yDiff) + pxl[1].B * xDiff * (1 - yDiff) +
                    pxl[2].B * yDiff * (1 - xDiff) + pxl[3].B * (xDiff * yDiff));

            memcpy(out->format.ppm + offset++, &temp, sizeof(PPM));
        }
    }
    return 1;
}

//=================================================================================
// Function round() rounds the value. This is used in function rotateImage().     
//=================================================================================
double round(double val)
{    
    return floor(val + 0.5);
}

//=================================================================================
// Function rotate90() rotates the image in 90 degrees.                           
//=================================================================================
void rotate90(fileType *out, fileFormat *src, int width, int height)
{
    PPM *temp;
    int i;
    int j;

    temp = out->format.ppm;
    //the 2 loops for the rotation
    for(i =0 ; i < width ; i++){
        for(j = height-1 ; j>=0 ; j--){
            memcpy(out->format.ppm++, src->ppm + i + j*width, sizeof(PPM));
        }
    }
    out->format.ppm = temp;
}

/*
 *=================================================================================
 *
 * int rotateImage(fileType *, fileFormat *, int, int ,int)
 * 
 * Description:
 *   rotates P6 PPM image using biliniear interpolation   
 * Return:
 *   returns 1 if successful; else 0;                         
 *
 *=================================================================================
 */
int rotateImage(fileType *out, fileFormat *src, int angle, int width, int height)
{
    const double cnAngle = (angle * M_PI / 180);
    int          i;
    int          j;
    int          x;
    int          y;
    int          iFloorX;
    int          iCeilingX;
    int          iFloorY;
    int          iCeilingY;
    int          iCentreX;
    int          iCentreY;
    int          iDestCentreX;
    int          iDestCentreY;
    int          iWidth;
    int          iHeight;
    double       fDistance;
    double       fPolarAngle;
    double       fTrueX;
    double       fTrueY;
    double       fDeltaX;
    double       fDeltaY;
    double       fTopRed;
    double       fTopGreen;
    double       fTopBlue;
    double       fBottomRed;
    double       fBottomGreen;
    double       fBottomBlue;
    PPM          color[4];
    PPM          tempRGB;


    //pythagorean theorem
    //sin(angle) * hypotenuse + cos(angle) * hypotenuse 
    iWidth  = ceil( abs(sin(cnAngle) * headerInfo[1]) + abs(cos(cnAngle) * headerInfo[0]));
    iHeight = ceil( abs(sin(cnAngle) * headerInfo[0]) + abs(cos(cnAngle) * headerInfo[1]));

    iCentreX = width / 2;
    iCentreY = height / 2;

    iDestCentreX = iWidth / 2;
    iDestCentreY = iHeight / 2;
    headerInfo[0] = iWidth;
    headerInfo[1] = iHeight;

    free(out->format.ppm);
    out->format.ppm = NULL;
    if(!allocMem(out)){
        return 0;
    }
    
    //angles 0, 180, 90 and 270 should have a different function and should not reuse flips and rotate90
    //planning to revise
    if(angle == 0){
        memcpy(out->format.ppm, src->ppm, sizeof(PPM)*width*height);
        return 1;
    }
    if(angle == 180){
        flipHorizontal(out, src, width, height);
        memcpy(src->ppm, out->format.ppm, out->size);
        flipVertical(out, src, width, height);
        return 1;
    }
    if(angle == 90){
        rotate90(out, src, width, height);
        return 1;
    }
    if(angle == 270){
        rotate90(out, src, width, height);
        memcpy(src->ppm, out->format.ppm, out->size);
        flipVertical(out,src,height,width);
        memcpy(src->ppm, out->format.ppm, out->size);
        flipHorizontal(out,src,height,width);
        return 1;
    }


    memset(out->format.ppm, 0, out->size);
    
    for(i=0 ; i < iHeight ; ++i){
        
        for(j=0 ; j < iWidth ; ++j){
            x = j - iDestCentreX;
            y = iDestCentreY - i;

            fDistance = sqrt( x * x + y * y);
            fPolarAngle = 0.0;
            if(x == 0){
                fPolarAngle = (y < 0)? 1.5 * M_PI : 0.5 * M_PI;
            }else{
                fPolarAngle = atan2(y,x);
            }

            fPolarAngle += cnAngle;
            
            fTrueX = fDistance * cos(fPolarAngle);
            fTrueY = fDistance * sin(fPolarAngle);

            fTrueX +=iCentreX;
            fTrueY = iCentreY - fTrueY;

            iFloorX = floor(fTrueX);
            iFloorY = floor(fTrueY);
            iCeilingX = ceil(fTrueX);
            iCeilingY = ceil(fTrueY);
            
            // check bounds
            if (iFloorX < 0 || iCeilingX < 0 || iFloorX >= width || iCeilingX >= width || iFloorY < 0 || iCeilingY < 0 || iFloorY >= height || iCeilingY >= height) continue;
            
            fDeltaX = fTrueX - iFloorX;
            fDeltaY = fTrueY - iFloorY;

            //colors from topleft, topright, bottomleft and bottomright respectively
            color[0] = *(src->ppm + iFloorX + iFloorY * width);
            color[1] = *(src->ppm + iCeilingX + iFloorY * width);
            color[2] = *(src->ppm + iFloorX + iCeilingY * width);
            color[3] = *(src->ppm + iCeilingX +  iCeilingY * width);
            
            // linearly interpolate horizontally between top neighbours
            fTopRed = (1 - fDeltaX) * color[0].R + fDeltaX * color[1].R;
            fTopGreen = (1 - fDeltaX) * color[0].G + fDeltaX * color[1].G;
            fTopBlue = (1 - fDeltaX) * color[0].B + fDeltaX * color[1].B;

            // linearly interpolate horizontally between bottom neighbours
            fBottomRed = (1 - fDeltaX) * color[2].R + fDeltaX * color[3].R;
            fBottomGreen = (1 - fDeltaX) * color[2].G + fDeltaX * color[3].G;
            fBottomBlue = (1 - fDeltaX) * color[2].B + fDeltaX * color[3].B;

            // linearly interpolate vertically between top and bottom interpolated results
            tempRGB.R = round((1 - fDeltaY) * fTopRed + fDeltaY * fBottomRed);
            tempRGB.G = round((1 - fDeltaY) * fTopGreen + fDeltaY * fBottomGreen);
            tempRGB.B = round((1 - fDeltaY) * fTopBlue + fDeltaY * fBottomBlue);

            memcpy(out->format.ppm + j +  i*iWidth, &tempRGB, sizeof(PPM));
            
        }
    }

    return 1;
}

/*
 *=================================================================================
 *
 * void toGrayScale(fileFormat *out, fileFormat *, int, int )
 * 
 * Description:
 *   converts 3 bytes RGB pixel to 1 byte grayscale pixel  
 *                                    
 *=================================================================================
 */
void toGrayScale(fileFormat *out, fileFormat *src, int width, int height)
{
    int     i;
    int     size;
    PGM     grey;
    PPM     *rgb;

    size = height*width;
    for(i=0; i < size; i++){
        //gets the RGB pixels sequentially 
        rgb = src->ppm + i;
        //converts 3 bytes RGB pixel to 1 byte grayscaled pixel
        grey = ((rgb->R * 299) + (rgb->G * 587) + (rgb->B * 114))/1000;
        memcpy(out->pgm + i, &grey,sizeof(PGM));
    }
}

/*
 *=================================================================================
 *
 * void dithering()
 * 
 * Description:
 *   converts P5 PGM to P4 PBM using ordered dithering technique (bayer 8x8)
 *                                    
 *=================================================================================
 */
void dithering(fileFormat *out, fileFormat *src, int width, int height)
{
    int     i;
    int     j;
    int     iMtrx;
    int     jMtrx;
    int     n = 128;
    int     error = 0;
    PBM     pbm;
    PBM     *temp;
    PBM     *oldPxl;
#if 0
    int     bayer[8][8] = { { 96,  40,  48, 104, 140, 188, 196, 148},
                            { 32,   4,   8,  56, 180, 236, 244, 204},
                            { 88,  24,  16,  64, 172, 228, 252, 212},
                            {120,  80,  72, 112, 132, 164, 220, 156},
                            {136, 184, 192, 144, 100,  44,  52, 108},
                            {176, 232, 240, 200,  36,   4,  12,  60},
                            {168, 224, 248, 208,  92,  28,  20,  68},
                            {128, 160, 216, 152, 124,  84,  76, 166}};
#else
     int    bayer[4][4] = {{ 16, 143,  47, 175},
                           {207,  79, 239, 111},
                           { 63, 191,  31, 159},
                           {255, 127, 223,  95}};
#endif  
    //holds the first address of the image
    temp = out->pgm;
    for(i = 0 ; i < height ; i++){
        for(j = 0 ; j < width ; ){
            for(n = 128, pbm = 0 ; j < width && n > 0 ;  j++, n >>=1){
                oldPxl = src->pbm + j + i * width;
                //masks old pixel to its corresponding bayer mask
                pbm = (*oldPxl <= bayer[i % 4][j % 4])? pbm | n : pbm;  
            }
            //writes the 8 pixels (1 byte)
            memcpy(out->pbm++, &pbm, sizeof(PBM));
        }
    }
    //gets the first address of the image
    out->pbm = temp;
    
}

/*
 *=================================================================================
 *
 * void flipHorizontal(fileType *, fileFormat *, int ,int );
 * 
 * Description:
 *   flips the image horizontally
 *                                    
 *=================================================================================
 */
void flipHorizontal(fileType *out, fileFormat *src, int width, int height)
{
    int          i;
    int          j;
    fileFormat   temp;
    
    //holds the first address of the image
    temp.ppm = out->format.ppm;
    for(i=0; i < height; i++){
        for(j = width-1; j>=0;j--){
            memcpy(out->format.ppm, src->ppm + j + i*width,sizeof(PPM));
            out->format.ppm++;
        }
    }
    //gets the first address of the image
    out->format.ppm = temp.ppm;
    
}

/*
 *=================================================================================
 *
 * void flipVertical(fileType *, fileFormat *, int ,int )
 * 
 * Description:
 *   flips the image vertically
 *                                    
 *=================================================================================
 */
void flipVertical(fileType *out, fileFormat *src,  int width, int height)
{
    int          i;
    int          j;
    fileFormat   temp;

    //holds the first address of the image
    temp.ppm = out->format.ppm;
    for(i = height - 1; i >= 0; i--){
        for(j = 0; j < width; j++){
            memcpy(out->format.ppm, src->ppm + j + i*width, sizeof(PPM));
            out->format.ppm++;
        }
    }
    //gets the first address of the image
    out->format.ppm = temp.ppm;
    
}

/*
 *=================================================================================
 *
 * void options()
 * 
 * Description:
 *   displays usage of the program
 *                                    
 *=================================================================================
 */
void options()
{
    printf("\nUsage: ppmx [options] (filename.ppm)");
    printf("\nOptions:\n-fv\t\tFlip vertically");
    printf("\n-fh\t\tFlip horizontally");
    printf("\n-w<width>\tScale to the new width (0 - 9999)");
    printf("\n-r<angle>\tRotate CW (0 - 359)\n-mono\t\tConvert to bilevel (.pbm)format");
    printf("\n-gray\t\tConvert to grayscale (.pgm) format\n");
}

