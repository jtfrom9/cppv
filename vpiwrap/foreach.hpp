#ifndef FOREACH_HPP
#define FOREACH_HPP

#include <vector>
#include <list>

namespace foreach_internal {

template<typename ContainerType>
class wrapper
{
    const ContainerType& _container;
    typename ContainerType::const_iterator _iter;

public:
    wrapper( const ContainerType& container ):
        _container(container),
        _iter(container.begin())
    {}

    operator typename ContainerType::value_type() const
    {
        return *_iter;
    }
    
    bool end()
    {
        return _iter == _container.end();
    }

    void next()
    {
        ++_iter;
    }
};
}

#define VECTOR_FOREACH(vtype,var,container) \
    for( foreach_internal::wrapper< std::vector< vtype > > var(container); \
         !var.end(); var.next() )

#define LIST_FOREACH(vtype,var,container) \
    for( foreach_internal::wrapper< std::list< vtype > > var(container); \
         !var.end(); var.next() )

#endif
