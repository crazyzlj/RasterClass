/*!
 * \brief Implementation of clsRasterData class
 *
 * 1. Using GDAL and MongoDB (currently, mongo-c-driver 1.5.0)
 * 2. Array1D and Array2D raster data are supported
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date Apr. 2011
 * \revised May. 2016
 *
 */
#ifndef CLS_RASTER_DATA
#include "clsRasterData.h"

/************* Construct functions ***************/

template<typename T>
inline clsRasterData<T>::clsRasterData(void) {
    m_rasterPositionData = NULL;
    m_rasterData = NULL;
    m_mask = NULL;
    m_is2DRaster = false;
    m_raster2DData = NULL;
	m_calcPositions = true;
	m_useMaskExtent = true;
}

template<typename T>
clsRasterData<T>::clsRasterData(string filename, clsRasterData<int> *mask, bool calcPositions, bool useMaskExtent) {
    m_rasterPositionData = NULL;
    m_rasterData = NULL;
    m_fileName = filename;
    m_calcPositions = calcPositions;
	m_useMaskExtent = useMaskExtent;
    m_is2DRaster = false;
    m_raster2DData = NULL;
    if (StringMatch(GetUpper(GetSuffix(filename)), ASCIIExtension))
        ReadASCFile(filename, mask, calcPositions, useMaskExtent);
//    else
//        ReadFromGDAL(filename, excludeNODATA, mask);
}

template<typename T>
clsRasterData<T>::~clsRasterData(void) {
    if (m_rasterData != NULL) Release1DArray(m_rasterData);
    if (m_rasterPositionData != NULL) Release2DArray(m_nCells, m_rasterPositionData);
    if (m_raster2DData != NULL) Release2DArray(m_nCells, m_raster2DData);
}

/************* Get information functions ***************/

template<typename T>
float clsRasterData<T>::getAverage() {
    double temp = 0.;
    for (int i = 0; i < m_nCells; i++) {
        temp += m_rasterData[i];
    }
    return float(temp / m_nCells);
}

template<typename T>
float clsRasterData<T>::getAverage(int lyr) {
    if (!m_is2DRaster && m_nLyrs == 1)
        return getAverage();
    else {
        if (lyr < m_nLyrs && m_raster2DData != NULL) {
            double temp = 0.;
            for (int i = 0; i < m_nCells; i++) {
                temp += m_raster2DData[i][lyr];
            }
            return float(temp / m_nCells);
        } else
            throw ModelException("clsRasterData", "getAverage",
                                 "The given layer number is exceed the maximum layers.\n");
    }
}

template<typename T>
int clsRasterData<T>::getPosition(int row, int col) {
    if (m_rasterPositionData == NULL) return -1;
    for (int i = 0; i < m_nCells; i++) {
        if (row == m_rasterPositionData[i][0] && col == m_rasterPositionData[i][1]) return i;
    }
    return -1;
}

template<typename T>
int clsRasterData<T>::getPosition(float x, float y) {
    return getPosition((double) x, (double) y);
}

template<typename T>
int clsRasterData<T>::getPosition(double x, double y) {
    double xllCenter = this->getXllCenter();
    double yllCenter = this->getYllCenter();
    float dx = this->getCellWidth();
    float dy = this->getCellWidth();
    int nRows = this->getRows();
    int nCols = this->getCols();

    double xmin = xllCenter - dx / 2.;
    double xMax = xmin + dx * nCols;
    if (x > xMax || x < xllCenter) throw ModelException("Raster", "At", "The x coordinate is beyond the scale!");

    double ymin = yllCenter - dy / 2.;
    double yMax = ymin + dy * nRows;
    if (y > yMax || y < yllCenter) throw ModelException("Raster", "At", "The y coordinate is beyond the scale!");

    int nRow = (int) ((yMax - y) / dy); //calculate from ymax
    int nCol = (int) ((x - xmin) / dx); //calculate from xmin

    return getPosition(nRow, nCol);
}

template<typename T>
void clsRasterData<T>::getRasterData(int *nRows, T **data) {
    *nRows = m_nCells;
    *data = m_rasterData;
}

template<typename T>
void clsRasterData<T>::get2DRasterData(int *nRows, int *nCols, T ***data) {
    *nRows = m_nCells;
    *nCols = m_nLyrs;
    *data = m_raster2DData;
}

template<typename T>
map<string, double> *clsRasterData<T>::getRasterHeader() {
    if (m_mask != NULL) return m_mask->getRasterHeader();
    return &m_headers;
}

