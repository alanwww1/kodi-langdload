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

#ifndef LCODES_H
#define LCODES_H

#pragma once

#include <string>
#include <map>
#include <list>
#include "LCodeHandler.h"


class CLCode
{
public:
  CLCode();
  ~CLCode();
  void Init(std::string strURL, std::string strProjectname, const std::string& strBaseLCode);
  std::string GetLangCodeFromAlias(std::string Alias, std::string AliasForm, std::string strProjectname);
  std::string GetLangFromLCode(std::string LangCode, std::string AliasForm, std::string strProjectname);
  std::string VerifyLangCode(std::string LangCode, const std::string &strLangformat, std::string strProjectname);
  void CleanLangform (std::string &strLangform);
private:
  std::map <std::string, CLCodeHandler> m_mapLCodeHandlers;
  std::map <std::string, CLCodeHandler>::iterator itmapLCodeHandlers;
};

extern CLCode g_LCode;
#endif
