/* ----------------------------------------------------------------------- *
 * wcam-make-movie.c v1.1 20160911 Frank4DD [fm4dd.com]                    *
 *                                                                         *
 * This program goes through the archived and timestamped .jpg images of a *
 * day, generates the frameXXX.jpg sequence files through hard links and   *
 * calls ffmpeg for the conversion to a .mpg movie. Finally, it cleans the *
 * frameXXX.jpg files up. wcam-make-movie runs once a day after midnight.  * 
 * crontab entry: 30 0 * * * /home/bin/wcam-make-movie                     *
 *                                                                         *
 * compilation: gcc wcam-make-movie.c -o wcam-make-movie                   *
 *                                                                         *
 * Requires: ffmpeg library, and imagemagick for mogrify (time imprint)    *
 *                                                                         *
 * v1.0 20050307 Frank4DD	[www.frank4dd.com]                         *
 * ----------------------------------------------------------------------- */
#define PROG_NAME "wcam-make-movie"
#define ARCH_HOME "/home/pi/camera/wcam-arch"
#define FFMPEGBIN "/usr/bin/ffmpeg"
#define MOVIE_DST "/tmp/yesterday.mpg"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>

/* --------------------------------------------------------------- *
 * global variables                                                *
 * --------------------------------------------------------------- */
static char const usage[] = "\
Usage: " PROG_NAME " [OPTIONS]\n\
Options:\n\
  -a   the path to the archived images (default: " ARCH_HOME ")\n\
  -f   the path to the ffmpeg program  (default: " FFMPEGBIN ")\n\
  -d   the day to create the movie for (default: yesterday)\n\
  -o   the path and name for 2nd movie (default: " MOVIE_DST ")\n\
  -h   print program usage and exit\n\
  -v   enable debug output\n\
";

static char const short_options[] = "a:f:d:o:h:v";

static char *archi_home = ARCH_HOME;     // the archived images home directory
static char *ffmpeg_bin = FFMPEGBIN;     // the path to the ffmpeg program
static char target_day[11];              // the target day to process (yyyy-mm-dd\0)
static char *movie_file = "";            // the generated movie file
int debug = -1;                          // debug output, default "off"

/* --------------------------------------------------------------- *
 * function parse_arg()                                            *
 * --------------------------------------------------------------- */
static void parse_arg(int key, char *arg, char *pname) {
   switch(key) {
      case 'a':
         free(archi_home);
         archi_home = strdup(arg);
         break;
      case 'f':
         free(ffmpeg_bin);
         ffmpeg_bin = strdup(arg);
         break;
      case 'd':
         if(strlen(arg) == 10) strncpy(target_day, arg, 10);
         else {
            fprintf(stderr, "%s: target_day wrong length %d.\n", pname, strlen(arg));
            printf(usage);
            exit(0);
         }
         printf("arg length: %d target_day: %s t length: %d\n", strlen(arg), target_day, strlen(target_day));
         break;
      case 'o':
         free(movie_file);
         if(strlen(arg) > 0) movie_file = strdup(arg);
         else movie_file = strdup(MOVIE_DST);
         break;
      case 'h':
         printf(usage);
         exit(0);
         break;
      case 'v':
         debug = 1;
         break;
      default:
         printf(usage);
         exit(0);
   }
}

/* ------------------------------------------------------------- *
 * scandir filter returns 1/true if name of file ends in .jpg    *
 * ------------------------------------------------------------- */
int filter(const struct dirent *entry) {
   const char *s = entry->d_name;
   if ((strcmp(s, ".") == 0) || (strcmp(s, "..") == 0)) return(0);
   int len = strlen(s) - 4; // index of start of . in .jpg
   if(len >= 0) { 
      if (strncmp(s + len, ".jpg", 4) == 0) { return(1); }
   }
   return(0);
}

/* ------------------------------------------------------------- *
 * function delete_tmp cleans up the jpg image files copied here *
 * ------------------------------------------------------------- */
int delete_tmp(char *dir) {
   struct dirent *tmpfile_list;
   DIR *tmp_dir;
   char file[255];
   int i=0;

   tmp_dir = opendir(dir);
   for(;;) {
      tmpfile_list = readdir(tmp_dir);
      if(tmpfile_list == NULL) break;
      if(strstr(tmpfile_list->d_name, ".jpg")) {
         snprintf(file, sizeof(file), "%s/%s", dir, tmpfile_list->d_name);
         unlink(file);
         i++;
      }
   }
   free(tmpfile_list);
   return(i);
}

