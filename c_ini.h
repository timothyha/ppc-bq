#if !defined(__C_INI__)
#define __C_INI__

#define MAX_NAME 32
#define MAX_VALUE 256
#define DEF_BUFSIZE 1024
#define DEF_STARTSIZE 1

class CIni
{
public:
	CIni();
	~CIni();

	BOOL LoadIniFile ( TCHAR* );

	int GetSectionIndexByName ( char* );
	int GetSectionIndexByName ( LPWSTR );

	int GetParamIndexByName ( char*, char* );
	int GetParamIndexByName ( int, char* );
	int GetParamIndexByName ( int, LPWSTR );
	int GetParamIndexByName ( LPWSTR, LPWSTR );

	int GetSectionCount ();

	int GetParamCount ( int );
	int GetParamCount ( char* );
	int GetParamCount ( LPWSTR );

	int GetSectionNameCount ( int );
	int GetParamValueCount ( int, int );
	int GetParamNameCount ( int, int );

	char* GetSectionName ( int );
	int GetSectionName ( int, LPWSTR, int );

	char* GetParamName ( int, int );
	char* GetParamName ( char*, int );
	int GetParamName ( int, int, LPWSTR, int );
	int GetParamName ( char*, int, LPWSTR, int );
	int GetParamName ( LPWSTR, int, LPWSTR, int );

	int GetParamValue ( int, int, LPWSTR, int );
	char* GetParamValue ( int, int );
	char* GetParamValue ( char*, char* );
	char* GetParamValue ( char*, int );
	char* GetParamValue ( int, char* );
	int GetParamValue ( LPWSTR, LPWSTR, LPWSTR, int );
	int GetParamValue ( char*, char*, LPWSTR, int );

private:
	typedef struct _CINI_PARAM
	{
		char szName[MAX_NAME];
		char szValue[MAX_VALUE];
	} CINI_PARAM, *LPCINI_PARAMS;

	typedef struct _CINI_SECTION
	{
		char szName[MAX_NAME];
		int nCount;
		LPCINI_PARAMS parameters;
	} CINI_SECTION, *LPCINI_SECTIONS;

	typedef struct _CINI_
	{
		int nCount;
		LPCINI_SECTIONS sections;
	} CINI, *LPCINI;

private:
	LPCINI ini;
	BOOL bError;

/******************************************
 Parser
*******************************************/
private:
	BOOL CreateNullSections ( LPCINI* );
	BOOL FreeSections ( LPCINI* );
	BOOL ParseBuffer ( LPCINI*, char*, DWORD );
	void AddCharTo ( char*, char, int );
	BOOL AddSection ( LPCINI*, char* );
	BOOL AddParam ( LPCINI*, char*, char*, char* );

	int GetSectionIndexByName ( LPCINI*, char* name );
	int GetParamIndexByName ( LPCINI*, char*, char* );
	int GetParamIndexByName ( LPCINI*, int, char* );

private:
	char szSection[MAX_NAME];
	char szParam[MAX_NAME];
	char szValue[MAX_VALUE];
	char c_case;
	int n_i;

};

#endif