template<typename T>
void clsRasterData<T>::getRasterPositionData(int *nRows, int ***data) {
    if (m_mask != NULL)
        m_mask->getRasterPositionData(nRows, data);
    else {
        *nRows = m_nCells;
        *data = m_rasterPositionData;
    }
}

template<typename T>
T clsRasterData<T>::getValue(int validCellIndex) {
    if (m_rasterData == NULL)
        throw ModelException("Raster", "getValue", "Please first initialize the raster object.");
    if (m_nCells < validCellIndex)
        throw ModelException("Raster", "getValue",
                             "The index is too big! There are not so many valid cell in the raster.");
    return m_rasterData[validCellIndex];
}

template<typename T>
T *clsRasterData<T>::getValue(int validCellIndex, int *nLyrs) {
    if (m_nCells < validCellIndex)
        throw ModelException("Raster", "getValue",
                             "The index is too big! There are not so many valid cell in the raster.");
    if (m_is2DRaster) {
        if (m_raster2DData == NULL)
            throw ModelException("Raster", "getValue", "Please first initialize the 2D raster object.");
        T *cellValues = new float[m_nLyrs];
        for (int i = 0; i < m_nLyrs; i++)
            cellValues[i] = m_raster2DData[validCellIndex][i];
        *nLyrs = m_nLyrs;
        return cellValues;
    } else {
        if (m_rasterData == NULL)
            throw ModelException("Raster", "getValue", "Please first initialize the raster object.");
        *nLyrs = 1;
        T *cellValues = new float[1];
        cellValues[0] = m_rasterData[validCellIndex];
        return cellValues;
    }
}

template<typename T>
T clsRasterData<T>::getValue(clsRasterData *templateRasterData, T *rasterData, int row, int col) {
    if (templateRasterData == NULL || rasterData == NULL) return NODATA_VALUE;
    int position = templateRasterData->getPosition(row, col);
    if (position == -1) return NODATA_VALUE;
    return rasterData[position];
}

template<typename T>
T *clsRasterData<T>::getValue(clsRasterData *templateRasterData, T *rasterData, int row, int col, int *nLyrs) {
    if (templateRasterData == NULL || rasterData == NULL) return NULL;
    int position = templateRasterData->getPosition(row, col);
    if (position == -1) return NULL;
    return getValue(position, nLyrs);
}

template<typename T>
T clsRasterData<T>::getValue(int row, int col) {
    if (m_rasterData == NULL) return m_noDataValue;
    if (m_calcPositions && m_rasterPositionData != NULL){
        int validCellIndex = this->getPosition(row, col);
        if (validCellIndex == -1)
            return m_noDataValue;
        else
            return this->getValue(validCellIndex);
    } else
    {
        //if (row < 0 || row > this->getRows()) ModelException("clsRasterData", "ValidInput", "Row number is illegal!");
        //if (col < 0 || col > this->getCols()) ModelException("clsRasterData", "ValidInput", "col number is illegal!");
        if ((row < 0 || row > this->getRows()) || (col < 0 || col > this->getCols()))
            return m_noDataValue;
        return m_rasterData[row * this->getRows() + col];
    }
}

template<typename T>
T *clsRasterData<T>::getValue(int row, int col, int *nLyrs) {
    int validCellIndex = this->getPosition(row, col);
    if (validCellIndex == -1) {
        *nLyrs = -1;
        return NULL;
    } else {
        return getValue(validCellIndex, nLyrs);
    }
}
/************* Write functions ***************/

template<typename T>
void clsRasterData<T>::outputASCFile(string &filename) {
    if (m_is2DRaster)
        clsRasterData<T>::outputASCFile(m_headers, m_nCells, m_rasterPositionData, m_raster2DData, filename);
    else
        clsRasterData<T>::outputASCFile(m_headers, m_nCells, m_rasterPositionData, m_rasterData, filename);
}

