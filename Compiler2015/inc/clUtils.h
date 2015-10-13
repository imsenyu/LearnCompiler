#ifndef CLUTILS_H_INCLUDED
#define CLUTILS_H_INCLUDED

#include "stdafx.h"
#include "json/json.h"

using namespace std;

namespace clUtils{
    int atoi(const string& str);
    string itoa(int k);
};

template<typename _Key, typename _Ele>
class D1Map {
private:
    typedef map<_Key, _Ele> mapKE;
    mapKE data;
public:
    D1Map() {}
    typename mapKE::iterator begin() { return data.begin(); }
    typename mapKE::iterator end() { return data.end(); }
    bool add(const _Key& key, const _Ele& ele, bool _replace = false) {
        typename mapKE::iterator kter = data.find(key);
        if ( kter == data.end() ) {
            data.insert( make_pair(key,ele) );
            return true;
        }
        if ( true == _replace ) {
            kter->second = ele;
            return true;
        }
        return false;
    }
    bool has( const _Key& key ) {
        return !! get(key );
    }
    _Ele* get( const _Key& key ) {
        typename mapKE::iterator kter = data.find(key);
        if ( kter == data.end() ) return NULL;
        else return &kter->second;
    }

};

template<typename _Row, typename _Col, typename _Ele>
class D2Map {
private:
    typedef map< _Col, _Ele> mapCol;
    typedef map< _Row, mapCol> mapRowCol;
    mapRowCol data;
public:
    D2Map() {}
    inline typename mapRowCol::iterator begin() { return data.begin(); }
    inline typename mapRowCol::iterator end() { return data.end(); }

    typename mapCol::iterator colBegin( _Row& row) {
        typename mapRowCol::iterator rter = data.find( row );
        if ( rter == data.end() ) return data.begin()->second.end();
        return rter->second.begin();
    }
    typename mapCol::iterator colEnd( _Row& row ) {
        typename mapRowCol::iterator rter = data.find( row );
        if ( rter == data.end() ) return data.begin()->second.end();
        return rter->second.end();
    }

    bool add( const _Row& row, const _Col& col, _Ele ele, bool _replace = false ) {
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
        if ( true == _replace ) {
            ctor->second = ele;
            return true;
        }
        return false;

    }
    bool has( const _Row& row) {
        typename mapRowCol::iterator rter = data.find( row );
        if ( rter == data.end() ) return false;
        else return true;
    }
    _Ele* get( const _Row& row, const _Col& col) {
        typename mapRowCol::iterator rter = data.find( row );
        if ( rter == data.end() ) return NULL;
        typename mapCol::iterator ctor = rter->second.find( col );
        if ( ctor == rter->second.end() ) return NULL;

        return &ctor->second;
    }
};

#endif // CLUTILS_H_INCLUDED
