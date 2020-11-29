#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "os_defs.h"
#ifdef OS_WINDOWS
# include <windows.h>
#else
# include <errno.h>
# define GetLastError errno
#endif


int lockDemo(char* filename);
static void printHelpAndExit(char *progname, char *errormessage);
static char * curtimeString();
int sysMainLine(int argc, char *argv[]);

int main(int argc, char *argv[]) {



  if (!argc) {
    printHelpAndExit(argv[0], "test");
    curtimeString();
  }
  return 0;
}

static void printHelpAndExit(char *progname, char *errormessage)
{
	if (errormessage != NULL) {
		fprintf(stderr, "Error: %s\n\n", errormessage);
	}

	fprintf(stderr, "%s <datafile> [ <commandfile> ]\n", progname);
	fprintf(stderr, "    Commands come in the form:\n");
	fprintf(stderr, "        OPN OFF LEN\n");
	fprintf(stderr, "    where:\n");
	fprintf(stderr, "        OPN (operation) is one of:\n");
	fprintf(stderr, "           X : exclusive lock of region, waiting for availbility \n");
	fprintf(stderr, "           S : shared lock of region, waiting for availbility \n");
#ifndef  OS_WINDOWS
	fprintf(stderr, "           T : test whether region is locked\n");
#endif
	fprintf(stderr, "           U : unlock region\n");
	fprintf(stderr, "        OFF, and LEN are the offset and length in bytes\n");
	fprintf(stderr, "    If <commandfile> is not given, stdin is read\n");

	exit (-1);
}

static char * curtimeString() {
  time_t curtime;
  curtime = time(NULL);
  return ctime(&curtime);
}

// System mainline -- simply process the arguments and input lines
int sysMainLine(int argc, char *argv[]) {
  /** use O/S specific type to track the right kind of file handle */
  #ifdef OS_WINDOWS
    LockFileHandle lockFileHandle;
  #else
    void* lockFileHandle;
  #endif
  char commandLine[BUFSIZ];
  FILE *commandFP = stdin;
  int nItemsRead, keepGoing = 1;
  long offset, length;
  char operation;


  /** check that the arguments are correct */
  if (argc != 2 && argc != 3) {
		printHelpAndExit(argv[0], "incorrect arguments");
	}


  // if we need to open a data file, we can do so with fopen() so that we can use fgets()
  if (argc == 3) {
    if ((commandFP = fopen(argv[2], "r")) == NULL) {
      fprintf(stderr, "Cannot open command file '%s' : %s\n", argv[2], strerror(errno));
      exit (-1);
    }
  }

    
  /** Windows specific file open */
  #ifdef OS_WINDOWS
  lockFileHandle = CreateFile(argv[1],
    GENERIC_READ | GENERIC_WRITE,       // dwDesiredAccess
    FILE_SHARE_READ | FILE_SHARE_WRITE, //  dwShareMode
    NULL,           // lpSecurityAttributes
    OPEN_EXISTING,  // dwCreationDisposition
    FILE_ATTRIBUTE_NORMAL,  // dwFlagsAndAtributes
    NULL);  // hTemplateFile
  #endif

  if (lockFileHandle == NULL) {
    fprintf(stderr, "Failed opening '%s' : %d\n", argv[1], GetLastError());
    exit (-1);
  }

  /**
   * Print out the help overview on all O/S (but there is no test function
   * on Windows)
   */
  printf("Processing locks on '%s'\n", argv[1]);
  printf("Available commands:\n");
  printf("    X <start> <len>   : exclusive lock of <len> bytes from <start>\n");
  printf("    S <start> <len>   : shared lock of <len> bytes from <start>\n");
#ifndef OS_WINDOWS
  printf("    T <start> <len>   : test of lock of <len> bytes from <start>\n");
#endif
  printf("    U <start> <len>   : unlock <len> bytes from <start>\n");
  printf("    Q                 : quit the program\n");


	/** print first prompt */
	if (isatty(0))
			printf("CMD> ") >= 0 && fflush(stdout);

	// loop reading commands we use fgets() to ensure that we are reading whole lines
	while (keepGoing && fgets(commandLine, BUFSIZ, commandFP) != NULL) {
		// scanf can do this sort of processing pretty well
		nItemsRead = sscanf(commandLine, "%c %ld %ld", &operation, &offset, &length);

    /** if we get some flavour of Q, we are bailing out */
    if (toupper(operation) != 'Q') {

      /** make sure that we have enough data */
      if (nItemsRead < 3 || strchr(VALID_COMMANDS, toupper(operation)) == NULL)
        fprintf(stderr, "Invalid operation - need \"OPN OFF LEN\"\n");
      else {
        printf("CMD %c %2ld %2ld\n",toupper(operation),offset, length);

        /** we ignore the results of the lock action here */
        (void) doLockAction(lockFileHandle, toupper(operation), offset, length);
      }

      /* prompt again */
      if (isatty(0)) {
        printf("CMD> ") >= 0 && fflush(stdout);
			}

    } else {
      printf("Exitting ...\n");
      keepGoing = 0;
    }
  }

  /** clean up our files */
  CloseHandle(lockFileHandle);

  if (commandFP != stdin) {
		fclose(commandFP); 
	}
	exit(0);
}


int lockDemo(char* filename) {
/* l_type   l_whence  l_start  l_len  l_pid   */
  struct flock fl = { F_WRLCK, SEEK_SET, 0,       0,     0 };
  int fd;

  fl.l_pid = getpid();

  // if (argc > 1) 
  //   fl.l_type = F_RDLCK;

  if ((fd = open(filename, O_RDWR)) == -1) {
    perror("open");
    exit(1);
  }

  printf("Press <RETURN> to try to get lock: ");
  getchar();
  printf("Trying to get lock...");

  if (fcntl(fd, F_SETLKW, &fl) == -1) {
    perror("fcntl");
    exit(1);
  }

  printf("got lock\n");
  printf("Press <RETURN> to release lock: ");
  getchar();

  fl.l_type = F_UNLCK;  /* set to unlock same region */

  if (fcntl(fd, F_SETLK, &fl) == -1) {
    perror("fcntl");
    exit(1);
  }

  printf("Unlocked.\n");

  close(fd);
  return 0;
}