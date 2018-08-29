#ifndef MAPS_H
#define MAPS_H

#include <map>

template <typename key_t, typename value_t>
typename std::map<key_t,value_t>::iterator mapSearchByKey(std::map<key_t,value_t> &_map, const value_t _val)
// returns the iterator to the pair
{
    using it_t = typename std::map<key_t,value_t>::iterator;
    it_t end = _map.end();
    for (it_t it = _map.begin(); it != end; ++it)
    {
        if ( it->second == _val )
            return it;
    }
    return end;
}

#endif // MAPS_H
