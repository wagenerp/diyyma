#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef _WIN32
typedef unsigned int u_int32_t;
typedef signed int int32_t;

typedef unsigned short u_int16_t;
typedef signed short int16_t;

typedef unsigned char u_int8_t;
typedef signed char int8_t;

#endif

#include "diyyma/xco.h"


int _xcoerrno=0;

/** \file xco.c
  *
  * Extended Chunk Object (XCO) library
  *
  * The Extended Chunk Object is a simple format for storing structured data annotated with
  * Version information for backward- and forward compatibility with different versions of
  * an application as well as well as distinguishing XCOs created by different applications.
  *
  * Each XCO is a tree of chunks, where each chunk consists of a list of subchunks and a
  * single, continuous block of data.
  *
  * Chunk header:
  * 
  *       Offset     Field        description       
  *       0          id           ID of this chunk
  *       4          size         Total size of the chunk: entire section between the start of the 
  *                               header and offsEnd.
  *       8          offsData     Offset relative to the beginning of the XCO file to this chunk's
  *                               data segment.
  *       12         offsChunks   Offset relative to the beginning of the XCO file to this chunk's
  *                               subchunk area.
  *       16         offsEnd      Offset relative to the beginning of the XCO file to the next sibling.
  *       20         nChunks      Number of subchunks for this chunk.
  *       24         flags        Flags (unused)
  * 
  * Chunk layout:
  *
  *       Offset     Data               Size
  *       P+0        XCOChunkHead       28
  *       offsData   data segment       offsChunks-offsData
  *       offsChunks subchunks          offsEnd-offsChunks        
  *  
  *
  */

#define XCOERR(no,ret) { _xcoerrno=no; return ret; }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// common

/** \file xco.c
  * Whenever an xco_ method returns 0, an XCO_ERR_* error constant is stored which can be retrieved
  * with xco_getError or xco_getErrorString.
  */

/** \brief Returns the latest xco error's numeric value */
int xco_getError() { return _xcoerrno; }

