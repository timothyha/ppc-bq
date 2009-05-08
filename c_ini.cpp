#include <windows.h>

#include "c_ini.h"

CIni::CIni()
{
	this->bError = FALSE;
	this->ini = NULL;
	if ( !this->CreateNullSections ( &(this->ini) ) )
		this->bError = TRUE;
}

CIni::~CIni()
{
	if ( ini != NULL && !bError )
		this->FreeSections ( &(this->ini) );
}

BOOL CIni::LoadIniFile ( TCHAR* szFilePath )
{
	HANDLE hFile = NULL;
	DWORD fSize = 0, offset = 0, dw = 0, bufSize = 0;
	BOOL bRet = FALSE;
	char szBuff[DEF_BUFSIZE];
	LPCINI tempIni;

	if ( bError )
		return FALSE;
	
	c_case = 0;

	if ( !this->CreateNullSections ( &tempIni ) )
		return FALSE;

	hFile = CreateFile ( szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( ( hFile == INVALID_HANDLE_VALUE ) || ( hFile == NULL ) )
		return FALSE;

	fSize = GetFileSize ( hFile, NULL );

	if ( fSize < 3 )
		goto EndLoadFile;

	while ( offset < fSize )
	{
		bufSize = fSize - offset;
		if ( bufSize > DEF_BUFSIZE ) bufSize = DEF_BUFSIZE;
		SetFilePointer ( hFile, offset, NULL, FILE_BEGIN );
		ReadFile ( hFile, szBuff, bufSize, &dw, NULL );
		if ( dw != bufSize )
			goto EndLoadFile;
		if ( !this->ParseBuffer ( &tempIni, szBuff, bufSize ) )
			goto EndLoadFile;
		offset += dw;
	}

	bRet = TRUE;

EndLoadFile:
	CloseHandle ( hFile );
	if ( !bRet )
	{
		this->FreeSections ( &tempIni );
	}else{
		this->FreeSections ( &ini );
		this->ini = tempIni;
	}
	return bRet;
}

BOOL CIni::CreateNullSections ( LPCINI* _ini )
{
	LPCINI s;
	s = (LPCINI) malloc ( sizeof(CINI) );
	if ( s == NULL )
		return FALSE;
	s->nCount = 0;
	s->sections = NULL;
	*_ini = s;
	return TRUE;
}

BOOL CIni::FreeSections ( LPCINI* _ini )
{
	int i;
	if ( _ini == NULL )
		return FALSE;

	for ( i = 0 ; i < (*_ini)->nCount ; i ++ )
	{
		free ( (*_ini)->sections[i].parameters );
	}
	free ( (*_ini)->sections );
	free ( *_ini );

	*_ini = NULL;

	return TRUE;
}

BOOL CIni::ParseBuffer ( LPCINI* _ini, char* buff, DWORD size )
{
	DWORD i;
	BOOL bRet = FALSE;
	for ( i = 0 ; i < size ; i ++ )
	{
		switch ( buff[i] )
		{
			case '[':
				c_case = 1;
				n_i = 0;
				memset ( szSection, 0, MAX_NAME );
				break;

			case '\n':
				if ( c_case == 2 )
				{
					if ( ! this->AddSection ( _ini, szSection ) )
						goto cancelparse;
				}
				if ( c_case >= 3 )
				{
					if ( ! this->AddParam ( _ini, szSection, szParam, szValue ) )
						goto cancelparse;
				}
			case ']':
				c_case = 2;
				n_i = 0;
				memset ( szParam, 0, MAX_NAME );
				memset ( szValue, 0, MAX_VALUE );
				break;

			case '=':
				c_case = 3;
				n_i = 0;
				break;

			case ';':
				c_case = 4;
				break;

			case '\r':
				// skipped
				break;

			default:
				switch ( c_case )
				{
					case 1:
						this->AddCharTo ( szSection, buff[i], MAX_NAME );
						break;

					case 2:
						this->AddCharTo ( szParam, buff[i], MAX_NAME );
						break;

					case 3:
						this->AddCharTo ( szValue, buff[i], MAX_VALUE );
						break;
				}
				break;
		}
	}
	if ( c_case >= 3 )
	{
		if ( ! this->AddParam ( _ini, szSection, szParam, szValue ) )
			goto cancelparse;
	}
	bRet = TRUE;

cancelparse:
	return bRet;
}

void CIni::AddCharTo ( char* param, char add, int max )
{
	if ( n_i < max )
		param[n_i] = add;
	n_i ++;
}

BOOL CIni::AddSection ( LPCINI* _ini, char* name )
{
	LPCINI_SECTIONS sects = NULL;

	if ( this->GetSectionIndexByName ( _ini, name ) != -1 )
		return TRUE;

	sects = (LPCINI_SECTIONS) malloc ( sizeof ( CINI_SECTION ) * ( (*_ini)->nCount + 1 ) );
	if ( sects == NULL )
		return FALSE;

	if ( (*_ini)->nCount > 0 )
	{
		memcpy ( sects, (*_ini)->sections, sizeof ( CINI_SECTION ) * ( (*_ini)->nCount ) );

		free ( (*_ini)->sections );
	}

	sects[(*_ini)->nCount].parameters = NULL;
	sects[(*_ini)->nCount].nCount = 0;
	strcpy ( sects[(*_ini)->nCount].szName, name );

	(*_ini)->sections = sects;

	(*_ini)->nCount ++;

	return TRUE;
}

int CIni::GetSectionIndexByName ( LPCINI* _ini, char* name )
{
	int i;
	if ( _ini == NULL || name == NULL )
		return -1;
	for ( i = 0 ; i < (*_ini)->nCount ; i ++ )
	{
		if ( !strcmp ( name, (*_ini)->sections[i].szName ) )
			return i;
	}
	return -1;
}

int CIni::GetSectionIndexByName ( char* name )
{
	if ( this->bError )
		return -1;
	return this->GetSectionIndexByName ( &(this->ini), name );
}

int CIni::GetSectionIndexByName ( LPWSTR name )
{
	char* sname = NULL;
	int iname = 0, i_ret = 0;

	if ( name == NULL )
		return -1;

	if ( this->bError )
		return -1;

	iname = wcslen ( name ) + 1;
	sname = (char*)malloc ( iname );
	if ( sname == NULL )
		return -1;
	memset ( sname, 0, iname );

	if ( WideCharToMultiByte ( CP_ACP, 0, name, -1, sname, iname, NULL, NULL ) == 0 )
		goto returning;

	i_ret = this->GetSectionIndexByName ( sname );

returning:
	if ( sname != NULL ) free ( sname );
	return i_ret;
}

int CIni::GetParamIndexByName ( LPCINI* _ini, int i_ret, char* name )
{
	int i;
	if ( _ini == NULL || name == NULL )
		return -1;
	for ( i = 0 ; i < (*_ini)->sections[i_ret].nCount ; i ++ )
	{
		if ( ! strcmp ( (*_ini)->sections[i_ret].parameters[i].szName, name ) )
			return i;
	}
	return -1;
}

int CIni::GetParamIndexByName ( LPCINI* _ini, char* section, char* name )
{
	int i_ret = -1;
	if ( ( i_ret = this->GetSectionIndexByName ( _ini, section ) ) == -1 )
		return -1;
	return this->GetParamIndexByName ( _ini, i_ret, name );
}

int CIni::GetParamIndexByName ( char* section, char* name )
{
	if ( this->bError )
		return -1;
	return this->GetParamIndexByName ( &(this->ini), section, name );
}

int CIni::GetParamIndexByName ( int i_sec, char* name )
{
	if ( this->bError )
		return -1;
	if ( i_sec < this->ini->nCount )
		return this->GetParamIndexByName ( &(this->ini), i_sec, name );
	return -1;
}

int CIni::GetParamIndexByName ( int i_sec, LPWSTR name )
{
	char* sname = NULL;
	int iname = 0, i_ret = -1;

	if ( name == NULL )
		return -1;

	if ( this->bError )
		return -1;

	if ( i_sec >= this->ini->nCount )
		return -1;

	iname = wcslen ( name ) + 1;
	sname = (char*) malloc ( iname );
	if ( sname == NULL )
		return -1;
	memset ( sname, 0, iname );

	if ( WideCharToMultiByte ( CP_ACP, 0, name, -1, sname, iname, NULL, NULL ) == 0 )
		goto returning;

	i_ret = this->GetParamIndexByName ( i_sec, sname );

returning:
	if ( sname != NULL ) free ( sname );
	return i_ret;
}

int CIni::GetParamIndexByName ( LPWSTR sec, LPWSTR name )
{
	char* ssec = NULL;
	int isec = 0, i_ret = -1;

	if ( sec == NULL || name == NULL )
		return -1;

	if ( this->bError )
		return -1;

	isec = wcslen ( sec ) + 1;
	ssec = (char*) malloc ( isec );
	if ( ssec == NULL )
		return -1;
	memset ( ssec, 0, isec );

	if ( WideCharToMultiByte ( CP_ACP, 0, sec, -1, ssec, isec, NULL, NULL ) == 0 )
		goto returning;

	isec = this->GetSectionIndexByName ( ssec );
	if ( isec == -1 )
		goto returning;

	i_ret = this->GetParamIndexByName ( isec, name );

returning:
	if ( ssec != NULL ) free ( ssec );
	return i_ret;
}

BOOL CIni::AddParam ( LPCINI* _ini, char* section, char* name, char* value )
{
	int i_ret = -1;
	int i_par = -1;
	LPCINI_PARAMS params = NULL;

	if ( _ini == NULL || section == NULL || name == NULL || value == NULL )
		return FALSE;

	i_ret = this->GetSectionIndexByName ( _ini, section );
	if ( i_ret == -1 )
		return FALSE;

	if ( ( i_par = this->GetParamIndexByName ( _ini, i_ret, name ) ) == -1 )
	{
		params = (LPCINI_PARAMS) malloc ( sizeof ( CINI_PARAM ) * ( (*_ini)->sections[i_ret].nCount + 1 ) );
		if ( params == NULL )
			return FALSE;

		if ( (*_ini)->sections[i_ret].nCount > 0 )
		{
			memcpy ( params, (*_ini)->sections[i_ret].parameters, sizeof ( CINI_PARAM ) * ( (*_ini)->sections[i_ret].nCount ) );

			free ( (*_ini)->sections[i_ret].parameters );
		}

		strcpy ( params[(*_ini)->sections[i_ret].nCount].szValue, value );
		strcpy ( params[(*_ini)->sections[i_ret].nCount].szName, name );
		(*_ini)->sections[i_ret].nCount ++;
		(*_ini)->sections[i_ret].parameters = params;
	}else{
		strcpy ( (*_ini)->sections[i_ret].parameters[i_par].szValue, value );
	}
	return TRUE;
}

/***************************************
 Gettings...
***************************************/

//***************** Sections

int CIni::GetSectionCount (void)
{
	if ( this->bError )
		return -1;
	return this->ini->nCount;
}

int CIni::GetSectionNameCount ( int ind )
{
	if ( this->bError )
		return -1;
	if ( ind >= this->ini->nCount )
		return -1;
	return strlen ( this->ini->sections[ind].szName );
}

char* CIni::GetSectionName ( int ind )
{
	if ( this->bError )
		return NULL;
	if ( ind >= this->ini->nCount )
		return NULL;
	return this->ini->sections[ind].szName;
}

int CIni::GetSectionName ( int ind, LPWSTR lpRet, int size )
{
	int len = -1;
	if ( lpRet == NULL || size == 0 )
		return -1;
	len = this->GetSectionNameCount ( ind );
	if ( len == -1 )
		return -1;
	if ( size < len )
		return -1;
	if ( MultiByteToWideChar ( CP_ACP, 0, this->ini->sections[ind].szName, -1, lpRet, size ) == 0 )
		return -1;
	return len;
}

//*************************************

//*********************** Param Name
int CIni::GetParamCount ( int i_sec )
{
	if ( this->bError )
		return -1;
	if ( i_sec < this->ini->nCount )
		return this->ini->sections[i_sec].nCount;
	return -1;
}

int CIni::GetParamCount ( char* sec )
{
	int i_sec;
	if ( this->bError )
		return -1;
	if ( ( i_sec = this->GetSectionIndexByName ( &(this->ini), sec ) ) != -1 )
		return this->ini->sections[i_sec].nCount;
	return -1;
}

int CIni::GetParamCount ( LPWSTR sec )
{
	char* ssec = NULL;
	int isec = -1, i_ret = -1;

	if ( sec == NULL )
		return -1;

	if ( this->bError )
		return -1;

	isec = wcslen ( sec ) + 1;
	ssec = (char*) malloc ( isec );
	if ( ssec == NULL )
		return -1;
	memset ( ssec, 0, isec );

	if ( WideCharToMultiByte ( CP_ACP, 0, sec, -1, ssec, isec, NULL, NULL ) == 0 )
		goto returning;

	if ( ( isec = this->GetSectionIndexByName ( &(this->ini), ssec ) ) != -1 )
		i_ret = this->ini->sections[isec].nCount;

returning:
	if ( ssec != NULL ) free ( ssec );
	return i_ret;
}

int CIni::GetParamNameCount ( int i_sec, int i_par )
{
	if ( this->bError )
		return -1;
	if ( i_sec >= this->ini->nCount )
		return -1;
	if ( i_par >= this->ini->sections[i_sec].nCount )
		return -1;
	return strlen ( this->ini->sections[i_sec].parameters[i_par].szName );
}

char* CIni::GetParamName ( int i_sec, int i_par )
{
	if ( this->bError )
		return NULL;
	if ( i_sec >= this->ini->nCount )
		return NULL;
	if ( i_par >= this->ini->sections[i_sec].nCount )
		return NULL;
	return this->ini->sections[i_sec].parameters[i_par].szName;
}

char* CIni::GetParamName ( char* sec, int i_par )
{
	int i_sec = -1;
	if ( this->bError )
		return NULL;

	i_sec = this->GetSectionIndexByName ( sec );
	if ( i_sec == -1 )
		return NULL;

	if ( i_par >= this->ini->sections[i_sec].nCount )
		return NULL;
	return this->ini->sections[i_sec].parameters[i_par].szName;
}

int CIni::GetParamName ( int i_sec, int i_par, LPWSTR lpRet, int size )
{
	int len = -1;
	if ( lpRet == NULL || size == 0 )
		return -1;
	len = this->GetParamNameCount ( i_sec, i_par );
	if ( len == -1 )
		return -1;
	if ( size < len )
		return -1;
	if ( MultiByteToWideChar ( CP_ACP, 0, this->ini->sections[i_sec].parameters[i_par].szName, -1, lpRet, size ) == 0 )
		return -1;
	return len;
}

int CIni::GetParamName ( char* sec, int i_par, LPWSTR lpRet, int size )
{
	int len = -1, i_sec = -1;

	if ( lpRet == NULL || size == NULL )
		return -1;

	i_sec = this->GetSectionIndexByName ( sec );
	if ( i_sec == -1 )
		return -1;

	len = this->GetParamNameCount ( i_sec, i_par );
	if ( len == -1 )
		return -1;
	if ( size < len )
		return -1;
	if ( MultiByteToWideChar ( CP_ACP, 0, this->ini->sections[i_sec].parameters[i_par].szName, -1, lpRet, size ) == 0 )
		return -1;
	return len;
}

int CIni::GetParamName ( LPWSTR sec, int i_par, LPWSTR lpRet, int size )
{
	int isec = 0, i_ret = -1;
	char* ssec = NULL;

	if ( sec == NULL || lpRet == NULL || size == 0 ) return -1;

	isec = wcslen ( sec ) + 1;
	ssec = (char*) malloc ( isec );
	if ( ssec == NULL )
		return -1;
	memset ( ssec, 0, isec );

	if ( WideCharToMultiByte ( CP_ACP, 0, sec, -1, ssec, isec, NULL, NULL ) ==0 )
		goto returning;

	i_ret = this->GetParamName ( ssec, i_par, lpRet, size );

returning:
	if ( ssec != NULL ) free ( ssec );
	return i_ret;
}

//****************************************************

//************************************* Param Value

int CIni::GetParamValueCount ( int i_sec, int i_par )
{
	if ( this->bError )
		return -1;
	if ( i_sec >= this->ini->nCount )
		return -1;
	if ( i_par >= this->ini->sections[i_sec].nCount )
		return -1;
	return strlen ( this->ini->sections[i_sec].parameters[i_par].szValue );
}

char* CIni::GetParamValue ( int i_sec, int i_par )
{
	if ( this->bError )
		return NULL;
	if ( i_sec >= this->ini->nCount )
		return NULL;
	if ( i_par >= this->ini->sections[i_sec].nCount )
		return NULL;
	return this->ini->sections[i_sec].parameters[i_par].szValue;
}

int CIni::GetParamValue ( int i_sec, int i_par, LPWSTR lpRet, int size )
{
	int len = -1;
	if ( lpRet == NULL || size == 0 )
		return -1;
	len = this->GetParamValueCount ( i_sec, i_par );
	if ( len == -1 )
		return -1;
	if ( size < len )
		return -1;
	if ( MultiByteToWideChar ( CP_ACP, 0, this->ini->sections[i_sec].parameters[i_par].szValue, -1, lpRet, size ) == 0 )
		return -1;
	return len;
}

char* CIni::GetParamValue ( char* section, char* name )
{
	int i_sec = -1;
	int i_par = -1;
	i_sec = this->GetSectionIndexByName ( section );
	if ( i_sec == -1 )
		return NULL;
	i_par = this->GetParamIndexByName ( i_sec, name );
	if ( i_par == -1 )
		return NULL;
	return this->ini->sections[i_sec].parameters[i_par].szValue;
}

char* CIni::GetParamValue ( char* section, int i_par )
{
	int i_sec = -1;
	i_sec = this->GetSectionIndexByName ( section );
	if ( i_sec == -1 )
		return NULL;
	if ( i_par >= this->ini->sections[i_sec].nCount )
		return NULL;
	return this->ini->sections[i_sec].parameters[i_par].szValue;
}

char* CIni::GetParamValue ( int i_sec, char* name )
{
	int i_par = -1;
	if ( i_sec >= this->ini->nCount )
		return NULL;
	i_par = this->GetParamIndexByName ( i_sec, name );
	if ( i_par == -1 )
		return NULL;
	return this->ini->sections[i_sec].parameters[i_par].szValue;
}

int CIni::GetParamValue ( LPWSTR sec, LPWSTR par, LPWSTR lpRet, int size )
{
	int i_sec = -1, i_par = -1;

	if ( sec == NULL || par == NULL || lpRet == NULL || size == 0 )
		return -1;

	i_sec = this->GetSectionIndexByName ( sec );
	if ( i_sec == -1 )
		return -1;
	i_par = this->GetParamIndexByName ( i_sec, par );
	if ( i_par == -1 )
		return -1;

	return this->GetParamValue ( i_sec, i_par, lpRet, size );
}

int CIni::GetParamValue ( char* sec, char* par, LPWSTR lpRet, int size )
{
	int i_sec = -1, i_par = -1, len = 0;
	if ( lpRet == NULL || size == 0 )
		return -1;
	i_sec = this->GetSectionIndexByName ( sec );
	if ( i_sec == -1 )
		return -1;
	i_par = this->GetParamIndexByName ( i_sec, par );
	if ( i_par >= this->ini->sections[i_sec].nCount )
		return -1;


	len = strlen (this->ini->sections[i_sec].parameters[i_par].szValue);
	if ( size < len )
		return -1;
	if ( MultiByteToWideChar ( CP_ACP, 0, this->ini->sections[i_sec].parameters[i_par].szValue, -1, lpRet, size ) == 0 )
		return -1;
	return len;

}