template<typename T>
void
clsRasterData<T>::outputASCFile(map<string, double> header, int nRows, int **position, T *value, string filename) {
    ofstream rasterFile(filename.c_str());
    //write file
    int rows = int(header[HEADER_RS_NROWS]);
    int cols = int(header[HEADER_RS_NCOLS]);
    /// write header
    rasterFile << HEADER_RS_NCOLS << " " << cols << endl;
    rasterFile << HEADER_RS_NROWS << " " << rows << endl;
    rasterFile << HEADER_RS_XLL << " " << header[HEADER_RS_XLL] << endl;
    rasterFile << HEADER_RS_YLL << " " << header[HEADER_RS_YLL] << endl;
    rasterFile << HEADER_RS_CELLSIZE << " " << (float) header[HEADER_RS_CELLSIZE] << endl;
    rasterFile << HEADER_RS_NODATA << " " << setprecision(6) << header[HEADER_RS_NODATA] << endl;


    int index = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
			if (this->excludingNodata())
			{
				if (index < nRows) {
					if (position[index][0] == i && position[index][1] == j) {
						rasterFile << setprecision(6) << value[index] << " ";
						index++;
					} else rasterFile << setprecision(6) << NODATA_VALUE << " ";
				} else rasterFile << setprecision(6) << NODATA_VALUE << " ";
			} else{
				index = i * rows + j;
				rasterFile << setprecision(6) << value[index] << " ";
			}
        }
        rasterFile << endl;
    }
    rasterFile.close();
}

template<typename T>
void clsRasterData<T>::outputASCFile(map<string, double> header, int nRows, int **position, T **value,
                                     string filename) {
    string prePath = GetPathFromFullName(filename);
    string coreName = GetCoreFileName(filename);
    int nLyrs = (int) header[HEADER_RS_LAYERS];
    for (int lyr = 0; lyr < nLyrs; lyr++) {
        stringstream oss;
        oss << prePath << coreName << "_" << (lyr + 1) << ASCIIExtension;
        ofstream rasterFile(oss.str().c_str());

        //write file
        int rows = int(header[HEADER_RS_NROWS]);
        int cols = int(header[HEADER_RS_NCOLS]);
        /// write header
        rasterFile << HEADER_RS_NCOLS << " " << cols << endl;
        rasterFile << HEADER_RS_NROWS << " " << rows << endl;
        rasterFile << HEADER_RS_XLL << " " << header[HEADER_RS_XLL] << endl;
        rasterFile << HEADER_RS_YLL << " " << header[HEADER_RS_YLL] << endl;
        rasterFile << HEADER_RS_CELLSIZE << " " << (float) header[HEADER_RS_CELLSIZE] << endl;
        rasterFile << HEADER_RS_NODATA << " " << setprecision(6) << header[HEADER_RS_NODATA] << endl;


        int index = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (index < nRows) {
                    if (position[index][0] == i && position[index][1] == j) {
                        rasterFile << setprecision(6) << value[index][lyr] << " ";
                        index++;
                    } else rasterFile << setprecision(6) << NODATA_VALUE << " ";
                } else rasterFile << setprecision(6) << NODATA_VALUE << " ";
            }
            rasterFile << endl;
        }
        rasterFile.close();
    }
}

template<typename T>
void clsRasterData<T>::outputASCFile(clsRasterData *templateRasterData, T *value, string filename) {
    int nRows;
    float **position;
    templateRasterData->getRasterPositionData(&nRows, &position);
    clsRasterData<T>::outputASCFile(*(templateRasterData->getRasterHeader()), nRows, position, value, filename);
}

template<typename T>
void clsRasterData<T>::outputASCFile(clsRasterData *templateRasterData, T **value, string filename) {
    int nRows;
    int **position;
    templateRasterData->getRasterPositionData(&nRows, &position);
    clsRasterData<T>::outputASCFile(*(templateRasterData->getRasterHeader()), nRows, position, value, filename);
}

/************* Read functions ***************/

template<typename T>
void clsRasterData<T>::ReadASCFile(string ascFileName, clsRasterData<int> *mask, bool calcPositions) {
    utils util;
    if (!util.FileExists(ascFileName))
        throw ModelException("clsRasterData", "ReadASCFile",
                             "The file " + ascFileName + " does not exist or has not read permission.");
    StatusMessage(("Read " + ascFileName + "...").c_str());
    ifstream rasterFile(ascFileName.c_str());
    string tmp, xlls, ylls;
    T noData;
    double tempFloat;
    int rows, cols;

    /// read header
    rasterFile >> tmp >> cols;
    m_headers[HEADER_RS_NCOLS] = double(cols);
    rasterFile >> tmp >> rows;
    m_headers[HEADER_RS_NROWS] = double(rows);
    rasterFile >> xlls >> tempFloat;
    m_headers[HEADER_RS_XLL] = tempFloat;
    rasterFile >> ylls >> tempFloat;
    m_headers[HEADER_RS_YLL] = tempFloat;
    rasterFile >> tmp >> tempFloat;
    m_headers[HEADER_RS_CELLSIZE] = tempFloat;
    rasterFile >> tmp >> noData;
    m_headers[HEADER_RS_NODATA] = noData;
	m_noDataValue = (T) noData;
    /// default is center, if corner, then:
    if (StringMatch(xlls, "XLLCORNER")) m_headers[HEADER_RS_XLL] += 0.5 * m_headers[HEADER_RS_CELLSIZE];
    if (StringMatch(ylls, "YLLCORNER")) m_headers[HEADER_RS_YLL] += 0.5 * m_headers[HEADER_RS_CELLSIZE];

    /// get all raster values (i.e., include NODATA_VALUE, m_excludeNODATA = False)
    m_nCells = rows * cols;
    m_rasterData = new T[m_nCells];
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            rasterFile >> tempFloat;
            m_rasterData[i * cols + j] = (T) tempFloat;
        }
    }
    rasterFile.close();
	/******** ASCII file read done ********/

	/******** Mask and calculate valid positions ********/
	if (mask != NULL) {
		m_mask = mask;
		extractByMask();
	}
	calculateValidPositions();
}

