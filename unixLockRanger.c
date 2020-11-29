#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "os_defs.h"
#ifdef OS_WINDOWS
# include <windows.h>
# define HANDLE LockFileHandle
# define DWORD FileOffset
#else
# include <errno.h>
# define GetLastError() errno
# define HANDLE int
# define DWORD long int
#endif

# define VALID_COMMANDS  "XSU"

int lockDemo(char* filename);
static void printHelpAndExit(char *progname, char *errormessage);
static char * curtimeString();
int sysMainLine(int argc, char *argv[]);
int doLockAction(HANDLE lockFileHandle,char operation,DWORD offset,DWORD length);
int wfileLockEx(int fd, struct flock lockData, int typeCode, int offset, int length);

int wfileLockEx(int fd, struct flock fl, int typeCode, int offset, int length) {
  // F_SETLK    acquire a lock
  // F_SETLKW   release a lock
  // R_GETLK    test for the existence of a lock

  // F_RDLCK    read lock / shared lock
  // F_WRLCK    write lock / exclusive lock
  // F_UNLCK    unlock the lock

  // F_SETLKW   set lock
  // F_SETLK    unlock

  fl.l_pid = getpid();
  fl.l_whence = SEEK_SET;
  fl.l_len = length;
  fl.l_start = offset;

  if (typeCode == 2) {
    fl.l_type = F_WRLCK; // X - write lock
  } else {
    fl.l_type = F_RDLCK; // S - read lock
  }

  if (fcntl(fd, F_SETLKW, &fl) == -1) {
    perror("fcntl");
    exit(1);
  }

  return 0;
}

int main(int argc, char *argv[]) {

  sysMainLine(argc, argv);

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

// Perform the work of doing the locking operation, whatever it is.
int doLockAction(HANDLE lockFileHandle,char operation,DWORD offset,DWORD length) {
  char *operationTag;
#ifndef OS_WINDOWS
  struct flock lockData = {F_WRLCK, SEEK_SET, 0, 0, 0};
#else
  OVERLAPPED windowsOverlap;
#endif
  int typeCode;

  // Get a nice printable version of the lock operation
  operationTag = (operation == 'X' ? "Exclusive Lock":
                  operation == 'S' ? "Shared Lock" :
                  operation == 'T' ? "Test for lock" :
                  operation == 'U' ? "Unlock" : "??? unknown command ???");

  printf("PID %5d performing operation %s\n", getpid(), operationTag);

  /**
   * Code for Windows -- set up the OVERLAPPED structure, and use it
   * to create a lock.  This structure is used with the LockFileEx()
   * function to store the offset portion of our request.
   */
  #ifdef OS_WINDOWS
    memset(&windowsOverlap, 0, sizeof(windowsOverlap));
    windowsOverlap.hEvent = lockFileHandle;
    windowsOverlap.Offset = offset;
    windowsOverlap.OffsetHigh = 0;
  #endif

  // Print a message indicating that we are going into the lock request
  printf("PID %5d : requesting '%c' lock at %ld for %ld bytes at %s", getpid(), operation, offset, length, curtimeString());

  // If we want a lock, use LockFileEx() - otherwise give up the lock with UnlockFile()
  
  if (operation == 'X' || operation == 'S') {
    // shared locks are the default (no value) so only exclusive locks have a bit flag
    if (operation == 'X') {
      #ifdef OS_WINDOWS
      typeCode = LOCKFILE_EXCLUSIVE_LOCK;
      #else
      typeCode = 2;
      #endif
    }
    else {
      typeCode = 0;
    }

    #ifdef OS_WINDOWS
    // Use the OVERLAP structure and our other values to request the lock
    if ( ! LockFileEx(lockFileHandle,
        typeCode,
        0, // dwFlags : must be zero
        length, 0, // length of lock - location in overlap struct
        &windowsOverlap)) {
      fprintf(stderr, "Error: LockFile returned failure : %d\n", GetLastError());
      return -1;
    }
    #else
    wfileLockEx(lockFileHandle, lockData, typeCode, offset, length);
    #endif


  } else if (operation == 'U') {
    #ifdef OS_WINDOWS
    // Release a lock on a portion of a file
    if ( ! UnlockFile(lockFileHandle, offset, 0, length, 0)) {
      fprintf(stderr, "Error: LockFile returned failure : %d\n", GetLastError());
      return -1;
    }
    #else

    lockData.l_pid = getpid();
    lockData.l_whence = SEEK_SET;
    lockData.l_len = length;
    lockData.l_start = offset;
    lockData.l_type = F_UNLCK;

    if (fcntl(lockFileHandle, F_SETLK, &lockData) == -1) {
      perror("fcntl: error unlocking");
      exit(1);
    }

    #endif

    puts("attempt to unlock the part of the file");


  } else {
    fprintf(stderr, "Error: Unknown operation '%c'\n", operation);
    return -1;
  }  

  printf("PID %5d : received   '%c' lock at %ld for %ld bytes at %s", getpid(), operation, offset, length, curtimeString());

  return 0;
}

// System mainline -- simply process the arguments and input lines
int sysMainLine(int argc, char *argv[]) {
  /** use O/S specific type to track the right kind of file handle */
  HANDLE lockFileHandle;
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
  #else
    lockFileHandle = (int) open(argv[1], O_RDWR);
  #endif

  if (!lockFileHandle) {
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
  #ifdef OS_WINDOWS
    CloseHandle(lockFileHandle);
  #else
  close(lockFileHandle);
  #endif

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