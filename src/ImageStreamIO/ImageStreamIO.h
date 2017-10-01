/**
 * @file    ImageCreate.h
 * @brief   Function prototypes for ImageCreate
 * 
 *  
 * @author  O. Guyon
 * @date    12 Jul 2017
 *
 * 
 * @bug No known bugs.
 * 
 */


#ifndef _IMAGESTREAMIO_H
#define _IMAGESTREAMIO_H
 

int_fast8_t init_ImageStreamIO();


int ImageStreamIO_createSem(IMAGE *image, long NBsem);


int ImageStreamIO_createIm(IMAGE *image, const char *name, long naxis, uint32_t *size, uint8_t atype, int shared, int NBkw);


long ImageStreamIO_read_sharedmem_image_toIMAGE(const char *name, IMAGE *image);


#endif


