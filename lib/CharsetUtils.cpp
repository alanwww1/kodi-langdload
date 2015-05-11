/*
 *      Copyright (C) 2005-2014 Team Kodi
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
 *  along with Kodi; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <string>
#include "CharsetUtils.h"
#include "Log.h"
#include "LCode.h"

CCharsetUtils g_CharsetUtils;


bool CCharsetUtils::replaceAllStrParts(std::string * pstr, const std::string& from, const std::string& to)
{
  if (pstr->find(from) == std::string::npos)
    return false;

  size_t start_pos = 0;
  while((start_pos = pstr->find(from, start_pos)) != std::string::npos)
  {
    pstr->replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
  return true;
};

std::string CCharsetUtils::replaceStrParts(std::string strToReplace, const std::string& from, const std::string& to)
{
  replaceAllStrParts(&strToReplace, from, to);
  return strToReplace;
};


std::string CCharsetUtils::GetRoot(const std::string &strPath,const std::string &strFilename)
{
  return strPath.substr(0, strPath.size()-strFilename.size());
}

std::string CCharsetUtils::GetLangnameFromURL(std::string strName, std::string strURL, std::string strLangformat)
{
  //Get Directory nameformat
  size_t pos1 = strURL.find(strLangformat);
  if (pos1 == std::string::npos)
    CLog::Log(logERROR, "CharsetUtils::GetLangnameFromURL: Wrong URL format: %s", strURL.c_str());

  size_t pos2 = pos1 + strLangformat.size();
  if (pos2 > strURL.size())
    CLog::Log(logERROR, "CharsetUtils::GetLangnameFromURL: Wrong URL format: %s", strURL.c_str());

  size_t pos1per = strURL.substr(0,pos1).find_last_of("/");

  size_t pos2per = strURL.find_first_of("/",pos2);
  if (pos2per == std::string::npos)
    CLog::Log(logERROR, "CharsetUtils::GetLangnameFromURL: Wrong URL format: %s", strURL.c_str());

  std::string strPre;

  if (pos1per != std::string::npos)
    strPre = strURL.substr(pos1per+1, pos1-pos1per-1);
  else
    strPre = strURL.substr(0, pos1);

  std::string strPost = strURL.substr(pos2, pos2per-pos2);

  if (strPre != "" && strName.find(strPre) != 0 )
    return "";
  if (strName.rfind(strPost) != strName.size()-strPost.size())
    return "";

  return strName.substr(strPre.size(), strName.size()-strPre.size()-strPost.size());
}

std::string CCharsetUtils::ReplaceLanginURL(const std::string& strURL, const std::string& strLangFormat,
                                            const std::string& strLCode, const std::string& strProjectName)
{
  return replaceStrParts(strURL, strLangFormat, g_LCode.GetLangFromLCode(strLCode, strLangFormat, strProjectName));
}

bool CCharsetUtils::bISPOFile(const std::string strFilename)
{
  return (strFilename.find(".po") != std::string::npos || strFilename.find(".PO") != std::string::npos);
}

bool CCharsetUtils::bISXMLFile(const std::string strFilename)
{
  return (strFilename.find(".xml") != std::string::npos || strFilename.find(".XML") != std::string::npos);
}