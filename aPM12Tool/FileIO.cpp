/* 
 * Copyright (c) 2013,CVTE.医疗产品事业部
 * All rights reserved. 
 * 
 *文件名称：
 *文件标识：
 *摘   要：
 * 
 *当前版本： 1.0
 *作   者：邱伟波
 *完成日期：2013年7月1日 
 */
#include "stdafx.h"
#include "FileIO.h"


CFileIO::CFileIO(void)
{
}


CFileIO::~CFileIO(void)
{
}



/* 
 * 函数介绍： 
 * 输入参数： StringArray,把文件名都存入到那个数组中
              lpszExpression,要搜索的文件名,可以选择通配符，
                比如是"*.jpg"来搜索下面的所有的文件名
                需要绝对路径,所以需要的格式是类似这种形式："d:\\fp\\temp.bmp"。
 * 输出参数： 
 * 返 回 值： 
 */  
void CFileIO::GetFolderNamesInDir(CStringArray &StringArray, LPCTSTR lpszExpression)
{
    CFileFind FileFind;
    CString str = lpszExpression;
    str += "\\*.*";
    if(!FileFind.FindFile(str))
        return;
    BOOL bFound;
    do{
        bFound = FileFind.FindNextFile();
        if(FileFind.IsDirectory())
        {
            CString strFileName = FileFind.GetFileName();
            if(strFileName != "." && strFileName != "..")
                StringArray.Add(FileFind.GetFileName());
        }
    }while(bFound);
}


/* 
 * 函数介绍： 这个函数的目的就是为了打开一个文件夹的对话框让你选择。
              上面的函数如果和下面这个配套使用的话，
              需要注意加如"\",因为需要多一个。
              
 * 输入参数： 
 * 输出参数： 
 * 返 回 值： 最终得到的str就是那个路径的名字
 */
CString CFileIO::SelectDirectory(LPCTSTR lpszTitle)
{
    static TCHAR strDirName[MAX_PATH];
    BROWSEINFO bi;
    bi.hwndOwner = ::GetFocus();
    bi.pidlRoot = NULL;
    bi.pszDisplayName = strDirName;
    bi.lpszTitle = lpszTitle;
    bi.ulFlags = BIF_BROWSEFORCOMPUTER | BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
    bi.lpfn = NULL;
    bi.lParam = 0;
    bi.iImage = 0;
    LPITEMIDLIST pItemIDList = ::SHBrowseForFolder(&bi);
    if(pItemIDList == NULL)
    {
        return "";
    }
    ::SHGetPathFromIDList(pItemIDList, strDirName);
    CString str = strDirName;
    if(str != "" && str.Right(1) != '\\')
        str += '\\';
    return str;
}

/* 
 * 函数介绍： 还是存入数组，不过没有那些路径名，而只是文件名。
              还要不会搜索下面的所有的文件夹，只是当前文件夹下面是。
 * 输入参数： lpszExpression,传入一个路径
 * 输出参数： StringArray,得到该路径下面的所有的文件名
 * 返 回 值： 
 */
void CFileIO::GetFileNamesInDir(CStringArray &StringArray, LPCTSTR lpszExpression)
{
    // The File Name should apply with "d:\\fp\\temp.bmp" Style
    CFileFind FileFind;
    if(!FileFind.FindFile(lpszExpression))
        return;
    BOOL bFound;
    do{
        bFound = FileFind.FindNextFile();
        if(!FileFind.IsDirectory())
            StringArray.Add(FileFind.GetFileName());
    }while(bFound);
}
//
///* 
// * 函数介绍： 下面的这个就是实际的实现了读取一个目录下面所有的文件名，
//              并且保存在一个CString数组中，而且还是详细的文件路径
// * 输入参数： strArray是表示要存放的数组
//              strDir是你要查找的那个路径
//              strExt是查找的文件名
//              可以用通配符来实现查找
// * 输出参数： 
// * 返 回 值： 
// */
//void CFileIO::GetAllFilePathInDir( CStringArray &strArray, CString strDir, CString strExt )
//{
//    CFileFind finder;
//    CString strFileName;
//    if( strDir.IsEmpty() )
//        return;
//    if( strDir.Right(2) != "\\" )
//        strDir = strDir + "\\";
//    BOOL bWorking = finder.FindFile( strDir + "*.*" );
//    while( bWorking )
//    { 
//        bWorking = finder.FindNextFile();
//
//        if ( finder.IsDots() )
//            continue;
//
//        if ( finder.IsDirectory() )
//        {
//            GetAllFilePathInDir( strArray, finder.GetFilePath(), strExt );
//            continue;
//        }
//        strFileName = finder.GetFilePath();
//        strFileName.MakeLower();
//        CString needExt = GetFileExt( strExt );
//        CString srcExt = GetFileExt( strFileName );
//        if( strExt == "*.*" || needExt == srcExt )
//        {
//            strArray.Add( strFileName );
//        }
//    } 
//    finder.Close();
//}
