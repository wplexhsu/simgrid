/* mm_diff - Memory snapshooting and comparison                             */

/* Copyright (c) 2008-2012. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "xbt/ex_interface.h" /* internals of backtrace setup */

extern char *xbt_binary_name;

void mmalloc_backtrace_display(xbt_mheap_t mdp, void *ptr){
  size_t block = BLOCK(ptr);
  int type;
  xbt_ex_t e;

  if ((char *) ptr < (char *) mdp->heapbase || block > mdp->heapsize) {
    fprintf(stderr,"Ouch, this pointer is not mine. I cannot display its backtrace. I refuse it to death!!\n");
    abort();
  }

  type = mdp->heapinfo[block].type;

  if (type != 0) {
    //fprintf(stderr,"Only full blocks are backtraced for now. Ignoring your request.\n");
    return;
  }
  if (mdp->heapinfo[block].busy_block.bt_size == 0) {
    fprintf(stderr,"No backtrace available for that block, sorry.\n");
    return;
  }

  memcpy(&e.bt,&(mdp->heapinfo[block].busy_block.bt),sizeof(void*)*XBT_BACKTRACE_SIZE);
  e.used = mdp->heapinfo[block].busy_block.bt_size;

  xbt_ex_setup_backtrace(&e);
  if (e.used == 0) {
    fprintf(stderr, "(backtrace not set)\n");
  } else if (e.bt_strings == NULL) {
    fprintf(stderr, "(backtrace not ready to be computed. %s)\n",xbt_binary_name?"Dunno why":"xbt_binary_name not setup yet");
  } else {
    int i;

    fprintf(stderr, "Backtrace of where the block %p where malloced (%d frames):\n",ptr,e.used);
    for (i = 0; i < e.used; i++)       /* no need to display "xbt_backtrace_display" */{
      fprintf(stderr,"%d",i);fflush(NULL);
      fprintf(stderr, "---> %s\n", e.bt_strings[i] + 4);
    }
  }
}

int mmalloc_compare_heap(xbt_mheap_t mdp1, xbt_mheap_t mdp2, void *std_heap_addr){

  if(mdp1 == NULL && mdp2 == NULL){
    XBT_DEBUG("Malloc descriptors null");
    return 0;
  }

  /* Heapstats */

  int errors = mmalloc_compare_mdesc(mdp1, mdp2, std_heap_addr);

  return (errors > 0);

}

