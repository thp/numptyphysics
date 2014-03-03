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

#include <stdio.h>
#include <string.h>
#include <string>

class Http 
{
public:
  // start
  bool get( const char* uri, const char* file );
  bool post( const char* uri, const char*putname, const char* putfile,
	     const char* otherargs=NULL );

  // response
  std::string errorMessage();
  std::string getHeader( const char* name );

  // internals...
  FILE *m_file;
  int   m_size;
  std::string m_err;
  std::string m_npid;

};


