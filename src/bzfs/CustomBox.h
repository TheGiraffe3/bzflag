/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
#ifndef __CUSTOMBOX_H__
#define __CUSTOMBOX_H__

/* interface header */
#include "WorldFileObstacle.h"

/* local implementation headers */
#include "WorldInfo.h"


class CustomBox : public WorldFileObstacle {
  public:
    CustomBox();
    virtual void writeToGroupDef(GroupDefinition*) const;
};

#endif  /* __CUSTOMBOX_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
