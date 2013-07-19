#include "global.h"

/* Texture policies:
 *
 * Default: When DelayedDelete is off, delete unused textures immediately.
 *          When on, only delete textures when we change themes, on DoDelayedDelete().
 *			This is what you get if you call LoadTexture() on a texture that isn't
 *			loaded.
 *
 * Cached:  When DelayedDelete is off, delete unused textures when we change screens.
 *			When on, treat as Default.  This is used to precache textures that aren't
 *			loaded immediately; use CacheTexture.
 *	
 * Volatile: Delete unused textures once they've been used at least once.  Ignore
 *			 DelayedDelete.
 *
 *			 This is for banners.  We don't want to load all low-quality banners in
 *			 memory at once, since it might be ten megs of textures, and we don't
 *			 *have* to, since we can reload them very quickly.  We don't want to keep
 *			 high quality textures in memory, either, although it's unlikely that a
 *			 player could actually view all banners long enough to transition to them
 *			 all in the course of one song select screen.
 *
 * Permanent: Never delete the texture.
 * 			This is only used for BannerCache=2 mode.
 *
 * Policy priority is in the order PERMANENT, CACHED, VOLATILE, DEFAULT.  Textures that
 * are loaded DEFAULT can be changed to VOLATILE and CACHED; VOLATILE textures can only
 * be changed to CACHED.  CACHED flags are normally set explicitly on a per-texture
 * basis.  VOLATILE flags are set for all banners, but banners which are explicitly
 * set to CACHED should stay CACHED.  Finally, all textures, when they're finally used,
 * are loaded as NORMAL, and that should never override.
 */
	
#include "RageTextureManager.h"
#include "RageBitmapTexture.h"
#include "arch/MovieTexture/MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageDisplay.h"

RageTextureManager*		TEXTUREMAN		= NULL;

RageTextureManager::RageTextureManager()
{
	m_iNoWarnAboutOddDimensions = 0;
	m_TexturePolicy = RageTextureID::TEX_DEFAULT;
}

RageTextureManager::~RageTextureManager()
{
	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		if( pTexture->m_iRefCount )
			LOG->Trace( "TEXTUREMAN LEAK: '%s', RefCount = %d.", i->first.filename.c_str(), pTexture->m_iRefCount );
		SAFE_DELETE( pTexture );
	}
}

void RageTextureManager::Update( float fDeltaTime )
{
	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		pTexture->Update( fDeltaTime );
	}
}

void RageTextureManager::AdjustTextureID(RageTextureID &ID) const
{
	if( ID.iColorDepth == -1 )
		ID.iColorDepth = m_Prefs.m_iTextureColorDepth;
	ID.iMaxSize = min( ID.iMaxSize, m_Prefs.m_iMaxTextureResolution );
	if( m_Prefs.m_bMipMaps )
		ID.bMipMaps = true;
}

/* If you've set up a texture yourself, register it here so it can be referenced
 * and deleted by an ID.  This takes ownership.  They're kept around until the
 * ne*/
bool RageTextureManager::IsTextureRegistered( RageTextureID ID ) const
{
	AdjustTextureID(ID);
	return m_mapPathToTexture.find(ID) != m_mapPathToTexture.end();
}

void RageTextureManager::RegisterTexture( RageTextureID ID, RageTexture *pTexture )
{
	AdjustTextureID(ID);

	/* Make sure we don't already have a texture with this ID.  If we do, the
	 * caller should have used it. */
	std::map<RageTextureID, RageTexture*>::iterator p = m_mapPathToTexture.find(ID);
	if(p != m_mapPathToTexture.end())
		/* Oops, found the texture. */
		RageException::Throw("Custom texture \"%s\" already registered!", ID.filename.c_str());

	m_mapPathToTexture[ID] = pTexture;
}

