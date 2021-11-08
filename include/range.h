//
// Created by yuchengye on 2021/11/6.
//

#ifndef JOBLIB_RANGE_H
#define JOBLIB_RANGE_H

class Range {
public:
    class RangeIterator {
    public:
        explicit RangeIterator( long _var, long _step );
        RangeIterator& operator++();
        long operator*() const;
        bool operator!=( const RangeIterator& other ) const;
    private:
        long var, step;
    };
    explicit Range(long _end);
    Range( long _begin, long _end, long _step = 1 );
    RangeIterator begin() const;
    RangeIterator end() const;

private:
    long begin_var, end_var, step;
};

template<typename F>
int apply(F const& func){
    return func(1, 2);
}


#endif //JOBLIB_RANGE_H
