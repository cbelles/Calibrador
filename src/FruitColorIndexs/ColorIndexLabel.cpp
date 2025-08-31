#include "FruitColorIndexs/ColorIndexLabel.h"
#include "FruitColorIndexs/ColorIndexBar.h"


ColorIndexLabel::ColorIndexLabel(QWidget* parent, Qt::WindowFlags f) :QLabel(parent, f)
{
}

void ColorIndexLabel::create(std::string& colorindex, int rows)
{
	_cols = 256;
	_rows = rows;
	_imagecolorbarRGB.create(_rows, _cols, CV_8UC3);

	std::vector<std::vector<unsigned char>> bar;
	ColorIndexBar colorindexbar;
	colorindexbar.setColorIndex(colorindex);
	colorindexbar.getColorIndexBar(bar);

	cv::Mat linebase(1, _cols, CV_8UC3);
	unsigned char* p_linebase = linebase.ptr(0);
	for (int i = 0; i < _cols; i++)
	{
		p_linebase[i * 3 + 0] = bar[i][0];
		p_linebase[i * 3 + 1] = bar[i][1];
		p_linebase[i * 3 + 2] = bar[i][2];
	}

	uchar* src = linebase.ptr(0);
 	for (int i = 0; i < _rows; i++)
	{
		uchar* dst = _imagecolorbarRGB.ptr(i);
		memcpy(dst, src, _cols * 3);
	}
}

void ColorIndexLabel::mousePressEvent(QMouseEvent* ev)
{
	const QPoint p = ev->pos();
	emit mousePressed(p);
}

void ColorIndexLabel::paintBar()
{
	setPixmap(QPixmap::fromImage(QImage(_imagecolorbarRGB.data, _imagecolorbarRGB.cols, _imagecolorbarRGB.rows, _imagecolorbarRGB.step, QImage::Format_RGB888)));
}




