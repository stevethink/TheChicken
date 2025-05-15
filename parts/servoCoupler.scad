// $fn = 256;
$fn = 128;
// $fn = 64;
//$fn = 32;
//$fn = 16;

wall_thickness = 2;
inner_female = [15.5, 10.7, 18];
// inner_female = [15.5, 10.5, 5];

male_separator = 1.3;

inner_male_slot = [2.6, 8.8, 7];

module female() {
  difference() {
    cube(inner_female + [2 * wall_thickness, 2 * wall_thickness, wall_thickness]);
    translate([wall_thickness, wall_thickness, -0.1]) {
      cube(inner_female);
      translate([0.5, 1, 2 * wall_thickness])
        cube(inner_female - [1, 2, 0]);
    }
  }
}

module male() {
  difference() {
    cube([(4 * inner_male_slot[0]) + (3 * male_separator) + 2 * wall_thickness,
          inner_male_slot[1] + 2 * wall_thickness, inner_male_slot[2] - 0.2]);
    translate([wall_thickness, wall_thickness, -0.1])
    for (i = [0 : 3])
      translate([i * (inner_male_slot[0] + male_separator), 0, 0])
        cube(inner_male_slot);
  }
}

// female();
male();
