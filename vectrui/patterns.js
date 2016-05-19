var Patterns = function() {
  var patterns = [
  {
    name: "Strobe",
    args: [
    {
      name: "Group Size",
      min: 0,
      max: 9,
      tooltip: "Colors used in each set. If 0 or more than the number of colors, all the colors are used.",
      default: 0
    },
    {
      name: "Skip After",
      min: 0,
      max: 9,
      tooltip: "Colors skipped after each set. If 0, it's the same as group size.",
      default: 0
    },
    {
      name: "Repeat Group",
      min: 1,
      max: 100,
      tooltip: "Times set is repeated before skipping.",
      default: 1
    }
    ],
    timings: [
    {
      name: "Strobe",
      tooltip: "Length color is shown.",
      default: 10
    },
    {
      name: "Blank",
      tooltip: "Length blank is shown after each color.",
      default: 16
    },
    {
      name: "Gap",
      tooltip: "Length blank is shown after last blank in set.",
      default: 0
    }
    ]
  },
  {
    name: "Tracer",
    args: [
    {
      min: 0,
      max: 9,
      name: "Group Size",
      tooltip: "Colors used in each set. If 0 or more than the number of colors, all the colors are used.",
      default: 1
    },
    {
      min: 0,
      max: 9,
      name: "Skip After",
      tooltip: "Colors skipped after each set. If 0, it's the same as group size.",
      default: 0
    },
    {
      min: 1,
      max: 100,
      name: "Repeat Tracer",
      tooltip: "Times tracer is repeated before next color set is shown.",
      default: 1
    }
    ],
    timings: [
    {
      name: "Color Strobe",
      tooltip: "Length color is shown.",
      default: 10
    },
    {
      name: "Color Blank",
      tooltip: "Length blank is shown between colors in set. If group size is 1, this is never shown.",
      default: 0
    },
    {
      name: "Tracer Strobe",
      tooltip: "Length tracer color is shown.",
      default: 40
    },
    {
      name: "Tracer Blank",
      tooltip: "Length blank is shown between tracers. If repeat tracer is 1, this is never shown.",
      default: 0
    },
    {
      name: "Gap",
      tooltip: "Length blank is shown between tracer and color.",
      default: 0
    }
    ]
  },
  {
    name: "Morph",
    args: [
    {
      min: 1,
      max: 100,
      name: "Morph Steps",
      tooltip: "Steps from one color to the next.",
      default: 16
    },
    {
      min: 0,
      max: 1,
      name: "Smooth or Fused",
      tooltip: "If 0, colors morph smoothly (A to B to C). If 1, colors morph in reverse (B to A then C to B).",
      default: 0
    },
    ],
    timings: [
    {
      name: "Morph Strobe",
      tooltip: "Length morphing color is shown.",
      default: 6
    },
    {
      name: "Morph Blank",
      tooltip: "Length blank is shown.",
      default: 44
    }
    ]
  },
  {
    name: "Sword",
    args: [
    {
      min: 0,
      max: 9,
      name: "Group Size",
      tooltip: "Colors used in each set. If the last set is not full, blanks are shown.",
      default: 0
    },
    ],
    timings: [
    {
      name: "Outer Strobe",
      tooltip: "Length color is shown for the edges.",
      default: 3
    },
    {
      name: "Outer Blank",
      tooltip: "Length blank is shown between colors.",
      default: 0
    },
    {
      name: "Inner Strobe",
      tooltip: "Length center color is shown.",
      default: 15
    },
    {
      name: "Gap",
      tooltip: "Length blank is shown after last/before first edge color.",
      default: 140
    }
    ]
  },
  {
    name: "Wave",
    args: [
    {
      min: 1,
      max: 100,
      name: "Steps",
      tooltip: "Steps in the wave pattern. If direction is 2 (both), there are double the steps.",
      default: 32
    },
    {
      min: 0,
      max: 2,
      name: "Direction",
      tooltip: "If 0, the wave length increases. If 1, the wave length decreases. If 2, the wave length increases then decreases.",
      default: 2
    },
    {
      min: 0,
      max: 1,
      name: "Wave Target",
      tooltip: "If 0, the wave is color. If 1, the wave is blank.",
      default: 1
    },
    {
      min: 0,
      max: 1,
      name: "Color Change",
      tooltip: "If 0, the color changes after a full wave cycle. If 1, the color changes every strobe.",
      default: 1
    }
    ],
    timings: [
    {
      name: "Base Strobe",
      tooltip: "Minimum length color is shown.",
      default: 3
    },
    {
      name: "Base Blank",
      tooltip: "Minimum length blank is shown.",
      default: 0
    },
    {
      name: "Step Length",
      tooltip: "Length of each step in wave.",
      default: 2
    }
    ]
  },
  {
    name: "Stretch",
    args: [
    {
      min: 1,
      max: 100,
      name: "Steps",
      tooltip: "Steps in the stretch pattern. If direction is 2 (both), there are double the steps.",
      default: 20
    },
    {
      min: 0,
      max: 2,
      name: "Direction",
      tooltip: "If 0, the shift length increases. If 1, the shift length decreases. If 2, the shift length increases then decreases.",
      default: 2
    },
    {
      min: 0,
      max: 1,
      name: "Color Change",
      tooltip: "If 0, the color changes after a full wave cycle. If 1, the color changes every strobe.",
      default: 1
    }
    ],
    timings: [
    {
      name: "Base Strobe",
      tooltip: "Minimum length color is shown.",
      default: 0
    },
    {
      name: "Base Blank",
      tooltip: "Minimum length blank is shown.",
      default: 0
    },
    {
      name: "Step Length",
      tooltip: "Length of each step in stretch.",
      default: 5
    }
    ]
  },
  {
    name: "Shift",
    args: [
    {
      min: 1,
      max: 100,
      name: "Steps",
      tooltip: "Steps in the shift pattern. If direction is 2 (both), there are double the steps.",
      default: 4
    },
    {
      min: 0,
      max: 2,
      name: "Direction",
      tooltip: "If 0, the shift length increases. If 1, the shift length decreases. If 2, the shift length increases then decreases.",
      default: 2
    }
    ],
    timings: [
    {
      name: "Base Strobe",
      tooltip: "Minimum length color is shown.",
      default: 5
    },
    {
      name: "Base Blank",
      tooltip: "Minimum length blank is shown.",
      default: 25
    },
    {
      name: "Step Length",
      tooltip: "Length of each step in shift.",
      default: 5
    },
    {
      name: "Gap",
      tooltip: "Length blank is shown after all colors.",
      default: 100
    }
    ]
  },
  {
    name: "Triple",
    args: [
    {
      min: 1,
      max: 100,
      name: "Repeat A",
      tooltip: "Times to repeat first timing set.",
      default: 2
    },
    {
      min: 1,
      max: 100,
      name: "Repeat B",
      tooltip: "Times to repeat second timing set.",
      default: 2
    },
    {
      min: 0,
      max: 100,
      name: "Repeat C",
      tooltip: "Times to repeat third timing set.",
      default: 2
    },
    {
      min: 0,
      max: 8,
      name: "Skip Colors",
      tooltip: "Colors to skip between patterns.",
      default: 1
    }
    ],
    timings: [
    {
      name: "A Strobe",
      tooltip: "Length color is shown for first pattern.",
      default: 25
    },
    {
      name: "A Blank",
      tooltip: "Length blank is shown between strobes in first pattern. If repeat A is 1, this is not shown.",
      default: 25
    },
    {
      name: "B Strobe",
      tooltip: "Length color is shown for second pattern.",
      default: 10
    },
    {
      name: "B Blank",
      tooltip: "Length blank is shown between strobes in second pattern. If repeat B is 1, this is not shown.",
      default: 40
    },
    {
      name: "C Strobe",
      tooltip: "Length color is shown for third pattern.",
      default: 3
    },
    {
      name: "C Blank",
      tooltip: "Length blank is shown between strobes in third pattern. If repeat C is 1, this is not shown.",
      default: 47
    },
    {
      name: "Gap",
      tooltip: "Length blank is shown between patterns.",
      default: 25
    }
    ]
  },
  {
    name: "Stepper",
    args: [
    {
      min: 1,
      max: 7,
      name: "Steps",
      tooltip: "Number of step timings to use.",
      default: 7
    },
    {
      min: 0,
      max: 1,
      name: "Random Step",
      tooltip: "If 0, steps are in order. If 1, steps are chosen at random.",
      default: 0
    },
    {
      min: 0,
      max: 1,
      name: "Random Color",
      tooltip: "If 0, colors are chosen in order. If 1, colors are chosen at random.",
      default: 0
    },
    {
      min: 0,
      max: 1,
      name: "Step Target",
      tooltip: "If 0, color is shown during step. If 1, blank is shown.",
      default: 0
    }
    ],
    timings: [
    {
      name: "Gap",
      tooltip: "Length color or blank is shown between steps.",
      default: 25
    },
    {
      name: "Step 1",
      tooltip: "Length color or blank is shown for step 1.",
      default: 5
    },
    {
      name: "Step 2",
      tooltip: "Length color or blank is shown for step 2.",
      default: 10
    },
    {
      name: "Step 3",
      tooltip: "Length color or blank is shown for step 3.",
      default: 15
    },
    {
      name: "Step 4",
      tooltip: "Length color or blank is shown for step 4.",
      default: 20
    },
    {
      name: "Step 5",
      tooltip: "Length color or blank is shown for step 5.",
      default: 25
    },
    {
      name: "Step 6",
      tooltip: "Length color or blank is shown for step 6.",
      default: 30
    },
    {
      name: "Step 7",
      tooltip: "Length color or blank is shown for step 7.",
      default: 35
    }
    ]
  },
  {
    name: "Random",
    args: [
    {
      min: 0,
      max: 1,
      name: "Random Color",
      tooltip: "If 0, colors are shown in order. If 1, colors are shown at random.",
      default: 0
    },
    {
      min: 1,
      max: 25,
      name: "Multiplier",
      tooltip: "Multiplies the strobe and blank times.",
      default: 1
    }
    ],
    timings: [
    {
      name: "Strobe Low",
      tooltip: "Minimum length color is shown.",
      default: 5
    },
    {
      name: "Strobe High",
      tooltip: "Maximum length color is shown.",
      default: 25
    },
    {
      name: "Blank Low",
      tooltip: "Minimum length blank is shown.",
      default: 5
    },
    {
      name: "Blank High",
      tooltip: "Maximum length blank is shown.",
      default: 25
    }
    ]
  }
  ];

  return {
    getPatterns: function() { return patterns; },
    getPattern: function(idx) { return patterns[idx]; }
  }
}();
