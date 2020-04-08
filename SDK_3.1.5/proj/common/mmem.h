/********************************************************************************************************
 * @file     mmem.h 
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
 * This file is part of the Contiki operating system.
 */

/**
 * \defgroup mmem Managed memory allocator
 *
 * The managed memory allocator is a fragmentation-free memory
 * manager. It keeps the allocated memory free from fragmentation by
 * compacting the memory when blocks are freed. A program that uses
 * the managed memory module cannot be sure that allocated memory
 * stays in place. Therefore, a level of indirection is used: access
 * to allocated memory must always be done using a special macro.
 *
 * \note This module has not been heavily tested.
 * @{
 */

/**
 * \file
 *         Header file for the managed memory allocator
 *
 */

#pragma once

/*---------------------------------------------------------------------------*/
/**
 * \brief      Get a pointer to the managed memory
 * \param m    A pointer to the struct mmem
 * \return     A pointer to the memory block, or NULL if memory could
 *             not be allcated.
 * \author     Adam Dunkels
 *
 *             This macro is used to get a pointer to a memory block
 *             allocated with mmem_alloc().
 *
 * \hideinitializer
 */
#define MMEM_PTR(m) (struct mmem *)(m)->ptr
struct mmem {
  struct mmem *next;
  unsigned int size;
  void *ptr;
};

/* XXX: tagga minne med "interrupt usage", vilke gör att man är
   speciellt varsam under free(). */

int  mmem_alloc(struct mmem *m, unsigned int size);
void mmem_free(struct mmem *);
void mmem_init(void);


/** @} */
/** @} */