/************* Utility functions ***************/

template<typename T>
double* clsRasterData<T>::getCoordinateByRowCol(int row, int col) {
    double xllCenter = this->getXllCenter();
    double yllCenter = this->getYllCenter();
    double cs = this->getCellWidth();
    double nrows = this->getRows();
    double* xy = new double[2];
    xy[0] = xllCenter + col * cs;
    xy[1] = yllCenter + (nrows - row - 1) * cs;
    return xy;
}

template<typename T>
int* clsRasterData<T>::getPositionByCoordinate(double x, double y) {
    double xllCenter = this->getXllCenter();
    double yllCenter = this->getYllCenter();
    float dx = this->getCellWidth();
    float dy = this->getCellWidth();
    int nRows = this->getRows();
    int nCols = this->getCols();

    int *position = new int[2];

    double xmin = xllCenter - dx / 2.;
    double xMax = xmin + dx * nCols;

    double ymin = yllCenter - dy / 2.;
    double yMax = ymin + dy * nRows;
    if ((x > xMax || x < xllCenter) || (y > yMax || y < yllCenter)) {
        position[0] = -1;
        position[0] = -1;
    } else{
        position[0] = (int) ((yMax - y) / dy); //calculate from ymax
        position[1] = (int) ((x - xmin) / dx); //calculate from xmin
    }
    return position;
}

template<typename T>
void clsRasterData<T>::copyHeader(map<string, double> *maskHeader) {
    m_headers[HEADER_RS_NCOLS] = (*maskHeader)[HEADER_RS_NCOLS];
    m_headers[HEADER_RS_NROWS] = (*maskHeader)[HEADER_RS_NROWS];
    m_headers[HEADER_RS_NODATA] = (*maskHeader)[HEADER_RS_NODATA];
    m_headers[HEADER_RS_CELLSIZE] = (*maskHeader)[HEADER_RS_CELLSIZE];
    m_headers[HEADER_RS_XLL] = (*maskHeader)[HEADER_RS_XLL];
    m_headers[HEADER_RS_YLL] = (*maskHeader)[HEADER_RS_YLL];
}

template<typename T>
void clsRasterData<T>::extractByMask(){
	if (m_mask == NULL) return;

}

template<typename T>
void clsRasterData<T>::calculateValidPositions(){
	if (!m_calcPositions) return;
	if (m_rasterPositionData != NULL) Release2DArray(m_nCells, m_rasterPositionData);
	// m_rasterData is nrows*ncols, and NODATA included.
	vector<T> values;
	vector<int> positionRows;
	vector<int> positionCols;
	/// get all valid values (i.e., exclude NODATA_VALUE)
	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {
			tempFloat = m_rasterData[i * cols + j];
			if (DoubleEqual(tempFloat, double(noData))) continue;
			values.push_back((T) tempFloat);
			positionRows.push_back(i);
			positionCols.push_back(j);
		}
	}
	/// create float array
	m_nCells = (int) values.size();
	Release1DArray(m_rasterData);
	m_rasterData = new T[m_nCells];
	m_rasterPositionData = new int *[m_nCells];
	for (int i = 0; i < m_nCells; ++i) {
		m_rasterData[i] = values.at(i);
		m_rasterPositionData[i] = new int[2];
		m_rasterPositionData[i][0] = positionRows.at(i);
		m_rasterPositionData[i][1] = positionCols.at(i);
	}
}
#endif