/** \brief Returns the latest xco error's string representation. */
const char *xco_getErrorString() {
  switch(_xcoerrno) {
    case XCO_ERR_NONE: return "no error";
    case XCO_ERR_INSUFFICIENT_DATA: return "not enough data in then xco stream";
    case XCO_ERR_INVALID_TOKEN: return "data stream is not an xco";
    case XCO_ERR_INVALID_CONTEXT: return "invalid xco context";
    case XCO_ERR_NO_CHILDREN: return "current chunk has no children";
    case XCO_ERR_NO_SIBLING: return "current chunk has no next sibling";
    case XCO_ERR_NO_PARENT: return "current chunk is the root";
    case XCO_ERR_INVALID_STREAM: return "the xco stream is corrupted";
    case XCO_ERR_REPRESENTATION_BOUNDS_EXCEEDED: return "the array length exceeds the storage data type domain";
    case XCO_ERR_OUT_OF_MEMORY: return "out of memory";
    default: return "unknown error";
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// xco reader

/** \file xco.c
  * XCO Files can be read by using the XCOReaderContext structure with xcor_ methods.
  * The following example will load a tree of MY_CHUNK chunks and ouptut data from 
  * 0x10001 chunks encountered along the way.
  *       
  *       typedef struct {
  *         int foo;
  *       } MyStruct;
  *
  *       void handle_my_chunk(XCOReaderContext *ctx);
  *       
  *       void load_xco(const void *data, size_t cb) {          
  *         XCOReaderContext *ctx;
  *         
  *         if (!xcor_create(&ctx,data,cb)) { ... }             // handle error
  *          
  *         if (!xcor_chunk_sub(ctx)) { ... }                   // enter children area
  *         
  *         do switch(ctx->id) {
  *           case MY_CHUNK:
  *             handle_my_chunk(ctx);                           // handle application chunk
  *             break;
  *         } while(xcor_chunk_next(ctx));                      // next child
  *         
  *         xcor_chunk_close(&ctx);                             // leave children area
  *       }
  *       
  *       void handle_my_chunk(XCOReaderContext *ctx) {
  *         MyStruct my;
  *         MyStruct *pmy=&my;
  *         
  *         if (ctx->id!=MY_CHUNK) return;
  *         if (!xcor_chunk_sub(ctx)) return;                   // enter children area
  *         
  *         do switch(ctx->id) {
  *           case MY_CHUNK:                                    // recurse child
  *             handle_my_chunk(ctx);
  *             break;
  *           case 0x10001:                                     // data leaf chunk
  *             if (!xcor_data_readarr(ctx,(void**)&pmy,
  *                                   sizeof(MyStruct))) { 
  *               ...                                           // handle error
  *             };
  *             printf("my int: %i\n",my.int);
  *             break;
  *         } while(xcor_chunk_next(ctx));                      // next child
  *         xcor_chunk_close(ctx);                              // leave children area
  *       }
  *       
  *       
  */
  
/** \brief Allocates an XCOReaderContext for reading a single XCO 
  * 
  * \param ctx Pointer to a XCOReaderContext reference to be allocated
  * \param data Pointer to the beginning of an XCO
  * \param cb Length of the data segment pointed to by data
  * \return 1 on success, 0 on failure
  * \exception XCO_ERR_INSUFFICIENT_DATA The buffer contains insufficient data for an XCO
  * \exception XCO_ERR_OUT_OF_MEMORY The system is out of memory
  */
int xcor_create(XCOReaderContext **ctx, const void *data, size_t cb) {
  
  if (cb<sizeof(XCOChunkHead)) XCOERR(XCO_ERR_INSUFFICIENT_DATA,0);
  
  *ctx=(XCOReaderContext*)malloc(sizeof(XCOReaderContext));
  if (!*ctx) XCOERR(XCO_ERR_OUT_OF_MEMORY,0);
  
  memset(*ctx,0,sizeof(XCOReaderContext));
  
  (*ctx)->data=(char*)data;
  (*ctx)->p   =(char*)data+sizeof(XCOChunkHead);
  (*ctx)->end =(char*)data+cb;
  
  (*ctx)->head=(XCOChunkHead*)data;
  (*ctx)->idxSubChunk=0;
  (*ctx)->sHeads=XCO_DEFAULT_SHEADS;
  (*ctx)->sSubChunks=XCO_DEFAULT_SSUBCHUNKS;
  (*ctx)->heads=(XCOChunkHead**)malloc((*ctx)->sHeads*sizeof(XCOChunkHead*));
  (*ctx)->subChunks=(uint*)malloc((*ctx)->sSubChunks*sizeof(uint));
  (*ctx)->nHeads=1;
  (*ctx)->nSubChunks=0;
  (*ctx)->heads[0]=(*ctx)->head;
  
  
  return 1;
  
}

/** \brief Closes an XCOReaderContext for and frees up its memory.
  * 
  * \param ctx Pointer to a XCOReaderContext reference to be freed
  * \return This method always succeeds and returns 1.
  */
int xcor_close(XCOReaderContext **ctx) {
  if (!*ctx) return 1;
  if ((*ctx)->heads) free((void*)(*ctx)->heads);
  if ((*ctx)->subChunks) free((void*)(*ctx)->subChunks);
  
  free(*ctx);
  *ctx=0;
  return 1;
}

/** \brief Returns the position within the current chunk's data segment
  *
  * \returns Data position or -1 on error.
  * \exception XCO_ERR_INVALID_CONTEXT ctx is not a valid context.
  */
size_t xcor_data_pos(XCOReaderContext *ctx) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,-1);
  return (size_t)ctx->p-(size_t)ctx->data-ctx->head->offsData;
}
/** \brief Returns the size of the current chunk's data segment
  *
  * \returns Data position or -1 on error.
  * \exception XCO_ERR_INVALID_CONTEXT ctx is not a valid context.
  */
size_t xcor_data_size(XCOReaderContext *ctx) {
  if (!ctx->head) XCOERR(XCO_ERR_INVALID_CONTEXT,-1);
  return ctx->head->offsChunks-ctx->head->offsData;
}

