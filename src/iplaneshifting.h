
#ifndef IPLANESHIFTING_H_
#define IPLANESHIFTING_H_

/**
 * Interface class for Plane shifting.  This class is used in the Cmgui callbacks
 * to reduce the visible API.
 */
class IPlaneShifting
{
public:

	/**
	 * Destructor.
	 */
	virtual ~IPlaneShifting() {}

	/**
	 * Sets a start position.
	 *
	 * @param	x	The x coordinate.
	 * @param	y	The y coordinate.
	 */
	virtual void SetStartPosition(unsigned int x, unsigned int y) = 0;

	/**
	 * Updates the position.
	 *
	 * @param	x	The x coordinate.
	 * @param	y	The y coordinate.
	 */
	virtual void UpdatePosition(unsigned int x, unsigned int y) = 0;

	/**
	 * Sets an end position.
	 *
	 * @param	x	The x coordinate.
	 * @param	y	The y coordinate.
	 */
	virtual void SetEndPosition(unsigned int x, unsigned int y) = 0;
};

#endif /* IPLANESHIFTING_H_ */


