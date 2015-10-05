

/*
 * mm_alloc.c
 *
 * Stub implementations of the mm_* routines. Remove this comment and provide
 * a summary of your allocator's design here.
 */
#include "mm_alloc.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#define ASSERT     assert
#define align4(x)  (((((x)-1)>>2)<<2)+4)
#define TRUE 1
#define FALSE 0
                                                    
s_block_ptr base = NULL;            
s_block_ptr last;       
static s_block_ptr find_block(size_t size)    
{  
  s_block_ptr p = base;
  for (; p != NULL; p = p->next)            
   {
      last = p;
      if (p->free && p->size >= size)         
          return p;                           
   }
  return NULL;                                
}

static s_block_ptr extend_heap(size_t s)                
{    
    
    s_block_ptr new_block;                    
    new_block = (s_block_ptr)sbrk(0); 
      int sb = (int)sbrk(s + S_BLOCK_SIZE);                         
    if (sb < 0)      
       {                                                       
        return (NULL);
      }                                                         
    new_block->size = s;                                  
    new_block->ptr  = new_block->data;                       
    new_block->free = TRUE;                                 
     
    if (base == NULL)                       
    {
        new_block->prev = new_block->prev = NULL; 
        base = new_block;                         
    }
    else 
    {
        ASSERT(last->next == NULL);               
        new_block->next = NULL;                    
        new_block->prev = last;                    
        last->next = new_block;                   
    }
    printf("%x %x %d\n", new_block, new_block->ptr, new_block->size); 
    return new_block;                             
}

void split_block(s_block_ptr p, size_t new_size)
{
    s_block_ptr new_block = NULL;              
    if (p->size >= new_size + S_BLOCK_SIZE + 4)
    {
       
        p->size = new_size;  

       
        new_block = (s_block_ptr)(p->data + new_size);
        new_block->size = p->size - new_size - S_BLOCK_SIZE;
        new_block->ptr = new_block->data;
        new_block->free = TRUE;

        
        new_block->next = p->next;
        new_block->prev = p;
        if (p->next)
            p->next->prev = new_block;
        p->next = new_block;
    }

    
}

static s_block_ptr fusion_block(s_block_ptr pb)
{
    ASSERT(pb->free == TRUE); 

    
    if (pb->next && pb->next->free) 
    {
      pb->size = pb->size + BLOCK_SIZE + pb->next->size;
      if (pb->next->next)
        pb->next->next->prev = pb;
      pb->next = pb->next->next; 		
    }

    return (pb);
}


static s_block_ptr get_block(void *p)
{
    
    char *tmp;
    tmp = (char*)p;
    return (s_block_ptr)(tmp - BLOCK_SIZE);
}

static int is_valid_block_addr (void *p)
{
   s_block_ptr pb = get_block(p);
   if (base)
   {
     if(p > base && p < sbrk(0))
	return pb->ptr == p;
   }

   return FALSE;
}

static void copy_block(s_block_ptr src, s_block_ptr dst)
{
  int *sdata, *ddata; 
  size_t i;
  sdata = src->ptr;
  ddata = dst->ptr;
  for (i = 0; i * 4 < src->size && i * 4 < dst->size; i++)
    ddata[i] = sdata[i];
}

/*********************** Interfaces ***********************/
void* mm_malloc(size_t size)
{
#ifdef MM_USE_STUBS
    return calloc(1, size);
#else
    
    size_t s = align4(size);    
    s_block_ptr pb;

    
    if (base == NULL) 
    {
       pb = extend_heap(s);
       if (pb == NULL)
          return NULL;
       base = pb;
    }
    
    else 
    {
       
       pb = find_block(s);
       if (pb == NULL)
        {
          pb = extend_heap(s);
          if (pb == NULL)
            return NULL;
        }
       else
        {
          if (pb->size - s >= S_BLOCK_SIZE + 4)
            split_block(pb, s);
        }
    }
        
    pb->free = FALSE;
    return pb->ptr;
#endif
}

void* mm_realloc(void* ptr, size_t size)
{
#ifdef MM_USE_STUBS
    return realloc(ptr, size);
#else
    size_t s;
    void *newp;
    s_block_ptr pb, new;
    
    if (ptr == NULL)
        return mm_malloc(size);
    
    if ((pb=get_block(ptr)) != NULL)
    {
        s = align4(size);
        if (pb->size >= s)
         {
           
           if (pb->size - s >= (BLOCK_SIZE + 4))
             split_block(pb, s);
         }
    }
    else 
    {
        
        if (pb->next && pb->next->free
            && (pb->size + BLOCK_SIZE + pb->next->size) >= s)
        {
           fusion_block(pb);
           if (pb->size - s >= BLOCK_SIZE + 4)
             split_block(pb, s);
        }
        else
        {
           
           newp = mm_malloc(s);
           if (newp == NULL)
               return NULL;
           new = get_block(newp);
           copy_block(pb, new);
           mm_free(pb);
           return (newp);  
        }
        return (pb);
    } 
    return (NULL);
#endif
}
void mm_free(void* ptr)
{
#ifdef MM_USE_STUBS
    free(ptr);
#else
    
    s_block_ptr pb;
    
    if ((pb=get_block(ptr)) != NULL)
    {
       pb->free = TRUE;  

       if(pb->prev && pb->prev->free)
         fusion_block(pb->prev);
       
       if(pb->next)
         fusion_block(pb);
       
       else
       {
          
          if (pb->prev == NULL)
              base = NULL;
          
          else
              pb->prev->next = NULL;     
          brk(pb);
       }
    }
    
#endif
}
