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

#include "XMLHandler.h"
#include "Log.h"
#include "HTTPUtils.h"
#include "FileUtils.h"
#include <list>
#include "CharsetUtils.h"
#include "LCode.h"

using namespace std;

CXMLResdata::CXMLResdata()
{}

CXMLResdata::~CXMLResdata()
{}

CInputData::CInputData()
{
  bSkipChangelog = false;
  bClearLangdir =false;
  bSkipLangfiles = false;
  bSkipSRCLangfile = false;
  strGitExecPath = "C:\\Program Files (x86)\\Git\\bin\\";
}

CInputData::~CInputData()
{}

CUpdateXMLHandler::CUpdateXMLHandler()
{};

CUpdateXMLHandler::~CUpdateXMLHandler()
{};

bool CUpdateXMLHandler::DownloadXMLToMap (const std::string& strProjectname, const std::string& strURL)
{
  std::string strURLXMLFile = strURL + "kodi-txupdate.xml";

  std::string strCachename = strProjectname + "/kodi-txupdate";
  std::string strXMLFile = g_HTTPHandler.GetURLToSTR(strURLXMLFile, strCachename);
  if (strXMLFile.empty())
    CLog::Log(logERROR, "CXMLHandler::DownloadXMLToMap: http error getting XML file from upstream url: %s", strURL.c_str());
  TiXmlDocument xmlUpdateXML;

  if (!xmlUpdateXML.Parse(strXMLFile.c_str(), 0, TIXML_DEFAULT_ENCODING))
  {
    CLog::Log(logERROR, "CUpdateXMLHandler::DownloadXMLToMap: UpdateXML file problem: %s %s\n", xmlUpdateXML.ErrorDesc(), strURL.c_str());
    return false;
  }

  TiXmlElement* pProjectRootElement = xmlUpdateXML.RootElement();
  if (!pProjectRootElement || pProjectRootElement->NoChildren() || pProjectRootElement->ValueTStr()!="project")
  {
    CLog::Log(logERROR, "CXMLHandler::DownloadXMLToMap: No root element called \"project\" in xml file. Cannot continue. Please create it");
    return false;
  }

  TiXmlElement* pDataRootElement = pProjectRootElement->FirstChildElement("projectdata");
  if (!pDataRootElement || pDataRootElement->NoChildren())
  {
    CLog::Log(logERROR, "CXMLHandler::DownloadXMLToMap: No element called \"projectdata\" in xml file. Cannot continue. Please create it");
    return false;
  }

  TiXmlElement * pData;

  //TODO separate download TX projectname from upload TX projectname to handle project name changes
  std::string strProjName, strTargetProjectName;
  if ((pData = pDataRootElement->FirstChildElement("projectname")))
    strProjName = pData->FirstChild()->Value();

  if ((pData = pDataRootElement->FirstChildElement("targetprojectname")))
    strTargetProjectName = pData->FirstChild()->Value();
  if (strTargetProjectName.empty())
    strTargetProjectName = strProjName;

  std::string strDefLangdatabaseURL;
  if ((pData = pDataRootElement->FirstChildElement("langdatabaseurl")))
    strDefLangdatabaseURL = pData->FirstChild()->Value();
  if (strDefLangdatabaseURL.empty())
    strDefLangdatabaseURL = "https://raw.github.com/xbmc/translations/master/tool/lang-database/kodi-languages.json";

  std::string strBaseLcode;
  if ((pData = pDataRootElement->FirstChildElement("baselcode")))
    strBaseLcode = pData->FirstChild()->Value();
  if (strBaseLcode.empty())
    strBaseLcode = "$(LCODE)";

    std::string strMergedLangfileDir;
  if ((pData = pDataRootElement->FirstChildElement("merged_langfiledir")))
    strMergedLangfileDir = pData->FirstChild()->Value();
  if (strMergedLangfileDir.empty())
    strMergedLangfileDir = "merged-langfiles";

  std::string strSourcelcode;
  if ((pData = pDataRootElement->FirstChildElement("sourcelcode")))
    strSourcelcode = pData->FirstChild()->Value();
  if (strSourcelcode.empty())
    strSourcelcode = "en_GB";


  TiXmlElement* pRootElement = pProjectRootElement->FirstChildElement("resources");
  if (!pRootElement || pRootElement->NoChildren())
  {
    CLog::Log(logERROR, "CXMLHandler::DownloadXMLToMap: No element called \"resources\" in xml file. Cannot continue. Please create it");
  }

  const TiXmlElement *pChildResElement = pRootElement->FirstChildElement("resource");
  if (!pChildResElement || pChildResElement->NoChildren())
  {
    CLog::Log(logERROR, "CXMLHandler::DownloadXMLToMap: No xml element called \"resource\" exists in the xml file. Cannot continue. Please create at least one");
  }

  std::string strType;
  while (pChildResElement && pChildResElement->FirstChild())
  {
    CXMLResdata currResData;

    currResData.strTranslationrepoURL = strURL;
    currResData.strProjName = strTargetProjectName;
    currResData.strMergedLangfileDir = strMergedLangfileDir;

    std::string strResName;
    if (!pChildResElement->Attribute("name") || (strResName = pChildResElement->Attribute("name")) == "")
    {
      CLog::Log(logERROR, "CXMLHandler::DownloadXMLToMap: No name specified for resource. Cannot continue. Please specify it.");
    }
    currResData.strName =strResName;

    if (pChildResElement->FirstChild())
    {
      const TiXmlElement *pChildURLElement = pChildResElement->FirstChildElement("upstreamLangURL");
      if (pChildURLElement && pChildURLElement->FirstChild())
        currResData.strUPSLangURL = pChildURLElement->FirstChild()->Value();
      if (!(currResData.bHasOnlyAddonXML = currResData.strUPSLangURL.empty()) && (!GetParamsFromURLorPath (currResData.strUPSLangURL, currResData.strUPSLangFormat, currResData.strUPSLangFileName,
        currResData.strUPSLangURLRoot, '/')))
        CLog::Log(logERROR, "CXMLHandler::DownloadXMLToMap: UpstreamURL format is wrong for resource %s", strResName.c_str());
      if (!currResData.strUPSLangURLRoot.empty() && currResData.strUPSLangURLRoot.find (".github") == std::string::npos)
        CLog::Log(logERROR, "CXMLHandler::DownloadXMLToMap: Only github is supported as upstream repository for resource %s", strResName.c_str());

      const TiXmlElement *pChildURLSRCElement = pChildResElement->FirstChildElement("upstreamLangSRCURL");
      if (pChildURLSRCElement && pChildURLSRCElement->FirstChild())
        currResData.strUPSSourceLangURL = pChildURLSRCElement->FirstChild()->Value();

      const TiXmlElement *pChildURLSRCAddonElement = pChildResElement->FirstChildElement("upstreamAddonSRCURL");
      if (pChildURLSRCAddonElement && pChildURLSRCAddonElement->FirstChild())
        currResData.strUPSSourceLangAddonURL = pChildURLSRCAddonElement->FirstChild()->Value();

      const TiXmlElement *pChildAddonURLElement = pChildResElement->FirstChildElement("upstreamAddonURL");
      if (pChildAddonURLElement && pChildAddonURLElement->FirstChild())
        currResData.strUPSAddonURL = pChildAddonURLElement->FirstChild()->Value();
      if (currResData.strUPSAddonURL.empty())
        currResData.strUPSAddonURL = currResData.strUPSLangURL.substr(0,currResData.strUPSLangURL.find(currResData.strName)
        + currResData.strName.size()) + "/addon.xml";
      if (currResData.strUPSAddonURL.empty())
        CLog::Log(logERROR, "UpdXMLHandler: Unable to determine the URL for the addon.xml file for resource %s", strResName.c_str());
      GetParamsFromURLorPath (currResData.strUPSAddonURL, currResData.strUPSAddonLangFormat, currResData.strUPSAddonXMLFilename,
                              currResData.strUPSAddonURLRoot, '/');
      if (!currResData.strUPSAddonURL.empty() && currResData.strUPSAddonURL.find (".github") == std::string::npos)
        CLog::Log(logERROR, "UpdXMLHandler: Only github is supported as upstream repository for resource %s", strResName.c_str());
      currResData.bIsLanguageAddon = !currResData.strUPSAddonLangFormat.empty();

      const TiXmlElement *pChildChglogElement = pChildResElement->FirstChildElement("changelogFormat");
      if (pChildChglogElement && pChildChglogElement->FirstChild())
        currResData.strChangelogFormat = pChildChglogElement->FirstChild()->Value();

      const TiXmlElement *pChildChglogUElement = pChildResElement->FirstChildElement("upstreamChangelogURL");
      if (pChildChglogUElement && pChildChglogUElement->FirstChild())
        currResData.strUPSChangelogURL = pChildChglogUElement->FirstChild()->Value();
      else if (!currResData.strChangelogFormat.empty())
        currResData.strUPSChangelogURL = currResData.strUPSAddonURLRoot + "changelog.txt";
      if (!currResData.strChangelogFormat.empty())
        GetParamsFromURLorPath (currResData.strUPSChangelogURL, currResData.strUPSChangelogName,
                                currResData.strUPSChangelogURLRoot, '/');

      const TiXmlElement *pChildLocLangElement = pChildResElement->FirstChildElement("localLangPath");
      if (pChildLocLangElement && pChildLocLangElement->FirstChild())
        currResData.strLOCLangPath = pChildLocLangElement->FirstChild()->Value();
      if (currResData.strLOCLangPath.empty() && !currResData.bHasOnlyAddonXML)
        currResData.strLOCLangPath =  currResData.strUPSLangURL.substr(currResData.strUPSAddonURLRoot.size()-1);
      if (!currResData.bHasOnlyAddonXML && !GetParamsFromURLorPath (currResData.strLOCLangPath, currResData.strLOCLangFormat,
        currResData.strLOCLangFileName, currResData.strLOCLangPathRoot, DirSepChar))
        CLog::Log(logERROR, "UpdXMLHandler: Local langpath format is wrong for resource %s", strResName.c_str());

      const TiXmlElement *pChildLocAddonElement = pChildResElement->FirstChildElement("localAddonPath");
      if (pChildLocAddonElement && pChildLocAddonElement->FirstChild())
        currResData.strLOCAddonPath = pChildLocAddonElement->FirstChild()->Value();
      if (currResData.strLOCAddonPath.empty())
        currResData.strLOCAddonPath = currResData.strUPSAddonXMLFilename;
      GetParamsFromURLorPath (currResData.strLOCAddonPath, currResData.strLOCAddonLangFormat, currResData.strLOCAddonXMLFilename,
                              currResData.strLOCAddonPathRoot, DirSepChar);

      const TiXmlElement *pChildChglogLElement = pChildResElement->FirstChildElement("localChangelogPath");
      if (pChildChglogLElement && pChildChglogLElement->FirstChild())
        currResData.strLOCChangelogPath = pChildChglogLElement->FirstChild()->Value();
      else
        currResData.strLOCChangelogPath = currResData.strLOCAddonPathRoot + "changelog.txt";
      GetParamsFromURLorPath (currResData.strLOCChangelogPath, currResData.strLOCChangelogName,
                              currResData.strLOCChangelogPathRoot, '/');

      currResData.strResNameFull = strTargetProjectName + "/" + currResData.strName;
      currResData.strBaseLCode = strBaseLcode;
      currResData.strSourceLcode = strSourcelcode;
      currResData.LangDatabaseURL = strDefLangdatabaseURL;

     m_mapXMLResdata [currResData.strResNameFull] = currResData;

    }
    pChildResElement = pChildResElement->NextSiblingElement("resource");
  }

  CLog::Log(logLINEFEED, "");
  g_LCode.Init(strDefLangdatabaseURL, strTargetProjectName, strBaseLcode);

  return true;
};

