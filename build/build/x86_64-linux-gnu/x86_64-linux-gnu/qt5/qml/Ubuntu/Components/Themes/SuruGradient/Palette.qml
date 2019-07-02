/*
 * Copyright 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Ubuntu.Components.Themes 0.1

Palette {
    normal: PaletteValues {
        background: "#5E2750"
        backgroundText: "#33F3F3E7"
        base: "#19000000"
        baseText: "#F3F3E7"
        foreground: "#888888"
        foregroundText: "#F3F3E7"
        overlay: "#F5F5F5"
        overlayText: "#888888"
        field: "#19000000"
        fieldText: "#7F7F7F7F"
    }
    selected: PaletteValues {
        background: "#88D6D6D6" // FIXME: not from design
        backgroundText: "#F3F3E7"
        foreground: "#DD4814"
        foregroundText: "#F3F3E7"
        field: "#FFFFFF"
        fieldText: "#888888"
    }
}
