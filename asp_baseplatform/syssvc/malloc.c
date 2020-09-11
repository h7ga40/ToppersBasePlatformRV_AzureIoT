/*
 *  TOPPERS/ASP/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2008-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *  Copyright (C) 2015-2020 by TOPPERS PROJECT Educational Working Group.
 * 
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id$
 */
/*
 * 
 *  メモリアロケーション機能
 *
 */
#include <kernel.h>
#include <t_syslog.h>
#include <stdlib.h>
#include <string.h>
#include "kernel_cfg.h"

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE	    32	/* default cacle line size */
#endif

#if defined(TLSF_SEM)	/* TLSF */
#include "tlsf.h"

/*
 *  TLSF2.4.6には、アロケート、フリーを何度も繰り返すと
 *  アロケート領域の先頭８バイトが破壊される問題がある.
 *  TLSF側の修正は品質保証できないため、malloc/free関数で
 *  対応を行う.
 */
#define TLSF_PTR_AREA_SIZE  8

static void *memory_pool;

/*
 *  プール初期化
 */
void
heap_init(intptr_t exinf)
{
	intptr_t *param = (intptr_t *)exinf;
	uint32_t *p;
	int i, size;
	memory_pool = (void*)param[0];
	p = (uint32_t *)param[0];
	size = param[1] / sizeof(uint32_t);
	for(i = 0 ; i < size ; i++)
		*p++ = 0;
	init_memory_pool(param[1], (void*)memory_pool);
}

/*
 *  MALLOC
 */
void *
__wrap__malloc_r(struct _reent *reent, size_t __size)
{
	void *mempool, *addr;
	mempool = (void *)memory_pool;
	if(mempool == NULL)
		return NULL;
	wai_sem(TLSF_SEM);
	addr = (void*)malloc_ex(__size+TLSF_PTR_AREA_SIZE, mempool);
	sig_sem(TLSF_SEM);
	if(addr != NULL)
		return addr+TLSF_PTR_AREA_SIZE;
	else
		return addr;
}

/*
 *  CALLOC
 */
void *
__wrap__calloc_r(struct _reent *reent, size_t __nmemb, size_t __size)
{
	void *mempool, *addr;
	mempool = (void *)memory_pool;
	if(mempool == NULL)
		return NULL;
	wai_sem(TLSF_SEM);
	addr = (void*)malloc_ex((__nmemb * __size)+TLSF_PTR_AREA_SIZE, mempool);
	sig_sem(TLSF_SEM);
	if(addr != NULL){
		memset(addr+TLSF_PTR_AREA_SIZE, 0, __nmemb * __size);
		return addr+TLSF_PTR_AREA_SIZE;
	}
	else
		return addr;
}

/*
 *  REALLOC
 */
void *__wrap__realloc_r(struct _reent *reent, void *ptr, size_t __size)
{
	void *mempool, *addr;
	mempool = (void *)memory_pool;
	if(mempool == NULL)
		return NULL;
	wai_sem(TLSF_SEM);
	addr = (void*)realloc_ex(ptr, __size+TLSF_PTR_AREA_SIZE, mempool);
	sig_sem(TLSF_SEM);
	if(addr != NULL)
		return addr+TLSF_PTR_AREA_SIZE;
	else
		return addr;
}

/*
 *  FREE
 */
void
__wrap__free_r(struct _reent *reent, void *ptr)
{
	void *mempool = memory_pool;
	if(ptr == NULL)
		return;
	if(mempool != NULL){
		wai_sem(TLSF_SEM);
		free_ex(ptr-TLSF_PTR_AREA_SIZE, mempool);
		sig_sem(TLSF_SEM);
	}
}

#elif(LOCAL_MALLOC)		/* LOCAL MALLOC */

typedef struct _LMAP LMAP_t;
struct _LMAP {
  unsigned long lsize;		/* メモリサイズ */
  LMAP_t *      laddr;		/* メモリスタートポインタ */
};

static LMAP_t lmap;

/*
 *  ローカルアロケータ初期化
 */
void
heap_init(intptr_t exinf)
{
	intptr_t *param = (intptr_t *)exinf;
	LMAP_t *plmap;

	plmap = (LMAP_t *)param[0];
	plmap->lsize = (unsigned long)param[1];
	plmap->laddr = NULL;
	lmap.lsize   = plmap->lsize;
	lmap.laddr   = plmap;
}

/*
 *  MALLOC
 */
void *
__wrap__malloc_r(struct _reent *reent, size_t __size)
{
	LMAP_t *ph = &lmap;
	LMAP_t *p;

	__size = (__size+3+sizeof(unsigned int)) & ~3;
	if(__size < sizeof(LMAP_t))
		__size = sizeof(LMAP_t);

	loc_cpu();
	while((p = ph->laddr) != 0){
		if(p->lsize >= __size && p->lsize < (__size+sizeof(LMAP_t))){
			ph->laddr = p->laddr;
			unl_cpu();
			return (void *)(&p->laddr);
		}
		else if(p->lsize >= (__size+sizeof(LMAP_t))){
			LMAP_t *pw = (LMAP_t *)((char *)p+__size);
			pw->lsize = p->lsize-__size;
			pw->laddr = p->laddr;
			ph->laddr = pw;
			p->lsize  = __size;
			unl_cpu();
			return (void *)(&p->laddr);
		}
		ph = p;
	}
	unl_cpu();
	return NULL;
}

