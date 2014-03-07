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

#include <cstring>
#include <sys/types.h>
#include <dirent.h>

#include "Levels.h"
#include "Config.h"
#include "Os.h"

static const char MISC_COLLECTION[] = "My Levels";
static const char DEMO_COLLECTION[] = "My Solutions";

static int rankFromPath( const std::string& p, int defaultrank=9999 )
{
  if (p==MISC_COLLECTION) {
    return 10000;
  } else if (p==DEMO_COLLECTION) {
    return 20000;
  }
  const char *c = p.data();
  size_t i = p.rfind(Os::pathSep);
  if ( i != std::string::npos ) {
    c += i+1;
    if ( *c=='L' || *c == 'C' ){
      c++;
      int rank=0;
      while ( *c>='0' && *c<='9' ) {
	rank = rank*10 + (*c)-'0';
	c++;
      }
      return rank;
    } else {
      c++;
    }
  }
  return defaultrank;
}

std::string nameFromPath(const std::string& path) 
{
  // TODO extract name from collection manifest
  std::string name;
  size_t i = path.rfind(Os::pathSep);
  if ( i != std::string::npos ) {
    i++;
  } else {
    i = 0;
  }
  if (path[i] == 'C') i++;
  if (path[i] == 'L') i++;
  while (path[i] >= '0' && path[i] <= '9') i++;
  if (path[i] == '_') i++;
  size_t e = path.rfind('.');
  name = path.substr(i,e-i);
  for (i=0; i<name.size(); i++) {
    if (name[i]=='-' || name[i]=='_' || name[i]=='.') {
      name[i] = ' ';
    }
  }
  return name;
}

Levels::Levels(std::vector<std::string> dirs)
    : m_numLevels(0)
{
    for (auto &dir: dirs) {
        addPath(dir);
    }
}

static std::string fileExtension(const std::string &path)
{
    size_t pos = path.find_last_of('.');

    if (pos != std::string::npos) {
        return path.substr(pos);
    }

    return "";
}

bool Levels::addPath(const std::string &path)
{
    std::string ext = fileExtension(path);

    if (ext == ".nph" || ext == ".npd") {
        addLevel(path, rankFromPath(path));
        return true;
    }

    return scanCollection(path, rankFromPath(path));
}

bool Levels::addLevel(const std::string& file, int rank)
{
    if (fileExtension(file) == ".npd") {
        return addLevel(getCollection(DEMO_COLLECTION), file, rank);
    } else {
        return addLevel(getCollection(MISC_COLLECTION), file, rank);
    }
}

bool Levels::addLevel(Collection &collection, const std::string &file, int rank)
{
    auto &levels = collection.levels;
    for (auto it = levels.begin(); it != levels.end(); ++it) {
        auto &level = *it;

        if (level.file == file) {
            //printf("addLevel %s already present!\n",file.c_str());
            return false;
        } else if (level.rank > rank) {
            //printf("insert level %s at %d\n",file.c_str(),i);
            levels.insert(it, LevelDesc(file, rank));
            m_numLevels++;
            return true;
        }
    }

    levels.push_back(LevelDesc(file, rank));
    //printf("add level %s as %s[%d]\n",file.c_str(),
    // collection->file.c_str(), collection->levels.size());
    m_numLevels++;
    return true;
}


Collection &Levels::getCollection(const std::string &file)
{
    for (auto &collection: m_collections) {
        if (collection.file == file) {
            return collection;
        }
    }

    int rank = rankFromPath(file);
    for (auto it = m_collections.begin(); it != m_collections.end(); ++it) {
        auto &collection = *it;
        if (collection.rank > rank) {
            return *m_collections.insert(it, Collection(file, file, rank));
        }
    }

    m_collections.push_back(Collection(file, file, rank));
    return m_collections.back();
}


bool Levels::scanCollection( const std::string& file, int rank )
{
    std::string collectionName = file.substr(file.find_last_of('/')+1);
    DIR *dir = opendir(file.c_str());
    if (dir) {
        bool result = false;
        while (struct dirent *entry = readdir(dir)) {
            if (entry->d_name[0] == '.') {
                continue;
            }

            std::string filename = file + "/" + entry->d_name;
            std::string ext = fileExtension(filename);
            if (ext == ".nph") {
                if (addLevel(getCollection(collectionName), filename, rank)) {
                    result = true;
                }
            } else if (ext == ".npd") {
                if (addLevel(getCollection(DEMO_COLLECTION), filename, rank)) {
                    result = true;
                }
            } else if (addPath(filename)) {
                result = true;
            }
        }

        closedir(dir);
        return result;
    }

    return false;
}

