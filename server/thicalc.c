/* ------------------------------------------------------------ *
 * file:        thicalc.c                                       *
 * purpose:     Convert temperature (Celsius) and humidity (%)  *
 *              into the "real feel" temperature Humidity index *
 * author:      08/27/2016 Frank4DD                             *
 *                                                              *
 * gcc -o thicalc thicalc.c -lm                                 *
 * ------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* ------------------------------------------------------------ *
 * print_usage() prints the programs commandline instructions.  *
 * ------------------------------------------------------------ */
void print_usage() {
   static char const usage[] = "Usage: thicalc [temperature] [humidity] [-v]\n\
   Command line parameters have the following format:\n\
   1.  Temperature in degree Celsius, Example: 28.5\n\
   2.  Relative Humidity in Percent,  Example: 52.1\n\
   2.  optional: -v enables debug output for Fahrenheit values\n\
   Usage example: ./thicalc 28.5 52.1\n";
   printf(usage);
}


int main(int argc, char *argv[]) {
   int verbose = 0;
   // calcuation constants
   double c1 = -42.379,   c2 =  2.0490152, c3 =  10.1433312,
          c4 = -0.224755, c5 = -0.0068378, c6 = -0.05481717,
          c7 =  0.001228, c8 =  0.0008528, c9 = -0.00000199;

   // Check if we got correct # of parameters
   if (argc < 3 || argc > 4) { print_usage(); return -1; }

   // Assign parameters to the program variables
   double temp = strtod(argv[1], NULL);  // Temperature in degree Celsius
   double humi = strtod(argv[2], NULL);  // Relative Humidity in Percent
   if(argv[3] && strcmp(argv[3], "-v") == 0) verbose = 1;  // show Fahrenheit values
   // convert degree Celsius into Fahrenheit
   double tempF = (temp * 9/5) + 32;

   if(verbose == 1) printf("IN Temperature Celsius: %3.1f\tFahrenheit: %3.1f\n",temp, tempF);
   // Formula: https://en.wikipedia.org/wiki/Heat_index
   // HI = c1 + c2*T + c3*R + c4*T*R + c5*T^2 + c6*R^2 + c7*T^2*R + c8*T*R^2 + c9*T^2*R^2
   double realF = c1 + (c2 * tempF) + (c3 * humi) + 
                 (c4 * tempF * humi) + (c5 * tempF * tempF) +
                 (c6 * humi * humi) + (c7 * tempF * tempF * humi) +
                 (c8 * tempF * humi * humi) + (c9 * tempF * tempF * humi * humi);
   // ------------------------------------------------------------------------------
   // per http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
   // two adjustments are necessary to account for extreme high and low humidity
   // when Rothfusz regression is used. (Rothfusz regression is not valid for
   // temperature and relative humidity beyond the range of data considered by Steadman.
   // ------------------------------------------------------------------------------
   // adjustment for very low humidity: ADJ = [(13-RH)/4]*SQRT{[17-ABS(T-95.)]/17}
   if(humi < 13 && tempF < 110) {
      double adj  = ((13-humi)/4) * sqrt((17-fabs(tempF-95))/17);
      realF = realF - adj;
   }
   // adjustment for very high humidity: ADJ = [(RH-85)/10] * [(87-T)/5]
   if(humi > 85 && tempF < 87) {
      double adj  = ((humi-85)/10) * ((87-tempF)/5);
      realF = realF + adj;
   }

   // convert result back to degree Celsius
   double real = (realF - 32) / 1.8;

   if(verbose == 1)  printf("HI Temperature Celsius: %3.1f\tFahrenheit: %3.1f\n", real, realF);
   else printf("%3.1f", real);
   return(0);
}
