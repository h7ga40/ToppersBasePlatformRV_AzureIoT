/*
 *  TOPPERS BASE PLATFORM MIDDLEWARE
 * 
 *  Copyright (C) 2017-2019 by TOPPERS PROJECT
 *                             Educational Working Group.
 * 
 *  上記著作権者は，以下の (1)～(4) の条件か，Free Software Foundation 
 *  によって公表されている GNU General Public License の Version 2 に記
 *  述されている条件を満たす場合に限り，本ソフトウェア（本ソフトウェア
 *  を改変したものを含む．以下同じ）を使用・複製・改変・再配布（以下，
 *  利用と呼ぶ）することを無償で許諾する．
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
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，その適用可能性も
 *  含めて，いかなる保証も行わない．また，本ソフトウェアの利用により直
 *  接的または間接的に生じたいかなる損害に関しても，その責任を負わない．
 * 
 *  @(#) $Id$
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include <stdio.h>
#include <string.h>
#include <target_syssvc.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "device.h"
#include <fcntl.h>
#include "storagedevice.h"
#include "rom_file.h"


static int           num_files = 0;
static int           dirreadcount;
static fs_descrip_t  fs_descrip_table[MAX_DESCRIP];

static void  *romf_opendir(const char *pathname);
static int   romf_closedir(void *dir);
static int   romf_readdir(void *dir, void *pdirent);
static int   romf_open(int devno, const char *pathname, int flags);
static int   romf_close(int fd);
static int   romf_fstat(int fd, struct stat *buf);
static off_t romf_lseek(int fd, off_t offset, int whence, int *res);
static long  romf_read(int fd, void *buf, long count, int *res);

static const StorageDeviceFileFunc_t romfSDeviceFunc = {
	romf_opendir,
	romf_closedir,
	romf_readdir,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	romf_open,
	romf_close,
	romf_fstat,
	romf_lseek,
	romf_read,
	0,
	0
};


/*
 *  FLASH-ROMファイル関数郡の初期化
 */
ER
rom_file_init(void)
{
	StorageDevice_t *psdev;
	int result;

	num_files = 0;
	memset(fs_descrip_table, 0, sizeof(fs_descrip_table));
	result = SDMSetupDevice(DEFAULT_DEVNO, &psdev);
	if(result == E_OK){
		psdev->pdevff           = (StorageDeviceFileFunc_t *)&romfSDeviceFunc;
		psdev->_sdev_attribute |= SDEV_EMPLOY;
		return MAX_DESCRIP;
	}
	else
		return 0;
}

/*
 *  ROMファイルの生成
 */
int
create_rom_file(const char *pathname, uintptr_t start, uint32_t size)
{
	fs_descrip_t  *fdp;
	int           cd = -1;
	int           i;

	dis_dsp();			/* disable dispatch */
	for(i = 0 ; i < MAX_DESCRIP ; i++){
		fdp = &fs_descrip_table[i];
		if(fdp->create == 0){
			fdp->create   = 1;
			fdp->inuse    = 0;
			fdp->fp       = 0;
			fdp->addr     = start;
			fdp->size     = size;
			fdp->filename = pathname;
			cd = i;
			num_files++;
			break;
		}
	}
	ena_dsp();			/* enable dispatch */
	return cd;
}

/*
 *  FLASH-ROMファイルディレクトリオープン
 */
static void *
romf_opendir(const char *pathname)
{
	dirreadcount = 0;
	return (void *)&num_files;
}

/*
 *  ROMファイルディレクトリクローズ
 */
static int
romf_closedir(void *dir)
{
	return 0;
}

/*
 *  ROMファイルディレクトリ読み込み
 */
static int
romf_readdir(void *dir, void *pdirent)
{
	fs_descrip_t   *pf;
	struct dirent2 *pd = pdirent;
	unsigned int   i;

	if(dirreadcount >= num_files)
		return 0;
	else{
		pf = &fs_descrip_table[dirreadcount];
		pd->d_fsize = pf->size;
		pd->d_date  = 0;
		pd->d_time  = 0;
		pd->d_type  = AM_RDO;
		for(i = 0 ; i < 255 && pf->filename[i] != 0 ; i++){
			pd->d_name[i] = pf->filename[i];
		}
		pd->d_name[i] = 0;
		dirreadcount++;
		return dirreadcount;
	}
}

/*
 *  ROMファイルオープン
 */
static int
romf_open(int devno, const char *pathname, int flags)
{
	fs_descrip_t  *fdp = 0;
	int           fd;

	SDMGetDeviceNo(&pathname);
	if(*pathname == '/')
		pathname++;
	dis_dsp();			/* disable dispatch */
	for(fd = 0 ; fd < num_files ; fd++){
		fdp = &fs_descrip_table[fd];
		if(fdp->create && strcmp(pathname, fdp->filename) == 0){
			break;
		}
	}
	if(fd < num_files && (flags & 0x0f) == O_RDONLY && fdp->inuse == 0){
		fdp->inuse = 1;
		fdp->fp = 0;
		ena_dsp();			/* enable dispatch */
		return fd;
	}
	else{
		ena_dsp();			/* enable dispatch */
		return -1;
	}
}

/*
 *  ROMファイルクローズ
 */
static int
romf_close(int fd)
{
	fs_descrip_t  *fdp;

	if(fd < 0 || num_files < fd)
		return -1;
	fdp = &fs_descrip_table[fd];
	if(fdp->inuse == 1){
		fdp->inuse = 0;
		return 0;
	}
	else
		return -1;
}

/*
 *  ROMファイルステート取得
 */
static int
romf_fstat(int fd, struct stat *buf)
{
	fs_descrip_t  *fdp;

	if(fd < 0 || num_files < fd)
		return -1;
	fdp = &fs_descrip_table[fd];
	if(fdp->create == 1){
		memset(buf, 0, sizeof(struct stat));
		buf->st_size = fdp->size;
		return 0;
	}
	else
		return -1;
}

/*
 *  ROMファイルシーク
 */
static off_t
romf_lseek(int fd, off_t offset, int whence, int *res)
{
	fs_descrip_t  *fdp;

	*res = -1;
	if(fd < 0 || num_files < fd)
		return -1;
	fdp = &fs_descrip_table[fd];
	if(fdp->inuse == 1){
		switch (whence) {
		case SEEK_SET:
			fdp->fp = offset;
			break;
		case SEEK_CUR:
			fdp->fp += offset;
			break;
		case SEEK_END:
			fdp->fp = fdp->size + offset;
			break;
		}
		offset = fdp->fp;
		*res = 0;
		return offset;
	}
	else
		return -1;
}

/*
 *  ROMファイル読みだし
 */
static long
romf_read(int fd, void *buf, long count, int *res)
{
	fs_descrip_t*fdp;
	int         readsize;

	*res = -1;
	if(fd < 0 || num_files < fd)
		return -1;
	fdp = &fs_descrip_table[fd];
	if(fdp->inuse == 1){
		if(fdp->fp < fdp->size){
			if((fdp->fp + count) <= fdp->size)
				readsize = count;	/* all counts can be read */
			else
				readsize = fdp->size - fdp->fp;	/* a part of counts can be read */
			memcpy(buf, ((uint8_t *)fdp->addr + fdp->fp), readsize);
			buf += readsize;
			fdp->fp += readsize;
		}
		else
			readsize = 0;	/* no data can be read */
		*res = 0;
		return readsize;
	}
	else
		return -1;
}

