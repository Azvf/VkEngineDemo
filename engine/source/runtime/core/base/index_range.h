#pragma once

#include <algorithm>
#include <iosfwd>
#include <cassert>

namespace Chandelier
{
    template<typename T>
    class Span;

    class Range
    {
    private:
        int64_t start_ = 0;
        int64_t size_  = 0;

    public:
        constexpr Range() = default;

        constexpr explicit Range(int64_t size) : start_(0), size_(size) { assert(size >= 0); }

        constexpr Range(int64_t start, int64_t size) : start_(start), size_(size)
        {
            assert(start >= 0);
            assert(size >= 0);
        }

        class Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = int64_t;
            using pointer           = const int64_t*;
            using reference         = const int64_t&;
            using difference_type   = std::ptrdiff_t;

        private:
            int64_t current_;

        public:
            constexpr explicit Iterator(int64_t current) : current_(current) {}

            constexpr Iterator& operator++()
            {
                current_++;
                return *this;
            }

            constexpr Iterator operator++(int)
            {
                Iterator copied_iterator = *this;
                ++(*this);
                return copied_iterator;
            }

            constexpr friend bool operator!=(const Iterator& a, const Iterator& b) { return a.current_ != b.current_; }

            constexpr friend bool operator==(const Iterator& a, const Iterator& b) { return a.current_ == b.current_; }

            constexpr friend int64_t operator-(const Iterator& a, const Iterator& b) { return a.current_ - b.current_; }

            constexpr int64_t operator*() const { return current_; }
        };

        constexpr Iterator begin() const { return Iterator(start_); }

        constexpr Iterator end() const { return Iterator(start_ + size_); }

        /**
         * Access an element in the range.
         */
        constexpr int64_t operator[](int64_t index) const
        {
            assert(index >= 0);
            assert(index < this->size());
            return start_ + index;
        }

        /**
         * Two ranges compare equal when they contain the same numbers.
         */
        constexpr friend bool operator==(Range a, Range b)
        {
            return (a.size_ == b.size_) && (a.start_ == b.start_ || a.size_ == 0);
        }
        constexpr friend bool operator!=(Range a, Range b) { return !(a == b); }

        /**
         * Get the amount of numbers in the range.
         */
        constexpr int64_t size() const { return size_; }

        constexpr Range index_range() const { return Range(size_); }

        /**
         * Returns true if the size is zero.
         */
        constexpr bool is_empty() const { return size_ == 0; }

        /**
         * Create a new range starting at the end of the current one.
         */
        constexpr Range after(int64_t n) const
        {
            assert(n >= 0);
            return Range(start_ + size_, n);
        }

        /**
         * Create a new range that ends at the start of the current one.
         */
        constexpr Range before(int64_t n) const
        {
            assert(n >= 0);
            return Range(start_ - n, n);
        }

        /**
         * Get the first element in the range.
         * Asserts when the range is empty.
         */
        constexpr int64_t first() const
        {
            assert(this->size() > 0);
            return start_;
        }

        /**
         * Get the nth last element in the range.
         * Asserts when the range is empty or when n is negative.
         */
        constexpr int64_t last(const int64_t n = 0) const
        {
            assert(n >= 0);
            assert(n < size_);
            assert(this->size() > 0);
            return start_ + size_ - 1 - n;
        }

        /**
         * Get the element one before the beginning. The returned value is undefined when the range is
         * empty, and the range must start after zero already.
         */
        constexpr int64_t one_before_start() const
        {
            assert(start_ > 0);
            return start_ - 1;
        }

        /**
         * Get the element one after the end. The returned value is undefined when the range is empty.
         */
        constexpr int64_t one_after_last() const { return start_ + size_; }

        /**
         * Get the first element in the range. The returned value is undefined when the range is empty.
         */
        constexpr int64_t start() const { return start_; }

        /**
         * Returns true when the range contains a certain number, otherwise false.
         */
        constexpr bool contains(int64_t value) const { return value >= start_ && value < start_ + size_; }

        /**
         * Returns a new range, that contains a sub-interval of the current one.
         */
        constexpr Range slice(int64_t start, int64_t size) const
        {
            assert(start >= 0);
            assert(size >= 0);
            int64_t new_start = start_ + start;
            assert(new_start + size <= start_ + size_ || size == 0);
            return Range(new_start, size);
        }
        constexpr Range slice(Range range) const { return this->slice(range.start(), range.size()); }

        /**
         * Returns a new Range that contains the intersection of the current one with the given
         * range. Returns empty range if there are no overlapping indices. The returned range is always
         * a valid slice of this range.
         */
        constexpr Range intersect(Range other) const
        {
            const int64_t old_end   = start_ + size_;
            const int64_t new_start = std::min(old_end, std::max(start_, other.start_));
            const int64_t new_end   = std::max(new_start, std::min(old_end, other.start_ + other.size_));
            return Range(new_start, new_end - new_start);
        }

        /**
         * Returns a new Range with n elements removed from the beginning of the range.
         * This invokes undefined behavior when n is negative.
         */
        constexpr Range drop_front(int64_t n) const
        {
            assert(n >= 0);
            const int64_t new_size = std::max<int64_t>(0, size_ - n);
            return Range(start_ + n, new_size);
        }

        /**
         * Returns a new Range with n elements removed from the end of the range.
         * This invokes undefined behavior when n is negative.
         */
        constexpr Range drop_back(int64_t n) const
        {
            assert(n >= 0);
            const int64_t new_size = std::max<int64_t>(0, size_ - n);
            return Range(start_, new_size);
        }

        /**
         * Returns a new Range that only contains the first n elements. This invokes undefined
         * behavior when n is negative.
         */
        constexpr Range take_front(int64_t n) const
        {
            assert(n >= 0);
            const int64_t new_size = std::min<int64_t>(size_, n);
            return Range(start_, new_size);
        }

        /**
         * Returns a new Range that only contains the last n elements. This invokes undefined
         * behavior when n is negative.
         */
        constexpr Range take_back(int64_t n) const
        {
            assert(n >= 0);
            const int64_t new_size = std::min<int64_t>(size_, n);
            return Range(start_ + size_ - new_size, new_size);
        }

        /**
         * Move the range forward or backward within the larger array. The amount may be negative,
         * but its absolute value cannot be greater than the existing start of the range.
         */
        constexpr Range shift(int64_t n) const { return Range(start_ + n, size_); }

        friend std::ostream& operator<<(std::ostream& stream, Range range);
    };

    // struct AlignedIndexRanges
    // {
    //     Range prefix;
    //     Range aligned;
    //     Range suffix;
    // };
    // 
    // /**
    //  * Split a range into three parts so that the boundaries of the middle part are aligned to some
    //  * power of two.
    //  *
    //  * This can be used when an algorithm can be optimized on aligned indices/memory. The algorithm
    //  * then needs a slow path for the beginning and end, and a fast path for the aligned elements.
    //  */
    // AlignedIndexRanges split_index_range_by_alignment(const Range range, const int64_t alignment);

}