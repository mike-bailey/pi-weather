/* ------------------------------------------------------------------------- *
 * wcam-img-archive.c v1.1 20160904 Frank4DD [fm4dd.com]                     *
 *                                                                           *
 * This program archives the incoming webcam images for movie processing.    *
 * It creates a hard link to the original file with a new name created from  *
 * the file creation date & sorts them into a archive directory structure.   *
 * It runs via cron every minute between 6:00 and 22:00 o'clock.             *
 *                                                                           *
 * /etc/crontab entry: * * * * * pi /home/bin/wcam-img-archive               *
 * compilation: gcc wcam-img-archive.c -o wcam-img-archive                   *
 *                                                                           *
 * v1.0 20050307 initial release Frank4DD [www.frank4dd.com]                 *
 * v1.1 20160904 restrict function hard-coded with start and end times       *
 * ------------------------------------------------------------------------- */

#define WCAM_FILE "/tmp/raspicam.jpg"
#define ARCH_HOME "/home/pi/camera/wcam-arch"
#define START_HR  6
#define END_HR    21

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int main() {

  time_t wcam_tstamp;
  struct stat wcam_stat;
  struct stat arch_stat;
  char wcam_arch_dir[255];
  char wcam_arch_name[26];
  char new_file[255];
  mode_t mode;
  char year[5];
  char month[3];
  char day[3];
  char hourstr[3];
  int hour;

  /* Check if our source file exists */
  if (stat(WCAM_FILE, &wcam_stat) == -1) exit(0);

  /* Get the current Unix time */
  wcam_tstamp = wcam_stat.st_mtime;

  /* Check if our collection time is between START_HR and END_HR */
  strftime(hourstr, sizeof(hourstr), "%H", localtime(&wcam_tstamp));
  hour = atoi(hourstr);

  // exit if outside operational hours
  if(hour < START_HR || hour > (END_HR-1)) exit(0);

  /* Create the archive file name, format <wcam-yyyymmdd_hhmmss.jpg> */
  strftime(wcam_arch_name, sizeof(wcam_arch_name), "wcam-%Y%m%d_%H%M%S.jpg",
                                                  localtime(&wcam_tstamp));
  /* Create the archive directory structure <base><year><month><day>
     if not already there */

  mode= 0777 & ~umask(0);

  strftime(year, sizeof(year), "%Y", localtime(&wcam_tstamp));
  strftime(month, sizeof(month), "%m", localtime(&wcam_tstamp));
  strftime(day, sizeof(day), "%d", localtime(&wcam_tstamp));

  snprintf(wcam_arch_dir, sizeof(wcam_arch_dir), "%s/%s", ARCH_HOME, year);
  if(stat(wcam_arch_dir, &arch_stat) == -1) mkdir(wcam_arch_dir, mode);

  snprintf(wcam_arch_dir, sizeof(wcam_arch_dir), "%s/%s/%s",
                                                       ARCH_HOME, year, month);
  if(stat(wcam_arch_dir, &arch_stat) == -1) mkdir(wcam_arch_dir, mode);

  snprintf(wcam_arch_dir, sizeof(wcam_arch_dir), "%s/%s/%s/%s",
                                                  ARCH_HOME, year, month, day);
  if(stat(wcam_arch_dir, &arch_stat) == -1) mkdir(wcam_arch_dir, mode);

/* create a hard link so we have a new file without doing the actual copy
 * (and having the resulting i/o). */

  snprintf(new_file, sizeof(new_file), "%s/%s", wcam_arch_dir, wcam_arch_name);
  link(WCAM_FILE, new_file);
  exit(0);
}
