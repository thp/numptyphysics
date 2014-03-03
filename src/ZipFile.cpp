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

#include "ZipFile.h"
#include <string>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <zlib.h>

//zip file structs and uncompress_int below lifted from navit project (GPL).
struct zip_lfh {
	int ziplocsig;
	short zipver;
	short zipgenfld;
	short zipmthd;
	short ziptime;
	short zipdate;
	int zipcrc;
	unsigned int zipsize;
	unsigned int zipuncmp;
	unsigned short zipfnln;
	unsigned short zipxtraln;
	char zipname[0];
} __attribute__ ((packed));

struct zip_cd {
	int zipcensig;
	char zipcver;
	char zipcos;
	char zipcvxt;
	char zipcexos;
	short zipcflg;
	short zipcmthd;
	short ziptim;
	short zipdat;
	int zipccrc;
	unsigned int zipcsiz;
	unsigned int zipcunc;
	unsigned short zipcfnl;
	unsigned short zipcxtl;
	unsigned short zipccml;
	unsigned short zipdsk;
	unsigned short zipint;
	unsigned int zipext;
	unsigned int zipofst;
	char zipcfn[0];	
} __attribute__ ((packed));

struct zip_eoc {
	int zipesig;
	unsigned short zipedsk;
	unsigned short zipecen;
	unsigned short zipenum;
	unsigned short zipecenn;
	unsigned int zipecsz;
	unsigned int zipeofst;
	short zipecoml;
	char zipecom[0];
} __attribute__ ((packed));

int uncompress_int(unsigned char *dest, int *destLen, 
		   const unsigned char *source, int sourceLen)
{
  z_stream stream;
  int err;
  
  stream.next_in = (Bytef*)source;
  stream.avail_in = (uInt)sourceLen;
  stream.next_out = dest;
  stream.avail_out = (uInt)*destLen;
  
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;
  
  err = inflateInit2(&stream, -MAX_WBITS);
  if (err != Z_OK) return err;
    
  err = inflate(&stream, Z_FINISH);
  if (err != Z_STREAM_END) {
    inflateEnd(&stream);
    if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
      return Z_DATA_ERROR;
    return err;
    }
  *destLen = stream.total_out;
  
  err = inflateEnd(&stream);
  return err;
}



ZipFile::ZipFile(const std::string& fn)
{
  m_temp = NULL;
  m_fd=open(fn.c_str(), O_RDONLY);
  struct stat stat;
  if (fstat(m_fd, &stat)==0 && S_ISREG(stat.st_mode)) {
    m_dataLen = stat.st_size;
    // TODO - win32
    m_data = (unsigned char*)mmap(NULL,m_dataLen,PROT_READ,MAP_PRIVATE, m_fd, 0);
    if ( !m_data ) throw "mmap failed";
    if ( *(int*)&m_data[0] != 0x04034b50 ) throw "bad zip magic";
    m_eoc = (zip_eoc*)&m_data[m_dataLen-sizeof(zip_eoc)];
    m_firstcd = (zip_cd*)&m_data[m_eoc->zipeofst];
    if ( m_eoc && m_firstcd ) {
      m_entries = m_eoc->zipenum;
    } else {
      m_entries = 0;
    }
  } else {
    throw "invalid zip file";
  }
}


ZipFile::~ZipFile()
{
  delete[] m_temp;
  if ( m_data ) munmap( m_data, m_dataLen );
  if ( m_fd ) close( m_fd );
}


std::string ZipFile::entryName( int n )
{
  if ( n < 0 || n >= m_entries ) return std::string();
  zip_cd* cd = m_firstcd;
  for ( int count=0; cd < (zip_cd*)m_eoc && count < n; count++ ) {
    cd = (zip_cd*)(((char*)cd) + sizeof(zip_cd) + cd->zipcfnl + cd->zipcxtl + cd->zipccml);
  }    
  zip_lfh* lfh = (zip_lfh*)&m_data[cd->zipofst];

  if ( lfh ) {
    return std::string(lfh->zipname,lfh->zipfnln);
  }
  return std::string();
}

unsigned char* ZipFile::extract( int n, int *l )
{
  if ( n < 0 || n >= m_entries ) return NULL;
  zip_cd* cd = m_firstcd;
  for ( int count=0; cd < (zip_cd*)m_eoc && count < n; count++ ) {
    cd = (zip_cd*)(((char*)cd) + sizeof(zip_cd) + cd->zipcfnl + cd->zipcxtl + cd->zipccml);
  }    
  zip_lfh* lfh = (zip_lfh*)&m_data[cd->zipofst];

  if ( lfh ) {
    *l = lfh->zipuncmp;
    unsigned char* zdat = (unsigned char*)lfh + sizeof(*lfh) + lfh->zipfnln + lfh->zipxtraln;
    switch (lfh->zipmthd) {
    case 0: 
      return zdat;
    case 8:
      delete[] m_temp;
      m_temp = new unsigned char[*l];
      if ( uncompress_int(m_temp, l, zdat, lfh->zipsize) == Z_OK) {
	return m_temp;
      }
    }
  }
  return NULL;
}




