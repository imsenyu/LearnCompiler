#include "clUtils.h"

int clUtils::atoi(const string& str) {
    int ret = 0;
    for(auto ch : str) {
        ret *= 10;
        ret += ch-'0';
    }
    return ret;
}


template<typename _Key, typename _Ele>
bool D1Map<_Key,_Ele>::add(const _Key& key, const _Ele& ele, bool _replace) {
    typename D1Map<_Key,_Ele>::mapKE::iterator kter = data.find(key);
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

template<typename _Key, typename _Ele>
bool D1Map<_Key,_Ele>::has( const _Key& key ) {
    return !! get(key );
}

template<typename _Key, typename _Ele>
_Ele* D1Map<_Key,_Ele>::get( const _Key& key ) {
    typename mapKE::iterator kter = data.find(key);
    if ( kter == data.end() ) return NULL;
    else return &kter->second;
}



template<typename _Row, typename _Col, typename _Ele>
typename D2Map<_Row,_Col,_Ele>::mapCol::iterator D2Map<_Row,_Col,_Ele>::colBegin( _Row& row) {
    typename mapRowCol::iterator rter = data.find( row );
    if ( rter == data.end() ) return rter;
    return rter->second.begin();
}
template<typename _Row, typename _Col, typename _Ele>
typename D2Map<_Row,_Col,_Ele>::mapCol::iterator D2Map<_Row,_Col,_Ele>::colEnd( _Row& row ) {
    typename mapRowCol::iterator rter = data.find( row );
    if ( rter == data.end() ) return rter;
    return rter->second.end();
}
template<typename _Row, typename _Col, typename _Ele>
bool D2Map<_Row,_Col,_Ele>::add( const _Row& row, const _Col& col, _Ele ele, bool _replace ) {
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
    else {
        printf("CCCCCCCC\n");
    }
    return false;

}
template<typename _Row, typename _Col, typename _Ele>
_Ele* D2Map<_Row,_Col,_Ele>::get( const _Row& row, const _Col& col) {
    typename mapRowCol::iterator rter = data.find( row );
    if ( rter == data.end() ) return NULL;
    typename mapCol::iterator ctor = rter->second.find( col );
    if ( ctor == rter->second.end() ) return NULL;

    return &ctor->second;
}
