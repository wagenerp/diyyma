
/** \file config.h
  * \author Peter Wagener
  * \brief Configuration stuff. For configuration purposes.
  *
  */

#ifndef _DIYYMA_CONFIG_H
#define _DIYYMA_CONFIG_H

/** \brief Set to non-zero to enable OpenIL texture loading.
  * 
  * May be useful to turn this off in any final build to keep the
  * library overhead small.
  */
#define DIYYMA_TEXTURE_IL 1

#define DIYYMA_RC_NO_AUTODELETE 0
#define DIYYMA_RC_GLOBAL_LIST 0

#define DEBUG_DIYYMA_SPLINES

#endif