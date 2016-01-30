/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2008, 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
 * Coyright (c) 2012, 2014, 2015 Thomas Perl <m@thp.io>
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
 */

#include <cstring>
#include <sys/types.h>
#include <dirent.h>

#include "Levels.h"
#include "Config.h"
#include "Os.h"
#include "Regex.h"

#include "petals_log.h"

#include <iostream>
#include <string>
#include <algorithm>


static const char MISC_COLLECTION[] = "C99_My Levels";
static const char DEMO_COLLECTION[] = "D00_My Solutions";


static std::string nameFromPath(const std::string& path)
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
  if (path[i] == 'D') i++;
  if (path[i] == 'M') i++;
  while (path[i] >= '0' && path[i] <= '9') i++;
  if (path[i] == '_') i++;
  size_t e = path.rfind('.');
  if (e != 0) {
      name = path.substr(i,e-i);
  } else {
      name = path.substr(1);
  }
  for (i=0; i<name.size(); i++) {
    if (name[i]=='-' || name[i]=='_' || name[i]=='.') {
      name[i] = ' ';
    }
  }
  return name;
}

static int
compare_names(const std::string &a, const std::string &b)
{
    const auto RE = "([CDM])(\\d+)_(.*)";
    const auto GROUPS = 3;
    auto ma = NP::Regex::match_groups(RE, a, GROUPS);
    auto mb = NP::Regex::match_groups(RE, b, GROUPS);

    if (!ma.size() && !mb.size()) {
        // Simple string comparison
        return a.compare(b);
    } else if (!ma.size()) {
        return -1;
    } else if (!mb.size()) {
        return 1;
    }

    // Compare class character ([C]ollection, [D]emo, [M]enu)
    char ca = ma[0][0];
    char cb = mb[0][0];
    if (ca != cb) {
        return ca - cb;
    }

    // Compare order number (00, 01, ...)
    int oa = atoi(ma[1].c_str());
    int ob = atoi(mb[1].c_str());
    if (oa != ob) {
        return oa - ob;
    }

    // Compare rest-of-string (C01_foo)
    //                             ^^^
    return ma[2].compare(mb[2]);
}

bool
operator<(LevelDesc a, LevelDesc b)
{
    return compare_names(a.file, b.file) < 0;
}

void
LevelDesc::swap(LevelDesc &a, LevelDesc &b)
{
    std::swap(a.file, b.file);
}

bool
operator<(Collection a, Collection b)
{
    return compare_names(a.name, b.name) < 0;
}

void
Collection::swap(Collection &a, Collection &b)
{
    std::swap(a.file, b.file);
    std::swap(a.name, b.name);
    std::swap(a.levels, b.levels);
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
    bool result = false;
    std::string ext = fileExtension(path);

    if (ext == ".nph" || ext == ".npd" || ext == ".npsvg" || ext == ".npdsvg") {
        addLevel(path);
        result = true;
    } else {
        result = scanCollection(path);
    }

    sort();
    return result;
}

bool Levels::addLevel(const std::string& file)
{
    auto ext = fileExtension(file);
    if (ext == ".npd" || ext == ".npdsvg") {
        return addLevel(getCollection(DEMO_COLLECTION), file);
    } else {
        return addLevel(getCollection(MISC_COLLECTION), file);
    }
}

bool Levels::addLevel(Collection &collection, const std::string &file)
{
    collection.levels.push_back(LevelDesc(file));
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

    m_collections.push_back(Collection(file, file));
    return m_collections.back();
}


bool Levels::scanCollection(const std::string &file)
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
            if (ext == ".nph" || ext == ".npsvg") {
                if (addLevel(getCollection(collectionName), filename)) {
                    result = true;
                }
            } else if (ext == ".npd" || ext == ".npdsvg") {
                if (addLevel(getCollection(DEMO_COLLECTION), filename)) {
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


std::string Levels::load(int i)
{
  LevelDesc *lev = findLevel(i);

  if (lev) {
      return Config::readFile(lev->file);
  }

  return "";
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
Levels::sort()
{
    std::sort(m_collections.begin(), m_collections.end());
    for (auto &collection: m_collections) {
        std::sort(collection.levels.begin(), collection.levels.end());
    }
}

void
Levels::dump()
{
    for (int i=0; i<m_collections.size(); i++) {
        LOG_INFO("Collection #%d: %s", (i+1),
                    collectionName(i, true).c_str());
        for (int j=0; j<m_collections[i].levels.size(); j++) {
            LevelDesc &level = m_collections[i].levels[j];
            LOG_INFO(" Level #%d: %s\n", (j+1),
                        level.file.c_str());
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
  if (ext == ".npd" || ext == ".npdsvg") {
    /* Kludge: If the level from which we want to save a demo is
     * already a demo file, return an empty string to signal
     * "don't have this demo" - see Game.cpp */
    return "";
  }

  int c = collectionFromLevel(l);
  return Config::userRecordingCollectionDir(collectionName(c, false));
}

std::string Levels::demoName(int l)
{
  std::string name = levelName(l,false);

  name = Config::baseName(name);

  if (fileExtension(name) == ".nph") {
      name.resize(name.length()-4);
  }
  if (fileExtension(name) == ".npsvg") {
      name.resize(name.length()-6);
  }

  return Config::joinPath(demoPath(l), name + ".npdsvg");
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


