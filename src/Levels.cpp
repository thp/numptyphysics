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
#include "ZipFile.h"
#include "Config.h"
#include "Os.h"

using namespace std;

static const char MISC_COLLECTION[] = "My Levels";
static const char DEMO_COLLECTION[] = "My Solutions";

static int rankFromPath( const string& p, int defaultrank=9999 )
{
  if (p==MISC_COLLECTION) {
    return 10000;
  } else if (p==DEMO_COLLECTION) {
    return 20000;
  }
  const char *c = p.data();
  size_t i = p.rfind(Os::pathSep);
  if ( i != string::npos ) {
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
  if ( i != string::npos ) {
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

Levels::Levels( int numFiles, const char** names )
  : m_numLevels(0)
{
  for ( int d=0;d<numFiles;d++ ) {
    addPath( names[d] );
  }
}

bool Levels::addPath( const char* path )
{
  int len = strlen( path );
  if ( strcasecmp( path+len-4, ".npz" )==0 ) {
    scanCollection( string(path), rankFromPath(path) );
  } else if ( strcasecmp( path+len-4, ".nph" )==0 
	      || strcasecmp( path+len-4, ".npd" )==0) {
    addLevel( path, rankFromPath(path) );
  } else {
    DIR *dir = opendir( path );
    if ( dir ) {
      struct dirent* entry;
      while ( (entry = readdir( dir )) != NULL ) {
	if ( entry->d_name[0] != '.' ) {
	  string full( path );
	  full += "/";
	  full += entry->d_name;
	  //DANGER - recursion may not halt for linked dirs 
	  addPath( full.c_str() );
	}
      }
      closedir( dir );
    } else {
      //printf("bogus level path %s\n",path);
    }
  }
  return true;
}

bool Levels::addLevel( const string& file, int rank, int index )
{
  if (file.substr(file.length()-4) == ".npd") {
    return addLevel( getCollection(DEMO_COLLECTION), file, rank, index );
  } else {
    return addLevel( getCollection(MISC_COLLECTION), file, rank, index );
  }
}

bool Levels::addLevel( Collection* collection,
		       const string& file, int rank, int index )
{
  LevelDesc *e = new LevelDesc( file, rank, index );
  for ( int i=0; i<collection->levels.size(); i++ ) {
    if ( collection->levels[i]->file == file
	 && collection->levels[i]->index == index ) {
      //printf("addLevel %s already present!\n",file.c_str());
      return false;
    } else if ( collection->levels[i]->rank > rank ) {
      //printf("insert level %s+%d at %d\n",file.c_str(),index,i);
      collection->levels.insert(i,e);
      m_numLevels++;
      return true;
    }
  }
  collection->levels.append( e );
  //printf("add level %s+%d as %s[%d]\n",file.c_str(),index,
  // collection->file.c_str(), collection->levels.size());
  m_numLevels++;
  return true;
}


Levels::Collection* Levels::getCollection( const std::string& file )
{
  for (int i=0; i<m_collections.size(); i++) {
    if (m_collections[i]->file == file) {
      return m_collections[i];
    }
  }
  Collection *c = new Collection();
  //fprintf(stderr,"New Collection %s\n",file.c_str());
  c->file = file;
  c->name = file;
  c->rank = rankFromPath(file);
  for (int i=0; i<m_collections.size(); i++) {
    if (m_collections[i]->rank > c->rank) { 
      m_collections.insert(i,c);
      return c;
    }
  }
  m_collections.append(c);
  return c;
}


bool Levels::scanCollection( const std::string& file, int rank )
{
  try {
    ZipFile zf(file);
    Collection *collection = getCollection(file);
    //printf("found collection %s with %d levels\n",file.c_str(),zf.numEntries());
    for ( int i=0; i<zf.numEntries(); i++ ) {
      addLevel( collection, file, rankFromPath(zf.entryName(i),rank), i );
    }
  } catch (...) {
    fprintf(stderr,"invalid collection %s\n",file.c_str());
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
    if ( lev->index >= 0 ) {
      ZipFile zf( lev->file.c_str() );
      if ( lev->index < zf.numEntries() ) {
	
	unsigned char* d = zf.extract( lev->index, &l);
	if ( d && l <= bufLen ) {
	  memcpy( buf, d, l );
	}
      }
    } else {
      FILE *f = fopen( lev->file.c_str(), "rt" );
      if ( f ) {
	l = fread( buf, 1, bufLen, f );
	fclose(f);
      }
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
    if ( lev->index >= 0 ) {
      ZipFile zf( lev->file.c_str() );
      s = zf.entryName( lev->index );
    } else {
      s = lev->file;
    }
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
        for (int j=0; j<m_collections[i]->levels.size(); j++) {
            LevelDesc *level = m_collections[i]->levels[j];
            printf(" Level #%d: %s (index=%d, rank%d)\n", (j+1),
                    level->file.c_str(), level->index, level->rank);
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
      if ( i >= m_collections[c]->levels.size() ) {
	i -= m_collections[c]->levels.size();
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
      return nameFromPath(m_collections[i]->name);
    } else {
      return m_collections[i]->name;
    }
  }
  return "Bad Collection ID";
}


int Levels::collectionSize(int c)
{
  if (c>=0 && c<numCollections()) {
    return m_collections[c]->levels.size();
  }
  return 0;
}

int Levels::collectionLevel(int c, int i)
{
  if (c>=0 && c<numCollections()) {
    if (i>=0 && i<m_collections[c]->levels.size()) {
      int l = i;
      for (int j=0; j<c; j++) {
	l += m_collections[j]->levels.size();
      }
      return l;
    }
  }
  return 0;
}


std::string Levels::demoPath(int l)
{
  std::string name = levelName(l,false);
  if (name.substr(name.length()-4) == ".npd") {
    /* Kludge: If the level from which we want to save a demo is
     * already a demo file, return an empty string to signal
     * "don't have this demo" - see Game.cpp */
    return "";
  }

  int c = collectionFromLevel(l);
  std::string path = Config::userDataDir() + Os::pathSep
    + "Recordings" + Os::pathSep
    + collectionName(c,false);
  if (path.substr(path.length()-4) == ".npz") {
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
  if (name.substr(name.length()-4) == ".nph") {
    name.resize(name.length()-4);
  }
  return demoPath(l) + Os::pathSep + name + ".npd";
}

bool Levels::hasDemo(int l)
{
  return OS->exists(demoName(l));
}


Levels::LevelDesc* Levels::findLevel( int i )
{
  if (i < m_numLevels) {
    for ( int c=0; c<m_collections.size(); c++ ) {
      if ( i >= m_collections[c]->levels.size() ) {
	//fprintf(stderr,"index %d not in c%d (size=%d)\n",i,c,m_collections[c]->levels.size());
	i -= m_collections[c]->levels.size();
      } else {
	return m_collections[c]->levels[i];
      }
    }
  }
  return NULL;
}


int Levels::findLevel( const char *file )
{
  int index = 0;
  for ( int c=0; c<m_collections.size(); c++ ) {
    for ( int i=0; i<m_collections[c]->levels.size(); i++ ) {
      if ( m_collections[c]->levels[i]->file == file ) {
	return index + i;
      }
    }
    index += m_collections[c]->levels.size();
  }
  return -1;
}


