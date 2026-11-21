#!/usr/bin/env python3
#
# make_soccerwindow2_svg.py -- create the SVG files of the soccerwindow2 icon
#
# Usage: python3 icons/app/make_soccerwindow2_svg.py
#
# The soccerwindow2 icon shows a soccer ball in front of a field of the
# formation editor (players connected by the triangles of the formation),
# both seen obliquely from above. The ball is a truncated icosahedron
# projected on a sphere, so its pattern and the perspective of the field
# are computed by this script. The following files are written:
#
#   svg/soccerwindow2.svg      the icon on the 32x32 grid (32px and larger)
#   svg/soccerwindow2.16.svg   the simplified icon for 16px
#
# Then run icons/generate.sh soccerwindow2 to create the PNG files.
# Only the standard library of Python 3 is used. See docs/icons.md.

import math
import os

PHI = (1 + 5 ** 0.5) / 2

NAVY = "#233f8f"       # the patches and the seams of the ball
EDGE = "#1b2a4a"       # the outline of the ball
GREEN = "#1fa01f"      # the field
LINE = "#f8f8f8"       # the lines of the field
RED = "#e02020"        # the triangles of the formation
YELLOW = "#ffd700"     # the players

# the direction of the ball (degrees): rotation to the right and tilt toward
# the viewer (the view from above)
YAW = 22.0
TILT = 28.0


#
# the truncated icosahedron
#

def norm(v):
    length = math.sqrt(sum(x * x for x in v))
    return tuple(x / length for x in v)


def dot(a, b):
    return sum(x * y for x, y in zip(a, b))


def cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def even_permutations(t):
    a, b, c = t
    return [(a, b, c), (b, c, a), (c, a, b)]


def all_signs(t):
    out = set()
    for sx in ((1, -1) if t[0] else (1,)):
        for sy in ((1, -1) if t[1] else (1,)):
            for sz in ((1, -1) if t[2] else (1,)):
                out.add((sx * t[0], sy * t[1], sz * t[2]))
    return out


def points(bases):
    """the unit vectors of all the even permutations and signs of the bases"""
    out = set()
    for b in bases:
        for p in even_permutations(b):
            for s in all_signs(p):
                out.add(tuple(round(x, 9) for x in s))
    return sorted(norm(p) for p in out)


# the vertices, and the directions of the pentagons (the vertices of an
# icosahedron) and the hexagons (the faces of the icosahedron)
VERTICES = points([(0, 1, 3 * PHI), (1, 2 + PHI, 2 * PHI), (PHI, 2, 2 * PHI + 1)])
PENTAGON_DIRECTIONS = points([(0, 1, PHI)])
HEXAGON_DIRECTIONS = points([(1, 1, 1), (0, PHI, 1 / PHI)])


def face(direction):
    """the vertices of the face in the direction, in the order around it"""
    m = max(dot(v, direction) for v in VERTICES)
    vs = [v for v in VERTICES if dot(v, direction) > m - 1e-6]
    u = norm(cross(direction, (0.3, 0.5, 0.7)))
    w = cross(direction, u)
    vs.sort(key=lambda v: math.atan2(dot(v, w), dot(v, u)))
    return vs


PENTAGONS = [face(d) for d in PENTAGON_DIRECTIONS]
HEXAGONS = [face(d) for d in HEXAGON_DIRECTIONS]
EDGES = sorted({(min(f[i], f[(i + 1) % len(f)]), max(f[i], f[(i + 1) % len(f)]))
                for f in PENTAGONS + HEXAGONS for i in range(len(f))})
assert len(VERTICES) == 60 and len(PENTAGONS) == 12 and len(HEXAGONS) == 20 and len(EDGES) == 90


def rotation_x(a):
    c, s = math.cos(a), math.sin(a)
    return lambda v: (v[0], v[1] * c - v[2] * s, v[1] * s + v[2] * c)


def rotation_y(a):
    c, s = math.cos(a), math.sin(a)
    return lambda v: (v[0] * c + v[2] * s, v[1], -v[0] * s + v[2] * c)


def rotation_z(a):
    c, s = math.cos(a), math.sin(a)
    return lambda v: (v[0] * c - v[1] * s, v[0] * s + v[1] * c, v[2])


def ball_rotation(yaw, tilt):
    """a pentagon faces the viewer (+z) with a vertex up, then the ball is
    rotated to the right by yaw and tilted toward the viewer by tilt"""
    p0 = norm((0, 1, PHI))
    align = rotation_x(math.atan2(p0[1], p0[2]))
    top = max((align(v) for v in face(p0)), key=lambda v: v[1])
    up = rotation_z(math.atan2(top[0], top[1]))
    r_yaw = rotation_y(math.radians(yaw))
    r_tilt = rotation_x(math.radians(tilt))
    return lambda v: r_tilt(r_yaw(up(align(v))))


