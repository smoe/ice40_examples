// Example follows StackOverflow.com
// http://stackoverflow.com/questions/18108932/linux-c-serial-port-reading-writing
//
// Modified by Steffen Moeller, 2016, to work with the iCE40 HX1K

#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

static const unsigned char cmd[] = "INIT \r";
static char response[2048];

struct input {
   char a;
   char b;
} input;

struct output {
   char a;
   char b;
   char aplusb;
} output;

char* const write_n_and_read_m (const int device,
                       char const * const writeme, const int n,
                       char       * const readtome,const int m) {

   int n_written = 0,
       spot_w = 0;

   int n_read = 0,
       spot_r = 0;
   char buf = '\0';


   // Write:

   while( (n>0 && n>spot_w) || (0==n && strlen(writeme)>n_written)) {
           n_written = write( device, writeme+spot_w, 1);
           spot_w += n_written;
           fprintf(stderr,"Written: %d, n=%d, n_written=%d, spot_w=%d\n", *(writeme+spot_w), n, n_written, spot_w);
   }

   fprintf(stderr,"Now reading: n=%d, n_written=%d, spot_w=%d\n", n, n_written, spot_w);
   fprintf(stderr,"             m=%d, m_written=%d, spot_r=%d\n", n, n_read,    spot_r);

   // Read:

   do {
       n_read = read( device, readtome+n_read, 1 );
       spot_r += n_read;
       fprintf(stderr,"Read character! m=%d, n_read=%d, spot_r=%d\n",m,n_read,spot_r);
       
       if (n_read < 0) {
           fprintf(stderr,"Error %d reading: %s\n", errno, strerror(errno));
       }
       else if (n_read == 0) {
           fprintf(stderr,"Read nothing!\n");
       }
   } while ( (m>0 && spot_r<m) || (0==m && writeme[spot_w] != 0));

   if (0==m) readtome[spot_r]=0;
   return(readtome);
}


char main(int argc, char *argv[]) {

   if (argc != 4 || 0==strcmp("-h",argv[1]) || 0==strcmp("--help",argv[1])) {
      printf("Usage: %s <device> a b\n",argv[0]);
      exit(0);
   }


   // open the device expected to be specified in first argument

   const int USB = open( argv[1], O_RDWR| O_NOCTTY );

   if (USB<0) {
      fprintf(stderr,"Error %d opening %s : %s\n", errno, argv[1], strerror (errno));
      exit(errno);
   }

   struct termios tty;
   memset (&tty, 0, sizeof tty);

   /* Error Handling */
   if ( tcgetattr ( USB, &tty ) != 0 ) {
      fprintf(stderr, "Error %d from tcgetattr: %s\n", errno, strerror(errno));
      exit(errno);
   }

   /* Set Baud Rate */
   int ospeed = cfsetospeed (&tty, (speed_t)B9600);
   int ispeed = cfsetispeed (&tty, (speed_t)B9600);
   // This is seemingly a bug in the man page - those routines return 0, no the speed
   //fprintf(stderr,"Set speeds\n  input:  %d\n  output: %d\n",ispeed,ospeed);

   /* Setting other Port Stuff */
   tty.c_cflag     &=  ~PARENB;            // Make 8n1
   tty.c_cflag     &=  ~CSTOPB;
   tty.c_cflag     &=  ~CSIZE;
   tty.c_cflag     |=  CS8;

   tty.c_cflag     &=  ~CRTSCTS;           // no flow control
   tty.c_cc[VMIN]   =  1;                  // read doesn't block
   tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
   tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

   /* Make raw */
   cfmakeraw(&tty);

   /* Flush Port, then applies attributes */
   if (0 != tcflush( USB, TCIFLUSH )) {
      fprintf(stderr, "Error %d from tcflush: %s\n", errno, strerror(errno));
      exit(errno);
   }

   if ( tcsetattr ( USB, TCSANOW, &tty ) != 0) {
      fprintf(stderr, "Error %d from tcsetattr: %s\n", errno, strerror(errno));
      exit(errno);
   }

   input.a=atoi(argv[2]);
   input.b=atoi(argv[3]);


   fprintf(stderr,"I: sending A=%d and B=%d to the device.\n",input.a, input.b);
   write_n_and_read_m(USB,(char *) &input,  sizeof (struct input),
                          (char *) &output, sizeof (struct output));
   printf("Input  : A=%d, B=%d, A+B=%d\n",input.a,input.b,input.a+input.b);
   printf("Output : A=%d, B=%d, A+B=%d\n",output.a,output.b,output.aplusb);

   close(USB);

}

