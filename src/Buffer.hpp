#pragma once

#include <cstdint>
#include <utility>
#include <cstring>

using std::size_t;
const float BUFFER_GROWTH_MULTIPLIER = 1.5;

//      ┌────────────┬────────────┬────────────┐
//      │   unused   │    data    │   unused   │
//      └────────────┴────────────┴────────────┘
//      ^            ^            ^            ^
//m_buffer_begin   m_data_begin   m_data_end   m_buffer_end

class Buffer
{
  uint8_t* m_buffer_begin = nullptr;
  uint8_t* m_buffer_end   = nullptr;
  uint8_t* m_data_begin   = nullptr;
  uint8_t* m_data_end     = nullptr;

  size_t capacity() const
  {
    return static_cast<size_t>(m_buffer_end - m_buffer_begin);
  }
  size_t available_space() const
  {
    return static_cast<size_t>(m_buffer_end - m_data_end);
  }
  size_t unused_space() const 
  {
    return capacity() - size();
  }
  void grow_buffer(size_t a_bytes) //grow the buffer by a_bytes
  {
    size_t old_size = size();
    size_t new_size = (old_size + a_bytes) * BUFFER_GROWTH_MULTIPLIER;
    Buffer new_buffer = Buffer(new_size);
    new_buffer.append_back(data(), size());
    swap(*this, new_buffer);
  }
  void reset()
  {
    delete[] m_buffer_begin;
    m_buffer_begin = nullptr;
    m_buffer_end = nullptr;
    m_data_begin = nullptr;
    m_data_end = nullptr;
  }
  friend void swap(Buffer& a, Buffer& b) noexcept
  {
    std::swap(a.m_buffer_begin, b.m_buffer_begin);
    std::swap(a.m_buffer_end  , b.m_buffer_end);
    std::swap(a.m_data_begin  , b.m_data_begin);
    std::swap(a.m_data_end    , b.m_data_end);
  }

public:
  Buffer() = default;
  Buffer(size_t a_size)
  {
    m_buffer_begin = new uint8_t[a_size]; 
    m_buffer_end = m_buffer_begin + a_size;
    m_data_begin = m_buffer_begin;
    m_data_end = m_data_begin;
  }
  Buffer(const Buffer& a_buff) = delete;             //disable copy and assignment operator
  Buffer& operator=(const Buffer& a_buff) = delete;
  Buffer(Buffer&& a_buff) noexcept
  {
    swap(*this, a_buff);
  }
  Buffer& operator=(Buffer&& a_buff) noexcept
  {
    if(this != &a_buff)
    {
      reset();
      swap(*this, a_buff);
    }
    return *this;
  }

  ~Buffer()
  {
    reset();
  }
  bool empty() const
  {
    return (m_data_begin == m_data_end);
  }
  size_t size() const
  {
    return static_cast<size_t>(m_data_end - m_data_begin);
  }
  const uint8_t* data() const
  {
    return m_data_begin;
  }
  void erase_front(const size_t a_bytes)
  {
    size_t current_size = size();
    if(a_bytes > current_size)
    {
      return; //logerr
    }
    m_data_begin += a_bytes;
    if(empty())
    {
      reset();
    }
  }

  void append_back(const uint8_t* a_buff, size_t a_bytes)
  {
    if(available_space() < a_bytes)
    {
      grow_buffer(a_bytes);
    }
    memcpy(m_data_end, a_buff, a_bytes);
    m_data_end += a_bytes;
  }
};