def great_arc(a, b, n):
    """n points on the arc from a to b (b is not included)"""
    return [norm(tuple(a[k] + (b[k] - a[k]) * t / n for k in range(3))) for t in range(n)]


def fmt(p):
    return "%.2f,%.2f" % p


def ball(cx, cy, r, seam, outline, ident):
    """the SVG elements of the ball of the radius r at (cx, cy)"""
    rotate = ball_rotation(YAW, TILT)

    def project(v):
        x, y, z = rotate(v)
        if z < 0:  # behind the ball: move to the rim
            length = math.hypot(x, y)
            x, y = x / length, y / length
        return (cx + r * x, cy - r * y)

    out = ['<defs>',
           '  <clipPath id="%s-clip"><circle cx="%.2f" cy="%.2f" r="%.2f"/></clipPath>' % (ident, cx, cy, r),
           '  <radialGradient id="%s-shade" cx="%.2f" cy="%.2f" r="%.2f" gradientUnits="userSpaceOnUse">'
           % (ident, cx - 0.35 * r, cy - 0.4 * r, 1.45 * r),
           '    <stop offset="0" stop-color="#ffffff" stop-opacity="0.25"/>',
           '    <stop offset="0.45" stop-color="#ffffff" stop-opacity="0"/>',
           '    <stop offset="1" stop-color="#0b1633" stop-opacity="0.5"/>',
           '  </radialGradient>',
           '</defs>',
           '<circle cx="%.2f" cy="%.2f" r="%.2f" fill="#ffffff"/>' % (cx, cy, r),
           '<g clip-path="url(#%s-clip)">' % ident]
    for f in PENTAGONS:
        pts = []
        for i in range(5):
            pts += great_arc(f[i], f[(i + 1) % 5], 8)
        if all(rotate(p)[2] < 0 for p in pts):
            continue
        out.append('  <path d="M%s Z" fill="%s"/>' % (" L".join(fmt(project(p)) for p in pts), NAVY))
    seams = []
    for a, b in EDGES:
        current = []
        for p in great_arc(a, b, 10) + [b]:
            if rotate(p)[2] >= 0:
                current.append(project(p))
            else:
                if len(current) > 1:
                    seams.append(current)
                current = []
        if len(current) > 1:
            seams.append(current)
    out.append('  <path d="%s" fill="none" stroke="%s" stroke-width="%.2f" stroke-linecap="round" stroke-linejoin="round"/>'
               % (" ".join("M" + " L".join(fmt(p) for p in s) for s in seams), NAVY, seam))
    out.append('  <circle cx="%.2f" cy="%.2f" r="%.2f" fill="url(#%s-shade)"/>' % (cx, cy, r, ident))
    out.append('</g>')
    out.append('<circle cx="%.2f" cy="%.2f" r="%.2f" fill="none" stroke="%s" stroke-width="%.2f"/>'
               % (cx, cy, r - outline / 2, EDGE, outline))
    return out


#
# the field in perspective
#

def solve(a, b):
    """solve the linear equations a x = b by the Gauss-Jordan elimination"""
    n = len(b)
    m = [row[:] + [b[i]] for i, row in enumerate(a)]
    for c in range(n):
        p = max(range(c, n), key=lambda r: abs(m[r][c]))
        m[c], m[p] = m[p], m[c]
        for r in range(n):
            if r != c:
                k = m[r][c] / m[c][c]
                m[r] = [m[r][j] - k * m[c][j] for j in range(n + 1)]
    return [m[i][n] / m[i][i] for i in range(n)]


def homography(src, dst):
    """the projective transformation that maps the 4 points src to dst"""
    a, b = [], []
    for (x, y), (u, v) in zip(src, dst):
        a.append([x, y, 1, 0, 0, 0, -u * x, -u * y])
        b.append(u)
        a.append([0, 0, 0, x, y, 1, -v * x, -v * y])
        b.append(v)
    h = solve(a, b) + [1.0]

    def apply(p):
        x, y = p
        w = h[6] * x + h[7] * y + h[8]
        return ((h[0] * x + h[1] * y + h[2]) / w, (h[3] * x + h[4] * y + h[5]) / w)
    return apply


