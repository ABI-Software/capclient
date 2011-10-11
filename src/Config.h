/*
 * Config.h
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#ifndef CONFIG_H_
#define CONFIG_H_

namespace cap
{

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif
static const char* CAP_DATA_DIR = "Data/";

} // end namespace cap

#endif /* CONFIG_H_ */
