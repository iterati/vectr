var DefaultModes = function() {
  var modes = [
  {
    name: "Darkside of the Mode",
    id: "darkside-of-the-mode",
    bundles: [0, 1],
    slot: 1,

    type: 0,
    pattern: [0, 0],
    args: [
      [0, 0, 0, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [1, 0, 50, 0, 0, 0, 0, 0],
      [13, 0, 50, 0, 0, 0, 0, 0],
      [3, 12, 0, 0, 0, 0, 0, 0]
    ],
    numc: [6, 6, 1],
    colors: [
      [
        [24, 0, 24],
        [0, 0, 64],
        [0, 24, 24],
        [0, 64, 0],
        [24, 24, 0],
        [64, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ],
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 48, 48, 64],
    thresh1: [0, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Dash Dops",
    id: "dash-dops",
    bundles: [0],
    slot: 6,

    type: 0,
    pattern: [1, 0],
    args: [
      [0, 0, 5, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [8, 0, 8, 40, 40, 40, 0, 0],
      [40, 40, 8, 0, 40, 40, 0, 0],
      [8, 40, 40, 0, 40, 40, 0, 0]
    ],
    numc: [6, 1, 1],
    colors: [
      [
        [0, 70, 45],
        [39, 70, 0],
        [78, 28, 0],
        [91, 0, 15],
        [52, 0, 60],
        [13, 0, 105],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [8, 32, 40, 64],
    thresh1: [64, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Eye Bleach",
    id: "eye-bleach",
    bundles: [1],
    slot: 6,

    type: 0,
    pattern: [2, 0],
    args: [
      [10, 0, 0, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [1, 0, 0, 90, 0, 0, 0, 0],
      [10, 0, 0, 0, 0, 0, 0, 0],
      [1, 0, 0, 0, 0, 0, 0, 0]
    ],
    numc: [8, 8, 8],
    colors: [
      [
        [0, 70, 45],
        [33, 36, 38],
        [0, 0, 0],
        [0, 0, 0],
        [39, 70, 0],
        [33, 36, 38],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ],
      [
        [0, 70, 45],
        [33, 36, 38],
        [0, 0, 0],
        [0, 0, 0],
        [91, 0, 15],
        [33, 36, 38],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ],
      [
        [0, 70, 45],
        [33, 36, 38],
        [0, 0, 0],
        [0, 0, 0],
        [13, 0, 105],
        [33, 36, 38],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 40, 64, 64],
    thresh1: [0, 24, 24, 40],
    trigger: 0
  },
  {
    name: "Eye Floss",
    id: "eye-floss",
    bundles: [1],
    slot: 3,

    type: 0,
    pattern: [2, 0],
    args: [
      [10, 0, 5, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [1, 0, 0, 190, 10, 0, 0, 0],
      [5, 5, 0, 100, 10, 0, 0, 0],
      [20, 0, 0, 0, 10, 0, 0, 0]
    ],
    numc: [6, 1, 1],
    colors: [
      [
        [13, 47, 140],
        [12, 0, 104],
        [78, 0, 30],
        [36, 10, 108],
        [0, 28, 90],
        [97, 22, 33],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 48, 48, 64],
    thresh1: [64, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Heat Wave",
    id: "heat-wave",
    bundles: [0, 1],
    slot: 4,

    type: 0,
    pattern: [4, 0],
    args: [
      [64, 2, 1, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [1, 0, 1, 0, 0, 0, 0, 0],
      [5, 0, 1, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0]
    ],
    numc: [9, 9, 1],
    colors: [
      [
        [0, 140, 90],
        [0, 14, 45],
        [2, 0, 14],
        [26, 0, 210],
        [0, 35, 23],
        [0, 3, 11],
        [0, 56, 180],
        [7, 0, 53],
        [0, 9, 6],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ],
      [
        [156, 0, 60],
        [46, 7, 0],
        [6, 7, 0],
        [104, 112, 0],
        [39, 0, 15],
        [11, 3, 0],
        [182, 28, 0],
        [26, 28, 0],
        [10, 0, 4],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [8, 64, 64, 64],
    thresh1: [8, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Betamorph",
    id: "betamorph",
    bundles: [0],
    slot: 0,

    type: 0,
    pattern: [2, 0],
    args: [
      [200, 0, 0, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [10, 0, 0, 0, 0, 0, 0, 0],
      [1, 9, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
    ],
    numc: [15, 15, 1],
    colors: [
      [
        [65, 0, 0],
        [52, 14, 0],
        [39, 28, 0],
        [26, 42, 0],
        [13, 56, 0],
        [0, 70, 0],
        [0, 56, 15],
        [0, 42, 30],
        [0, 28, 45],
        [0, 14, 60],
        [0, 0, 75],
        [13, 0, 60],
        [26, 0, 45],
        [39, 0, 30],
        [52, 0, 15],
        [0, 0, 0]
      ],
      [
        [195, 0, 0],
        [156, 42, 0],
        [117, 84, 0],
        [78, 126, 0],
        [39, 168, 0],
        [0, 210, 0],
        [0, 168, 45],
        [0, 126, 90],
        [0, 84, 135],
        [0, 42, 180],
        [0, 0, 225],
        [39, 0, 180],
        [78, 0, 135],
        [117, 0, 90],
        [156, 0, 45],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 64, 64, 64],
    thresh1: [0, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Metamorph",
    id: "metamorph",
    bundles: [1],
    slot: 0,

    type: 0,
    pattern: [2, 0],
    args: [
      [200, 0, 0, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [1, 9, 0, 0, 0, 0, 0, 0],
      [10, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
    ],
    numc: [15, 15, 1],
    colors: [
      [
        [195, 0, 0],
        [156, 42, 0],
        [117, 84, 0],
        [78, 126, 0],
        [39, 168, 0],
        [0, 210, 0],
        [0, 168, 45],
        [0, 126, 90],
        [0, 84, 135],
        [0, 42, 180],
        [0, 0, 225],
        [39, 0, 180],
        [78, 0, 135],
        [117, 0, 90],
        [156, 0, 45],
        [0, 0, 0]
      ],
      [
        [65, 0, 0],
        [52, 14, 0],
        [39, 28, 0],
        [26, 42, 0],
        [13, 56, 0],
        [0, 70, 0],
        [0, 56, 15],
        [0, 42, 30],
        [0, 28, 45],
        [0, 14, 60],
        [0, 0, 75],
        [13, 0, 60],
        [26, 0, 45],
        [39, 0, 30],
        [52, 0, 15],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 64, 64, 64],
    thresh1: [0, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Nebulous",
    id: "nebulous",
    bundles: [0, 1],
    slot: 5,

    type: 0,
    pattern: [8, 0],
    args: [
      [7, 0, 0, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [20, 1, 8, 5, 9, 6, 2, 4],
      [60, 1, 8, 5, 9, 6, 2, 4],
      [80, 1, 8, 5, 9, 6, 2, 4]
    ],
    numc: [4, 1, 1],
    colors: [
      [
        [13, 70, 75],
        [26, 28, 90],
        [65, 14, 75],
        [78, 28, 30],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 32, 32, 64],
    thresh1: [64, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Rainbow Strobe B",
    id: "rainbow-strobe-b",
    bundles: [1],
    slot: 2,

    type: 0,
    pattern: [0, 0],
    args: [
      [5, 1, 24, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [1, 9, 0, 0, 0, 0, 0, 0],
      [10, 0, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
    ],
    numc: [15, 15, 1],
    colors: [
      [
        [130, 0, 0],
        [104, 0, 30],
        [78, 0, 60],
        [52, 0, 90],
        [26, 0, 120],
        [0, 0, 150],
        [0, 28, 120],
        [0, 56, 90],
        [0, 84, 60],
        [0, 112, 30],
        [0, 140, 0],
        [26, 112, 0],
        [52, 84, 0],
        [78, 56, 0],
        [104, 28, 0],
        [0, 0, 0]
      ],
      [
        [195, 0, 0],
        [156, 0, 45],
        [117, 0, 90],
        [78, 0, 135],
        [39, 0, 180],
        [0, 0, 225],
        [0, 42, 180],
        [0, 84, 135],
        [0, 126, 90],
        [0, 168, 45],
        [0, 210, 0],
        [39, 168, 0],
        [78, 126, 0],
        [117, 84, 0],
        [156, 42, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 64, 64, 64],
    thresh1: [0, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Rainbow Strobe",
    id: "rainbow-strobe",
    bundles: [0],
    slot: 2,

    type: 0,
    pattern: [0, 0],
    args: [
      [5, 1, 24, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [10, 0, 0, 0, 0, 0, 0, 0],
      [1, 9, 0, 0, 0, 0, 0, 0],
      [0, 0, 0, 0, 0, 0, 0, 0],
    ],
    numc: [15, 15, 1],
    colors: [
      [
        [130, 0, 0],
        [104, 0, 30],
        [78, 0, 60],
        [52, 0, 90],
        [26, 0, 120],
        [0, 0, 150],
        [0, 28, 120],
        [0, 56, 90],
        [0, 84, 60],
        [0, 112, 30],
        [0, 140, 0],
        [26, 112, 0],
        [52, 84, 0],
        [78, 56, 0],
        [104, 28, 0],
        [0, 0, 0]
      ],
      [
        [195, 0, 0],
        [156, 0, 45],
        [117, 0, 90],
        [78, 0, 135],
        [39, 0, 180],
        [0, 0, 225],
        [0, 42, 180],
        [0, 84, 135],
        [0, 126, 90],
        [0, 168, 45],
        [0, 210, 0],
        [39, 168, 0],
        [78, 126, 0],
        [117, 84, 0],
        [156, 42, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 64, 64, 64],
    thresh1: [0, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Sourcery",
    id: "sourcery",
    bundles: [0],
    slot: 3,

    type: 0,
    pattern: [1, 0],
    args: [
      [1, 1, 5, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [0, 5, 0, 20, 1, 1, 0, 0],
      [5, 0, 0, 20, 1, 1, 0, 0],
      [5, 0, 20, 0, 1, 1, 0, 0]
    ],
    numc: [4, 4, 1],
    colors: [
      [
        [13, 0, 0],
        [6, 0, 52],
        [0, 14, 45],
        [39, 0, 15],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ],
      [
        [1, 0, 0],
        [12, 0, 104],
        [0, 28, 90],
        [78, 0, 30],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
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
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [4, 14, 16, 64],
    thresh1: [16, 64, 64, 64],
    trigger: 0
  },
  {
    name: "Spark Plug",
    id: "spark-plug",
    bundles: [0, 1],
    slot: 7,

    type: 0,
    pattern: [3, 0],
    args: [
      [3, 0, 5, 0],
      [0, 0, 0, 0]
    ],
    timings: [
      [5, 10, 5, 10, 0, 0, 0, 0],
      [2, 0, 4, 20, 0, 0, 0, 0],
      [1, 0, 2, 20, 0, 0, 0, 0]
    ],
    numc: [9, 9, 9],
    colors: [
      [
        [52, 168, 0],
        [39, 14, 0],
        [10, 3, 0],
        [52, 168, 0],
        [6, 0, 45],
        [2, 0, 11],
        [52, 168, 0],
        [0, 35, 37],
        [0, 9, 9],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ],
      [
        [0, 224, 0],
        [39, 14, 0],
        [10, 3, 0],
        [0, 224, 0],
        [6, 0, 45],
        [2, 0, 11],
        [0, 224, 0],
        [0, 35, 37],
        [0, 9, 9],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ],
      [
        [0, 168, 60],
        [39, 14, 0],
        [10, 3, 0],
        [0, 168, 60],
        [6, 0, 45],
        [2, 0, 11],
        [0, 168, 60],
        [0, 35, 37],
        [0, 9, 9],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0]
      ]
    ],
    thresh0: [0, 32, 32, 64],
    thresh1: [0, 32, 32, 64],
    trigger: 0
  }
  ];

  return {
    getModes: function() { return modes; },
    getMode: function(idx) { return modes[idx]; }
  };
}();
