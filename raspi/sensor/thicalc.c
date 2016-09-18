/* ------------------------------------------------------------ *
 * file:        thicalc.c                                       *
 * purpose:     Convert temperature (Celsius) and humidity (%)  *
 *              into the "real feel" temperature Humidity index *
 * author:      08/27/2016 Frank4DD                             *
 *                                                              *
 * gcc -o thicalc thicalc.c                                     *
 * ------------------------------------------------------------ */
#include <stdlib.h>
#include <stdio.h>

/* ------------------------------------------------------------ *
 * print_usage() prints the programs commandline instructions.  *
 * ------------------------------------------------------------ */
void print_usage() {
   static char const usage[] = "Usage: thicalc [temperature] [humidity]\n\
   Command line parameters have the following format:\n\
   1.  Temperature in degree Celsius, Example: 28.5\n\
   2.  Relative Humidity in Percent,  Example: 52.1\n\
   Usage example: ./thicalc 28.5 52.1\n";
   printf(usage);
}


int main(int argc, char *argv[]) {
   // calcuation constants
   double c1 = -42.38, c2 = 2.049, c3 = 10.14,
          c4 = -0.2248, c5 = -6.838e-3, c6 = -5.482e-2,
          c7 = 1.228e-3, c8 = 8.528e-4, c9 = -1.99e-6;

   // Check if we got correct # of parameters
   if (argc != 3) { print_usage(); return -1; }

   // Assign parameters to the program variables
   double temp = strtod(argv[1], NULL);  // Temperature in degree Celsius
   double humi = strtod(argv[2], NULL);  // Relative Humidity in Percent

   // convert degree Celsius into Fahrenheit
   double tempF = (temp * 9/5) + 32;

   // mid-calculations
   double A = ((c5 * tempF) + c2) * tempF + c1;
   double B = ((c7 * tempF) + c4) * tempF + c3;
   double C = ((c9 * tempF) + c8) * tempF + c6;

   // final calculation returns Fahrenheit
   double realF = (C * humi + B) * humi + A;

   // convert result back to degree Celsius
   double real = (realF - 32) / 1.8;

   printf("%3.1f", real);
   return(0);
}
