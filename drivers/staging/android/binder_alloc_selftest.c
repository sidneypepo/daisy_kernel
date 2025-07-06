/* binder_alloc_selftest.c
 *
 * Android IPC Subsystem
 *
 * Copyright (C) 2017 Google, Inc.
 * Copyright (C) 2018 XiaoMi, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/mm_types.h>
#include <linux/err.h>
#include "binder_alloc.h"
#define BUFFER_NUM 5
#define BUFFER_MIN_SIZE (PAGE_SIZE / 8)
static bool binder_selftest_run = true;
static int binder_selftest_failures;
static DEFINE_MUTEX(binder_selftest_lock);
enum buf_end_align_type {
	SAME_PAGE_UNALIGNED = 0,
	SAME_PAGE_ALIGNED,
	NEXT_PAGE_UNALIGNED,
	NEXT_PAGE_ALIGNED,
	NEXT_NEXT_UNALIGNED,
	LOOP_END,
};
static void pr_err_size_seq(size_t *sizes, int *seq)
{
	int i;
	pr_err("alloc sizes: ");
	for (i = 0; i < BUFFER_NUM; i++)
		pr_cont("[%zu]", sizes[i]);
	pr_cont("\n");
	pr_err("free seq: ");
	for (i = 0; i < BUFFER_NUM; i++)
		pr_cont("[%d]", seq[i]);
	pr_cont("\n");
}
static bool check_buffer_pages_allocated(struct binder_alloc *alloc,
					 struct binder_buffer *buffer,
					 size_t size)
{
	void *page_addr, *end;
	int page_index;
	end = (void *)PAGE_ALIGN((uintptr_t)buffer->data + size);
	page_addr = buffer->data;
	for (; page_addr < end; page_addr += PAGE_SIZE) {
		page_index = (page_addr - alloc->buffer) / PAGE_SIZE;
		if (!alloc->pages[page_index].page_ptr ||
		    !list_empty(&alloc->pages[page_index].lru)) {
			pr_err("expect alloc but is %s at page index %d\n",
			       alloc->pages[page_index].page_ptr ?
			       "lru" : "free", page_index);
			return false;
		}
	}
	return true;
}
static void binder_selftest_alloc_buf(struct binder_alloc *alloc,
				      struct binder_buffer *buffers[],
				      size_t *sizes, int *seq)
{
	int i;
	for (i = 0; i < BUFFER_NUM; i++) {
		buffers[i] = binder_alloc_new_buf(alloc, sizes[i], 0, 0, 0);
		if (IS_ERR(buffers[i]) ||
		    !check_buffer_pages_allocated(alloc, buffers[i],
						  sizes[i])) {
			pr_err_size_seq(sizes, seq);
			binder_selftest_failures++;
		}
	}
}
static void binder_selftest_free_buf(struct binder_alloc *alloc,
				     struct binder_buffer *buffers[],
				     size_t *sizes, int *seq, size_t end)
{
	int i;
	for (i = 0; i < BUFFER_NUM; i++)
		binder_alloc_free_buf(alloc, buffers[seq[i]]);
	for (i = 0; i < end / PAGE_SIZE; i++) {
		if (list_empty(&alloc->pages[i].lru)) {
			pr_err_size_seq(sizes, seq);
			pr_err("expect lru but is %s at page index %d\n",
			       alloc->pages[i].page_ptr ? "alloc" : "free", i);
			binder_selftest_failures++;
		}
	}
}
static void binder_selftest_free_page(struct binder_alloc *alloc)
{
	int i;
	unsigned long count;
	while ((count = list_lru_count(&binder_alloc_lru))) {
		list_lru_walk(&binder_alloc_lru, binder_alloc_free_page,
			      NULL, count);
	}
	for (i = 0; i < (alloc->buffer_size / PAGE_SIZE); i++) {
		if (alloc->pages[i].page_ptr) {
			pr_err("expect free but is %s at page index %d\n",
			       list_empty(&alloc->pages[i].lru) ?
			       "alloc" : "lru", i);
			binder_selftest_failures++;
		}
	}
}
static void binder_selftest_alloc_free(struct binder_alloc *alloc,
				       size_t *sizes, int *seq, size_t end)
{
	struct binder_buffer *buffers[BUFFER_NUM];
	binder_selftest_alloc_buf(alloc, buffers, sizes, seq);
	binder_selftest_free_buf(alloc, buffers, sizes, seq, end);
	binder_selftest_alloc_buf(alloc, buffers, sizes, seq);
	if (list_lru_count(&binder_alloc_lru))
		pr_err("lru list should be empty but is not\n");
	binder_selftest_free_buf(alloc, buffers, sizes, seq, end);
	binder_selftest_free_page(alloc);
}
static bool is_dup(int *seq, int index, int val)
{
	int i;
	for (i = 0; i < index; i++) {
		if (seq[i] == val)
			return true;
	}
	return false;
}
static void binder_selftest_free_seq(struct binder_alloc *alloc,
				     size_t *sizes, int *seq,
				     int index, size_t end)
{
	int i;
	if (index == BUFFER_NUM) {
		binder_selftest_alloc_free(alloc, sizes, seq, end);
		return;
	}
	for (i = 0; i < BUFFER_NUM; i++) {
		if (is_dup(seq, index, i))
			continue;
		seq[index] = i;
		binder_selftest_free_seq(alloc, sizes, seq, index + 1, end);
	}
}
static void binder_selftest_alloc_size(struct binder_alloc *alloc,
				       size_t *end_offset)
{
	int i;
	int seq[BUFFER_NUM] = {0};
	size_t front_sizes[BUFFER_NUM];
	size_t back_sizes[BUFFER_NUM];
	size_t last_offset, offset = 0;
	for (i = 0; i < BUFFER_NUM; i++) {
		last_offset = offset;
		offset = end_offset[i];
		front_sizes[i] = offset - last_offset;
		back_sizes[BUFFER_NUM - i - 1] = front_sizes[i];
	}
	back_sizes[0] += alloc->buffer_size - end_offset[BUFFER_NUM - 1];
	binder_selftest_free_seq(alloc, front_sizes, seq, 0,
				 end_offset[BUFFER_NUM - 1]);
	binder_selftest_free_seq(alloc, back_sizes, seq, 0, alloc->buffer_size);
}
static void binder_selftest_alloc_offset(struct binder_alloc *alloc,
					 size_t *end_offset, int index)
{
	int align;
	size_t end, prev;
	if (index == BUFFER_NUM) {
		binder_selftest_alloc_size(alloc, end_offset);
		return;
	}
	prev = index == 0 ? 0 : end_offset[index - 1];
	end = prev;
	BUILD_BUG_ON(BUFFER_MIN_SIZE * BUFFER_NUM >= PAGE_SIZE);
	for (align = SAME_PAGE_UNALIGNED; align < LOOP_END; align++) {
		if (align % 2)
			end = ALIGN(end, PAGE_SIZE);
		else
			end += BUFFER_MIN_SIZE;
		end_offset[index] = end;
		binder_selftest_alloc_offset(alloc, end_offset, index + 1);
	}
}
void binder_selftest_alloc(struct binder_alloc *alloc)
{
	size_t end_offset[BUFFER_NUM];
	if (!binder_selftest_run)
		return;
	mutex_lock(&binder_selftest_lock);
	if (!binder_selftest_run || !alloc->vma)
		goto done;
	pr_info("STARTED\n");
	binder_selftest_alloc_offset(alloc, end_offset, 0);
	binder_selftest_run = false;
	if (binder_selftest_failures > 0)
		pr_info("%d tests FAILED\n", binder_selftest_failures);
	else
		pr_info("PASSED\n");
done:
	mutex_unlock(&binder_selftest_lock);
}
