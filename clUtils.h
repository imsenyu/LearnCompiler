#ifndef CLUTILS_H_INCLUDED
#define CLUTILS_H_INCLUDED

#include <map>
#include <iterator>

using namespace std;

template<typename _Row, typename _Col, typename _Ele>
class D2Map {
private:
    typedef map< _Col, _Ele> mapCol;
    typedef map< _Row, mapCol> mapRowCol;
    mapRowCol data;
public:
    D2Map() {}
    typename mapRowCol::iterator begin() {
        return data.begin();
    }
    typename mapRowCol::iterator end() {
        return data.end();
    }

    typename mapCol::iterator colBegin( _Row& row) {
        typename mapRowCol::iterator rter = data.find( row );
        if ( rter == data.end() ) return rter;
        return rter->second.begin();
    }
    typename mapCol::iterator colEnd( _Row& row ) {
        typename mapRowCol::iterator rter = data.find( row );
        if ( rter == data.end() ) return rter;
        return rter->second.end();
    }

    bool add( _Row& row, _Col& col, _Ele ele, bool _replace = true ) {
        typename mapRowCol::iterator rter = data.find( row );
        if ( rter == data.end() ) {
            data.insert( make_pair( row, mapCol() ) );
            rter = data.find( row );
        }
        typename mapCol::iterator ctor = rter->second.find( col );
        if ( ctor == rter->second.end() ) {
            rter->second.insert( make_pair( col, ele ) );
            return true;
        }
        if ( _replace ) {
            ctor->second = ele;
            return true;
        }
        return false;

    }
    _Ele* get( _Row& row, _Col& col) {
        typename mapRowCol::iterator rter = data.find( row );
        if ( rter == data.end() ) return NULL;
        typename mapCol::iterator ctor = rter->second.find( col );
        if ( ctor == rter->second.end() ) return NULL;

        return &ctor->second;
    }
};

#endif // CLUTILS_H_INCLUDED
