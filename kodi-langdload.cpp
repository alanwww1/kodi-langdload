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
  #include <crtdbg.h>
  _CrtMemState startMemState;
  _CrtMemState endMemState;
#endif

#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include "lib/HTTPUtils.h"
#include "lib/Log.h"
#include "lib/XMLHandler.h"
#include "lib/ResourceHandler.h"
#include "lib/FileUtils.h"
#include "lib/JSONHandler.h"
#include "lib/LCode.h"

using namespace std;

void PrintUsage()
{
  printf
  (
  "1.Simple mode\n"
  "  Usage: kodi-langdload PROJECTID/ADDONID LOCALDIR\n\n"
  "  PROJECTID: The id of the project defined on the kodi repo. eg. kodi-main\n"
  "  ADDONID: The id of the addon which is defined in the \"id\" tag in the addon.xml file\n"
  "  LOCALDIR: The local directory to copy the files to. This is where the addon.xml file gets.\n\n"
  "  Example: kodi-langdload kodi-addons/plugin.video.coolplugin /home/myname/somedir/\n\n"
  "2.Batch mode with xml file usage:\n"
  "  Usage: kodi-langdload XMLFILE\n\n"
  "  XMLFILE: The path and filename of the input XML file which holds the download data (check README for format)\n\n"
  "  Example: kodi-langdload kodi-langdload.xml\n\n"
  "3.List addons mode:\n"
  "  Usage: kodi-langdload list addons\n\n"
  "  In this mode you can fetch a current list of the available hosted addons on kodi translations github repo.\n"
  "  This list also shows what language fileformat is used (XML or PO) and if the addon has a changelog.txt hosted.\n\n"
  );
  #ifdef _MSC_VER
  printf
  (
  "Note for Windows users: In case you have whitespace or any special character\n"
  "in the directory/file argument, please use apostrophe around them. For example:\n"
  "kodi-langdload.exe kodi-skins/skin.essence \"C:\\some dir\\\"\n"
  "Also make sure you have write access to the local directory.\n"
  "Please run the command prompt in admin mode\n\n"
  );
  #endif
  return;
};

