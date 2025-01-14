//@ skip if ["arm", "mips"].include?($architecture) # Due to ICU version.
// Copyright 2019 the V8 project authors. All rights reserved.
// Copyright 2020 Apple Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license.

function shouldBe(actual, expected) {
    if (actual !== expected)
        throw new Error('bad value: ' + actual);
}

if ($vm.icuHeaderVersion() >= 67) {
    let df = new Intl.DateTimeFormat("en");
    // A modern date
    let d1 = new Date(2019, 3, 4);
    let d2 = new Date(2019, 4, 5);
    shouldBe(df.format(d1), df.formatRange(d1, d1));
    shouldBe(df.format(d2), df.formatRange(d2, d2));
    shouldBe("4/4/2019 – 5/5/2019", df.formatRange(d1, d2));
    // An old date just before the Julian / Gregorian switch
    let d3 = new Date(1582, 8, 13);
    let d4 = new Date(1582, 9, 14);
    shouldBe(df.format(d3), df.formatRange(d3, d3));
    shouldBe(df.format(d4), df.formatRange(d4, d4));
    shouldBe("9/13/1582 – 10/14/1582", df.formatRange(d3, d4));
    // An older date
    let d5 = new Date(1000, 0, 1);
    let d6 = new Date(1001, 1, 2);
    shouldBe(df.format(d5), df.formatRange(d5, d5));
    shouldBe(df.format(d6), df.formatRange(d6, d6));
    shouldBe("1/1/1000 – 2/2/1001", df.formatRange(d5, d6));
}
