/**************************************************************************
* This file is part of the Fraqtive program.
* Copyright (C) 2005 Michal Mecinski.
* This program is licensed under the GNU General Public License.
**************************************************************************/

#ifndef SPLINE_H
#define SPLINE_H

#include <vector>
#include <qsize.h>
#include <qpoint.h>

/*!
* \brief A spline used to store gradient components waveforms.
*/
class Spline
{
public:
	/*!
	* \brief Constructor.
	*/
	Spline();

	/*!
	* \brief Destructor.
	*/
	~Spline();

public:
	/*!
	* \brief Get the number of nodes in the spline.
	*
	* \return The number of nodes.
	*/
	int getNodesCnt() const
	{
		return _array.size();
	}

	/*!
	* \brief Remove all spline nodes.
	*/
	void clear()
	{
		_array.clear();
	}

	/*!
	* \brief Find a node at the specified position.
	*
	* \param gradSize The size of the gradient's area.
	* \param ptPos    Position of the point close to the node.
	* \param ptSize   Size of the node.
	* \return         The index of node or -1 if not found.
	*/
	int findNode(QSize gradSize, QPoint ptPos, QSize ptSize) const;

	/*!
        * \brief Find a segment at the specified position.
	*
	* \param gradSize The size of the gradient's area.
	* \param ptPos    Position of the point close to the node.
	* \param ptSize   Size of the node.
        * \param p1       Pointer to the index of first node or -1 if not found
        * \param p2       Pointer to the index of second node or -1 if not found
	*/
	void findSegment(QSize gradSize, QPoint ptPos, int& p1, int& p2) const;



	/*!
	* \brief Add a node to the spline.
	*
	* \note The nodes must be added in order of non-descending x coordinates.
	* The valid range of both coordinates is [0..1].
	*
	* \param x The x coordinate of the node.
	* \param y The y coordinate of the node.
	*/
	void addNode(double x, double y)
	{
		_array.push_back(Coord(x, y));
	}

	/*!
	* \brief Insert a node into the spline.
	*
	* \param x The x coordinate of the node.
	* \param y The y coordinate of the node.
	* \return  The index of the inserted node.
	*/
	int insertNode(double x, double y);

	/*!
	* \brief Change the position of a node.
	*
	* \param index The index of the node.
	* \param x     The new x coordinate of the node.
	* \param y     The new y coordinate of the node.
	*/
	void setNode(int index, double x, double y)
	{
		_array[index]._x = x;
		_array[index]._y = y;
	}

	/*!
	* \brief Remove a node.
	*
	* \param index The index of the node to remove.
	*/
	void removeNode(int index)
	{
		_array.erase(_array.begin() + index);
	}

	/*!
	* \brief Insert a node at the given position.
	*
	* \param index The index of the position to insert the node.
	*/
	void insertAt(int index)
	{
		_array.insert(_array.begin() + index, Coord());
	}

	/*!
	* \brief Get the x coordinate of the given node.
	*
	* \param index The index of the node.
	* \return      The horizontal position of the node.
	*/
	double getNodeX(int index) const
	{
		return _array[index]._x;
	}

	/*!
	* \brief Get the y coordinate of the given node.
	*
	* \param index The index of the node.
	* \return      The vertical position of the node.
	*/
	double getNodeY(int index) const
	{
		return _array[index]._y;
	}

	/*!
	* \brief Get the minimum x coordinate of the given node.
	*
	* \param index The index of the node.
	* \return      The minimum allowed position of the node.
	*/
	double getLimitMin(int index) const
	{
		if (index == 0)
			return 0.0;
		return _array[index - 1]._x;
	}

	/*!
	* \brief Get the maximum x coordinate of the given node.
	*
	* \param index The index of the node.
	* \return      The maximum allowed position of the node.
	*/
	double getLimitMax(int index) const
	{
		if (index == (int)_array.size() - 1)
			return 1.0;
		return _array[index + 1]._x;
	}

	/*!
	* \brief Invert the nodes in the spline.
	*/
	void invert();

	/*!
	* \brief Generate the spline waveform.
	*
	* \param[out] buffer The array to fill.
	* \param count       The size of the array.
	*/
	void generateSpline(double* buffer, int count) const;

	/*!
	* \brief Encode the spline data as a string.
	*
	* \return The encoded spline data.
	*/
	QString toString() const;

	/*!
	* \brief Decode the spline data from a string.
	*
	* \param string The encoded spline data.
	*/
	void fromString(const QString& string);

private:
	struct Coord
	{
		Coord()
		{
		}

		Coord(double x, double y) : _x(x), _y(y)
		{
		}

		double _x, _y;
	};

	typedef std::vector<Coord> CoordArray;

	CoordArray _array;
};

#endif