// Load and unload textures from disk.
RageTexture* RageTextureManager::LoadTextureInternal( RageTextureID ID )
{
	CHECKPOINT_M( ssprintf( "RageTextureManager::LoadTexture(%s).", ID.filename.c_str() ) );

	AdjustTextureID(ID);

	/* We could have two copies of the same bitmap if there are equivalent but
	 * different paths, e.g. "Bitmaps\me.bmp" and "..\Rage PC Edition\Bitmaps\me.bmp". */
	std::map<RageTextureID, RageTexture*>::iterator p = m_mapPathToTexture.find(ID);
	if(p != m_mapPathToTexture.end())
	{
		/* Found the texture.  Just increase the refcount and return it. */
		RageTexture* pTexture = p->second;
		pTexture->m_iRefCount++;
		return pTexture;
	}

	// The texture is not already loaded.  Load it.
	CString sExt = GetExtension(ID.filename);
	sExt.MakeLower();

	RageTexture* pTexture;
	if( sExt == "avi" || sExt == "mpg" || sExt == "mpeg" )
		pTexture = MakeRageMovieTexture( ID );
	else
		pTexture = new RageBitmapTexture( ID );

	m_mapPathToTexture[ID] = pTexture;

	return pTexture;
}

/* Load a normal texture.  Use this call to actually use a texture. */
RageTexture* RageTextureManager::LoadTexture( RageTextureID ID )
{
	RageTexture* pTexture = LoadTextureInternal( ID );
	if( pTexture )
		pTexture->m_bWasUsed = true;
	return pTexture;
}

void RageTextureManager::CacheTexture( RageTextureID ID )
{
	RageTexture* pTexture = LoadTextureInternal( ID );
	pTexture->GetPolicy() = min( pTexture->GetPolicy(), RageTextureID::TEX_CACHED );
	UnloadTexture( pTexture );
}

void RageTextureManager::VolatileTexture( RageTextureID ID )
{
	RageTexture* pTexture = LoadTextureInternal( ID );
	pTexture->GetPolicy() = min( pTexture->GetPolicy(), RageTextureID::TEX_VOLATILE );
	UnloadTexture( pTexture );
}

void RageTextureManager::PermanentTexture( RageTextureID ID )
{
	RageTexture* pTexture = LoadTextureInternal( ID );
	pTexture->GetPolicy() = min( pTexture->GetPolicy(), RageTextureID::TEX_PERMANENT );
	UnloadTexture( pTexture );
}

void RageTextureManager::UnloadTexture( RageTexture *t )
{
	if( t == NULL )
		return;

	t->m_iRefCount--;
	ASSERT( t->m_iRefCount >= 0 );

	if( t->m_iRefCount )
		return; /* Can't unload textures that are still referenced. */

	if( t->GetPolicy() == RageTextureID::TEX_PERMANENT )
		return; /* Never unload TEX_PERMANENT textures. */
	bool bDeleteThis = false;

	/* Always unload movies, so we don't waste time decoding.
	 *
	 * Actually, multiple refs to a movie won't work; they should play independently,
	 * but they'll actually share settings.  Not worth fixing, since we don't currently
	 * using movies for anything except BGAs (though we could).
	 */
	if( t->IsAMovie() )
		bDeleteThis = true;

	/* Delete normal textures immediately unless m_bDelayedDelete is is on. */
	if( t->GetPolicy() == RageTextureID::TEX_DEFAULT && !m_Prefs.m_bDelayedDelete )
		bDeleteThis = true;

	/* Delete volatile textures after they've been used at least once. */
	if( t->GetPolicy() == RageTextureID::TEX_VOLATILE && t->m_bWasUsed )
		bDeleteThis = true;
	
	if( bDeleteThis )
		DeleteTexture( t );
}

void RageTextureManager::DeleteTexture( RageTexture *t )
{
	ASSERT( t->m_iRefCount==0 );
	LOG->Trace( "RageTextureManager: deleting '%s'.", t->GetID().filename.c_str() );

	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); i++ )
	{
		if( i->second == t )
		{
			m_mapPathToTexture.erase( i );	// remove map entry
			SAFE_DELETE( t );	// free the texture
			return;
		}
	}

	ASSERT(0);	// we tried to delete a texture that wasn't loaded.
}

