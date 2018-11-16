/*
 * Simple MTD partitioning layer
 *
 * Copyright © 2000 Nicolas Pitre <nico@fluxnic.net>
 * Copyright © 2002 Thomas Gleixner <gleixner@linutronix.de>
 * Copyright © 2000-2010 David Woodhouse <dwmw2@infradead.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/kmod.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#if defined(CONFIG_SUPPORT_OPENWRT)
#include <linux/magic.h>
#endif
#include <linux/err.h>

#include "mtdcore.h"
#if defined(CONFIG_SUPPORT_OPENWRT)
#include "mtdsplit.h"
#define MTD_ERASE_PARTIAL	0x8000 /* partition only covers parts of an erase block */
#endif




/* Our partition linked list */
static LIST_HEAD(mtd_partitions);
static DEFINE_MUTEX(mtd_partitions_mutex);

/* Our partition node structure */
struct mtd_part {
	struct mtd_info mtd;
	struct mtd_info *master;
	uint64_t offset;
	struct list_head list;
};

#if defined(CONFIG_SUPPORT_OPENWRT)
static int bad_detected;
static void mtd_partition_split(struct mtd_info *master, struct mtd_part *part);
#endif

/*
 * Given a pointer to the MTD object in the mtd_part structure, we can retrieve
 * the pointer to that structure with this macro.
 */
#define PART(x)  ((struct mtd_part *)(x))


/*
 * MTD methods which simply translate the effective address and pass through
 * to the _real_ device.
 */

static int part_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	struct mtd_ecc_stats stats;
	int res;

	stats = part->master->ecc_stats;
	res = part->master->_read(part->master, from + part->offset, len,
				  retlen, buf);
	if (unlikely(mtd_is_eccerr(res)))
		mtd->ecc_stats.failed +=
			part->master->ecc_stats.failed - stats.failed;
	else
		mtd->ecc_stats.corrected +=
			part->master->ecc_stats.corrected - stats.corrected;
	return res;
}

static int part_point(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, void **virt, resource_size_t *phys)
{
	struct mtd_part *part = PART(mtd);

	return part->master->_point(part->master, from + part->offset, len,
				    retlen, virt, phys);
}

static int part_unpoint(struct mtd_info *mtd, loff_t from, size_t len)
{
	struct mtd_part *part = PART(mtd);

	return part->master->_unpoint(part->master, from + part->offset, len);
}

static unsigned long part_get_unmapped_area(struct mtd_info *mtd,
					    unsigned long len,
					    unsigned long offset,
					    unsigned long flags)
{
	struct mtd_part *part = PART(mtd);

	offset += part->offset;
	return part->master->_get_unmapped_area(part->master, len, offset,
						flags);
}

static int part_read_oob(struct mtd_info *mtd, loff_t from,
		struct mtd_oob_ops *ops)
{
	struct mtd_part *part = PART(mtd);
	int res;

	if (from >= mtd->size)
		return -EINVAL;
	if (ops->datbuf && from + ops->len > mtd->size)
		return -EINVAL;

	/*
	 * If OOB is also requested, make sure that we do not read past the end
	 * of this partition.
	 */
	if (ops->oobbuf) {
		size_t len, pages;

		if (ops->mode == MTD_OPS_AUTO_OOB)
			len = mtd->oobavail;
		else
			len = mtd->oobsize;
		pages = mtd_div_by_ws(mtd->size, mtd);
		pages -= mtd_div_by_ws(from, mtd);
		if (ops->ooboffs + ops->ooblen > pages * len)
			return -EINVAL;
	}

	res = part->master->_read_oob(part->master, from + part->offset, ops);
	if (unlikely(res)) {
		if (mtd_is_bitflip(res))
			mtd->ecc_stats.corrected++;
		if (mtd_is_eccerr(res))
			mtd->ecc_stats.failed++;
	}
	return res;
}

static int part_read_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_read_user_prot_reg(part->master, from, len,
						 retlen, buf);
}

static int part_get_user_prot_info(struct mtd_info *mtd,
		struct otp_info *buf, size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_get_user_prot_info(part->master, buf, len);
}

static int part_read_fact_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_read_fact_prot_reg(part->master, from, len,
						 retlen, buf);
}

static int part_get_fact_prot_info(struct mtd_info *mtd, struct otp_info *buf,
		size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_get_fact_prot_info(part->master, buf, len);
}

static int part_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_write(part->master, to + part->offset, len,
				    retlen, buf);
}

static int part_panic_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_panic_write(part->master, to + part->offset, len,
					  retlen, buf);
}

static int part_write_oob(struct mtd_info *mtd, loff_t to,
		struct mtd_oob_ops *ops)
{
	struct mtd_part *part = PART(mtd);

	if (to >= mtd->size)
		return -EINVAL;
	if (ops->datbuf && to + ops->len > mtd->size)
		return -EINVAL;
	return part->master->_write_oob(part->master, to + part->offset, ops);
}

