/*
 * Kernel-side shim for <os/firehose_buffer_private.h>.
 *
 * Upstream (libdispatch-1415.140.2, os/firehose_buffer_private.h) ships this
 * header inside libdispatch and the darwin-xnu-build fakeroot installs it via
 * `xcodebuild installhdrs` of the libfirehose_kernel target. The plain-make
 * CI path has no fakeroot, so xnu's own copy under libkern is staged instead.
 *
 * Only the KERNEL branch of the upstream header is reachable from xnu
 * (log.c compiles with -DKERNEL), and even there xnu implements the externals
 * itself — the sole hard dependencies are the reserve/flush prototypes and
 * FIREHOSE_BUFFER_KERNEL_CHUNK_COUNT. Keep this file to that surface.
 *
 * Upstream: https://github.com/apple-oss-distributions/libdispatch
 *           tag libdispatch-1415.140.2, Apache License 2.0
 */

#ifndef __FIREHOSE_BUFFER_PRIVATE__
#define __FIREHOSE_BUFFER_PRIVATE__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <firehose/firehose_types_private.h>
#include <firehose/chunk_private.h>
#include <firehose/tracepoint_private.h>

#define OS_FIREHOSE_SPI_VERSION 20180226

#define FIREHOSE_BUFFER_LIBTRACE_HEADER_SIZE	2048ul
#define FIREHOSE_BUFFER_KERNEL_MIN_CHUNK_COUNT	16
#define FIREHOSE_BUFFER_KERNEL_MAX_CHUNK_COUNT	64

typedef struct firehose_buffer_range_s {
	uint16_t fbr_offset; // offset from the start of the buffer
	uint16_t fbr_length;
} *firehose_buffer_range_t;

// implemented by libkern/os/log.c
extern void __firehose_buffer_push_to_logd(firehose_buffer_t fb, bool for_io);
extern void __firehose_critical_region_enter(void);
extern void __firehose_critical_region_leave(void);
extern void __firehose_allocate(vm_offset_t *addr, vm_size_t size);
extern uint8_t __firehose_buffer_kernel_chunk_count;

#define FIREHOSE_BUFFER_KERNEL_DEFAULT_CHUNK_COUNT FIREHOSE_BUFFER_KERNEL_MIN_CHUNK_COUNT
#define FIREHOSE_BUFFER_KERNEL_DEFAULT_IO_PAGES    8

#define FIREHOSE_BUFFER_KERNEL_CHUNK_COUNT __firehose_buffer_kernel_chunk_count
#define FIREHOSE_BUFFER_CHUNK_PREALLOCATED_COUNT (__firehose_buffer_kernel_chunk_count - 1) // the first chunk is the header

// exported for the kernel
firehose_tracepoint_t
__firehose_buffer_tracepoint_reserve(uint64_t stamp, firehose_stream_t stream,
		uint16_t pubsize, uint16_t privsize, uint8_t **privptr);

void
__firehose_buffer_tracepoint_flush(firehose_tracepoint_t vat,
		firehose_tracepoint_id_u vatid);

// consumed by bsd/kern/subr_log.c — implemented in libfirehose_kernel.a
firehose_buffer_t
__firehose_buffer_create(size_t *size);

bool
__firehose_merge_updates(firehose_push_reply_t update);

int
__firehose_kernel_configuration_valid(uint8_t chunk_count, uint8_t io_pages);

#endif /* __FIREHOSE_BUFFER_PRIVATE__ */
