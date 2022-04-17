// SPDX-License-Identifier: GPL-2.0-only
#include <linux/debugfs.h>
#include <linux/efi.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/pgtable.h>

extern int pgt_dump_process_id;

static int ptdump_show(struct seq_file *m, void *v)
{
	ptdump_walk_pgd_level_debugfs(m, &init_mm, false);
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(ptdump);

static int ptdump_curknl_show(struct seq_file *m, void *v)
{
	if (current->mm->pgd)
		ptdump_walk_pgd_level_debugfs(m, current->mm, false);
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(ptdump_curknl);

static int ptdump_process_show(struct seq_file *m, void *v)
{
	struct mm_struct *mm;

	if (pgt_dump_process_id > 0)
	{
		struct task_struct* task_oe =
			find_task_by_vpid((pid_t)pgt_dump_process_id);
		seq_printf(m, "Page tables for process id = %d\n",
				pgt_dump_process_id);

		if (task_oe == NULL) {
			seq_printf(m, "Process DNE!\n");
			return 0;
		}
		mm = task_oe->mm;
	} else {
		seq_printf(m, "process id is invalid!\n");
		return 0;
	}

	ptdump_walk_pgd_level_debugfs(m, mm, true);
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(ptdump_process);

#ifdef CONFIG_PAGE_TABLE_ISOLATION
static int ptdump_curusr_show(struct seq_file *m, void *v)
{
	if (current->mm->pgd)
		ptdump_walk_pgd_level_debugfs(m, current->mm, true);
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(ptdump_curusr);
#endif

#if defined(CONFIG_EFI) && defined(CONFIG_X86_64)
static int ptdump_efi_show(struct seq_file *m, void *v)
{
	if (efi_mm.pgd)
		ptdump_walk_pgd_level_debugfs(m, &efi_mm, false);
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(ptdump_efi);
#endif

static struct dentry *dir;

static int __init pt_dump_debug_init(void)
{
	dir = debugfs_create_dir("page_tables", NULL);

	debugfs_create_file("kernel", 0400, dir, NULL, &ptdump_fops);
	debugfs_create_file("current_kernel", 0400, dir, NULL,
			    &ptdump_curknl_fops);
	debugfs_create_file("process", 0400, dir, NULL,
			    &ptdump_process_fops);

#ifdef CONFIG_PAGE_TABLE_ISOLATION
	debugfs_create_file("current_user", 0400, dir, NULL,
			    &ptdump_curusr_fops);
#endif
#if defined(CONFIG_EFI) && defined(CONFIG_X86_64)
	debugfs_create_file("efi", 0400, dir, NULL, &ptdump_efi_fops);
#endif
	return 0;
}

static void __exit pt_dump_debug_exit(void)
{
	debugfs_remove_recursive(dir);
}

module_init(pt_dump_debug_init);
module_exit(pt_dump_debug_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arjan van de Ven <arjan@linux.intel.com>");
MODULE_DESCRIPTION("Kernel debugging helper that dumps pagetables");