int main(int argc, char* argv[])
{
  setbuf(stdout, NULL);
  if (argc > 4 || argc == 1)
  {
    printf ("\nUsage:\n\n");
    PrintUsage();
    return 1;
  }

  std::list<CInputData> listInputData;
  CInputData InputData;
  std::string strInputXMLPath;
  bool bListAddonsMode = false;

  if (argc == 3 || argc ==4 )
  {
    std::string strArg1, strArg2, strArg3;
    if (argv[1])
      strArg1 = argv[1];
    if (argv[2])
      strArg2 = argv[2];
    if (argc ==4 && argv[3])
      strArg3 = argv[3];


    if (strArg1 == "list" && strArg2 == "addons")
      bListAddonsMode = true;
    else
    {
      InputData.strAddonName = strArg1;
      InputData.strAddonDir = strArg2;

      if (InputData.strAddonDir.empty())
      {
        printf ("\nMissing or empty addon directory, stopping.\n\n");
        PrintUsage();
        return 1;
      }
      if (InputData.strAddonName.empty())
      {
        printf ("\nMissing or empty addon name, stopping.\n\n");
        PrintUsage();
        return 1;
      }

      if (!strArg3.empty() && strArg3.at(0) == '-')
      {
        if (strArg3.find("c") != std::string::npos)
          InputData.bSkipChangelog = true;
        if (strArg3.find("e") != std::string::npos)
          InputData.bSkipSRCLangfile = true;
      }
      listInputData.push_back(InputData);
    }
  }
  else if (argc == 2)
  {
    if (argv[1])
      strInputXMLPath = argv[1];
  }

  try
  {
    CLog::Log(logINFO, "KODI-LANGDLOAD v%s", VERSION.c_str());
    CLog::Log(logLINEFEED, "");

    if (!bListAddonsMode && listInputData.empty())
    {
      if (strInputXMLPath.empty())
        CLog::Log(logERROR, "Insufficient input data, cannot continue.");
      CInputXMLHandler InputXMLHander;
      listInputData = InputXMLHander.ReadXMLToMem(strInputXMLPath);
    }

    CResourceHandler ResourceHandler;
    CUpdateXMLHandler XMLHandler;

    std::list<std::string> listTXProjects;
    std::string strGithubURL, strGithubAPIURL;
    strGithubURL = "https://raw.githubusercontent.com/xbmc/translations/master/kodi-translations/";
    strGithubAPIURL = g_HTTPHandler.GetGitHUBAPIURL(strGithubURL);
    printf("\nprojectlist");

    std::string strtemp = g_HTTPHandler.GetURLToSTR(strGithubAPIURL);

    if (strtemp.empty())
      CLog::Log(logERROR, "Error getting TX project list from kodi translation github repo");

    listTXProjects = g_Json.ParseAvailDirsGITHUB(strtemp);
    std::string strListProjects;

    for (std::list<std::string>::iterator it = listTXProjects.begin(); it != listTXProjects.end(); it++)
    {
      // We get the version of the kodi-txupdate.xml files here
      std::string strGitHubURL = g_HTTPHandler.GetGitHUBAPIURL("https://raw.githubusercontent.com/xbmc/translations/master/kodi-translations/" + *it + "/");
      printf("\n%s, kodi-txupdate.xml", it->c_str());
      std::string strtemp = g_HTTPHandler.GetURLToSTR(strGitHubURL);
      if (strtemp.empty())
        CLog::Log(logERROR, "Error getting kodi-txupdate.xml file version for project: %s", it->c_str());

      std::string strCachename = *it + "/" + "kodi-txupdate";
      g_Json.ParseFileVersion(strtemp, "https://raw.githubusercontent.com/xbmc/translations/master/kodi-translations/" + *it + "/kodi-txupdate.xml", strCachename);

      XMLHandler.DownloadXMLToMap(*it, "https://raw.github.com/xbmc/translations/master/kodi-translations/" + *it + "/");
    }
    CLog::Log(logINFO, "Detected a total %i resources hosted in %i projects at kodi/translations Github repo", XMLHandler.m_mapXMLResdata.size(), listTXProjects.size());

    for (std::list<CInputData>::iterator it = listInputData.begin(); it != listInputData.end(); it++)
    {
      if (XMLHandler.m_mapXMLResdata.find(it->strAddonName) != XMLHandler.m_mapXMLResdata.end())
      {
        CXMLResdata XMLResdata = XMLHandler.m_mapXMLResdata[it->strAddonName];
        XMLResdata.strResLocalDirectory = it->strAddonDir;
        XMLResdata.strResLocalDirectoryForSRC = it->strAddonDirForSource;
        XMLResdata.bSkipChangelog = it->bSkipChangelog;
        XMLResdata.bSkipLangfiles = it->bSkipLangfiles;
	XMLResdata.bSkipSRCLangfile = it->bSkipSRCLangfile;
        XMLResdata.strGittemplate = it->strGittemplate;
        XMLResdata.strGitExecPath = it->strGitExecPath;
        XMLResdata.strGittemplateSRC = it->strGittemplateSRC;
        XMLResdata.strGitExecPathSRC = it->strGitExecPathSRC;
        XMLResdata.bClearLangdir = it->bClearLangdir;

        ResourceHandler.DloadLangFiles(XMLResdata);

        if (!XMLResdata.strGittemplate.empty())
        {
          size_t pos;
          std::string strFormat = XMLResdata.strGittemplate;
          if ((pos = strFormat.find("%v")) != std::string::npos)
            strFormat.replace(pos, 2, XMLResdata.strAddonVersion.c_str());
          if ((pos = strFormat.find("%n")) != std::string::npos)
            strFormat.replace(pos, 2, XMLResdata.strName.c_str());

          std::string strCommand;
          std::string strCDDirectory = XMLResdata.strResLocalDirectory;

#ifdef _MSC_VER
          strCommand += "cd " + strCDDirectory + " & ";
          strCommand += "\"" + XMLResdata.strGitExecPath + "sh.exe\" --login -i -c \"git add -A `git rev-parse --show-toplevel`\" & ";
          strCommand += "\"" + XMLResdata.strGitExecPath + "sh.exe\" --login -i -c \"git commit -m '" + strFormat + "'\"";
#else
          strCommand += "cd " + strCDDirectory + ";";
          strCommand += "git add -A `git rev-parse --show-toplevel`;";
          strCommand += "git commit -m \"" + strFormat + "\"";
#endif
          CLog::Log(logINFO, "GIT commit with the following command: %s", strCommand.c_str());
          system (strCommand.c_str());
        }
        else if (!XMLResdata.strGittemplateSRC.empty())
        {
          size_t pos;
          std::string strFormat = XMLResdata.strGittemplateSRC;
          if ((pos = strFormat.find("%n")) != std::string::npos)
            strFormat.replace(pos, 2, XMLResdata.strName.c_str());

          std::string strCommand;
          std::string strCDDirectory = XMLResdata.strResLocalDirectoryForSRC;

#ifdef _MSC_VER
          strCommand += "cd " + strCDDirectory + " & ";
          strCommand += "\"" + XMLResdata.strGitExecPathSRC + "sh.exe\" --login -i -c \"git add -A `git rev-parse --show-toplevel`\" & ";
          strCommand += "\"" + XMLResdata.strGitExecPathSRC + "sh.exe\" --login -i -c \"git commit -m '" + strFormat + "'\"";
#else
          strCommand += "cd " + strCDDirectory + ";";
          strCommand += "git add -A `git rev-parse --show-toplevel`;";
          strCommand += "git commit -m \"" + strFormat + "\"";
#endif
          CLog::Log(logINFO, "GIT commit with the following command: %s", strCommand.c_str());
          system (strCommand.c_str());
        }
      }
      else
        CLog::Log(logWARNING, "Addon name not found on kodi github repository: %s", it->strAddonName.c_str());
    }

    if (bListAddonsMode)
    {
      printf("\n"); 
      for (std::map<std::string, CXMLResdata>::iterator it = XMLHandler.m_mapXMLResdata.begin(); it != XMLHandler.m_mapXMLResdata.end(); it++)
      {
        printf ("                                %s (%s)\n", it->first.c_str(), !it->second.strChangelogFormat.empty()? " changelog.txt":"");
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
    return calcError;
  }
}
