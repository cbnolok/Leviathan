#ifndef COLLECTIONS_H
#define COLLECTIONS_H

#include <algorithm>
#include <vector>

template<typename T>
std::pair<bool, size_t> deleteElementReport(std::vector<T>* vec, T const& elem)
{
    const size_t sizePre = vec->size();
    if (vec->empty())
        return std::make_pair(false, sizePre);

    vec->erase(
                std::remove(vec->begin(), vec->end(), elem),
                vec->end());
    const size_t sizePost = vec->size();

    return std::make_pair((sizePost != sizePre), sizePost);
}

#endif // COLLECTIONS_H
