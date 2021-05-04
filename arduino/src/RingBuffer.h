/*
  Copyright (c) 2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef __cplusplus

#ifndef _RING_BUFFER_
#define _RING_BUFFER_

#include <stdint.h>

template <class T, int N> class RingBufferN
{
  private:
    T        _buff[N];
    uint32_t _size = N;
    uint32_t _head, _tail;
    uint32_t _hNdx, _tNdx;

  public:
    RingBufferN()
    {
        _head = _tail = 0;
        _hNdx = _tNdx = 0;
    }

    uint32_t GetSize()
    {
        return _size;
    }

    uint32_t GetNumObjStored()
    {
        return _head - _tail;
    }

    uint32_t GetAvailableSpace()
    {
        return _size - GetNumObjStored();
    }

    uint32_t Queue( T obj )
    {
        // Overall length check
        if( GetAvailableSpace() < 1 ) return 0;

        // Add the bytes
        _buff[_hNdx++] = obj;
        _head++;
        if( _hNdx >= _size ) _hNdx = 0;

        return 1;
    }

    uint32_t Queue( T *obj, uint32_t len )
    {
        // Null check
        if( obj == NULL ) return 0;

        // Overall length check
        if( GetAvailableSpace() < len ) return 0;

        // Overflow check
        uint32_t wLen = len;
        uint32_t tLen = ( ( _size - _hNdx ) < len ) ? ( _size - _hNdx ) : len;

        // Write to the buffer
        uint32_t i = 0;
        do {
            memcpy( &_buff[_hNdx], &obj[i], sizeof( T ) * tLen );
            _hNdx += tLen;
            _head += tLen;
            i += tLen;
            len -= tLen;
            if( _hNdx >= _size ) _hNdx = 0;
            tLen = ( ( _size - _hNdx ) < len ) ? ( _size - _hNdx ) : len;
        } while( len > 0 );

        return wLen;
    }

    uint32_t DeQueue( T *obj, uint32_t len = 1 )
    {
        // Null check
        if( obj == NULL ) return 0;

        // Overall length check
        if( GetNumObjStored() < len ) return 0;

        // If length is one make it easy
        if( len == 1 ) {
            *obj = _buff[_tNdx++];
            _tail++;
            if( _tNdx >= _size ) _tNdx = 0;
            return 1;
        }

        // Overflow check
        uint32_t rLen = len;
        uint32_t tLen = ( ( _size - _tNdx ) < len ) ? ( _size - _tNdx ) : len;

        // Read from the buffer
        uint32_t i = 0;
        do {
            memcpy( &obj[i], &_buff[_tNdx], sizeof( T ) * tLen );
            _tNdx += tLen;
            _tail += tLen;
            i += tLen;
            len -= tLen;
            if( _tNdx >= _size ) _tNdx = 0;
            tLen = ( ( _size - _tNdx ) < len ) ? ( _size - _tNdx ) : len;
        } while( len > 0 );

        return rLen;
    }

    T *AccessElement( uint32_t position )
    {
        if( position > GetNumObjStored() ) return NULL;
        uint32_t index = _tNdx + position;
        if( index >= _size ) index -= _size;
        return &_buff[index];
    }

    void Flush( uint32_t len = 0 )
    {
        if( len == 0 ) {
            _tail = _head;
            _tNdx = _hNdx;
        }
        else {
            if( len > GetNumObjStored() ) len = GetNumObjStored();
            _tail += len;
            _tNdx += len;
            if( _tNdx >= _size ) _tNdx -= _size;
        }
    }
};

#endif /* _RING_BUFFER_ */

#endif /* __cplusplus */
