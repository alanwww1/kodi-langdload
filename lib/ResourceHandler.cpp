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

#include <list>
#include <algorithm>
#include "ResourceHandler.h"
#include "JSONHandler.h"
#include "HTTPUtils.h"
#include "Log.h"
#include "FileUtils.h"
#include "LCode.h"
#include "CharsetUtils.h"

using namespace std;

CResourceHandler::CResourceHandler()
{};

CResourceHandler::~CResourceHandler()
{};

bool CResourceHandler::DloadLangFiles(CXMLResdata &XMLResdata)
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit();

  std::string strLogMessage = "DOWNLOADING RESOURCE: " + XMLResdata.strResNameFull + " FROM XBMC REPO";
  std::string strLogHeader;
  strLogHeader.resize(strLogMessage.size(), '*');
  CLog::Log(logLINEFEED, "");
  CLog::Log(logINFO, "%s", strLogHeader.c_str());
  CLog::Log(logINFO, "%s", strLogMessage.c_str());
  CLog::Log(logINFO, "%s", strLogHeader.c_str());
  CLog::IncIdent(2);

  if (!XMLResdata.strUPSAddonURL.empty()) //we do have an addon.xml file existing
  {
    std::string strDloadURL = XMLResdata.strTranslationrepoURL;
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strMergedLangfileDir);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strLOCAddonPath);

    std::string strFilename = XMLResdata.strResLocalDirectory;
    g_File.AddToFilename(strFilename, XMLResdata.strLOCAddonPath);

    std::string strAddonXMLFile = g_HTTPHandler.GetURLToSTR(strDloadURL);
    if (!XMLResdata.strGittemplate.empty())
      XMLResdata.strAddonVersion = GetAddonVersion(strAddonXMLFile);
    g_File.WriteFileFromStr(strFilename, strAddonXMLFile);

    CLog::Log(logINFO, "ResHandler: addon.xml downloaded for resource: %s",XMLResdata.strResNameFull.c_str());
  }

  if (!XMLResdata.bSkipChangelog && !XMLResdata.strChangelogFormat.empty())
  {
    std::string strDloadURL = XMLResdata.strTranslationrepoURL;
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strMergedLangfileDir);
    g_HTTPHandler.AddToURL(strDloadURL, XMLResdata.strLOCChangelogPath);

    std::string strFilename = XMLResdata.strResLocalDirectory;
    g_File.AddToFilename(strFilename, XMLResdata.strLOCChangelogPath);

    g_HTTPHandler.DloadURLToFile(strDloadURL, strFilename);
    CLog::Log(logINFO, "ResHandler: changelog.txt downloaded for resource: %s",XMLResdata.strResNameFull.c_str());
  }

  if (XMLResdata.bHasOnlyAddonXML)
  {
    CLog::DecIdent(2);
    return true;
  }

  std::list<std::string> listLangs;


  std::string strtemp = g_HTTPHandler.GetURLToSTR(g_HTTPHandler.GetGitHUBAPIURL(XMLResdata.strTranslationrepoURL + XMLResdata.strLOCLangPathRoot));

  if (strtemp.empty())
    CLog::Log(logERROR, "ResHandler::DloadLangFiles: error getting langfile list from xbmc translation github repo");

  listLangs = g_Json.ParseAvailDirsGITHUB(strtemp);

  std::string strLangDloadURL = XMLResdata.strTranslationrepoURL;
  g_HTTPHandler.AddToURL(strLangDloadURL, XMLResdata.strLOCLangPath);

  std::string strLangFilename = XMLResdata.strResLocalDirectory;
  g_File.AddToFilename(strLangFilename, XMLResdata.strLOCLangPath);

//TODO delete directory refactor to new langformat
//  if (XMLResdata.bClearLangdir)
//  {
//    g_File.DelDirectory(strLangFilename);
//    g_File.MakeDir(strLangFilename);
//  }

  CLog::Log(logINFO, "ResHandler: Downloading language files:");

  for (std::list<std::string>::iterator it = listLangs.begin(); it != listLangs.end(); it++)
  {
    if (XMLResdata.bSkipEnglishFile && *it == XMLResdata.strSourceLcode)
      continue;

    printf (" %s", it->c_str());

    std::string strDloadURL = strLangDloadURL;
    g_CharsetUtils.ReplaceLanginURL(strDloadURL, XMLResdata.strLOCLangFormat, *it, XMLResdata.strProjName);

    std::string strFilename = strLangFilename;
    g_CharsetUtils.ReplaceLanginURL(strDloadURL, XMLResdata.strLOCLangFormat, *it, XMLResdata.strProjName);

    g_HTTPHandler.DloadURLToFile(strDloadURL, strFilename);
  }

  int langcount = listLangs.size();
  if (XMLResdata.bSkipEnglishFile)
    langcount--;
  printf ("\n\n");
  CLog::Log(logINFO, "ResHandler: %i language files were downloaded for resource: %s",langcount, XMLResdata.strName.c_str());
  CLog::DecIdent(2);

  return true;
}

std::string CResourceHandler::GetAddonVersion(std::string const &strAddonXMLFile)
{
  if (strAddonXMLFile.empty())
    CLog::Log(logERROR, "CResourceHandler::GetAddonVersion: Error reading the addon.xml file. File is empty.");

  TiXmlDocument xmlAddonXML;

  if (!xmlAddonXML.Parse(strAddonXMLFile.c_str(), 0, TIXML_DEFAULT_ENCODING))
    CLog::Log(logERROR, "CResourceHandler::GetAddonVersion: XML file problem: %s\n", xmlAddonXML.ErrorDesc());

  TiXmlElement* pRootElement = xmlAddonXML.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="addon")
    CLog::Log(logERROR, "CResourceHandler::GetAddonVersion: No root element called \"addon\" in xml file. Cannot continue.");

  std::string strAddonVersion;
  if (!pRootElement->Attribute("version") || (strAddonVersion = pRootElement->Attribute("version")) == "")
    CLog::Log(logERROR, "CResourceHandler::GetAddonVersion: No addon version is specified in the addon.xml file. Cannot continue. "
    "Please contact the addon developer about this problem!");

  return strAddonVersion;
}