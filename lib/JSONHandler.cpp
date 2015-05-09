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

#include "JSONHandler.h"
#include "Log.h"
#include <list>
#include <stdlib.h>
#include "Fileversioning.h"
#include "XMLHandler.h"
#include <algorithm>

CJSONHandler g_Json;

using namespace std;

CJSONHandler::CJSONHandler()
{};

CJSONHandler::~CJSONHandler()
{};


std::list<std::string> CJSONHandler::ParseAvailDirsGITHUB(std::string strJSON)
{
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  std::string lang;
  std::list<std::string> listLangs;

  bool parsingSuccessful = reader.parse(strJSON, root );
  if ( !parsingSuccessful )
    CLog::Log(logERROR, "CJSONHandler::ParseAvailDirsGITHUB: no valid JSON data downloaded from Github");

  const Json::Value JLangs = root;

  for(Json::ValueIterator itr = JLangs.begin() ; itr !=JLangs.end() ; itr++)
  {
    Json::Value JValu = *itr;
    std::string strType =JValu.get("type", "unknown").asString();
    if (strType == "unknown")
      CLog::Log(logERROR, "CJSONHandler::ParseAvailDirsGITHUB: no valid JSON data downloaded from Github");
    else if (strType != "dir")
    {
      CLog::Log(logWARNING, "CJSONHandler::ParseAvailDirsGITHUB: unknown file found in language directory");
      continue;
    }
    lang =JValu.get("name", "unknown").asString();
    if (strType == "unknown")
      CLog::Log(logERROR, "CJSONHandler::ParseAvailDirsGITHUB: no valid JSON data downloaded from Github");
    listLangs.push_back(lang);
  };

  return listLangs;
};

void CJSONHandler::ParseLangDatabaseVersion(const std::string &strJSON, const std::string &strURL)
{
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  std::string strName, strVersion;

  std::string strDatabaseFilename = strURL.substr(strURL.rfind("/")+1,std::string::npos);

  bool parsingSuccessful = reader.parse(strJSON, root );
  if ( !parsingSuccessful )
    CLog::Log(logERROR, "CJSONHandler::ParseAddonXMLVersionGITHUB: no valid JSON data downloaded from Github");

  const Json::Value JFiles = root;

  for(Json::ValueIterator itr = JFiles.begin() ; itr !=JFiles.end() ; itr++)
  {
    Json::Value JValu = *itr;
    std::string strType =JValu.get("type", "unknown").asString();

    if (strType == "unknown")
      CLog::Log(logERROR, "CJSONHandler::ParseLangDatabaseVersion: no valid JSON data downloaded from Github");

    strName =JValu.get("name", "unknown").asString();

    if (strName == "unknown")
      CLog::Log(logERROR, "CJSONHandler::ParseLangDatabaseVersion: no valid JSON data downloaded from Github");

    if (strType == "file" && strName == strDatabaseFilename)
    {
      strVersion =JValu.get("sha", "unknown").asString();

      if (strVersion == "unknown")
        CLog::Log(logERROR, "CJSONHandler::ParseLangDatabaseVersion: no valid sha JSON data downloaded from Github");

      g_Fileversion.SetVersionForURL(strURL, strVersion);
    }
  };
};