static int part_write_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len, size_t *retlen, u_char *buf)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_write_user_prot_reg(part->master, from, len,
						  retlen, buf);
}

static int part_lock_user_prot_reg(struct mtd_info *mtd, loff_t from,
		size_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_lock_user_prot_reg(part->master, from, len);
}

static int part_writev(struct mtd_info *mtd, const struct kvec *vecs,
		unsigned long count, loff_t to, size_t *retlen)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_writev(part->master, vecs, count,
				     to + part->offset, retlen);
}

static int part_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct mtd_part *part = PART(mtd);
	int ret;

#if defined(CONFIG_SUPPORT_OPENWRT)
	instr->partial_start = false;
	if (mtd->flags & MTD_ERASE_PARTIAL) {
		size_t readlen = 0;
		u64 mtd_ofs;

		instr->erase_buf = kmalloc(part->master->erasesize, GFP_ATOMIC);
		if (!instr->erase_buf)
			return -ENOMEM;

		mtd_ofs = part->offset + instr->addr;
		instr->erase_buf_ofs = do_div(mtd_ofs, part->master->erasesize);

		if (instr->erase_buf_ofs > 0) {
			instr->addr -= instr->erase_buf_ofs;
			ret = mtd_read(part->master,
				instr->addr + part->offset,
				part->master->erasesize,
				&readlen, instr->erase_buf);

			instr->partial_start = true;
		} else {
			mtd_ofs = part->offset + part->mtd.size;
			instr->erase_buf_ofs = part->master->erasesize -
				do_div(mtd_ofs, part->master->erasesize);

			if (instr->erase_buf_ofs > 0) {
				instr->len += instr->erase_buf_ofs;
				ret = mtd_read(part->master,
					part->offset + instr->addr +
					instr->len - part->master->erasesize,
					part->master->erasesize, &readlen,
					instr->erase_buf);
			} else {
				ret = 0;
			}
		}
		if (ret < 0) {
			kfree(instr->erase_buf);
			return ret;
		}

	}
#endif

	instr->addr += part->offset;
	ret = part->master->_erase(part->master, instr);
	if (ret) {
		if (instr->fail_addr != MTD_FAIL_ADDR_UNKNOWN)
			instr->fail_addr -= part->offset;
		instr->addr -= part->offset;
#if defined(CONFIG_SUPPORT_OPENWRT)
		if (mtd->flags & MTD_ERASE_PARTIAL)
			kfree(instr->erase_buf);
#endif
	}

	return ret;
}

void mtd_erase_callback(struct erase_info *instr)
{
	if (instr->mtd->_erase == part_erase) {
		struct mtd_part *part = PART(instr->mtd);
#if defined(CONFIG_SUPPORT_OPENWRT)
		size_t wrlen = 0;

		if (instr->mtd->flags & MTD_ERASE_PARTIAL) {
			if (instr->partial_start) {
				part->master->_write(part->master,
					instr->addr, instr->erase_buf_ofs,
					&wrlen, instr->erase_buf);
				instr->addr += instr->erase_buf_ofs;
			} else {
				instr->len -= instr->erase_buf_ofs;
				part->master->_write(part->master,
					instr->addr + instr->len,
					instr->erase_buf_ofs, &wrlen,
					instr->erase_buf +
					part->master->erasesize -
					instr->erase_buf_ofs);
			}
			kfree(instr->erase_buf);
		}
#endif
		if (instr->fail_addr != MTD_FAIL_ADDR_UNKNOWN)
			instr->fail_addr -= part->offset;
		instr->addr -= part->offset;
	}
	if (instr->callback)
		instr->callback(instr);
}
EXPORT_SYMBOL_GPL(mtd_erase_callback);

static int part_lock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_lock(part->master, ofs + part->offset, len);
}

static int part_unlock(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct mtd_part *part = PART(mtd);
#if defined(CONFIG_SUPPORT_OPENWRT)
	ofs += part->offset;
	if (mtd->flags & MTD_ERASE_PARTIAL) {
		/* round up len to next erasesize and round down offset to prev block */
		len = (mtd_div_by_eb(len, part->master) + 1) * part->master->erasesize;
		ofs &= ~(part->master->erasesize - 1);
	}
	return part->master->_unlock(part->master, ofs, len);
#else
	return part->master->_unlock(part->master, ofs + part->offset, len);
#endif
}

static int part_is_locked(struct mtd_info *mtd, loff_t ofs, uint64_t len)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_is_locked(part->master, ofs + part->offset, len);
}

static void part_sync(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	part->master->_sync(part->master);
}

static int part_suspend(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	return part->master->_suspend(part->master);
}

static void part_resume(struct mtd_info *mtd)
{
	struct mtd_part *part = PART(mtd);
	part->master->_resume(part->master);
}

