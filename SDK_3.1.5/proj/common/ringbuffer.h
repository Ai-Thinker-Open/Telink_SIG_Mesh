/********************************************************************************************************
 * @file     ringbuffer.h 
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2010, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/

/*
 *  \code
 *      // Create the buffer structure and its underlying storage array
 *      ringbuffer_t buffer;
 *      u8      bufferData[128];
 *
 *      // Initialize the buffer with the created storage array
 *      ringbuffer_init(&buffer, bufferData, sizeof(bufferData));
 *
 *      // Insert some data into the buffer
 *      ringbuffer_Insert(buffer, 'H');
 *      ringbuffer_Insert(buffer, 'E');
 *      ringbuffer_Insert(buffer, 'L');
 *      ringbuffer_Insert(buffer, 'L');
 *      ringbuffer_Insert(buffer, 'O');
 *
 *      // Cache the number of stored bytes in the buffer
 *      u16 bufferCount = ringbuffer_get_count(&buffer);
 *
 *      // Printer stored data length
 *      printf("buffer Length: %d, buffer Data: \r\n", bufferCount);
 *
 *      // Print contents of the buffer one character at a time
 *      while (bufferCount--)
 *        putc(ringbuffer_Remove(&buffer));
 *  \endcode
 *
 *  @{
 */

#pragma once

/* Includes: */
#include "types.h"
#include "../mcu/irq_i.h"

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
	extern "C" {
#endif

/* Type Defines: */
/** \brief Ring buffer Management Structure.
 *
 *  Type define for a new ring buffer object. buffers should be initialized via a call to
 *  \ref ringbuffer_init() before use.
 */
typedef struct
{
	u8* in; /**< Current storage location in the circular buffer. */
	u8* out; /**< Current retrieval location in the circular buffer. */
	u8* start; /**< Pointer to the start of the buffer's underlying storage array. */
	u8* end; /**< Pointer to the end of the buffer's underlying storage array. */
	u16 size; /**< size of the buffer's underlying storage array. */
	u16 sount; /**< Number of bytes currently stored in the buffer. */
} ringbuffer_t;

/* Inline Functions: */
/** Initializes a ring buffer ready for use. buffers must be initialized via this function
 *  before any operations are called upon them. Already initialized buffers may be reset
 *  by re-initializing them using this function.
 *
 *  \param[out] buffer   Pointer to a ring buffer structure to initialize.
 *  \param[out] dataptr  Pointer to a global array that will hold the data stored into the ring buffer.
 *  \param[out] size     Maximum number of bytes that can be stored in the underlying data array.
 */
static inline void ringbuffer_init(ringbuffer_t* buffer, u8* const dataptr, const u16 size)
{
	u8 r = irq_disable();

	buffer->in     = dataptr;
	buffer->out    = dataptr;
	buffer->start  = &dataptr[0];
	buffer->end    = &dataptr[size];
	buffer->size   = size;
	buffer->sount  = 0;

	irq_restore(r);
}

/** Retrieves the current number of bytes stored in a particular buffer. This value is computed
 *  by entering an atomic lock on the buffer, so that the buffer cannot be modified while the
 *  computation takes place. This value should be cached when reading out the contents of the buffer,
 *  so that as small a time as possible is spent in an atomic lock.
 *
 *  \note The value returned by this function is guaranteed to only be the minimum number of bytes
 *        stored in the given buffer; this value may change as other threads write new data, thus
 *        the returned number should be used only to determine how many successive reads may safely
 *        be performed on the buffer.
 *
 *  \param[in] buffer  Pointer to a ring buffer structure whose count is to be computed.
 *
 *  \return Number of bytes currently stored in the buffer.
 */
static inline u16 ringbuffer_get_count(ringbuffer_t* const buffer)
{
	u8 r = irq_disable();
	u16 sount = buffer->sount;
	irq_restore(r);
	return sount;
}

/** Retrieves the free space in a particular buffer. This value is computed by entering an atomic lock
 *  on the buffer, so that the buffer cannot be modified while the computation takes place.
 *
 *  \note The value returned by this function is guaranteed to only be the maximum number of bytes
 *        free in the given buffer; this value may change as other threads write new data, thus
 *        the returned number should be used only to determine how many successive writes may safely
 *        be performed on the buffer when there is a single writer thread.
 *
 *  \param[in] buffer  Pointer to a ring buffer structure whose free count is to be computed.
 *
 *  \return Number of free bytes in the buffer.
 */
static inline u16 ringbuffer_get_free_count(ringbuffer_t* const buffer)
{
	return (buffer->size - ringbuffer_get_count(buffer));
}

/** Atomically determines if the specified ring buffer contains any data. This should
 *  be tested before removing data from the buffer, to ensure that the buffer does not
 *  underflow.
 *
 *  If the data is to be removed in a loop, store the total number of bytes stored in the
 *  buffer (via a call to the \ref ringbuffer_get_count() function) in a temporary variable
 *  to reduce the time spent in atomicity locks.
 *
 *  \param[in,out] buffer  Pointer to a ring buffer structure to insert into.
 *
 *  \return Boolean \c true if the buffer contains no free space, false otherwise.
 */
static inline u8 ringbuffer_is_empty(ringbuffer_t* const buffer)
{
	return (ringbuffer_get_count(buffer) == 0);
}

/** Atomically determines if the specified ring buffer contains any free space. This should
 *  be tested before storing data to the buffer, to ensure that no data is lost due to a
 *  buffer overrun.
 *
 *  \param[in,out] buffer  Pointer to a ring buffer structure to insert into.
 *
 *  \return Boolean \c true if the buffer contains no free space, false otherwise.
 */
static inline u8 ringbuffer_is_full(ringbuffer_t* const buffer)
{
	return (ringbuffer_get_count(buffer) == buffer->size);
}

/** Inserts an element into the ring buffer.
 *
 *  \note Only one execution thread (main program thread or an ISR) may insert into a single buffer
 *        otherwise data corruption may occur. Insertion and removal may occur from different execution
 *        threads.
 *
 *  \param[in,out] buffer  Pointer to a ring buffer structure to insert into.
 *  \param[in]     Data    Data element to insert into the buffer.
 */
static inline void ringbuffer_insert(ringbuffer_t* buffer, const u8 Data)
{
	*buffer->in = Data;

	if (++buffer->in == buffer->end)
	  buffer->in = buffer->start;

	u8 r = irq_disable();

	buffer->sount++;

	irq_restore(r);
}

/** Removes an element from the ring buffer.
 *
 *  \note Only one execution thread (main program thread or an ISR) may remove from a single buffer
 *        otherwise data corruption may occur. Insertion and removal may occur from different execution
 *        threads.
 *
 *  \param[in,out] buffer  Pointer to a ring buffer structure to retrieve from.
 *
 *  \return Next data element stored in the buffer.
 */
static inline u8 ringbuffer_remove(ringbuffer_t* buffer)
{
	u8 Data = *buffer->out;

	if (++buffer->out == buffer->end)
	  buffer->out = buffer->start;

	u8 r = irq_disable();

	buffer->sount--;

	irq_restore(r);

	return Data;
}

/** Returns the next element stored in the ring buffer, without removing it.
 *
 *  \param[in,out] buffer  Pointer to a ring buffer structure to retrieve from.
 *
 *  \return Next data element stored in the buffer.
 */
static inline u8 ringbuffer_peek(ringbuffer_t* const buffer)
{
	return *buffer->out;
}

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

/** @} */

