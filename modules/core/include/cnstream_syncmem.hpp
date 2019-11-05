/*********************************************************************************************************
 * All modification made by Cambricon Corporation: © 2018--2019 Cambricon Corporation
 * All rights reserved.
 * All other contributions:
 * Copyright (c) 2014--2018, the respective contributors
 * All rights reserved.
 * For the list of contributors go to https://github.com/BVLC/caffe/blob/master/CONTRIBUTORS.md
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Intel Corporation nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************************************************/

#ifndef CNSTREAM_SYNCMEM_HPP_
#define CNSTREAM_SYNCMEM_HPP_

/**
 * @file cnstream_syncmem.hpp
 *
 * This file contains a declaration of the CNSyncedMemory class.
 */

#include <cstddef>

namespace cnstream {

/**
 * Allocates data on a host.
 *
 * @param ptr Outputs data pointer.
 * @param size Size of the data to allocate.
 */
void CNStreamMallocHost(void** ptr, size_t size);

/**
 * Frees the data allocated by CNStreamMallocHost.
 *
 * @param ptr The data address to be freed.
 */
inline void CNStreamFreeHost(void* ptr) { free(ptr); }

/*
  @attention
 */
/**
 * @class CNSyncedMemory
 *
 * CNSyncedMemory used to sync memory between CPU and MLU.
 *
 * @note CNSyncedMemory::Head() will always return CNSyncedMemory::UNINITIALIZED when size is 0.
 */
class CNSyncedMemory {
 public:
  /**
   * Constructor.
   */
  CNSyncedMemory();
  /**
   * Constructor.
   *
   * @param size The size of the memory.
   */
  explicit CNSyncedMemory(size_t size);
  /**
   * Constructor.
   *
   * @param size The size of the memory.
   * @param mlu_dev_id MLU device id that is incremented from 0.
   * @param mlu_ddr_chn MLU DDR channel that is greater than or equal to 0, and is less
   *                    than 4. It specifies which piece of DDR the memory allocated on.
   */
  CNSyncedMemory(size_t size, int mlu_dev_id, int mlu_ddr_chn);
  ~CNSyncedMemory();
  /**
   * Gets the CPU data.
   *
   * @return Returns the CPU data pointer.
   *
   * @note If the size is 0, always returns nullptr.
   */
  const void* GetCpuData();
  /**
   * Sets the CPU data.
   *
   * @param data The data pointer on CPU.
   *
   * @return Void.
   */
  void SetCpuData(void* data);
  /**
   * Gets the MLU data.
   *
   * @return Returns the MLU data pointer.
   *
   * @note If the size is 0, always returns nullptr.
   */
  const void* GetMluData();
  /**
   * Sets the MLU data.
   *
   * @param data The data pointer on MLU
   */
  void SetMluData(void* data);
  /**
   * Sets the MLU device context
   *
   * @param dev_id MLU device id that is incremented from 0.
   * @param ddr_chn MLU DDR channel id that is greater than or equal to 0, and less than
   *                4. It specifies which piece of DDR the memory allocated on.
   *
   * @note Do this before all getter and setter.
   */
  void SetMluDevContext(int dev_id, int ddr_chn = 0);
  /**
   * Gets the MLU device id.
   *
   * @return Returns the device that the MLU memory allocated on.
   */
  int GetMluDevId() const;
  /**
   * Gets the channel id of the MLU DDR.
   *
   * @return Returns the DDR channel of the MLU memory allocated on.
   */
  int GetMluDdrChnId() const;
  /**
   * Gets the mutable CPU data.
   *
   * @return Returns the CPU data pointer.
   */
  void* GetMutableCpuData();
  /**
   * Gets the mutable MLU data.
   *
   * @return Returns the MLU data pointer.
   */
  void* GetMutableMluData();
  /**
   * Synced head.
   */
  enum SyncedHead {
    UNINITIALIZED,  ///< The memory has not been allocated.
    HEAD_AT_CPU,    ///< The data has been updated to CPU but has not been synced to MLU yet.
    HEAD_AT_MLU,    ///< The data has been updated to MLU but has not been synced to CPU yet.
    SYNCED          ///< The data has been synced to both CPU and MLU.
  };
  /**
   * Gets synced head.
   *
   * @return Returns synced head.
   */
  SyncedHead GetHead() const { return head_; }
  /**
   * Gets data bytes.
   *
   * @return Returns data bytes.
   */
  size_t GetSize() const { return size_; }

 private:
  /**
   * Synchronizes the memory data to CPU.
   */
  void ToCpu();
  /**
   * Synchronizes the memory data to MLU.
   */
  void ToMlu();

  void* cpu_ptr_ = nullptr;  ///< CPU data pointer.
  void* mlu_ptr_ = nullptr;  ///< MLU data pointer.

#ifdef TEST

 public:
#endif
  /*
   * Allocates memory by CNSyncedMemory if true.
   */
  bool own_cpu_data_ = false;  ///< Whether CPU data is allocated by SyncedMemory.
  bool own_mlu_data_ = false;  ///< Whether MLU data is allocated by SyncedMemory.

#ifdef TEST

 private:
#endif
  SyncedHead head_ = UNINITIALIZED;  ///< Identifies which device is the data synchronized on.
  size_t size_ = 0;                  ///< The data size.

  int dev_id_ = 0;   ///< Ordinal MLU device id.
  int ddr_chn_ = 0;  ///< Ordinal MLU DDR channel id. The value should be [0, 4).

  DISABLE_COPY_AND_ASSIGN(CNSyncedMemory);
};  // class CNSyncedMemory

}  // namespace cnstream

#endif  // CNSTREAM_SYNCMEM_HPP_