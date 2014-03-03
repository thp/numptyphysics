/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2008 Tim Edmonds
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#ifndef ARRAY_H
#define ARRAY_H

#include "Common.h"
#include <memory.h>

template <typename T>
class Array
{
 public:

  Array( int cap=0 ) : m_data(NULL), m_size(0), m_capacity(0)
  {
    capacity( cap );
  }
  
 Array( int n, const T* d ) : m_data(NULL), m_size(0), m_capacity(0)
  {
    if ( n ) {
      capacity( n );
      memcpy( m_data, d, n * sizeof(T) );
      m_size = n;
    }
  }

  Array( const Array& other ) : m_data(NULL), m_size(0), m_capacity(0)
  {
    if ( other.size() ) {
      capacity( other.size() );
      memcpy( m_data, other.m_data, other.size() * sizeof(T) );
      m_size = other.size();
    }
  }

  ~Array()
  {
    if ( m_data ) {
      free( m_data );
    }
  }

  int size() const
  {
    return m_size;
  }

  void empty()
  {
    m_size = 0;
  }

  T& at( int i )
  {
    ASSERT( i < m_size );
    return m_data[i];
  }

  const T& at( int i ) const
  {
    ASSERT( i < m_size );
    return m_data[i];
  }

  void append( const T& t )
  {
    ensureCapacity( m_size + 1 );
    m_data[ m_size++ ] = t;
  }

  void insert( int i, const T& t )
  {
    if ( i==m_size ) {
      append( t );
    } else {
      ASSERT( i < m_size );
      ensureCapacity( m_size + 1 );
      for ( int j=m_size-1; j>=i; j-- ) {
	m_data[j+1] = m_data[j];
      }
      m_data[ i ] = t;
      m_size++;
    }
  }

  void erase( int i )
  {
    if (i >= 0 ) {
      ASSERT( i < m_size );
      if ( i < m_size-1 ) {
	memcpy( m_data+i, m_data+i+1, (m_size-i-1)*sizeof(T) );
      }
      m_size--;
    }
  }

  void trim( int i )
  {
    ASSERT( i < m_size );
    m_size -= i;
  }

  void capacity( int c )
  {
    if ( c >= m_size ) {
      if ( m_capacity ) {
	m_data = (T*)realloc( m_data, c * sizeof(T) );
      } else {
	m_data = (T*)malloc( c * sizeof(T) );
      }
      m_capacity = c;
    }
  }

  int indexOf( const T& t )
  {
    for ( int i=0; i<m_size; i++ ) {
      if ( m_data[i] == t ) {
	return i;
      }
    }
    return -1;
  }

  T& operator[]( int i )
  {
    return at(i);
  }

  const T& operator[]( int i ) const
  {
    return at(i);
  }

  Array<T>& operator=(const Array<T>& other) 
  {
    m_size = 0;
    if ( other.size() ) {
      capacity( other.size() );
      memcpy( m_data, other.m_data, other.size() * sizeof(T) );
      m_size = other.size();
    }
    return *this;
  }

 private:
  void ensureCapacity( int c ) 
  {
    if ( c > m_capacity ) {
      int newc = m_capacity ? m_capacity : 4;
      while ( newc < c  ) {
	newc += newc;
      }
      capacity( newc );
    }
  }

  T* m_data;
  int m_size;
  int m_capacity;
};

#endif //ARRAY_H
