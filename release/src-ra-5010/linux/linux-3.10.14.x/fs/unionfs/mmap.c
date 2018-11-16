/*
 * Copyright (c) 2003-2014 Erez Zadok
 * Copyright (c) 2003-2006 Charles P. Wright
 * Copyright (c) 2005-2007 Josef 'Jeff' Sipek
 * Copyright (c) 2005-2006 Junjiro Okajima
 * Copyright (c) 2006      Shaya Potter
 * Copyright (c) 2005      Arun M. Krishnakumar
 * Copyright (c) 2004-2006 David P. Quigley
 * Copyright (c) 2003-2004 Mohammad Nayyer Zubair
 * Copyright (c) 2003      Puja Gupta
 * Copyright (c) 2003      Harikesavan Krishnan
 * Copyright (c) 2003-2014 Stony Brook University
 * Copyright (c) 2003-2014 The Research Foundation of SUNY
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "union.h"


/*
 * XXX: we need a dummy readpage handler because generic_file_mmap (which we
 * use in unionfs_mmap) checks for the existence of
 * mapping->a_ops->readpage, else it returns -ENOEXEC.  The VFS will need to
 * be fixed to allow a file system to define vm_ops->fault without any
 * address_space_ops whatsoever.
 *
 * Otherwise, we don't want to use our readpage method at all.
 */
static int unionfs_readpage(struct file *file, struct page *page)
{
	BUG();
	return -EINVAL;
}

static int unionfs_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	int err;
	struct file *file, *lower_file;
	const struct vm_operations_struct *lower_vm_ops;
	struct vm_area_struct lower_vma;

	BUG_ON(!vma);
	memcpy(&lower_vma, vma, sizeof(struct vm_area_struct));
	file = lower_vma.vm_file;
	lower_vm_ops = UNIONFS_F(file)->lower_vm_ops;
	BUG_ON(!lower_vm_ops);

	lower_file = unionfs_lower_file(file);
	BUG_ON(!lower_file);
	/*
	 * XXX: vm_ops->fault may be called in parallel.  Because we have to
	 * resort to temporarily changing the vma->vm_file to point to the
	 * lower file, a concurrent invocation of unionfs_fault could see a
	 * different value.  In this workaround, we keep a different copy of
	 * the vma structure in our stack, so we never expose a different
	 * value of the vma->vm_file called to us, even temporarily.  A
	 * better fix would be to change the calling semantics of ->fault to
	 * take an explicit file pointer.
	 */
	lower_vma.vm_file = lower_file;
	err = lower_vm_ops->fault(&lower_vma, vmf);
	return err;
}

/*
 * XXX: the default address_space_ops for unionfs is empty.  We cannot set
 * our inode->i_mapping->a_ops to NULL because too many code paths expect
 * the a_ops vector to be non-NULL.
 */
struct address_space_operations unionfs_aops = {
	/* empty on purpose */
};

/*
 * XXX: we need a second, dummy address_space_ops vector, to be used
 * temporarily during unionfs_mmap, because the latter calls
 * generic_file_mmap, which checks if ->readpage exists, else returns
 * -ENOEXEC.
 */
struct address_space_operations unionfs_dummy_aops = {
	.readpage	= unionfs_readpage,
};

struct vm_operations_struct unionfs_vm_ops = {
	.fault		= unionfs_fault,
};
