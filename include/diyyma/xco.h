
/** \file xco.h
  * \author Peter Wagener
  * \brief Extended Chunk Object library.
  *
  * 
  */

#ifndef _DIYYMA_XCO_H
#define _DIYYMA_XCO_H

#define uint unsigned int

typedef struct XCOChunkHead {
  uint id;
  uint size;
  uint offsData;
  uint offsChunks;
  uint offsEnd;
  uint nChunks;
  uint flags;
} XCOChunkHead;

typedef struct XCOChunkState {
  int idSubChunk;
} XCOChunkState;

#define XCO_TOKEN 0x314f4358
#define XCO_DEFAULT_BUFSIZE 40000

typedef struct XCOReaderContext {
  const char *data;
  const char *p, *end;
  
  XCOChunkHead *head;
  XCOChunkHead **heads;
  uint *idxSubChunk;
  uint *subChunks;
  
  
  int nHeads;
  int nSubChunks;
  
  int sHeads;
  int sSubChunks;
  
} XCOReaderContext;

typedef struct XCOWriterContext {
  char *data;
  char *p, *end;
  
  XCOChunkHead *head;
  XCOChunkHead **heads;
  
  int chunkMode;
  
  int nHeads;
  int sHeads;
  
} XCOWriterContext;

#define XCO_DEFAULT_SHEADS 20
#define XCO_DEFAULT_SSUBCHUNKS 20

#define XCO_ERR_NONE 0
#define XCO_ERR_INSUFFICIENT_DATA 1
#define XCO_ERR_INVALID_TOKEN 2
#define XCO_ERR_INVALID_CONTEXT 3
#define XCO_ERR_NO_CHILDREN 4
#define XCO_ERR_NO_SIBLING 5
#define XCO_ERR_NO_PARENT 6
#define XCO_ERR_INVALID_STREAM 7
#define XCO_ERR_REPRESENTATION_BOUNDS_EXCEEDED 8
#define XCO_ERR_OUT_OF_MEMORY 9


int xco_getError();
const char *xco_getErrorString();



int xcor_create(XCOReaderContext **ctx, const void *data, size_t cb);
int xcor_close(XCOReaderContext **ctx);
size_t xcor_data_pos(XCOReaderContext *ctx);
size_t xcor_data_size(XCOReaderContext *ctx);
size_t xcor_data_remain(XCOReaderContext *ctx);

size_t xcor_data_seek(XCOReaderContext *ctx, int offs, int r);

#define xcor_data_map(ctx) (ctx->head->offsData+ctx->data)

#ifdef _MSC_VER
#define xcor_data_read(ctx,d) { if (xcor_data_remain(ctx)>=sizeof(d)) { d=*(decltype(d)*)((ctx)->p); (ctx)->p+=sizeof(d); } }
#else
#define xcor_data_read(ctx,d) { if (xcor_data_remain(ctx)>=sizeof(d)) { d=*(decltype(d)*)(ctx)->p; (ctx)->p+=sizeof(d); } }
#endif

int xcor_data_readarr(XCOReaderContext *ctx, void **arr, size_t cb);
int xcor_data_readarr_b(XCOReaderContext *ctx, void **arr, size_t *pcb);
int xcor_data_readarr_s(XCOReaderContext *ctx, void **arr, size_t *pcb);
int xcor_data_readarr_i(XCOReaderContext *ctx, void **arr, size_t *pcb);

int xcor_chunk_sub(XCOReaderContext *ctx);
int xcor_chunk_next(XCOReaderContext *ctx);
int xcor_chunk_close(XCOReaderContext *ctx);



#define xcow_size(ctx) ((int)(ctx)->p-(int)(ctx)->data)

void xcow_ensure_byte_count(XCOWriterContext *ctx, int n);
int xcow_create(XCOWriterContext **ctx);
int xcow_close(XCOWriterContext **ctx);

int xcow_chunk_new(XCOWriterContext *ctx, uint id);
int xcow_chunk_close(XCOWriterContext *ctx);
int xcow_finalize(XCOWriterContext *ctx);
int xcow_reset(XCOWriterContext *ctx);

#define xcow_data_write(ctx,d) { if (!(ctx)->chunkMode) { xcow_ensure_byte_count(ctx,sizeof(d)); *(decltype(d)*)(ctx)->p=d; (ctx)->p+=sizeof(d); } }
int xcow_data_writearr(XCOWriterContext *ctx, void *arr, int cb);
int xcow_data_writearr_b(XCOWriterContext *ctx, void *arr, int cr, int cbr);
int xcow_data_writearr_s(XCOWriterContext *ctx, void *arr, int cr, int cbr);
int xcow_data_writearr_i(XCOWriterContext *ctx, void *arr, int cr, int cbr);

#endif
