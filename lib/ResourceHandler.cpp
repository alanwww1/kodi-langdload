/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
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

#include "ResourceHandler.h"
#include <list>
#include <algorithm>
#include "JSONHandler.h"
#include "HTTPUtils.h"
#include "Log.h"
#include "FileUtils.h"

using namespace std;

CResourceHandler::CResourceHandler()
{};

CResourceHandler::~CResourceHandler()
{};

bool CResourceHandler::DloadLangFiles(CXMLResdata XMLResdata)
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit();
  CLog::Log(logINFO, "ResHandler: Starting to load resource from URL: %s into memory",XMLResdata.strTranslationrepoURL.c_str());

  std::string strLangURLSuffix = GetLangURLSuffix(XMLResdata);

  if (!XMLResdata.Restype != CORE)
  {
    std::string strDloadURL = XMLResdata.strTranslationrepoURL;
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strMergedLangfileDir);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strResDirectory);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strResName);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strDIRprefix);
    g_HTTPHandler.AddToURL(strDloadURL, "addon.xml");
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strAddonXMLSuffix);

    std::string strFilename = XMLResdata.strResLocalDirectory;
    g_File.AddToFilename(strFilename, XMLResdata.strDIRprefix);
    g_File.AddToFilename(strFilename, "addon.xml");
    g_File.AddToFilename(strFilename, XMLResdata.strAddonXMLSuffix);

    g_HTTPHandler.DloadURLToFile(strDloadURL, strFilename);
  }

  if (XMLResdata.bHasChangelog && !XMLResdata.Restype != CORE)
  {
    std::string strDloadURL = XMLResdata.strTranslationrepoURL;
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strMergedLangfileDir);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strResDirectory);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strResName);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strDIRprefix);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strLogFilename);

    std::string strFilename = XMLResdata.strResLocalDirectory;
    g_File.AddToFilename(strFilename, XMLResdata.strDIRprefix);
    g_File.AddToFilename(strFilename, XMLResdata.strLogFilename);
    g_File.AddToFilename(strFilename, XMLResdata.strAddonXMLSuffix);

    g_HTTPHandler.DloadURLToFile(strDloadURL, strFilename);
  }

  if (XMLResdata.Restype == ADDON_NOSTRINGS)
    return true;

  std::list<std::string> listLangs;

  size_t pos1, pos2, pos3;
  std::string strGitHubURL, strGitBranch;
  if (XMLResdata.strTranslationrepoURL.find("raw.github.com/") == std::string::npos)
    CLog::Log(logERROR, "ResHandler: Wrong Github URL format");
  pos1 = XMLResdata.strTranslationrepoURL.find("raw.github.com/")+15;
  pos2 = XMLResdata.strTranslationrepoURL.find("/", pos1+1);
  pos2 = XMLResdata.strTranslationrepoURL.find("/", pos2+1);
  pos3 = XMLResdata.strTranslationrepoURL.find("/", pos2+1);
  strGitHubURL = "https://api.github.com/repos/" + XMLResdata.strTranslationrepoURL.substr(pos1, pos2-pos1);
  strGitHubURL += "/contents";
  strGitHubURL += XMLResdata.strTranslationrepoURL.substr(pos3, XMLResdata.strTranslationrepoURL.size() - pos3 - 1);
  strGitBranch = XMLResdata.strTranslationrepoURL.substr(pos2+1, pos3-pos2-1);

  strGitHubURL += "/" + XMLResdata.strMergedLangfileDir + XMLResdata.strResDirectory + "/" + XMLResdata.strResName + XMLResdata.strDIRprefix;

  if (XMLResdata.Restype == SKIN || XMLResdata.Restype == CORE)
    strGitHubURL += "/language";
  else if (XMLResdata.Restype == ADDON)
    strGitHubURL += "/resources/language";
  strGitHubURL += "?ref=" + strGitBranch;

  std::string strtemp = g_HTTPHandler.GetURLToSTR(strGitHubURL);
  if (strtemp.empty())
    CLog::Log(logERROR, "ResHandler::DloadLangFiles: error getting langfile list from xbmc translation github repo");

  char cstrtemp[strtemp.size()];
  strcpy(cstrtemp, strtemp.c_str());

  listLangs = g_Json.ParseAvailLanguagesGITHUB(strtemp);

  for (std::list<std::string>::iterator it = listLangs.begin(); it != listLangs.end(); it++)
  {
    printf (" %s", it->c_str());

    if (XMLResdata.bWriteXML)
      g_HTTPHandler.DloadURLToFile(XMLResdata.strTranslationrepoURL + strLangURLSuffix + *it + "/strings.xml" + XMLResdata.strURLSuffix,
                                   GetLangDir(XMLResdata) + "strings.xml");
    else if (XMLResdata.bWritePO)
      g_HTTPHandler.DloadURLToFile(XMLResdata.strTranslationrepoURL + strLangURLSuffix + *it + "/strings.po" + XMLResdata.strURLSuffix,
                                   GetLangDir(XMLResdata) + "strings.po");
  }
  return true;
}

std::string CResourceHandler::GetLangDir(CXMLResdata const &XMLResdata)
{
  std::string strLangDir;
  switch (XMLResdata.Restype)
  {
    case ADDON: case ADDON_NOSTRINGS:
      strLangDir = XMLResdata.strResLocalDirectory + "resources" + DirSepChar + "language" + DirSepChar;
      break;
    case SKIN:
      strLangDir = XMLResdata.strResLocalDirectory + "language" + DirSepChar;
      break;
    case CORE:
      strLangDir = XMLResdata.strResLocalDirectory + "language" + DirSepChar;
      break;
    default:
      CLog::Log(logERROR, "ResHandler: No resourcetype defined for resource: %s",XMLResdata.strResName.c_str());
  }
  return strLangDir;
}

std::string CResourceHandler::GetLangURLSuffix(CXMLResdata const &XMLResdata)
{
  std::string strLangURLSuffix;
  switch (XMLResdata.Restype)
  {
    case ADDON:
      strLangURLSuffix = "resources/language/";
      break;
    case SKIN:
      strLangURLSuffix = "language/";
      break;
    case CORE:
      strLangURLSuffix = "language/";
      break;
    default:
      CLog::Log(logERROR, "ResHandler: No resourcetype defined for resource: %s",XMLResdata.strResName.c_str());
  }
  return strLangURLSuffix;
}
