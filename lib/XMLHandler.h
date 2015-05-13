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
#pragma once
#ifndef XMLHANDLER_H
#define XMLHANDLER_H

#include "TinyXML/tinyxml.h"
#include <string>
#include <map>
#include <list>

class CInputData
{
public:
  CInputData();
  ~CInputData();
  std::string strAddonName;
  std::string strAddonDir;
  std::string strAddonDirForSource;
  std::string strGittemplate;
  std::string strGittemplateSRC;
  std::string strGitExecPath;
  std::string strGitExecPathSRC;
  bool bSkipChangelog;
  bool bClearLangdir;
  bool bSkipLangfiles;
  bool bSkipSRCLangfile;
};

class CXMLResdata
{
public:
  CXMLResdata();
  ~CXMLResdata();
  std::string strName, strResNameFull;

  std::string strTXName, strTargetTXName;

  std::string strUPSLangURL, strUPSLangURLRoot, strUPSLangFormat, strUPSLangFileName;
  std::string strUPSSourceLangURL;
  std::string strUPSAddonURL, strUPSAddonURLRoot, strUPSAddonLangFormat, strUPSAddonLangFormatinXML, strUPSAddonXMLFilename;
  std::string strUPSSourceLangAddonURL;
  std::string strUPSChangelogURL, strUPSChangelogURLRoot, strUPSChangelogName;

  std::string strLOCLangPath, strLOCLangPathRoot, strLOCLangFormat, strLOCLangFileName;
  std::string strLOCAddonPath, strLOCAddonPathRoot, strLOCAddonLangFormat, strLOCAddonLangFormatinXML, strLOCAddonXMLFilename;
  std::string strLOCChangelogPath, strLOCChangelogPathRoot, strLOCChangelogName;

  std::string strChangelogFormat;
  bool bIsLanguageAddon;

  bool bHasOnlyAddonXML;
  bool bSkipChangelog;
  bool bClearLangdir;
  bool bSkipLangfiles;
  bool bSkipSRCLangfile;
  std::string strTranslationrepoURL;
  std::string strProjName;
  std::string strMergedLangfileDir;
  std::string strSourceLcode;
  std::string strBaseLCode;
  std::string DefaultAddonLFormatinXML;
  std::string LangDatabaseURL;

  std::string strResLocalDirectory;
  std::string strResLocalDirectoryForSRC;
  std::string strGittemplate;
  std::string strGitExecPath;
  std::string strGittemplateSRC;
  std::string strGitExecPathSRC;
  std::string strAddonVersion;
};

class CUpdateXMLHandler
{
public:
  CUpdateXMLHandler();
  ~CUpdateXMLHandler();
  bool DownloadXMLToMap (std::string strURL);
  bool GetParamsFromURLorPath (std::string const &strURL, std::string &strLangFormat, std::string &strFileName,
                               std::string &strURLRoot, const char strSeparator);
  bool GetParamsFromURLorPath (std::string const &strURL, std::string &strFileName,
                               std::string &strURLRoot, const char strSeparator);

  std::map<std::string, CXMLResdata> m_mapXMLResdata;
  std::map<std::string, CXMLResdata>::iterator itXMLResdata;

//  size_t m_CacheExpire;
//  int m_minComplPercentage;
//  std::string m_strProjectName;
//  std::string m_strTargetProjectName;
//  std::string m_strMergedLangfilesDir;
//  std::string m_strProjectnameLong;
//  std::string m_strSourceLcode;
//  std::string m_strBaseLCode;
//  std::string m_strDefTXLFormat;
//  std::string m_strTargTXLFormat;
//  std::string m_DefaultAddonLFormatinXML;
//  std::string m_LangDatabaseURL;
};

class CInputXMLHandler
{
public:
  CInputXMLHandler();
  ~CInputXMLHandler();
  std::list<CInputData> ReadXMLToMem (std::string strFileName);
};

#endif