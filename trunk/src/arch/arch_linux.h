#ifndef LINUX_ARCH_H
#define LINUX_ARCH_H

#ifdef HAVE_ALSA
#include "Sound/RageSoundDriver_ALSA9.h"
#include "Sound/RageSoundDriver_ALSA9_Software.h"
#endif

#ifdef HAVE_OSS
#include "Sound/RageSoundDriver_OSS.h"
#endif

#ifdef HAVE_GTK
#include "LoadingWindow/LoadingWindow_Gtk.h"
#endif

/* Load this even if we have GTK, since we can fall back if GTK is missing. */
#include "LoadingWindow/LoadingWindow_SDL.h"

#include "ArchHooks/ArchHooks_Unix.h"

#include "Lights/LightsDriver_LinuxWeedTech.h"
#include "Lights/LightsDriver_LinuxParallel.h"

#include "MemoryCard/MemoryCardDriverThreaded_Linux.h"

#endif

/*
 * (c) 2002-2003 Glenn Maynard
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

