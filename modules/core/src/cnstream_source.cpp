/*************************************************************************
 * Copyright (C) [2019] by Cambricon, Inc. All rights reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *************************************************************************/
#include "cnstream_source.hpp"
#include "cnstream_eventbus.hpp"
#include "cnstream_pipeline.hpp"

#include <bitset>
#include <string>
#include <unordered_map>

namespace cnstream {

static CNSpinLock stream_idx_lock;
static std::unordered_map<std::string, uint32_t> stream_idx_map;

static const uint32_t MAX_STREAM_NUM = 64;
static std::bitset<MAX_STREAM_NUM> stream_bitset(0);

uint32_t GetMaxStreamNumber() { return MAX_STREAM_NUM; }

static uint32_t _GetStreamIndex(const std::string &stream_id) {
  CNSpinLockGuard guard(stream_idx_lock);
  auto search = stream_idx_map.find(stream_id);
  if (search != stream_idx_map.end()) {
    return search->second;
  }

  for (uint32_t i = 0; i < GetMaxStreamNumber(); i++) {
    if (!stream_bitset[i]) {
      stream_bitset.set(i);
      stream_idx_map[stream_id] = i;
      return i;
    }
  }
  return INVALID_STREAM_IDX;
}

static int _ReturnStreamIndex(const std::string &stream_id) {
  CNSpinLockGuard guard(stream_idx_lock);
  auto search = stream_idx_map.find(stream_id);
  if (search == stream_idx_map.end()) {
    return -1;
  }
  uint32_t stream_idx = search->second;
  if (stream_idx >= GetMaxStreamNumber()) {
    return -1;
  }
  stream_bitset.reset(stream_idx);
  stream_idx_map.erase(search);
  return 0;
}

uint32_t SourceModule::GetStreamIndex(const std::string &stream_id) { return _GetStreamIndex(stream_id); }

void SourceModule::ReturnStreamIndex(const std::string &stream_id) { _ReturnStreamIndex(stream_id); }

int SourceModule::AddVideoSource(const std::string &stream_id, const std::string &filename, int framerate, bool loop) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (source_map_.find(stream_id) != source_map_.end()) {
    LOG(ERROR) << "Duplicate stream_id\n";
    return -1;
  }
  std::shared_ptr<SourceHandler> source = CreateSource(stream_id, filename, framerate, loop);
  if (source.get() != nullptr) {
    if (source->Open() != true) {
      LOG(ERROR) << "source Open failed";
      return -1;
    }
    source_map_[stream_id] = source;
    return 0;
  }
  return -1;
}

int SourceModule::RemoveSource(const std::string &stream_id) {
  std::unique_lock<std::mutex> lock(mutex_);
  auto iter = source_map_.find(stream_id);
  if (iter == source_map_.end()) {
    LOG(WARNING) << "source does not exist\n";
    return 0;
  }
  iter->second->Close();
  source_map_.erase(iter);
  return 0;
}

int SourceModule::RemoveSources() {
  std::vector<std::future<int>> future_vec;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    std::map<std::string, std::shared_ptr<SourceHandler>>::iterator iter;
    for (iter = source_map_.begin(); iter != source_map_.end();) {
      std::shared_ptr<SourceHandler> handler = iter->second;
      future_vec.push_back(std::async(std::launch::async, [handler]() {
        handler->Close();
        return 0;
      }));
      source_map_.erase(iter++);
    }
  }
  for (auto &f : future_vec) {
    f.wait();
  }
  return 0;
}

int SourceModule::Process(std::shared_ptr<CNFrameInfo> data) {
  (void)data;
  LOG(ERROR) << "As a source module, Process() should not be invoked\n";
  return 0;
}

bool SourceModule::SendData(std::shared_ptr<CNFrameInfo> data) {
  if (data->channel_idx == INVALID_STREAM_IDX) {
    LOG(ERROR) << "CNFrameInfo->channel_idx not initialized";
    return false;
  }
  if (container_) {
    return container_->ProvideData(this, data);
  }
  return false;
}

}  // namespace cnstream
