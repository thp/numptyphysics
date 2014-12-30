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
#include <stdlib.h>

template <typename T>
class SortableVector : public std::vector<T> {
public:
    SortableVector() : std::vector<T>() {}

    void sort()
    {
        qsort(std::vector<T>::data(), std::vector<T>::size(), sizeof(T),
              (int (*)(const void *, const void *))T::compare);
    }
};

struct LevelDesc {
    LevelDesc()
        : file()
    {
    }

    LevelDesc(const std::string &file)
        : file(file)
    {
    }

    static int compare(const LevelDesc *a, const LevelDesc *b);

    std::string file;
};

struct Collection {
    Collection()
        : file()
        , name()
        , levels()
    {
    }

    Collection(const std::string &file, const std::string &name)
        : file(file)
        , name(name)
        , levels()
    {
    }

    static int compare(const Collection *a, const Collection *b);

    std::string file;
    std::string name;
    SortableVector<LevelDesc> levels;
};

class Levels
{
 public:
  Levels(std::vector<std::string> dirs);

  bool addPath(const std::string &path);

  int  numLevels();
  std::string load(int i);
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

  void sort();

 private:
  bool addLevel(const std::string &file);

  bool addLevel(Collection &collection, const std::string &file);
  LevelDesc *findLevel(int i);
  Collection &getCollection(const std::string &file);
  bool scanCollection(const std::string& file);

  int m_numLevels;
  SortableVector<Collection> m_collections;
};

#endif //LEVELS_H