std::map<std::string, CLangcodes> CJSONHandler::ParseTransifexLanguageDatabase(std::string strJSON, const std::string& strBaseLCode)
{
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  std::string lang;

  bool parsingSuccessful = reader.parse(strJSON, root );
  if ( !parsingSuccessful )
  {
    CLog::Log(logERROR, "JSONHandler: ParseTXLanguageDB: no valid JSON data");
  }

  std::map<std::string, CLangcodes> mapTXLangs;

  const Json::Value JRoot = root;
  const Json::Value JLangs =  JRoot["fixtures"];

  for (Json::ValueIterator itrlangs = JLangs.begin() ; itrlangs !=JLangs.end() ; itrlangs++)
  {
    Json::Value JValu = *itrlangs;
    const Json::Value JAliases =JValu.get("aliases", "unknown");

    CLangcodes LangData;
    std::string strLCode;

    for (Json::ValueIterator itralias = JAliases.begin(); itralias !=JAliases.end() ; itralias++)
    {
      std::string langstrKey = itralias.key().asString();
      std::string langstrName = (*itralias).asString();
      LangData.mapLangdata[langstrKey] = langstrName;
      if ( "$(" + langstrKey + ")" == strBaseLCode)
        strLCode = langstrName;
    }

    if (strLCode.empty())
      CLog::Log(logERROR, "JSONHandler: ParseTXLanguageDB: Missing base langcode key in language database aliases");

    LangData.Pluralform = JValu.get("pluralequation", "unknown").asString();
    LangData.nplurals = JValu.get("nplurals", 0).asInt();

    if (!LangData.mapLangdata.empty() && LangData.Pluralform != "unknown" && LangData.nplurals != 0)
      mapTXLangs[strLCode] = LangData;
    else
      CLog::Log(logWARNING, "JSONHandler: ParseTXLanguageDB: corrupt JSON data found while parsing Language Database");
  };

  const Json::Value JRules =  JRoot.get("rules", "unknown");
  const Json::Value JRulesGen =  JRules.get("general","unknown");
  const Json::Value JRulesCust =  JRules["custom"];

  for (Json::ValueIterator itrules = JRulesGen.begin() ; itrules !=JRulesGen.end() ; itrules++)
  {
    std::string strLeft = itrules.key().asString();
    std::string strRight = (*itrules).asString();
    AddGeneralRule(mapTXLangs, strLeft, strRight);
  }

  for (Json::ValueIterator itrules = JRulesCust.begin() ; itrules !=JRulesCust.end() ; itrules++)
  {
    std::string strLangformat = itrules.key().asString();
    const Json::Value JRulesCustR = (*itrules);

    for (Json::ValueIterator itrulesR = JRulesCustR.begin() ; itrulesR !=JRulesCustR.end() ; itrulesR++)
    {
      std::string strLeft = itrulesR.key().asString(); //= itrulesR.key().asString();
      std::string strRight = (*itrulesR).asString();
      AddCustomRule(mapTXLangs, strLangformat, strLeft, strRight);
    }
  }

  return mapTXLangs;
};

void CJSONHandler::AddGeneralRule(std::map<std::string, CLangcodes> &mapTXLangs, const std::string &strLeft,
                                  std::string strRight)
{
  std::string strModifier;
  size_t pos1, pos2;
  if ((pos1 = strRight.find("(")) != std::string::npos || pos1 == 0) //we have a modifier
  {
    pos2 = strRight.find(")");
    strModifier = strRight.substr(1, pos2-1);
    strRight = strRight.substr(pos2+1,strRight.size()-pos2);
  }

  std::map<std::string, CLangcodes>::iterator itmapTXLangs;
  for (itmapTXLangs = mapTXLangs.begin(); itmapTXLangs != mapTXLangs.end(); itmapTXLangs++)
  {
    std::string strLangnametoAdd;
    if (strModifier == "lcase")
    {
      strLangnametoAdd = itmapTXLangs->second.mapLangdata[strRight];
      std::transform(strLangnametoAdd.begin(), strLangnametoAdd.end(), strLangnametoAdd.begin(), ::tolower);
    }
    else
      strLangnametoAdd = itmapTXLangs->second.mapLangdata[strRight];

    itmapTXLangs->second.mapLangdata[strLeft] = strLangnametoAdd;
  }
}

void CJSONHandler::AddCustomRule(std::map<std::string, CLangcodes> &mapTXLangs, const std::string &strLangformat,
                                 const std::string &strLeft, const std::string &strRight)
{
  mapTXLangs[strLeft].mapLangdata[strLangformat] = strRight;
}

