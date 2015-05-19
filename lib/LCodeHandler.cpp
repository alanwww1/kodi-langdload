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
#include <stdio.h>
#include "LCodeHandler.h"
#include "Log.h"
#include "HTTPUtils.h"
#include "JSONHandler.h"
#include "TinyXML/tinyxml.h"

using namespace std;

CLCodeHandler::CLCodeHandler()
{}

CLCodeHandler::~CLCodeHandler()
{}

void CLCodeHandler::Init(std::string strURL, const std::string& strBaseLCode, const std::string& strProjectname)
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit(); 

  // We get the version of the language database files here
  std::string strGitHubURL = g_HTTPHandler.GetGitHUBAPIURL(strURL.substr(0,strURL.find_last_of("/")+1));
  CLog::Log(logINFONLF, "Language database version for project %s%s%s",KMAG, strProjectname.c_str(), RESET);

  std::string strtemp = g_HTTPHandler.GetURLToSTR(strGitHubURL);
  CLog::Log(logLINEFEED, "");

  if (strtemp.empty())
    CLog::Log(logERROR, "CLCodeHandler::Init: error getting language file version from github.com with URL: %s", strURL.c_str());

  std::string strCachename = strProjectname + "/" + "Langdatabase";
  g_Json.ParseFileVersion(strtemp, strURL, strCachename);

  CLog::Log(logINFONLF, "Language database file for project %s%s%s", KMAG, strProjectname.c_str(), RESET);

  strtemp = g_HTTPHandler.GetURLToSTR(strURL, strCachename);

  if (strtemp.empty())
  {
    CLog::Log(logLINEFEED, "");
    CLog::Log(logERROR, "LangCode::Init: error getting available language list from URL %s", strURL.c_str());
  }

  m_mapLCodes = g_Json.ParseTransifexLanguageDatabase(strtemp, strBaseLCode);

  CLog::Log(logINFO, "(fetched %s%i%s language codes)", KCYN, m_mapLCodes.size(), RESET);
  CLog::Log(logLINEFEED, "");
}

int CLCodeHandler::GetnPlurals(std::string LangCode)
{
  if (m_mapLCodes.find(LangCode) != m_mapLCodes.end())
    return m_mapLCodes[LangCode].nplurals;
  CLog::Log(logERROR, "LangCodes: GetnPlurals: unable to find langcode: %s", LangCode.c_str());
  return 0;
}

std::string CLCodeHandler::GetPlurForm(std::string LangCode)
{
  if (m_mapLCodes.find(LangCode) != m_mapLCodes.end())
    return m_mapLCodes[LangCode].Pluralform;
  CLog::Log(logERROR, "LangCodes: GetPlurForm: unable to find langcode: %s", LangCode.c_str());
  return "(n != 1)";
}

std::string CLCodeHandler::GetLangFromLCode(std::string LangCode, std::string AliasForm)
{
  CleanLangform(AliasForm);
  if (m_mapLCodes.find(LangCode) != m_mapLCodes.end() &&
      m_mapLCodes[LangCode].mapLangdata.find(AliasForm) != m_mapLCodes[LangCode].mapLangdata.end())
    return m_mapLCodes[LangCode].mapLangdata[AliasForm];
  return "";
}

std::string CLCodeHandler::GetLangCodeFromAlias(std::string Alias, std::string AliasForm)
{
  if (Alias == "")
    return "";

  CleanLangform(AliasForm);

  for (itmapLCodes = m_mapLCodes.begin(); itmapLCodes != m_mapLCodes.end() ; itmapLCodes++)
  {
    std::map<std::string, std::string> mapLangdata = itmapLCodes->second.mapLangdata;
    if (itmapLCodes->second.mapLangdata.find(AliasForm) != itmapLCodes->second.mapLangdata.end() &&
        Alias == itmapLCodes->second.mapLangdata[AliasForm])
      return itmapLCodes->first;
  }
  return "";
}

std::string CLCodeHandler::VerifyLangCode(std::string LangCode, const std::string &strLangformat)
{
  if (strLangformat == "$(OLDLCODE)")
  {
    std::string strOldCode = LangCode;

    // common mistakes, we correct them on the fly
    if (LangCode == "kr") LangCode = "ko";
    if (LangCode == "cr") LangCode = "hr";
    if (LangCode == "cz") LangCode = "cs";

    if (strOldCode != LangCode)
      CLog::Log(logWARNING, "LangCodes: problematic language code: %s was corrected to %s", strOldCode.c_str(), LangCode.c_str());
  }

  if ((LangCode = GetLangCodeFromAlias(LangCode, strLangformat)) != "")
    return LangCode;
  return "";
}

void CLCodeHandler::CleanLangform (std::string &strLangform)
{
  size_t pos1, pos2;
  pos1 = strLangform.find_first_not_of("$(");
  pos2 = strLangform.find_last_not_of(")");
  strLangform = strLangform.substr(pos1, pos2-pos1+1);
}
