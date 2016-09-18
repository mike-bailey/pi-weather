/* ------------------------------------------------------ *
 * switchd:                                               *
 * When push button-1 is pushed, it checks if the program *
 * runs. If yes, its is stopped, and led-1 is turned off. *
 * If no, the program is started, and led-1 lights up.    *
 *                                                        *
 * When push button-2 is pushed less than 2 secs, the     *
 * daemon executes a system shutdown. If button-2 is held *  
 * down for 2 secs or longer, theystem is rebooted.       *
 *                                                        *
 * gcc switchd.c -o switchd -l wiringPi                   *
 * ------------------------------------------------------ */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>
#include <wiringPi.h>

// We want a success/failure return value from 'wiringPiSetup()'
#define WIRINGPI_CODES		1
#define DAEMON_NAME		"switchd"
#define PID_FILE		"/var/run/" DAEMON_NAME ".pid"

#define SWI1			0	/* wiringPi pin 0 = BCM_GPIO 17, physical pin 11 */
#define SWI2			4	/* wiringPi pin 4 = BCM_GPIO 23, physical pin 16 */
#define SWI1_STR		"0"
#define SWI2_STR		"0"

#define LED1			2	/* wiringPi pin 2 = BCM_GPIO 21, physical pin 13 */
#define LED2			3	/* wiringPi pin 3 = BCM_GPIO 22, physical pin 15 */
#define LED1_STR		"0"
#define LED2_STR		"0"

/* Function prototypes */
void Daemon_Stop(int signum);
void Swi1_Pressed(void);
void Swi2_Pressed(void);

int main(int argc, char *argv[]) {

   /* Logging */
   setlogmask(LOG_UPTO(LOG_INFO));
   openlog(DAEMON_NAME, LOG_CONS | LOG_PERROR, LOG_USER);
   syslog(LOG_INFO, "Daemon starting up");

   /* This daemon can only run by root. */
   if (geteuid() != 0) {
      char  *err_str = "This daemon can only run by root, exiting";
      printf(err_str);
      syslog(LOG_ERR, err_str);
      exit(EXIT_FAILURE);
   }

   /* Make sure the file '/usr/bin/gpio' exists */
   struct stat filestat;
   if (stat("/usr/bin/gpio", &filestat) == -1) {
      char  *err_str = "The program '/usr/bin/gpio' is missing, exiting";
      printf(err_str);
      syslog(LOG_ERR, err_str);
      exit(EXIT_FAILURE);
   }

   /* ---------------------------------------------------------------- *
    * Create the daemon PID file. We update the pid file access rights *
    * to let all users see the status without sudo, using the command  *
    * 'service switchd status'. The correct file permission are        *
    * Read/Write for owner, Read for group and others (644).           *
    * ---------------------------------------------------------------- */
   const int PIDFILE_PERMISSION = 0644;
   int pidFilehandle = open(PID_FILE, O_RDWR | O_CREAT, PIDFILE_PERMISSION);

   if (pidFilehandle == -1) {
      char  *err_str = "Could not open PID lock file";
      printf("%s %s, exiting", err_str, PID_FILE);
      syslog(LOG_ERR, "%s %s, exiting", err_str, PID_FILE);
      exit(EXIT_FAILURE);
   }

   /* Try to lock PID file */
   if (lockf(pidFilehandle, F_TLOCK, 0) == -1) {
      char  *err_str = "Could not lock PID lock file";
      printf("%s %s, exiting", err_str, PID_FILE);
      syslog(LOG_ERR, "%s %s, exiting", err_str, PID_FILE);
      exit(EXIT_FAILURE);
   }

   /* Our process ID and Session ID */
   pid_t pid, sid;

   /* Fork off the parent process */
   pid = fork();

   /* If we got a good PID, then we can exit the parent process. */
   if (pid < 0) exit(EXIT_FAILURE);
   if (pid > 0) exit(EXIT_SUCCESS);

   /* Get and format PID */
   char szPID[16];
   sprintf(szPID, "%d\n", getpid());

   /* write pid to lockfile */
   write(pidFilehandle, szPID, strlen(szPID));

   /* Change the file mode mask */
   umask(0);

   /* Create a new SID for the child process */
   sid = setsid();
   if (sid < 0) exit(EXIT_FAILURE);

   /* Change the current working directory */
   if (chdir("/") < 0) exit(EXIT_FAILURE);

   /* Close out the standard file descriptors */
   close(STDIN_FILENO);
   close(STDOUT_FILENO);
   close(STDERR_FILENO);

   /* Add a process termination handler for handling daemon stop requests */
   signal(SIGTERM, &Daemon_Stop);

   /* Initialize the 'wiringPi' library */
   if (wiringPiSetup() == -1) {
      char *err_str = "'wiringPi' library couldn't be initialized, exiting";
      printf(err_str);
      syslog(LOG_ERR, err_str);
      exit(EXIT_FAILURE);
   }

   /* Define the PIN data direction mode */
   pinMode(SWI1, INPUT);
   pinMode(SWI2, INPUT);
   pullUpDnControl(SWI1, PUD_UP);
   pullUpDnControl(SWI2, PUD_UP);

   pinMode(LED1, OUTPUT);
   pinMode(LED2, OUTPUT);
   digitalWrite (LED1, LOW);
   digitalWrite (LED2, LOW);

   /* Setup pin mode and interrupt handler */
   if (wiringPiISR(SWI1, INT_EDGE_RISING, &Swi1_Pressed) == -1) {
      char *err_str = "Unable to set interrupt handler for specified switch pins, exiting";
      printf(err_str);
      syslog(LOG_ERR, err_str);
      exit(EXIT_FAILURE);
   }

   /* run in a continous loop until there's an interrupt or system shutdown */
   system("/srv/scripts/lcdinit");
   system("/srv/scripts/lcdwrite 1 'switchd:'");
   system("/srv/scripts/lcdwrite 2 'start OK'");
   while (TRUE) sleep(20);
}