int Levels::numLevels()
{
  return m_numLevels;
}


int Levels::load( int i, unsigned char* buf, int bufLen )
{
  int l = 0;

  LevelDesc *lev = findLevel(i);
  if (lev) {
      FILE *f = fopen( lev->file.c_str(), "rt" );
      if ( f ) {
	l = fread( buf, 1, bufLen, f );
	fclose(f);
      }
    return l;
  }

  throw "invalid level index";  
}

std::string Levels::levelName( int i, bool pretty )
{
  std::string s = "end";
  LevelDesc *lev = findLevel(i);
  if (lev) {
      s = lev->file;
  } else {
    s = "err";
  }
  return pretty ? nameFromPath(s) : s;
}

void
Levels::dump()
{
    for (int i=0; i<m_collections.size(); i++) {
        printf("Collection #%d: %s\n", (i+1),
                collectionName(i, true).c_str());
        for (int j=0; j<m_collections[i].levels.size(); j++) {
            LevelDesc &level = m_collections[i].levels[j];
            printf(" Level #%d: %s (rank=%d)\n", (j+1),
                    level.file.c_str(), level.rank);
        }
    }
}

int Levels::numCollections()
{
  return m_collections.size();
}

int Levels::collectionFromLevel( int i, int *indexInCol )
{
  if (i < m_numLevels) {
    for ( int c=0; c<m_collections.size(); c++ ) {
      if ( i >= m_collections[c].levels.size() ) {
	i -= m_collections[c].levels.size();
      } else {
	if (indexInCol) *indexInCol = i;
	return c;
      }
    }
  }

  return 0;
}

std::string Levels::collectionName( int i, bool pretty )
{
  if (i>=0 && i<numCollections()) {
    if (pretty) {
      return nameFromPath(m_collections[i].name);
    } else {
      return m_collections[i].name;
    }
  }
  return "Bad Collection ID";
}


int Levels::collectionSize(int c)
{
  if (c>=0 && c<numCollections()) {
    return m_collections[c].levels.size();
  }
  return 0;
}

int Levels::collectionLevel(int c, int i)
{
  if (c>=0 && c<numCollections()) {
    if (i>=0 && i<m_collections[c].levels.size()) {
      int l = i;
      for (int j=0; j<c; j++) {
	l += m_collections[j].levels.size();
      }
      return l;
    }
  }
  return 0;
}


std::string Levels::demoPath(int l)
{
  std::string name = levelName(l,false);
  std::string ext = fileExtension(name);
  if (ext == ".npd") {
    /* Kludge: If the level from which we want to save a demo is
     * already a demo file, return an empty string to signal
     * "don't have this demo" - see Game.cpp */
    return "";
  }

  int c = collectionFromLevel(l);
  std::string path = Config::userDataDir() + Os::pathSep
    + "Recordings" + Os::pathSep
    + collectionName(c,false);
  if (fileExtension(path) == ".npz") {
    path.resize(path.length()-4);
  }
  return path;
}

std::string Levels::demoName(int l)
{
  std::string name = levelName(l,false);

  size_t sep = name.rfind(Os::pathSep);
  if (sep != std::string::npos) {
    name = name.substr(sep);
  }

  if (fileExtension(name) == ".nph") {
      name.resize(name.length()-4);
  }
  return demoPath(l) + Os::pathSep + name + ".npd";
}

bool Levels::hasDemo(int l)
{
  return OS->exists(demoName(l));
}


LevelDesc* Levels::findLevel( int i )
{
  if (i < m_numLevels) {
    for ( int c=0; c<m_collections.size(); c++ ) {
      if ( i >= m_collections[c].levels.size() ) {
	//fprintf(stderr,"index %d not in c%d (size=%d)\n",i,c,m_collections[c]->levels.size());
	i -= m_collections[c].levels.size();
      } else {
	return &(m_collections[c].levels[i]);
      }
    }
  }
  return NULL;
}


int Levels::findLevel( const char *file )
{
  int index = 0;
  for ( int c=0; c<m_collections.size(); c++ ) {
    for ( int i=0; i<m_collections[c].levels.size(); i++ ) {
      if ( m_collections[c].levels[i].file == file ) {
	return index + i;
      }
    }
    index += m_collections[c].levels.size();
  }
  return -1;
}