static int part_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = PART(mtd);
	ofs += part->offset;
	return part->master->_block_isbad(part->master, ofs);
}

static int part_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct mtd_part *part = PART(mtd);
	int res;

	ofs += part->offset;
	res = part->master->_block_markbad(part->master, ofs);
	if (!res)
		mtd->ecc_stats.badblocks++;
	return res;
}

static inline void free_partition(struct mtd_part *p)
{
	kfree(p->mtd.name);
	kfree(p);
}

/*
 * This function unregisters and destroy all slave MTD objects which are
 * attached to the given master MTD object.
 */

int del_mtd_partitions(struct mtd_info *master)
{
	struct mtd_part *slave, *next;
	int ret, err = 0;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry_safe(slave, next, &mtd_partitions, list)
		if (slave->master == master) {
			ret = del_mtd_device(&slave->mtd);
			if (ret < 0) {
				err = ret;
				continue;
			}
			list_del(&slave->list);
			free_partition(slave);
		}
	mutex_unlock(&mtd_partitions_mutex);

	return err;
}

static struct mtd_part *allocate_partition(struct mtd_info *master,
			const struct mtd_partition *part, int partno,
			uint64_t cur_offset)
{
	struct mtd_part *slave;
	char *name;

	/* allocate the partition structure */
	slave = kzalloc(sizeof(*slave), GFP_KERNEL);
	name = kstrdup(part->name, GFP_KERNEL);
	if (!name || !slave) {
		printk(KERN_ERR"memory allocation error while creating partitions for \"%s\"\n",
		       master->name);
		kfree(name);
		kfree(slave);
		return ERR_PTR(-ENOMEM);
	}

	/* set up the MTD object for this partition */
	slave->mtd.type = master->type;
	slave->mtd.flags = master->flags & ~part->mask_flags;
	slave->mtd.size = part->size;
	slave->mtd.writesize = master->writesize;
	slave->mtd.writebufsize = master->writebufsize;
	slave->mtd.oobsize = master->oobsize;
	slave->mtd.oobavail = master->oobavail;
	slave->mtd.subpage_sft = master->subpage_sft;

	slave->mtd.name = name;
	slave->mtd.owner = master->owner;
	slave->mtd.backing_dev_info = master->backing_dev_info;

	/* NOTE:  we don't arrange MTDs as a tree; it'd be error-prone
	 * to have the same data be in two different partitions.
	 */
	slave->mtd.dev.parent = master->dev.parent;

	slave->mtd._read = part_read;
	slave->mtd._write = part_write;

	if (master->_panic_write)
		slave->mtd._panic_write = part_panic_write;

	if (master->_point && master->_unpoint) {
		slave->mtd._point = part_point;
		slave->mtd._unpoint = part_unpoint;
	}

	if (master->_get_unmapped_area)
		slave->mtd._get_unmapped_area = part_get_unmapped_area;
	if (master->_read_oob)
		slave->mtd._read_oob = part_read_oob;
	if (master->_write_oob)
		slave->mtd._write_oob = part_write_oob;
	if (master->_read_user_prot_reg)
		slave->mtd._read_user_prot_reg = part_read_user_prot_reg;
	if (master->_read_fact_prot_reg)
		slave->mtd._read_fact_prot_reg = part_read_fact_prot_reg;
	if (master->_write_user_prot_reg)
		slave->mtd._write_user_prot_reg = part_write_user_prot_reg;
	if (master->_lock_user_prot_reg)
		slave->mtd._lock_user_prot_reg = part_lock_user_prot_reg;
	if (master->_get_user_prot_info)
		slave->mtd._get_user_prot_info = part_get_user_prot_info;
	if (master->_get_fact_prot_info)
		slave->mtd._get_fact_prot_info = part_get_fact_prot_info;
	if (master->_sync)
		slave->mtd._sync = part_sync;
	if (!partno && !master->dev.class && master->_suspend &&
	    master->_resume) {
			slave->mtd._suspend = part_suspend;
			slave->mtd._resume = part_resume;
	}
	if (master->_writev)
		slave->mtd._writev = part_writev;
	if (master->_lock)
		slave->mtd._lock = part_lock;
	if (master->_unlock)
		slave->mtd._unlock = part_unlock;
	if (master->_is_locked)
		slave->mtd._is_locked = part_is_locked;
	if (master->_block_isbad)
		slave->mtd._block_isbad = part_block_isbad;
	if (master->_block_markbad)
		slave->mtd._block_markbad = part_block_markbad;
	slave->mtd._erase = part_erase;
	slave->master = master;
	slave->offset = part->offset;

	if (slave->offset == MTDPART_OFS_APPEND)
		slave->offset = cur_offset;
	if (slave->offset == MTDPART_OFS_NXTBLK) {
#if defined(CONFIG_SUPPORT_OPENWRT)
		/* Round up to next erasesize */
		slave->offset = mtd_roundup_to_eb(cur_offset, master);
		if (slave->offset != cur_offset)
			printk(KERN_NOTICE "Moving partition %d: "
			       "0x%012llx -> 0x%012llx\n", partno,
			       (unsigned long long)cur_offset, (unsigned long long)slave->offset);
#else
		slave->offset = cur_offset;
		if (mtd_mod_by_eb(cur_offset, master) != 0) {
			/* Round up to next erasesize */
			slave->offset = (mtd_div_by_eb(cur_offset, master) + 1) * master->erasesize;
			printk(KERN_NOTICE "Moving partition %d: "
			       "0x%012llx -> 0x%012llx\n", partno,
			       (unsigned long long)cur_offset, (unsigned long long)slave->offset);
		}
#endif
	}
	if (slave->offset == MTDPART_OFS_RETAIN) {
		slave->offset = cur_offset;
		if (master->size - slave->offset >= slave->mtd.size) {
			slave->mtd.size = master->size - slave->offset
							- slave->mtd.size;
		} else {
			printk(KERN_ERR "mtd partition \"%s\" doesn't have enough space: %#llx < %#llx, disabled\n",
				part->name, master->size - slave->offset,
				slave->mtd.size);
			/* register to preserve ordering */
			goto out_register;
		}
	}
	if (slave->mtd.size == MTDPART_SIZ_FULL)
		slave->mtd.size = master->size - slave->offset;

	printk(KERN_NOTICE "0x%012llx-0x%012llx : \"%s\"\n", (unsigned long long)slave->offset,
		(unsigned long long)(slave->offset + slave->mtd.size), slave->mtd.name);

	/* let's do some sanity checks */
	if (slave->offset >= master->size) {
		/* let's register it anyway to preserve ordering */
		slave->offset = 0;
		slave->mtd.size = 0;
		printk(KERN_ERR"mtd: partition \"%s\" is out of reach -- disabled\n",
			part->name);
		goto out_register;
	}
	if (slave->offset + slave->mtd.size > master->size) {
		slave->mtd.size = master->size - slave->offset;
		printk(KERN_WARNING"mtd: partition \"%s\" extends beyond the end of device \"%s\" -- size truncated to %#llx\n",
			part->name, master->name, (unsigned long long)slave->mtd.size);
	}
	if (master->numeraseregions > 1) {
		/* Deal with variable erase size stuff */
		int i, max = master->numeraseregions;
		u64 end = slave->offset + slave->mtd.size;
		struct mtd_erase_region_info *regions = master->eraseregions;

		/* Find the first erase regions which is part of this
		 * partition. */
		for (i = 0; i < max && regions[i].offset <= slave->offset; i++)
			;
		/* The loop searched for the region _behind_ the first one */
		if (i > 0)
			i--;

		/* Pick biggest erasesize */
		for (; i < max && regions[i].offset < end; i++) {
			if (slave->mtd.erasesize < regions[i].erasesize) {
				slave->mtd.erasesize = regions[i].erasesize;
			}
		}
		BUG_ON(slave->mtd.erasesize == 0);
	} else {
		/* Single erase size */
		slave->mtd.erasesize = master->erasesize;
	}

#if defined(CONFIG_SUPPORT_OPENWRT)
	if ((slave->mtd.flags & MTD_WRITEABLE) &&
	    mtd_mod_by_eb(slave->offset, &slave->mtd)) {
		/* Doesn't start on a boundary of major erase size */
		slave->mtd.flags |= MTD_ERASE_PARTIAL;
		if (((u32) slave->mtd.size) > master->erasesize)
			slave->mtd.flags &= ~MTD_WRITEABLE;
		else
			slave->mtd.erasesize = slave->mtd.size;
	}
	if ((slave->mtd.flags & MTD_WRITEABLE) &&
	    mtd_mod_by_eb(slave->offset + slave->mtd.size, &slave->mtd)) {
		slave->mtd.flags |= MTD_ERASE_PARTIAL;

		if ((u32) slave->mtd.size > master->erasesize)
			slave->mtd.flags &= ~MTD_WRITEABLE;
		else
			slave->mtd.erasesize = slave->mtd.size;
	}
	if ((slave->mtd.flags & (MTD_ERASE_PARTIAL|MTD_WRITEABLE)) == MTD_ERASE_PARTIAL)
		printk(KERN_WARNING"mtd: partition \"%s\" must either start or end on erase block boundary or be smaller than an erase block -- forcing read-only\n",
				part->name);
#else
	if ((slave->mtd.flags & MTD_WRITEABLE) &&
	    mtd_mod_by_eb(slave->offset, &slave->mtd)) {
		/* Doesn't start on a boundary of major erase size */
		/* FIXME: Let it be writable if it is on a boundary of
		 * _minor_ erase size though */
		slave->mtd.flags &= ~MTD_WRITEABLE;
		printk(KERN_WARNING"mtd: partition \"%s\" doesn't start on an erase block boundary -- force read-only\n",
			part->name);
	}
	if ((slave->mtd.flags & MTD_WRITEABLE) &&
	    mtd_mod_by_eb(slave->mtd.size, &slave->mtd)) {
		slave->mtd.flags &= ~MTD_WRITEABLE;
		printk(KERN_WARNING"mtd: partition \"%s\" doesn't end on an erase block -- force read-only\n",
			part->name);
	}
#endif

	slave->mtd.ecclayout = master->ecclayout;
	slave->mtd.ecc_strength = master->ecc_strength;
	slave->mtd.bitflip_threshold = master->bitflip_threshold;

	if (master->_block_isbad) {
		uint64_t offs = 0;

		while (offs < slave->mtd.size) {
			if (mtd_block_isbad(master, offs + slave->offset))
				slave->mtd.ecc_stats.badblocks++;
			offs += slave->mtd.erasesize;
		}
	}

out_register:
	return slave;
}

