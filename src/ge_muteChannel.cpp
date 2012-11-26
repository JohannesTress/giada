/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_muteChannel
 * a widget that represents mute actions inside the action editor.
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ------------------------------------------------------------------ */


#include "ge_muteChannel.h"


gMuteChannel::gMuteChannel(int x, int y, gdActionEditor *parent)
: Fl_Widget(x, y, 200, 60), parent(parent), draggedPoint(-1), selectedPoint(-1)
{
	extractPoints();
}


/* ------------------------------------------------------------------ */


void gMuteChannel::draw() {

	w(parent->totalWidth);

	/* clear everything */

	fl_rectf(x(), y(), parent->totalWidth, h(), COLOR_BG_MAIN);

	/* border */

	fl_color(COLOR_BD_0);
	fl_rect(x(), y(), parent->totalWidth, h());

	/* cover unused area */

	fl_rectf(parent->coverX, y()+1, parent->totalWidth-parent->coverX+x(), h()-2, COLOR_BG_1);

	/* draw "on" and "off" labels. Must stay in background */

	fl_color(COLOR_BG_1);
	fl_font(FL_HELVETICA, 9);
	fl_draw("on",  x()+4, y(),        w(), h(), (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
	fl_draw("off", x()+4, y()+h()-14, w(), h(), (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));

	/* draw on-off points. On = higher rect, off = lower rect. It always
	 * starts with a note_off */

	fl_color(COLOR_BG_2);

	int pxOld = x()+1;
	int pxNew = 0;
	int py    = y()+h()-5;
	int pyDot = py-6;

	for (unsigned i=0; i<points.size; i++) {

		/* next px */

		pxNew = points.at(i).x+x();

		/* draw line from pxOld to pxNew.
		 * i % 2 == 0: first point, mute_on
		 * i % 2 != 0: second point, mute_off */

		fl_line(pxOld, py, pxNew, py);
		pxOld = pxNew;

		py = i % 2 == 0 ? y()+4 : y()+h()-5;

		/* draw dots (handles) */

		fl_line(pxNew, y()+h()-5, pxNew, y()+4);

		if (selectedPoint == (int) i) {
			fl_color(COLOR_BD_1);
			fl_rectf(pxNew-3, pyDot, 7, 7);
			fl_color(COLOR_BG_2);
		}
		else
			fl_rectf(pxNew-3, pyDot, 7, 7);
	}

	/* last section */

	py = y()+h()-5;
	fl_line(pxNew+3, py, parent->coverX-1, py);
}


/* ------------------------------------------------------------------ */


void gMuteChannel::extractPoints() {

	points.clear();

	/* actions are already sorted by recorder::sortActions() */

	for (unsigned i=0; i<recorder::frames.size; i++) {
		for (unsigned j=0; j<recorder::global.at(i).size; j++) {
			if (recorder::global.at(i).at(j)->chan == (unsigned)parent->chan) {
				if (recorder::global.at(i).at(j)->type & (ACTION_MUTEON | ACTION_MUTEOFF)) {
					point p;
					p.frame = recorder::frames.at(i);
					p.type  = recorder::global.at(i).at(j)->type;
					p.x     = p.frame / 2 / parent->zoom;
					points.add(p);
					//printf("[gMuteChannel::extractPoints] point found, type=%d, frame=%d\n", p.type, p.frame);
				}
			}
		}
	}
}


/* ------------------------------------------------------------------ */


void gMuteChannel::updatePoints() {
	for (unsigned i=0; i<points.size; i++)
		points.at(i).x = points.at(i).frame / 2 / parent->zoom;
}


/* ------------------------------------------------------------------ */


int gMuteChannel::handle(int e) {

	int ret = 0;
	int mouseX = Fl::event_x()-x();

	switch (e) {

		case FL_ENTER: {
			ret = 1;
			break;
		}

		case FL_MOVE: {
			selectedPoint = getSelectedPoint();
			redraw();
			ret = 1;
			break;
		}

		case FL_LEAVE: {
			draggedPoint  = -1;
			selectedPoint = -1;
			redraw();
			ret = 1;
			break;
		}

		case FL_PUSH: {

			/* left click on point: drag
			 * right click on point: delete
			 * left click on void: add */

			if (Fl::event_button1())  {
				if (selectedPoint != -1) {
					draggedPoint   = selectedPoint;
					previousXPoint = points.at(selectedPoint).x;
				}
				else {

					/* click in the middle of a long mute_on (between two points): new actions
					 * must be added in reverse: first mute_off then mute_on. Let's find the
					 * next point from here. */

					unsigned nextPoint = points.size;
					for (unsigned i=0; i<points.size; i++) {
						if (mouseX < points.at(i).x) {
							nextPoint = i;
							break;
						}
					}

					/* next point odd = mute_on [click here] mute_off
					 * next point even = mute_off [click here] mute_on */

					unsigned frame_a = mouseX*2*parent->zoom;
					if (nextPoint % 2 != 0) {
						recorder::rec(parent->chan, ACTION_MUTEOFF, frame_a);
						recorder::rec(parent->chan, ACTION_MUTEON,  frame_a+2048);
						recorder::sortActions();
					}
					else {
						recorder::rec(parent->chan, ACTION_MUTEON,  frame_a);
						recorder::rec(parent->chan, ACTION_MUTEOFF, frame_a+2048);
						recorder::sortActions();
					}
					glue_setChannelWithActions(parent->chan); // update mainWindow
					extractPoints();
					redraw();
				}
			}
			else {

				/* delete points pair */

				if (selectedPoint != -1) {

					unsigned a;
					unsigned b;

					if (points.at(selectedPoint).type == ACTION_MUTEOFF) {
						a = selectedPoint-1;
						b = selectedPoint;
					}
					else {
						a = selectedPoint;
						b = selectedPoint+1;
					}

					//printf("selected: a=%d, b=%d >>> frame_a=%d, frame_b=%d\n",
					//		a, b, points.at(a).frame, points.at(b).frame);

					recorder::deleteAction(parent->chan,	points.at(a).frame,	points.at(a).type);
					recorder::deleteAction(parent->chan,	points.at(b).frame,	points.at(b).type);
					recorder::sortActions();

					glue_setChannelWithActions(parent->chan); // update mainWindow
					extractPoints();
					redraw();
				}
			}
			ret = 1;
			break;
		}

		case FL_RELEASE: {
			if (draggedPoint != -1) {

				if (points.at(draggedPoint).x == previousXPoint) {
					//puts("nothing to do");
				}
				else {

					unsigned newFrame = (points.at(draggedPoint).x*parent->zoom)*2;

					recorder::deleteAction(
							parent->chan,
							points.at(draggedPoint).frame,
							points.at(draggedPoint).type);

					recorder::rec(parent->chan,
							points.at(draggedPoint).type,
							newFrame);

					recorder::sortActions();

					points.at(draggedPoint).frame = newFrame;

					draggedPoint  = -1;
					selectedPoint = -1;
				}
			}
			ret = 1;
			break;
		}

		case FL_DRAG: {

			if (draggedPoint != -1) {

				/* constrain the point between two ends (leftBorder-point,
				 * point-point, point-rightBorder) */

				int prevPoint;
				int nextPoint;

				if (draggedPoint == 0) {
					prevPoint = 0;
					nextPoint = points.at(draggedPoint+1).x;
				}
				else
				if ((unsigned) draggedPoint == points.size-1) {
					prevPoint = points.at(draggedPoint-1).x;
					nextPoint = parent->coverX-x();
				}
				else {
					prevPoint = points.at(draggedPoint-1).x;
					nextPoint = points.at(draggedPoint+1).x;
				}

				if (mouseX < prevPoint)
					points.at(draggedPoint).x = prevPoint;
				else
				if (mouseX >= nextPoint)
					points.at(draggedPoint).x = nextPoint;
				else
					points.at(draggedPoint).x = mouseX;

				redraw();
			}
			ret = 1;
			break;
		}
	}


	return ret;
}


/* ------------------------------------------------------------------ */


int gMuteChannel::getSelectedPoint() {

	/* point is a 7x7 dot */

	for (unsigned i=0; i<points.size; i++) {
		if (Fl::event_x() >= points.at(i).x+x()-3 &&
				Fl::event_x() <= points.at(i).x+x()+3)
		return i;
	}
	return -1;
}