/** \brief Returns the number of remaining bytes within the current chunk's data segment
  *
  * \returns Data position or -1 on error.
  * \exception XCO_ERR_INVALID_CONTEXT ctx is not a valid context.
  */
size_t xcor_data_remain(XCOReaderContext *ctx) {
  if (!ctx->head) XCOERR(XCO_ERR_INVALID_CONTEXT,-1);
  return ctx->head->offsChunks-(size_t)ctx->p+(size_t)ctx->data;
}


/** \brief Moves the cursor to a position within the current chunk's data segment.
  *
  * \param offs offset in bytes, can be negative.
  * \param r Relative starting position: 
  * - 0 for beginning of the data segment
  * - 1 for the current position
  * - 2 for the end of the data segment
  *
  * \returns New data position or -1 on error.
  * \exception XCO_ERR_INVALID_CONTEXT ctx is not a valid context.
  * \exception XCO_ERR_INSUFFICIENT_DATA The targeted position is not within
  * the current chunk's data segment.
  */
size_t xcor_data_seek(XCOReaderContext *ctx, int offs, int r) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,-1);
  const char *p;
  switch(r) {
    default:
    case 0: p=ctx->data+ctx->head->offsData+offs; break;
    case 1: p=ctx->p+offs; break;
    case 2: p=ctx->data+ctx->head->offsChunks+offs; break;
  }
  if (((size_t)p<(size_t)ctx->data+ctx->head->offsData)
  ||((size_t)p>(size_t)ctx->data+ctx->head->offsChunks)) 
    XCOERR(XCO_ERR_INSUFFICIENT_DATA,-1);
  ctx->p=p;
  return (size_t)p-(size_t)ctx->data-ctx->head->offsData;
}

int xcor_data_readarr(XCOReaderContext *ctx, void **arr, size_t cb) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  if (xcor_data_remain(ctx)<cb) return 0;
  *arr=realloc(*arr,cb);
  memcpy(*arr,ctx->p,cb);
  ctx->p+=cb;
  return 1;
}


/** \brief Will attempt to read an array from the data section.
  *
  * Tries to read an 8 bit unsigned integer denoting the length of the array
  * and stores the result in arr. If arr points to null or *pcb is too small,
  * *arr will be (re)allocated to accomodate the array being read.
  *
  * If pcb is null, the read array is extended with a terminating 0 byte.
  *
  * \param ctx Pointer to a XCOReaderContext representing the xco being read.
  * \param arr Pointer to an array to take the result data.
  * \param pcb Pointer to the length of the input / output array, may be null.
  */
int xcor_data_readarr_b(XCOReaderContext *ctx, void **arr, size_t *pcb) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  size_t rem=xcor_data_remain(ctx);
  size_t cb;
  
  if (rem<1) return 0;
  cb=*(u_int8_t*)ctx->p; 
  if (rem<1+cb) return 0;
  ctx->p+=1;
  if (!pcb) *arr=realloc(*arr,cb+1);
  else      *arr=realloc(*arr,cb);
  memcpy(*arr,ctx->p,cb);
  ctx->p+=cb;
  if (!pcb) ((char*)arr)[cb]=0; 
  else      *pcb=cb;
  return 1;
}

/** \brief Will attempt to read an array from the data section.
  *
  * Tries to read a 16 bit unsigned integer denoting the length of the array
  * and stores the result in arr. If arr points to null or *pcb is too small,
  * *arr will be (re)allocated to accomodate the array being read.
  *
  * If pcb is null, the read array is extended with a terminating 0 byte.
  *
  * \param ctx Pointer to a XCOReaderContext representing the xco being read.
  * \param arr Pointer to an array to take the result data.
  * \param pcb Pointer to the length of the input / output array, may be null.
  */
int xcor_data_readarr_s(XCOReaderContext *ctx, void **arr, size_t *pcb) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  size_t rem=xcor_data_remain(ctx);
  size_t cb;
  if (rem<2) return 0;
  cb=*(u_int16_t*)ctx->p;
  if (rem<2+cb) return 0;
  ctx->p+=2;
  if (!pcb) *arr=realloc(*arr,cb+1);
  else      *arr=realloc(*arr,cb);
  memcpy(*arr,ctx->p,cb);
  ctx->p+=cb;
  if (!pcb) ((char*)*arr)[cb]=0; 
  else      *pcb=cb;
  return 1;
}


