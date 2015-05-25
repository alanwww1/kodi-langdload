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

#include "ConfigHandler.h"
#include "FileUtils.h"
#include "Log.h"
#include <stdlib.h>

CConfigData::CConfigData()
{}

CConfigData::~CConfigData()
{}


CConfigXMLHandler::CConfigXMLHandler()
{}

CConfigXMLHandler::~CConfigXMLHandler()
{}

CConfigData CConfigXMLHandler::ReadConfigXML()
{
  const char* home = getenv("HOME");

  std::string strFileName;
  if (home)
  {
    std::string path(home);
    path += "/.config/kodi-langdload/config.xml";
    strFileName = path;
  }
  else
    CLog::Log(logERROR, "unable to determine HOME environment variable");

  CConfigData ConfigData;

  if (!g_File.FileExist(strFileName))
    return ConfigData;

  std::string strXMLFile = g_File.ReadFileToStr(strFileName);
  if (strXMLFile.empty())
    CLog::Log(logERROR, "CConfigXMLHandler::ReadXMLToMem: http error getting XML file from path: %s", strFileName.c_str());

  TiXmlDocument xmlConfigXML;

  if (!xmlConfigXML.Parse(strXMLFile.c_str(), 0, TIXML_DEFAULT_ENCODING))
    CLog::Log(logERROR, "CConfigXMLHandler::ReadXMLToMem: UpdateXML file problem: %s %s\n", xmlConfigXML.ErrorDesc(), strFileName.c_str());

  TiXmlElement* pRootElement = xmlConfigXML.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="config")
    CLog::Log(logERROR, "CConfigXMLHandler::ReadXMLToMem: No root element called \"config\" in xml file. Cannot continue. Please create it");

  const TiXmlElement *pChildUNElement = pRootElement->FirstChildElement("githubUsername");
  if (pChildUNElement && pChildUNElement->FirstChild())
    ConfigData.strGithubUsername = pChildUNElement->FirstChild()->Value();

  const TiXmlElement *pChildPWElement = pRootElement->FirstChildElement("githubPassword");
  if (pChildPWElement && pChildPWElement->FirstChild())
    ConfigData.strGithubPassword = pChildPWElement->FirstChild()->Value();

  const TiXmlElement *pChildTXElement = pRootElement->FirstChildElement("koditxupdatePath");
  if (pChildTXElement && pChildTXElement->FirstChild())
    ConfigData.strkoditxupdatePath = pChildTXElement->FirstChild()->Value();

  return ConfigData;
}
