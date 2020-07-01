#ifndef TIM_SORT_H_
#define TIM_SORT_H_

#include <algorithm>
#include <functional>
#include <stack>
#include <vector>

#ifndef GALOOP_MODE
#define GALOOP_MODE true
#endif

#ifndef MIN_GALOOP
#define MIN_GALOOP 7
#endif

namespace timsort
{
template <typename Iterator>
using IsRandomAccessIterator = typename std::enable_if<
    std::__and_<
        std::is_same<typename std::iterator_traits<Iterator>::iterator_category, std::random_access_iterator_tag>,
        std::is_same<typename std::iterator_traits<Iterator>::difference_type, ptrdiff_t>,
        std::is_same<typename std::iterator_traits<Iterator>::pointer, typename std::iterator_traits<Iterator>::value_type *>,
        std::is_same<typename std::iterator_traits<Iterator>::reference, typename std::iterator_traits<Iterator>::value_type &>>::value,
    int>::type;

template <typename T, typename Less = std::less<T>>
void tim_sort(T arr[], size_t len);

template <typename Iterator, typename Less = std::less<typename std::iterator_traits<Iterator>::value_type>, IsRandomAccessIterator<Iterator> = 0>
void tim_sort(Iterator first, Iterator last);

template <typename Iterator, IsRandomAccessIterator<Iterator> = 0>
void reverse(Iterator first, size_t l, size_t r);

template <typename Iterator, typename Less = std::less<typename std::iterator_traits<Iterator>::value_type>, IsRandomAccessIterator<Iterator> = 0>
void insertion_sort(Iterator first, size_t l, size_t r);

template <typename Iterator, typename Less = std::less<typename std::iterator_traits<Iterator>::value_type>, IsRandomAccessIterator<Iterator> = 0>
void merge_sort(Iterator first, size_t l, size_t m, size_t r);

template <typename Iterator, typename Less = std::less<typename std::iterator_traits<Iterator>::value_type>, IsRandomAccessIterator<Iterator> = 0>
void merge_lo(Iterator first, typename std::iterator_traits<Iterator>::value_type lo[], Iterator hi, size_t lo_len, size_t hi_len);

template <typename Iterator, typename Less = std::less<typename std::iterator_traits<Iterator>::value_type>, IsRandomAccessIterator<Iterator> = 0>
void merge_hi(Iterator first, Iterator lo, typename std::iterator_traits<Iterator>::value_type hi[], size_t lo_len, size_t hi_len);

template <typename Iterator, typename Less = std::less<typename std::iterator_traits<Iterator>::value_type>, IsRandomAccessIterator<Iterator> = 0>
size_t bisect_left(Iterator first, size_t len, typename std::iterator_traits<Iterator>::value_type v);

template <typename Iterator, typename Less = std::less<typename std::iterator_traits<Iterator>::value_type>, IsRandomAccessIterator<Iterator> = 0>
size_t bisect_right(Iterator first, size_t len, typename std::iterator_traits<Iterator>::value_type v);
} /* namespace timsort */

template <typename T, typename Less>
inline void timsort::tim_sort(T arr[], size_t len)
{
    tim_sort(arr, reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(arr) + len * sizeof(T)));
}

template <typename Iterator, typename Less, timsort::IsRandomAccessIterator<Iterator>>
inline void timsort::tim_sort(Iterator first, Iterator last)
{
    static const Less less;
    size_t len = last - first;
    size_t min_run;
    {
        size_t r = 0, n = len;
        while (n >= 64)
        {
            r |= n & 1;
            n >>= 1;
        }
        min_run = n + r;
    }
    std::stack<std::pair<size_t, size_t>> runs;
    size_t base_run = 0;
    while (base_run < len - 1)
    {
        size_t i = base_run, run_size = 2;
        if (!less(first[i + 1], first[i]))
        {
            ++i;
            while (i < len - 1 && !less(first[i + 1], first[i]))
            {
                ++i;
                ++run_size;
            }
            if (run_size < min_run)
            {
                size_t r = std::min(base_run + min_run - 1, len - 1);
                insertion_sort<Iterator, Less>(first, base_run, r);
                run_size = r + 1 - base_run;
            }
        }
        else
        {
            ++i;
            while (i < len - 1 && less(first[i + 1], first[i]))
            {
                ++i;
                ++run_size;
            }
            if (run_size < min_run)
            {
                size_t r = std::min(base_run + min_run - 1, len - 1);
                insertion_sort<Iterator, Less>(first, base_run, r);
                run_size = r + 1 - base_run;
            }
            else
            {
                reverse(first, base_run, base_run + run_size - 1);
            }
        }
        runs.emplace(base_run, run_size);
        base_run += run_size;
        while (runs.size() >= 2)
        {
            if (runs.size() == 2)
            {
                auto x = runs.top();
                runs.pop();
                auto y = runs.top();
                runs.pop();
                if (y.second > x.second)
                {
                    runs.push(y);
                    runs.push(x);
                    break;
                }
                merge_sort<Iterator, Less>(first, y.first, x.first - 1, x.first + x.second - 1);
                runs.emplace(y.first, x.second + y.second);
            }
            else
            {
                auto x = runs.top();
                runs.pop();
                auto y = runs.top();
                runs.pop();
                auto z = runs.top();
                runs.pop();
                if (y.second > x.second && z.second > x.second + y.second)
                {
                    runs.push(z);
                    runs.push(y);
                    runs.push(x);
                    break;
                }
                if (x.second <= z.second)
                {
                    merge_sort<Iterator, Less>(first, y.first, x.first - 1, x.first + x.second - 1);
                    runs.push(z);
                    runs.emplace(y.first, x.second + y.second);
                }
                else
                {
                    merge_sort<Iterator, Less>(first, z.first, y.first - 1, y.first + y.second - 1);
                    runs.emplace(z.first, y.second + z.second);
                    runs.push(x);
                }
            }
        }
    }
    while (runs.size() >= 2)
    {
        auto x = runs.top();
        runs.pop();
        auto y = runs.top();
        runs.pop();
        merge_sort<Iterator, Less>(first, y.first, x.first - 1, x.first + x.second - 1);
        runs.emplace(y.first, x.second + y.second);
    }
}

