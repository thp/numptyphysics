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
#ifndef ZIPFILE_H
#define ZIPFILE_H

#include <string>

struct zip_eoc;
struct zip_cd;
struct zip_lfh;

class ZipFile 
{
public:
  ZipFile(const std::string& fn);
  ~ZipFile();
  int numEntries() { return m_entries; }
  std::string entryName( int n );
  unsigned char* extract( int n, int *l );

private:
  int m_fd;
  int m_dataLen;
  unsigned char* m_data;
  zip_eoc* m_eoc;
  zip_cd*  m_firstcd; 
  int m_entries;
  unsigned char*m_temp;
};


#endif //ZIPFILE_H
