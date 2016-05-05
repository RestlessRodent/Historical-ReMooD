// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ----------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2015 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see AUTHORS.
// ----------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3 (or later), see COPYING.
// ----------------------------------------------------------------------------

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.CharBuffer;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.text.MessageFormat;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.ResourceBundle;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.GZIPOutputStream;

/**
 * This creates a gigantic WAD file, maximum file size and maximum lump size.
 *
 * The WAD is written compressed so space usage is reduced.
 *
 * @since 2015/09/19
 */
public class GigaWAD
{
	/** Debug logger. */
	static final Logger LOG =
		Logger.getLogger(GigaWAD.class.getName());
	
	/** Name offset. */
	public static final int NAME_OFFSET =
		8;
	
	/** Slice size. */
	public static final int SLICE =
		4096;
	
	/**
	 * Main entry point.
	 *
	 * @param __args Program arguments.
	 * @since 2015/09/19
	 */
	public static void main(String... __args)
	{
		// Calculate lump count
		int lumpcount = (int)((0xFFFF_FFFFL - 12L) / 16L);
		if (lumpcount < 0)
			lumpcount = Integer.MAX_VALUE;
		long filesize = 12 + (lumpcount * 16);
		
		// Create WAD
		try (GZIPOutputStream os = new GZIPOutputStream(
			new FileOutputStream("gigawad.wad.gz")))
		{
			// Prepare header
			byte bhead[] = new byte[12];
			ByteBuffer uhead = ByteBuffer.wrap(bhead);
			uhead.order(ByteOrder.LITTLE_ENDIAN);
			
			// Some details
			uhead.put(0, (byte)'P');
			uhead.put(1, (byte)'W');
			uhead.put(2, (byte)'A');
			uhead.put(3, (byte)'D');
			
			// Lump count
			uhead.putInt(4, (int)lumpcount);
			
			// Table offset
			uhead.putInt(8, 12);
			
			// Write
			os.write(bhead);
			
			// Prepare index bits
			byte bdex[] = new byte[16];
			ByteBuffer udex = ByteBuffer.wrap(bdex);
			udex.order(ByteOrder.LITTLE_ENDIAN);
			
			// File position is always zero
			udex.putInt(0, 0);
			
			// Entry is the the size of the enitre WAD
			udex.putInt(4, (int)filesize);
			
			// Create a big slice to reduce writes
			byte slicebytes[] = new byte[16 * SLICE];
			ByteBuffer bigslice = ByteBuffer.wrap(slicebytes);
			bigslice.order(ByteOrder.LITTLE_ENDIAN);
			
			// Initialize basic big slice
			udex.mark();
			for (int i = 0; i < SLICE; i++)
			{
				bigslice.put(udex);
				udex.reset();
			}
			
			// Write indexes, will take awhile
			long tstart = System.nanoTime();
			String lcstr = "" + lumpcount;
			int slmask = (SLICE - 1) << 3;
			for (int i = 0; i < lumpcount; i += SLICE)
			{
				// Log, since it will take a long time
				if ((i & slmask) == 0)
				{
					// Get current time
					long tnow = System.nanoTime();
					
					// Total duration passed
					long tdur = tnow - tstart;
					
					// The number of things made per nanosecond
					long tdidxin = tdur / (long)(i + 1);
					
					// Remaining left
					int remcount = lumpcount - i;
					
					// Estimated time to finish
					long testfin = (long)remcount * tdidxin;
					String fxfin;
					
					// Minutes remain
					if (testfin >= 60_000_000_000L)
					{
						testfin /= 60_000_000_000L;
						fxfin = "min";
					}
					
					// Milliseconds remain
					else
					{
						testfin /= 1_000_000L;
						fxfin = "ms";
					}
					
					// Print
					System.err.print(i);
					System.err.print(" of ");
					System.err.print(lcstr);
					System.err.print(" (ran ");
					System.err.print(tdur / 1_000_000L);
					System.err.print(" ms, est ");
					System.err.print(testfin);
					System.err.print(" ");
					System.err.print(fxfin);
					System.err.print(")\r");
				}
				
				// Initialize slice names
				int sbxll = 0;
				for (int j = 0; j < SLICE; j++)
				{
					// True index
					int ti = (i + j);
					
					// Exceeds bounds?
					if (ti >= lumpcount)
					{
						sbxll = 16 * (j - 1);
						break;
					}
					else
						sbxll = slicebytes.length;
					
					// Name of the lump
					String name = Integer.toString(ti, 36);
					
					// Store name
					int n = name.length();
					for (int k = 0; k < n; k++)
						bigslice.put((j * 16) + 8 + k,
							(byte)Character.toUpperCase(name.charAt(k)));
				}
				
				// Mega write
				os.write(slicebytes, 0, sbxll);
			}
			
			// Final flush
			if (os instanceof DeflaterOutputStream)
				((DeflaterOutputStream)os).finish();
			os.flush();
			
			// Done
			System.err.println();
		}
		
		// Maybe out of disk space?
		catch (IOException ioe)
		{
			throw new RuntimeException(ioe);
		}
	}
}