/** \brief Will attempt to read an array from the data section.
  *
  * Tries to read a 32 bit unsigned integer denoting the length of the array
  * and stores the result in arr. If arr points to null or *pcb is too small,
  * *arr will be (re)allocated to accomodate the array being read.
  *
  * If pcb is null, the read array is extended with a terminating 0 byte.
  *
  * \param ctx Pointer to a XCOReaderContext representing the xco being read.
  * \param arr Pointer to an array to take the result data.
  * \param pcb Pointer to the length of the input / output array, may be null.
  */
int xcor_data_readarr_i(XCOReaderContext *ctx, void **arr, size_t *pcb) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  size_t rem=xcor_data_remain(ctx);
  size_t cb;
  
  if (rem<4) return 0;
  cb=*(u_int32_t*)ctx->p; 
  if (rem<4+cb) return 0;
  ctx->p+=4;
  if (!pcb) *arr=realloc(*arr,cb+1);
  else      *arr=realloc(*arr,cb);
  memcpy(*arr,ctx->p,cb);
  ctx->p+=cb;
  if (!pcb) ((char*)arr)[cb]=0; 
  else      *pcb=cb;
  return 1;
}

int _xcor_subchunk_push(XCOReaderContext *ctx, int idx) {
  if (ctx->sSubChunks<=ctx->nSubChunks) {
    ctx->sSubChunks=ctx->nSubChunks+1;
    ctx->subChunks=(uint*)realloc((void*)ctx->heads,ctx->sSubChunks*sizeof(uint));
  }
  ctx->subChunks[ctx->nSubChunks++]=idx;
  return 1;
}

int _xcor_chunk_push(XCOReaderContext *ctx, XCOChunkHead *head) {
  if (ctx->sHeads<=ctx->nHeads) {
    ctx->sHeads=ctx->nHeads+1;
    ctx->heads=(XCOChunkHead**)realloc((void*)ctx->heads,ctx->sHeads*sizeof(XCOChunkHead*));
  }
  ctx->heads[ctx->nHeads++]=head;
  return 1;
}

int xcor_chunk_sub(XCOReaderContext *ctx) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  if (ctx->head->nChunks<1) XCOERR(XCO_ERR_NO_CHILDREN,0);
  
  XCOChunkHead *head=(XCOChunkHead*)(ctx->data+ctx->head->offsChunks);
  
  if (head->size>=ctx->head->size) XCOERR(XCO_ERR_INVALID_STREAM,0);
  if (head->offsData  >ctx->head->offsChunks+head->size) XCOERR(XCO_ERR_INVALID_STREAM,0);
  if (head->offsChunks>ctx->head->offsChunks+head->size) XCOERR(XCO_ERR_INVALID_STREAM,0);
  if (head->offsEnd   >ctx->head->offsChunks+head->size) XCOERR(XCO_ERR_INVALID_STREAM,0);
  
  ctx->head=head;
  
  _xcor_chunk_push(ctx,head);
  _xcor_subchunk_push(ctx,0);
  ctx->idxSubChunk=ctx->subChunks+ctx->nSubChunks-1;
  
  ctx->p=ctx->data+head->offsData;
  
  return 1;
  
}

int xcor_chunk_next(XCOReaderContext *ctx) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  if (ctx->nHeads<2) XCOERR(XCO_ERR_NO_SIBLING,0);
  
  XCOChunkHead *head=ctx->heads[ctx->nHeads-2];
  if (*ctx->idxSubChunk+1>=head->nChunks) XCOERR(XCO_ERR_NO_SIBLING,0);
  
  XCOChunkHead *newHead=(XCOChunkHead*)(ctx->data+ctx->head->offsEnd);
  
  if (newHead->size+ctx->head->offsEnd>head->offsEnd) XCOERR(XCO_ERR_INVALID_STREAM,0);
  if (newHead->offsData  >newHead->offsChunks   ) XCOERR(XCO_ERR_INVALID_STREAM,0);
  if (newHead->offsChunks>newHead->offsEnd      ) XCOERR(XCO_ERR_INVALID_STREAM,0);
  if (newHead->offsEnd   >head->offsEnd         ) XCOERR(XCO_ERR_INVALID_STREAM,0);
  
  ctx->heads[ctx->nHeads-1]=newHead;
  ctx->head=newHead;
  (*ctx->idxSubChunk)++;
  ctx->p=ctx->data+newHead->offsData;
  
  return 1;
}

