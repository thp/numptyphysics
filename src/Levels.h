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

#ifndef LEVELS_H
#define LEVELS_H

#include <cstdio>
#include <sstream>
#include "Array.h"

class Levels
{
 public:
  Levels( int numDirs=0, const char** dirs=NULL );
  bool addPath( const char* path );
  bool addLevel( const std::string& file, int rank=-1, int index=-1 );
  int  numLevels();
  int load( int i, unsigned char* buf, int bufLen );
  std::string levelName( int i, bool pretty=true );
  int findLevel( const char *file );

  void dump();

  int  numCollections();
  int  collectionFromLevel( int l, int *indexInCol=NULL );
  std::string collectionName( int i, bool pretty=true );
  int  collectionSize(int c);
  int  collectionLevel(int c, int i);

  std::string demoPath(int l);
  std::string demoName(int l);
  bool hasDemo(int l);

 private:

  struct LevelDesc
  {
  LevelDesc( const std::string& f,int r=0, int i=-1)
  : file(f), index(i), rank(r) {}
    std::string file;
    int         index;
    int         rank;
  };

  struct Collection
  {
    std::string file;
    std::string name;
    int rank;
    Array<LevelDesc*> levels;
  };

  bool addLevel( Collection* collection,
		 const std::string& file, int rank, int index );
  LevelDesc* findLevel( int i );
  Collection* getCollection( const std::string& file );
  bool scanCollection( const std::string& file, int rank );

  int m_numLevels;
  Array<Collection*> m_collections;
};

#endif //LEVELS_H