#if defined(CONFIG_SUPPORT_OPENWRT)
static int
__mtd_add_partition(struct mtd_info *master, char *name,
		    long long offset, long long length, bool dup_check)
{
	struct mtd_partition part;
	struct mtd_part *p, *new;
	uint64_t start, end;
	int ret = 0;

	/* the direct offset is expected */
	if (offset == MTDPART_OFS_APPEND ||
	    offset == MTDPART_OFS_NXTBLK)
		return -EINVAL;

	if (length == MTDPART_SIZ_FULL)
		length = master->size - offset;

	if (length <= 0)
		return -EINVAL;

	part.name = name;
	part.size = length;
	part.offset = offset;
	part.mask_flags = 0;
	part.ecclayout = NULL;

	new = allocate_partition(master, &part, -1, offset);
	if (IS_ERR(new))
		return PTR_ERR(new);

	start = offset;
	end = offset + length;

	mutex_lock(&mtd_partitions_mutex);
	if (dup_check) {
		list_for_each_entry(p, &mtd_partitions, list)
			if (p->master == master) {
				if ((start >= p->offset) &&
				    (start < (p->offset + p->mtd.size)))
					goto err_inv;

				if ((end >= p->offset) &&
				    (end < (p->offset + p->mtd.size)))
					goto err_inv;
			}
	}

	list_add(&new->list, &mtd_partitions);
	mutex_unlock(&mtd_partitions_mutex);

	add_mtd_device(&new->mtd);
	mtd_partition_split(master, new);

	return ret;
err_inv:
	mutex_unlock(&mtd_partitions_mutex);
	free_partition(new);
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(mtd_add_partition);

int mtd_add_partition(struct mtd_info *master, char *name,
		      long long offset, long long length)
{
	return __mtd_add_partition(master, name, offset, length, true);
}
#else /* CONFIG_SUPPORT_OPENWRT */
int mtd_add_partition(struct mtd_info *master, char *name,
		      long long offset, long long length)
{
	struct mtd_partition part;
	struct mtd_part *p, *new;
	uint64_t start, end;
	int ret = 0;

	/* the direct offset is expected */
	if (offset == MTDPART_OFS_APPEND ||
	    offset == MTDPART_OFS_NXTBLK)
		return -EINVAL;

	if (length == MTDPART_SIZ_FULL)
		length = master->size - offset;

	if (length <= 0)
		return -EINVAL;

	part.name = name;
	part.size = length;
	part.offset = offset;
	part.mask_flags = 0;
	part.ecclayout = NULL;

	new = allocate_partition(master, &part, -1, offset);
	if (IS_ERR(new))
		return PTR_ERR(new);

	start = offset;
	end = offset + length;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry(p, &mtd_partitions, list)
		if (p->master == master) {
			if ((start >= p->offset) &&
			    (start < (p->offset + p->mtd.size)))
				goto err_inv;

			if ((end >= p->offset) &&
			    (end < (p->offset + p->mtd.size)))
				goto err_inv;
		}

	list_add(&new->list, &mtd_partitions);
	mutex_unlock(&mtd_partitions_mutex);

	add_mtd_device(&new->mtd);

	return ret;
err_inv:
	mutex_unlock(&mtd_partitions_mutex);
	free_partition(new);
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(mtd_add_partition);
#endif /* CONFIG_SUPPORT_OPENWRT */

int mtd_del_partition(struct mtd_info *master, int partno)
{
	struct mtd_part *slave, *next;
	int ret = -EINVAL;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry_safe(slave, next, &mtd_partitions, list)
		if ((slave->master == master) &&
		    (slave->mtd.index == partno)) {
			ret = del_mtd_device(&slave->mtd);
			if (ret < 0)
				break;

			list_del(&slave->list);
			free_partition(slave);
			break;
		}
	mutex_unlock(&mtd_partitions_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(mtd_del_partition);

#if defined(CONFIG_SUPPORT_OPENWRT)
static int
run_parsers_by_type(struct mtd_part *slave, enum mtd_parser_type type)
{
	struct mtd_partition *parts;
	int nr_parts;
	int i;

	nr_parts = parse_mtd_partitions_by_type(&slave->mtd, type, &parts,
						NULL);
	if (nr_parts <= 0)
		return nr_parts;

	if (WARN_ON(!parts))
		return 0;

	for (i = 0; i < nr_parts; i++) {
		/* adjust partition offsets */
		parts[i].offset += slave->offset;

		__mtd_add_partition(slave->master,
				    parts[i].name,
				    parts[i].offset,
				    parts[i].size,
				    false);
	}

	kfree(parts);

	return nr_parts;
}

static inline unsigned long
mtd_pad_erasesize(struct mtd_info *mtd, int offset, int len)
{
	unsigned long mask = mtd->erasesize - 1;

	len += offset & mask;
	len = (len + mask) & ~mask;
	len -= offset & mask;
	if (mtd->type & MTD_NANDFLASH)
	{
		unsigned char buf[16];
		int start_offset = (offset + mask) & ~mask;
		int rootfs_len = len & ~mask;
		struct mtd_oob_ops ops;
		memset(&ops, 0, sizeof(ops));
		ops.mode = MTD_OPS_RAW;
		ops.ooblen=1;
		ops.oobbuf = buf;
		
		// the total length was reduced if there are bad block in kernel image, now add it back
		rootfs_len += bad_detected * mtd->erasesize;
		while(rootfs_len >= 0)
		{
			mtd_read_oob(mtd, start_offset, &ops);
			if (buf[0] != (unsigned char)0xff)
			{
				bad_detected++;
				printk("bad_detected= %x\n", bad_detected);
			}
			else
			{
				rootfs_len -= mtd->erasesize;
			}
			start_offset += mtd->erasesize;
		}

		len += bad_detected * mtd->erasesize;
	}
	return len;
}

static int split_squashfs(struct mtd_info *master, int offset, int *split_offset)
{
	size_t squashfs_len;
	int len, ret;

	ret = mtd_get_squashfs_len(master, offset, &squashfs_len);
	if (ret)
		return ret;

	len = mtd_pad_erasesize(master, offset, squashfs_len);
	*split_offset = offset + len;

	return 0;
}

#ifdef CONFIG_SUPPORT_OPENWRT
#if defined(CONFIG_MTD_ANY_RALINK) || defined(CONFIG_MTK_MTD_NAND)
extern unsigned int rootfs_data_offset;
extern unsigned int rootfs_offset;
#endif
#endif

static void split_rootfs_data(struct mtd_info *master, struct mtd_part *part)
{
	unsigned int split_offset = 0;
	unsigned int split_size;
	int ret;

	ret = run_parsers_by_type(part, MTD_PARSER_TYPE_ROOTFS);
	if (ret > 0)
		return;

	ret = split_squashfs(master, part->offset, &split_offset);
	if (ret)
		return;

	if (split_offset <= 0)
		return;

	split_size = part->mtd.size - (split_offset - part->offset);
	printk(KERN_INFO "mtd: partition \"%s\" created automatically, ofs=0x%x, len=0x%x\n",
		ROOTFS_SPLIT_NAME, split_offset, split_size);

#ifdef CONFIG_SUPPORT_OPENWRT
#if defined(CONFIG_MTD_ANY_RALINK) || defined(CONFIG_MTK_MTD_NAND)
	rootfs_data_offset = split_offset;
#endif
#endif

	__mtd_add_partition(master, ROOTFS_SPLIT_NAME, split_offset,
			    split_size, false);
}

#define UBOOT_MAGIC	0x27051956

static void split_uimage(struct mtd_info *master, struct mtd_part *part)
{
	struct {
		__be32 magic;
		__be32 pad0[2];
		__be32 size;
		__be32 pad1[4];
		__be32 name[7];
		__be32 kern_size;
	} hdr;
	size_t len;

	if (mtd_read(master, part->offset, sizeof(hdr), &len, (void *) &hdr))
	{
		int i = master->erasesize;
		while (mtd_read(master, part->offset + i, sizeof(hdr), &len, (void *) &hdr))
			i+= master->erasesize;
		//return;
	}

	if (len != sizeof(hdr) || hdr.magic != cpu_to_be32(UBOOT_MAGIC))
		return;

	if (hdr.kern_size != 0 && hdr.name[0] == 0)
		len = be32_to_cpu(hdr.kern_size);
	else
		len = be32_to_cpu(hdr.size) + 0x40;

#ifdef CONFIG_SUPPORT_OPENWRT
#if defined(CONFIG_MTD_ANY_RALINK) || defined(CONFIG_MTK_MTD_NAND)
	if (master->type & MTD_NANDFLASH)
	{
		unsigned char buf[16];
		int i;
		struct mtd_oob_ops ops;
		memset(&ops, 0, sizeof(ops));
		ops.mode = MTD_OPS_RAW;
		ops.ooblen=1;
		ops.oobbuf = buf;
		bad_detected = 0;
		for (i = part->offset; i < (part->offset + len + (bad_detected * master->erasesize)); i+= master->erasesize)
		{
			mtd_read_oob(master, i, &ops);
			if (buf[0] != (unsigned char)0xff)
			{
				bad_detected++;
			}
		}
		
		rootfs_offset = part->offset + len + bad_detected * master->erasesize;

		__mtd_add_partition(master, "rootfs", part->offset + len + bad_detected * master->erasesize,
				    part->mtd.size - len - bad_detected * master->erasesize, false);
	}
	else
#endif
#endif
	__mtd_add_partition(master, "rootfs", part->offset + len,
			    part->mtd.size - len, false);
}

#ifdef CONFIG_MTD_SPLIT_FIRMWARE_NAME
#define SPLIT_FIRMWARE_NAME	CONFIG_MTD_SPLIT_FIRMWARE_NAME
#else
#define SPLIT_FIRMWARE_NAME	"unused"
#endif

static void split_firmware(struct mtd_info *master, struct mtd_part *part)
{
	int ret;

	ret = run_parsers_by_type(part, MTD_PARSER_TYPE_FIRMWARE);
	if (ret > 0)
		return;

	if (config_enabled(CONFIG_MTD_UIMAGE_SPLIT))
		split_uimage(master, part);
}

void __weak arch_split_mtd_part(struct mtd_info *master, const char *name,
                                int offset, int size)
{
}

static void mtd_partition_split(struct mtd_info *master, struct mtd_part *part)
{
	static int rootfs_found = 0;

	if (rootfs_found)
		return;

	if (!strcmp(part->mtd.name, "rootfs")) {
		rootfs_found = 1;

		if (config_enabled(CONFIG_MTD_ROOTFS_SPLIT))
			split_rootfs_data(master, part);
	}

	if (!strcmp(part->mtd.name, SPLIT_FIRMWARE_NAME) &&
	    config_enabled(CONFIG_MTD_SPLIT_FIRMWARE))
		split_firmware(master, part);

	arch_split_mtd_part(master, part->mtd.name, part->offset,
			    part->mtd.size);
}
#endif /* CONFIG_SUPPORT_OPENWRT */

/*
 * This function, given a master MTD object and a partition table, creates
 * and registers slave MTD objects which are bound to the master according to
 * the partition definitions.
 *
 * We don't register the master, or expect the caller to have done so,
 * for reasons of data integrity.
 */

int add_mtd_partitions(struct mtd_info *master,
		       const struct mtd_partition *parts,
		       int nbparts)
{
	struct mtd_part *slave;
	uint64_t cur_offset = 0;
	int i;

	printk(KERN_NOTICE "Creating %d MTD partitions on \"%s\":\n", nbparts, master->name);

	for (i = 0; i < nbparts; i++) {
		slave = allocate_partition(master, parts + i, i, cur_offset);
		if (IS_ERR(slave))
			return PTR_ERR(slave);

		mutex_lock(&mtd_partitions_mutex);
		list_add(&slave->list, &mtd_partitions);
		mutex_unlock(&mtd_partitions_mutex);

		add_mtd_device(&slave->mtd);
#if defined(CONFIG_SUPPORT_OPENWRT)
		mtd_partition_split(master, slave);
#endif
		cur_offset = slave->offset + slave->mtd.size;
	}

	return 0;
}

static DEFINE_SPINLOCK(part_parser_lock);
static LIST_HEAD(part_parsers);

static struct mtd_part_parser *get_partition_parser(const char *name)
{
	struct mtd_part_parser *p, *ret = NULL;

	spin_lock(&part_parser_lock);

	list_for_each_entry(p, &part_parsers, list)
		if (!strcmp(p->name, name) && try_module_get(p->owner)) {
			ret = p;
			break;
		}

	spin_unlock(&part_parser_lock);

	return ret;
}

#define put_partition_parser(p) do { module_put((p)->owner); } while (0)

#if defined(CONFIG_SUPPORT_OPENWRT)
static struct mtd_part_parser *
get_partition_parser_by_type(enum mtd_parser_type type,
			     struct mtd_part_parser *start)
{
	struct mtd_part_parser *p, *ret = NULL;

	spin_lock(&part_parser_lock);

	p = list_prepare_entry(start, &part_parsers, list);
	if (start)
		put_partition_parser(start);

	list_for_each_entry_continue(p, &part_parsers, list) {
		if (p->type == type && try_module_get(p->owner)) {
			ret = p;
			break;
		}
	}

	spin_unlock(&part_parser_lock);

	return ret;
}
#endif

int register_mtd_parser(struct mtd_part_parser *p)
{
	spin_lock(&part_parser_lock);
	list_add(&p->list, &part_parsers);
	spin_unlock(&part_parser_lock);

	return 0;
}
EXPORT_SYMBOL_GPL(register_mtd_parser);

int deregister_mtd_parser(struct mtd_part_parser *p)
{
	spin_lock(&part_parser_lock);
	list_del(&p->list);
	spin_unlock(&part_parser_lock);
	return 0;
}
EXPORT_SYMBOL_GPL(deregister_mtd_parser);

/*
 * Do not forget to update 'parse_mtd_partitions()' kerneldoc comment if you
 * are changing this array!
 */
static const char * const default_mtd_part_types[] = {
	"cmdlinepart",
	"ofpart",
	NULL
};

/**
 * parse_mtd_partitions - parse MTD partitions
 * @master: the master partition (describes whole MTD device)
 * @types: names of partition parsers to try or %NULL
 * @pparts: array of partitions found is returned here
 * @data: MTD partition parser-specific data
 *
 * This function tries to find partition on MTD device @master. It uses MTD
 * partition parsers, specified in @types. However, if @types is %NULL, then
 * the default list of parsers is used. The default list contains only the
 * "cmdlinepart" and "ofpart" parsers ATM.
 * Note: If there are more then one parser in @types, the kernel only takes the
 * partitions parsed out by the first parser.
 *
 * This function may return:
 * o a negative error code in case of failure
 * o zero if no partitions were found
 * o a positive number of found partitions, in which case on exit @pparts will
 *   point to an array containing this number of &struct mtd_info objects.
 */
int parse_mtd_partitions(struct mtd_info *master, const char *const *types,
			 struct mtd_partition **pparts,
			 struct mtd_part_parser_data *data)
{
	struct mtd_part_parser *parser;
	int ret = 0;

	if (!types)
		types = default_mtd_part_types;

	for ( ; ret <= 0 && *types; types++) {
		parser = get_partition_parser(*types);
		if (!parser && !request_module("%s", *types))
			parser = get_partition_parser(*types);
		if (!parser)
			continue;
		ret = (*parser->parse_fn)(master, pparts, data);
		put_partition_parser(parser);
		if (ret > 0) {
			printk(KERN_NOTICE "%d %s partitions found on MTD device %s\n",
			       ret, parser->name, master->name);
			break;
		}
	}
	return ret;
}

#if defined(CONFIG_SUPPORT_OPENWRT)
int parse_mtd_partitions_by_type(struct mtd_info *master,
				 enum mtd_parser_type type,
				 struct mtd_partition **pparts,
				 struct mtd_part_parser_data *data)
{
	struct mtd_part_parser *prev = NULL;
	int ret = 0;

	while (1) {
		struct mtd_part_parser *parser;

		parser = get_partition_parser_by_type(type, prev);
		if (!parser)
			break;

		ret = (*parser->parse_fn)(master, pparts, data);

		if (ret > 0) {
			put_partition_parser(parser);
			printk(KERN_NOTICE
			       "%d %s partitions found on MTD device %s\n",
			       ret, parser->name, master->name);
			break;
		}

		prev = parser;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(parse_mtd_partitions_by_type);
#endif /* CONFIG_SUPPORT_OPENWRT */

int mtd_is_partition(const struct mtd_info *mtd)
{
	struct mtd_part *part;
	int ispart = 0;

	mutex_lock(&mtd_partitions_mutex);
	list_for_each_entry(part, &mtd_partitions, list)
		if (&part->mtd == mtd) {
			ispart = 1;
			break;
		}
	mutex_unlock(&mtd_partitions_mutex);

	return ispart;
}
EXPORT_SYMBOL_GPL(mtd_is_partition);

#if defined(CONFIG_SUPPORT_OPENWRT)
struct mtd_info *mtdpart_get_master(const struct mtd_info *mtd)
{
	if (!mtd_is_partition(mtd))
		return (struct mtd_info *)mtd;

	return PART(mtd)->master;
}
EXPORT_SYMBOL_GPL(mtdpart_get_master);

uint64_t mtdpart_get_offset(const struct mtd_info *mtd)
{
	if (!mtd_is_partition(mtd))
		return 0;

	return PART(mtd)->offset;
}
EXPORT_SYMBOL_GPL(mtdpart_get_offset);
#endif /* CONFIG_SUPPORT_OPENWRT */

/* Returns the size of the entire flash chip */
uint64_t mtd_get_device_size(const struct mtd_info *mtd)
{
	if (!mtd_is_partition(mtd))
		return mtd->size;

	return PART(mtd)->master->size;
}
EXPORT_SYMBOL_GPL(mtd_get_device_size);
