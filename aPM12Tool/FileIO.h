#pragma once

class CFileIO
{
public:
    CFileIO(void);
    ~CFileIO(void);
public:
    static void        GetFolderNamesInDir(CStringArray &StringArray, LPCTSTR lpszExpression);
    static CString     SelectDirectory(LPCTSTR lpszTitle);
    static void        GetFileNamesInDir(CStringArray &StringArray, LPCTSTR lpszExpression);
    //static void GetAllFilePathInDir( CStringArray &strArray, CString strDir, CString strExt );
};

