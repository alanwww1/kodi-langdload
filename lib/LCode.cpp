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
#include "LCode.h"
#include "Log.h"

using namespace std;

CLCode g_LCode;

CLCode::CLCode()
{}

CLCode::~CLCode()
{}

void CLCode::Init(std::string strURL, std::string strProjectname, const std::string& strBaseLCode)
{
  if (m_mapLCodeHandlers.find(strProjectname) != m_mapLCodeHandlers.end())
    return;
  CLCodeHandler LCodeHandler;
  LCodeHandler.Init(strURL, strBaseLCode);
  m_mapLCodeHandlers[strProjectname] = LCodeHandler;
}

std::string CLCode::GetLangFromLCode(std::string LangCode, std::string AliasForm, std::string strProjectname)
{
  if (m_mapLCodeHandlers.find(strProjectname) == m_mapLCodeHandlers.end())
    return "";
  return m_mapLCodeHandlers[strProjectname].GetLangFromLCode(LangCode, AliasForm);
}

std::string CLCode::GetLangCodeFromAlias(std::string Alias, std::string AliasForm, std::string strProjectname)
{
  if (m_mapLCodeHandlers.find(strProjectname) == m_mapLCodeHandlers.end())
    return "";
  return m_mapLCodeHandlers[strProjectname].GetLangCodeFromAlias(Alias, AliasForm);
}

std::string CLCode::VerifyLangCode(std::string LangCode, const std::string &strLangformat, std::string strProjectname)
{
  if (m_mapLCodeHandlers.find(strProjectname) == m_mapLCodeHandlers.end())
    return "";
  return m_mapLCodeHandlers[strProjectname].VerifyLangCode(LangCode, strLangformat);
}

void CLCode::CleanLangform (std::string &strLangform)
{
  size_t pos1, pos2;
  pos1 = strLangform.find_first_not_of("$(");
  pos2 = strLangform.find_last_not_of(")");
  strLangform = strLangform.substr(pos1, pos2-pos1+1);
}
