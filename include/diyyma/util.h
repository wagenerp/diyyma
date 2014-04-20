
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
#include <string.h>

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

#define APPEND(n,o) \
  (n##_v=(typeof(n##_v))realloc( \
    (void*)(n##_v), \
    sizeof(typeof(*n##_v))*((n##_n)+1)))[(n##_n)++]=o;

#define ARRAY(t,n) \
  t *n##_v; \
  size_t n##_n; 
  
#define ARRAY_INIT(n) \
  n##_v=0; \
  n##_n=0;

#define ARRAY_DESTROY(n) \
  if (n##_v) { \
    free((void*)n##_v); \
    n##_v=0; \
    n##_n=0; \
  }

#define ARRAY_SETSIZE(n,o) \
  n##_v=(typeof(n##_v))realloc( \
    (void*)(n##_v), \
    sizeof(typeof(*n##_v))*(++(n##_n)));


#define FOREACH(i,o,n) \
  for((i)=0,(o)=(n##_v);(i)<(n##_n);(i)++,(o)++)



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

/** \brief Interface for RCObjects. 
  *
  * Used only on other interfaces that
  * have a chance of being implemented / inherited by classes otherwise
  * implementing / inheriting an RCObject. Make sure to perform virtual
  * inheritance in such a case.
  */
class IRCObject {
  public:
    virtual ~IRCObject() { }
    virtual void grab() =0;
    virtual void drop() =0;
};

/** \brief Reference-count garbage collected objects.
  *
  * Since we may very well have objects being referenced by multiple 
  * other objects, we perform simple reference counting to keep track of
  * used and unused objects.
  *
  * Initially the reference counter is set to 0, meaning the object
  * can be deleted just like any other, however if it is being assigned to
  * other objects and therefore grabbed, the reference counter goes up
  * and deletion should not be performed. ever.
  *
  * When the reference counter hits 0 (from above) it is automatically 
  * deleted. Therefore, when you call drop() on a reference to an objects,
  * you must not access it afterwards.
  */
class RCObject : public virtual IRCObject {
  private:
    int _refcount;
  public:
    RCObject();
    virtual ~RCObject() { }
    
    /** \brief increases the reference counter by one. */
    virtual void grab();
    
    /** \brief decreases the reference counter by one, deleting the
      * object if it hits zero. */
    virtual void drop();
};

/** \brief Abstract class shared by all assets (shaders, textures, meshes, etc).
  *
  * This common class models the notion of file system connection and related 
  * reloading of resources (e.g. for quickly reloading shaders in development
  * without reloading the whole application)
  */
class IAsset : public RCObject {
  public:
    virtual ~IAsset() { }
    
    /** \brief Reloads whatever resources this asset loaded initially.*/
    virtual void reload() =0;
    
    /** \brief Returns the newest modification timestamp of all the files
      * related to this asset. 
      *
      * Most often this is just one file (a texture, a mesh, a sound file, etc)
      * however in the case of shaders, multiple files are loaded.
      */
    virtual timestamp_t filesTimestamp() =0;
    
    /** \brief For managed assets, this method makes it easier to manage them. 
      * 
      * Asset managers understand the notion of failure during resource loading
      * and therefore require a return value on load.
      */
    virtual int load(const char *fn) =0;
    
};

/** \brief Common interface for anything that iterates - mostly scene nodes.*/
class IIterator {
  public:
    virtual ~IIterator() { }
    virtual void iterate(double dt, double t) =0;
};

/** \brief It's always nice to implement a sorting algorithm. */
template<class T, class K> void sortByKeys(T *objects, K *keys, size_t n);

template<class T, class K> void quickSort(
  T *objects, K *keys, 
  size_t l, size_t r);


template<class T, class K> void qs_swap(
  T *objects, K *keys, 
  size_t a, size_t b) {
  { T t=objects[a]; objects[a]=objects[b]; objects[b]=t; }
  { K t=keys[a]; keys[a]=keys[b]; keys[b]=t; }
}


template<class T, class K> void quickSort(
  T *objects, K *keys, 
  size_t l, size_t r) {
  int s;
  K pr;
  int i;
  
  if (r<=l) return;
  
  s=(l+r)/2;
  
  if (keys[l]<keys[r]) {
    if (keys[s]<keys[l]) s=l;
    else if (keys[r]<keys[s]) s=r;
  } else {
    if (keys[s]<keys[r]) s=r;
    else if (keys[l]<keys[s]) s=l;
  }
  
  pr=keys[s];
  qs_swap<T,K>(objects,keys,s,r);
  
  s=l;
  for(i=l;i<r;i++) if (keys[i]<pr) qs_swap<T,K>(objects,keys,i,s++);
  qs_swap<T,K>(objects,keys,s,r);
  
  
  quickSort(objects,keys,l,s-1);
  quickSort(objects,keys,s+1,r);
}

template<class T, class K> void sortByKeys(T *objects, K *keys, size_t n) {
  
  quickSort<T,K>(objects,keys,0,n-1);
}

double randf();

int strcmp_ic(const char *a, const char *b);

/** \brief Asset manager for a single type of asset. 
  *
  * Make sure T actually implements IAsset.
  *
  * Instead of loading assets directly via constructors, invoking the
  * related manager's get method offers a way to obtain a reference
  * to an asset already loaded, in contrast to either having to manage 
  * reused assets manually or loading it multiple times. 
  *
  * At the same time, the AssetRegistry performs virtual file system lookups
  * to support multiple search paths per asset type. */
  */
  
template<class T> class AssetRegistry {
  private:
    ARRAY(T*,_assets);
    ARRAY(const char*,_names);
    
    int _repository_mask;
  
  public:
    /** \brief Constructor.
      *
      * \param repository_mask Bitmask of VFS repositories to use when locating
      * files.
      */
    AssetRegistry(int repository_mask) {
      ARRAY_INIT(_assets);
      ARRAY_INIT(_names);
      _repository_mask=repository_mask;
    }
    
    ~AssetRegistry() {
      int idx;
      const char **pstr;
      
      FOREACH(idx,pstr,_names) free((void*)*pstr);
      
      ARRAY_DESTROY(_assets);
      ARRAY_DESTROY(_names);
    }
    
    /** \brief Returns a reference to an asset of the specified base name.
      * 
      * In most cases, the base name is actually a file name, but it does not
      * need to be. 
      * The individual asset class may perform name translation, such as
      * the Shader type: the base name is appended with an extension per
      * shader type. 
      */
    T *get(const char *name) {
      int idx;
      const char **pstr;
      char *str;
      FOREACH(idx,pstr,_names) if (strcmp(*pstr,name)==0) {
        return _assets_v[idx];
      }
      
      T *res=new T();
      if (!res->load(name)) {
        delete res;
        return 0;
      }
      
      APPEND(_assets,res);
      APPEND(_names,strdup(name));
      
      return res;
    }
    
};


#endif