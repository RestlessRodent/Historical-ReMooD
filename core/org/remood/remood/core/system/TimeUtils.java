// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.system;

/**
 * This class contains utilities that are used for calculating the time that
 * has passed on the system.
 *
 * @since 2016/05/19
 */
public final class TimeUtils
{
	/** The base time for low-resolution relative offset. */
	private static final long _BASE_TIME =
		System.nanoTime();
	
	/** Nanoseconds per tick. */
	private static final long _NS_PER_TIC =
		28_571_428L;
	
	/**
	 * Not used.
	 *
	 * @since 2016/05/19
	 */
	private TimeUtils()
	{
	}
	
	/**
	 * Returns the number of nanoseconds that have passed.
	 *
	 * @return The number of nanoseconds that have passed since the base clock
	 * was initialized.
	 * @since 2016/05/19
	 */
	public static long baseTime()
	{
		return System.nanoTime() - _BASE_TIME;
	}
	
	/**
	 * Returns the number of milliseconds which have passed.
	 *
	 * @return The number of milliseconds that have passed.
	 * @since 2016/05/19
	 */
	public static long getMilliseconds()
	{
		return baseTime() / 1_000_000L;
	}
	
	/**
	 * Returns the number of tics which have passed.
	 *
	 * @return The number of tics which have passed.
	 * @since 2016/05/19
	 */
	public static long getTics()
	{
		return baseTime() / _NS_PER_TIC;
	}
}

