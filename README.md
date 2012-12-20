xbmc-langdload
==============

Downloader utility to help pulling XBMC translations from XBMC translations github repo to local repos.

Usage:

1.Easy mode

  Usage: xbmc-langdload PROJECTID/ADDONID LOCALDIR
  PROJECTID: The id of the project defined on the xbmc repo. eg. xbmc-main-frodo
  ADDONID: The id of the addon which is defined in the "id" tag in the addon.xml file
  LOCALDIR: The local directory to copy the files to. This is where the addon.xml file gets.

  Example: xbmc-langdload xbmc-addons/plugin.video.coolplugin /home/myname/somedir/

2.Batch mode with xml file usage

  Usage: xbmc-langdload XMLFILE
  XMLFILE: The path and filename of the input XML file which holds the download data (check README for format)

  Example: xbmc-langdload ./xbmc-langdload.xml

Note for Windows users: In case you have whitespace or any special character
in the directory/file argument, please use apostrophe around them. For example:
xbmc-langdload.exe xbmc-skins/skin.essence "C:\somedir\"
