// -*- Mode: Java; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// ReMooD Doom Source Port <http://remood.org/>
//   Copyright (C) 2005-2016 GhostlyDeath <ghostlydeath@remood.org>
//     For more credits, see readme.mkd.
// ---------------------------------------------------------------------------
// ReMooD is under the GNU General Public License v3+, see license.mkd.
// ---------------------------------------------------------------------------

package org.remood.javaseremood;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.awt.Rectangle;
import javax.swing.JFrame;
import javax.swing.JPanel;
import org.remood.remood.core.system.video.VideoDriver;
import org.remood.remood.core.system.video.VideoException;
import org.remood.remood.core.system.video.VideoSoftwareSurface;
import org.remood.remood.core.system.video.VideoSurface;

/**
 * This class provides a video surface which uses Swing.
 *
 * @since 2016/07/17
 */
public class JavaSEVideoSurface
	extends VideoSoftwareSurface
{
	/** The display frame. */
	private volatile JFrame _frame;
	
	/** The image backing the framebuffer. */
	private volatile BufferedImage _image;
	
	/** The view pane. */
	private volatile __Pane__ _pane;
	
	/**
	 * Initializes the video surface.
	 *
	 * @param __w The surface width.
	 * @param __h The surface height.
	 * @since 2016/07/17
	 */
	public JavaSEVideoSurface(int __w, int __h)
		throws VideoException
	{
		super(__w, __h, null, __w);
	}
	
	/**
	 * {@inheritDoc}
	 * @since 2016/07/17
	 */
	@Override
	protected int[] createFramebuffer()
	{
		// Setup frame
		JFrame frame = new JFrame("ReMooD");
		this._frame = frame;
		
		// Limit frame size to the resolution size
		int w = this.width, h = this.height;
		frame.setMinimumSize(new Dimension(w, h));
		
		// Setup image
		BufferedImage image = new BufferedImage(
			w, h, BufferedImage.TYPE_INT_RGB);
		this._image = image;
		
		// Create panel
		__Pane__ pane = new __Pane__();
		this._pane = pane;
		frame.add(pane);
		
		// Make it visible and center on the screen
		frame.setVisible(true);
		frame.pack();
		frame.setLocationRelativeTo(null);
		
		// Return raw pixel buffer to the framebuffer
		return ((DataBufferInt)image.getRaster().getDataBuffer()).getData();
	}
	
	/**
	 * Panel used for drawing.
	 *
	 * @since 2016/07/17
	 */
	private class __Pane__
		extends JPanel
	{
		/**
		 * {@inheritDoc}
		 * @since 2016/07/17
		 */
		@Override
		public void paintComponent(Graphics __g)
		{
			// Draw parent components
			super.paintComponent(__g);
			
			// Get area which is being drawn
			Rectangle re = __g.getClip().getBounds();
			int bx = re.x;
			int by = re.y;
			int bw = re.width;
			int bh = re.height;
			
			// Get video size
			int w = JavaSEVideoSurface.this.width,
				h = JavaSEVideoSurface.this.height;
			
			// Draw framebuffer
			__g.drawImage(JavaSEVideoSurface.this._image.getSubimage(
				0, 0, bw, bh), bx, by, null);
		}
	}
}