template <typename Iterator, timsort::IsRandomAccessIterator<Iterator>>
inline void timsort::reverse(Iterator first, size_t l, size_t r)
{
    size_t m = (l + r) / 2;
    for (size_t i = l, j = r; i <= m; ++i, --j)
    {
        std::swap(first[i], first[j]);
    }
}

template <typename Iterator, typename Less, timsort::IsRandomAccessIterator<Iterator>>
inline void timsort::insertion_sort(Iterator first, size_t l, size_t r)
{
    static const Less less;
    for (size_t i = l + 1; i <= r; ++i)
    {
        auto temp = first[i];
        size_t j = i - 1;
        while (less(temp, first[j]) && j >= l)
        {
            first[j + 1] = first[j];
            --j;
        }
        first[j + 1] = temp;
    }
}

template <typename Iterator, typename Less, timsort::IsRandomAccessIterator<Iterator>>
inline void timsort::merge_sort(Iterator first, size_t l, size_t m, size_t r)
{
    using T = typename std::iterator_traits<Iterator>::value_type;
#if GALOOP_MODE
    static const Less less;
#endif
    size_t len1 = m - l + 1, len2 = r - m;
    if (len1 <= len2)
    {
        T *left = new T[len1];
        for (size_t i = 0; i < len1; ++i)
        {
            left[i] = first[i + l];
        }
        auto right = first + (m + 1);
#if GALOOP_MODE
        if (less(first[m], right[len2 - MIN_GALOOP]))
        {
            size_t i = bisect_left<Iterator, Less>(right, len2, first[m]);
            merge_lo<Iterator, Less>(first + l, left, right, len1, i);
        }
        else
        {
            merge_lo<Iterator, Less>(first + l, left, right, len1, len2);
        }
#else
        merge_lo<Iterator, Less>(first + l, left, right, len1, len2);
#endif
        delete[] left;
    }
    else
    {
        auto left = first + l;
        T *right = new T[len2];
        for (size_t i = 0; i < len2; ++i)
        {
            right[i] = first[i + m + 1];
        }
#if GALOOP_MODE
        if (less(left[MIN_GALOOP], right[0]))
        {
            size_t i = bisect_right<Iterator, Less>(left, len1, right[0]);
            left = left + i;
            merge_hi<Iterator, Less>(left, left, right, len1 - i, len2);
        }
        else
        {
            merge_hi<Iterator, Less>(left, left, right, len1, len2);
        }
#else
        merge_hi<Iterator, Less>(left, left, right, len1, len2);
#endif
        delete[] right;
    }
}

template <typename Iterator, typename Less, timsort::IsRandomAccessIterator<Iterator>>
inline void timsort::merge_lo(Iterator first, typename std::iterator_traits<Iterator>::value_type lo[], Iterator hi, size_t lo_len, size_t hi_len)
{
    static const Less less;
    size_t i = 0, j = 0, k = 0;
    while (i < lo_len && j < hi_len)
    {
        if (less(lo[i], hi[j]))
        {
            first[k++] = lo[i++];
        }
        else
        {
            first[k++] = hi[j++];
        }
    }
    while (i < lo_len)
    {
        first[k++] = lo[i++];
    }
    while (j < hi_len)
    {
        first[k++] = hi[j++];
    }
}

template <typename Iterator, typename Less, timsort::IsRandomAccessIterator<Iterator>>
inline void timsort::merge_hi(Iterator first, Iterator lo, typename std::iterator_traits<Iterator>::value_type hi[], size_t lo_len, size_t hi_len)
{
    static const Less less;
    int64_t i = lo_len - 1, j = hi_len - 1, k = lo_len + hi_len - 1;
    while (i >= 0 && j >= 0)
    {
        if (less(lo[i], hi[j]))
        {
            first[k--] = hi[j--];
        }
        else
        {
            first[k--] = lo[i--];
        }
    }
    while (i >= 0)
    {
        first[k--] = lo[i--];
    }
    while (j >= 0)
    {
        first[k--] = hi[j--];
    }
}

template <typename Iterator, typename Less, timsort::IsRandomAccessIterator<Iterator>>
inline size_t timsort::bisect_left(Iterator first, size_t len, typename std::iterator_traits<Iterator>::value_type v)
{
    static const Less less;
    size_t l = 0;
    while (l < len)
    {
        size_t m = (len + l) / 2;
        if (less(first[m], v))
        {
            l = m + 1;
        }
        else
        {
            len = m;
        }
    }
    return l;
}

template <typename Iterator, typename Less, timsort::IsRandomAccessIterator<Iterator>>
inline size_t timsort::bisect_right(Iterator first, size_t len, typename std::iterator_traits<Iterator>::value_type v)
{
    static const Less less;
    size_t l = 0;
    while (l < len)
    {
        size_t m = (len + l) / 2;
        if (less(v, first[m]))
        {
            len = m;
        }
        else
        {
            l = m + 1;
        }
    }
    return l;
}

#endif