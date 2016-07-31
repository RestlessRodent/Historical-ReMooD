// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.remood.core.config;

import java.util.ArrayList;
import java.util.Deque;
import java.util.LinkedList;
import java.util.List;
import org.remood.remood.core.console.GameConsole;

/**
 * This class contains some default command line arguments which may be used
 * to initialize the game or setup some other state (such as playing a demo).
 *
 * Arguments are either switches which start with a dash, or console commands
 * to be executed which start with a plus. Unlike previously some arguments
 * now accept multiple options (such as demo playback).
 *
 * @since 2016/07/16
 */
public class CommandLineArguments
{
	/** Developer mode? */
	protected final boolean devparm;
	
	/** No mouse. */
	protected final boolean nomouse;
	
	/** Early console commands. */
	private final String[] _earlyconsole;
	
	/** Late console commands. */
	private final String[] _lateconsole;
	
	/**
	 * This parses the command line arguments which may be used when parsing
	 * the configuration file.
	 *
	 * @param __gc The console to print messages to.
	 * @param __args Program arguments.
	 * @throws NullPointerException On null arguments.
	 * @since 2016/07/16
	 */
	public CommandLineArguments(GameConsole __gc, String... __args)
		throws NullPointerException
	{
		// Check
		if (__gc == null || __args == null)
			throw new NullPointerException();
		
		// Temporary hold
		List<String> earlyconsole = new ArrayList<>();
		List<String> lateconsole = new ArrayList<>();
		boolean devparm = false;
		boolean nomouse = false;
		
		// Go through all arguments
		int n = __args.length;
		List<String> sub = new ArrayList<>();
		for (int i = 0; i < n; i += (1 + sub.size()))
		{
			// Get
			String arg = __args[i];
			int an = arg.length();
			char fc = (an > 0 ? arg.charAt(0) : 0);
			
			// Determine the sub-arguments to this argument
			sub.clear();
			for (int j = i + 1; j < n; j++)
			{
				String sx = __args[j];
				
				// If it starts with a minus or plus then it is a new argument
				// group
				int sxn = sx.length();
				char qq = (sxn > 0 ? sx.charAt(0) : 0);
				if (qq == '-' || qq == '+')
					break;
				
				// Add to list
				sub.add(sx);
			}
			
			// A normal command line switch?
			switch (arg)
			{
					// Ignore these arguments
				case "-java":
					break;
					
					// Developer mode?
				case "-devparm":
					devparm = true;
					break;
					
					// No mouse
				case "-nomouse":
					nomouse = true;
					break;
				
					// Unknown, either an illegal switch or a console command
				default:
					// Unknown switch
					if (fc == '-')
					{
						// Warn on it
						System.err.printf("Unknown command line switch: %s%n",
							arg);
					}
					
					// Console command
					else if (fc == '+')
					{
						// ++ are late console commands to be executed at the
						// titlescreen or before an actual game has been
						// launched.
						char sc = (an > 1 ? arg.charAt(1) : 0);
						String m = __consoleMerge(arg, sub);
						if (sc == '+')
							lateconsole.add(m);
						
						// Otherwise execute following a configuration load
						else
							earlyconsole.add(m);
					}
					
					// Assume -file
					else
					{
						if (true)
							throw new Error("TODO");
					}
					break;
			}
		}
		
		// Set
		this.devparm = devparm;
		this.nomouse = nomouse;
		this._earlyconsole = earlyconsole.<String>toArray(
			new String[earlyconsole.size()]);
		this._lateconsole = lateconsole.<String>toArray(
			new String[lateconsole.size()]);
	}
	
	/**
	 * Allow mice to be used?
	 *
	 * @return {@code true} if mice are to be used.
	 * @since 2016/07/16
	 */
	public boolean allowMouse()
	{
		return !this.nomouse;
	}
	
	/**
	 * Is this developer mode?
	 *
	 * @return {@code true} if this is developer mode.
	 * @since 2016/07/16
	 */
	public boolean isDeveloper()
	{
		return this.devparm;
	}
	
	/**
	 * Merges all of the strings so that they may be used for input to the
	 * console.
	 *
	 * @param __a The first string, may be {@code null}.
	 * @param __b The remaining strings, may also be {@code null}.
	 * @return A merged string which is suitable for usage by the console.
	 * @since 2016/07/16
	 */
	private static String __consoleMerge(String __a, Iterable<String> __b)
	{
		StringBuilder sb = new StringBuilder();
		
		// First string
		if (__a != null)
			sb.append(__a);
		
		// Add all b
		if (__b != null)
		{
			for (String b : __b)
			{
				// Ignore null strings.
				if (b == null)
					continue;
				
				// Space and starting quote
				sb.append(' ');
				sb.append('"');
				
				// Need to escape slashes and quotes because input command
				// line arguments tend to be pre-quoted.
				int n = b.length();
				for (int i = 0; i < n; i++)
				{
					char c = b.charAt(i);
					
					if (c == '"')
						sb.append("\\\"");
					else if (c == '\\')
						sb.append("\\\\");
					else
						sb.append(c);
				}
				
				// End quote
				sb.append('"');
			}
		}
		
		// Done
		return sb.toString();
	}
}