CInputXMLHandler::CInputXMLHandler()
{}

CInputXMLHandler::~CInputXMLHandler()
{}

std::list<CInputData> CInputXMLHandler::ReadXMLToMem(string strFileName)
{
  std::string strXMLFile = g_File.ReadFileToStr(strFileName);
  if (strXMLFile.empty())
    CLog::Log(logERROR, "CInputXMLHandler::ReadXMLToMem: http error getting XML file from path: %s", strFileName.c_str());

  TiXmlDocument xmlUpdateXML;

  if (!xmlUpdateXML.Parse(strXMLFile.c_str(), 0, TIXML_DEFAULT_ENCODING))
    CLog::Log(logERROR, "CInputXMLHandler::ReadXMLToMem: UpdateXML file problem: %s %s\n", xmlUpdateXML.ErrorDesc(), strFileName.c_str());

  TiXmlElement* pRootElement = xmlUpdateXML.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="addonlist")
    CLog::Log(logERROR, "CInputXMLHandler::ReadXMLToMem: No root element called \"addonlist\" in xml file. Cannot continue. Please create it");

  const TiXmlElement *pChildResElement = pRootElement->FirstChildElement("addon");
  if (!pChildResElement || pChildResElement->NoChildren())
    CLog::Log(logERROR, "CInputXMLHandler::ReadXMLToMem: No xml element called \"addon\" exists in the xml file. Please contact TeamKODI about this problem!");

  std::list<CInputData> listInputData;

  while (pChildResElement && pChildResElement->FirstChild())
  {
    CInputData currInputData;

    std::string strResName;
    if (!pChildResElement->Attribute("name") || (strResName = pChildResElement->Attribute("name")) == "")
      CLog::Log(logERROR, "CInputXMLHandler::ReadXMLToMem: No name specified for addon. Cannot continue.");

    currInputData.strAddonName = strResName;

    const TiXmlElement *pChildDirElement = pChildResElement->FirstChildElement("localdir");
    if (pChildDirElement && pChildDirElement->FirstChild())
      currInputData.strAddonDir = pChildDirElement->FirstChild()->Value();
    if (currInputData.strAddonDir.empty())
      CLog::Log(logERROR, "CInputXMLHandler::ReadXMLToMem: Local directory is missing for addon: %s", strResName.c_str());

    const TiXmlElement *pChildDirSRCElement = pChildResElement->FirstChildElement("localdirsource");
    if (pChildDirSRCElement && pChildDirSRCElement->FirstChild())
      currInputData.strAddonDirForSource = pChildDirSRCElement->FirstChild()->Value();

    std::string strBool;
    const TiXmlElement *pChildSkipchlogElement = pChildResElement->FirstChildElement("skipchangelog");
    if (pChildSkipchlogElement && pChildSkipchlogElement->FirstChild())
      strBool = pChildSkipchlogElement->FirstChild()->Value();
    currInputData.bSkipChangelog = (strBool == "true");

    strBool.clear();
    const TiXmlElement *pChildClearLangdir = pChildResElement->FirstChildElement("clearlangdir");
    if (pChildClearLangdir && pChildClearLangdir->FirstChild())
      strBool = pChildClearLangdir->FirstChild()->Value();
    currInputData.bClearLangdir = (strBool == "true");

    strBool.clear();
    const TiXmlElement *pChildSkipLangFiles = pChildResElement->FirstChildElement("skiplangfiles");
    if (pChildSkipLangFiles && pChildSkipLangFiles->FirstChild())
      strBool = pChildSkipLangFiles->FirstChild()->Value();
    currInputData.bSkipLangfiles = (strBool == "true");

    strBool.clear();
    const TiXmlElement *pChildSkipSrcLangFiles = pChildResElement->FirstChildElement("skipsrclangfile");
    if (pChildSkipSrcLangFiles && pChildSkipSrcLangFiles->FirstChild())
      strBool = pChildSkipSrcLangFiles->FirstChild()->Value();
    currInputData.bSkipSRCLangfile = (strBool == "true");

    const TiXmlElement *pChildGittemplElement = pChildResElement->FirstChildElement("gittemplate");
    if (pChildGittemplElement && pChildGittemplElement->FirstChild())
    {
      currInputData.strGittemplate = pChildGittemplElement->FirstChild()->Value();
      std::string strGitExecPath;
      if (pChildGittemplElement->Attribute("gitexecpath") && (strGitExecPath = pChildGittemplElement->Attribute("gitexecpath")) != "")
        currInputData.strGitExecPath = strGitExecPath;
    }

    const TiXmlElement *pChildGittemplSRCElement = pChildResElement->FirstChildElement("gittemplatesource");
    if (pChildGittemplSRCElement && pChildGittemplSRCElement->FirstChild())
    {
      currInputData.strGittemplateSRC = pChildGittemplSRCElement->FirstChild()->Value();
      std::string strGitExecPathSRC;
      if (pChildGittemplSRCElement->Attribute("gitexecpathsource") && (strGitExecPathSRC = pChildGittemplSRCElement->Attribute("gitexecpathsource")) != "")
        currInputData.strGitExecPathSRC = strGitExecPathSRC;
    }

    listInputData.push_back(currInputData);

    pChildResElement = pChildResElement->NextSiblingElement("addon");
  }
  return listInputData;
}

bool CUpdateXMLHandler::GetParamsFromURLorPath (string const &strURL, string &strLangFormat, string &strFileName,
                                                string &strURLRoot, const char strSeparator)
{
  if (strURL.empty())
    return false;

  size_t pos0, posStart, posEnd;

  pos0 = strURL.find_last_of("$");
  if (((posStart = strURL.find("$("), pos0) != std::string::npos) && ((posEnd = strURL.find(")",posStart)) != std::string::npos))
    strLangFormat = strURL.substr(posStart, posEnd - posStart +1);

  return GetParamsFromURLorPath (strURL, strFileName, strURLRoot, strSeparator);
}

bool CUpdateXMLHandler::GetParamsFromURLorPath (string const &strURL, string &strFileName,
                                                string &strURLRoot, const char strSeparator)
{
  if (strURL.empty())
    return false;

  if (strURL.find(strSeparator) == std::string::npos)
  {
    strFileName = strURL;
    strURLRoot = "";
    return false;
  }

  size_t possep = strURL.find_last_of(strSeparator);

  strFileName = strURL.substr(possep+1);
  strURLRoot = g_CharsetUtils.GetRoot(strURL, strFileName);
  return true;
}