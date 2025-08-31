#ifndef _COLORINDEXLABEL_H
#define _COLORINDEXLABEL_H

#include <QLabel>
#include <QMouseEvent>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class ColorIndexLabel : public QLabel
{
	Q_OBJECT

signals:
	void mousePressed(const QPoint&);

public:

	ColorIndexLabel(QWidget* parent = 0, Qt::WindowFlags f = 0);
	
	void create(std::string& colorindex, int rows);

	void mousePressEvent(QMouseEvent* ev);

	void paintBar();

private:

	int _cols;
	int _rows;

	cv::Mat _imagecolorbarRGB;

};

#endif //_COLORINDEXLABEL_H