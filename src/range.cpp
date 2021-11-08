//
// Created by yuchengye on 2021/11/6.
//
#include "library.h"
#include "range.h"
#include <string>

Range::RangeIterator::RangeIterator(long _var, long _step): var( _var ), step( _step ) {}

Range::RangeIterator &Range::RangeIterator::operator++() {
    var += step;
    return *this;
}

long Range::RangeIterator::operator*() const  { return var; }

bool Range::RangeIterator::operator!=(const Range::RangeIterator &other) const {
    if( step < 0 )
        return (this->var > other.var);
    return (this->var < other.var);
}

/**
 * @brief similar to python class 'range'
 * @param _begin
 * @param _end
 * @param _step
 */
Range::Range(long _begin, long _end, long _step): begin_var( _begin ), end_var( _end ), step( _step ) {}

Range::RangeIterator Range::begin() const { return RangeIterator( begin_var, step ); }

Range::RangeIterator Range::end() const { return RangeIterator( end_var, step ); }

Range::Range(long _end): begin_var(0), end_var(_end), step(1) {}
