
/** \file util.h
  * \author Peter Wagener
  * \brief Miscellaneous utility makros and methods.
  *
  * Logging and error handling / output is handled here, among other things.
  *
  */
  
#ifndef DIYYMA_UTIL_H
#define DIYYMA_UTIL_H
#include <stdio.h>
#include <stdarg.h>

#define SDL_ASSERT_WARN(r,name) { \
  if (!(r)) { \
    LOG_WARNING("WARNING: %s failed (%s)\n",name,SDL_GetError()); \
    fflush(stdout); \
  } \
}

#define SDL_ASSERT(r,name) { \
  if (!(r)) { \
    LOG_WARNING("ERROR: %s failed (%s)\n",name,SDL_GetError()); \
    fflush(stdout); \
    return 1; \
  } \
}

#define SDL_ASSERTJ(r,name,lbl) { \
  if (!(r)) { \
    LOG_ERROR("ERROR: %s failed (%s)\n",name,SDL_GetError()); \
    fflush(stdout); \
    goto lbl; \
  } \
}

#define ASSERT_WARN(r,name) { \
  if (!(r)) { \
    LOG_WARNING("WARNING: %s failed\n",name); \
    fflush(stdout); \
  } \
}

#define ASSERT(r,name) { \
  if (!(r)) { \
    LOG_ERROR("ERROR: %s failed\n",name); \
    fflush(stdout); \
    return 1; \
  } \
}

#define ASSERTJ(r,name,lbl) { \
  if (!(r)) { \
    LOG_ERROR("ERROR: %s failed\n",name); \
    fflush(stdout); \
    goto lbl; \
  } \
}

#define LOG(mask,...) {if(logMask()&mask) { printf(__VA_ARGS__); }}
#define LOG_ERROR(...) LOG(LOG_MASK_ERROR,__VA_ARGS__)
#define LOG_WARNING(...) LOG(LOG_MASK_WARNING,__VA_ARGS__)
#define LOG_DEBUG(...) LOG(LOG_MASK_DEBUG,__VA_ARGS__)
#define LOG_INFO(...) LOG(LOG_MASK_INFO,__VA_ARGS__)

#define LOG_MASK_ERROR 0x01
#define LOG_MASK_WARNING 0x02
#define LOG_MASK_DEBUG 0x04
#define LOG_MASK_INFO 0x08
#define LOG_MASK_DB 0x10

#define LOG_MASK_VERBOSE (LOG_MASK_ERROR|LOG_MASK_WARNING|LOG_MASK_DEBUG|LOG_MASK_INFO|LOG_MASK_DB)

#define ERROR(...) { LOG_ERROR(__VA_ARGS__); return 0; }
#define ERRORJ(lbl,...) { LOG_ERROR(__VA_ARGS__); goto lbl; }

#ifdef _MSC_VER
#define _snprintf _snprintf_s
#else
#define _snprintf snprintf
#endif

#ifdef _WIN32
typedef unsigned long u_int32_t;
#endif

typedef unsigned long long int timestamp_t;

int logMask();
void setLogMask(int v);

/** \brief Text-to-speech interface */
void speakrf(char *fmt,...);


/** \brief Reads the whole of a specified file into a buffer.
  *
  * This method always allocates enough memory to store the
  * entire file, plus an additional zero character for termination
  * of text-based files.
  *
  * \param fn File name to load.
  * \param data Reference to an untyped pointer receiving the content.
  * \param cb Reference to a size_t receiving the size of the loaded file.
  * This does not include the termination character.
  * \return 1 on success, 0 on error. No memory is allocated and cb is 
  * undefined on error.
  */
int readFile(const char *fn,void **data, size_t *cb);

/** \brief Checks if a given file exists.
  *
  * \return 1 if it does, 0 otherwise.
  */

int file_exists(const char *fn);

/** \brief Returns a timestamp for an input file.
  *
  * If the file does not exist, 0 is returned, otherwise a system-specific
  * timestamp is returned.
  */
timestamp_t file_timestamp(const char *fn);

#define REPOSITORY_TEXTURE 0
#define REPSOITORY_MESH    1
#define REPOSITORY_SHADER  2
#define REPOSITORY_COUNT   3

#define REPOSITORY_MASK_TEXTURE 0x01
#define REPOSITORY_MASK_MESH    0x02
#define REPOSITORY_MASK_SHADER  0x04

/** \brief Registers a path for loading assets from.
  * \param path Path to register. Must end on a path delimiter.
  * \param mask Combination of REPOSITORY_MASK_* flags denoting
  * the repository types to assign the path to.
  */
void vfs_registerPath(const char *path, int mask);

/** \brief Attempts to resolve a local file name within repositories.
  *
  * If the file was found, its path is returned. This is always a unique
  * string so it must be freed by the callee.
  *
  * \param path Local file name to resolve, may be an actually existing
  * file name, which is then returned as a duplicate.
  * \param mask Combination of REPOSITORY_MASK_* flags denoting the
  * types of repositories to query for the file. 
  */
char *vfs_locate(const char *path, int mask);


class RCObject {
  private:
    int _refcount;
  public:
    RCObject(): _refcount(0) { }
    void grab() { _refcount++; }
    void drop() { _refcount--; if (!_refcount) delete this; }
};

class IAsset : public RCObject {
  public:
    virtual ~IAsset() { }
    virtual void reload() =0;
    virtual timestamp_t filesTimestamp() =0;
    
};

template<class T, class K> void sortByKeys(T *objects, K *keys, size_t n);

template<class T, class K> void quickSort(
  T *objects, K *keys, 
  size_t l, size_t r);

#endif