int xcor_chunk_close(XCOReaderContext *ctx) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  if (ctx->nHeads<2) XCOERR(XCO_ERR_NO_PARENT,0);
  
  ctx->nHeads--;
  ctx->nSubChunks--;
  ctx->head=ctx->heads[ctx->nHeads-1];
  ctx->idxSubChunk=ctx->subChunks+ctx->nSubChunks-1;
  ctx->p=ctx->data+ctx->head->offsData;
  
  return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// xco writer

void xcow_ensure_byte_count(XCOWriterContext *ctx, int n) {
  if ((long)ctx->p+n>(long)ctx->end) {
    long cb1=(long)(((long)ctx->p-(long)ctx->data+n)/XCO_DEFAULT_BUFSIZE+1)*XCO_DEFAULT_BUFSIZE;
    
    const char *data0=ctx->data;
    ctx->data=(char*)realloc((void*)ctx->data,cb1);
    
    long offs=(long)ctx->data-(long)data0;
    
    ctx->p+=offs;
    ctx->head=(XCOChunkHead*)((char*)ctx->head+offs);
    
    long i;
    for(i=0;i<ctx->nHeads;i++) ctx->heads[i]=(XCOChunkHead*)((char*)ctx->heads[i]+offs);
    ctx->end=ctx->data+cb1;
  }
}

int _xcow_chunk_push(XCOWriterContext *ctx, XCOChunkHead *head) {
  if (ctx->sHeads<=ctx->nHeads) {
    ctx->sHeads=ctx->nHeads+1;
    ctx->heads=(XCOChunkHead**)realloc((char*)ctx->heads,ctx->sHeads*sizeof(XCOChunkHead*));
  }
  ctx->heads[ctx->nHeads++]=head;
  return 1;
}

int xcow_create(XCOWriterContext **ctx) {
  
  *ctx=(XCOWriterContext*)malloc(sizeof(XCOWriterContext));
  
  (*ctx)->data=(char*)malloc(XCO_DEFAULT_BUFSIZE);
  (*ctx)->end=(*ctx)->data+XCO_DEFAULT_BUFSIZE;
  (*ctx)->p=(*ctx)->data;
  
  (*ctx)->chunkMode=0;
  
  xcow_ensure_byte_count((*ctx),sizeof(XCOChunkHead));
  
  (*ctx)->head=(XCOChunkHead*)(*ctx)->p;
  (*ctx)->p  +=sizeof(XCOChunkHead);
  
  (*ctx)->head->id=XCO_TOKEN;
  (*ctx)->head->offsData=(long)(*ctx)->p-(long)(*ctx)->data;
  (*ctx)->head->nChunks=0;
  (*ctx)->head->flags=0;
  
  (*ctx)->sHeads=XCO_DEFAULT_SHEADS;
  (*ctx)->heads=(XCOChunkHead**)malloc((*ctx)->sHeads*sizeof(XCOChunkHead*));
  (*ctx)->nHeads=0;
  _xcow_chunk_push((*ctx),(*ctx)->head);
  
  return 1;
}

int xcow_close(XCOWriterContext **ctx) {
  if (!(*ctx)) return 1;
  if ((*ctx)->heads) free((void*)(*ctx)->heads);
  if ((*ctx)->data) free((*ctx)->data);
  
  free(*ctx);
  *ctx=0;
  return 1;
}

