#ifndef _COLORINDEXBAR_H
#define _COLORINDEXBAR_H

#include <string>
#include <vector>

class ColorIndexBar 
{

public:

	ColorIndexBar();

	void setColorIndex(const std::string& colorindex);  // Cambiar a referencia constante
	void getColorIndex(std::string& colorindex) { colorindex = _colorindex; }

	bool getColorIndexBar(std::vector<std::vector<unsigned char>>& bar);
	void getColorValue(std::vector<unsigned char>& color, unsigned char index);

private:

	std::string _colorindex;
	const std::vector<std::vector<unsigned char>>* _currentBar;  
	static std::vector<std::vector<unsigned char> > _barHUE; 
	static std::vector<std::vector<unsigned char> > _barNARANJA;
	static std::vector<std::vector<unsigned char> > _barBLACKFIG;

};

#endif //_COLORINDEXBAR_H