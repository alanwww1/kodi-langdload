/*
 *      Copyright (C) 2012 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined( WIN32 ) && defined( TUNE )
  #include <crtdbg.h>
  _CrtMemState startMemState;
  _CrtMemState endMemState;
#endif

#include <string>
#include <list>
#include <stdio.h>
#include "lib/HTTPUtils.h"
#include "lib/Log.h"
#include "lib/UpdateXMLHandler.h"
#include "lib/ResourceHandler.h"
#include "lib/FileUtils.h"

class CInputData
{
public:
  CInputData();
  ~CInputData();
  std::string strAddonName;
  std::string strAddonDir;
  bool bDloadChangelog;
  bool bDloadEnglishFile;
};
CInputData::CInputData()
{
  bDloadChangelog = true;
  bDloadEnglishFile =true;
}

CInputData::~CInputData()
{}


using namespace std;

void PrintUsage()
{
  printf
  (
  "Usage: xbmc-langdload ADDONID LOCALDIR\n\n"
  "ADDONID: The id of the addon which is defined in the \"id\" tag in the addon.xml file\n"
  "LOCALDIR: The local directory\n"

  "            This will be the directory where your merged and transifex update files get generated.\n\n"
  "Working modes:\n"
  "     -d   Only download to local cache, without performing a merge.\n"
  "     -dm  Download and create merged and tx update files, but no upload performed.\n"
  "     -dmu Download, merge and upload the new files to transifex.\n"
  "     -u   Only upload the previously prepared files. Note that this needs downlad and merge ran before.\n\n"
  "     No working mode arguments used, performs as -dm\n\n"
  );
  #ifdef _MSC_VER
  printf
  (
  "Note for Windows users: In case you have whitespace or any special character\n"
  "in the directory argument, please use apostrophe around them. For example:\n"
  "xbmc-txupdate.exe \"C:\\xbmc dir\\language\"\n"
  );
  #endif
  return;
};

int main(int argc, char* argv[])
{
  setbuf(stdout, NULL);
  if (argc > 3 || argc == 0)
  {
    printf ("\nArgument counr error\n\n");
    PrintUsage();
    return 1;
  }

  std::list<CInputData> listInputData;
  CInputData InputData;

  if (argc == 3)
  {
    if (argv[1])
      InputData.strAddonName = argv[1];
    if (argv[2])
      InputData.strAddonDir = argv[2];
    if (InputData.strAddonDir.empty())
    {
      printf ("\nMissing or empty addon directory, stopping.\n\n");
      PrintUsage();
      return 1;
    }
    if (InputData.strAddonName.empty())
    {
      printf ("\nMissing or empty addonname, stopping.\n\n");
      PrintUsage();
      return 1;
    }
    listInputData.push_back(InputData);
  }

  try
  {
    CLog::Log(logINFO, "XBMC-LANGDLOAD v%s", VERSION.c_str());
    CLog::Log(logLINEFEED, "");

    std::map<std::string, CXMLResdata> mapResourceData;
    CUpdateXMLHandler UpdateXMLHandler;
    CResourceHandler ResourceHandler;

    UpdateXMLHandler.DownloadXMLToMap("https://raw.github.com/xbmc/translations/master/translations/xbmc-main-frodo/", mapResourceData);

    for (std::list<CInputData>::iterator it = listInputData.begin(); it != listInputData.end(); it++)
    {
      if (mapResourceData.find(it->strAddonName) != mapResourceData.end())
      {
        CXMLResdata XMLResdata = mapResourceData[it->strAddonName];
        XMLResdata.strResLocalDirectory = it->strAddonDir;
        ResourceHandler.DloadLangFiles(XMLResdata);
      }
    }

    std::string strLogMessage = "PROCESS FINISHED WITH " + g_File.IntToStr(CLog::GetWarnCount()) + " WARNINGS";
    std::string strLogHeader;
    strLogHeader.resize(strLogMessage.size(), '*');
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "%s", strLogHeader.c_str());
    CLog::Log(logINFO, "%s", strLogMessage.c_str());
    CLog::Log(logINFO, "%s", strLogHeader.c_str());

    g_HTTPHandler.Cleanup();
  }
  catch (const int calcError)
  {
    g_HTTPHandler.Cleanup();
    return 0;
  }
}
