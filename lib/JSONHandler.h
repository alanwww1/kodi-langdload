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

#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#pragma once

#include <string>
#include <stdio.h>
#include "Json-cpp/include/json/json.h"
#include <map>
#include <list>
#include "LCode.h"
#include "XMLHandler.h"

class CJSONHandler
{
public:
  CJSONHandler();
  ~CJSONHandler();
  std::list<std::string> ParseAvailDirsGITHUB(std::string strJSON);
  std::map<std::string, CLangcodes> ParseTransifexLanguageDatabase(std::string strJSON, const std::string& strBaseLCode);
  void ParseLangDatabaseVersion(const std::string &strJSON, const std::string &strURL, const std::string& strCachename);
  std::list<std::string> ParseAvailLangDirsGITHUB(const std::string &strJSON, const CXMLResdata &XMLResdata);
  void ParseAddonXMLVersionGITHUB(const std::string &strJSON, const CXMLResdata & XMLResdata);
  void AddGeneralRule(std::map<std::string, CLangcodes> &mapTXLangs, const std::string &strLeft, std::string strRight);
  void AddCustomRule(std::map<std::string, CLangcodes> &mapTXLangs, const std::string &strLangformat, const std::string &strLeft, const std::string &strRight);
};
extern CJSONHandler g_Json;
#endif