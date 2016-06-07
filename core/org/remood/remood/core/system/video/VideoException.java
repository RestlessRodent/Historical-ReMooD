// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system.video;

/**
 * This is thrown when there is a problem with a video driver.
 *
 * @since 2016/06/07
 */
public class VideoException
	extends RuntimeException
{
	/**
	 * Initializes the exception with no message or cause.
	 *
	 * @since 2016/06/07
	 */
	public VideoException()
	{
	}
	
	/**
	 * Initializes the exception with the given message.
	 *
	 * @param __m The exception message.
	 * @since 2016/06/07
	 */
	public VideoException(String __m)
	{
		super(__m);
	}
	
	/**
	 * Initializes the exception with the given message and cause.
	 *
	 * @param __m The exception message.
	 * @param __c The exception cause.
	 * @since 2016/06/07
	 */
	public VideoException(String __m, Throwable __c)
	{
		super(__m, __c);
	}
	
	/**
	 * Initializes the exception with the given cause.
	 *
	 * @param __c The exception cause.
	 * @since 2016/06/07
	 */
	public VideoException(Throwable __c)
	{
		super(__c);
	}
}

