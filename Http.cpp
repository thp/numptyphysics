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

#include <cstdlib>
#include <stdio.h>
#include <string.h>

#include "Http.h"
//#include "happyhttp.h"
//using namespace happyhttp;


#if 0

static void http_begin_cb( const Response* r, void* userdata )
{
  switch ( r->getstatus() ) {
  case OK:
    ((Http*)userdata)->m_size = 0;
    break;
  default:
    //fprintf(stderr,"http status=%d %s\n",r->getstatus(),r->getreason());
    ((Http*)userdata)->m_err = r->getreason();
    ((Http*)userdata)->m_size = -1;
    break;
  }
}

static void http_get_cb( const Response* r, void* userdata,
		   const unsigned char* data, int numbytes )
{
  fwrite( data, 1, numbytes, ((Http*)userdata)->m_file );
  ((Http*)userdata)->m_size += numbytes;
}

static void http_post_cb( const Response* r, void* userdata,
			  const unsigned char* data, int numbytes )
{
  //printf("received %d bytes [%s]\n",numbytes,data);
}

static void http_complete_cb( const Response* r, void* userdata )
{
  if ( r->getreason() ) {
    ((Http*)userdata)->m_err = r->getreason();
  }
  if ( r->getheader("NP-Upload-Id") ) {
    ((Http*)userdata)->m_npid = r->getheader("NP-Upload-Id");
  }
}


static bool parseUri( const char * uri,
		      char* outHost,
		      int * outPort,
		      char* outPath )
{
  if ( strncmp(uri,"http://",strlen("http://"))==0 ) {
    uri += strlen("http://");
  }
  strcpy(outHost,uri);
  char* e = strchr(outHost,'/');
  *outPort = 80;

  if ( e ) {
    *e = '\0';
  }
  e = strchr(outHost,':');
  if ( e ) {
    *e = '\0';
    *outPort=atoi(e+1);
  }
  strcpy( outPath, strchr(uri,'/') );
  //fprintf(stderr,"Http::get host=%s port=%d file=%s\n",
  //        outHost,*outPort,outPath);
  return true;
}
#endif

bool Http::get( const char* uri,
		const char* file )
{
    return false;
#if 0
  char host[256];
  char path[256];
  int port;
  
  m_file = fopen( file, "wt" );
  m_size = -1;

  if ( parseUri( uri, &host[0], &port, &path[0] )
       && path[0] && host[0] ) {
    try {
      Connection con( host, port );
      con.setcallbacks( http_begin_cb, http_get_cb, http_complete_cb, this );
      con.request("GET",path,NULL,NULL,0);
      while ( con.outstanding() ) {
	//fprintf(stderr,"http_get pump\n");
	con.pump();
      }
    } catch ( Wobbly w ) {
      fprintf(stderr,"http_get wobbly: %s\n",w.what());
    }
  }

  fclose ( m_file );
  return m_size > 0;
#endif
}


bool Http::post( const char* uri, const char*putname, const char* putfile,
		 const char* otherargs )
{
    return false;
#if 0
  char host[256];
  char path[256];
  char data[64*1024];
  int port;
  static char hex[16+1] = "0123456789ABCDEF";

  if ( otherargs ) {
    sprintf(data,"%s&%s=",otherargs,putname);
  } else {
    sprintf(data,"%s=",putname);
  }
  char *buf = &data[strlen(data)];
  
  m_file = fopen( putfile, "rt" );
  while ( !feof(m_file) ) {
    unsigned char c = fgetc( m_file );
    switch ( c ) {
    case 'a'...'z':
    case 'A'...'Z':
    case '0'...'9':
      *buf++ = c;
      break;
    default:
      *buf++ = '%';
      *buf++ = hex[c>>4];
      *buf++ = hex[c&0xf];
      break;
    }
    //m_size = fread(data,1,sizeof(data),m_file);
  }
  fclose ( m_file );
  m_size = buf - &data[0];

  const char* headers[] = {
    "Connection", "close",
    "Content-type", "application/x-www-form-urlencoded",
    "Accept", "text/plain",
    0
  };
  
  if ( parseUri( uri, &host[0], &port, &path[0] ) ) {
    try {
      Connection con( host, port );
      con.setcallbacks( http_begin_cb, http_post_cb, http_complete_cb, this );
      con.request("POST",path,headers,(unsigned char*)data,m_size);
      while ( con.outstanding() ) {
	//fprintf(stderr,"http::post pump\n");
	con.pump();
      }
    } catch ( Wobbly w ) {
      fprintf(stderr,"http_get wobbly: %s\n",w.what());
    }
  }
#endif
}


// response
std::string Http::errorMessage()
{
    return "";
//  return m_err;
}

std::string Http::getHeader( const char* name )
{
    return "";
//  return m_npid;
}