/*
 *  CALLOC
 */
void *
__wrap__calloc_r(struct _reent *reent, size_t __nmemb, size_t __size)
{
	return __wrap__malloc_r(reent, __nmemb * __size);
}

/*
 *  FREE
 */
void
__wrap__free_r(struct _reent *reent, void *ptr)
{
	LMAP_t *ph = &lmap;
	LMAP_t *p;
	char   *t, *pf;

	if(ptr == NULL)
		return;
	p  = (LMAP_t *)((char *)ptr-sizeof(unsigned long));
	t  = (char *)p+p->lsize;
	loc_cpu();
	for(;;){
		if(ph->laddr == NULL){		/* 空きメモリなし */
			p->laddr  = NULL;
			ph->laddr = p;
			break;
		}
		else if(t == (char *)ph->laddr){
			p->lsize += (ph->laddr)->lsize;
			p->laddr  = (ph->laddr)->laddr;
			ph->laddr = p;
			break;
		}
		else if(t < (char *)ph->laddr){
			p->laddr = ph->laddr;
			ph->laddr = p;
			break;
		}
		ph = ph->laddr;
		pf = (char *)ph+ph->lsize;
		if(pf > (char *)p){			/* 領域オーバーラップ */
			unl_cpu();
			syslog_0(LOG_EMERG, "free:FATAL:irrguler free memory !");
			return;
		}
		else if(pf == (char *)p){
			ph->lsize += p->lsize;
			if(t > (char *)ph->laddr){	/* 領域オーバーラップ */
				unl_cpu();
				syslog_0(LOG_EMERG, "free:FATAL:irrguler free memory !");
				return;
			}
			else if(t == (char *)ph->laddr){
				ph->lsize += (ph->laddr)->lsize;
				ph->laddr  = (ph->laddr)->laddr;
			}
			break;
		}
	}
	unl_cpu();
}

#else		/* LIBC MALLOC */
#include <kernel_impl.h>

static uintptr_t *heap_param;
static int malloc_lock_count;

/*
 *  LIBCアロケータ初期化
 */
void
heap_init(intptr_t exinf)
{
	heap_param = (uintptr_t *)exinf;
	malloc_lock_count = 0;
}

void
__malloc_lock(void * reent)
{
	malloc_lock_count++;
	if (malloc_lock_count == 1){
		ER ret;
		if (sense_context())
			//ret = iloc_cpu();
			ret = E_CTX;
		else
			ret = wai_sem(MALLOC_SEM);
		if (ret != E_OK)
			syslog_1(LOG_ERROR, "__malloc_unlock: count[%d] loc_cpu", malloc_lock_count);
	}
}

void
__malloc_unlock(void * reent)
{
	malloc_lock_count--;
	if (malloc_lock_count == 0){
		ER ret;
		if (sense_context())
			//ret = iunl_cpu();
			ret = E_CTX;
		else
			ret = sig_sem(MALLOC_SEM);
		if (ret != E_OK)
			syslog_1(LOG_ERROR, "__malloc_unlock: count[%d] unl_cpu error !", malloc_lock_count);
	}
}

void *
_sbrk(int incr)
{ 
	static char *heap_end = NULL;
	char        *prev_heap_end;

	if(incr < 0 || incr > heap_param[1]){
		syslog_2(LOG_ERROR, "_sbrk: incr[%d] parameter error ! %d", incr, malloc_lock_count);
		return (void *)-1;
	}

	if(heap_end == NULL)
		heap_end = (char *)heap_param[0];

	prev_heap_end  = heap_end;
	heap_end      += incr;

	if(heap_end < (char *)(heap_param[0]+heap_param[1]))
		return (void *) prev_heap_end;
	else{
		heap_end = prev_heap_end;
		syslog_2(LOG_ERROR, "_sbrk: incr[%d] allocation error ! %d", incr, malloc_lock_count);
		return (void *)-1;
	}
}

#endif

/*
 *  CACHE ALINE MALLOC
 */
void *
malloc_cache(uint32_t len)
{
	void * addr = malloc(len+CACHE_LINE_SIZE);
	uint32_t *p;
	intptr_t uaddr;

	if(addr == NULL)
		return addr;
	p = addr;
	uaddr = (intptr_t)addr;
	if((uaddr & (CACHE_LINE_SIZE-1)) != 0)
		uaddr &= ~(CACHE_LINE_SIZE-1);
	uaddr += CACHE_LINE_SIZE;
	p = (uint32_t *)uaddr;
	*(p-1) = (intptr_t)addr;
	return p;
}

/*
 *  CACHE ALINE FREE
 */
void
free_cache(void *addr)
{
	intptr_t *p = addr;

	if(addr == NULL)
		return;
	addr = (void*)(*(p-1));
	free(addr);
}

