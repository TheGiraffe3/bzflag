/* bzflag
 * Copyright (c) 1993-2023 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * HUDuiTypeIn:
 *  User interface class for the heads-up display's editable input control.
 */

#ifndef __HUDUITYPEIN_H__
#define __HUDUITYPEIN_H__

#include "common.h"

/* system interface headers */
#include <string>

/* common interface headers */
#include "BzfEvent.h"
#include "HUDuiControl.h"


typedef std::string (*TypeInColorFunc)(const std::string&);


class HUDuiTypeIn : public HUDuiControl
{
public:
    HUDuiTypeIn();
    ~HUDuiTypeIn();

    void        setObfuscation(bool on);
    size_t      getMaxLength() const;
    std::string     getString() const;

    void        setMaxLength(size_t);
    void        setString(const std::string&);
    void        setEditing(bool _allowEdit);
    void        setColorFunc(TypeInColorFunc func)
    {
        colorFunc = func;
    }

protected:
    bool        doKeyPress(const BzfKeyEvent&);
    bool        doKeyRelease(const BzfKeyEvent&);
    void        doRender();

private:
    size_t          maxLength;
    std::string     string;
    size_t          cursorPos;
    bool        allowEdit;
    bool        obfuscate;
    TypeInColorFunc colorFunc;
};

#endif // __HUDUITYPEIN_H__

// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