int xcow_chunk_new(XCOWriterContext *ctx, uint id) {
  if (ctx->chunkMode) {
    ctx->head->nChunks++;
  } else {
    ctx->head->offsChunks=(long)ctx->p-(long)ctx->data;
    ctx->head->nChunks=1;
  }
  
  xcow_ensure_byte_count(ctx,sizeof(XCOChunkHead));
  
  ctx->head=(XCOChunkHead*)ctx->p;
  ctx->p  +=sizeof(XCOChunkHead);
  
  ctx->head->id=id;
  ctx->head->offsData=(long)ctx->p-(long)ctx->data;
  ctx->head->nChunks=0;
  ctx->head->flags=0;
  
  _xcow_chunk_push(ctx,ctx->head);
  
  ctx->chunkMode=0;
  
  return 1;
}

int xcow_chunk_close(XCOWriterContext *ctx) {
  if (ctx->nHeads<2) XCOERR(XCO_ERR_NO_PARENT,0);
  
  ctx->head->size=sizeof(XCOChunkHead)+(long)ctx->p-(long)ctx->data-ctx->head->offsData;
  ctx->head->offsEnd=(long)ctx->p-(long)ctx->data;
  
  if (!ctx->chunkMode) ctx->head->offsChunks=ctx->head->offsEnd;
  
  ctx->nHeads--;
  ctx->head=ctx->heads[ctx->nHeads-1];
  
  ctx->chunkMode=1;
  
  return 1;
}

int xcow_finalize(XCOWriterContext *ctx) {
  while(ctx->nHeads>1) xcow_chunk_close(ctx);
  
  long dp=(long)ctx->p-(long)ctx->data;
  
  if (!ctx->chunkMode) ctx->head->offsChunks=dp;
  
  ctx->head->size=sizeof(XCOChunkHead)+dp-ctx->head->offsData;
  ctx->head->offsEnd=dp;
  
  return 1;
}

int xcow_reset(XCOWriterContext *ctx) {
  ctx->p=ctx->data+sizeof(XCOChunkHead);
  ctx->head=(XCOChunkHead*)ctx->data;
  
  ctx->head->id=XCO_TOKEN;
  ctx->head->size=0;
  ctx->head->offsData=(long)ctx->p-(long)ctx->data;
  ctx->head->offsChunks=0;
  ctx->head->offsEnd=0;
  ctx->head->nChunks=0;
  ctx->head->flags=0;
  
  ctx->nHeads=1,
  ctx->heads[0]=ctx->head;
  
  ctx->chunkMode=0;
  
  return 1;
}

int xcow_data_writearr(XCOWriterContext *ctx, void *arr, int cb) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  xcow_ensure_byte_count(ctx,cb);
  
  memcpy(ctx->p,arr,cb);
  ctx->p+=cb;
  
  return 1;
}

int xcow_data_writearr_b(XCOWriterContext *ctx, void *arr, int cr, int cbr) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  int cb=cr*cbr;
  if (cr>0xff) XCOERR(XCO_ERR_REPRESENTATION_BOUNDS_EXCEEDED,0);
  xcow_ensure_byte_count(ctx,cb+1);
  
  *(unsigned char*)ctx->p=(unsigned char)cr;
  ctx->p+=1;
  
  memcpy(ctx->p,arr,cb);
  ctx->p+=cb;
  
  return 1;
}

int xcow_data_writearr_s(XCOWriterContext *ctx, void *arr, int cr, int cbr) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  int cb=cr*cbr;
  if (cr>0xffff) XCOERR(XCO_ERR_REPRESENTATION_BOUNDS_EXCEEDED,0);
  xcow_ensure_byte_count(ctx,cb+2);
  
  *(unsigned short*)ctx->p=(unsigned short)cr;
  ctx->p+=2;
  
  memcpy(ctx->p,arr,cb);
  ctx->p+=cb;
  
  return 1;
}

int xcow_data_writearr_i(XCOWriterContext *ctx, void *arr, int cr, int cbr) {
  if (!ctx->data) XCOERR(XCO_ERR_INVALID_CONTEXT,0);
  int cb=cr*cbr;
  xcow_ensure_byte_count(ctx,cb+4);
  
  *(int*)ctx->p=cr;
  ctx->p+=4;
  
  memcpy(ctx->p,arr,cb);
  ctx->p+=cb;
  
  return 1;
}

