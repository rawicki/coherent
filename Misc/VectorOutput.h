#ifndef MISC_VECTOROUTPUT_H
#define MISC_VECTOROUTPUT_H

#include <iostream>
#include <vector>

template <typename T>
std::ostream& operator<< (std::ostream& os, const std::vector<T>& vec)
{
    os << "[";
    for (typename std::vector<T>::const_iterator it=vec.begin() ; it!=vec.end(); ++it)
    {
        if (it!=vec.begin()) {
            os << ", ";
        }
        os << *it;
    }
    os << "]";
    return os;
}


#endif /* MISC_VECTOROUTPUT_H */