# the field, the players and the triangles of the fedit2 icon (app/svg/fedit2.svg)
FIELD = [(1, 4), (31, 4), (31, 28), (1, 28)]
FIELD_LINES = [[(16, 4), (16, 28)],
               [(1, 10.5), (6.5, 10.5), (6.5, 21.5), (1, 21.5)],
               [(31, 10.5), (25.5, 10.5), (25.5, 21.5), (31, 21.5)]]
CENTER_CIRCLE = [(16 + 3.5 * math.cos(t * math.pi / 18), 16 + 3.5 * math.sin(t * math.pi / 18)) for t in range(36)]
PLAYERS = [(5, 16), (10, 9), (10, 23), (18, 12), (18, 21), (25, 9), (25, 16), (25, 24)]
TRIANGLES = [[(5, 16), (10, 9), (10, 23)], [(10, 9), (18, 12), (10, 23)], [(18, 12), (18, 21), (10, 23)],
             [(18, 12), (25, 9), (25, 16)], [(18, 21), (25, 16), (25, 24)], [(18, 12), (25, 16), (18, 21)]]


def field(quad, player_radius):
    """the SVG elements of the field of fedit2 mapped to the quadrilateral quad
    (the upper left, upper right, lower right and lower left corners)"""
    h = homography(FIELD, quad)
    out = ['<path d="M%s Z" fill="%s"/>' % (" L".join(fmt(p) for p in quad), GREEN)]
    d = " ".join("M" + " L".join(fmt(h(p)) for p in line) for line in FIELD_LINES)
    d += " M" + " L".join(fmt(h(p)) for p in CENTER_CIRCLE) + " Z"
    out.append('<path d="%s" fill="none" stroke="%s" stroke-width="0.8" stroke-linejoin="round"/>' % (d, LINE))
    d = " ".join("M" + " L".join(fmt(h(p)) for p in t) + " Z" for t in TRIANGLES)
    out.append('<path d="%s" fill="none" stroke="%s" stroke-width="0.8" stroke-linejoin="round"/>' % (d, RED))
    for p in PLAYERS:
        q = h(p)
        ex = h((p[0] + 1, p[1]))
        ey = h((p[0], p[1] + 1))
        out.append('<ellipse cx="%.2f" cy="%.2f" rx="%.2f" ry="%.2f" fill="%s"/>'
                   % (q[0], q[1], player_radius * math.hypot(ex[0] - q[0], ex[1] - q[1]),
                      player_radius * math.hypot(ey[0] - q[0], ey[1] - q[1]), YELLOW))
    return out


def svg_file(size, comment, elements):
    lines = ['<?xml version="1.0" encoding="UTF-8"?>',
             '<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">'
             % (size, size, size, size),
             '  <!-- %s -->' % comment,
             '  <!-- Generated by make_soccerwindow2_svg.py. -->']
    lines += ["  " + e for e in elements]
    lines.append('</svg>')
    return "\n".join(lines) + "\n"


def main():
    svg_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "svg")

    # 32px and larger: the field in perspective and the ball with its shadow
    elements = field([(4.5, 3.5), (27.5, 3.5), (31.5, 23), (0.5, 23)], 2.0)
    elements.append('<ellipse cx="22.8" cy="29.6" rx="6" ry="1.6" fill="#000000" fill-opacity="0.25"/>')
    elements += ball(21.5, 23, 7.8, 0.55, 0.7, "ball")
    path = os.path.join(svg_dir, "soccerwindow2.svg")
    with open(path, "w") as f:
        f.write(svg_file(32, "soccerwindow2: a soccer ball in front of the field of the formation editor", elements))
    print("wrote " + path)

    # 16px: the field with the center line, two players and a line, and the ball
    elements = ['<path d="M2,1.5 H14 L16,11.5 H0 Z" fill="%s"/>' % GREEN,
                '<path d="M8,1.5 V11.5" fill="none" stroke="%s"/>' % LINE,
                '<path d="M3,6.5 L7,3.5" fill="none" stroke="%s"/>' % RED,
                '<rect x="1.5" y="5.5" width="3" height="3" rx="1" fill="%s"/>' % YELLOW,
                '<rect x="5.5" y="2" width="3" height="3" rx="1" fill="%s"/>' % YELLOW]
    elements += ball(10.5, 10.5, 5.5, 0.45, 0.6, "ball")
    path = os.path.join(svg_dir, "soccerwindow2.16.svg")
    with open(path, "w") as f:
        f.write(svg_file(16, "soccerwindow2 (16x16): a simplified version for the small size", elements))
    print("wrote " + path)


if __name__ == "__main__":
    main()
