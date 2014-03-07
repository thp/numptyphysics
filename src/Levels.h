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
#include <vector>

struct LevelDesc {
    LevelDesc()
        : file()
        , rank()
    {
    }

    LevelDesc(const std::string &file, int rank)
        : file(file)
        , rank(rank)
    {
    }

    std::string file;
    int rank;
};

struct Collection {
    Collection()
        : file()
        , name()
        , rank()
        , levels()
    {
    }

    Collection(const std::string &file, const std::string &name, int rank)
        : file(file)
        , name(name)
        , rank(rank)
        , levels()
    {
    }

    std::string file;
    std::string name;
    int rank;
    std::vector<LevelDesc> levels;
};

class Levels
{
 public:
  Levels(std::vector<std::string> dirs);
  bool addPath(const std::string &path);
  bool addLevel(const std::string &file, int rank);
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

  bool addLevel(Collection &collection, const std::string &file, int rank);
  LevelDesc *findLevel(int i);
  Collection &getCollection(const std::string &file);
  bool scanCollection( const std::string& file, int rank );

  int m_numLevels;
  std::vector<Collection> m_collections;
};

#endif //LEVELS_H
