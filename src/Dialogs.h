/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2008 Tim Edmonds
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <string>

class Widget;
class MenuItem;
class GameControl;

Widget* createMainMenu(GameControl* game);
Widget* createPlayOpts(GameControl* game);
Widget* createEditOpts(GameControl* game);
Widget* createColourDialog(GameControl* game, int n, const int* cols);
Widget* createToolDialog(GameControl* game);
Widget *createNextLevelDialog( GameControl* game );
Widget *createEditDoneDialog( GameControl* game );


#endif //DIALOG_H
