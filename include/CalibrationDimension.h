#ifndef CALIBRATIONDIMENSION_H
#define CALIBRATIONDIMENSION_H

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class CalibrationDimension {
public:

    CalibrationDimension() {};

    void setDimension(int index, string& name);
    //void setDomain(int domain_min, int domain_max);
    void setDomain(pair<int,int>& domain);
    bool setMinValue(int min_value);
    bool setMaxValue(int max_value);

    int getMinValue() const { return _min_value; }
    
    int getMaxValue() const { return _max_value; }

    int getDomainMin() const { return _domain_min; }

    int getDomainMax() const { return _domain_max; }
    
    int getIndex() const { return _index; }

    string getName() const { return _name; }

    static void setDimensionNames(const vector<string>& dimension_names) { 
        _dimension_names.clear();
        _dimension_names = dimension_names; 
    }
    
    static void setDimensionDomains(const vector<pair<int,int>>& dimensiones_domain_value) { 
        _dimension_domain_value.clear();
        _dimension_domain_value = dimensiones_domain_value; 
    }
    static int getIndexOfName(string& name)
    {
        for (int i = 0; i < _dimension_names.size(); i++)
        {
            if (_dimension_names[i] == name)
            {
                return i;
            }
        }
        return -1;
    }

public: 
    static vector<string> _dimension_names;
    static vector<pair<int, int>> _dimension_domain_value;

private:
    string _name;
    int _index;    
    int _min_value;
    int _max_value;
    int _domain_min;
    int _domain_max;

};
#endif // CALIBRATIONDIMENSION_H
