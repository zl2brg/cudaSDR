/**
* @file  cusdr_oglUtils.h
* @brief Utils header file for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2011-11-17
*/

/*
 *   Copyright 2011 Hermann von Hasseln, DL3HVH
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _CUSDR_OPENGLTYPES_H
#define _CUSDR_OPENGLTYPES_H

#include <QOpenGLWidget>
//#include <QList>
//#include <QRect>
//#include <QColor>
//#include <QVarLengthArray>
#include <QOpenGLFramebufferObject>
#include <QtCore/qmath.h>

#define GL_CLAMP_TO_EDGE	0x812F


typedef struct _gl2i {

	GLint x;
	GLint y;

} TGL2int;

typedef struct _gl3i {

	GLint x;
	GLint y;
	GLint z;

} TGL3int;

typedef struct _gl2f {

	GLfloat x;
	GLfloat y;

} TGL2float;

typedef struct _gl3f {

	GLfloat x;
	GLfloat y;
	GLfloat z;

} TGL3float;

typedef struct _ucharRGBA {

	uchar	red;
	uchar	green;
	uchar	blue;
	uchar	alpha;

} ucharRGBA;

typedef struct _glubyteRGBA {

	GLubyte	red;
	GLubyte	green;
	GLubyte	blue;
	GLubyte	alpha;

} TGL_ubyteRGBA;

typedef struct _scaleSteps {

	double smallStep;
	double bigStep;

} TScaleSteps;

typedef struct _scale {

	QList<int> mainPointPositions;
	QList<int> subPointPositions;

	QList<qreal> mainPoints;
	QList<qreal> subPoints;

} TScale;


struct s_glRGBA_float {

	GLfloat r, g, b, a;

	s_glRGBA_float() : r(0), g(0), b(0), a(0) {}

	s_glRGBA_float(GLfloat red, GLfloat grn, GLfloat blu, GLfloat alpha) : r(red), g(grn), b(blu), a(alpha) {}
};

struct s_glRGBA_uByte {

	GLubyte r, g, b, a;

	s_glRGBA_uByte() : r(0), g(0), b(0), a(0) {}

	s_glRGBA_uByte(GLubyte red, GLubyte grn, GLubyte blu, GLubyte alpha) : r(red), g(grn), b(blu), a(alpha) {}
};

typedef struct _widebandDisplayData {

	QSize		size;

	QRect		widebandPanRect;
	QRect		freqScaleWidebandPanRect;
	QRect		dBmScaleWidebandPanRect;

	QVector<qreal>	widebandPanBins;
	
	qreal		dBmPanMin;
	qreal		dBmPanMax;
	qreal		scaleMult;
	qreal		freqScaleZoomFactor;

	long		frequency;

	bool		freqScaleWidebandUpdate;
	bool		freqScaleWidebandRenew;
	bool		dBmScaleWidebandUpdate;
	bool		dBmScaleWidebandRenew;
	bool		widebandPanGridUpdate;
	bool		widebandPanGridRenew;

} TWideBandDisplayData;
 
//**************************************************************
inline QString frequencyString(double frequency, bool addPlusSign = false) {

	QString str("");

	double f = qAbs(frequency);

	if (f >= 1e9) {

		str = QString::number(f / 1e9, 'f', 6);
		str.insert(str.size() - 3, '.');
		str += " GHz";
	}
	else
	if (f >= 1e6) {

		str = QString::number(f / 1e6, 'f', 6 + 1);
		str.insert(str.size() - 4, '.');
		str.insert(str.size() - 1, '.');
		str += " MHz";
	}
	else
	if (f >= 1e3) {

		str = QString::number(f / 1e3, 'f', 3 + 1);
		str.insert(str.size() - 1, '.');
		str += " kHz";
	}
	else {

		str = QString::number(f, 'f', 1) + "Hz";
	}

	if (frequency < 0) str = '-' + str;
	else
	if (frequency > 0 && addPlusSign) str = '+' + str;

	return str;
}

inline int nextPowerOfTwo(int value) {

	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	++value;
	return value;
}

inline TScaleSteps getXScale(double size) {

	TScaleSteps s;

	qint64 base = 1;
	int mult = 1;
	while (size > 10.0f) {
        size /= 10;
		base *= 10;
    }

	if (size < 2)		mult = 2;
	else if (size < 5)	mult = 5;
	else				mult = 10;

	s.bigStep = base * mult;
	switch (mult) {

		case 1:	 s.smallStep = s.bigStep / 5; break;
		case 2:	 s.smallStep = s.bigStep / 2; break;
		case 5:	 s.smallStep = s.bigStep / 5; break;
		case 10: s.smallStep = s.bigStep / 5; break;
	}
	return s;
}

inline TScaleSteps getXScale(double size, float scale) {

	TScaleSteps s;

	qint64 base = 1;
	int mult = 1;
	while (size > scale) {
        size /= scale;
		base *= scale;
    }

	if (size < 2)		mult = 2;
	else if (size < 5)	mult = 5;
	else				mult = 10;

	s.bigStep = base * mult;
	switch (mult) {

		case 1:	 s.smallStep = s.bigStep / 5; break;
		case 2:	 s.smallStep = s.bigStep / 2; break;
		case 5:	 s.smallStep = s.bigStep / 5; break;
		case 10: s.smallStep = s.bigStep / 5; break;
	}
	return s;
}

inline TScaleSteps getYScale(double size, float scale) {

	TScaleSteps s;

	qint64 base = 1;
	int mult = 1;
	while (size > scale) {
        size /= scale;
		base *= scale;
    }

	if (size < 2)		mult = 2;
	else if (size < 5)	mult = 5;
	else				mult = 10;

	s.bigStep = base * mult;
	switch (mult) {

		case 1:	 s.smallStep = s.bigStep / 5; break;
		case 2:	 s.smallStep = s.bigStep / 2; break;
		case 5:	 s.smallStep = s.bigStep / 5; break;
		case 10: s.smallStep = s.bigStep / 2; break;
	}
	return s;
}

inline TScale getXRuler(const QRect &rect, int fontMaxWidth, qreal unit, qreal lo, qreal hi) {

	TScale ruler;
	
	TScaleSteps scale = getXScale(fontMaxWidth / unit);
	qreal value = qFloor(lo / scale.bigStep) * scale.bigStep;

	if (rect.width() > 0 && unit > 0) {
		qreal firstMajor = qFloor(lo / scale.bigStep) * scale.bigStep;
		if (firstMajor < lo)
			firstMajor += scale.bigStep;
		const int firstMajorX = qRound(unit * (firstMajor - lo));

		if (firstMajorX > fontMaxWidth) {
			ruler.mainPoints << lo;
			ruler.mainPointPositions << 0;
			if (scale.smallStep > 0) {
				for (qreal sv = lo + scale.smallStep; sv < firstMajor && sv < hi; sv += scale.smallStep) {
					const int x = qRound(unit * (sv - lo));
					if (x > 0 && x < rect.width())
						ruler.subPointPositions << x;
				}
			}
		}
	}

	while (value < hi) {

		int x = qRound(unit * (value - lo));

		if (x >= rect.width()) break;
		if (x > 0) {

			ruler.mainPoints << value;
			ruler.mainPointPositions << x;
		}

		if (scale.smallStep > 0) {

			qreal smallValue = value + scale.smallStep;
			qreal smallUpperValue = value + scale.bigStep;

			while (smallValue < smallUpperValue && smallValue < hi) {

				int x = qRound(unit * (smallValue - lo));
				if (x >= rect.width()) break;
				if (x > 0) ruler.subPointPositions << x;

				smallValue += scale.smallStep;
			}
		}
		value += scale.bigStep;
	}

	return ruler;
}

inline TScale getXRuler(const QRect &rect, int fontMaxWidth, qreal unit, qreal lo, qreal hi, float s) {

	TScale ruler;
	
	TScaleSteps scale = getXScale(fontMaxWidth / unit, s);
	qreal value = qFloor(lo / scale.bigStep) * scale.bigStep;

	if (rect.width() > 0 && unit > 0) {
		qreal firstMajor = qFloor(lo / scale.bigStep) * scale.bigStep;
		if (firstMajor < lo)
			firstMajor += scale.bigStep;
		const int firstMajorX = qRound(unit * (firstMajor - lo));

		if (firstMajorX > fontMaxWidth) {
			ruler.mainPoints << lo;
			ruler.mainPointPositions << 0;
			if (scale.smallStep > 0) {
				for (qreal sv = lo + scale.smallStep; sv < firstMajor && sv < hi; sv += scale.smallStep) {
					const int x = qRound(unit * (sv - lo));
					if (x > 0 && x < rect.width())
						ruler.subPointPositions << x;
				}
			}
		}
	}

	while (value < hi) {

		int x = qRound(unit * (value - lo));

		if (x >= rect.width()) break;
		if (x > 0) {

			ruler.mainPoints << value;
			ruler.mainPointPositions << x;
		}

		if (scale.smallStep > 0) {

			qreal smallValue = value + scale.smallStep;
			qreal smallUpperValue = value + scale.bigStep;

			while (smallValue < smallUpperValue && smallValue < hi) {

				int x = qRound(unit * (smallValue - lo));
				if (x >= rect.width()) break;
				if (x > 0) ruler.subPointPositions << x;

				smallValue += scale.smallStep;
			}
		}
		value += scale.bigStep;
	}

	return ruler;
}

inline TScale getYRuler(const QRect &rect, int fontHeight, qreal unit, qreal lo, qreal hi) {

	TScale ruler;
	
	TScaleSteps scale = getYScale(fontHeight / unit, 10.0f);
	//qreal value = ceil(hi / scale.bigStep) * scale.bigStep;
	qreal value = qCeil(hi / scale.bigStep) * scale.bigStep;

	while (value >= lo) {

		int y = qRound(unit * -(value - hi));
		if (y > 0 && y < rect.height()) {

			if (ruler.mainPointPositions.length() < 100) {

				ruler.mainPoints << value;
				ruler.mainPointPositions << rect.top() + y;
			}
		}

		if (scale.smallStep > 0) {

			qreal smallValue = value - scale.smallStep;
			qreal smallEndValue = value - scale.bigStep;
			while (smallValue > smallEndValue && smallValue > lo) {

				int y = qRound(unit * -(smallValue - hi));
				if (y > 0 && y < rect.height())	{

					if (ruler.subPointPositions.length() < 200)
						ruler.subPointPositions << rect.top() + y;
				}
				smallValue -= scale.smallStep;
			}
		}
		value -= scale.bigStep;
	}

	return ruler;
}

inline TScale getYRuler2(const QRect &rect, int fontHeight, qreal unit, qreal lo, qreal hi) {

	TScale ruler;
	
	TScaleSteps scale = getYScale(fontHeight / unit, 10.0f);
	qreal value = qCeil(hi / scale.bigStep) * scale.bigStep;

	while (value >= lo) {

		int y = qRound(unit * -(value - hi));
		if (y > 0 && y < rect.height()) {

			if (ruler.mainPointPositions.length() < 100) {

				ruler.mainPoints << value;
				//ruler.mainPointPositions << rect.top() + y;
				ruler.mainPointPositions << y;
			}
		}

		if (scale.smallStep > 0) {

			qreal smallValue = value - scale.smallStep;
			qreal smallEndValue = value - scale.bigStep;
			while (smallValue > smallEndValue && smallValue > lo) {

				int y = qRound(unit * -(smallValue - hi));
				if (y > 0 && y < rect.height())	{

					if (ruler.subPointPositions.length() < 200)
						ruler.subPointPositions << y;
						//ruler.subPointPositions << rect.top() + y;
				}
				smallValue -= scale.smallStep;
			}
		}
		value -= scale.bigStep;
	}

	return ruler;
}

inline TScale getYRuler3(const QRect &rect, int fontHeight, qreal unit, qreal lo, qreal hi, float v) {

	TScale ruler;

	TScaleSteps scale = getYScale(fontHeight / unit, v);
	qreal value = qCeil(hi / scale.bigStep) * scale.bigStep;

	while (value >= lo) {

		int y = qRound(unit * -(value - hi));
		if (y > 0 && y < rect.height()) {

			if (ruler.mainPointPositions.length() < 100) {

				ruler.mainPoints << value;
				//ruler.mainPointPositions << rect.top() + y;
				ruler.mainPointPositions << y;
			}
		}

		if (scale.smallStep > 0) {

			qreal smallValue = value - scale.smallStep;
			qreal smallEndValue = value - scale.bigStep;
			while (smallValue > smallEndValue && smallValue > lo) {

				int y = qRound(unit * -(smallValue - hi));
				if (y > 0 && y < rect.height())	{

					if (ruler.subPointPositions.length() < 200)
						ruler.subPointPositions << y;
						//ruler.subPointPositions << rect.top() + y;
				}
				smallValue -= scale.smallStep;
			}
		}
		value -= scale.bigStep;
	}

	return ruler;
}

inline GLfloat dBmToGLPixel(const QRect &rect, qreal dBmMax, qreal dBmMin, qreal value) {

	GLfloat y;

	qreal yScale = rect.height() / qAbs(dBmMax - dBmMin);
	y = (GLfloat)(yScale * (dBmMax - value) + (qreal)rect.top());

	return y;
}

inline qreal glPixelTodBm(const QRect &rect, qreal dBmMax, qreal dBmMin, int position) {

	qreal dBm;

	float yScale = rect.height() / qAbs(dBmMax - dBmMin);
	dBm = dBmMax - (qreal)(position - rect.top())/yScale;
	//qreal dBm = m_dBmPanMax - ((m_dBmPanMax - m_dBmPanMin) * ((qreal)(position - rect.top()) / rect.height()));

	return dBm;
}


// Legacy fixed-function helpers (setProjectionOrthographic, drawGLRect, renderTexture,
// glBegin-based quads, etc.) were removed for OpenGL 3.3 Core Profile.
// Use GlDraw::* and QMatrix4x4 orthographic projections instead.

#endif // _CUSDR_OPENGLTYPES_H