int mmalloc_compare_mdesc(struct mdesc *mdp1, struct mdesc *mdp2, void *std_heap_addr){

  int errors = 0;

  if(mdp1->headersize != mdp2->headersize){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different size of the file header for the mapped files");
      errors++;
    }else{
      return 1;
    }
  }

  if(mdp1->refcount != mdp2->refcount){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different number of processes that attached the heap");
      errors++;
    }else{
      return 1;
    }
  }

  if(strcmp(mdp1->magic, mdp2->magic) != 0){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different magic number");
      errors++;
    }else{
      return 1;
    }
  }

  if(mdp1->flags != mdp2->flags){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different flags");
      errors++;
    }else{
      return 1;
    }
  }

  if(mdp1->heapsize != mdp2->heapsize){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different number of info entries");
      errors++;
    }else{
      return 1;
    }
  }

  //XBT_DEBUG("Heap size : %zu", mdp1->heapsize);

  if(mdp1->heapbase != mdp2->heapbase){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different first block of the heap");
      errors++;
    }else{
      return 1;
    }
  }


  if(mdp1->heapindex != mdp2->heapindex){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different index for the heap table : %zu - %zu", mdp1->heapindex, mdp2->heapindex);
      errors++;
    }else{
      return 1;
    }
  }

  //XBT_DEBUG("Heap index : %zu", mdp1->heapindex);

  if(mdp1->base != mdp2->base){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different base address of the memory region");
      errors++;
    }else{
      return 1;
    }
  }

  if(mdp1->breakval != mdp2->breakval){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different current location in the memory region");
      errors++;
    }else{
      return 1;
    }
  }

  if(mdp1->top != mdp2->top){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different end of the current location in the memory region");
      errors++;
    }else{
      return 1;
    }
  }

  if(mdp1->heaplimit != mdp2->heaplimit){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different limit of valid info table indices");
      errors++;
    }else{
      return 1;
    }
  }

  if(mdp1->fd != mdp2->fd){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different file descriptor for the file to which this malloc heap is mapped");
      errors++;
    }else{
      return 1;
    }
  }

  if(mdp1->version != mdp2->version){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different version of the mmalloc package");
      errors++;
    }else{
      return 1;
    }
  }


  size_t block_free1, block_free2 , next_block_free, first_block_free, block_free ;
  size_t i, j;
  void *addr_block1, *addr_block2;
  size_t frag_size;


  /* Search index of the first free block */

  block_free1 = mdp1->heapindex;
  block_free2 = mdp2->heapindex;

  while(mdp1->heapinfo[block_free1].free_block.prev != 0){
    block_free1 = mdp1->heapinfo[block_free1].free_block.prev;
  }

  while(mdp2->heapinfo[block_free2].free_block.prev != 0){
    block_free2 = mdp1->heapinfo[block_free2].free_block.prev;
  }

  if(block_free1 !=  block_free2){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different first free block");
      errors++;
    }else{
      return 1;
    }
  }

  first_block_free = block_free1;

  if(mdp1->heapinfo[first_block_free].free_block.size != mdp2->heapinfo[first_block_free].free_block.size){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different size (in blocks) of the first free cluster");
      errors++;
    }else{
      return 1;
    }
  }

  /* Check busy blocks (circular checking)*/

  i = first_block_free + mdp1->heapinfo[first_block_free].free_block.size;

  if(mdp1->heapinfo[first_block_free].free_block.next != mdp2->heapinfo[first_block_free].free_block.next){
    if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
      XBT_DEBUG("Different next block free");
      errors++;
    }else{
      return 1;
    }
  }

  block_free = first_block_free;
  next_block_free = mdp1->heapinfo[first_block_free].free_block.next;

  if(next_block_free == 0)
    next_block_free = mdp1->heaplimit;

  while(i != first_block_free){

    while(i<next_block_free){

      if(mdp1->heapinfo[i].type != mdp2->heapinfo[i].type){
        if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
          XBT_DEBUG("Different type of busy block");
          errors++;
        }else{
          return 1;
        }
      }else{

        addr_block1 = (char *)mdp1 + sizeof(struct mdesc) + ((i-1) * BLOCKSIZE);
        addr_block2 = (char *)mdp2 + sizeof(struct mdesc) + ((i-1) * BLOCKSIZE);

        switch(mdp1->heapinfo[i].type){ //FIXME deal with type<0 == free
        case 0 :
          if(mdp1->heapinfo[i].busy_block.size != mdp2->heapinfo[i].busy_block.size){
            if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
              XBT_DEBUG("Different size of a large cluster");
              errors++;
            }else{
              return 1;
            }
          }else{
            if(memcmp(addr_block1, addr_block2, (mdp1->heapinfo[i].busy_block.size * BLOCKSIZE)) != 0){
              if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
                XBT_DEBUG("Different data in block %zu (size = %zu) (addr_block1 = %p (current = %p) - addr_block2 = %p)", i, mdp1->heapinfo[i].busy_block.size, addr_block1, (char *)std_heap_addr + sizeof(struct mdesc) + ((i-1) * BLOCKSIZE), addr_block2);
                errors++;
              }else{
                return 1;
              }
            }
          }
          i = i+mdp1->heapinfo[i].busy_block.size;

          break;
        default :
          if(mdp1->heapinfo[i].busy_frag.nfree != mdp2->heapinfo[i].busy_frag.nfree){
            if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
              XBT_DEBUG("Different free fragments in the fragmented block %zu", i);
              errors++;
            }else{
              return 1;
            }
          }else{
            if(mdp1->heapinfo[i].busy_frag.first != mdp2->heapinfo[i].busy_frag.first){
              if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
                XBT_DEBUG("Different first free fragments in the block %zu", i);
                errors++;
              }else{
                return 1;
              }
            }else{
              frag_size = pow(2,mdp1->heapinfo[i].type);
              for(j=0 ; j< (BLOCKSIZE/frag_size); j++){
                if(memcmp((char *)addr_block1 + (j * frag_size), (char *)addr_block2 + (j * frag_size), frag_size) != 0){
                  if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
                    XBT_DEBUG("Different data in fragment %zu (addr_frag1 = %p - addr_frag2 = %p) of block %zu", j + 1, (char *)addr_block1 + (j * frag_size), (char *)addr_block2 + (j * frag_size),  i);
                    errors++;
                  }else{
                    return 1;
                  }
                }
              }
            }
          }

          i++;

          break;
        }

      }
    }

    if( i != first_block_free){

      if(mdp1->heapinfo[block_free].free_block.next != mdp2->heapinfo[block_free].free_block.next){
        if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
          XBT_DEBUG("Different next block free");
          errors++;
        }else{
          return 1;
        }
      }

      block_free = mdp1->heapinfo[block_free].free_block.next;
      next_block_free = mdp1->heapinfo[block_free].free_block.next;

      i = block_free + mdp1->heapinfo[block_free].free_block.size;

      if((next_block_free == 0) && (i != mdp1->heaplimit)){

        while(i < mdp1->heaplimit){

          if(mdp1->heapinfo[i].type != mdp2->heapinfo[i].type){
            if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
              XBT_DEBUG("Different type of busy block");
              errors++;
            }else{
              return 1;
            }
          }else{

            addr_block1 = (char *)mdp1 + sizeof(struct mdesc) + ((i-1) * BLOCKSIZE);
            addr_block2 = (char *)mdp2 + sizeof(struct mdesc) + ((i-1) * BLOCKSIZE);

            switch(mdp1->heapinfo[i].type){
            case 0 :
              if(mdp1->heapinfo[i].busy_block.size != mdp2->heapinfo[i].busy_block.size){
                if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
                  XBT_DEBUG("Different size of a large cluster");
                  errors++;
                }else{
                  return 1;
                }
              }else{
                if(memcmp(addr_block1, addr_block2, (mdp1->heapinfo[i].busy_block.size * BLOCKSIZE)) != 0){
                  if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
                    XBT_DEBUG("Different data in block %zu (addr_block1 = %p (current = %p) - addr_block2 = %p)", i, addr_block1, (char *)std_heap_addr + sizeof(struct mdesc) + ((i-1) * BLOCKSIZE), addr_block2);
                    errors++;
                  }else{
                    return 1;
                  }
                }
              }

              i = i+mdp1->heapinfo[i].busy_block.size;

              break;
            default :
              if(mdp1->heapinfo[i].busy_frag.nfree != mdp2->heapinfo[i].busy_frag.nfree){
                if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
                  XBT_DEBUG("Different free fragments in the fragmented block %zu", i);
                  errors++;
                }else{
                  return 1;
                }
              }else{
                if(mdp1->heapinfo[i].busy_frag.first != mdp2->heapinfo[i].busy_frag.first){
                  if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
                    XBT_DEBUG("Different first free fragments in the block %zu", i);
                    errors++;
                  }else{
                    return 1;
                  }
                }else{
                  frag_size = pow(2,mdp1->heapinfo[i].type);
                  for(j=0 ; j< (BLOCKSIZE/frag_size); j++){
                    if(memcmp((char *)addr_block1 + (j * frag_size), (char *)addr_block2 + (j * frag_size), frag_size) != 0){
                      if(XBT_LOG_ISENABLED(xbt_mm_legacy, xbt_log_priority_debug)){
                        XBT_DEBUG("Different data in fragment %zu (addr_frag1 = %p - addr_frag2 = %p) of block %zu", j + 1, (char *)addr_block1 + (j * frag_size), (char *)addr_block2 + (j * frag_size),  i);
                        errors++;
                      }else{
                        return 1;
                      }
                    }
                  }
                }
              }

              i++;

              break;
            }
          }
        }

      }

    }
  }

  return (errors>0);
}


void mmalloc_display_info_heap(xbt_mheap_t h){

}