void RageTextureManager::GarbageCollect( GCType type )
{
	// Search for old textures with refcount==0 to unload
	LOG->Trace("Performing texture garbage collection.");

	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); )
	{
		std::map<RageTextureID, RageTexture*>::iterator j = i;
		i++;

		CString sPath = j->first.filename;
		RageTexture* t = j->second;

		if( t->m_iRefCount )
			continue; /* Can't unload textures that are still referenced. */
		if( t->GetPolicy() == RageTextureID::TEX_PERMANENT )
			return; /* Never unload TEX_PERMANENT textures. */

		bool bDeleteThis = false;
		if( type==screen_changed )
		{
			switch( t->GetPolicy() )
			{
			case RageTextureID::TEX_DEFAULT: 
				/* If m_bDelayedDelete, wait until delayed_delete.  If !m_bDelayedDelete,
				 * it should have been deleted when it reached no references, but we
				 * might have just changed the preference. */
				if( !m_Prefs.m_bDelayedDelete )
					bDeleteThis = true;
				break;
			case RageTextureID::TEX_CACHED:
				if( !m_Prefs.m_bDelayedDelete )
					bDeleteThis = true;
				break;
			case RageTextureID::TEX_VOLATILE:
				bDeleteThis = true;
				break;
			default: ASSERT(0);
			}
		}

		/* This happens when we change themes; free all textures. */
		if( type==delayed_delete )
			bDeleteThis = true;
			
		if( bDeleteThis )
			DeleteTexture( t );
	}
}


void RageTextureManager::ReloadAll()
{
	TEXTUREMAN->DisableOddDimensionWarning();

	/* Let's get rid of all unreferenced textures, so we don't reload a
	 * ton of cached data that we're not necessarily going to use. */
	DoDelayedDelete();

	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		i->second->Reload();
	}

	TEXTUREMAN->EnableOddDimensionWarning();
}

/* In some cases, changing the display mode will reset the rendering context,
 * releasing all textures.  We don't want to reload immediately if that happens,
 * since we might be changing texture preferences too, which also may have to reload
 * textures.  Instead, tell all textures that their texture ID is invalid, so it
 * doesn't try to free it later when we really do reload (since that ID might be
 * associated with a different texture).  Ack. */
void RageTextureManager::InvalidateTextures()
{
	std::map<RageTextureID, RageTexture*>::iterator i;
	for( i = m_mapPathToTexture.begin(); i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		pTexture->Invalidate();
	}
}

bool RageTextureManager::SetPrefs( RageTextureManagerPrefs prefs )
{
	bool need_reload = false;
	if( m_Prefs != prefs )
		need_reload = true;

	m_Prefs = prefs;
	
	ASSERT( m_Prefs.m_iTextureColorDepth==16 || m_Prefs.m_iTextureColorDepth==32 );
	ASSERT( m_Prefs.m_iMovieColorDepth==16 || m_Prefs.m_iMovieColorDepth==32 );
	return need_reload;
}

void RageTextureManager::DiagnosticOutput() const
{
	unsigned cnt = distance(m_mapPathToTexture.begin(), m_mapPathToTexture.end());
	LOG->Trace("%u textures loaded:", cnt);

	int total = 0;
	for( std::map<RageTextureID, RageTexture*>::const_iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i )
	{
		const RageTextureID &ID = i->first;
		const RageTexture *tex = i->second;

		CString diags = DISPLAY->GetTextureDiagnostics( tex->GetTexHandle() );
		CString str = ssprintf("%3ix%3i (%2i)",
			tex->GetTextureHeight(), tex->GetTextureWidth(),
			tex->m_iRefCount);

		if( diags != "" )
			str += " " + diags;

		LOG->Trace(" %-40s %s",
			str.c_str(), Basename(ID.filename).c_str() );
		total += tex->GetTextureHeight() * tex->GetTextureWidth();
	}
	LOG->Trace("total %3i texels", total);
}

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
