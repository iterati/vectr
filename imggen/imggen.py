from PIL import Image


def interp(s, e, t, d):
    return s + int((e - s) * (t / float(d)))


def make_block(color, length, height=8):
    return Image.new("RGB", (length, height), color)


class Pattern(object):
    def __init__(self, name, **kwargs):
        self.name = name
        self.args = kwargs.get('args', [1, 0, 0, 0])
        self.timings = kwargs.get('timings', [
            [0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0, 0],
        ])
        self.thresh = kwargs.get('thresh', [
            [64, 64, 64, 64],
            [64, 64, 64, 64],
        ])
        self.colors = kwargs.get('colors', [
            [
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
            ],
            [
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
            ],
            [
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
                [0, 0, 0],
            ],
        ])
        self.numc = kwargs.get('numc', [1, 1, 1])

    def _get_timing_at(self, idx, velocity):
        if velocity < self.thresh[0][0]:
            s = 0
            t = 0
            d = 1
        elif velocity < self.thresh[0][1]:
            s = 0
            t = velocity - self.thresh[0][0]
            d = self.thresh[0][1] - self.thresh[0][0]
        elif velocity < self.thresh[0][2]:
            s = 1
            t = 0
            d = 1
        elif velocity < self.thresh[0][3]:
            s = 1
            t = velocity - self.thresh[0][2]
            d = self.thresh[0][3] - self.thresh[0][2]
        else:
            s = 1
            t = 1
            d = 1

        return interp(self.timings[s][idx], self.timings[s + 1][idx], t, d)

    def _get_timings(self, velocity):
        return [self._get_timing_at(i, velocity) for i in range(8)]

    def _get_numc(self, velocity):
        if velocity < self.thresh[1][0]:
            return self.numc[0]
        elif velocity < self.thresh[1][1]:
            return min(self.numc[0:2])
        elif velocity < self.thresh[1][2]:
            return self.numc[1]
        elif velocity < self.thresh[1][3]:
            return min(self.numc[1:3])
        else:
            return self.numc[2]

    def _get_color_at(self, idx, velocity):
        if velocity < self.thresh[1][0]:
            s = 0
            t = 0
            d = 1
        elif velocity < self.thresh[1][1]:
            s = 0
            t = self.thresh[1][1] - velocity
            d = self.thresh[1][1] - self.thresh[1][0]
        elif velocity < self.thresh[1][2]:
            s = 1
            t = 0
            d = 1
        elif velocity < self.thresh[1][3]:
            s = 1
            t = self.thresh[1][3] - velocity
            d = self.thresh[1][3] - self.thresh[1][2]
        else:
            s = 1
            t = 1
            d = 1

        return (
            interp(self.colors[s][idx][0], self.colors[s + 1][idx][0], t, d),
            interp(self.colors[s][idx][1], self.colors[s + 1][idx][1], t, d),
            interp(self.colors[s][idx][2], self.colors[s + 1][idx][2], t, d),
        )

    def _get_colors(self, velocity):
        return [self._get_color_at(i, velocity) for i in range(9)]

    def render(self, name):
        img = Image.new("RGB", (1280, 650), (0, 0, 0))
        for velocity in range(65):
            strip = self.render_for_velocity(velocity)
            img.paste(strip, (0, (10 * velocity) + 9))

        img.save(self.name + ".png")

    def render_for_velocity(self, velocity):
        raise NotImplementedError


class PatternStrobe(Pattern):
    def render_for_velocity(self, velocity):
        numc = self._get_numc(velocity)
        timings = self._get_timings(velocity)
        colors = self._get_colors(velocity)

        pick = self.args[0] or numc
        skip = self.args[1] or pick
        repeat = self.args[2] or 1

        img = Image.new("RGB", (1280, 8), (0, 0, 0))

        i = 0
        segm = 0
        cntr = 0
        cidx = 0

        while i < 1280:
            if segm == 0:
                length = timings[2]
            elif segm % 2 == 1:
                length = timings[0]
            else:
                length = timings[1]

            show_blank = segm % 2 == 0
            color = cidx + (segm / 2)
            if color >= numc:
                color %= numc

            if show_blank:
                color = (0, 0, 0)
            else:
                color = colors[color]

            block = make_block(color, length)
            img.paste(block, (i, 0))
            i += length

            segm += 1
            if segm >= (2 * pick) + 1:
                segm = 0
                cntr += 1
                if cntr >= repeat:
                    cntr = 0
                    cidx += skip
                    if cidx >= numc:
                        if pick == skip:
                            cidx = 0
                        else:
                            cidx %= numc

        return img


class PatternTracer(Pattern):
    def render_for_velocity(self, velocity):
        numc = self._get_numc(velocity)
        timings = self._get_timings(velocity)
        colors = self._get_colors(velocity)

        pick = self.args[0] or numc
        skip = self.args[1] or pick
        repeat = self.args[2] or 1

        img = Image.new("RGB", (1280, 8), (0, 0, 0))

        i = 0
        segm = 0
        cntr = 0
        cidx = 0

        while i < 1280:
            if segm == 0:
                length = timings[2]
            elif segm % 2 == 1:
                length = timings[0]
            else:
                length = timings[1]

            show_blank = segm % 2 == 0
            color = cidx + (segm / 2)
            if color >= numc:
                color %= numc

            if show_blank:
                color = (0, 0, 0)
            else:
                color = colors[color]

            block = make_block(color, length)
            img.paste(block, (i, 0))
            i += length

            segm += 1
            if segm >= (2 * pick) + 1:
                segm = 0
                cntr += 1
                if cntr >= repeat:
                    cntr = 0
                    cidx += skip
                    if cidx >= numc:
                        if pick == skip:
                            cidx = 0
                        else:
                            cidx %= numc

        return img


darkside = PatternStrobe(
    "Darkside of the Mode",
    args=[0, 0, 0, 0],
    timings=[
        [1, 0, 200, 0, 0, 0, 0, 0],
        [13, 0, 200, 0, 0, 0, 0, 0],
        [3, 50, 0, 0, 0, 0, 0, 0],
    ],
    numc=[6, 6, 1],
    colors=[
        [
            [48, 0, 48],
            [0, 0, 128],
            [0, 48, 48],
            [0, 128, 0],
            [48, 48, 0],
            [128, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
        ],
        [
            [96, 0, 96],
            [0, 0, 255],
            [0, 96, 96],
            [0, 255, 0],
            [96, 96, 0],
            [255, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
        ],
        [
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
        ],
    ],
    thresh=[
        [0, 16, 16, 58],
        [16, 58, 64, 64],
    ],
)
