#ifndef __FORMATTER_DEFS_HEADER__
#define __FORMATTER_DEFS_HEADER__

/* System include files. */
#ifndef MAKEDEPEND
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#endif


	/*
	 * You will have to add more code here -- use the -E or /E flag of
	 * your compiler to see what it defines by default, in order to
	 * be able to figure out what operating system/compiler combo you are
	 * on.  You should be able to define one of the following macros to
	 * identify each of the indicated systems:
	 *	OS_BSD
	 *		defined only if the environment is some form of BSD
	 *		(this one is done for you)
	 *	OS_WINDOWS
	 *		defined only if the environment is some Windows variant
	 *		(Windows 10, etc.)
	 *	OS_LINUX
	 *		defined only if the environment is some version of Linux
	 *	OS_DARWIN
	 *		defined only if the environment is some version of Darwin/MacOSX
	 */

/*
 * Compilation Environment Identification
 */
#if defined( __NetBSD__ ) || defined( __OpenBSD__) || defined( __FreeBSD__ )

#   define OS_BSD


/* we saw this one in pipeToMore.c */
#elif defined( __APPLE__ )

#   define OS_DARWIN


#elif defined(__linux__)

#   define OS_LINUX

// remember to figure out Windows
// https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
#elif defined(_WIN64)

#	define OS_WINDOWS

#elif defined(_WIN32)

#	define OS_WINDOWS

#else
#   error Unknown operating system -- need more defines in os_defs.h!
#endif



/*
 * if we are using a dynamic loaded library, windows wants to know
 * in what way we are using it
 */
#if defined( OS_WINDOWS )
# if defined( OS_USE_DYNAMIC )
	/**
	 **	if OS_USE_DYNAMIC is declared before importing this header, then
	 **	we want OS_EXPORT declarations in other header files to turn into
	 **	_import_ declarations
	 **/
#  define OS_EXPORT __declspec(dllimport)
#  define OS_C_DECL __cdecl

# elif defined( OS_STATIC )
	/**
	 **	if we are statically linking, these should be nothing, as on other
	 ** OSs
	 **/
#  define OS_EXPORT
#  define OS_C_DECL


# else
	/**
	 **	default to dynamic linking, and declaring the objects as such in
	 ** headers and in code
	 **/
#  define OS_EXPORT __declspec(dllexport)
#  define OS_C_DECL __cdecl
# endif
#else

	/**
	 **	If not on Windows, none of this applies, so simply deflate the
	 **	macro to nothing
	 **/
#  define OS_EXPORT
#  define OS_C_DECL
#endif



/**
 * Win32 hacks...
 */
#ifdef  OS_WINDOWS

	/*
	 * Define a character and string for use when trying to put
	 * together directory paths for our various operating systems.
	 * Everyone but Microsoft uses /, but MS uses \
	 */
# define	OS_PATH_DELIM				'\\'
# define	OS_PATH_DELIM_STRING		"\\"

#else

	/** define the path delimiter for all non-Windows machines */
# define	OS_PATH_DELIM			'/'
# define	OS_PATH_DELIM_STRING		"/"
#endif


#if defined(OS_LINUX) || defined(OS_DARWIN)
	/*
	 * the C spec is silent on the ability to cast between
	 * void pointers and function pointers.
	 *
	 * Both Linux and Darwin assume that this is OK, and have
	 * no function-pointer-specific accessor for text symbols
	 * (in contrast to BSD, below)
	 *
	 * We therefore will call dlsym() regardless of whether the
	 * symbol type is text or data, and assume that the result
	 * can be cast to a function pointer as appropriate
	 */
# define	DL_FUNC_TYPE	void *
# define	DL_FUNC(h,s)	dlsym(h,s)


#elif defined( OS_BSD )
	/*
	 * The BSD family defines dlfunc() to return a function pointer
	 * specific accessor specifically for this case
	 */
# define	DL_FUNC_TYPE	dlfunc_t
# define	DL_FUNC(h,s)	dlfunc(h,s)

#elif defined( OS_WINDOWS )
# define	DL_FUNC_TYPE	void *
# define	DL_FUNC(h,s)	GetProcAddress(h,s)

#else

# error figure out DL_FUNC for this operating system

#endif

#endif /* __FORMATTER_DEFS_HEADER__ */
