/**
 * @file    statistic.h
 * @brief   Function prototypes for statistic module
 * 
 * Functions provide statistical tools
 *  
 * @author  O. Guyon
 * @date    19 Jun 2017
 *
 * 
 * @bug No known bugs.
 * 
 * @see https://github.com/oguyon/Cfits
 */



#ifndef _STATISTIC_H
#define _STATISTIC_H


int_fast8_t init_statistic();



/** @brief Uniform distribution from 0 to 1
 */
double ran1();



/** @brief Normal distribution, mean=0, sigma=1
 */
double gauss();



/** @brief truncated (-1/+1) sigma = 1 mean = 0 gaussian probability 
 */
double gauss_trc();


/** @brief Poisson distribution
 * 
 * @param mu   Distribution mean
 */
long poisson(double mu);



/** @brief Gamma function
 */
double gammaln(double xx);


double better_poisson(double mu);

double fast_poisson(double mu);



/** @brief Apply Poisson noise to image
 */
long put_poisson_noise(const char *ID_in_name, const char *ID_out_name);


/** @brief Apply Gaussian noise to image
 */
long put_gauss_noise(const char *ID_in_name, const char *ID_out_name, double ampl);



#endif
