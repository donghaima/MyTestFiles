/=============================================================
// AsyncFileMonitor.h
//
// Copyright (c) 2012 Cisco Systems, Inc.
//
//=============================================================

#pragma once

#include <boost/thread/thread.hpp>

using namespace std;


class AsyncFileMonitor
{
public:
  

private:
  string m_filePath;
  string m_fileName;

  void ThreadWork();
  boost::thread  m_thread;  
};
