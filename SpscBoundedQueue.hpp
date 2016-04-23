#ifndef CONCURRENT_LOCKFREE_SPSCBOUNDEDQUEUE_HPP_INCLUDED
#define CONCURRENT_LOCKFREE_SPSCBOUNDEDQUEUE_HPP_INCLUDED

#include <atomic>
#include <cstddef>
#include <memory>

#include "../Aligned/Aligned.hpp";

namespace Concurrent {
	namespace LockFree {
		template<typename T, std::size_t CacheLineSize = 64>
		class SpscBoundedQueue {
			Aligned<std::atomic<T>[], CacheLineSize> buffer;
			Aligned<std::atomic<std::size_t>, CacheLineSize> size;
			std::size_t readIndex;
			std::size_t writeIndex;
		public:
			// Constructors
			SpscBoundedQueue(SpscBoundedQueue const &) = delete;
			SpscBoundedQueue const & operator=(SpscBoundedQueue const &) = delete;
			explicit SpscBoundedQueue(std::size_t capacity)
				: buffer(capacity)
				, size(0)
				, readIndex(0)
				, writeIndex(0)
			{}

			// Capacity
			std::size_t Capacity() const { return buffer.Size();                              }
			std::size_t Size()     const { return size.Ref().load(std::memory_order_seq_cst); }
			bool        Empty()    const { return Size() == 0;                                }
			bool        Full()     const { return Size() == Capacity();                       }

			// Modifiers
			bool Push(T const && source) {
				if (Full())
					return false;
				buffer[writeIndex].store(source, std::memory_order_seq_cst);
				size.Ref().fetch_add(1, std::memory_order_seq_cst);
				++writeIndex;
				if (writeIndex == Capacity())
					writeIndex = 0;
				return true;
			}
			bool Pop(T & destination) {
				if (Empty())
					return false;
				destination = buffer[readIndex].load(std::memory_order_seq_cst);
				size.Ref().fetch_sub(1, std::memory_order_seq_cst);
				++readIndex;
				if (readIndex == Capacity())
					readIndex = 0;
				return true;
			}
		};
	}
}

#endif
