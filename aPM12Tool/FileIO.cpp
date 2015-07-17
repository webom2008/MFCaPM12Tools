/* 
 * Copyright (c) 2013,CVTE.ҽ�Ʋ�Ʒ��ҵ��
 * All rights reserved. 
 * 
 *�ļ����ƣ�
 *�ļ���ʶ��
 *ժ   Ҫ��
 * 
 *��ǰ�汾�� 1.0
 *��   �ߣ���ΰ��
 *������ڣ�2013��7��1�� 
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
 * �������ܣ� 
 * ��������� StringArray,���ļ��������뵽�Ǹ�������
              lpszExpression,Ҫ�������ļ���,����ѡ��ͨ�����
                ������"*.jpg"��������������е��ļ���
                ��Ҫ����·��,������Ҫ�ĸ�ʽ������������ʽ��"d:\\fp\\temp.bmp"��
 * ��������� 
 * �� �� ֵ�� 
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
 * �������ܣ� ���������Ŀ�ľ���Ϊ�˴�һ���ļ��еĶԻ�������ѡ��
              ����ĺ�������������������ʹ�õĻ���
              ��Ҫע�����"\",��Ϊ��Ҫ��һ����
              
 * ��������� 
 * ��������� 
 * �� �� ֵ�� ���յõ���str�����Ǹ�·��������
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
 * �������ܣ� ���Ǵ������飬����û����Щ·��������ֻ���ļ�����
              ��Ҫ����������������е��ļ��У�ֻ�ǵ�ǰ�ļ��������ǡ�
 * ��������� lpszExpression,����һ��·��
 * ��������� StringArray,�õ���·����������е��ļ���
 * �� �� ֵ�� 
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
// * �������ܣ� ������������ʵ�ʵ�ʵ���˶�ȡһ��Ŀ¼�������е��ļ�����
//              ���ұ�����һ��CString�����У����һ�����ϸ���ļ�·��
// * ��������� strArray�Ǳ�ʾҪ��ŵ�����
//              strDir����Ҫ���ҵ��Ǹ�·��
//              strExt�ǲ��ҵ��ļ���
//              ������ͨ�����ʵ�ֲ���
// * ��������� 
// * �� �� ֵ�� 
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
