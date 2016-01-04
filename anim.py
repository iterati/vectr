def strobe(numc, pick, skip, repeat, st, bt, lt):
    tick = trip = cidx = cntr = segm = 0
    init = True

    repeat = max(1, repeat)
    pick = pick or numc
    skip = skip or pick

    while True:
        if (tick >= trip):
            tick = trip = 0
            while trip == 0:
                if not init:
                    segm += 1
                    if segm >= ((2 * pick) + 1):
                        segm = 0
                        cntr += 1
                        if cntr >= repeat:
                            cntr = 0
                            cidx += skip
                            if cidx >= numc:
                                cidx = 0 if pick == skip else cidx % numc

                if (segm == 2 * pick):
                    trip = lt
                    rtn = -1
                elif segm % 2 == 0:
                    trip = st
                    rtn = (segm / 2) + cidx
                else:
                    trip = bt
                    rtn = -1

                init = False

        tick += 1
        if rtn >= numc:
            rtn = -1 if pick == skip else rtn % numc

        yield rtn


def vexer(numc, repeat_c, repeat_t, cst, cbt, tst, tbt):
    tick = trip = cidx = cntr = segm = 0
    init = True

    repeat_c = max(1, repeat_c)
    repeat_t = max(1, repeat_t)

    while True:
        if (tick >= trip):
            tick = trip = 0
            while trip == 0:
                if not init:
                    segm += 1
                    if segm >= 2 * (repeat_c + repeat_t + 1):
                        segm = 0
                        cidx = (cidx + 1) % (numc - 1)

                if segm < (2 * repeat_c) + 1:
                    if segm % 2 == 0:
                        trip = cbt
                        rtn = -1
                    else:
                        trip = cst
                        rtn = cidx + 1
                else:
                    if segm % 2 == 1:
                        trip = tbt
                        rtn = -1
                    else:
                        trip = tst
                        rtn = 0

                init = False

        tick += 1
        yield rtn


def double(numc, repeat_c, repeat_d, skip, cst, cbt, dst, dbt):
    tick = trip = cidx = cntr = segm = 0
    init = True

    numc = max(1, numc);
    repeat_c = max(1, repeat_c)
    repeat_d = max(1, repeat_d)
    skip = min(numc - 1, skip)

    while True:
        if tick == 0 or tick >= trip:
            tick = trip = 0
            while trip == 0:
                if not init:
                    segm += 1
                    if segm >= 2 * (repeat_c + repeat_d):
                        segm = 0
                        cidx = (cidx + 1) % numc

                if segm < 2 * repeat_d:
                    if segm % 2 == 0:
                        trip = dst
                        rtn = (cidx + skip) % numc
                    else:
                        trip = dbt
                        rtn = -1
                else:
                    if segm % 2 == 0:
                        trip = cst
                        rtn = cidx
                    else:
                        trip = cbt
                        rtn = -1

                init = False

        tick += 1
        yield rtn


def edge(numc, pick, cst, cbt, est, ebt):
    tick = trip = cidx = cntr = segm = 0
    init = True

    numc = min(max(1, numc), 9)
    pick = pick or numc

    while True:
        if (tick >= trip):
            tick = trip = 0
            while trip == 0:
                if not init:
                    segm += 1
                    if segm >= ((4 * pick) - 2):
                        segm = 0
                        cidx += pick
                        if cidx >= numc:
                            cidx = 0

                if segm == 0:
                    trip = ebt
                    rtn = -1
                elif segm == (2 * pick) - 1:
                    trip = est
                    rtn = 0
                elif segm % 2 == 0:
                    trip = cbt
                    rtn = -1
                else:
                    trip = cst
                    rtn = abs((segm / 2) - (pick - 1))

                init = False

        tick += 1
        if rtn > numc:
            rtn = -1
        yield rtn


def runner(numc, repeat, cst, cbt, rst, rbt):
    tick = trip = cidx = cntr = segm = 0
    init = True

    numc = min(max(1, numc), 9)
    repeat = repeat or numc - 1

    while True:
        if tick >= trip:
            tick = trip = 0
            while trip == 0:
                segm += 1
                if not init:
                    segm = (segm + 1) % (2 * (repeat + (numc - 1) + 1))

                if segm < (2 * (numc - 1)) + 1:
                    if segm % 2 == 0:
                        trip = cbt
                        rtn = -1
                    else:
                        trip = cst
                        rtn = (segm / 2) + 1
                else:
                    if segm % 2 == 1:
                        trip = rbt
                        rtn = 0
                    else:
                        trip = rst
                        rtn = -1

                init = False

        tick += 1
        yield rtn
