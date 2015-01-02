import subprocess
import sys
import re
import collections

LICENSE_BLOCK_RE = r'/[*].*?This program is free software.*?\*/'

HEADER_TEMPLATE = """/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * <<COPYRIGHT>>
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
 */"""

def update(filename):
    d = subprocess.check_output(['git', 'log', '--format=%an <%ae>|%ad', '--follow', filename])

    dd = collections.defaultdict(list)
    for author, date in map(lambda x: x.split('|'), d.decode('utf-8').splitlines()):
        dd[author].append(date)

    def combine(s, date):
        # "Sat Feb 18 12:58:00 2012 -0800"
        s.add(date.split()[4])
        return s

    for author in dd:
        dd[author] = sorted(reduce(combine, dd[author], set()))

    def year_sort(item):
        _, years = item
        return tuple(map(int, years))

    def inject():
        for line in HEADER_TEMPLATE.splitlines():
            line = line.rstrip('\n')
            if '<<COPYRIGHT>>' in line:
                for author, years in sorted(dd.items(), key=year_sort):
                    copyright = 'Coyright (c) {years} {author}'.format(years=', '.join(years), author=author)
                    yield line.replace('<<COPYRIGHT>>', copyright)
                continue
            yield line

    license = '\n'.join(inject())

    d = open(filename).read()
    if re.search(LICENSE_BLOCK_RE, d, re.DOTALL) is None:
        open(filename, 'w').write(license + '\n\n' + d)
    else:
        d = re.sub(LICENSE_BLOCK_RE, license, d, 0, re.DOTALL)
        open(filename, 'w').write(d)

for filename in sys.argv[1:]:
    print 'Updating:', filename
    update(filename)
