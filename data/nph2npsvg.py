#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# nph2npsvg.py: Convert NumptyPhysics levels to SVG
# Thomas Perl <m@thp.io>; 2014-12-26
#

import re
import os
import sys

WIDTH = 800
HEIGHT = 480
URL = 'http://numptyphysics.garage.maemo.org/'

FLAGS = [
    'token',
    'goal',
    'fixed',
    'sleeping',
    'decor',
    'rope',
    'interactive',
]

# Taken from src/Colour.{h,cpp}
COLORS = [
    0xb80000,
    0xeec900,
    0x000077,
    0x108710,
    0x101010,
    0x8b4513,
    0x87cefa,
    0xee6aa7,
    0xb23aee,
    0x00fa9a,
    0xff7f00,
    0x6c7b8b,
]

class NumptyPhysicsLevel(object):
    def __init__(self):
        self.metadata = {}
        self.interactions = []
        self.strokes = []
        self.events = []

    def parse_nph(self, fp):
        for line in fp:
            key, value = (x.strip() for x in line.split(':', 1))
            if key[0] in ('T', 'A', 'B', 'G'):
                self.metadata[key] = value
            elif key[0] == 'E':
                self.events.append(value)
            elif key[0] == 'I':
                self.interactions.append(value.split(' = '))
            elif key[0] == 'S':
                stroke_type = key[1:]
                stroke_flags, stroke_color = re.match(r'([a-z]*)([0-9]*)', stroke_type).groups()
                stroke_color = int(stroke_color or 0)

                if 't' in stroke_flags:
                    stroke_color = 0
                elif 'g' in stroke_flags:
                    stroke_color = 1

                flags = [next(flag for flag in FLAGS if flag.startswith(c)) for c in stroke_flags]

                stroke_points = [tuple(map(int, part.split(','))) for part in value.split()]
                self.strokes.append((flags, stroke_color, stroke_points))
            else:
                raise NotImplementedError('Cannot parse line: {}: {}'.format(key, value))

    def write_svg(self, fp):
        stroke_width = 2

        print >>fp, '<svg width="{}" height="{}" xmlns:np="{}">'.format(WIDTH, HEIGHT, URL)
        print >>fp, '<rect x="0" y="0" width="{}" height="{}" fill="white" stroke="none" />'.format(WIDTH, HEIGHT)
        print >>fp, '<np:meta {} />'.format(' '.join('{}="{}"'.format(key.lower(), value)
                                                     for key, value in sorted(self.metadata.items())))

        for color, action in self.interactions:
            print >>fp, '<np:interaction np:color="{}" np:action="{}" />'.format(color, action)

        for stroke_flags, stroke_color, stroke_points in self.strokes:
            node = '<path class="{}" fill="none" stroke="{}" stroke-width="{}" d="M{}" />'
            print >>fp, node.format(' '.join(stroke_flags), '#%06x' % COLORS[stroke_color], stroke_width,
                                    'L'.join('%d %d' % (x, y) for x, y in stroke_points))

        for event in self.events:
            print >>fp, '<np:event value="{}" />'.format(event)

        print >>fp, '</svg>'


if __name__ == '__main__':
    for filename in sys.argv[1:]:
        level = NumptyPhysicsLevel()

        with open(filename) as fp:
            level.parse_nph(fp)

        base, ext = os.path.splitext(filename)
        outfile = base + '.npsvg'
        with open(outfile, 'w') as fp:
            level.write_svg(fp)
        print filename, '->', outfile
