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

#ifndef CHARSETUTILS_H
#define CHARSETUTILS_H

#pragma once

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


class CCharsetUtils
{
public:
  std::string GetRoot(const std::string &strPath,const std::string &strFilename);
  std::string GetRootDir(std::string strPath);
  std::string GetLangnameFromURL(std::string strName, std::string strURL, std::string strLangformat);
  std::string GetFilenameFromURL(const std::string& strURL);
  std::string ReplaceLanginURL(const std::string &strURL,const std::string &strLangFormat, const std::string &strLCode, const std::string& strProjectName);
  bool replaceAllStrParts(std::string * pstr, const std::string& from, const std::string& to);
  std::string replaceStrParts(std::string strToReplace, const std::string& from, const std::string& to);
  bool bISPOFile(const std::string strFilename);
  bool bISXMLFile(const std::string strFilename);
};

extern CCharsetUtils g_CharsetUtils;

#endif