void Daemon_Stop(int signum) {
   /* 'SIGTERM' was issued, system is telling this daemon to stop */
   system("/srv/scripts/lcdinit");
   system("/srv/scripts/lcdwrite 1 'switchd:'");
   system("/srv/scripts/lcdwrite 2 'stop OK'");
   syslog(LOG_INFO, "Stopping daemon");
   exit(EXIT_SUCCESS);
}

/* Handle button pressed interrupts */
void Swi1_Pressed(void) {
   switch (digitalRead(SWI1)) {
      case LOW:
         if(digitalRead(LED1) == LOW) {
            digitalWrite (LED1, HIGH);
            system("/srv/scripts/lcdinit");
            system("/srv/scripts/lcdwrite 1 'switch1:'");
            system("/srv/scripts/lcdwrite 2 'BTCstart'");
            system("/etc/init.d/mining start");
         }
         else {
            digitalWrite (LED1, LOW);
            system("/srv/scripts/lcdinit");
            system("/srv/scripts/lcdwrite 1 'switch1:'");
            system("/srv/scripts/lcdwrite 2 'BTCstop'");
            system("/etc/init.d/mining stop");
         }
   }
}

void Swi2_Pressed(void) {

   /* Disable further interrupts. NOTE: 'wiringPi' library doesn't
      support unhooking an existing interrupt handler, so we need
      to use the 'gpio' binary to do this according to the author */
   system("/usr/bin/gpio edge " SWI2_STR " none");

   /* check if user pressed the button over 2 secs */
   sleep(2);

   switch (digitalRead(SWI2)) {
      case LOW:         // Shutdown requested
         digitalWrite (LED2, HIGH);
         system("/srv/scripts/lcdinit");
         system("/srv/scripts/lcdwrite 1 'switch2:'");
         system("/srv/scripts/lcdwrite 2 'shutdown'");
         syslog(LOG_INFO, "Shutting down system");
         sleep(2);
         if (execl("/sbin/poweroff", "poweroff", NULL) == -1)
         syslog(LOG_ERR, "'poweroff' program failed to run with error: %d", errno);
         // NOTE: Execution will not reach here if 'execl()' succeeds
      break;

      case HIGH:        // Restart requested
         digitalWrite (LED2, HIGH);
         system("/srv/scripts/lcdinit");
         system("/srv/scripts/lcdwrite 1 'switch2:'");
         system("/srv/scripts/lcdwrite 2 'reboot'");
         syslog(LOG_INFO, "Restarting system");
         sleep(2);
         if (execl("/sbin/shutdown", "shutdown", "-r", "now", NULL) == -1)
         syslog(LOG_ERR, "'shutdown' program failed to run with error: %d", errno);
         // NOTE: Execution will not reach here if 'execl()' succeeds
   }
   exit(EXIT_SUCCESS);
}
