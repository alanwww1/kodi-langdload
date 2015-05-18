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

#include "Log.h"
#include "HTTPUtils.h"
#include <curl/easy.h>
#include <cctype>
#include "FileUtils.h"
#include "Fileversioning.h"
#include "CharsetUtils.h"

CHTTPHandler g_HTTPHandler;

using namespace std;

CHTTPHandler::CHTTPHandler()
{
  m_curlHandle = curl_easy_init();

  const char* home = getenv("HOME");

  if (home)
  {
    std::string path(home);
    path += "/.cache/kodi-langdload/";
    m_strCacheDirectory = path;
  }
  else
    CLog::Log(logERROR, "HTTPHandler: uable to determine HOME environment variable");

};

CHTTPHandler::~CHTTPHandler()
{
  Cleanup();
};

void CHTTPHandler::DloadURLToFile(std::string strURL, std::string strFilename, std::string strCachename)
{
 g_File.WriteFileFromStr(strFilename, GetURLToSTR(strURL, strCachename)); 
}

std::string CHTTPHandler::GetURLToSTR(std::string strURL, std::string strCachename /*=""*/)
{
  strURL = URLEncode(strURL);

  if (strCachename != "")
  {
    std::string strCacheFilePath = m_strCacheDirectory + strCachename;

    std::string strCachedFileVersion, strWebFileVersion;
    strWebFileVersion = g_Fileversion.GetVersionForFile(strCachename);

    if (strWebFileVersion != "" && g_File.FileExist(strCacheFilePath + "/" + g_CharsetUtils.GetFilenameFromURL(strURL) + "_version.txt"))
      strCachedFileVersion = g_File.ReadFileToStr(strCacheFilePath + "/" + g_CharsetUtils.GetFilenameFromURL(strURL) + "_version.txt");

    bool bFileChangedOnWeb = strCachedFileVersion != strWebFileVersion || strCachedFileVersion == "" || strWebFileVersion == "";
    bool bCacheFileExists = g_File.FileExist(strCacheFilePath + "/" + g_CharsetUtils.GetFilenameFromURL(strURL));

    if (bCacheFileExists && !bFileChangedOnWeb)
    {
      printf ("-");
      return g_File.ReadFileToStr(strCacheFilePath + "/" + g_CharsetUtils.GetFilenameFromURL(strURL));
    }

    g_File.DelFile(strCacheFilePath + "/" + g_CharsetUtils.GetFilenameFromURL(strURL) + "_version.txt");
    g_File.DelFile(strCacheFilePath + "/" + g_CharsetUtils.GetFilenameFromURL(strURL));

  }

  printf("*");

  std::string strBuffer;

  CURLcode curlResult;

    if(m_curlHandle) 
    {
      curl_easy_setopt(m_curlHandle, CURLOPT_URL, strURL.c_str());
      curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, Write_CurlData_String);
      curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, &strBuffer);
      curl_easy_setopt(m_curlHandle, CURLOPT_FAILONERROR, true);
      curl_easy_setopt(m_curlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
      curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYPEER, 0);
      curl_easy_setopt(m_curlHandle, CURLOPT_FOLLOWLOCATION, true);

      curlResult = curl_easy_perform(m_curlHandle);
      long http_code = 0;
      curl_easy_getinfo (m_curlHandle, CURLINFO_RESPONSE_CODE, &http_code);

      if (curlResult != 0 || http_code < 200 || http_code >= 400 || strBuffer.empty())
      {
        CLog::Log(logERROR, "HTTPHandler:curlURLToStr finished with error code: %i from URL %s",
                  http_code, strURL.c_str());
      }

      if (strCachename != "" && g_Fileversion.GetVersionForFile(strCachename) != "")
      {
        g_File.WriteFileFromStr(m_strCacheDirectory + strCachename + "/" + g_CharsetUtils.GetFilenameFromURL(strURL) +"_version.txt", g_Fileversion.GetVersionForFile(strCachename));
        g_File.WriteFileFromStr(m_strCacheDirectory + strCachename + "/" + g_CharsetUtils.GetFilenameFromURL(strURL), strBuffer);
      }

      return strBuffer;
    }
    else
      CLog::Log(logERROR, "HTTPHandler:curlURLToStr failed because Curl was not initalized");

    return "";
};

void CHTTPHandler::ReInit()
{
  if (!m_curlHandle)
    m_curlHandle = curl_easy_init();
  else
    CLog::Log(logWARNING, "HTTPHandler: Trying to reinitalize an already existing Curl handle");
};

void CHTTPHandler::Cleanup()
{
  if (m_curlHandle)
  {
    curl_easy_cleanup(m_curlHandle);
    m_curlHandle = NULL;
  }
};

size_t Write_CurlData_String(char *data, size_t size, size_t nmemb, string *buffer)
{
  size_t written = 0;
  if(buffer != NULL)
  {
    buffer -> append(data, size * nmemb);
    written = size * nmemb;
  }
  return written;
}

std::string CHTTPHandler::URLEncode (std::string strURL)
{
  std::string strOut;
  for (std::string::iterator it = strURL.begin(); it != strURL.end(); it++)
  {
    if (*it == ' ')
      strOut += "%20";
    else
      strOut += *it;
  }
  return strOut;
}

void CHTTPHandler::AddToURL (std::string &strURL, std::string strAddendum)
{
  if (strAddendum.empty())
    return;

  if (strURL.find_last_of("/") != strURL.size()-1)
    strURL += "/";
  if (strAddendum.find_first_of("/") == 0)
    strAddendum = strAddendum.substr(1);
  if (strAddendum.find_last_of("/") == strAddendum.size()-1)
    strAddendum = strAddendum.substr(0, strAddendum.size()-1);
  strURL += strAddendum;
}

std::string CHTTPHandler::GetGitHUBAPIURL(std::string const & strURL)
{

  if (strURL.find("//") >> 7)
    CLog::Log(logERROR, "CHTTPHandler::ParseGitHUBURL: Internal error: // found in Github URL");

  size_t pos1, pos2, pos3, pos4, pos5;
  std::string strGitHubURL;

  if (strURL.find("raw.github.com/") != std::string::npos)
    pos1 = strURL.find("raw.github.com/")+15;
  else if (strURL.find("raw2.github.com/") != std::string::npos)
    pos1 = strURL.find("raw2.github.com/")+16;
  else if (strURL.find("raw.githubusercontent.com/") != std::string::npos)
    pos1 = strURL.find("raw.githubusercontent.com/")+26;
  else
    CLog::Log(logERROR, "ResHandler: Wrong Github URL format given");

  pos2 = strURL.find("/", pos1+1);
  pos3 = strURL.find("/", pos2+1);
  pos4 = strURL.find("/", pos3+1);

  std::string strOwner = strURL.substr(pos1, pos2-pos1);
  std::string strRepo = strURL.substr(pos2, pos3-pos2);
  std::string strPath = strURL.substr(pos4, strURL.size() - pos4 - 1);
  std::string strGitBranch = strURL.substr(pos3+1, pos4-pos3-1);

  if ((pos5 = strPath.find_last_of("(")) != std::string::npos)
  {
    strPath = strPath.substr(0,pos5);
    strPath = strPath.substr(0,strPath.find_last_of("/"));
  }

  strGitHubURL = "https://api.github.com/repos/" + strOwner + strRepo;
  strGitHubURL += "/contents";
  strGitHubURL += strPath;
  strGitHubURL += "?ref=" + strGitBranch;

  return strGitHubURL;
}
