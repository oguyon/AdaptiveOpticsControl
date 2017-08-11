/*
 * Example code to write image in shared memory
 * 
 * compile with:
 * gcc ImCreate_test.c ImageStreamIO.c -lm -lpthread 
 * 
 * Required files in compilation directory :
 * ImCreate_test.c   : source code (this file)
 * ImageStreamIO.c   : ImageStreamIO source code
 * ImageStreamIO.h   : ImageCreate function prototypes
 * ImageStruct.h     : Image structure definition
 * 
 * EXECUTION:
 * ./a.out  
 * (no argument)
 * 
 * Creates an image imtest00 in shared memory
 * Updates the image every ~ 10ms, forever...
 * A square is rotating around the center of the image
 * 
 */




#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ImageStruct.h"
#include "ImageStreamIO.h"




int main()
{
	IMAGE *imarray;     // pointer to array of images
	int NBIMAGES = 10;  // can hold 10 images
	long naxis;        // number of axis
	uint8_t atype;     // data type
	uint32_t *imsize;  // image size 
	int shared;        // 1 if image in shared memory
	int NBkw;          // number of keywords supported



	

	// allocate memory for array of images
	imarray = (IMAGE*) malloc(sizeof(IMAGE)*NBIMAGES);

	
	// image will be 2D
	naxis = 2;
	
	// image size will be 512 x 512
	imsize = (uint32_t *) malloc(sizeof(uint32_t)*naxis);
	imsize[0] = 512;
	imsize[1] = 512;
	
	// image will be float type
	// see file ImageStruct.h for list of supported types
	atype = _DATATYPE_FLOAT;
	
	// image will be in shared memory
	shared = 1;
	
	// allocate space for 10 keywords
	NBkw = 10;


	
	// create an image in shared memory
	ImageStreamIO_createIm(&imarray[0], "imtest00", naxis, imsize, atype, shared, NBkw);



	float angle; 
	float r;
	float r1;
	long ii, jj;
	float x, y, x0, y0, xc, yc, dx, dy;
	float squarerad=20;
	long dtus = 10000; // update every 10ms
	float dangle = 0.02;
	
	int s;
	int semval;

	// writes a square in image
	// square location rotates around center
	angle = 0.0;
	r = 100.0;
	x0 = 0.5*imarray[0].md[0].size[0];
	y0 = 0.5*imarray[0].md[0].size[1];
	while (1)
	{
		// disk location
		xc = x0 + r*cos(angle);
		yc = y0 + r*sin(angle);
		
		
		imarray[0].md[0].write = 1; // set this flag to 1 when writing data
		
		for(ii=0; ii<imarray[0].md[0].size[0]; ii++)
			for(jj=0; jj<imarray[0].md[0].size[1]; jj++)
			{
				x = 1.0*ii;
				y = 1.0*jj;
				dx = x-xc;
				dy = y-yc;
				
				imarray[0].array.F[jj*imarray[0].md[0].size[0]+ii] = cos(0.03*dx)*cos(0.03*dy)*exp(-1.0e-4*(dx*dx+dy*dy));
				
				//if( (x-xc<squarerad) && (x-xc>-squarerad) && (y-yc<squarerad) && (y-yc>-squarerad))
				//	imarray[0].array.F[jj*imarray[0].md[0].size[0]+ii] = 1.0;
				//else
				//	imarray[0].array.F[jj*imarray[0].md[0].size[0]+ii] = 0.0;
			}
		
		// POST ALL SEMAPHORES
		for(s=0; s<imarray[0].md[0].sem; s++)
        {
            sem_getvalue(imarray[0].semptr[s], &semval);
            if(semval<SEMAPHORE_MAXVAL)
                sem_post(imarray[0].semptr[s]);
        }
		sem_getvalue(imarray[0].semlog, &semval);
        if(semval<SEMAPHORE_MAXVAL)
            sem_post(imarray[0].semlog);
		
		
		imarray[0].md[0].write = 0; // Done writing data
		imarray[0].md[0].cnt0++;
		imarray[0].md[0].cnt1++;
		
		
		usleep(dtus);
		angle += dangle;
		if(angle > 2.0*M_PI)
			angle -= 2.0*M_PI;
		//printf("Wrote square at position xc = %16f  yc = %16f\n", xc, yc);
		//fflush(stdout);
	}
	


	free(imsize);
	free(imarray);
	
	return 0;
}
