#ifndef CLUTILS_H_INCLUDED
#define CLUTILS_H_INCLUDED

#include <map>

using namespace std;

template<typename _Row, typename _Col, typename _Ele>
class D2Map {
private:
    typedef map< _Col, _Ele> mapCol;
    typedef map< _Row, mapCol> mapRowCol;
    mapRowCol data;
public:
    D2Map() {}
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
