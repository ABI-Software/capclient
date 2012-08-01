/*
 * Config.h
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#ifndef CAPCLIENT_CONFIG_H_
#define CAPCLIENT_CONFIG_H_

namespace cap
{

#ifdef _MSC_VER
#define __func__ __FUNCTION__
#endif
#cmakedefine CAPCLIENT_VERSION_STRING "@CAPCLIENT_VERSION_STRING@"
#cmakedefine CAPCLIENT_DEFINITELY_NON_CLINICAL

} // end namespace cap

#endif /* CAPCLIENT_CONFIG_H_ */
