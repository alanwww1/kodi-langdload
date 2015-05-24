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
#include "lib/CharsetUtils.h"

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
  bool bDoGitPull = true;
  bool bDownloadLangFiles = false;

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

    printf("%s", KGRN);
    std::string strLogMessage = "Downloading project list and project files from https://github.com/xbmc/translations/tree/master/kodi-translations";
    std::string strLogHeader;
    strLogHeader.resize(strLogMessage.size(), '*');
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "%s", strLogHeader.c_str());
    CLog::Log(logINFO, "%s", strLogMessage.c_str());
    CLog::Log(logINFO, "%s", strLogHeader.c_str());
    CLog::IncIdent(2);
    printf("%s", RESET);

    CResourceHandler ResourceHandler;
    CUpdateXMLHandler XMLHandler;

    std::list<std::string> listTXProjects;
    std::string strGithubURL, strGithubAPIURL;
    strGithubURL = "https://raw.githubusercontent.com/xbmc/translations/master/kodi-translations/";
    strGithubAPIURL = g_HTTPHandler.GetGitHUBAPIURL(strGithubURL);

    CLog::Log(logINFONLF, "Project list");

    std::string strtemp = g_HTTPHandler.GetURLToSTR(strGithubAPIURL);
    CLog::Log(logLINEFEED, "");


    if (strtemp.empty())
      CLog::Log(logERROR, "Error getting TX project list from kodi translation github repo");

    listTXProjects = g_Json.ParseAvailDirsGITHUB(strtemp);
    std::string strListProjects;

    for (std::list<std::string>::iterator it = listTXProjects.begin(); it != listTXProjects.end(); it++)
    {
      // We get the version of the kodi-txupdate.xml files here
      std::string strGitHubURL = g_HTTPHandler.GetGitHUBAPIURL("https://raw.githubusercontent.com/xbmc/translations/master/kodi-translations/" + *it + "/");
      CLog::Log(logINFONLF, "Project file version for project %s%s%s", KMAG, it->c_str(), RESET);

      std::string strtemp = g_HTTPHandler.GetURLToSTR(strGitHubURL);
      CLog::Log(logLINEFEED, "");

      if (strtemp.empty())
        CLog::Log(logERROR, "Error getting kodi-txupdate.xml file version for project: %s%s%s", KMAG, it->c_str(), RESET);

      std::string strCachename = *it + "/" + "kodi-txupdate";
      g_Json.ParseFileVersion(strtemp, "https://raw.githubusercontent.com/xbmc/translations/master/kodi-translations/" + *it + "/kodi-txupdate.xml", strCachename);
      CLog::Log(logINFONLF, "Project file for project %s%s%s", KMAG, it->c_str(), RESET);

      XMLHandler.DownloadXMLToMap(*it, "https://raw.github.com/xbmc/translations/master/kodi-translations/" + *it + "/");
      CLog::Log(logLINEFEED, "");
    }
    CLog::Log(logINFO, "Detected a total %s%i%s resources hosted in %s%i%s projects at kodi/translations Github repo", KCYN, XMLHandler.m_mapXMLResdata.size(), RESET, KCYN, listTXProjects.size(), RESET);

    CLog::DecIdent(2);


    //do git pull
    if (bDoGitPull)
    {
      printf("%s", KGRN);
      strLogMessage = "DOING GIT PULL OPERATIONS FOR UPSTREAM GITHUB REPOS";
      strLogHeader.clear();
      strLogHeader.resize(strLogMessage.size(), '*');
      CLog::Log(logLINEFEED, "");
      CLog::Log(logINFO, "%s", strLogHeader.c_str());
      CLog::Log(logINFO, "%s", strLogMessage.c_str());
      CLog::Log(logINFO, "%s", strLogHeader.c_str());
      printf("%s", RESET);

      std::map<std::string, CGithubData> mapGithubRepos;
      std::map<std::string, CGithubData> mapGithubReposCommited;

      std::list<CXMLResdata> listXMLdata;

      for (std::list<CInputData>::iterator it = listInputData.begin(); it != listInputData.end(); it++)
      {
        if (XMLHandler.m_mapXMLResdata.find(it->strAddonName) != XMLHandler.m_mapXMLResdata.end())
        {
          CXMLResdata XMLResdata = XMLHandler.GetXMLResdata(*it);
          std::string strGitCloneURL, strGitCloneURLPlusBranch;
          CGithubURLData GithubURLData;

          if (!XMLResdata.bIsLanguageAddon || !XMLResdata.bSkipLangfiles)
          {
            if (!XMLResdata.bHasOnlyAddonXML)
              g_HTTPHandler.GetGitCloneURL(XMLResdata.strUPSLangURLRoot, strGitCloneURL, GithubURLData);
            else
              g_HTTPHandler.GetGitCloneURL(XMLResdata.strUPSAddonURLRoot, strGitCloneURL, GithubURLData);

            strGitCloneURLPlusBranch = strGitCloneURL + " branch:" + GithubURLData.strGitBranch;

            if (mapGithubRepos.find(strGitCloneURLPlusBranch) != mapGithubRepos.end())
            {
              mapGithubRepos[strGitCloneURLPlusBranch].listXMLResdata.push_back(XMLResdata);
            }
            else
            {
              CGithubData GithubData;
              GithubData.listXMLResdata.push_back(XMLResdata);
              GithubData.strGithubURL = strGitCloneURL;
              GithubData.GithubURLData.strGitBranch = GithubURLData.strGitBranch;
              mapGithubRepos[strGitCloneURLPlusBranch] = GithubData;
            }
            if (!XMLResdata.strGittemplate.empty())
            {
              mapGithubRepos[strGitCloneURLPlusBranch].strLocalGithubPath = XMLResdata.strGitToplevelPath;
              mapGithubRepos[strGitCloneURLPlusBranch].strGitCloneName = XMLResdata.strGitCloneName;
              mapGithubReposCommited[strGitCloneURLPlusBranch] = mapGithubRepos[strGitCloneURLPlusBranch];
            }
          }

          // Source language file handling for language-addons
          if (!XMLResdata.strUPSSourceLangURL.empty() && !XMLResdata.bSkipSRCLangfile)
          {
            g_HTTPHandler.GetGitCloneURL(XMLResdata.strUPSSourceLangURL, strGitCloneURL, GithubURLData);

            strGitCloneURLPlusBranch = strGitCloneURL + " branch:" + GithubURLData.strGitBranch;

            if (mapGithubRepos.find(strGitCloneURLPlusBranch) != mapGithubRepos.end())
            {
              mapGithubRepos[strGitCloneURLPlusBranch].listXMLResdata.push_back(XMLResdata);
            }
            else
            {
              CGithubData GithubData;
              GithubData.listXMLResdata.push_back(XMLResdata);
              GithubData.strGithubURL = strGitCloneURL;
              GithubData.GithubURLData.strGitBranch = GithubURLData.strGitBranch;
              mapGithubRepos[strGitCloneURLPlusBranch] = GithubData;
            }
            if (!XMLResdata.strGittemplateSRC.empty())
            {
              mapGithubRepos[strGitCloneURLPlusBranch].strLocalGithubPath = XMLResdata.strGitToplevelPathSRC;
              mapGithubRepos[strGitCloneURLPlusBranch].strGitCloneName = XMLResdata.strGitCloneNameSRC;
              mapGithubReposCommited[strGitCloneURLPlusBranch] = mapGithubRepos[strGitCloneURLPlusBranch];
            }
          }
        }
        else
          CLog::Log(logWARNING, "Addon name not found on kodi github repository: %s", it->strAddonName.c_str());
      }

      for (std::map<std::string, CGithubData>::iterator itmap = mapGithubRepos.begin(); itmap != mapGithubRepos.end(); itmap++)
      {
        CGithubData GitData = itmap->second;
        CLog::Log(logINFONLF, "%s branch: %s%s%s for addon(s): ", GitData.strGithubURL.c_str(),KCYN, GitData.GithubURLData.strGitBranch.c_str(), RESET);

        std::map<std::string, std::string> listTouchedProjects;
        for (std::list<CXMLResdata>::iterator itresdata = GitData.listXMLResdata.begin(); itresdata != GitData.listXMLResdata.end();itresdata++)
        {
          // create a list of corresponding projects for later use (kodi-txupdate)
          if (listTouchedProjects.find (itresdata->strProjName) == listTouchedProjects.end())
            listTouchedProjects[itresdata->strProjName] = itresdata->strProjName;

          CLog::Log(logINFONLF, "%s%s%s ", KMAG, itresdata->strName.c_str(), RESET);
        }

        if (mapGithubReposCommited.find(itmap->first) == mapGithubReposCommited.end())
          CLog::Log(logWARNING, "%s(not to be commited)%s", KRED, RESET);
        else
        {
          std::string strCommand;
          CLog::Log(logLINEFEED, "");

          if (!g_File.FileExist(GitData.strLocalGithubPath + GitData.strGitCloneName + "/.git/config"))
          //no local directory present, cloning one
          {
            CLog::Log(logLINEFEED, "");
            // clean directory if exists, unless git clone fails
            g_File.DelDirectory(GitData.strLocalGithubPath);
            g_File.MakeDir(GitData.strLocalGithubPath);

            strCommand = "cd " + GitData.strLocalGithubPath + ";";
            strCommand += "git clone " + GitData.strGithubURL + " " + GitData.strGitCloneName;
            CLog::Log(logINFO, "%sGIT cloning with the following command:%s\n%s%s%s",KMAG, RESET, KYEL, strCommand.c_str(), RESET);
            g_File.SytemCommand(strCommand);

            strCommand = "cd " + GitData.strLocalGithubPath + GitData.strGitCloneName + ";";
            strCommand += "git checkout " + GitData.GithubURLData.strGitBranch;
            CLog::Log(logINFO, "%sGIT checkout branch: %s%s%s%s\n%s%s%s",KMAG, RESET, KCYN,GitData.GithubURLData.strGitBranch.c_str(), RESET, KYEL, strCommand.c_str(), RESET);
            g_File.SytemCommand(strCommand);
          }
          else
          //local git clone is present, we clean it and update it
          {
            CLog::Log(logLINEFEED, "");

            strCommand = "cd " + GitData.strLocalGithubPath + GitData.strGitCloneName + ";";
            strCommand += "git reset --hard origin/" + GitData.GithubURLData.strGitBranch;
            CLog::Log(logINFO, "%sGIT reset existing local repo to branch: %s%s%s%s\n%s%s%s",KMAG, RESET, KCYN,GitData.GithubURLData.strGitBranch.c_str(), RESET, KYEL, strCommand.c_str(), RESET);
            g_File.SytemCommand(strCommand);

            strCommand = "cd " + GitData.strLocalGithubPath + GitData.strGitCloneName + ";";
            strCommand += "git clean -f -d -x";
            CLog::Log(logINFO, "%sRemove untracked files%s\n%s%s%s", KMAG, RESET, KYEL, strCommand.c_str(), RESET);
            g_File.SytemCommand(strCommand);

            strCommand = "cd " + GitData.strLocalGithubPath + GitData.strGitCloneName + ";";
            strCommand += "git pull";
            CLog::Log(logINFO, "%sPull latest git changes%s\n%s%s%s", KMAG, RESET, KYEL, strCommand.c_str(), RESET);
            g_File.SytemCommand(strCommand);
          }
        }

        CLog::Log(logLINEFEED, "");
      }
    }


    // Language file downloading
    if (bDownloadLangFiles)
    {
      printf("%s", KGRN);
      strLogMessage = "DOWNLOADING LANGUAGE DATA FROM KODI TRANSLATION GITHUB REPO";
      strLogHeader.clear();
      strLogHeader.resize(strLogMessage.size(), '*');
      CLog::Log(logLINEFEED, "");
      CLog::Log(logINFO, "%s", strLogHeader.c_str());
      CLog::Log(logINFO, "%s", strLogMessage.c_str());
      CLog::Log(logINFO, "%s", strLogHeader.c_str());
      printf("%s", RESET);

      for (std::list<CInputData>::iterator it = listInputData.begin(); it != listInputData.end(); it++)
      {
        if (XMLHandler.m_mapXMLResdata.find(it->strAddonName) != XMLHandler.m_mapXMLResdata.end())
        {
          CXMLResdata XMLResdata = XMLHandler.GetXMLResdata(*it);
          CLog::Log(logINFO, "%s%s%s", KMAG, XMLResdata.strResNameFull.c_str(), RESET);

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
    }


    // list addons mode
    if (bListAddonsMode)
    {
      printf("\n"); 
      for (std::map<std::string, CXMLResdata>::iterator it = XMLHandler.m_mapXMLResdata.begin(); it != XMLHandler.m_mapXMLResdata.end(); it++)
      {
        printf ("%s%s%s (%s)\n", KMAG, it->first.c_str(), RESET, !it->second.strChangelogFormat.empty()? " changelog.txt":"");
      }
    }

    strLogMessage = "PROCESS FINISHED WITH " + g_File.IntToStr(CLog::GetWarnCount()) + " WARNINGS";
    strLogHeader.clear();
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
