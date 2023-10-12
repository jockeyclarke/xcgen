/*******************************************************************
**    File: XCResource.cpp
**      By: Paul L. Rowan     
** Created: 970415
**
** Description:
**
********************************************************************/

/*--- INCLUDES ----------------------------------------------------*/
#include "xcgenint.h"
#include <stdio.h>
#include <windows.h>
#include <string.h>

/*--- CONSTANTS ---------------------------------------------------*/


/************************ IMPORT SECTION ***************************/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/

/*-------- GLOBAL VARIABLES ---------------------------------------*/


/************************ PRIVATE SECTION **************************/
/*-------- GLOBAL VARIABLES ---------------------------------------*/
/*-------- FUNCTION PROTOTYPES ------------------------------------*/
//******************************************************************
//	BuildName -- Build the full pathname of the resource file to load.
//******************************************************************
Void BuildName( CharPtr pPath, CharPtr pName, CharPtr pExt, CharPtr pOutName )
{
	ENTER;

	strcpy( pOutName, pPath );
	strcat( pOutName, "\\" );
	strcat( pOutName, pName );
	strcat( pOutName, "." );
	strcat( pOutName, pExt );

	EXIT( "BuildName" );
}

Long ResourceSize( XCGENResourcePtr pRes )
{
	ENTER;

	return( pRes->size );

	EXIT ( "ResourceSize" );
}

//******************************************************************
//	LoadResource -- Physically load the resource file into memory
//******************************************************************
XCGENResourcePtr LoadResource( CharPtr pName, CharPtr pType )
{
	ENTER;

	Long					size;
	Char					filename[128];
	XCGENResourcePtr		pRes;

	BuildName( szDatabasePath, pName, pType, filename );

	if ((pRes=XCGENResourcePtr(GetData(filename, &size)))!=NULL)
	{
		pRes->size = size;
		return( pRes );	
	}
	sprintf( filename, "Resource file not found: %s - %s", pName, pType );
	FATALError( filename );
	return( NULL );

	EXIT( "LoadResource" );
}

//******************************************************************
// FindType -- Determine if a resource type has been loaded.
//******************************************************************
ResArrayPtr FindType( CharPtr pType )
{
	ENTER;

	Int		i;

	for (i=0;i<MAX_RESTYPES;i++)
	{
		if (!strcmp(pType,gResMap[i].type))
			return( &gResMap[i] );
	}
	return( NULL );

	EXIT( "FindType" );
}

//******************************************************************
// AddType -- Add a resource type to the map
//******************************************************************
ResArrayPtr AddType( CharPtr pType )
{
	ENTER;

	Int		i;

	for (i=0;i<MAX_RESTYPES;i++)
	{
		if (gResMap[i].type[0]=='\0')
		{
			strcpy(gResMap[i].type, pType);
			return( &gResMap[i] );
		}
	}
	FATALError( "Too many resource types" );
	return( NULL );

	EXIT( "AddType" );
}

//******************************************************************
//	FindResource -- Determine if a resource is loaded in memory
//******************************************************************
XCGENResourcePtr FindResource( CharPtr pName, CharPtr pType )
{
	ENTER;

	ResArrayPtr		pResEntries;
	Int				i;

	if ((pResEntries=FindType(pType))!=NULL)
	{
		for (i=0;i<MAX_RESOURCES;i++)
		{
			if (!strcmp(pName,pResEntries->resList[i].name))
				return( pResEntries->resList[i].pRes );
		}
	}
	return( NULL );

	EXIT( "FindResource" );
}

//******************************************************************
//	AddResource -- Add a resource to the map and load it's file into memory
//******************************************************************
XCGENResourcePtr AddResource( CharPtr pName, CharPtr pType )
{
	ENTER;

	Int				i;
	ResArrayPtr		pTypeRA;

	if ((pTypeRA=FindType( pType ))==NULL)
	{
		pTypeRA = AddType( pType );
	}

	for (i=0;i<MAX_RESOURCES;i++)
	{
		if (pTypeRA->resList[i].name[0]=='\0')
		{
			strcpy( pTypeRA->resList[i].name, pName );
			pTypeRA->resList[i].pRes = LoadResource( pName, pTypeRA->type );
			return( pTypeRA->resList[i].pRes );
		}
	}
	FATALError( "Too many resources" );
	return( NULL );

	EXIT( "AddResource" );
}

//******************************************************************
// RemoveResource -- remove a resource from the map and unload it from memory
//******************************************************************
Void RemoveResource( CharPtr pName, CharPtr pType )
{
	ENTER;

	ResArrayPtr		pTypeRA;
	Int				i;
	
	if ((pTypeRA=FindType(pType))!=NULL)
	{
		for (i=0;i<MAX_RESOURCES;i++)
		{
			if (!strcmp(pTypeRA->resList[i].name,pName))
			{
				pTypeRA->resList[i].name[0] = '\0';
				ReleaseData( pTypeRA->resList[i].pRes );
				return;
			}
		}
	}
	FATALError( "Cannot find resource to unload" );

	EXIT( "RemoveResource" );
}

/************************ MEMBER FUNCTIONS *************************/
XCGENResourcePtr	GetResData( CharPtr pName, CharPtr pType )
{
	ENTER;

	XCGENResourcePtr	pRes;

	if ((pRes=FindResource(pName,pType))!=NULL)
	{
		pRes->ref++;
		return( pRes );
	}

	if ((pRes=AddResource( pName, pType ))!=NULL)
	{
		pRes->ref = 1;
		return( pRes );
	}

	return( NULL );

	EXIT( "GetResData" );
}

Int	ReleaseResData( XCGENResourcePtr pRes )
{
	ENTER;

	pRes->ref--;

	if (pRes->ref==0)
	{
		RemoveResource( pRes->name, pRes->type );
		return( 0 );
	}

	return( pRes->ref );

	EXIT( "ReleaseResData" );
}

VoidPtr GetData( CharPtr pName, LongPtr pSize )
{
	ENTER;

	HANDLE		hFile;
	ULong		len, lenRead;
	UCharPtr	pData;

	hFile = CreateFile( pName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL );

	// Runtime-exception handling
	if (hFile==INVALID_HANDLE_VALUE)
	{
		Char	tex[128];

		sprintf( tex, "File not found: %s", pName );
		FATALError( tex );
		return( NULL );
	}
	
	
	*pSize = len = GetFileSize( hFile, NULL );

	pData = new UChar[len];
	DEBUGAssert( pData );

	ReadFile( hFile, pData, len, &lenRead, NULL );

	DEBUGAssert( len==lenRead );

	CloseHandle( hFile ); 

	return( VoidPtr(pData) );

	EXIT( "GetData" );
}

Void ReleaseData( VoidPtr pData )
{
	ENTER;

	delete pData;

	EXIT( "ReleaseData" );
}