/* ------------------------------------------------------------- *
 * function origin_zip puts the camera jpg image files in a .zip *
 * e.g. # zip -q -m wcam-2016909-jpg *.jpg                       *
 * ------------------------------------------------------------- */
int origin_zip(char *dir) {
   char cmd[255];
   snprintf(cmd, sizeof(cmd), "/usr/bin/zip -q -m %s/wcam-%s %s/*.jpg", dir, target_day, dir);
   if(debug) printf("cmd: %s\n", cmd);
   system(cmd);
}


int main(int argc, char *argv[]) {
/* ------------------------------------------------------------- *
 * get current time                                              *
 * ------------------------------------------------------------- */
   time_t tstamp = time(NULL);

/* ------------------------------------------------------------- *
 * process commandline arguments                                 *
 * ------------------------------------------------------------- */
   int key;
   while (1) {
      key = getopt(argc, argv, short_options);
      if (key < 0) break;
      parse_arg(key, optarg, argv[0]);
   }
   if(debug) printf("%s run started on %s\n", PROG_NAME, ctime(&tstamp));

   // if target_day was not given, set target_day to default yesterday
   if(strlen(target_day) == 0) { 
      tstamp = tstamp - 86400;                    // calculate yesterday 
      strftime(target_day, 11, "%Y-%m-%d", localtime(&tstamp));
      if(debug) printf("target_day: %s (default)\n", target_day);
   } else {
      if(debug) printf("target_day: %s\n", target_day);
   }

   if(debug) {
      printf("archi_home: %s\n", archi_home);
      printf("ffmpeg_bin: %s\n", ffmpeg_bin);
      if(strlen(movie_file)>0) printf("movie_file: %s\n", movie_file);
   }

/* ------------------------------------------------------------- *
 * we expect the following archive directory structure:          *
 * archi_home/<year>/<month>/<day>                               *
 * ------------------------------------------------------------- */
   int i;
   char buf_day[11];
   char srcimg_dir[255];

   /* create a tmp day string, replacing '-' with '/' */
   for(i=0; i<=strlen(target_day); i++) {
      if(target_day[i] == '-') buf_day[i] = '/';
      else buf_day[i] = target_day[i];
   }
   buf_day[i+1] = '\0';    // terminate the tmp day string

   snprintf(srcimg_dir, sizeof(srcimg_dir), "%s/%s", archi_home, buf_day);
   if(debug) printf("srcimg_dir: %s\n", srcimg_dir);

/* ------------------------------------------------------------- *
 * Check if the image source directory exists                    *
 * ------------------------------------------------------------- */
   struct stat srcimg_stat;
   if(stat(srcimg_dir, &srcimg_stat) == -1) {
      printf("Error: srcimg_dir: %s does not exist\n", srcimg_dir);
      exit(-1);
   }

/* ------------------------------------------------------------- *
 * Check if the movie destination directory exists               *
 * ------------------------------------------------------------- */
   if(strlen(movie_file)>0) {
      struct stat dstmov_stat;
      char *dstmov_name = strrchr(movie_file, '/') + 1;
      int len = dstmov_name - movie_file - 1;
      if(debug) printf("dstmov_dir len: %d\n", len);

      char dstmov_dir[len];
      for(i=0; i<len; i++) {
         dstmov_dir[i] = movie_file[i];
      }
      dstmov_dir[i] = '\0';
      if(debug) printf("dstmov_dir: %s\n", dstmov_dir);

      if(stat(dstmov_dir, &dstmov_stat) == -1) {
         printf("Error: dstmov_dir: %s does not exist\n", dstmov_dir);
         exit(-1);
      }
   }

/* ------------------------------------------------------------- *
 * Try to access the source images files                         *
 * ------------------------------------------------------------- */
   DIR *src_dir;
   src_dir = opendir(srcimg_dir);
   if (src_dir == NULL) {
      printf("Error: srcimg_dir: %s cannot open\n", srcimg_dir);
      exit(-1);
   }

/* ------------------------------------------------------------- *
 * Get the list of .jpg files and sort them by time              *
 * ------------------------------------------------------------- */
   int file_counter=0;
   struct dirent **imgfile_list;

   file_counter = scandir(srcimg_dir, &imgfile_list, filter, alphasort);
      if(debug) printf("srcimg_dir: %s - %d files found.\n", srcimg_dir, file_counter);
   if(file_counter<=0) {
      printf("Error: srcimg_dir: %s - %d files found\n", srcimg_dir, file_counter);
      exit(-1);
   }

/* ------------------------------------------------------------- *
 * Check for temporary work directory                            *
 * ------------------------------------------------------------- */
   char tmpimg_dir[strlen(archi_home)+strlen("/tmp")+1];
   snprintf(tmpimg_dir, sizeof(tmpimg_dir), "%s/tmp", archi_home);
   if(debug) printf("tmpimg_dir: %s\n", tmpimg_dir);

   src_dir = opendir(tmpimg_dir);
   if (src_dir == NULL) {
      printf("Error: tmpimg_dir: %s cannot open\n", tmpimg_dir);
      exit(-1);
   }

/* ------------------------------------------------------------- *
 * create image work copies in temporary work directory          *
 * ------------------------------------------------------------- */
   char arch_file[255];  // e.g. /home/pi/camera/wcam-arch/2016/09/10/wcam-20160910_181907.jpg
   char new_file[255];   // e.g. /home/pi/camera/wcam-arch/tmp/frame739.jpg
   char system_cmd[255];

   if(debug) printf("Copying %d srcimg files -> %s\n", file_counter, tmpimg_dir);

   for(i=0; i<file_counter; i++) {
      snprintf(new_file, sizeof(new_file), "%s/frame%03d.jpg", tmpimg_dir, i);
      snprintf(arch_file, sizeof(arch_file), "%s/%s", srcimg_dir, imgfile_list[i]->d_name);

      snprintf(system_cmd, sizeof(system_cmd), "/bin/cp %s %s", arch_file, new_file);
      if(debug) printf("system_cmd: %s\n", system_cmd);
      system(system_cmd);

/* ------------------------------------------------------------- *
 * add date and time imprint to images, example:                 *
 * mogrify -font Liberation-Sans -fill white -undercolor \       *
 * '#00000080' -pointsize 26 -gravity SouthEast -annotate \      *
 * +10+10 "test" frame001.jpg                                    *
 *  ------------------------------------------------------------- */
      char time_hr[3];
      int hlen = strlen(arch_file)-10;
      strncpy(time_hr, arch_file+hlen, 2);
      time_hr[2] = '\0';
      char time_mn[3];
      int mlen = strlen(arch_file)-8;
      strncpy(time_mn, arch_file+mlen, 2);
      time_mn[2] = '\0';
      char imprint[255];
      snprintf(imprint, sizeof(imprint), "FM4DD Raspi-Cam %s Time: %s:%s", target_day, time_hr, time_mn);
      //if(debug) printf("imprint str: %s\n", imprint);
      snprintf(system_cmd, sizeof(system_cmd), "/usr/bin/mogrify -fill white -undercolor '#00000080' -pointsize 20 -gravity SouthEast -annotate +20+20 '%s' %s", imprint, new_file);
      //if(debug) printf("system_cmd: %s\n", system_cmd);
      system(system_cmd);
   }
   free(imgfile_list);

/* ------------------------------------------------------------- *
 * generate the movie file                                       *
 * ------------------------------------------------------------- */
   char mov_file[255];
   snprintf(mov_file, sizeof(mov_file), "%s/wcam-%s.mpg", srcimg_dir, target_day);
   if(debug) printf("create movie_file 1: %s\n", mov_file);

   snprintf(system_cmd, sizeof(system_cmd), "%s -i %s/frame%%03d.jpg %s >/dev/null 2>&1", ffmpeg_bin, tmpimg_dir, mov_file);

   /* add the silencer for non-debug output */
   if(! debug) snprintf(system_cmd, sizeof(system_cmd), "%s >/dev/null 2>&1", system_cmd);

   if(debug) printf("system_cmd: %s\n", system_cmd);
   system(system_cmd);

   /* create copy if option -o was given, e.g. /tmp/yesterday.mpg */
   if(strlen(movie_file)>0) {
      link(mov_file, movie_file);
      if(debug) printf("created movie_file 2: %s\n", movie_file);
   }

/* ------------------------------------------------------------- *
 * clean up the temporary frames                                 *
 * ------------------------------------------------------------- */
   int deleted = 0;
   deleted = delete_tmp(tmpimg_dir);
   if(debug) printf("Deleted: %d jpeg files in %s\n", deleted, tmpimg_dir);

/* ------------------------------------------------------------- *
 * zip up the original jpeg files per day                        *
 * ------------------------------------------------------------- */
   origin_zip(srcimg_dir);
   exit(0);
}
