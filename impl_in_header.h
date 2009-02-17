/*
 * impl_in_header.h
 *
 *  Created on: Feb 15, 2009
 *      Author: jchu014
 */

#ifndef IMPL_IN_HEADER_H_
#define IMPL_IN_HEADER_H_

class tesss
{
	void print_tess() // inline!!
	{
		printf("eee\n"); 
	}
};// no linking error: linker automatically removes duplicate symbol definitions (inline)

/*
void print()
{
	printf("ss");
}// this causes linking error ld: dupliacate symbol print()
*/

inline void print()
{
	printf("ss");
}// this is ok

#endif /* IMPL_IN_HEADER_H_ */
